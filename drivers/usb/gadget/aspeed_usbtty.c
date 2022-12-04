// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 *
 */

#include <asm/io.h>
#include <asm/byteorder.h>
#include <common.h>
#include <config.h>
#include <dm.h>
#include <fdtdec.h>
#include <reset.h>
#include <usbdevice.h>

#include "ep0.h"

/* number of endpoints on this UDC */
#define UDC_MAX_ENDPOINTS	21

struct aspeed_udc_priv {
	u32				udc_base;
	struct udevice			*dev;
};

static struct urb *ep0_urb;
static struct usb_device_instance *udc_device;

static struct aspeed_udc_priv *aspeed_udc;

/*************************************************************************************/
#define AST_VHUB_CTRL				0x00
#define AST_VHUB_CONF				0x04
#define AST_VHUB_IER				0x08
#define AST_VHUB_ISR				0x0C
#define AST_VHUB_EP_ACK_IER			0x10
#define AST_VHUB_EP_NAK_IER			0x14
#define AST_VHUB_EP_ACK_ISR			0x18
#define AST_VHUB_EP_NAK_ISR			0x1C
#define AST_VHUB_DEV_RESET			0x20
#define AST_VHUB_USB_STS			0x24
#define AST_VHUB_EP_DATA			0x28
#define AST_VHUB_ISO_TX_FAIL			0x2C
#define AST_VHUB_EP0_CTRL			0x30
#define AST_VHUB_EP0_DATA_BUFF			0x34
#define AST_VHUB_EP1_CTRL			0x38
#define AST_VHUB_EP1_STS_CHG			0x3C

#define AST_VHUB_SETUP_DATA0			0x80
#define AST_VHUB_SETUP_DATA1			0x84

/*  ************************************************************************************/
/* AST_VHUB_CTRL				0x00 */
#define ROOT_PHY_CLK_EN				BIT(31)
#define ROOT_PHY_RESET_DIS			BIT(11)
#define ROOT_UPSTREAM_EN			BIT(0)

/* AST_VHUB_ISR					0x0C */
#define ISR_EP_NAK				BIT(17)
#define ISR_EP_ACK_STALL			BIT(16)
#define ISR_DEVICE7				BIT(15)
#define ISR_DEVICE6				BIT(14)
#define ISR_DEVICE5				BIT(13)
#define ISR_DEVICE4				BIT(12)
#define ISR_DEVICE3				BIT(11)
#define ISR_DEVICE2				BIT(10)
#define ISR_DEVICE1				BIT(9)
#define ISR_SUSPEND_RESUME			BIT(8)
#define ISR_BUS_SUSPEND				BIT(7)
#define ISR_BUS_RESET				BIT(6)
#define ISR_HUB_EP1_IN_DATA_ACK			BIT(5)
#define ISR_HUB_EP0_IN_DATA_NAK			BIT(4)
#define ISR_HUB_EP0_IN_ACK_STALL		BIT(3)
#define ISR_HUB_EP0_OUT_NAK			BIT(2)
#define ISR_HUB_EP0_OUT_ACK_STALL		BIT(1)
#define ISR_HUB_EP0_SETUP			BIT(0)

/* AST_VHUB_USB_STS				0x24 */
#define USB_BUS_HIGH_SPEED			BIT(27)

/* AST_VHUB_EP0_CTRL				0x30 */
#define EP0_GET_RX_LEN(x)			(((x) >> 16) & 0x7f)
#define EP0_TX_LEN(x)				(((x) & 0x7f) << 8)
#define EP0_RX_BUFF_RDY				BIT(2)
#define EP0_TX_BUFF_RDY				BIT(1)
#define EP0_STALL				BIT(0)

#define AST_UDC_DEV_CTRL			0x00
#define AST_UDC_DEV_ISR				0x04
#define AST_UDC_DEV_EP0_CTRL			0x08
#define AST_UDC_DEV_EP0_DATA_BUFF		0x0C

