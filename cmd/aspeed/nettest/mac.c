// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */
//#define MAC_DEBUG_REGRW_MAC
//#define MAC_DEBUG_REGRW_PHY
//#define MAC_DEBUG_REGRW_SCU
//#define MAC_DEBUG_REGRW_WDT
//#define MAC_DEBUG_REGRW_SDR
//#define MAC_DEBUG_REGRW_SMB
//#define MAC_DEBUG_REGRW_TIMER
//#define MAC_DEBUG_REGRW_GPIO
//#define MAC_DEBUG_MEMRW_Dat
//#define MAC_DEBUG_MEMRW_Des

#define MAC_C

#include "swfunc.h"

#include "comminf.h"
#include <command.h>
#include <common.h>
#include <malloc.h>
#include "mem_io.h"
// -------------------------------------------------------------
const uint32_t ARP_org_data[16] = {
    0xffffffff,
    0x0000ffff, // SA:00-00-
    0x12345678, // SA:78-56-34-12
    0x01000608, // ARP(0x0806)
    0x04060008,
    0x00000100, // sender MAC Address: 00 00
    0x12345678, // sender MAC Address: 12 34 56 78
    0xeb00a8c0, // sender IP Address:  192.168.0.235 (C0.A8.0.EB)
    0x00000000, // target MAC Address: 00 00 00 00
    0xa8c00000, // target MAC Address: 00 00, target IP Address:192.168
    0x00005c00, // target IP Address:  0.92 (C0.A8.0.5C)
		//	0x00000100, // target IP Address:  0.1 (C0.A8.0.1)
		//	0x0000de00, // target IP Address:  0.222 (C0.A8.0.DE)
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc68e2bd5};

//------------------------------------------------------------
// Read Memory
//------------------------------------------------------------
uint32_t Read_Mem_Dat_NCSI_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMRd-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( readl(addr) ) );
#endif
	return ( SWAP_4B_LEDN_MEM( readl(addr) ) );
}

uint32_t Read_Mem_Des_NCSI_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMRd-Des] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(readl(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(readl(addr)));
}

uint32_t Read_Mem_Dat_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMRd-Dat] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(readl(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(readl(addr)));
}

uint32_t Read_Mem_Des_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMRd-Des] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(readl(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(readl(addr)));
}

//------------------------------------------------------------
// Read Register
//------------------------------------------------------------
uint32_t mac_reg_read(MAC_ENGINE *p_eng, uint32_t addr)
{
	return readl(p_eng->run.mac_base + addr);
}

//------------------------------------------------------------
// Write Memory
//------------------------------------------------------------
void Write_Mem_Dat_NCSI_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMWr-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	writel(data, addr);
}
void Write_Mem_Des_NCSI_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMWr-Des] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	writel(data, addr);
}
void Write_Mem_Dat_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMWr-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	writel(data, addr);
}
void Write_Mem_Des_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMWr-Des] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	writel(data, addr);
}

//------------------------------------------------------------
// Write Register
//------------------------------------------------------------
void mac_reg_write(MAC_ENGINE *p_eng, uint32_t addr, uint32_t data)
{
	writel(data, p_eng->run.mac_base + addr);
}


//------------------------------------------------------------
// Others
//------------------------------------------------------------
void debug_pause (void) {
#ifdef DbgPrn_Enable_Debug_pause
	GET_CAHR();
#endif
}

//------------------------------------------------------------
void dump_mac_ROreg(MAC_ENGINE *p_eng) 
{	
	int i = 0xa0;

	printf("\nMAC%d base 0x%08x", p_eng->run.mac_idx, p_eng->run.mac_base);
	printf("\n%02x:", i);
	for (i = 0xa0; i <= 0xc8; i += 4) {
		printf("%08x ", mac_reg_read(p_eng, i));
		if ((i & 0xf) == 0xc)
			printf("\n%02x:", i + 4);
	}
	printf("\n");
}

//------------------------------------------------------------
// IO delay
//------------------------------------------------------------
static void get_mac_1g_delay_1(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	int tx_d, rx_d;
	mac_delay_1g_t reg;

	reg.w = readl(addr);
	tx_d = reg.b.tx_delay_1;
	rx_d = reg.b.rx_delay_1;	
#ifdef CONFIG_ASPEED_AST2600
	if (reg.b.rx_clk_inv_1 == 1) {
		rx_d = (-1) * rx_d;
	}
#endif
	*p_tx_d = tx_d;
	*p_rx_d = rx_d;
	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void get_mac_1g_delay_2(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	int tx_d, rx_d;
	mac_delay_1g_t reg;

	reg.w = readl(addr);
	tx_d = reg.b.tx_delay_2;
	rx_d = reg.b.rx_delay_2;	
#ifdef CONFIG_ASPEED_AST2600
	if (reg.b.rx_clk_inv_2 == 1) {
		rx_d = (-1) * rx_d;
	}
#endif
	*p_tx_d = tx_d;
	*p_rx_d = rx_d;
	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void get_mac_100_10_delay_1(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	int tx_d, rx_d;
	mac_delay_100_10_t reg;

	reg.w = readl(addr);
	tx_d = reg.b.tx_delay_1;
	rx_d = reg.b.rx_delay_1;	
#ifdef CONFIG_ASPEED_AST2600
	if (reg.b.rx_clk_inv_1 == 1) {
		rx_d = (-1) * rx_d;
	}
#endif
	*p_tx_d = tx_d;
	*p_rx_d = rx_d;
	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void get_mac_100_10_delay_2(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	int tx_d, rx_d;
	mac_delay_100_10_t reg;

	reg.w = readl(addr);
	tx_d = reg.b.tx_delay_2;
	rx_d = reg.b.rx_delay_2;	
#ifdef CONFIG_ASPEED_AST2600
	if (reg.b.rx_clk_inv_2 == 1) {
		rx_d = (-1) * rx_d;
	}
#endif
	*p_tx_d = tx_d;
	*p_rx_d = rx_d;
	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void get_mac_rmii_delay_1(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	mac_delay_1g_t reg;
	
	reg.w = readl(addr);
	*p_rx_d = reg.b.rx_delay_1;
	*p_tx_d = reg.b.rmii_tx_data_at_falling_1;

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       *p_rx_d, *p_tx_d);
}
static void get_mac_rmii_delay_2(uint32_t addr, int32_t *p_rx_d, int32_t *p_tx_d)
{
	mac_delay_1g_t reg;
	
	reg.w = readl(addr);
	*p_rx_d = reg.b.rx_delay_2;
	*p_tx_d = reg.b.rmii_tx_data_at_falling_2;

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       *p_rx_d, *p_tx_d);
}

static 
void get_mac1_1g_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_1g_delay_1(p_eng->io.mac12_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac1_100m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_1(p_eng->io.mac12_100m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac1_10m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_1(p_eng->io.mac12_10m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac2_1g_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_1g_delay_2(p_eng->io.mac12_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac2_100m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_2(p_eng->io.mac12_100m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac2_10m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_2(p_eng->io.mac12_10m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac3_1g_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_1g_delay_1(p_eng->io.mac34_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac3_100m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_1(p_eng->io.mac34_100m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac3_10m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_1(p_eng->io.mac34_10m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac4_1g_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_1g_delay_2(p_eng->io.mac34_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac4_100m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_2(p_eng->io.mac34_100m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac4_10m_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_100_10_delay_2(p_eng->io.mac34_10m_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac1_rmii_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_rmii_delay_1(p_eng->io.mac12_1g_delay.addr, p_rx_d, p_tx_d);	
}
static
void get_mac2_rmii_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_rmii_delay_2(p_eng->io.mac12_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac3_rmii_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_rmii_delay_1(p_eng->io.mac34_1g_delay.addr, p_rx_d, p_tx_d);
}
static
void get_mac4_rmii_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	get_mac_rmii_delay_2(p_eng->io.mac34_1g_delay.addr, p_rx_d, p_tx_d);
}
#if !defined(CONFIG_ASPEED_AST2600)
static
void get_dummy_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	debug("%s\n", __func__);
}
#endif

/**
 * @brief function pointer table to get current delay setting
 * 
 * get_delay_func_tbl[rmii/rgmii][mac_idx][speed_idx 1g/100m/10m]
*/
typedef void (*pfn_get_delay) (MAC_ENGINE *, int32_t *, int32_t *);
pfn_get_delay get_delay_func_tbl[2][4][3] = {
	{
		{get_mac1_rmii_delay, get_mac1_rmii_delay, get_mac1_rmii_delay},
		{get_mac2_rmii_delay, get_mac2_rmii_delay, get_mac2_rmii_delay},
#if defined(CONFIG_ASPEED_AST2600)
		{get_mac3_rmii_delay, get_mac3_rmii_delay, get_mac3_rmii_delay},
		{get_mac4_rmii_delay, get_mac4_rmii_delay, get_mac4_rmii_delay},
#else
		{get_dummy_delay, get_dummy_delay, get_dummy_delay},
		{get_dummy_delay, get_dummy_delay, get_dummy_delay},
#endif		
	},
	{
		{get_mac1_1g_delay, get_mac1_100m_delay, get_mac1_10m_delay},
		{get_mac2_1g_delay, get_mac2_100m_delay, get_mac2_10m_delay},
#if defined(CONFIG_ASPEED_AST2600)		
		{get_mac3_1g_delay, get_mac3_100m_delay, get_mac3_10m_delay},
		{get_mac4_1g_delay, get_mac4_100m_delay, get_mac4_10m_delay},
#else
		{get_dummy_delay, get_dummy_delay, get_dummy_delay},
		{get_dummy_delay, get_dummy_delay, get_dummy_delay},
#endif		
	}
};
void mac_get_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
#if 1
	uint32_t rgmii = (uint32_t)p_eng->run.is_rgmii;
	uint32_t mac_idx = p_eng->run.mac_idx;
	uint32_t speed_idx = p_eng->run.speed_idx;

	get_delay_func_tbl[rgmii][mac_idx][speed_idx] (p_eng, p_rx_d, p_tx_d);
#else
	/* for test */
	uint32_t rgmii;
	uint32_t mac_idx;
	uint32_t speed_idx;
	for (rgmii = 0; rgmii < 2; rgmii++)
		for (mac_idx = 0; mac_idx < 4; mac_idx++)
			for (speed_idx = 0; speed_idx < 3; speed_idx++)
				get_delay_func_tbl[rgmii][mac_idx][speed_idx](
				    p_eng, p_rx_d, p_tx_d);
#endif	
}

void mac_get_max_available_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	uint32_t rgmii = (uint32_t)p_eng->run.is_rgmii;
	uint32_t mac_idx = p_eng->run.mac_idx;
	int32_t tx_max, rx_max;

	if (rgmii) {
		if (mac_idx > 1) {
			tx_max = p_eng->io.mac34_1g_delay.tx_max;
			rx_max = p_eng->io.mac34_1g_delay.rx_max;
		} else {
			tx_max = p_eng->io.mac12_1g_delay.tx_max;
			rx_max = p_eng->io.mac12_1g_delay.rx_max;
		}
	} else {
		if (mac_idx > 1) {
			tx_max = p_eng->io.mac34_1g_delay.rmii_tx_max;
			rx_max = p_eng->io.mac34_1g_delay.rmii_rx_max;
		} else {
			tx_max = p_eng->io.mac12_1g_delay.rmii_tx_max;
			rx_max = p_eng->io.mac12_1g_delay.rmii_rx_max;
		}
	}
	*p_tx_d = tx_max;
	*p_rx_d = rx_max;
}

void mac_get_min_available_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d)
{
	uint32_t rgmii = (uint32_t)p_eng->run.is_rgmii;
	uint32_t mac_idx = p_eng->run.mac_idx;
	int32_t tx_min, rx_min;

	if (rgmii) {
		if (mac_idx > 1) {
			tx_min = p_eng->io.mac34_1g_delay.tx_min;
			rx_min = p_eng->io.mac34_1g_delay.rx_min;
		} else {
			tx_min = p_eng->io.mac12_1g_delay.tx_min;
			rx_min = p_eng->io.mac12_1g_delay.rx_min;
		}
	} else {
		if (mac_idx > 1) {
			tx_min = p_eng->io.mac34_1g_delay.rmii_tx_min;
			rx_min = p_eng->io.mac34_1g_delay.rmii_rx_min;
		} else {
			tx_min = p_eng->io.mac12_1g_delay.rmii_tx_min;
			rx_min = p_eng->io.mac12_1g_delay.rmii_rx_min;
		}
	}
	*p_tx_d = tx_min;
	*p_rx_d = rx_min;
}

static void set_mac_1g_delay_1(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_1g_t reg;

	reg.w = readl(addr);
#ifdef CONFIG_ASPEED_AST2600
	if (rx_d < 0) {
		reg.b.rx_clk_inv_1 = 1;
		rx_d = abs(rx_d);
	}
#endif
	reg.b.rx_delay_1 = rx_d;
	reg.b.tx_delay_1 = tx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void set_mac_1g_delay_2(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_1g_t reg;

	reg.w = readl(addr);
#ifdef CONFIG_ASPEED_AST2600
	if (rx_d < 0) {
		reg.b.rx_clk_inv_2 = 1;
		rx_d = abs(rx_d);
	}
#endif
	reg.b.rx_delay_2 = rx_d;
	reg.b.tx_delay_2 = tx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void set_mac_100_10_delay_1(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_100_10_t reg;

	reg.w = readl(addr);
#ifdef CONFIG_ASPEED_AST2600
	if (rx_d < 0) {
		reg.b.rx_clk_inv_1 = 1;
		rx_d = abs(rx_d);
	}
#endif
	reg.b.rx_delay_1 = rx_d;
	reg.b.tx_delay_1 = tx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void set_mac_100_10_delay_2(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_100_10_t reg;

	reg.w = readl(addr);
#ifdef CONFIG_ASPEED_AST2600
	if (rx_d < 0) {
		reg.b.rx_clk_inv_2 = 1;
		rx_d = abs(rx_d);
	}
#endif
	reg.b.rx_delay_2 = rx_d;
	reg.b.tx_delay_2 = tx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void set_mac_rmii_delay_1(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_1g_t reg;

	reg.w = readl(addr);
	reg.b.rmii_tx_data_at_falling_1 = tx_d;
	reg.b.rx_delay_1 = rx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}

static void set_mac_rmii_delay_2(uint32_t addr, int32_t rx_d, int32_t tx_d)
{
	mac_delay_1g_t reg;

	reg.w = readl(addr);
	reg.b.rmii_tx_data_at_falling_2 = tx_d;
	reg.b.rx_delay_2 = rx_d;
	writel(reg.w, addr);

	debug("%s:[%08x] %08x, rx_d=%d, tx_d=%d\n", __func__, addr, reg.w,
	       rx_d, tx_d);
}


static void set_mac1_1g_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_1g_delay_1(p_eng->io.mac12_1g_delay.addr, rx_d, tx_d);
}
static void set_mac1_100m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_1(p_eng->io.mac12_100m_delay.addr, rx_d, tx_d);
}
static void set_mac1_10m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_1(p_eng->io.mac12_10m_delay.addr, rx_d, tx_d);
}
static void set_mac2_1g_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_1g_delay_2(p_eng->io.mac12_1g_delay.addr, rx_d, tx_d);
}
static void set_mac2_100m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_2(p_eng->io.mac12_100m_delay.addr, rx_d, tx_d);
}
static void set_mac2_10m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_2(p_eng->io.mac12_10m_delay.addr, rx_d, tx_d);
}
static void set_mac3_1g_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_1g_delay_1(p_eng->io.mac34_1g_delay.addr, rx_d, tx_d);
}
static void set_mac3_100m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_1(p_eng->io.mac34_100m_delay.addr, rx_d, tx_d);
}
static void set_mac3_10m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_1(p_eng->io.mac34_10m_delay.addr, rx_d, tx_d);
}
static void set_mac4_1g_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_1g_delay_2(p_eng->io.mac34_1g_delay.addr, rx_d, tx_d);
}
static void set_mac4_100m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_2(p_eng->io.mac34_100m_delay.addr, rx_d, tx_d);
}
static void set_mac4_10m_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_100_10_delay_2(p_eng->io.mac34_10m_delay.addr, rx_d, tx_d);
}
static void set_mac1_rmii_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_rmii_delay_1(p_eng->io.mac12_1g_delay.addr, rx_d, tx_d);
}
static void set_mac2_rmii_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_rmii_delay_2(p_eng->io.mac12_1g_delay.addr, rx_d, tx_d);
}

