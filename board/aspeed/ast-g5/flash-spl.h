/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */


#define AST_FMC_WP_ON    0
#define AST_FMC_WP_OFF   2
#define AST_FMC_ERROR    1

/**
 * ast_fmc_spi_check() - Reset AST2500 FMC SPI chip select 1 status.
 *
 * For the SPL boot the second flash (SPI0.1) CS1 in the FMC "controller" must
 * be reset. The FMC does not auto-detect and set the CS1 "status register"
 * based on the mode of the part, like it does for SPI0.0.
 *
 * should_lock - If hardware enforcement is enabled this instructs
 * the spi flashes to be locked.
 *
 * This will return 0 on success (CS1 was reset and CS0 is protected).
 * This will return 1 if CS0 cannot set protections (critical error).
 * This will return 2 if CS0 is not pulling WP# active low.
 *
 * Expect the following defines:
 *   AST_FMC_CS1_BASE - the MMIO location of SPI0.1 (FMC CS1).
 *   AST_FMC_BASE     - the MMIO base of the FMC register group.
 *
 */
int ast_fmc_spi_check(bool should_lock);
