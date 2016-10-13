/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/arch/ast_g5_platform.h>

#include "flash-spl.h"

#define SPI_CK_USERMODE_LOW  0x00000603
#define SPI_CK_USERMODE_HIGH 0x00000607

#define AST_FMC_WRITE_ENABLE 0x800f0000
#define AST_FMC_STATUS_RESET 0x000b0641
#define AST_FMC_CE1_CONTROL  0x14

#define SPI_CMD_3B 0xe9
#define SPI_CMD_WE 0x06
#define SPI_CMD_RS 0x05
#define SPI_CMD_WS 0x01

inline u32 read_reg(u32 reg) {
  return *(volatile u32*)reg;
}

inline uchar read_byte(u32 reg) {
  return *(volatile uchar*)reg;
}

inline void write_reg(u32 reg, u32 val) {
  *((volatile u32*) reg) = val;
}

inline void write_byte(u32 reg, uchar val) {
  *((volatile uchar*) reg) = (uchar)val;
}

inline void spi_low(u32 ctrl) {
  write_reg(AST_FMC_BASE + ctrl, SPI_CK_USERMODE_LOW);
}

inline void spi_high(u32 ctrl) {
  write_reg(AST_FMC_BASE + ctrl, SPI_CK_USERMODE_HIGH);
}

inline void spi_cmd(u32 base, uchar cmd) {
  write_byte(base, cmd);
}

inline void spi_enable_write(u32 base, u32 ctrl) {
  write_reg(AST_FMC_BASE, read_reg(AST_FMC_BASE) | AST_FMC_WRITE_ENABLE);
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_WE);
  spi_high(ctrl);
}

inline void spi_reset(u32 ctrl) {
  write_reg(AST_FMC_BASE + ctrl, AST_FMC_STATUS_RESET);
}

inline int spi_status(u32 base, u32 ctrl, uchar mask, bool invert) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_RS);
  uchar r1; /* Only need to read_byte(AST_FMC_CS1_BASE); */
  u32 timeout = 1000;
  do {
    r1 = read_byte(base);
    if (--timeout == 0) {
      return 0;
    }
  } while ((invert && !(r1 & mask)) || (!invert && (r1 & mask)));
  spi_high(ctrl);
  return 1;
}

inline void spi_write_status(u32 base, u32 ctrl, uchar cmd) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_WS);
  spi_cmd(base, cmd);
  spi_high(ctrl);
}

inline void spi_enable_3b(u32 base, u32 ctrl) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_3B);
  spi_high(ctrl);
}

int ast_fmc_spi_cs1_reset(void) {
  /* Enable register changes for the FMC, then send write-enable to the SPI. */
  spi_enable_write(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL);
  if (!spi_status(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL, 0x02, true)) {
    return 0;
  }

  /* Write and clear the status register for the SPI chip. */
  spi_write_status(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL, 0x00);
  if (!spi_status(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL, 0x01, false)) {
    return 0;
  }

  /* Set the SPI chip to use 3-byte addressing mode (default). */
  spi_enable_3b(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL);

  /* Reset the FMC state. */
  spi_reset(AST_FMC_CE1_CONTROL);
  return 1;
}