/*************************************************************************************/
#define AST_EP_BASE				0x200
#define AST_EP_OFFSET				0x10
#define AST_EP_CONFIG				0x00
#define AST_EP_DMA_CTRL				0x04
#define AST_EP_DMA_BUFF				0x08
#define AST_EP_DMA_STS				0x0C

/*************************************************************************************/
/* AST_EP_CONFIG				0x00 */
#define EP_SET_MAX_PKT(x)			(((x) & 0x3ff) << 16)
#define EP_SET_EP_STALL				BIT(12)
#define EP_SET_EP_NUM(x)			(((x) & 0xf) << 8)
#define EP_TYPE_BULK_IN				(0x2 << 4)
#define EP_TYPE_BULK_OUT			(0x3 << 4)
#define EP_TYPE_INT_IN				(0x4 << 4)
#define EP_TYPE_INT_OUT				(0x5 << 4)
#define EP_TYPE_ISO_IN				(0x6 << 4)
#define EP_TYPE_ISO_OUT				(0x7 << 4)
#define EP_ENABLE				BIT(0)

/* AST_EP_DMA_CTRL				0x04 */
#define EP_SINGLE_DESC_MODE			BIT(1)

/* AST_EP_DMA_STS				0x0C */
#define AST_EP_TX_DATA_BYTE(x)			(((x) & 0x3ff) << 16)
#define AST_EP_START_TRANS			BIT(0)

/*-------------------------------------------------------------------------*/
#define ast_udc_read(offset) \
	__raw_readl(aspeed_udc->udc_base + (offset))
#define ast_udc_write(val, offset) \
	__raw_writel((u32)val, aspeed_udc->udc_base + (offset))

#define ast_ep_read(ep_reg, reg) \
	__raw_readl((ep_reg) + (reg))
#define ast_ep_write(ep_reg, val, reg) \
	__raw_writel((u32)val, (ep_reg) + (reg))
/*-------------------------------------------------------------------------*/

int is_usbd_high_speed(void)
{
	if (ast_udc_read(AST_VHUB_USB_STS) & USB_BUS_HIGH_SPEED)
		return 1;

	return 0;
}

static void udc_stall_ep(u32 ep_num)
{
	u32 ep_reg;

	usbdbg("stall ep: %d", ep_num);

	if (ep_num) {
		ep_reg = aspeed_udc->udc_base + AST_EP_BASE +
			 (AST_EP_OFFSET * (ep_num - 1));
		ast_ep_write(ep_reg, ast_ep_read(ep_reg, AST_EP_CONFIG) |
			     EP_SET_EP_STALL,
			     AST_EP_CONFIG);

	} else {
		ast_udc_write(ast_udc_read(AST_VHUB_EP0_CTRL) | EP0_STALL,
			      AST_VHUB_EP0_CTRL);
	}
}

int udc_endpoint_write(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb = endpoint->tx_urb;
	u32 remaining_packet;
	u32 ep_reg, length;
	int timeout = 2000; // 2ms
	int ep_num;
	u8 *data;

	if (!endpoint) {
		usberr("Error input: endpoint\n");
		return -1;
	}

	ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;
	ep_reg = aspeed_udc->udc_base + AST_EP_BASE + (AST_EP_OFFSET * (ep_num - 1));
	remaining_packet = urb->actual_length - endpoint->sent;

	if (endpoint->tx_packetSize < remaining_packet)
		length = endpoint->tx_packetSize;
	else
		length = remaining_packet;

//	usbdbg("ep: %d, trans len: %d, ep sent: %d, urb actual len: %d\n",
//		ep_num, length, endpoint->sent, urb->actual_length);

	data = (u8 *)urb->buffer;
	data += endpoint->sent;

	// tx trigger
	ast_ep_write(ep_reg, data, AST_EP_DMA_BUFF);
	ast_ep_write(ep_reg, AST_EP_TX_DATA_BYTE(length), AST_EP_DMA_STS);
	ast_ep_write(ep_reg, AST_EP_TX_DATA_BYTE(length) | AST_EP_START_TRANS,
		     AST_EP_DMA_STS);

	endpoint->last = length;

	// wait for tx complete
	while (ast_ep_read(ep_reg, AST_EP_DMA_STS) & 0x1) {
		if (timeout-- == 0)
			return -1;

		udelay(1);
	}

	return 0;
}

