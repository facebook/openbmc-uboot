/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <debug_uart.h>
#include <spl.h>
#include <dm.h>
#include <mmc.h>
#include <xyzModem.h>
#include <asm/io.h>
#include <asm/arch/aspeed_verify.h>

static int aspeed_spl_ram_load_image(struct spl_image_info *spl_image,
				      struct spl_boot_device *bootdev)
{
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_SPI_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_SPI_BASE;
	return 0;
}
SPL_LOAD_IMAGE_METHOD("RAM", 0, ASPEED_BOOT_DEVICE_RAM, aspeed_spl_ram_load_image);

static int aspeed_secboot_spl_ram_load_image(struct spl_image_info *spl_image,
				      struct spl_boot_device *bootdev)
{
	struct aspeed_secboot_header *sb_hdr =
		(struct aspeed_secboot_header *)CONFIG_ASPEED_UBOOT_DRAM_BASE - 1;

	memcpy(sb_hdr, (void *)(CONFIG_ASPEED_UBOOT_SPI_BASE), CONFIG_ASPEED_UBOOT_SPI_SIZE);
	if (aspeed_bl2_verify(sb_hdr, CONFIG_SPL_TEXT_BASE) != 0)
		return -EPERM;

	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_DRAM_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_DRAM_BASE;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("RAM with Aspeed Secure Boot", 0, ASPEED_SECBOOT_DEVICE_RAM, aspeed_secboot_spl_ram_load_image);

static int aspeed_spl_mmc_load_image(struct spl_image_info *spl_image,
				      struct spl_boot_device *bootdev)
{
	int err;
	u32 count;

	struct mmc *mmc = NULL;
	struct udevice *dev;
	struct blk_desc *bd;

	err = mmc_initialize(NULL);
	if (err) {
		printf("spl: could not initialize mmc. error: %d\n", err);
		return err;
	}

	err = uclass_get_device(UCLASS_MMC, 0, &dev);
	if (err) {
		printf("spl: mmc get device through DM with error: %d\n", err);
		return err;
	}

	mmc = mmc_get_mmc_dev(dev);
	if (mmc == NULL) {
		printf("spl: could not find mmc device\n");
		return -ENODEV;
	}

	err = mmc_init(mmc);
	if (err) {
		printf("spl: mmc init failed with error: %d\n", err);
		return err;
	}

	bd = mmc_get_blk_desc(mmc);

	count = blk_dread(bd, CONFIG_ASPEED_UBOOT_MMC_BASE, CONFIG_ASPEED_UBOOT_MMC_SIZE,
			(void *)CONFIG_ASPEED_UBOOT_DRAM_BASE);
	if (count != CONFIG_ASPEED_UBOOT_MMC_SIZE) {
		printf("spl: mmc raw sector read failed\n");
		return -EIO;
	}

	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_DRAM_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_DRAM_BASE;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("MMC", 0, ASPEED_BOOT_DEVICE_MMC, aspeed_spl_mmc_load_image);

static int aspeed_secboot_spl_mmc_load_image(struct spl_image_info *spl_image,
				      struct spl_boot_device *bootdev)
{
	int err;
	int part = CONFIG_ASPEED_UBOOT_MMC_PART;
	u32 count;

	struct mmc *mmc = NULL;
	struct udevice *dev;
	struct blk_desc *bd;

	struct aspeed_secboot_header *sb_hdr;

	err = mmc_initialize(NULL);
	if (err) {
		printf("spl: could not initialize mmc. error: %d\n", err);
		return err;
	}

	err = uclass_get_device(UCLASS_MMC, 0, &dev);
	if (err) {
		printf("spl: mmc get device through DM with error: %d\n", err);
		return err;
	}

	mmc = mmc_get_mmc_dev(dev);
	if (mmc == NULL) {
		printf("spl: could not find mmc device\n");
		return -ENODEV;
	}

	err = mmc_init(mmc);
	if (err) {
		printf("spl: mmc init failed with error: %d\n", err);
		return err;
	}

	bd = mmc_get_blk_desc(mmc);
	if (sizeof(*sb_hdr) != bd->blksz) {
		printf("spl: secure boot header size must equal to mmc block size\n");
		return -EINVAL;
	}

	if (part) {
		if (CONFIG_IS_ENABLED(MMC_TINY))
			err = mmc_switch_part(mmc, part);
		else
			err = blk_dselect_hwpart(bd, part);
	}

	sb_hdr = (struct aspeed_secboot_header *)CONFIG_ASPEED_UBOOT_DRAM_BASE - 1;
	count = blk_dread(bd, CONFIG_ASPEED_UBOOT_MMC_BASE, CONFIG_ASPEED_UBOOT_MMC_SIZE, sb_hdr);
	if (count != (CONFIG_ASPEED_UBOOT_MMC_SIZE)) {
		printf("spl: mmc raw sector read failed\n");
		return -EIO;
	}

	if (aspeed_bl2_verify(sb_hdr, CONFIG_SPL_TEXT_BASE) != 0)
		return -EPERM;

	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_DRAM_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_DRAM_BASE;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("MMC with Aspeed Secure Boot", 0, ASPEED_SECBOOT_DEVICE_MMC, aspeed_secboot_spl_mmc_load_image);

static int getcymodem(void)
{
	if (tstc())
		return (getc());
	return -1;
}

static int aspeed_spl_ymodem_load_image(struct spl_image_info *spl_image,
		struct spl_boot_device *bootdev)
{
	connection_info_t conn_info;
	int err;
	int res;
	int ret = 0;

	/*
	 * disable ABR WDT for eMMC and boot SPI, otherwise, image
	 * transmission will be interrupted during boot from UART.
	 */
	writel(0x0, 0x1e6f20a0);
	writel(0x0, 0x1e620064);

	conn_info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&conn_info, &err);
	if (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		return ret;
	}

	res = xyzModem_stream_read((char *)CONFIG_ASPEED_UBOOT_DRAM_BASE,
			CONFIG_ASPEED_UBOOT_UART_SIZE, &err);
	if (res <= 0) {
		ret = -EIO;
		goto end_stream;
	}

	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_DRAM_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_DRAM_BASE;

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	return ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, ASPEED_BOOT_DEVICE_UART, aspeed_spl_ymodem_load_image);

static int aspeed_secboot_spl_ymodem_load_image(struct spl_image_info *spl_image,
		struct spl_boot_device *bootdev)
{
	struct aspeed_secboot_header *sb_hdr;
	connection_info_t conn_info;
	ulong size = 0;
	int err;
	int res;
	int ret = 0;

	conn_info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&conn_info, &err);
	if (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		return ret;
	}

	sb_hdr = (struct aspeed_secboot_header *)CONFIG_ASPEED_UBOOT_DRAM_BASE - 1;
	res = xyzModem_stream_read((char *)sb_hdr, CONFIG_ASPEED_UBOOT_UART_SIZE, &err);
	if (res <= 0) {
		ret = -EIO;
		goto end_stream;
	}

	if (aspeed_bl2_verify(sb_hdr, CONFIG_SPL_TEXT_BASE) != 0)
		return -EPERM;

	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_image->entry_point = CONFIG_ASPEED_UBOOT_DRAM_BASE;
	spl_image->load_addr = CONFIG_ASPEED_UBOOT_DRAM_BASE;

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

	return ret;
}
SPL_LOAD_IMAGE_METHOD("UART with Aspeed Secure Boot", 0, ASPEED_SECBOOT_DEVICE_UART, aspeed_secboot_spl_ymodem_load_image);
