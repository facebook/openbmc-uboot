ifdef CONFIG_ARCH_AST2500
obj-y += ../ast-sdk/ast-scu.o ../ast-sdk/ast-sdmc.o ../ast-sdk/platform_g5.o scu_ast2500.o
else
obj-y += platform.o aspeed_scu_info.o
obj-$(CONFIG_SPL_BUILD) += spl.o
endif
