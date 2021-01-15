#include <asm/arch/platform.h>
#include "mem_io.h"

uint32_t SRAM_RD(uint32_t addr)
{
	return readl(ASPEED_SRAM_BASE + addr);
}