// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
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
	SOC_ID("AST2600-A1", 0x05010303),
	SOC_ID("AST2620-A1", 0x05010203),
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
		printf("UnKnow-SOC: %x \n",rev_id);
	else
		printf("SOC: %4s \n",soc_map_table[i].name);
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
	if(readl(ASPEED_HW_STRAP1) & BIT(1))
		printf("Security Boot \n");
}	

/*	ASPEED_SYS_RESET_CTRL	: System reset contrl/status register*/
#define SYS_WDT8_SW_RESET	BIT(15)
#define SYS_WDT8_ARM_RESET	BIT(14)
#define SYS_WDT8_FULL_RESET	BIT(13)
#define SYS_WDT8_SOC_RESET	BIT(12)
#define SYS_WDT7_SW_RESET	BIT(11)
#define SYS_WDT7_ARM_RESET	BIT(10)
#define SYS_WDT7_FULL_RESET	BIT(9)
#define SYS_WDT7_SOC_RESET	BIT(8)
#define SYS_WDT6_SW_RESET	BIT(7)
#define SYS_WDT6_ARM_RESET	BIT(6)
#define SYS_WDT6_FULL_RESET	BIT(5)
#define SYS_WDT6_SOC_RESET	BIT(4)
#define SYS_WDT5_SW_RESET	BIT(3)
#define SYS_WDT5_ARM_RESET	BIT(2)
#define SYS_WDT5_FULL_RESET	BIT(1)
#define SYS_WDT5_SOC_RESET	BIT(0)

#define SYS_WDT4_SW_RESET	BIT(31)
#define SYS_WDT4_ARM_RESET	BIT(30)
#define SYS_WDT4_FULL_RESET	BIT(29)
#define SYS_WDT4_SOC_RESET	BIT(28)
#define SYS_WDT3_SW_RESET	BIT(27)
#define SYS_WDT3_ARM_RESET	BIT(26)
#define SYS_WDT3_FULL_RESET	BIT(25)
#define SYS_WDT3_SOC_RESET	BIT(24)
#define SYS_WDT2_SW_RESET	BIT(23)
#define SYS_WDT2_ARM_RESET	BIT(22)
#define SYS_WDT2_FULL_RESET	BIT(21)
#define SYS_WDT2_SOC_RESET	BIT(20)
#define SYS_WDT1_SW_RESET	BIT(19)
#define SYS_WDT1_ARM_RESET	BIT(18)
#define SYS_WDT1_FULL_RESET	BIT(17)
#define SYS_WDT1_SOC_RESET	BIT(16)

#define SYS_CM3_EXT_RESET	BIT(6)
#define SYS_PCI2_RESET		BIT(5)
#define SYS_PCI1_RESET		BIT(4)
#define SYS_DRAM_ECC_RESET	BIT(3)
#define SYS_FLASH_ABR_RESET	BIT(2)
#define SYS_EXT_RESET		BIT(1)
#define SYS_PWR_RESET_FLAG	BIT(0)

#define BIT_WDT_SOC(x)	SYS_WDT ## x ## _SOC_RESET
#define BIT_WDT_FULL(x)	SYS_WDT ## x ## _FULL_RESET
#define BIT_WDT_ARM(x)	SYS_WDT ## x ## _ARM_RESET
#define BIT_WDT_SW(x)	SYS_WDT ## x ## _SW_RESET

#define HANDLE_WDTx_RESET(x, event_log, event_log_reg) \
	if (event_log & (BIT_WDT_SOC(x) | BIT_WDT_FULL(x) | BIT_WDT_ARM(x) | BIT_WDT_SW(x))) { \
		printf("RST: WDT%d ", x); \
		if (event_log & BIT_WDT_SOC(x)) { \
			printf("SOC "); \
			writel(BIT_WDT_SOC(x), event_log_reg); \
		} \
		if (event_log & BIT_WDT_FULL(x)) { \
			printf("FULL "); \
			writel(BIT_WDT_FULL(x), event_log_reg); \
		} \
		if (event_log & BIT_WDT_ARM(x)) { \
			printf("ARM "); \
			writel(BIT_WDT_ARM(x), event_log_reg); \
		} \
		if (event_log & BIT_WDT_SW(x)) { \
			printf("SW "); \
			writel(BIT_WDT_SW(x), event_log_reg); \
		} \
		printf("\n"); \
	} \
	(void)(x)

