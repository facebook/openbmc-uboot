// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/aspeed_scu_info.h>

/* SoC mapping Table */
#define SOC_ID(str, rev) { .name = str, .rev_id = rev, }

struct soc_id {
	const char *name;
	u32	   rev_id;
};

static struct soc_id soc_map_table[] = {
	SOC_ID("AST1100/AST2050-A0", 0x00000200),
	SOC_ID("AST1100/AST2050-A1", 0x00000201),
	SOC_ID("AST1100/AST2050-A2,3/AST2150-A0,1", 0x00000202),
	SOC_ID("AST1510/AST2100-A0", 0x00000300),
	SOC_ID("AST1510/AST2100-A1", 0x00000301),
	SOC_ID("AST1510/AST2100-A2,3", 0x00000302),
	SOC_ID("AST2200-A0,1", 0x00000102),
	SOC_ID("AST2300-A0", 0x01000003),
	SOC_ID("AST2300-A1", 0x01010303),
	SOC_ID("AST1300-A1", 0x01010003),
	SOC_ID("AST1050-A1", 0x01010203),
	SOC_ID("AST2400-A0", 0x02000303),
	SOC_ID("AST2400-A1", 0x02010303),
	SOC_ID("AST1010-A0", 0x03000003),
	SOC_ID("AST1010-A1", 0x03010003),
	SOC_ID("AST3200-A0", 0x04002003),
	SOC_ID("AST3200-A1", 0x04012003),
	SOC_ID("AST3200-A2", 0x04032003),
	SOC_ID("AST1520-A0", 0x03000203),	
	SOC_ID("AST1520-A1", 0x03010203),
	SOC_ID("AST2510-A0", 0x04000103),
	SOC_ID("AST2510-A1", 0x04010103),
	SOC_ID("AST2510-A2", 0x04030103),	
	SOC_ID("AST2520-A0", 0x04000203),
	SOC_ID("AST2520-A1", 0x04010203),
	SOC_ID("AST2520-A2", 0x04030203),
	SOC_ID("AST2500-A0", 0x04000303),	
	SOC_ID("AST2500-A1", 0x04010303),
	SOC_ID("AST2500-A2", 0x04030303),	
	SOC_ID("AST2530-A0", 0x04000403),
	SOC_ID("AST2530-A1", 0x04010403),
	SOC_ID("AST2530-A2", 0x04030403),	
	SOC_ID("AST2600-A0", 0x05000303),
};

void aspeed_print_soc_id(void)
{
	int i;
	u32 rev_id = readl(ASPEED_REVISION_ID);
	for (i = 0; i < ARRAY_SIZE(soc_map_table); i++) {
		if (rev_id == soc_map_table[i].rev_id)
			break;
	}
	if (i == ARRAY_SIZE(soc_map_table))
		printf("UnKnow-SOC : %x \n",rev_id);
	else
		printf("SOC : %4s \n",soc_map_table[i].name);
}

int aspeed_get_mac_phy_interface(u8 num)
{
	u32 strap1 = readl(ASPEED_HW_STRAP1);
#ifdef ASPEED_HW_STRAP2
	u32 strap2 = readl(ASPEED_HW_STRAP2);
#endif
	switch(num) {
		case 0:
			if(strap1 & BIT(6)) {
				return 1;
			} else {
				return 0;
			}
			break;
		case 1:
			if(strap1 & BIT(7)) {
				return 1;
			} else {
				return 0;
			}
			break;
#ifdef ASPEED_HW_STRAP2			
		case 2:
			if(strap2 & BIT(0)) {
				return 1;
			} else {
				return 0;
			}
			break;
		case 3:
			if(strap2 & BIT(1)) {
				return 1;
			} else {
				return 0;
			}
			break;
#endif			
	}
	return -1;
}