static void set_mac3_rmii_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_rmii_delay_1(p_eng->io.mac34_1g_delay.addr, rx_d, tx_d);
}

static void set_mac4_rmii_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	set_mac_rmii_delay_2(p_eng->io.mac34_1g_delay.addr, rx_d, tx_d);
}

void set_dummy_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	printf("%s: %d, %d\n", __func__, rx_d, tx_d);
}

/**
 * @brief function pointer table for delay setting
 * 
 * set_delay_func_tbl[rmii/rgmii][mac_idx][speed_idx 1g/100m/10m]
*/
typedef void (*pfn_set_delay) (MAC_ENGINE *, int32_t, int32_t);
pfn_set_delay set_delay_func_tbl[2][4][3] = {
	{
		{set_mac1_rmii_delay, set_mac1_rmii_delay, set_mac1_rmii_delay},
		{set_mac2_rmii_delay, set_mac2_rmii_delay, set_mac2_rmii_delay},
#if defined(CONFIG_ASPEED_AST2600)
		{set_mac3_rmii_delay, set_mac3_rmii_delay, set_mac3_rmii_delay},
		{set_mac4_rmii_delay, set_mac4_rmii_delay, set_mac4_rmii_delay},
#else
		{set_dummy_delay, set_dummy_delay, set_dummy_delay},
		{set_dummy_delay, set_dummy_delay, set_dummy_delay},
#endif		
	},
	{
		{set_mac1_1g_delay, set_mac1_100m_delay, set_mac1_10m_delay},
		{set_mac2_1g_delay, set_mac2_100m_delay, set_mac2_10m_delay},
#if defined(CONFIG_ASPEED_AST2600)		
		{set_mac3_1g_delay, set_mac3_100m_delay, set_mac3_10m_delay},
		{set_mac4_1g_delay, set_mac4_100m_delay, set_mac4_10m_delay},
#else
		{set_dummy_delay, set_dummy_delay, set_dummy_delay},
		{set_dummy_delay, set_dummy_delay, set_dummy_delay},
#endif		
	}
};

void mac_set_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d)
{
	uint32_t rgmii = (uint32_t)p_eng->run.is_rgmii;
	uint32_t mac_idx = p_eng->run.mac_idx;
	u32 speed_idx = p_eng->run.speed_idx;

	set_delay_func_tbl[rgmii][mac_idx][speed_idx] (p_eng, rx_d, tx_d);
}

uint32_t mac_get_driving_strength(MAC_ENGINE *p_eng)
{
#ifdef CONFIG_ASPEED_AST2600
	mac34_drv_t reg;
	
	reg.w = readl(p_eng->io.mac34_drv_reg.addr);
	/* ast2600 : only MAC#3 & MAC#4 have driving strength setting */
	if (p_eng->run.mac_idx == 2) {
		return (reg.b.mac3_tx_drv);
	} else if (p_eng->run.mac_idx == 3) {
		return (reg.b.mac4_tx_drv);
	} else {
		return 0;
	}
#else
	mac12_drv_t reg;

	reg.w = readl(p_eng->io.mac12_drv_reg.addr);
	
	if (p_eng->run.mac_idx == 0) {
		return reg.b.mac1_rgmii_tx_drv;
	} else if (p_eng->run.mac_idx == 1) {
		return reg.b.mac2_rgmii_tx_drv;
	} else {
		return 0;
	}
#endif		
}
void mac_set_driving_strength(MAC_ENGINE *p_eng, uint32_t strength)
{
#ifdef CONFIG_ASPEED_AST2600
	mac34_drv_t reg;

	if (strength > p_eng->io.mac34_drv_reg.drv_max) {
		printf("invalid driving strength value\n");
		return;
	}

	/**
	 * read->modify->write for driving strength control register 
	 * ast2600 : only MAC#3 & MAC#4 have driving strength setting
	 */
	reg.w = readl(p_eng->io.mac34_drv_reg.addr);

	/* ast2600 : only MAC#3 & MAC#4 have driving strength setting */
	if (p_eng->run.mac_idx == 2) {
		reg.b.mac3_tx_drv = strength;
	} else if (p_eng->run.mac_idx == 3) {
		reg.b.mac4_tx_drv = strength;
	}

	writel(reg.w, p_eng->io.mac34_drv_reg.addr);
#else
	mac12_drv_t reg;

	if (strength > p_eng->io.mac12_drv_reg.drv_max) {
		printf("invalid driving strength value\n");
		return;
	}

	/* read->modify->write for driving strength control register */
	reg.w = readl(p_eng->io.mac12_drv_reg.addr);
	if (p_eng->run.is_rgmii) {
		if (p_eng->run.mac_idx == 0) {
			reg.b.mac1_rgmii_tx_drv =
			    strength;
		} else if (p_eng->run.mac_idx == 2) {
			reg.b.mac2_rgmii_tx_drv =
			    strength;
		}
	} else {
		if (p_eng->run.mac_idx == 0) {
			reg.b.mac1_rmii_tx_drv =
			    strength;
		} else if (p_eng->run.mac_idx == 1) {
			reg.b.mac2_rmii_tx_drv =
			    strength;
		}
	}
	writel(reg.w, p_eng->io.mac12_drv_reg.addr);
#endif
}

void mac_set_rmii_50m_output_enable(MAC_ENGINE *p_eng)
{
	uint32_t addr;
	mac_delay_1g_t value;

	if (p_eng->run.mac_idx > 1) {
		addr = p_eng->io.mac34_1g_delay.addr;
	} else {
		addr = p_eng->io.mac12_1g_delay.addr;
	}

	value.w = readl(addr);
	if (p_eng->run.mac_idx & BIT(0)) {
		value.b.rmii_50m_oe_2 = 1;
	} else {
		value.b.rmii_50m_oe_1 = 1;
	}
	writel(value.w, addr);
}

