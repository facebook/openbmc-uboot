// SPDX-License-Identifier: GPL-2.0
// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/err.h>

static int ast_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *wdt;
	int ret = uclass_first_device(UCLASS_WDT, &wdt);

	if (ret)
		return ret;

	ret = wdt_expire_now(wdt, 0);
	if (ret) {
		debug("Sysreset failed: %d", ret);
		return ret;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops ast_sysreset = {
	.request	= ast_sysreset_request,
};

U_BOOT_DRIVER(sysreset_ast) = {
	.name	= "ast_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &ast_sysreset,
};
