obj-$(CONFIG_ELBERTVBOOT) += elbert.o 
obj-$(CONFIG_SPL) += vboot-abb.o
ifeq ($(CONFIG_SPL_BUILD), y)
obj-y += vboot-spl.o flash-spl.o
else
obj-$(CONFIG_TARGET_FB_AST_GEN_4) += ast-g4.o
obj-$(CONFIG_TARGET_FB_AST_GEN_5) += ast-g5.o vboot.o
obj-$(CONFIG_TARGET_FB_AST_GEN_6) += ast-g6.o vboot.o
endif

ifeq ($(CONFIG_TPM_V2), y)
obj-$(CONFIG_ASPEED_TPM) += tpm2-spl.o
else
obj-$(CONFIG_ASPEED_TPM) += tpm-spl.o
endif
obj-y += util.o
