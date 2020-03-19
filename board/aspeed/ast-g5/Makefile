
ifeq ($(CONFIG_SPL_BUILD),y)
obj-y   = ast-g5-spl.o flash-spl.o
else
obj-y   = ast-g5.o 
endif
obj-$(CONFIG_ASPEED_TPM) += tpm-spl.o
obj-y += util.o
