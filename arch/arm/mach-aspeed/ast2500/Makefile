ifdef CONFIG_ARCH_AST2500
obj-y += ../ast-sdk/ast-scu.o ../ast-sdk/ast-sdmc.o ../ast-sdk/platform_g5.o scu_ast2500.o utils.o
else
obj-y += platform.o board_common.o scu_info.o utils.o
obj-$(CONFIG_SPL_BUILD) += spl.o
endif
