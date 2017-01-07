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
#define AST_FMC_CE0_CONTROL  0x10
#define AST_FMC_CE_CONTROL   0x04

#define SPI_CMD_3B 0xe9
#define SPI_CMD_4B 0xb7
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
  udelay(200);
}

inline void spi_high(u32 ctrl) {
  write_reg(AST_FMC_BASE + ctrl, SPI_CK_USERMODE_HIGH);
  udelay(200);
}

inline void spi_cmd(u32 base, uchar cmd) {
  write_byte(base, cmd);
}

inline void spi_enable_write(u32 base, u32 ctrl) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_WE);
  udelay(10);
  spi_high(ctrl);
}

inline int spi_wel(u32 base, u32 ctrl) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_RS);
  udelay(10);

  uchar ret = 1;
  u32 timeout = 8000;
  uchar r1;
  do {
    r1 = read_byte(base);
    if (--timeout == 0) {
      ret = 0;
      break;
    }
  } while (r1 == 0xff || !(r1 & 0x02));

  spi_high(ctrl);
  return ret;
}

inline int spi_wip(u32 base, u32 ctrl) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_RS);
  udelay(10);

  uchar r1;
  uchar ret = 1;
  u32 timeout = 8000;
  do {
    r1 = read_byte(base);
    if (--timeout == 0) {
      ret = 0;
      break;
    }
  } while (r1 == 0xff || (r1 & 0x01));

  spi_high(ctrl);
  return ret;
}

inline void spi_id(u32 base, u32 ctrl) {
  char id[3];

  spi_low(ctrl);
  spi_cmd(base, 0x9f);
  udelay(10);

  id[0] = read_byte(base);
  id[1] = read_byte(base);
  id[2] = read_byte(base);
  spi_high(ctrl);

  debug("AST FMC SPI: 0x%08x ID %02x%02x%02x\n", base, id[0], id[1], id[2]);
}

inline void spi_write_status(u32 base, u32 ctrl, uchar cmd) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_WS);
  udelay(10);
  spi_cmd(base, cmd);
  spi_high(ctrl);
}

inline void spi_enable4b(u32 base, u32 ctrl) {
  spi_low(ctrl);
  spi_cmd(base, SPI_CMD_4B);
  spi_high(ctrl);
}

inline void fmc_enable_write(void) {
  /* If FMC00:{16, 17, 18} is 0 then it needs to be enabled with FMCA4. */
  /* Set FMCA4 to |= 2AA */
  write_reg(AST_FMC_BASE, read_reg(AST_FMC_BASE) | AST_FMC_WRITE_ENABLE);
}

inline void fmc_reset(u32 ctrl) {
  write_reg(AST_FMC_BASE + ctrl, AST_FMC_STATUS_RESET);
}

inline void scu_multifunction(void) {
  write_reg(AST_SCU_BASE + 0x88, read_reg(AST_SCU_BASE + 0x88) | 0x03000000);
}

inline void scu_enable4b(void) {
  write_reg(AST_SCU_BASE + 0x70, read_reg(AST_SCU_BASE + 0x70) | 0x10);
  write_reg(AST_FMC_BASE + AST_FMC_CE_CONTROL,
    read_reg(AST_FMC_BASE + AST_FMC_CE_CONTROL) | (0x01 << 1));
}

int ast_fmc_spi_cs1_reset(void) {
  timer_init();

  fmc_enable_write();
  udelay(100);

  scu_multifunction();
  spi_id(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL);
  udelay(100);

  /* Enable register changes for the FMC, then send write-enable to the SPI. */
  scu_multifunction();
  spi_enable_write(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL);
  if (!spi_wel(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL)) {
    debug("AST FMC CE1: Did not enable WEL (1)\n");
  }

  /* Write and clear the status register for the SPI chip. */
  spi_write_status(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL, 0x00);
  if (!spi_wip(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL)) {
    debug("AST FMC CE1: Did not exit WIP (0)\n");
    return 1;
  }

  scu_multifunction();
  scu_enable4b();
  spi_enable4b(AST_FMC_CS1_BASE, AST_FMC_CE1_CONTROL);
  udelay(100);

  /* Reset the FMC state. */
  fmc_reset(AST_FMC_CE1_CONTROL);
  udelay(100);

  return 1;
}