static void aspeed_udc_ep_handle(struct usb_endpoint_instance *endpoint)
{
	int ep_isout, ep_num;
	int nbytes;
	u32 ep_reg;

	ep_isout = (endpoint->endpoint_address & USB_ENDPOINT_DIR_MASK) ==
		   USB_DIR_OUT;
	ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;

	ep_reg = aspeed_udc->udc_base + AST_EP_BASE + (AST_EP_OFFSET * (ep_num - 1));

	if (ep_isout) {
		nbytes = (ast_ep_read(ep_reg, AST_EP_DMA_STS) >> 16) & 0x7ff;
		usbd_rcv_complete(endpoint, nbytes, 0);

		//trigger next
		ast_ep_write(ep_reg, AST_EP_START_TRANS, AST_EP_DMA_STS);

	} else {
		usbd_tx_complete(endpoint);
	}
}

static void aspeed_udc_ep0_rx(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb;
	u8 *buff;

	if (!endpoint) {
		usberr("Error input: endpoint\n");
		return;
	}

	urb = endpoint->rcv_urb;
	if (!urb) {
		usberr("Error: rcv_urb is empty\n");
		return;
	}

	buff = urb->buffer;
	ast_udc_write(buff, AST_VHUB_EP0_DATA_BUFF);

	// trigger rx
	ast_udc_write(EP0_RX_BUFF_RDY, AST_VHUB_EP0_CTRL);
}

static void aspeed_udc_ep0_out(struct usb_endpoint_instance *endpoint)
{
	u16 rx_len = EP0_GET_RX_LEN(ast_udc_read(AST_VHUB_EP0_CTRL));

	/* Check direction */
	if ((ep0_urb->device_request.bmRequestType & USB_REQ_DIRECTION_MASK) ==
		USB_REQ_DEVICE2HOST) {
		if (rx_len != 0)
			usberr("Unexpected USB REQ direction: D2H\n");

	} else {
		usbdbg("EP0 OUT packet ACK, sent zero-length packet");
		ast_udc_write(EP0_TX_BUFF_RDY, AST_VHUB_EP0_CTRL);
	}
}

static void aspeed_udc_ep0_tx(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb;
	u32 last;

	if (!endpoint) {
		usberr("Error input: endpoint\n");
		return;
	}

	urb = endpoint->tx_urb;
	if (!urb) {
		usberr("Error: tx_urb is empty\n");
		return;
	}

	usbdbg("urb->buffer: %p, buffer_length: %d, actual_length: %d, sent:%d",
	       urb->buffer, urb->buffer_length,
	       urb->actual_length, endpoint->sent);

	last = min((int)(urb->actual_length - endpoint->sent),
		   (int)endpoint->tx_packetSize);

	if (last) {
		u8 *cp = urb->buffer + endpoint->sent;

		usbdbg("send address %x", (u32)cp);

		/*
		 * This ensures that USBD packet fifo is accessed
		 * - through word aligned pointer or
		 * - through non word aligned pointer but only
		 *   with a max length to make the next packet
		 *   word aligned
		 */

		usbdbg("ep sent: %d, tx_packetSize: %d, last: %d",
		       endpoint->sent, endpoint->tx_packetSize, last);

		ast_udc_write(cp, AST_VHUB_EP0_DATA_BUFF);

		// trigger tx
		ast_udc_write(EP0_TX_LEN(last), AST_VHUB_EP0_CTRL);
		ast_udc_write(EP0_TX_LEN(last) | EP0_TX_BUFF_RDY, AST_VHUB_EP0_CTRL);
	}

	endpoint->last = last;
}

