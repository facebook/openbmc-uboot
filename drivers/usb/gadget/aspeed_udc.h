/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 *
 */

#define gadget_to_aspeed_udc(g)		(container_of(g, struct aspeed_udc_priv, gadget))
#define req_to_aspeed_udc_req(r)	(container_of(r, struct aspeed_udc_request, req))
#define ep_to_aspeed_udc_ep(e)		(container_of(e, struct aspeed_udc_ep, ep))

#define AST_NUM_ENDPOINTS		(21 + 1)

struct aspeed_ep_desc {
	u32	des_0;
	u32	des_1;
};

struct aspeed_udc_ep {
	u32				ep_base;
	char				name[5];
	struct usb_ep			ep;
	unsigned			stopped:1;
	u8				ep_dir;
	void				*ep_buf;
	dma_addr_t			ep_dma;

	/* Request queue */
	struct list_head		queue;
	struct aspeed_udc_priv		*udc;
	struct aspeed_ep_desc		*dma_desc_list;
	dma_addr_t			dma_desc_dma_handle;
	u32				dma_desc_list_wptr;
	u32				data_toggle;
	u32				chunk_max;
};

struct aspeed_udc_priv {
	u32				udc_base;
	u32				maximum_speed;
	struct udevice			*dev;
	struct usb_gadget		gadget;
	struct usb_gadget_driver	*gadget_driver;
	struct aspeed_udc_ep		ep[AST_NUM_ENDPOINTS];
	int				init;
	struct usb_ctrlrequest		*root_setup;

	/* EP0 DMA buffers allocated in one chunk */
	void				*ep0_ctrl_buf;
	u32				ep0_ctrl_dma;
	unsigned			is_udc_control_tx:1;
	int				desc_mode;
	spinlock_t			lock;	/* lock for udc device */
};

struct aspeed_udc_request {
	struct usb_request		req;
	struct list_head		queue;
	u32				actual_dma_length;
	u32				saved_dma_wptr;
};
