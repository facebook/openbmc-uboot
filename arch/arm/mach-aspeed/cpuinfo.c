// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/aspeed_scu_info.h>
#include <asm/arch/platform.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
/* SoC mapping Table */
struct soc_id {
	const char *name;
	u32	   rev_id;
};

#define SOC_ID(str, rev) { .name = str, .rev_id = rev, }

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

void aspeed_get_revision_id(void)
{
	int i;
	u32 rev_id = readl(ASPEED_REVISION_ID);
	for(i=0;i<ARRAY_SIZE(soc_map_table);i++) {
		if(rev_id == soc_map_table[i].rev_id)
			break;
	}
	if(i == ARRAY_SIZE(soc_map_table))
		printf("UnKnow-SOC : %x \n",rev_id);
	else
		printf("SOC : %4s \n",soc_map_table[i].name);
}

int print_cpuinfo(void)
{
	int i = 0;
//	ulong size = 0;

	aspeed_get_revision_id();
	aspeed_sys_reset_info();
	aspeed_security_info();

#if 0
	printf("PLL :   %4s MHz\n", strmhz(buf, aspeed_get_clk_in_rate()));

	printf("CPU :   %4s MHz\n", strmhz(buf, aspeed_get_hpll_clk_rate()));
	printf("MPLL :	%4s MHz, ECC: %s, ",
	       strmhz(buf, aspeed_get_mpll_clk_rate()),
	       ast_sdmc_get_ecc() ? "Enable" : "Disable");

	if(ast_sdmc_get_ecc())
		printf("recover %d, un-recover %d, ", ast_sdmc_get_ecc_recover_count(), ast_sdmc_get_ecc_unrecover_count());
	if(ast_sdmc_get_ecc())
		printf("Size : %d MB, ", ast_sdmc_get_ecc_size()/1024/1024);

#if defined(CONFIG_MACH_ASPEED_G5)
	printf("Cache: %s ",ast_sdmc_get_cache() ? "Enable" : "Disable");
#endif
	aspeed_who_init_dram();

	size = ast_sdmc_get_vram_size();

	puts("VGA :    ");
	print_size(size, "- ");

	size = ast_sdmc_get_mem_size();
	puts("Total DRAM : ");
	print_size(size, "\n");
#endif	

	aspeed_2nd_wdt_mode();

	aspeed_spi_strap_mode();

	aspeed_espi_mode();

	puts("Eth :    ");
	for(i = 0; i < ASPEED_MAC_COUNT; i++) {
		printf("MAC%d: %s ",i, aspeed_get_mac_phy_interface(i) ? "RGMII" : "RMII/NCSI");
		if(i != (ASPEED_MAC_COUNT - 1))
			printf(",");
	}
	puts("\n");

	return 0;
}
#endif