void aspeed_udc_ep0_in(struct usb_endpoint_instance *endpoint)
{
	struct usb_device_request *request = &ep0_urb->device_request;

	usbdbg("aspeed_udc_ep0_in");

	/* Check direction */
	if ((request->bmRequestType & USB_REQ_DIRECTION_MASK) ==
	    USB_REQ_HOST2DEVICE) {
		/*
		 * This tx interrupt must be for a control write status
		 * stage packet.
		 */
		usbdbg("ACK on EP0 control write status stage packet");
	} else {
		/*
		 * This tx interrupt must be for a control read data
		 * stage packet.
		 */
		int wLength = le16_to_cpu(request->wLength);

		/*
		 * Update our count of bytes sent so far in this
		 * transfer.
		 */
		endpoint->sent += endpoint->last;

		/*
		 * We are finished with this transfer if we have sent
		 * all of the bytes in our tx urb (urb->actual_length)
		 * unless we need a zero-length terminating packet.  We
		 * need a zero-length terminating packet if we returned
		 * fewer bytes than were requested (wLength) by the host,
		 * and the number of bytes we returned is an exact
		 * multiple of the packet size endpoint->tx_packetSize.
		 */
		if (endpoint->sent == ep0_urb->actual_length &&
		    (ep0_urb->actual_length == wLength ||
		     endpoint->last != endpoint->tx_packetSize)) {
			/* Done with control read data stage. */
			usbdbg("control read data stage complete");
			//trigger for rx
			endpoint->rcv_urb = ep0_urb;
			endpoint->sent = 0;
			aspeed_udc_ep0_rx(endpoint);

		} else {
			/*
			 * We still have another packet of data to send
			 * in this control read data stage or else we
			 * need a zero-length terminating packet.
			 */
			usbdbg("ACK control read data stage packet");

			aspeed_udc_ep0_tx(endpoint);
		}
	}
}

static void aspeed_udc_setup_handle(struct usb_endpoint_instance *endpoint)
{
	u32 *setup = (u32 *)(aspeed_udc->udc_base + AST_VHUB_SETUP_DATA0);
	u8 *datap = (u8 *)&ep0_urb->device_request;

	usbdbg("-> Entering device setup");

	/* 8 bytes setup packet */
	memcpy(datap, setup, sizeof(setup) * 2);

	/* Try to process setup packet */
	if (ep0_recv_setup(ep0_urb)) {
		/* Not a setup packet, stall next EP0 transaction */
		udc_stall_ep(0);
		usbinfo("Cannot parse setup packet, wait another setup...\n");
		return;
	}

	/* Check direction */
	if ((ep0_urb->device_request.bmRequestType & USB_REQ_DIRECTION_MASK) ==
	    USB_REQ_HOST2DEVICE) {
		switch (ep0_urb->device_request.bRequest) {
		case USB_REQ_SET_ADDRESS:
			usbdbg("set addr: %x", ep0_urb->device_request.wValue);
			ast_udc_write(ep0_urb->device_request.wValue,
				      AST_VHUB_CONF);
			ast_udc_write(EP0_TX_BUFF_RDY, AST_VHUB_EP0_CTRL);
			usbd_device_event_irq(udc_device,
					      DEVICE_ADDRESS_ASSIGNED, 0);
			break;

		case USB_REQ_SET_CONFIGURATION:
			usbdbg("set configuration");
			ast_udc_write(EP0_TX_BUFF_RDY, AST_VHUB_EP0_CTRL);
			usbd_device_event_irq(udc_device,
					      DEVICE_CONFIGURED, 0);
			break;

		default:
			if (ep0_urb->device_request.wLength) {
				endpoint->rcv_urb = ep0_urb;
				endpoint->sent = 0;
				aspeed_udc_ep0_rx(endpoint);

			} else {
				// send zero-length IN packet
				ast_udc_write(EP0_TX_BUFF_RDY,
					      AST_VHUB_EP0_CTRL);
			}
			break;
		}

	} else {
		usbdbg("control read on EP0");
		/*
		 * The ep0_recv_setup function has already placed our response
		 * packet data in ep0_urb->buffer and the packet length in
		 * ep0_urb->actual_length.
		 */
		endpoint->tx_urb = ep0_urb;
		endpoint->sent = 0;
		aspeed_udc_ep0_tx(endpoint);
	}

	usbdbg("<- Leaving device setup");
}

