/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>

/**
 * ast_fmc_spi_reset() - Reset AST2500 FMC SPI chip select 1 status.
 *
 * For the SPL boot the second flash (SPI0.1) CS1 in the FMC "controller" must
 * be reset. The FMC does not auto-detect and set the CS1 "status register"
 * based on the mode of the part, like it does for SPI0.0.
 *
 * Expect the following defines:
 *   AST_FMC_CS1_BASE - the MMIO location of SPI0.1 (FMC CS1).
 *   AST_FMC_BASE     - the MMIO base of the FMC register group.
 *
 */
void ast_fmc_spi_cs1_reset(void);