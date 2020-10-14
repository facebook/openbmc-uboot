#include <asm/io.h>
#include <asm/arch/aspeed_flash.h>
#include <common.h>
#include <asm/arch/platform.h>


/* for DMA */

#define REG_FLASH_INTERRUPT_STATUS	0x08
#define REG_FLASH_DMA_CONTROL		0x80
#define REG_FLASH_DMA_FLASH_BASE	0x84
#define REG_FLASH_DMA_DRAM_BASE		0x88
#define REG_FLASH_DMA_LENGTH		0x8c

#define FLASH_STATUS_DMA_BUSY		0x0000
#define FLASH_STATUS_DMA_READY		0x0800
#define FLASH_STATUS_DMA_CLEAR		0x0800

#define AST_FMC_INT_CTRL_STAT   (ASPEED_FMC_BASE + REG_FLASH_INTERRUPT_STATUS)
#define AST_FMC_DMA_CTRL        (ASPEED_FMC_BASE + REG_FLASH_DMA_CONTROL)
#define AST_FMC_DMA_FLASH_ADDR  (ASPEED_FMC_BASE + REG_FLASH_DMA_FLASH_BASE)
#define AST_FMC_DMA_DRAM_ADDR   (ASPEED_FMC_BASE + REG_FLASH_DMA_DRAM_BASE)
#define AST_FMC_DMA_LENGTH      (ASPEED_FMC_BASE + REG_FLASH_DMA_LENGTH)


#define AST_FMC_DMA_CTRL_ENABLE          (1 << 0)
#define AST_FMC_DMA_CTRL_DIR_READ        (0 << 1)
#define AST_FMC_DMA_CTRL_CLEAR_EN_DIR     ~(0x03)  // Clear DMA direction, enable


void aspeed_spi_dma_copy(void * dest, const void *src, size_t count)
{
	u32 count_align, poll_time, data;

	count_align = (count + 3) & 0xFFFFFFFC; /* 4-bytes align */
	poll_time = 100; /* set 100 us as default */

	writel((u32)src, AST_FMC_DMA_FLASH_ADDR);
	writel((u32)dest, AST_FMC_DMA_DRAM_ADDR);
	writel(count_align, AST_FMC_DMA_LENGTH);
	data = readl(AST_FMC_DMA_CTRL);
	data &= AST_FMC_DMA_CTRL_CLEAR_EN_DIR;
	data |= (AST_FMC_DMA_CTRL_DIR_READ | AST_FMC_DMA_CTRL_ENABLE);
	writel(data, AST_FMC_DMA_CTRL);

	/* wait poll */
	do {
		udelay(poll_time);
		data = readl(AST_FMC_INT_CTRL_STAT);
	} while (!(data & FLASH_STATUS_DMA_READY));

	data = readl(AST_FMC_DMA_CTRL);
	data &= ~AST_FMC_DMA_CTRL_ENABLE;
	writel(data, AST_FMC_DMA_CTRL);
}