void udc_irq(void)
{
	u32 isr = ast_udc_read(AST_VHUB_ISR);
	u32 ep_isr;
	int i;

	if (!isr)
		return;

	if (isr & ISR_BUS_RESET) {
		usbdbg("ISR_BUS_RESET");
		ast_udc_write(ISR_BUS_RESET, AST_VHUB_ISR);
		usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
	}

	if (isr & ISR_BUS_SUSPEND) {
		usbdbg("ISR_BUS_SUSPEND");
		ast_udc_write(ISR_BUS_SUSPEND, AST_VHUB_ISR);
		usbd_device_event_irq(udc_device, DEVICE_BUS_INACTIVE, 0);
	}

	if (isr & ISR_SUSPEND_RESUME) {
		usbdbg("ISR_SUSPEND_RESUME");
		ast_udc_write(ISR_SUSPEND_RESUME, AST_VHUB_ISR);
		usbd_device_event_irq(udc_device, DEVICE_BUS_ACTIVITY, 0);
	}

	if (isr & ISR_HUB_EP0_IN_ACK_STALL) {
//		usbdbg("ISR_HUB_EP0_IN_ACK_STALL");
		ast_udc_write(ISR_HUB_EP0_IN_ACK_STALL, AST_VHUB_ISR);
		aspeed_udc_ep0_in(udc_device->bus->endpoint_array);
	}

	if (isr & ISR_HUB_EP0_OUT_ACK_STALL) {
//		usbdbg("ISR_HUB_EP0_OUT_ACK_STALL");
		ast_udc_write(ISR_HUB_EP0_OUT_ACK_STALL, AST_VHUB_ISR);
		aspeed_udc_ep0_out(udc_device->bus->endpoint_array);
	}

	if (isr & ISR_HUB_EP0_OUT_NAK) {
//		usbdbg("ISR_HUB_EP0_OUT_NAK");
		ast_udc_write(ISR_HUB_EP0_OUT_NAK, AST_VHUB_ISR);
	}

	if (isr & ISR_HUB_EP0_IN_DATA_NAK) {
//		usbdbg("ISR_HUB_EP0_IN_DATA_ACK");
		ast_udc_write(ISR_HUB_EP0_IN_DATA_NAK, AST_VHUB_ISR);
	}

	if (isr & ISR_HUB_EP0_SETUP) {
		usbdbg("SETUP");
		ast_udc_write(ISR_HUB_EP0_SETUP, AST_VHUB_ISR);
		aspeed_udc_setup_handle(udc_device->bus->endpoint_array);
	}

	if (isr & ISR_HUB_EP1_IN_DATA_ACK) {
		// HUB Bitmap control
		usberr("Error: EP1 IN ACK");
		ast_udc_write(ISR_HUB_EP1_IN_DATA_ACK, AST_VHUB_ISR);
		ast_udc_write(0x00, AST_VHUB_EP1_STS_CHG);
	}

	if (isr & ISR_DEVICE1)
		usberr("ISR_DEVICE1");

	if (isr & ISR_DEVICE2)
		usberr("ISR_DEVICE2");

	if (isr & ISR_DEVICE3)
		usberr("ISR_DEVICE3");

	if (isr & ISR_DEVICE4)
		usberr("ISR_DEVICE4");

	if (isr & ISR_DEVICE5)
		usberr("ISR_DEVICE5");

	if (isr & ISR_DEVICE6)
		usberr("ISR_DEVICE6");

	if (isr & ISR_DEVICE7)
		usberr("ISR_DEVICE7");

	if (isr & ISR_EP_ACK_STALL) {
//		usbdbg("ISR_EP_ACK_STALL");
		ep_isr = ast_udc_read(AST_VHUB_EP_ACK_ISR);
		for (i = 0; i < UDC_MAX_ENDPOINTS; i++) {
			if (ep_isr & (0x1 << i)) {
				ast_udc_write(BIT(i), AST_VHUB_EP_ACK_ISR);
				aspeed_udc_ep_handle(udc_device->bus->endpoint_array + i + 1);
			}
		}
	}

	if (isr & ISR_EP_NAK) {
		usbdbg("ISR_EP_NAK");
		ast_udc_write(ISR_EP_NAK, AST_VHUB_ISR);
	}
}

