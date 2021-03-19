
ifeq ($(CONFIG_SPL_BUILD),y)
obj-y   = ast-g5-spl.o flash-spl.o
else
obj-y   = ast-g5.o 
endif
ifeq ($(CONFIG_TPM_V2), y)
obj-$(CONFIG_ASPEED_TPM) += tpm2-spl.o
else
obj-$(CONFIG_ASPEED_TPM) += tpm-spl.o
endif
obj-y += util.o
