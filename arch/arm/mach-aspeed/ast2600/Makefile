ifeq ($(CONFIG_FB_AST_GEN_6),n) # Aspeed SDK
obj-y   += platform.o board_common.o scu_info.o utils.o cache.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_FMC_DUAL_BOOT) += fmc_dual_boot.o
else

# Facebook AST2600 platform common code

ifeq ($(CONFIG_SPL_BUILD), y)
# SPL we only need
obj-y 	+= platform.o
else
obj-y   += platform.o board_common.o scu_info.o utils.o cache.o
obj-$(CONFIG_FMC_DUAL_BOOT) += fmc_dual_boot.o
endif

endif