/*
 * udc_unset_nak
 *
 * Suspend sending of NAK tokens for DATA OUT tokens on a given endpoint.
 * Switch off NAKing on this endpoint to accept more data output from host.
 */
void udc_unset_nak(int ep_num)
{
/* Do nothing */
}

/*
 * udc_set_nak
 *
 * Allow upper layers to signal lower layers should not accept more RX data
 */
void udc_set_nak(int ep_num)
{
/* Do nothing */
}

/* Associate a physical endpoint with endpoint instance */
void udc_setup_ep(struct usb_device_instance *device, unsigned int id,
		  struct usb_endpoint_instance *endpoint)
{
	int ep_num, ep_addr, ep_isout, ep_type, ep_size;
	u32 ep_conf;
	u32 ep_reg;

	usbdbg("setting up endpoint id: %d", id);

	if (!endpoint) {
		usberr("Error: invalid endpoint");
		return;
	}

	ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;
	if (ep_num >= UDC_MAX_ENDPOINTS) {
		usberr("Error: ep num is out-of-range %d", ep_num);
		return;
	}

	if (ep_num == 0) {
		/* Done for ep0 */
		return;
	}

	ep_addr = endpoint->endpoint_address;
	ep_isout = (ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT;
	ep_type = ep_isout ? endpoint->rcv_attributes : endpoint->tx_attributes;
	ep_size = ep_isout ? endpoint->rcv_packetSize : endpoint->tx_packetSize;

	usbdbg("addr %x, num %d, dir %s, type %s, packet size %d",
	       ep_addr, ep_num,
	       ep_isout ? "out" : "in",
	       ep_type == USB_ENDPOINT_XFER_ISOC ? "isoc" :
	       ep_type == USB_ENDPOINT_XFER_BULK ? "bulk" :
	       ep_type == USB_ENDPOINT_XFER_INT ? "int" : "???",
	       ep_size);

	/* Configure EP */
	if (ep_size == 1024)
		ep_conf = 0;
	else
		ep_conf = EP_SET_MAX_PKT(ep_size);

	ep_conf |= EP_SET_EP_NUM(ep_num);

	switch (ep_type) {
	case USB_ENDPOINT_XFER_ISOC:
		if (ep_isout)
			ep_conf |= EP_TYPE_ISO_OUT;
		else
			ep_conf |= EP_TYPE_ISO_IN;
		break;
	case USB_ENDPOINT_XFER_BULK:
		if (ep_isout)
			ep_conf |= EP_TYPE_BULK_OUT;
		else
			ep_conf |= EP_TYPE_BULK_IN;
		break;
	case USB_ENDPOINT_XFER_INT:
		if (ep_isout)
			ep_conf |= EP_TYPE_INT_OUT;
		else
			ep_conf |= EP_TYPE_INT_IN;
		break;
	}

	ep_reg = aspeed_udc->udc_base + AST_EP_BASE + (AST_EP_OFFSET * (ep_num - 1));

	ast_ep_write(ep_reg, EP_SINGLE_DESC_MODE, AST_EP_DMA_CTRL);
	ast_ep_write(ep_reg, 0, AST_EP_DMA_STS);
	ast_ep_write(ep_reg, ep_conf | EP_ENABLE, AST_EP_CONFIG);

	//also setup dma
	if (ep_isout) {
		ast_ep_write(ep_reg, endpoint->rcv_urb->buffer, AST_EP_DMA_BUFF);
		ast_ep_write(ep_reg, AST_EP_START_TRANS, AST_EP_DMA_STS);

	} else {
		ast_ep_write(ep_reg, endpoint->tx_urb->buffer, AST_EP_DMA_BUFF);
	}
}

/* Connect the USB device to the bus */
void udc_connect(void)
{
	usbdbg("UDC connect");
	ast_udc_write(ast_udc_read(AST_VHUB_CTRL) | ROOT_UPSTREAM_EN,
		      AST_VHUB_CTRL);
}

/* Disconnect the USB device to the bus */
void udc_disconnect(void)
{
	usbdbg("UDC disconnect");
	ast_udc_write(ast_udc_read(AST_VHUB_CTRL) & ~ROOT_UPSTREAM_EN,
		      AST_VHUB_CTRL);
}

void udc_enable(struct usb_device_instance *device)
{
	usbdbg("enable UDC");

	udc_device = device;
	if (!ep0_urb)
		ep0_urb = usbd_alloc_urb(udc_device,
					 udc_device->bus->endpoint_array);
	else
		usbinfo("ep0_urb %p already allocated", ep0_urb);
}

void udc_disable(void)
{
	usbdbg("disable UDC");

	/* Free ep0 URB */
	if (ep0_urb) {
		usbd_dealloc_urb(ep0_urb);
		ep0_urb = NULL;
	}

	/* Reset device pointer */
	udc_device = NULL;
}

/* Allow udc code to do any additional startup */
void udc_startup_events(struct usb_device_instance *device)
{
	usbdbg("udc_startup_events");

	/* The DEVICE_INIT event puts the USB device in the state STATE_INIT */
	usbd_device_event_irq(device, DEVICE_INIT, 0);

	/* The DEVICE_CREATE event puts the USB device in the state
	 * STATE_ATTACHED
	 */
	usbd_device_event_irq(device, DEVICE_CREATE, 0);

	udc_enable(device);
}

int udc_init(void)
{
	usbdbg("udc_init");

	if (!aspeed_udc) {
		usberr("Error: udc driver is not init yet");
		return -1;
	}

	// Disable PHY reset
	ast_udc_write(ROOT_PHY_CLK_EN | ROOT_PHY_RESET_DIS, AST_VHUB_CTRL);

	ast_udc_write(0, AST_VHUB_DEV_RESET);

	ast_udc_write(~BIT(18), AST_VHUB_ISR);
	ast_udc_write(~BIT(18), AST_VHUB_IER);

	ast_udc_write(~BIT(UDC_MAX_ENDPOINTS), AST_VHUB_EP_ACK_ISR);
	ast_udc_write(~BIT(UDC_MAX_ENDPOINTS), AST_VHUB_EP_ACK_IER);

	ast_udc_write(0, AST_VHUB_EP0_CTRL);
	ast_udc_write(0, AST_VHUB_EP1_CTRL);

	return 0;
}

static int aspeed_udc_probe(struct udevice *dev)
{
	struct reset_ctl udc_reset_ctl;
	int ret;

	ret = reset_get_by_index(dev, 0, &udc_reset_ctl);
	if (ret) {
		printf("%s: Failed to get udc reset signal\n", __func__);
		return ret;
	}

	reset_assert(&udc_reset_ctl);

	// Wait 10ms for PLL locking
	mdelay(10);
	reset_deassert(&udc_reset_ctl);

	return 0;
}

static int aspeed_udc_ofdata_to_platdata(struct udevice *dev)
{
	aspeed_udc = dev_get_priv(dev);

	/* Get the controller base address */
	aspeed_udc->udc_base = (u32)devfdt_get_addr_index(dev, 0);

	return 0;
}

static const struct udevice_id aspeed_udc_ids[] = {
	{ .compatible = "aspeed,ast2600-usb-vhub" },
	{ }
};

U_BOOT_DRIVER(aspeed_udc) = {
	.name			= "aspeed_udc",
	.id			= UCLASS_MISC,
	.of_match		= aspeed_udc_ids,
	.probe			= aspeed_udc_probe,
	.ofdata_to_platdata	= aspeed_udc_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct aspeed_udc_priv),
};