//------------------------------------------------------------
int mac_set_scan_boundary(MAC_ENGINE *p_eng)
{
	int32_t rx_cur, tx_cur;
	int32_t rx_min, rx_max, tx_min, tx_max;
	int32_t rx_scaling, tx_scaling;

	nt_log_func_name();

	/* get current delay setting */
	mac_get_delay(p_eng, &rx_cur, &tx_cur);
	
	/* get physical boundaries */
	mac_get_max_available_delay(p_eng, &rx_max, &tx_max);
	mac_get_min_available_delay(p_eng, &rx_min, &tx_min);

	if ((p_eng->run.is_rgmii) && (p_eng->arg.ctrl.b.inv_rgmii_rxclk)) {
		rx_max = (rx_max > 0) ? 0 : rx_max;
	} else {
		rx_min = (rx_min < 0) ? 0 : rx_min;
	}

	if (p_eng->run.TM_IOTiming) {
		if (p_eng->arg.ctrl.b.full_range) {
			tx_scaling = 0;
			rx_scaling = 0;
		} else {
			/* down-scaling to save test time */
			tx_scaling = TX_DELAY_SCALING;
			rx_scaling = RX_DELAY_SCALING;
		}
		p_eng->io.rx_delay_scan.step = 1;
		p_eng->io.tx_delay_scan.step = 1;
		p_eng->io.rx_delay_scan.begin = rx_min >> rx_scaling;
		p_eng->io.rx_delay_scan.end = rx_max >> rx_scaling;
		p_eng->io.tx_delay_scan.begin = tx_min >> tx_scaling;
		p_eng->io.tx_delay_scan.end = tx_max >> tx_scaling;
	} else if (p_eng->run.delay_margin) {
		p_eng->io.rx_delay_scan.step = 1;
		p_eng->io.tx_delay_scan.step = 1;
		p_eng->io.rx_delay_scan.begin = rx_cur - p_eng->run.delay_margin;
		p_eng->io.rx_delay_scan.end = rx_cur + p_eng->run.delay_margin;
		p_eng->io.tx_delay_scan.begin = tx_cur - p_eng->run.delay_margin;
		p_eng->io.tx_delay_scan.end = tx_cur + p_eng->run.delay_margin;
	} else {
		p_eng->io.rx_delay_scan.step = 1;
		p_eng->io.tx_delay_scan.step = 1;
		p_eng->io.rx_delay_scan.begin = 0;
		p_eng->io.rx_delay_scan.end = 0;
		p_eng->io.tx_delay_scan.begin = 0;
		p_eng->io.tx_delay_scan.end = 0;
	}

	/* backup current setting as the original for plotting result */
	p_eng->io.rx_delay_scan.orig = rx_cur;
	p_eng->io.tx_delay_scan.orig = tx_cur;

	/* check if setting is legal or not */
	if (p_eng->io.rx_delay_scan.begin < rx_min)
		p_eng->io.rx_delay_scan.begin = rx_min;

	if (p_eng->io.tx_delay_scan.begin < tx_min)
		p_eng->io.tx_delay_scan.begin = tx_min;

	if (p_eng->io.rx_delay_scan.end > rx_max)
		p_eng->io.rx_delay_scan.end = rx_max;

	if (p_eng->io.tx_delay_scan.end > tx_max)
		p_eng->io.tx_delay_scan.end = tx_max;

	if (p_eng->io.rx_delay_scan.begin > p_eng->io.rx_delay_scan.end)
		p_eng->io.rx_delay_scan.begin = p_eng->io.rx_delay_scan.end;

	if (p_eng->io.tx_delay_scan.begin > p_eng->io.tx_delay_scan.end)
		p_eng->io.tx_delay_scan.begin = p_eng->io.tx_delay_scan.end;

	if (p_eng->run.IO_MrgChk) {
		if ((p_eng->io.rx_delay_scan.orig <
		     p_eng->io.rx_delay_scan.begin) ||
		    (p_eng->io.rx_delay_scan.orig >
		     p_eng->io.rx_delay_scan.end)) {
			printf("Warning: current delay is not in the "
			       "scan-range\n");
			printf("RX delay scan range:%d ~ %d, curr:%d\n",
			       p_eng->io.rx_delay_scan.begin,
			       p_eng->io.rx_delay_scan.end,
			       p_eng->io.rx_delay_scan.orig);
			printf("TX delay scan range:%d ~ %d, curr:%d\n",
			       p_eng->io.tx_delay_scan.begin,
			       p_eng->io.tx_delay_scan.end,
			       p_eng->io.tx_delay_scan.orig);
		}
	}

	return (0);
}

//------------------------------------------------------------
// MAC
//------------------------------------------------------------
void mac_set_addr(MAC_ENGINE *p_eng)
{
	nt_log_func_name();	
	
	uint32_t madr = p_eng->reg.mac_madr;
	uint32_t ladr = p_eng->reg.mac_ladr;

	if (((madr == 0x0000) && (ladr == 0x00000000)) ||
	    ((madr == 0xffff) && (ladr == 0xffffffff))) {
		/* FIXME: shall use random gen */    
		madr = 0x0000000a;
		ladr = 0xf7837dd4;
	}

	p_eng->inf.SA[0] = (madr >> 8) & 0xff; // MSB
	p_eng->inf.SA[1] = (madr >> 0) & 0xff;
	p_eng->inf.SA[2] = (ladr >> 24) & 0xff;
	p_eng->inf.SA[3] = (ladr >> 16) & 0xff;
	p_eng->inf.SA[4] = (ladr >> 8) & 0xff;
	p_eng->inf.SA[5] = (ladr >> 0) & 0xff; // LSB	

	printf("mac address: ");
	for (int i = 0; i < 6; i++) {
		printf("%02x:", p_eng->inf.SA[i]);
	}
	printf("\n");
}

void mac_set_interal_loopback(MAC_ENGINE *p_eng)
{
	uint32_t reg = mac_reg_read(p_eng, 0x40);
	mac_reg_write(p_eng, 0x40, reg | BIT(30)); 
}

//------------------------------------------------------------
void init_mac (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	mac_cr_t maccr;	

#ifdef Enable_MAC_SWRst
	maccr.w = 0;
	maccr.b.sw_rst = 1;
	mac_reg_write(eng, 0x50, maccr.w);

	do {
		DELAY(Delay_MACRst);
		maccr.w = mac_reg_read(eng, 0x50);
	} while(maccr.b.sw_rst);
#endif

	mac_reg_write(eng, 0x20, eng->run.tdes_base - ASPEED_DRAM_BASE);
	mac_reg_write(eng, 0x24, eng->run.rdes_base - ASPEED_DRAM_BASE);

	mac_reg_write(eng, 0x08, eng->reg.mac_madr);
	mac_reg_write(eng, 0x0c, eng->reg.mac_ladr);

#ifdef MAC_030_def
	mac_reg_write( eng, 0x30, MAC_030_def );//Int Thr/Cnt
#endif
#ifdef MAC_034_def
	mac_reg_write( eng, 0x34, MAC_034_def );//Poll Cnt
#endif
#ifdef MAC_038_def
	mac_reg_write( eng, 0x38, MAC_038_def );
#endif
#ifdef MAC_048_def
	mac_reg_write( eng, 0x48, MAC_048_def );
#endif
#ifdef MAC_058_def
	mac_reg_write( eng, 0x58, MAC_058_def );
#endif

	if ( eng->arg.run_mode == MODE_NCSI )
		mac_reg_write( eng, 0x4c, NCSI_RxDMA_PakSize );
	else
		mac_reg_write( eng, 0x4c, DMA_PakSize );

	maccr.b.txdma_en = 1;
	maccr.b.rxdma_en = 1;
	maccr.b.txmac_en = 1;
	maccr.b.rxmac_en = 1;
	maccr.b.fulldup = 1;
	maccr.b.crc_apd = 1;

	if (eng->run.speed_sel[0]) {
		maccr.b.gmac_mode = 1;
	} else if (eng->run.speed_sel[1]) {
		maccr.b.speed_100 = 1;
	}

	if (eng->arg.run_mode == MODE_NCSI) {
		maccr.b.rx_broadpkt_en = 1;
		maccr.b.speed_100 = 1;
	}
	else {
		maccr.b.rx_alladr = 1;
#ifdef Enable_Runt
		maccr.b.rx_runt = 1;
#endif
	}
	mac_reg_write(eng, 0x50, maccr.w);
	DELAY(Delay_MACRst);
} // End void init_mac (MAC_ENGINE *eng)

//------------------------------------------------------------
// Basic
//------------------------------------------------------------
void FPri_RegValue (MAC_ENGINE *eng, uint8_t option) 
{
	nt_log_func_name();

	PRINTF( option, "[SRAM] Date:%08x\n", SRAM_RD( 0x88 ) );
	PRINTF( option, "[SRAM]  80:%08x %08x %08x %08x\n", SRAM_RD( 0x80 ), SRAM_RD( 0x84 ), SRAM_RD( 0x88 ), SRAM_RD( 0x8c ) );
	
	PRINTF( option, "[SCU]  a0:%08x  a4:%08x  b8:%08x  bc:%08x\n", SCU_RD( 0x0a0 ), SCU_RD( 0x0a4 ), SCU_RD( 0x0b8 ), SCU_RD( 0x0bc ));

	PRINTF( option, "[SCU] 13c:%08x 140:%08x 144:%08x 1dc:%08x\n", SCU_RD( 0x13c ), SCU_RD( 0x140 ), SCU_RD( 0x144 ), SCU_RD( 0x1dc ) );
	PRINTF( option, "[WDT]  0c:%08x  2c:%08x  4c:%08x\n", eng->reg.WDT_00c, eng->reg.WDT_02c, eng->reg.WDT_04c );
	PRINTF( option, "[MAC]  A0|%08x %08x %08x %08x\n", mac_reg_read( eng, 0xa0 ), mac_reg_read( eng, 0xa4 ), mac_reg_read( eng, 0xa8 ), mac_reg_read( eng, 0xac ) );
	PRINTF( option, "[MAC]  B0|%08x %08x %08x %08x\n", mac_reg_read( eng, 0xb0 ), mac_reg_read( eng, 0xb4 ), mac_reg_read( eng, 0xb8 ), mac_reg_read( eng, 0xbc ) );
	PRINTF( option, "[MAC]  C0|%08x %08x %08x\n",       mac_reg_read( eng, 0xc0 ), mac_reg_read( eng, 0xc4 ), mac_reg_read( eng, 0xc8 ) );

} // End void FPri_RegValue (MAC_ENGINE *eng, uint8_t *fp)

