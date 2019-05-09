/*
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/arch/aspeed.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];
	ulong size = 0;

	ast_scu_revision_id();

	ast_scu_sys_rest_info();

#ifdef AST_SOC_G5
	ast_scu_security_info();
#endif
	printf("PLL :   %4s MHz\n", strmhz(buf, ast_get_clk_source()));
	printf("CPU :   %4s MHz\n", strmhz(buf, ast_get_h_pll_clk()));
#ifdef AST_SOC_G5
	printf("MEM :	%4s MHz, EEC: %s, Cache: %s \n",
	       strmhz(buf, ast_get_m_pll_clk() * 2),
	       ast_sdmc_get_eec() ? "Enable" : "Disable",
	       ast_sdmc_get_cache() ? "Enable" : "Disable");
#else
	printf("MEM :   %4s MHz, EEC:%s \n",
	       strmhz(buf, ast_get_m_pll_clk()),
	       ast_sdmc_get_eec() ? "Enable" : "Disable");
#endif
	size = ast_scu_get_vga_memsize();

	puts("VGA :    ");
	print_size(size, "\n");

	ast_scu_get_who_init_dram();
	return 0;
}
#endif
