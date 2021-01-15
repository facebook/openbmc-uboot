// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/err.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#define SDMC_CONFIG_VRAM_GET(x)         ((x >> 2) & 0x3)
#define SDMC_CONFIG_MEM_GET(x)          (x & 0x3)

static const u32 ast2400_dram_table[] = {
	0x04000000,     //64MB
	0x08000000,     //128MB
	0x10000000, 	//256MB
	0x20000000,     //512MB
};

u32
ast_sdmc_get_mem_size(void)
{
	u32 size = 0;
	u32 size_conf = SDMC_CONFIG_MEM_GET(readl(0x1e6e0004));

	size = ast2400_dram_table[size_conf];

	return size;
}


static const u32 aspeed_vram_table[] = {
	0x00800000,     //8MB
	0x01000000,     //16MB
	0x02000000,     //32MB
	0x04000000,     //64MB
};

u32
ast_sdmc_get_vram_size(void)
{
	u32 size_conf = SDMC_CONFIG_VRAM_GET(readl(0x1e6e0004));
	return aspeed_vram_table[size_conf];
}

__weak int dram_init(void)
{
#if 0
	struct udevice *dev;
	struct ram_info ram;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM FAIL1\r\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("DRAM FAIL2\r\n");
		return ret;
	}

	gd->ram_size = ram.size;
#else
	u32 vga = ast_sdmc_get_vram_size();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = (dram - vga);
#endif
	return 0;
}
