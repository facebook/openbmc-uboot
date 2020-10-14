#ifndef __ASPEED_FLASH_H_INCLUDED
#define __ASPEED_FLASH_H_INCLUDED

#ifdef CONFIG_ASPEED_SPI_DMA
extern void aspeed_spi_dma_copy(void * dest, const void *src, size_t count);
#endif

#endif