void aspeed_print_security_info(void)
{
	switch((readl(ASPEED_HW_STRAP2) >> 18) & 0x3) {
		case 1:
			printf("SEC : DSS Mode \n");
			break;
		case 2:
			printf("SEC : UnKnow \n");
			break;			
		case 3:
			printf("SEC : SPI2 Mode \n");
			break;						
	}
}	

/*	ASPEED_SYS_RESET_CTRL	: System reset contrl/status register*/
#define SYS_WDT3_RESET			BIT(4)
#define SYS_WDT2_RESET			BIT(3)
#define SYS_WDT1_RESET			BIT(2)
#define SYS_EXT_RESET			BIT(1)
#define SYS_PWR_RESET_FLAG		BIT(0)

void aspeed_print_sysrst_info(void)
{
	u32 rest = readl(ASPEED_SYS_RESET_CTRL);

	if (rest & SYS_WDT1_RESET) {
		printf("RST : WDT1 \n");		
		writel(readl(ASPEED_SYS_RESET_CTRL) & ~SYS_WDT1_RESET, ASPEED_SYS_RESET_CTRL);
	}
	if (rest & SYS_WDT2_RESET) {
		printf("RST : WDT2 - 2nd Boot \n");
		writel(readl(ASPEED_SYS_RESET_CTRL) & ~SYS_WDT2_RESET, ASPEED_SYS_RESET_CTRL);
		if(readl(0x1e785030) & BIT(1))
			puts("default boot\n");
		else
			puts("second boot\n");		
	}
	if (rest & SYS_WDT3_RESET) {
		printf("RST : WDT3 - Boot\n");
		writel(readl(ASPEED_SYS_RESET_CTRL) & ~SYS_WDT3_RESET, ASPEED_SYS_RESET_CTRL);
	}
	if(rest & SYS_EXT_RESET) {
		printf("RST : External \n");
		writel(readl(ASPEED_SYS_RESET_CTRL) & ~SYS_EXT_RESET, ASPEED_SYS_RESET_CTRL);
	}	
	if (rest & SYS_PWR_RESET_FLAG) {
		printf("RST : Power On \n");
		writel(readl(ASPEED_SYS_RESET_CTRL) & ~SYS_PWR_RESET_FLAG, ASPEED_SYS_RESET_CTRL);
	}
}

#define SOC_FW_INIT_DRAM		BIT(7)

void aspeed_print_dram_initializer(void)
{
	if(readl(ASPEED_VGA_HANDSHAKE0) & SOC_FW_INIT_DRAM)
		printf("[init by SOC]\n");
	else
		printf("[init by VBIOS]\n");
}

void aspeed_print_2nd_wdt_mode(void)
{
	if(readl(ASPEED_HW_STRAP1) & BIT(17))
		printf("2nd Boot : Enable\n");
}

void aspeed_print_spi_strap_mode(void)
{
	return;
}

void aspeed_print_espi_mode(void)
{
	int espi_mode = 0;
	int sio_disable = 0;
	u32 sio_addr = 0x2e;

	if(readl(ASPEED_HW_STRAP1) & BIT(25))
		espi_mode = 1;
	else
		espi_mode = 0;

	if(readl(ASPEED_HW_STRAP1) & BIT(16))
		sio_addr = 0x4e;

	if(readl(ASPEED_HW_STRAP1) & BIT(20))
		sio_disable = 1;

	if(espi_mode)
		printf("eSPI Mode : SIO:%s ",  sio_disable ? "Disable" : "Enable");
	else
		printf("LPC Mode : SIO:%s ",  sio_disable ? "Disable" : "Enable");

	if(!sio_disable)
		printf(": SuperIO-%02x\n",  sio_addr);
	else
		printf("\n");
}

void aspeed_print_mac_info(void)
{
	int i;
	printf("Eth :\n");
	for (i = 0; i < ASPEED_MAC_COUNT; i++)
		printf("    MAC%d: %s\n", i,
				aspeed_get_mac_phy_interface(i) ? "RGMII" : "RMII/NCSI");
}