void aspeed_print_sysrst_info(void)
{
	u32 rest = readl(ASPEED_SYS_RESET_CTRL);
	u32 rest3 = readl(ASPEED_SYS_RESET_CTRL3);

	if (rest & SYS_PWR_RESET_FLAG) {
		printf("RST: Power On \n");
		writel(rest, ASPEED_SYS_RESET_CTRL);
	} else {
		HANDLE_WDTx_RESET(8, rest3, ASPEED_SYS_RESET_CTRL3);
		HANDLE_WDTx_RESET(7, rest3, ASPEED_SYS_RESET_CTRL3);
		HANDLE_WDTx_RESET(6, rest3, ASPEED_SYS_RESET_CTRL3);
		HANDLE_WDTx_RESET(5, rest3, ASPEED_SYS_RESET_CTRL3);
		HANDLE_WDTx_RESET(4, rest, ASPEED_SYS_RESET_CTRL);
		HANDLE_WDTx_RESET(3, rest, ASPEED_SYS_RESET_CTRL);
		HANDLE_WDTx_RESET(2, rest, ASPEED_SYS_RESET_CTRL);
		HANDLE_WDTx_RESET(1, rest, ASPEED_SYS_RESET_CTRL);

		if (rest & SYS_CM3_EXT_RESET) {
			printf("RST: SYS_CM3_EXT_RESET \n");
			writel(SYS_CM3_EXT_RESET, ASPEED_SYS_RESET_CTRL);		
		}
		
		if (rest & (SYS_PCI1_RESET | SYS_PCI2_RESET)) {
			printf("PCI RST: ");
			if (rest & SYS_PCI1_RESET) {
				printf("#1 ");
				writel(SYS_PCI1_RESET, ASPEED_SYS_RESET_CTRL);		
			}
			
			if (rest & SYS_PCI2_RESET) {
				printf("#2 ");
				writel(SYS_PCI2_RESET, ASPEED_SYS_RESET_CTRL);		
			}
			printf("\n");
		}

		if (rest & SYS_DRAM_ECC_RESET) {
			printf("RST: DRAM_ECC_RESET \n");
			writel(SYS_FLASH_ABR_RESET, ASPEED_SYS_RESET_CTRL);		
		}

		if (rest & SYS_FLASH_ABR_RESET) {
			printf("RST: SYS_FLASH_ABR_RESET \n");
			writel(SYS_FLASH_ABR_RESET, ASPEED_SYS_RESET_CTRL);		
		}
		if (rest & SYS_EXT_RESET) {
			printf("RST: External \n");
			writel(SYS_EXT_RESET, ASPEED_SYS_RESET_CTRL);
		}	
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
	if(readl(ASPEED_HW_STRAP2) & BIT(11)) {
		printf("2nd Boot: Enable, ");
		if(readl(ASPEED_HW_STRAP2) & BIT(12))
			printf("Single SPI ");
		else
			printf("Dual SPI ");
		printf(": %s", readl(0x1e620064) & BIT(4) ? "Alternate":"Primary");

		if(readl(ASPEED_HW_STRAP2) & GENMASK(15, 13)) {
			printf(", bspi_size : %ld MB\n", BIT((readl(ASPEED_HW_STRAP2) >> 13) & 0x7));
		} else
			printf("\n");
	}

	if(readl(ASPEED_HW_STRAP2) & BIT(22)) {
		printf("SPI aux control : Enable");
		//gpioY6 : BSPI_ABR 
		if (readl(0x1e7801e0) & BIT(6))
			printf(", Force Alt boot ");

		//gpioY7 : BSPI_WP_N
		printf(", BSPI_WP : %s \n", readl(0x1e7801e0) & BIT(7) ? "Disable":"Enable");
	}
}

void aspeed_print_spi_strap_mode(void)
{
	if(readl(ASPEED_HW_STRAP2) & BIT(10))
		printf("SPI: 3/4 byte mode auto detection \n");
}

void aspeed_print_espi_mode(void)
{
	int espi_mode = 0;
	int sio_disable = 0;
	u32 sio_addr = 0x2e;

	if (readl(ASPEED_HW_STRAP2) & BIT(6))
		espi_mode = 0;
	else
		espi_mode = 1;

	if (readl(ASPEED_HW_STRAP2) & BIT(2))
		sio_addr = 0x4e;

	if (readl(ASPEED_HW_STRAP2) & BIT(3))
		sio_disable = 1;

	if (espi_mode)
		printf("eSPI Mode: SIO:%s ", sio_disable ? "Disable" : "Enable");
	else
		printf("LPC Mode: SIO:%s ", sio_disable ? "Disable" : "Enable");

	if (!sio_disable)
		printf(": SuperIO-%02x\n", sio_addr);
	else
		printf("\n");
}

void aspeed_print_mac_info(void)
{
	int i;
	printf("Eth: ");
	for (i = 0; i < ASPEED_MAC_COUNT; i++) {
		printf("MAC%d: %s", i,
				aspeed_get_mac_phy_interface(i) ? "RGMII" : "RMII/NCSI");
		if (i != (ASPEED_MAC_COUNT -1))
			printf(", ");
	}
	printf("\n");
}