//------------------------------------------------------------
void FPri_End (MAC_ENGINE *eng, uint8_t option) 
{
	nt_log_func_name();
	if ((0 == eng->run.is_rgmii) && (eng->phy.RMIICK_IOMode != 0) &&
	    eng->run.IO_MrgChk && eng->flg.all_fail) {
		if ( eng->arg.ctrl.b.rmii_phy_in == 0 ) {
			PRINTF( option, "\n\n\n\n\n\n[Info] The PHY's RMII reference clock pin is setting to the OUTPUT mode now.\n" );
			PRINTF( option, "       Maybe you can run the INPUT mode command \"mactest  %d %d %d %d %d %d %d\".\n\n\n\n", eng->arg.mac_idx, eng->arg.run_speed, (eng->arg.ctrl.w | 0x80), eng->arg.loop_max, eng->arg.test_mode, eng->arg.phy_addr, eng->arg.delay_scan_range );
		}
		else {
			PRINTF( option, "\n\n\n\n\n\n[Info] The PHY's RMII reference clock pin is setting to the INPUT mode now.\n" );
			PRINTF( option, "       Maybe you can run the OUTPUT mode command \"mactest  %d %d %d %d %d %d %d\".\n\n\n\n", eng->arg.mac_idx, eng->arg.run_speed, (eng->arg.ctrl.w & 0x7f), eng->arg.loop_max, eng->arg.test_mode, eng->arg.phy_addr, eng->arg.delay_scan_range );
		}
	}

	if (!eng->run.TM_RxDataEn) {
	} else if (eng->flg.error) {
		PRINTF(option, "                    \n----> fail !!!\n");
	}

	//------------------------------
	//[Warning] PHY Address
	//------------------------------
	if ( eng->arg.run_mode == MODE_DEDICATED ) {
		if ( eng->arg.phy_addr != eng->phy.Adr )
			PRINTF( option, "\n[Warning] PHY Address change from %d to %d !!!\n", eng->arg.phy_addr, eng->phy.Adr );
	}

	/* [Warning] IO Strength */
	if (eng->io.init_done) {
#ifdef CONFIG_ASPEED_AST2600
		if ((eng->io.mac34_drv_reg.value.b.mac3_tx_drv != 0x3) ||
		    (eng->io.mac34_drv_reg.value.b.mac4_tx_drv != 0x3)) {
			PRINTF(option,
			       "\n[Warning] [%08x] bit[3:0] 0x%02x is not the recommended value "
			       "0xf.\n",
			       eng->io.mac34_drv_reg.addr,
			       eng->io.mac34_drv_reg.value.w & 0xf);
		}
#else
		if (eng->io.mac12_drv_reg.value.w & GENMASK(11, 8)) {
			PRINTF(option,
			       "\n[Warning] [%08X] 0x%08x is not the recommended value "
			       "0.\n",
			       eng->io.mac12_drv_reg.addr,
			       eng->io.mac12_drv_reg.value.w);
		}
#endif
	}

	//------------------------------
	//[Warning] IO Timing
	//------------------------------
	if ( eng->arg.run_mode == MODE_NCSI ) {
		PRINTF( option, "\n[Arg] %d %d %d %d %d %d %d {%d}\n", eng->arg.mac_idx, eng->arg.GPackageTolNum, eng->arg.GChannelTolNum, eng->arg.test_mode, eng->arg.delay_scan_range, eng->arg.ctrl.w, eng->arg.GARPNumCnt, TIME_OUT_NCSI );

		switch ( eng->ncsi_cap.PCI_DID_VID ) {
			case PCI_DID_VID_Intel_82574L             : { PRINTF( option, "[NC]%08x %08x: Intel 82574L       \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10d6         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10a7         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10a9         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10c9         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e6         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e7         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e8         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_1518         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_1526         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_150a         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_150d         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82599_10fb         : { PRINTF( option, "[NC]%08x %08x: Intel 82599        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82599_1557         : { PRINTF( option, "[NC]%08x %08x: Intel 82599        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I210_1533          : { PRINTF( option, "[NC]%08x %08x: Intel I210         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I210_1537          : { PRINTF( option, "[NC]%08x %08x: Intel I210         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I350_1521          : { PRINTF( option, "[NC]%08x %08x: Intel I350         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I350_1523          : { PRINTF( option, "[NC]%08x %08x: Intel I350         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X540               : { PRINTF( option, "[NC]%08x %08x: Intel X540         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X550               : { PRINTF( option, "[NC]%08x %08x: Intel X550         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_Broadwell_DE       : { PRINTF( option, "[NC]%08x %08x: Intel Broadwell-DE \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X722_37d0          : { PRINTF( option, "[NC]%08x %08x: Intel X722         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5718         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5718   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5719         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5719   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5720         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5720   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5725         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5725   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM57810S       : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM57810S \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_Cumulus         : { PRINTF( option, "[NC]%08x %08x: Broadcom Cumulus   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM57302        : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM57302  \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM957452       : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM957452 \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_3_1003 : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-3\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_3_1007 : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-3\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_4      : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-4\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			default:
			switch ( eng->ncsi_cap.manufacturer_id ) {
				case ManufacturerID_Intel    : { PRINTF( option, "[NC]%08x %08x: Intel              \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Broadcom : { PRINTF( option, "[NC]%08x %08x: Broadcom           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Mellanox : { PRINTF( option, "[NC]%08x %08x: Mellanox           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Mellanox1: { PRINTF( option, "[NC]%08x %08x: Mellanox           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Emulex   : { PRINTF( option, "[NC]%08x %08x: Emulex             \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				default                      : { PRINTF( option, "[NC]%08x %08x                     \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			} // End switch ( eng->ncsi_cap.manufacturer_id )
		} // End switch ( eng->ncsi_cap.PCI_DID_VID )
	}
	else {
		PRINTF(option, "[PHY] @addr %d: id = %04x_%04x (%s)\n",
		       eng->phy.Adr, eng->phy.id1, eng->phy.id2,
		       eng->phy.phy_name);
	} // End if ( eng->arg.run_mode == MODE_NCSI )	
} // End void FPri_End (MAC_ENGINE *eng, uint8_t option)

//------------------------------------------------------------
void FPri_ErrFlag (MAC_ENGINE *eng, uint8_t option) 
{
	nt_log_func_name();
	if ( eng->flg.print_en ) {
		if ( eng->flg.warn ) {
			if ( eng->flg.warn & Wrn_Flag_IOMarginOUF ) {
				PRINTF(option, "[Warning] IO timing testing "
					       "range out of boundary\n");

				if (0 == eng->run.is_rgmii) {
					PRINTF( option, "      (reg:%d,%d) %dx1(%d~%d,%d)\n", eng->io.Dly_in_reg_idx,
											      eng->io.Dly_out_reg_idx,
											      eng->run.delay_margin,
											      eng->io.Dly_in_min,
											      eng->io.Dly_in_max,
											      eng->io.Dly_out_min );
				}
				else {
					PRINTF( option, "      (reg:%d,%d) %dx%d(%d~%d,%d~%d)\n", eng->io.Dly_in_reg_idx,
												  eng->io.Dly_out_reg_idx,
												  eng->run.delay_margin,
												  eng->run.delay_margin,
												  eng->io.Dly_in_min,
												  eng->io.Dly_in_max,
												  eng->io.Dly_out_min,
												  eng->io.Dly_out_max );
				}
			} // End if ( eng->flg.warn & Wrn_Flag_IOMarginOUF )
			if ( eng->flg.warn & Wrn_Flag_RxErFloatting ) {
				PRINTF( option, "[Warning] NCSI RXER pin may be floatting to the MAC !!!\n" );
				PRINTF( option, "          Please contact with the ASPEED Inc. for more help.\n" );
			} // End if ( eng->flg.warn & Wrn_Flag_RxErFloatting )
		} // End if ( eng->flg.warn )

		if ( eng->flg.error ) {
			PRINTF( option, "\n\n" );
//PRINTF( option, "error: %x\n\n", eng->flg.error );

			if ( eng->flg.error & Err_Flag_PHY_Type                ) { PRINTF( option, "[Err] Unidentifiable PHY                                     \n" ); }
			if ( eng->flg.error & Err_Flag_MALLOC_FrmSize          ) { PRINTF( option, "[Err] Malloc fail at frame size buffer                       \n" ); }
			if ( eng->flg.error & Err_Flag_MALLOC_LastWP           ) { PRINTF( option, "[Err] Malloc fail at last WP buffer                          \n" ); }
			if ( eng->flg.error & Err_Flag_Check_Buf_Data          ) { PRINTF( option, "[Err] Received data mismatch                                 \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_Check_TxOwnTimeOut ) { PRINTF( option, "[Err] Time out of checking Tx owner bit in NCSI packet       \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_Check_RxOwnTimeOut ) { PRINTF( option, "[Err] Time out of checking Rx owner bit in NCSI packet       \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_Check_ARPOwnTimeOut) { PRINTF( option, "[Err] Time out of checking ARP owner bit in NCSI packet      \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_No_PHY             ) { PRINTF( option, "[Err] Can not find NCSI PHY                                  \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_Channel_Num        ) { PRINTF( option, "[Err] NCSI Channel Number Mismatch                           \n" ); }
			if ( eng->flg.error & Err_Flag_NCSI_Package_Num        ) { PRINTF( option, "[Err] NCSI Package Number Mismatch                           \n" ); }
			if ( eng->flg.error & Err_Flag_PHY_TimeOut_RW          ) { PRINTF( option, "[Err] Time out of read/write PHY register                    \n" ); }
			if ( eng->flg.error & Err_Flag_PHY_TimeOut_Rst         ) { PRINTF( option, "[Err] Time out of reset PHY register                         \n" ); }
			if ( eng->flg.error & Err_Flag_RXBUF_UNAVA             ) { PRINTF( option, "[Err] MAC00h[2]:Receiving buffer unavailable                 \n" ); }
			if ( eng->flg.error & Err_Flag_RPKT_LOST               ) { PRINTF( option, "[Err] MAC00h[3]:Received packet lost due to RX FIFO full     \n" ); }
			if ( eng->flg.error & Err_Flag_NPTXBUF_UNAVA           ) { PRINTF( option, "[Err] MAC00h[6]:Normal priority transmit buffer unavailable  \n" ); }
			if ( eng->flg.error & Err_Flag_TPKT_LOST               ) { PRINTF( option, "[Err] MAC00h[7]:Packets transmitted to Ethernet lost         \n" ); }
			if ( eng->flg.error & Err_Flag_DMABufNum               ) { PRINTF( option, "[Err] DMA Buffer is not enough                               \n" ); }
			if ( eng->flg.error & Err_Flag_IOMargin                ) { PRINTF( option, "[Err] IO timing margin is not enough                         \n" ); }

			if ( eng->flg.error & Err_Flag_MHCLK_Ratio             ) {
				PRINTF( option, "[Err] Error setting of MAC AHB bus clock (SCU08[18:16])      \n" );
				if ( eng->env.at_least_1g_valid )
					{ PRINTF( option, "      SCU08[18:16] == 0x%01x is not the suggestion value 2.\n", eng->env.MHCLK_Ratio ); }
				else
					{ PRINTF( option, "      SCU08[18:16] == 0x%01x is not the suggestion value 4.\n", eng->env.MHCLK_Ratio ); }
			} // End if ( eng->flg.error & Err_Flag_MHCLK_Ratio             )

			if ( eng->flg.error & Err_Flag_IOMarginOUF ) {
				PRINTF( option, "[Err] IO timing testing range out of boundary\n");
				if (0 == eng->run.is_rgmii) {
					PRINTF( option, "      (reg:%d,%d) %dx1(%d~%d,%d)\n", eng->io.Dly_in_reg_idx,
											      eng->io.Dly_out_reg_idx,
											      eng->run.delay_margin,
											      eng->io.Dly_in_min,
											      eng->io.Dly_in_max,
											      eng->io.Dly_out_min );
				}
				else {
					PRINTF( option, "      (reg:%d,%d) %dx%d(%d~%d,%d~%d)\n", eng->io.Dly_in_reg_idx,
												  eng->io.Dly_out_reg_idx,
												  eng->run.delay_margin,
												  eng->run.delay_margin,
												  eng->io.Dly_in_min,
												  eng->io.Dly_in_max,
												  eng->io.Dly_out_min,
												  eng->io.Dly_out_max );
				}
			} // End if ( eng->flg.error & Err_Flag_IOMarginOUF )

			if ( eng->flg.error & Err_Flag_Check_Des ) {
				PRINTF( option, "[Err] Descriptor error\n");
				if ( eng->flg.desc & Des_Flag_TxOwnTimeOut ) { PRINTF( option, "[Des] Time out of checking Tx owner bit\n" ); }
				if ( eng->flg.desc & Des_Flag_RxOwnTimeOut ) { PRINTF( option, "[Des] Time out of checking Rx owner bit\n" ); }
				if ( eng->flg.desc & Des_Flag_FrameLen     ) { PRINTF( option, "[Des] Frame length mismatch            \n" ); }
				if ( eng->flg.desc & Des_Flag_RxErr        ) { PRINTF( option, "[Des] Input signal RxErr               \n" ); }
				if ( eng->flg.desc & Des_Flag_CRC          ) { PRINTF( option, "[Des] CRC error of frame               \n" ); }
				if ( eng->flg.desc & Des_Flag_FTL          ) { PRINTF( option, "[Des] Frame too long                   \n" ); }
				if ( eng->flg.desc & Des_Flag_Runt         ) { PRINTF( option, "[Des] Runt packet                      \n" ); }
				if ( eng->flg.desc & Des_Flag_OddNibble    ) { PRINTF( option, "[Des] Nibble bit happen                \n" ); }
				if ( eng->flg.desc & Des_Flag_RxFIFOFull   ) { PRINTF( option, "[Des] Rx FIFO full                     \n" ); }
			} // End if ( eng->flg.error & Err_Flag_Check_Des )

			if ( eng->flg.error & Err_Flag_MACMode ) {
				PRINTF( option, "[Err] MAC interface mode mismatch\n" );
				for (int i = 0; i < 4; i++) {
					if (eng->env.is_1g_valid[i]) {
						PRINTF(option,
						       "[MAC%d] is RGMII\n", i);
					} else {
						PRINTF(option,
						       "[MAC%d] RMII\n", i);
					}
				}
			} // End if ( eng->flg.error & Err_Flag_MACMode )

			if ( eng->arg.run_mode == MODE_NCSI ) {
				if ( eng->flg.error & ERR_FLAG_NCSI_LINKFAIL ) {
					PRINTF( option, "[Err] NCSI packet retry number over flows when find channel\n" );

					if ( eng->flg.ncsi & NCSI_Flag_Get_Version_ID                  ) { PRINTF( option, "[NCSI] Time out when Get Version ID                  \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Get_Capabilities                ) { PRINTF( option, "[NCSI] Time out when Get Capabilities                \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Select_Active_Package           ) { PRINTF( option, "[NCSI] Time out when Select Active Package           \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Enable_Set_MAC_Address          ) { PRINTF( option, "[NCSI] Time out when Enable Set MAC Address          \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Enable_Broadcast_Filter         ) { PRINTF( option, "[NCSI] Time out when Enable Broadcast Filter         \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Enable_Network_TX               ) { PRINTF( option, "[NCSI] Time out when Enable Network TX               \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Enable_Channel                  ) { PRINTF( option, "[NCSI] Time out when Enable Channel                  \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Disable_Network_TX              ) { PRINTF( option, "[NCSI] Time out when Disable Network TX              \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Disable_Channel                 ) { PRINTF( option, "[NCSI] Time out when Disable Channel                 \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Select_Package                  ) { PRINTF( option, "[NCSI] Time out when Select Package                  \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Deselect_Package                ) { PRINTF( option, "[NCSI] Time out when Deselect Package                \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Set_Link                        ) { PRINTF( option, "[NCSI] Time out when Set Link                        \n" ); }
					if ( eng->flg.ncsi & NCSI_Flag_Get_Controller_Packet_Statistics) { PRINTF( option, "[NCSI] Time out when Get Controller Packet Statistics\n" ); }
				}

				if ( eng->flg.error & Err_Flag_NCSI_Channel_Num ) { PRINTF( option, "[NCSI] Channel number expected: %d, real: %d\n", eng->arg.GChannelTolNum, eng->dat.number_chl ); }
				if ( eng->flg.error & Err_Flag_NCSI_Package_Num ) { PRINTF( option, "[NCSI] Peckage number expected: %d, real: %d\n", eng->arg.GPackageTolNum, eng->dat.number_pak ); }
			} // End if ( eng->arg.run_mode == MODE_NCSI )
		} // End if ( eng->flg.error )
	} // End if ( eng->flg.print_en )
} // End void FPri_ErrFlag (MAC_ENGINE *eng, uint8_t option)

//------------------------------------------------------------

//------------------------------------------------------------
int FindErr (MAC_ENGINE *p_eng, int value) 
{
	p_eng->flg.error = p_eng->flg.error | value;

	if (DBG_PRINT_ERR_FLAG)
		printf("flags: error = %08x\n", p_eng->flg.error);

	return (1);
}

//------------------------------------------------------------
int FindErr_Des (MAC_ENGINE *p_eng, int value) 
{
	p_eng->flg.error = p_eng->flg.error | Err_Flag_Check_Des;
	p_eng->flg.desc = p_eng->flg.desc | value;
	if (DBG_PRINT_ERR_FLAG)
		printf("flags: error = %08x, desc = %08x\n", p_eng->flg.error, p_eng->flg.desc);

	return (1);
}

//------------------------------------------------------------
// Get and Check status of Interrupt
//------------------------------------------------------------
int check_int (MAC_ENGINE *eng, char *type ) 
{
	nt_log_func_name();

	uint32_t mac_00;

	mac_00 = mac_reg_read(eng, 0x00);
#ifdef CheckRxbufUNAVA
	if (mac_00 & BIT(2)) {
		PRINTF( FP_LOG, "[%sIntStatus] Receiving buffer unavailable               : %08x [loop[%d]:%d]\n", type, mac_00, eng->run.loop_of_cnt, eng->run.loop_cnt );
		FindErr( eng, Err_Flag_RXBUF_UNAVA );
	}
#endif

#ifdef CheckRPktLost
	if (mac_00 & BIT(3)) {
		PRINTF( FP_LOG, "[%sIntStatus] Received packet lost due to RX FIFO full   : %08x [loop[%d]:%d]\n", type, mac_00, eng->run.loop_of_cnt, eng->run.loop_cnt );
		FindErr( eng, Err_Flag_RPKT_LOST );
	}
#endif

#ifdef CheckNPTxbufUNAVA
	if (mac_00 & BIT(6) ) {
		PRINTF( FP_LOG, "[%sIntStatus] Normal priority transmit buffer unavailable: %08x [loop[%d]:%d]\n", type, mac_00, eng->run.loop_of_cnt, eng->run.loop_cnt );
		FindErr( eng, Err_Flag_NPTXBUF_UNAVA );
	}
#endif

#ifdef CheckTPktLost
	if (mac_00 & BIT(7)) {
		PRINTF( FP_LOG, "[%sIntStatus] Packets transmitted to Ethernet lost       : %08x [loop[%d]:%d]\n", type, mac_00, eng->run.loop_of_cnt, eng->run.loop_cnt );
		FindErr( eng, Err_Flag_TPKT_LOST );
	}
#endif

	if ( eng->flg.error )
		return(1);
	else
		return(0);
} // End int check_int (MAC_ENGINE *eng, char *type)


//------------------------------------------------------------
// Buffer
//------------------------------------------------------------
void setup_framesize (MAC_ENGINE *eng) 
{
	int32_t       des_num;

	nt_log_func_name();

	//------------------------------
	// Fill Frame Size out descriptor area
	//------------------------------
	if (0) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			if ( RAND_SIZE_SIMPLE )
				switch( rand() % 5 ) {
					case 0 : eng->dat.FRAME_LEN[ des_num ] = 0x4e ; break;
					case 1 : eng->dat.FRAME_LEN[ des_num ] = 0x4ba; break;
					default: eng->dat.FRAME_LEN[ des_num ] = 0x5ea; break;
				}
			else
//				eng->dat.FRAME_LEN[ des_num ] = ( rand() + RAND_SIZE_MIN ) % ( RAND_SIZE_MAX + 1 );
				eng->dat.FRAME_LEN[ des_num ] = RAND_SIZE_MIN + ( rand() % ( RAND_SIZE_MAX - RAND_SIZE_MIN + 1 ) );

			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_framesize] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.loop_of_cnt, eng->run.loop_cnt );
		}
	}
	else {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
#ifdef SelectSimpleLength
			if ( des_num % FRAME_SELH_PERD )
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
			else
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
#else
			if ( eng->run.tm_tx_only ) {
				if ( eng->run.TM_IEEE )
					eng->dat.FRAME_LEN[ des_num ] = 1514;
				else  
					eng->dat.FRAME_LEN[ des_num ] = 60;  
			}
			else {
  #ifdef SelectLengthInc
				eng->dat.FRAME_LEN[ des_num ] = 1514 - ( des_num % 1455 );
  #else
				if ( des_num % FRAME_SELH_PERD )
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
				else
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
  #endif
			} // End if ( eng->run.tm_tx_only )
#endif
			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_framesize] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.loop_of_cnt, eng->run.loop_cnt );

		} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
	} // End if ( ENABLE_RAND_SIZE )

	// Calculate average of frame size
#ifdef Enable_ShowBW
	eng->dat.Total_frame_len = 0;

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		eng->dat.Total_frame_len += eng->dat.FRAME_LEN[ des_num ];
#endif

	//------------------------------
	// Write Plane
	//------------------------------
	switch( ZeroCopy_OFFSET & 0x3 ) {
		case 0: eng->dat.wp_fir = 0xffffffff; break;
		case 1: eng->dat.wp_fir = 0xffffff00; break;
		case 2: eng->dat.wp_fir = 0xffff0000; break;
		case 3: eng->dat.wp_fir = 0xff000000; break;
	}

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		switch( ( ZeroCopy_OFFSET + eng->dat.FRAME_LEN[ des_num ] - 1 ) & 0x3 ) {
			case 0: eng->dat.wp_lst[ des_num ] = 0x000000ff; break;
			case 1: eng->dat.wp_lst[ des_num ] = 0x0000ffff; break;
			case 2: eng->dat.wp_lst[ des_num ] = 0x00ffffff; break;
			case 3: eng->dat.wp_lst[ des_num ] = 0xffffffff; break;
		}
} // End void setup_framesize (void)

//------------------------------------------------------------
void setup_arp(MAC_ENGINE *eng)
{

	nt_log_func_name();

	memcpy(eng->dat.ARP_data, ARP_org_data, sizeof(ARP_org_data));

	eng->dat.ARP_data[1] &= ~GENMASK(31, 16);
	eng->dat.ARP_data[1] |= (eng->inf.SA[1] << 24) | (eng->inf.SA[0] << 16);
	eng->dat.ARP_data[2] = (eng->inf.SA[5] << 24) | (eng->inf.SA[4] << 16) |
			       (eng->inf.SA[3] << 8) | (eng->inf.SA[2]);
	eng->dat.ARP_data[5] &= ~GENMASK(31, 16);
	eng->dat.ARP_data[5] |= (eng->inf.SA[1] << 24) | (eng->inf.SA[0] << 16);
	eng->dat.ARP_data[6] = (eng->inf.SA[5] << 24) | (eng->inf.SA[4] << 16) |
			       (eng->inf.SA[3] << 8) | (eng->inf.SA[2]);
}

//------------------------------------------------------------
void setup_buf (MAC_ENGINE *eng) 
{
	int32_t des_num_max;
	int32_t des_num;
	int i;
	uint32_t adr;
	uint32_t adr_srt;
	uint32_t adr_end;
	uint32_t Current_framelen;
	uint32_t gdata = 0;
#ifdef SelectSimpleDA	
	int cnt;
	uint32_t len;
#endif	

	nt_log_func_name();

	// It need be multiple of 4
	eng->dat.DMA_Base_Setup = DMA_BASE & 0xfffffffc;
	adr_srt = eng->dat.DMA_Base_Setup;

	if (eng->run.tm_tx_only) {
		if (eng->run.TM_IEEE) {
			for (des_num = 0; des_num < eng->dat.Des_Num; des_num++) {
				if ( DbgPrn_BufAdr )
					printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.loop_of_cnt, eng->run.loop_cnt, des_num, adr_srt);
				Write_Mem_Dat_DD(adr_srt, 0xffffffff);
				Write_Mem_Dat_DD(adr_srt + 4,
						 eng->dat.ARP_data[1]);
				Write_Mem_Dat_DD(adr_srt + 8,
						 eng->dat.ARP_data[2]);

				for (adr = (adr_srt + 12);
				     adr < (adr_srt + DMA_PakSize); adr += 4) {
					switch (eng->arg.test_mode) {
					case 4:
						gdata = rand() | (rand() << 16);
						break;
					case 5:
						gdata = eng->arg.user_def_val;
						break;
					}
					Write_Mem_Dat_DD(adr, gdata);
				}
				adr_srt += DMA_PakSize;
			}
		} else {
			printf("----->[ARP] 60 bytes\n");
			for (i = 0; i < 16; i++)
				printf("      [Tx%02d] %08x %08x\n", i, eng->dat.ARP_data[i], SWAP_4B( eng->dat.ARP_data[i] ) );

			for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
				if ( DbgPrn_BufAdr )
					printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.loop_of_cnt, eng->run.loop_cnt, des_num, adr_srt);

				for (i = 0; i < 16; i++)
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ), eng->dat.ARP_data[i] );


				adr_srt += DMA_PakSize;
			} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
		} // End if ( eng->run.TM_IEEE )
	} else {
		if ( eng->arg.ctrl.b.single_packet )
			des_num_max = 1;
		else
			des_num_max = eng->dat.Des_Num;

		for (des_num = 0; des_num < des_num_max; des_num++) {
			if (DbgPrn_BufAdr)
				printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.loop_of_cnt, eng->run.loop_cnt, des_num, adr_srt);
  #ifdef SelectSimpleData
    #ifdef SimpleData_Fix
			switch( des_num % SimpleData_FixNum ) {
				case  0 : gdata = SimpleData_FixVal00; break;
				case  1 : gdata = SimpleData_FixVal01; break;
				case  2 : gdata = SimpleData_FixVal02; break;
				case  3 : gdata = SimpleData_FixVal03; break;
				case  4 : gdata = SimpleData_FixVal04; break;
				case  5 : gdata = SimpleData_FixVal05; break;
				case  6 : gdata = SimpleData_FixVal06; break;
				case  7 : gdata = SimpleData_FixVal07; break;
				case  8 : gdata = SimpleData_FixVal08; break;
				case  9 : gdata = SimpleData_FixVal09; break;
				case 10 : gdata = SimpleData_FixVal10; break;
				default : gdata = SimpleData_FixVal11; break;
			}
    #else
			gdata   = 0x11111111 * ((des_num + SEED_START) % 256);
    #endif
  #else
			gdata   = DATA_SEED( des_num + SEED_START );
  #endif
			Current_framelen = eng->dat.FRAME_LEN[ des_num ];

			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_buf      ] Current_framelen:%08x[Des:%d][loop[%d]:%d]\n", Current_framelen, des_num, eng->run.loop_of_cnt, eng->run.loop_cnt );
#ifdef SelectSimpleDA
			cnt     = 0;
			len     = ( ( ( Current_framelen - 14 ) & 0xff ) << 8) |
			            ( ( Current_framelen - 14 ) >> 8 );
#endif				    
			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {
  #ifdef SelectSimpleDA
				cnt++;
				if      ( cnt == 1 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat0 );
				else if ( cnt == 2 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat1 );
				else if ( cnt == 3 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat2 );
				else if ( cnt == 4 ) Write_Mem_Dat_DD( adr, len | (len << 16)   );
				else
  #endif
				                     Write_Mem_Dat_DD( adr, gdata );
  #ifdef SelectSimpleData
				gdata = gdata ^ SimpleData_XORVal;
  #else
				gdata += DATA_IncVal;
  #endif
			}
			adr_srt += DMA_PakSize;
		} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
	} // End if ( eng->run.tm_tx_only )
} // End void setup_buf (MAC_ENGINE *eng)

//------------------------------------------------------------
// Check data of one packet
//------------------------------------------------------------
char check_Data (MAC_ENGINE *eng, uint32_t datbase, int32_t number) 
{
	int32_t       number_dat;
	int        index;
	uint32_t      rdata;
	uint32_t      wp_lst_cur;
	uint32_t      adr_las;
	uint32_t      adr;
	uint32_t      adr_srt;
	uint32_t      adr_end;
#ifdef SelectSimpleDA
	int        cnt;
	uint32_t      len;
	uint32_t      gdata_bak;
#endif
	uint32_t      gdata;

	uint32_t      wp;

	nt_log_func_name();

	if (eng->arg.ctrl.b.single_packet)
		number_dat = 0;
	else
		number_dat = number;

	wp_lst_cur             = eng->dat.wp_lst[ number ];
	eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ number_dat ];

	if ( DbgPrn_FRAME_LEN )
		PRINTF( FP_LOG, "[check_Data     ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN_Cur, number, eng->run.loop_of_cnt, eng->run.loop_cnt );

	adr_srt = datbase;
	adr_end = adr_srt + PktByteSize;

#if defined(SelectSimpleData)
    #ifdef SimpleData_Fix
	switch( number_dat % SimpleData_FixNum ) {
		case  0 : gdata = SimpleData_FixVal00; break;
		case  1 : gdata = SimpleData_FixVal01; break;
		case  2 : gdata = SimpleData_FixVal02; break;
		case  3 : gdata = SimpleData_FixVal03; break;
		case  4 : gdata = SimpleData_FixVal04; break;
		case  5 : gdata = SimpleData_FixVal05; break;
		case  6 : gdata = SimpleData_FixVal06; break;
		case  7 : gdata = SimpleData_FixVal07; break;
		case  8 : gdata = SimpleData_FixVal08; break;
		case  9 : gdata = SimpleData_FixVal09; break;
		case 10 : gdata = SimpleData_FixVal10; break;
		default : gdata = SimpleData_FixVal11; break;
	}
    #else
	gdata   = 0x11111111 * (( number_dat + SEED_START ) % 256 );
    #endif
#else
	gdata   = DATA_SEED( number_dat + SEED_START );
#endif

//printf("check_buf: %08x - %08x [%08x]\n", adr_srt, adr_end, datbase);
	wp      = eng->dat.wp_fir;
	adr_las = adr_end - 4;
#ifdef SelectSimpleDA
	cnt     = 0;
	len     = ((( eng->dat.FRAME_LEN_Cur-14 ) & 0xff ) << 8 ) |
	          ( ( eng->dat.FRAME_LEN_Cur-14 )          >> 8 );
#endif

	if ( DbgPrn_Bufdat )
		PRINTF( FP_LOG, " Inf:%08x ~ %08x(%08x) %08x [Des:%d][loop[%d]:%d]\n", adr_srt, adr_end, adr_las, gdata, number, eng->run.loop_of_cnt, eng->run.loop_cnt );

	for ( adr = adr_srt; adr < adr_end; adr+=4 ) {
#ifdef SelectSimpleDA
		cnt++;
		if      ( cnt == 1 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat0; }
		else if ( cnt == 2 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat1; }
		else if ( cnt == 3 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat2; }
		else if ( cnt == 4 ) { gdata_bak = gdata; gdata = len | (len << 16);   }
#endif
		rdata = Read_Mem_Dat_DD( adr );
		if ( adr == adr_las )
			wp = wp & wp_lst_cur;

		if ( ( rdata & wp ) != ( gdata & wp ) ) {
			PRINTF( FP_LOG, "\nError: Adr:%08x[%3d] (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.loop_of_cnt, eng->run.loop_cnt );
			for ( index = 0; index < 6; index++ )
				PRINTF( FP_LOG, "Rep  : Adr:%08x      (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, Read_Mem_Dat_DD( adr ), gdata, wp, number, eng->run.loop_of_cnt, eng->run.loop_cnt );

			if (DbgPrn_DumpMACCnt)
				dump_mac_ROreg(eng);

			return( FindErr( eng, Err_Flag_Check_Buf_Data ) );
		} // End if ( (rdata & wp) != (gdata & wp) )
		if ( DbgPrn_BufdatDetail )
			PRINTF( FP_LOG, " Adr:%08x[%3d] (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.loop_of_cnt, eng->run.loop_cnt );

#ifdef SelectSimpleDA
		if ( cnt <= 4 )
			gdata = gdata_bak;
#endif

#if defined(SelectSimpleData)
		gdata = gdata ^ SimpleData_XORVal;
#else
		gdata += DATA_IncVal;
#endif

		wp     = 0xffffffff;
	}
	return(0);
} // End char check_Data (MAC_ENGINE *eng, uint32_t datbase, int32_t number)

//------------------------------------------------------------
char check_buf (MAC_ENGINE *eng, int loopcnt) 
{
	int32_t des_num;
	uint32_t desadr;
#ifdef CHECK_RX_DATA
	uint32_t datbase;
#endif
	nt_log_func_name();

	desadr = eng->run.rdes_base + (16 * eng->dat.Des_Num) - 4;
	for (des_num = eng->dat.Des_Num - 1; des_num >= 0; des_num--) {
#ifdef CHECK_RX_DATA
		datbase = AT_BUF_MEMRW(Read_Mem_Des_DD(desadr) & 0xfffffffc);
		if (check_Data(eng, datbase, des_num)) {
			check_int(eng, "");
			return (1);
		}
		if (check_int(eng, ""))
			return 1;
#endif
		desadr -= 16;
	}
	if (check_int(eng, ""))
		return (1);

#if defined(Delay_CheckData_LoopNum) && defined(Delay_CheckData)
	if ((loopcnt % Delay_CheckData_LoopNum) == 0)
		DELAY(Delay_CheckData);
#endif
	return (0);
} // End char check_buf (MAC_ENGINE *eng, int loopcnt)

//------------------------------------------------------------
// Descriptor
//------------------------------------------------------------
void setup_txdes (MAC_ENGINE *eng, uint32_t desadr, uint32_t bufbase) 
{
	uint32_t bufadr;
	uint32_t bufadrgap;
	uint32_t desval = 0;
	int32_t des_num;

	nt_log_func_name();

	bufadr = bufbase;
	if (eng->arg.ctrl.b.single_packet)
		bufadrgap = 0;
	else
		bufadrgap = DMA_PakSize;

	if (eng->run.TM_TxDataEn) {
		for (des_num = 0; des_num < eng->dat.Des_Num; des_num++) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[des_num];
			desval = TDES_IniVal;
			Write_Mem_Des_DD(desadr + 0x04, 0);
			Write_Mem_Des_DD(desadr + 0x08, 0);
			Write_Mem_Des_DD(desadr + 0x0C, bufadr);
			Write_Mem_Des_DD(desadr, desval);

			if (DbgPrn_FRAME_LEN)
				PRINTF(
				    FP_LOG,
				    "[setup_txdes    ] "
				    "FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n",
				    eng->dat.FRAME_LEN_Cur, des_num,
				    eng->run.loop_of_cnt, eng->run.loop_cnt);

			if (DbgPrn_BufAdr)
				printf("[loop[%d]:%4d][des:%4d][setup_txdes] "
				       "%08x [%08x]\n",
				       eng->run.loop_of_cnt, eng->run.loop_cnt,
				       des_num, desadr, bufadr);

			desadr += 16;
			bufadr += bufadrgap;
		}
		barrier();
		Write_Mem_Des_DD(desadr - 0x10, desval | EOR_IniVal);
	} else {
		Write_Mem_Des_DD(desadr, 0);
	}
}

//------------------------------------------------------------
void setup_rxdes (MAC_ENGINE *eng, uint32_t desadr, uint32_t bufbase) 
{
	uint32_t      bufadr;
	uint32_t      desval;
	int32_t       des_num;

	nt_log_func_name();

	bufadr = bufbase;
	desval = RDES_IniVal;
	if ( eng->run.TM_RxDataEn ) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			Write_Mem_Des_DD(desadr + 0x04, 0     );
			Write_Mem_Des_DD(desadr + 0x08, 0     );
			Write_Mem_Des_DD(desadr + 0x0C, bufadr);
			Write_Mem_Des_DD(desadr + 0x00, desval);

			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_rxdes] %08x [%08x]\n", eng->run.loop_of_cnt, eng->run.loop_cnt, des_num, desadr, bufadr);

			desadr += 16;
			bufadr += DMA_PakSize;
		}
		barrier();
		Write_Mem_Des_DD( desadr - 0x10, desval | EOR_IniVal );
	}
	else {
		Write_Mem_Des_DD( desadr, 0x80000000 );
	} // End if ( eng->run.TM_RxDataEn )
} // End void setup_rxdes (uint32_t desadr, uint32_t bufbase)

//------------------------------------------------------------
// First setting TX and RX information
//------------------------------------------------------------
void setup_des (MAC_ENGINE *eng, uint32_t bufnum) 
{
	if (DbgPrn_BufAdr) {
		printf("setup_des: %d\n", bufnum);
		debug_pause();
	}

	eng->dat.DMA_Base_Tx =
	    ZeroCopy_OFFSET + eng->dat.DMA_Base_Setup;
	eng->dat.DMA_Base_Rx = ZeroCopy_OFFSET + GET_DMA_BASE(eng, 0);	

	setup_txdes(eng, eng->run.tdes_base,
		    AT_MEMRW_BUF(eng->dat.DMA_Base_Tx));
	setup_rxdes(eng, eng->run.rdes_base,
		    AT_MEMRW_BUF(eng->dat.DMA_Base_Rx));
} // End void setup_des (uint32_t bufnum)

//------------------------------------------------------------
// Move buffer point of TX and RX descriptor to next DMA buffer
//------------------------------------------------------------
void setup_des_loop (MAC_ENGINE *eng, uint32_t bufnum) 
{
	int32_t des_num;
	uint32_t H_rx_desadr;
	uint32_t H_tx_desadr;
	uint32_t H_tx_bufadr;
	uint32_t H_rx_bufadr;

	nt_log_func_name();

	if (eng->run.TM_RxDataEn) {
		H_rx_bufadr = AT_MEMRW_BUF(eng->dat.DMA_Base_Rx);
		H_rx_desadr = eng->run.rdes_base;
		for (des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++) {
			Write_Mem_Des_DD(H_rx_desadr + 0x0C, H_rx_bufadr);
			Write_Mem_Des_DD(H_rx_desadr, RDES_IniVal);
			if (DbgPrn_BufAdr)
				printf("[loop[%d]:%4d][des:%4d][setup_rxdes] "
				       "%08x [%08x]\n",
				       eng->run.loop_of_cnt, eng->run.loop_cnt,
				       des_num, H_rx_desadr, H_rx_bufadr);

			H_rx_bufadr += DMA_PakSize;
			H_rx_desadr += 16;
		}
		Write_Mem_Des_DD(H_rx_desadr + 0x0C, H_rx_bufadr);
		Write_Mem_Des_DD(H_rx_desadr, RDES_IniVal | EOR_IniVal);
		if (DbgPrn_BufAdr)
			printf("[loop[%d]:%4d][des:%4d][setup_rxdes] %08x "
			       "[%08x]\n",
			       eng->run.loop_of_cnt, eng->run.loop_cnt, des_num,
			       H_rx_desadr, H_rx_bufadr);
	}

	if (eng->run.TM_TxDataEn) {
		H_tx_bufadr = AT_MEMRW_BUF(eng->dat.DMA_Base_Tx);
		H_tx_desadr = eng->run.tdes_base;
		for (des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[des_num];
			Write_Mem_Des_DD(H_tx_desadr + 0x0C, H_tx_bufadr);
			Write_Mem_Des_DD(H_tx_desadr, TDES_IniVal);
			if (DbgPrn_BufAdr)
				printf("[loop[%d]:%4d][des:%4d][setup_txdes] "
				       "%08x [%08x]\n",
				       eng->run.loop_of_cnt, eng->run.loop_cnt,
				       des_num, H_tx_desadr, H_tx_bufadr);

			H_tx_bufadr += DMA_PakSize;
			H_tx_desadr += 16;
		}
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[des_num];
		Write_Mem_Des_DD(H_tx_desadr + 0x0C, H_tx_bufadr);
		Write_Mem_Des_DD(H_tx_desadr, TDES_IniVal | EOR_IniVal);
		if (DbgPrn_BufAdr)
			printf("[loop[%d]:%4d][des:%4d][setup_txdes] %08x "
			       "[%08x]\n",
			       eng->run.loop_of_cnt, eng->run.loop_cnt, des_num,
			       H_tx_desadr, H_tx_bufadr);
	}
} // End void setup_des_loop (uint32_t bufnum)

//------------------------------------------------------------
char check_des_header_Tx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum) 
{
	int timeout = 0;

	eng->dat.TxDes0DW = Read_Mem_Des_DD(adr);

	while (HWOwnTx(eng->dat.TxDes0DW)) {
		// we will run again, if transfer has not been completed.
		if ((eng->run.tm_tx_only || eng->run.TM_RxDataEn) &&
		    (++timeout > eng->run.timeout_th)) {
			PRINTF(FP_LOG,
			       "[%sTxDesOwn] Address %08x = %08x "
			       "[Des:%d][loop[%d]:%d]\n",
			       type, adr, eng->dat.TxDes0DW, desnum,
			       eng->run.loop_of_cnt, eng->run.loop_cnt);
			return (FindErr_Des(eng, Des_Flag_TxOwnTimeOut));
		}

#ifdef Delay_ChkTxOwn
		DELAY(Delay_ChkTxOwn);
#endif
		eng->dat.TxDes0DW = Read_Mem_Des_DD(adr);
	}

	return(0);
} // End char check_des_header_Tx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum)

//------------------------------------------------------------
char check_des_header_Rx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum) 
{
#ifdef CheckRxOwn
	int timeout = 0;

	eng->dat.RxDes0DW = Read_Mem_Des_DD(adr);

	while (HWOwnRx(eng->dat.RxDes0DW)) {
		// we will run again, if transfer has not been completed.
		if (eng->run.TM_TxDataEn && (++timeout > eng->run.timeout_th)) {
#if 0			
			printf("[%sRxDesOwn] Address %08x = %08x "
			       "[Des:%d][loop[%d]:%d]\n",
			       type, adr, eng->dat.RxDes0DW, desnum,
			       eng->run.loop_of_cnt, eng->run.loop_cnt);
#endif
			FindErr_Des(eng, Des_Flag_RxOwnTimeOut);
			return (2);
		}

  #ifdef Delay_ChkRxOwn
		DELAY(Delay_ChkRxOwn);
  #endif
		eng->dat.RxDes0DW = Read_Mem_Des_DD(adr);
	};

  #ifdef CheckRxLen
	if ( DbgPrn_FRAME_LEN )
		PRINTF( FP_LOG, "[%sRxDes          ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", type, ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );

	if ( ( eng->dat.RxDes0DW & 0x3fff ) != ( eng->dat.FRAME_LEN_Cur + 4 ) ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
		PRINTF( FP_LOG, "[%sRxDes] Error Frame Length %08x:%08x %08x(%4d/%4d) [Des:%d][loop[%d]:%d]\n",   type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, ( eng->dat.RxDes0DW & 0x3fff ), ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
		FindErr_Des( eng, Des_Flag_FrameLen );
	}
  #endif // End CheckRxLen

	if ( eng->dat.RxDes0DW & RXDES_EM_ALL ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
  #ifdef CheckRxErr
		if ( eng->dat.RxDes0DW & RXDES_EM_RXERR ) {
			PRINTF( FP_LOG, "[%sRxDes] Error RxErr        %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_RxErr );
		}
  #endif // End CheckRxErr

  #ifdef CheckCRC
		if ( eng->dat.RxDes0DW & RXDES_EM_CRC ) {
			PRINTF( FP_LOG, "[%sRxDes] Error CRC          %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_CRC );
		}
  #endif // End CheckCRC

  #ifdef CheckFTL
		if ( eng->dat.RxDes0DW & RXDES_EM_FTL ) {
			PRINTF( FP_LOG, "[%sRxDes] Error FTL          %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_FTL );
		}
  #endif // End CheckFTL

  #ifdef CheckRunt
		if ( eng->dat.RxDes0DW & RXDES_EM_RUNT) {
			PRINTF( FP_LOG, "[%sRxDes] Error Runt         %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_Runt );
		}
  #endif // End CheckRunt

  #ifdef CheckOddNibble
		if ( eng->dat.RxDes0DW & RXDES_EM_ODD_NB ) {
			PRINTF( FP_LOG, "[%sRxDes] Odd Nibble         %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_OddNibble );
		}
  #endif // End CheckOddNibble

  #ifdef CheckRxFIFOFull
		if ( eng->dat.RxDes0DW & RXDES_EM_FIFO_FULL ) {
			PRINTF( FP_LOG, "[%sRxDes] Error Rx FIFO Full %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.loop_of_cnt, eng->run.loop_cnt );
			FindErr_Des( eng, Des_Flag_RxFIFOFull );
		}
  #endif // End CheckRxFIFOFull
	}

#endif // End CheckRxOwn

	if ( eng->flg.error )
		return(1);
	else
		return(0);
} // End char check_des_header_Rx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum)

//------------------------------------------------------------
char check_des (MAC_ENGINE *eng, uint32_t bufnum, int checkpoint) 
{
	int32_t       desnum;
	int8_t       desnum_last;
	uint32_t      H_rx_desadr;
	uint32_t      H_tx_desadr;
	uint32_t      H_tx_bufadr;
	uint32_t      H_rx_bufadr;
#ifdef Delay_DesGap
	uint32_t      dly_cnt = 0;
	uint32_t      dly_max = Delay_CntMaxIncVal;
#endif
	int ret;

	nt_log_func_name();

	/* Fire the engine to send and recvice */
	mac_reg_write(eng, 0x1c, 0x00000001); // Rx Poll
	mac_reg_write(eng, 0x18, 0x00000001); // Tx Poll

#ifndef SelectSimpleDes
	/* base of the descriptors */
	H_tx_bufadr = AT_MEMRW_BUF(eng->dat.DMA_Base_Tx);
	H_rx_bufadr = AT_MEMRW_BUF(eng->dat.DMA_Base_Rx);
#endif
	H_rx_desadr = eng->run.rdes_base;
	H_tx_desadr = eng->run.tdes_base;

#ifdef Delay_DES
	DELAY(Delay_DES);
#endif

	for (desnum = 0; desnum < eng->dat.Des_Num; desnum++) {
		desnum_last = (desnum == (eng->dat.Des_Num - 1)) ? 1 : 0;
		if ( DbgPrn_BufAdr ) {
			if ( checkpoint )
				printf("[loop[%d]:%4d][des:%4d][check_des  ] %08x %08x [%08x %08x]\n", eng->run.loop_of_cnt, eng->run.loop_cnt, desnum, ( H_tx_desadr ), ( H_rx_desadr ), Read_Mem_Des_DD( H_tx_desadr + 12 ), Read_Mem_Des_DD( H_rx_desadr + 12 ) );
			else
				printf("[loop[%d]:%4d][des:%4d][check_des  ] %08x %08x [%08x %08x]->[%08x %08x]\n", eng->run.loop_of_cnt, eng->run.loop_cnt, desnum, ( H_tx_desadr ), ( H_rx_desadr ), Read_Mem_Des_DD( H_tx_desadr + 12 ), Read_Mem_Des_DD( H_rx_desadr + 12 ), H_tx_bufadr, H_rx_bufadr );
		}

		//[Delay]--------------------
#ifdef Delay_DesGap
//		if ( dly_cnt++ > 3 ) {
		if ( dly_cnt > Delay_CntMax ) {
//			switch ( rand() % 12 ) {
//				case 1 : dly_max = 00000; break;
//				case 3 : dly_max = 20000; break;
//				case 5 : dly_max = 40000; break;
//				case 7 : dly_max = 60000; break;
//				defaule: dly_max = 70000; break;
//			}
//
//			dly_max += ( rand() % 4 ) * 14321;
//
//			while (dly_cnt < dly_max) {
//				dly_cnt++;
//			}
			DELAY( Delay_DesGap );
			dly_cnt = 0;
		}
		else {
			dly_cnt++;
//			timeout = 0;
//			while (timeout < 50000) {timeout++;};
		}
#endif // End Delay_DesGap

		//[Check Owner Bit]--------------------
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[desnum];
		if (DbgPrn_FRAME_LEN)
			PRINTF(FP_LOG,
			       "[check_des      ] "
			       "FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]%d\n",
			       eng->dat.FRAME_LEN_Cur, desnum,
			       eng->run.loop_of_cnt, eng->run.loop_cnt,
			       checkpoint);

		// Check the description of Tx and Rx
		if (eng->run.TM_TxDataEn) {
			ret = check_des_header_Tx(eng, "", H_tx_desadr, desnum);
			if (ret) {
				eng->flg.n_desc_fail = desnum;
				return ret;
			}
		}
		if (eng->run.TM_RxDataEn) {
			ret = check_des_header_Rx(eng, "", H_rx_desadr, desnum);
			if (ret) {
				eng->flg.n_desc_fail = desnum;
				return ret;
				
			}
		}

#ifndef SelectSimpleDes
		if (!checkpoint) {
			// Setting buffer address to description of Tx and Rx on next stage
			if ( eng->run.TM_RxDataEn ) {
				Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal );

				readl(H_rx_desadr);
				mac_reg_write(eng, 0x1c, 0x00000000); //Rx Poll
				H_rx_bufadr += DMA_PakSize;
			}
			if ( eng->run.TM_TxDataEn ) {
				Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal );
				
				readl(H_tx_desadr);
				mac_reg_write(eng, 0x18, 0x00000000); //Tx Poll
				H_tx_bufadr += DMA_PakSize;
			}
		}
#endif // End SelectSimpleDes

		H_rx_desadr += 16;
		H_tx_desadr += 16;
	} // End for (desnum = 0; desnum < eng->dat.Des_Num; desnum++)

	return(0);
} // End char check_des (MAC_ENGINE *eng, uint32_t bufnum, int checkpoint)
//#endif

//------------------------------------------------------------
// Print
//-----------------------------------------------------------
void PrintIO_Header (MAC_ENGINE *eng, uint8_t option) 
{
	int32_t rx_d, step, tmp;

	if (eng->run.TM_IOStrength) {
		if (eng->io.drv_upper_bond > 1) {
#ifdef CONFIG_ASPEED_AST2600			
			PRINTF(option, "<IO Strength register: [%08x] 0x%08x>",
			       eng->io.mac34_drv_reg.addr,
			       eng->io.mac34_drv_reg.value.w);
#else
			PRINTF(option, "<IO Strength register: [%08x] 0x%08x>",
			       eng->io.mac12_drv_reg.addr,
			       eng->io.mac12_drv_reg.value.w);
#endif			       
		}
	}

	if      ( eng->run.speed_sel[ 0 ] ) { PRINTF( option, "\n[1G  ]========================================>\n" ); }
	else if ( eng->run.speed_sel[ 1 ] ) { PRINTF( option, "\n[100M]========================================>\n" ); }
	else                                { PRINTF( option, "\n[10M ]========================================>\n" ); }

	if ( !(option == FP_LOG) ) {
		step = eng->io.rx_delay_scan.step;

		PRINTF(option, "\n    ");
		for (rx_d = eng->io.rx_delay_scan.begin; rx_d <= eng->io.rx_delay_scan.end; rx_d += step) {

			if (rx_d < 0) {
				PRINTF(option, "-" );
			} else {
				PRINTF(option, "+" );
			}
		}

		PRINTF(option, "\n    ");
		for (rx_d = eng->io.rx_delay_scan.begin; rx_d <= eng->io.rx_delay_scan.end; rx_d += step) {
			tmp = (abs(rx_d) >> 4) & 0xf;
			if (tmp == 0) {
				PRINTF(option, "0" );
			} else {
				PRINTF(option, "%1x", tmp);
			}
		}

		PRINTF(option, "\n    ");
		for (rx_d = eng->io.rx_delay_scan.begin;
		     rx_d <= eng->io.rx_delay_scan.end; rx_d += step) {
			PRINTF(option, "%1x", (uint32_t)abs(rx_d) & 0xf);
		}

		PRINTF(option, "\n    ");
		for (rx_d = eng->io.rx_delay_scan.begin; rx_d <= eng->io.rx_delay_scan.end; rx_d += step) {
			if (eng->io.rx_delay_scan.orig == rx_d) {
				PRINTF(option, "|" );
			} else {
				PRINTF(option, " " );
			}
		}
		PRINTF( option, "\n");
	}
}

//------------------------------------------------------------
void PrintIO_LineS(MAC_ENGINE *p_eng, uint8_t option)
{
	if (p_eng->io.tx_delay_scan.orig == p_eng->io.Dly_out_selval) {
		PRINTF( option, "%02x:-", p_eng->io.Dly_out_selval); 
	} else {
		PRINTF( option, "%02x: ", p_eng->io.Dly_out_selval);
	}	
} // End void PrintIO_LineS (MAC_ENGINE *eng, uint8_t option)

//------------------------------------------------------------
void PrintIO_Line(MAC_ENGINE *p_eng, uint8_t option) 
{
	if ((p_eng->io.Dly_in_selval == p_eng->io.rx_delay_scan.orig) && 
	    (p_eng->io.Dly_out_selval == p_eng->io.tx_delay_scan.orig)) {
		if (1 == p_eng->io.result) {
			PRINTF(option, "X");
		} else if (2 == p_eng->io.result) {
			PRINTF(option, "*");
		} else {
			PRINTF(option, "O");
		}
	} else {
		if (1 == p_eng->io.result) {
			PRINTF(option, "x");
		} else if (2 == p_eng->io.result) {
			PRINTF(option, ".");
		} else {
			PRINTF(option, "o");
		}
	}	
}

//------------------------------------------------------------
// main
//------------------------------------------------------------

//------------------------------------------------------------
void TestingSetup (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	//[Setup]--------------------
	setup_framesize( eng );
	setup_buf( eng );
}

//------------------------------------------------------------
// Return 1 ==> fail
// Return 0 ==> PASS
//------------------------------------------------------------
char TestingLoop (MAC_ENGINE *eng, uint32_t loop_checknum) 
{
	char       checkprd;
	char       looplast;
	char       checken;
	int ret;

	nt_log_func_name();

	if (DbgPrn_DumpMACCnt)
		dump_mac_ROreg(eng);

	//[Setup]--------------------
	eng->run.loop_cnt = 0;
	checkprd = 0;
	checken  = 0;
	looplast = 0;


	setup_des(eng, 0);

	if ( eng->run.TM_WaitStart ) {
		printf("Press any key to start...\n");
		GET_CAHR();
	}


	while ( ( eng->run.loop_cnt < eng->run.loop_max ) || eng->arg.loop_inf ) {
		looplast = !eng->arg.loop_inf && ( eng->run.loop_cnt == eng->run.loop_max - 1 );

#ifdef CheckRxBuf
		if (!eng->run.tm_tx_only)
			checkprd = ((eng->run.loop_cnt % loop_checknum) == (loop_checknum - 1));
		checken = looplast | checkprd;
#endif

		if (DbgPrn_BufAdr) {
			printf("for start ======> [%d]%d/%d(%d) looplast:%d "
			       "checkprd:%d checken:%d\n",
			       eng->run.loop_of_cnt, eng->run.loop_cnt,
			       eng->run.loop_max, eng->arg.loop_inf,
			       looplast, checkprd, checken);
			debug_pause();
		}


		if (eng->run.TM_RxDataEn)
			eng->dat.DMA_Base_Tx = eng->dat.DMA_Base_Rx;

		eng->dat.DMA_Base_Rx =
		    ZeroCopy_OFFSET + GET_DMA_BASE(eng, eng->run.loop_cnt + 1);
		//[Check DES]--------------------
		ret = check_des(eng, eng->run.loop_cnt, checken);
		if (ret) {
			//descriptor error
			eng->dat.Des_Num = eng->flg.n_desc_fail + 1;
#ifdef CheckRxBuf
			if (checkprd)
				check_buf(eng, loop_checknum);
			else
				check_buf(eng, (eng->run.loop_max % loop_checknum));
			eng->dat.Des_Num = eng->dat.Des_Num_Org;
#endif

			if (DbgPrn_DumpMACCnt)
				dump_mac_ROreg(eng);

			return ret;
		}

		//[Check Buf]--------------------
		if (eng->run.TM_RxDataEn && checken) {
			if (checkprd) {
#ifdef Enable_ShowBW  
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				PRINTF( FP_LOG, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );  
#endif

#ifdef CheckRxBuf
				if (check_buf(eng, loop_checknum))
					return(1);
#endif
			} else {
#ifdef Enable_ShowBW  
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.loop_max % loop_checknum), ((double)(eng->run.loop_max % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				PRINTF( FP_LOG, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.loop_max % loop_checknum), ((double)(eng->run.loop_max % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );  
#endif

#ifdef CheckRxBuf
				if (check_buf(eng, (eng->run.loop_max % loop_checknum)))
					return(1);
#endif
			} // End if ( checkprd )

#ifndef SelectSimpleDes
			if (!looplast)
				setup_des_loop(eng, eng->run.loop_cnt);
#endif

#ifdef Enable_ShowBW
			timeold = clock();
#endif
		} // End if ( eng->run.TM_RxDataEn && checken )

#ifdef SelectSimpleDes
		if (!looplast)
			setup_des_loop(eng, eng->run.loop_cnt);
#endif

		if ( eng->arg.loop_inf )
			printf("===============> Loop[%d]: %d  \r", eng->run.loop_of_cnt, eng->run.loop_cnt);
		else if ( eng->arg.test_mode == 0 ) {
			if ( !( DbgPrn_BufAdr || eng->run.delay_margin ) )
				printf(" [%d]%d                        \r", eng->run.loop_of_cnt, eng->run.loop_cnt);
		}

		if (DbgPrn_BufAdr) {
			printf("for end   ======> [%d]%d/%d(%d)\n",
			       eng->run.loop_of_cnt, eng->run.loop_cnt,
			       eng->run.loop_max, eng->arg.loop_inf);
			debug_pause();
		}

		if (eng->run.loop_cnt >= 0x7fffffff) {	
			debug("loop counter wrapped around\n");
			eng->run.loop_cnt = 0;
			eng->run.loop_of_cnt++;
		} else
			eng->run.loop_cnt++;
	} // End while ( ( eng->run.loop_cnt < eng->run.loop_max ) || eng->arg.loop_inf )

	eng->flg.all_fail = 0;
	return(0);
} // End char TestingLoop (MAC_ENGINE *eng, uint32_t loop_checknum)
