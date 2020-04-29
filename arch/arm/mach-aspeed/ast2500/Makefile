obj-y += clk_ast2500.o

ifdef CONFIG_ARCH_AST2500
obj-y += ../ast-sdk/ast-scu.o ../ast-sdk/ast-sdmc.o ../ast-sdk/platform_g5.o ../ast-sdk/flash.o scu_ast2500.o

ifdef CONFIG_ASPEED_SPI_DMA
obj-y += utils.o
endif

else
obj-y += sdram_ast2500.o
endif
