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

extern int
aspeed_get_mac_phy_interface(u8 num)
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

extern void
aspeed_security_info(void)
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

extern void 
aspeed_sys_reset_info(void)
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

extern void
aspeed_who_init_dram(void)
{
	if(readl(ASPEED_VGA_HANDSHAKE0) & SOC_FW_INIT_DRAM)
		printf("[init by SOC]\n");
	else
		printf("[init by VBIOS]\n");
}

extern void
aspeed_2nd_wdt_mode(void)
{
	if(readl(ASPEED_HW_STRAP1) & BIT(17))
		printf("2nd Boot : Enable\n");
}

extern void
aspeed_spi_strap_mode(void)
{
	return;
}

extern void
aspeed_espi_mode(void)
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

