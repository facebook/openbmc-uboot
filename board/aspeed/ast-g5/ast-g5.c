/*
 * Copyright 2016 IBM Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <common.h>
#include <netdev.h>
#include <malloc.h>
#include <image.h>

#include <asm/arch/ast-sdk/ast_scu.h>
#include <asm/arch/ast-sdk/ast-sdmc.h>
#include <asm/arch/ast-sdk/vbs.h>
#include <asm/io.h>

#include "tpm-spl.h"
#include "util.h"

DECLARE_GLOBAL_DATA_PTR;

static void vboot_check_enforce(void)
{
  /* Clean the handoff marker from ROM. */
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  if (vbs->hardware_enforce) {
    /* If we are hardware-enforcing then this U-Boot is verified. */
    env_set("verify", "yes");
  }
}

static void vboot_finish(void)
{
  /* Clean the handoff marker from ROM. */
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  vbs->rom_handoff = 0x0;

#ifdef CONFIG_ASPEED_TPM
  ast_tpm_finish();
#endif
}

char* fit_cert_store(void)
{
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  return (char*)(vbs->subordinate_keys);
}

void arch_preboot_os(void) {
  vboot_finish();
}

#if defined(CONFIG_FBTP) || defined(CONFIG_PWNEPTUNE)
static void fan_init(void)
{
  __raw_writel(0x43004300, 0x1e786008);
}

static int get_svr_pwr(void)
{
  // GPIOB6
  return ((__raw_readl(AST_GPIO_BASE + 0x00) >> 14) & 1);
}

static void set_svr_pwr_btn(int level)
{
  u32 reg;
  // GPIOE3
  // output
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  __raw_writel(reg | (1 << 3), AST_GPIO_BASE + 0x24);

  reg = __raw_readl(AST_GPIO_BASE + 0x20);
  if (level) //high
    reg |= (1<<3);
  else // low
    reg &= ~(1<<3);
  __raw_writel(reg, AST_GPIO_BASE + 0x20);
}

static void policy_init(void)
{
  u32 reg;
  char *policy = NULL;
  char *last_state = NULL;
  char *result;
  int to_pwr_on = 0;

  // BMC's SCU3C: System Reset Control/Status Register
  reg = __raw_readl(AST_SCU_BASE + 0x3c);
  // Power on reset flag(SCU3C[0])
  // POR flag bit will be cleared at Linux init
  if (reg & 0x1) {
    // getenv return the same buffer,
    // duplicate result before call it again.
    result = env_get("por_policy");
    policy = (result) ? strdup(result) : NULL;
    // printf("%X por_policy:%s\n", policy, policy?policy:"null");

    result = env_get("por_ls");
    last_state = (result) ? strdup(result) : NULL;
    // printf("%X por_ls:%s\n", last_state, last_state?last_state:"null");

    if (policy && last_state){
      if ((!strcmp(policy, "on")) ||
          (!strcmp(policy, "lps") && !strcmp(last_state, "on"))
      ) {
        to_pwr_on = 1;
      }
    } else {
      // default power on if no por config
      to_pwr_on = 1;
    }
  }
  printf("to_pwr_on: %d, policy:%s, ls:%s, scu3c:%08X\n",
    to_pwr_on,
    policy ? policy : "null",
    last_state ? last_state : "null",
    reg);

  // Host Server should power on
  if (to_pwr_on == 1) {
    // Host Server is not on
    if (!get_svr_pwr()) {
      set_svr_pwr_btn(0);
      udelay(1000*1000);
      set_svr_pwr_btn(1);
      udelay(1000*1000);
      if (!get_svr_pwr())
        printf("!!!! Power On failed !!!!\n");
    }
  }

  // free duplicated string buffer
  if (policy)
    free(policy);
  if (last_state)
    free(last_state);
}

static void disable_bios_debug(void)
{
  u32 reg;
  // Set GPIOD0's direction as Output
  reg = __raw_readl(AST_GPIO_BASE + 0x4);
  __raw_writel(reg | (1 << 24), AST_GPIO_BASE + 0x4);

  // Set GPIOD0's value as HIGH
  reg = __raw_readl(AST_GPIO_BASE + 0x0);
  reg |= (1<<24);
  __raw_writel(reg, AST_GPIO_BASE + 0x0);
}

static int disable_snoop_dma_interrupt(void)
{
  // Disable interrupt which will not be clearred by wdt reset
  // to avoid interrupts triggered before linux kernel can handle it.
  // PCCR0: Post Code Control Register 0
#ifdef DEBUG
  printf("pccr0: %08X\n", __raw_readl(AST_LPC_BASE + 0x130));
#endif
  __raw_writel(0x0, AST_LPC_BASE + 0x130);

  return 0;
}

#endif

#ifdef CONFIG_FBY2
static void fan_init(void)
{
  u32 reg;

  // enable PWM0 and PWM1 function pin
  reg = __raw_readl(AST_SCU_BASE + 0x88);
  reg |= 0x03;
  __raw_writel(reg, AST_SCU_BASE + 0x88);

  reg = __raw_readl(AST_SCU_BASE + 0x04);
  reg &= ~0x200;
  __raw_writel(reg, AST_SCU_BASE + 0x04);

  // set PWM0 and PWM1 to 70%
  __raw_writel(0x09435F05, AST_PWM_BASE + 0x04);
  __raw_writel(0x43004300, AST_PWM_BASE + 0x08);
  __raw_writel(0x00000301, AST_PWM_BASE + 0x00);
}

static int mux_init(void)
{
  u8 loc;
  u32 reg, pwr_en;

  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  loc = ((reg >> 20) & 0xF) % 5;

  // USB MUX, P3V3 enable
  // enable GPIOAB3,AB1 WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x18C);
  reg |= 0xA000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x18C);
  // set USB MUX, P3V3 enable
  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  if (loc < MAX_NODES) {
    pwr_en = __raw_readl(AST_GPIO_BASE + 0x70);
    reg = (reg & ~0x8000000) | ((pwr_en & (1 << loc)) ? 0x2000000 : 0xA000000);
  } else {
    reg |= 0xA000000;
  }
  __raw_writel(reg, AST_GPIO_BASE + 0x1E0);
  // set GPIOAB3,AB1 as output
  reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  reg |= 0xA000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E4);

  // USB MUX
  // enable GPIOE[5:4] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x3C);
  reg |= 0x30;
  __raw_writel(reg, AST_GPIO_BASE + 0x3C);
  // set USB MUX location
  reg = __raw_readl(AST_GPIO_BASE + 0x20);
  if (loc < MAX_NODES) {
    reg = (reg & ~0x30) | (loc << 4);
  }
  __raw_writel(reg, AST_GPIO_BASE + 0x20);
  // set GPIOE[5:4] as output
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  reg |= 0x30;
  __raw_writel(reg, AST_GPIO_BASE + 0x24);

  // VGA MUX
  // enable GPIOJ[3:0] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0xAC);
  reg |= 0xF00;
  __raw_writel(reg, AST_GPIO_BASE + 0xAC);
  // set VGA MUX location
  reg = __raw_readl(AST_GPIO_BASE + 0x70);
  if (loc < MAX_NODES) {
    reg = (reg & ~0xC00);
  }
  __raw_writel(reg, AST_GPIO_BASE + 0x70);
  // set GPIOJ[3:0] as output
  reg = __raw_readl(AST_GPIO_BASE + 0x74);
  reg |= 0xF00;
  __raw_writel(reg, AST_GPIO_BASE + 0x74);

  // PCIe Clk/Rst buffer
  // enable GPIOB[7:4] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x1C);
  reg |= 0xF000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1C);
  // set PCIe Clk/Rst buffer
  reg = __raw_readl(AST_GPIO_BASE + 0x00);
  __raw_writel(reg, AST_GPIO_BASE + 0x00);
  // set GPIOB[7:4] as output
  reg = __raw_readl(AST_GPIO_BASE + 0x04);
  reg |= 0xF000;
  __raw_writel(reg, AST_GPIO_BASE + 0x04);

  return 0;
}

static int slot_12V_init(void)
{
  u32 slot_present_reg;
  u32 slot_12v_reg;
  u32 dir_reg;
  u32 toler_reg;
  uint8_t val_prim;
  uint8_t val_ext;
  int i;

  //Read GPIOZ0~Z3 and AA0~AA3
  slot_present_reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  //Read GPIOO4~O7
  slot_12v_reg = __raw_readl(AST_GPIO_BASE + 0x078);

  for (i = 0; i < MAX_NODES; i++) {
    val_ext = (slot_present_reg >> (2*MAX_NODES+i)) & 0x1;
    val_prim = (slot_present_reg >> (4*MAX_NODES+i)) & 0x1;

    if (val_prim || val_ext) {
      slot_12v_reg &= ~(1<<(5*MAX_NODES+i));
    } else {
      slot_12v_reg |= (1<<(5*MAX_NODES+i));
    }
  }

  //Set GPIOO4~O7 Watchdog reset tolerance
  toler_reg = __raw_readl(AST_GPIO_BASE + 0x0FC);
  toler_reg |= 0xF00000;
  __raw_writel(toler_reg, AST_GPIO_BASE + 0x0FC);

  //Configure GPIOO4~O7
  __raw_writel(slot_12v_reg, AST_GPIO_BASE + 0x078);
  dir_reg = __raw_readl(AST_GPIO_BASE + 0x07C);
  dir_reg |= 0xF00000;
  __raw_writel(dir_reg, AST_GPIO_BASE + 0x07C);
  __raw_writel(slot_12v_reg, AST_GPIO_BASE + 0x078);

  return 0;
}

static int slot_led_init(void)
{
  u32 reg, dir_reg;
  u32 fan_latch;
  u32 slot_present_reg;
  uint8_t val_prim;
  uint8_t val_ext;
  int i;

  // enable GPIOAC0~AC3, and AC7
  reg = __raw_readl(AST_SCU_BASE + 0xAC);
  reg &= ~0x8F;
  __raw_writel(reg, AST_SCU_BASE + 0xAC);

  // read GPIOH5
  fan_latch = __raw_readl(AST_GPIO_BASE + 0x020) & 0x20000000;

  //Read GPIOZ0~Z3 and AA0~AA3
  slot_present_reg = __raw_readl(AST_GPIO_BASE + 0x1E0);

  // configure GPIOAC0~AC3, and AC7
  reg = __raw_readl(AST_GPIO_BASE + 0x1E8);

  for (i = 0; i < MAX_NODES; i++) {
    val_ext = (slot_present_reg >> (2*MAX_NODES+i)) & 0x1;
    val_prim = (slot_present_reg >> (4*MAX_NODES+i)) & 0x1;

    if (val_prim || val_ext)
      reg &= ~(1 << i);
    else
      reg |= (1 << i);
  }

  reg = (fan_latch) ? (reg | 0x80) : (reg & ~0x8F);

  dir_reg = __raw_readl(AST_GPIO_BASE + 0x1EC) | 0x8F;
  __raw_writel(dir_reg, AST_GPIO_BASE + 0x1EC);
  __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

  return 0;
}
#endif


#ifdef CONFIG_MINILAKETB
static void fan_init(void)
{
  u32 reg;

  // enable PWM0 and PWM1 function pin
  reg = __raw_readl(AST_SCU_BASE + 0x88);
  reg |= 0x03;
  __raw_writel(reg, AST_SCU_BASE + 0x88);

  reg = __raw_readl(AST_SCU_BASE + 0x04);
  reg &= ~0x200;
  __raw_writel(reg, AST_SCU_BASE + 0x04);

  // set PWM0 and PWM1 to 70%
  __raw_writel(0x09435F05, AST_PWM_BASE + 0x04);
  __raw_writel(0x43004300, AST_PWM_BASE + 0x08);
  __raw_writel(0x00000301, AST_PWM_BASE + 0x00);
}

static int mux_init(void)
{
  u8 loc;
  u32 reg;

  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  loc = ((reg >> 20) & 0x0F) % 5;

  // USB MUX, P3V3 enable
  // enable GPIOAB3,AB1 WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x18C);
  reg |= 0xA000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x18C);
  // set USB MUX, P3V3 enable
  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  reg = (reg & ~0xA000000) | (loc < MAX_NODES)?0x2000000:0xA000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E0);
  // set GPIOAB3,AB1 as output
  reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  reg |= 0xA000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E4);

  /*minilaketb : use GPIOE[4], GPIOF[1] as input gpio*/
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  reg &= ~0x210;
  __raw_writel(reg, AST_GPIO_BASE + 0x24);

  // USB MUX
  // enable GPIOE[5:4] WDT reset tolerance
  // reg = __raw_readl(AST_GPIO_BASE + 0x3C);
  // reg |= 0x30;
  // __raw_writel(reg, AST_GPIO_BASE + 0x3C);
  // set USB MUX location
  // reg = __raw_readl(AST_GPIO_BASE + 0x20);
  // if (loc < MAX_NODES) {
  //   reg = (reg & ~0x30) | (loc << 4);
  // }
  // __raw_writel(reg, AST_GPIO_BASE + 0x20);
  // set GPIOE[5:4] as output
  // reg = __raw_readl(AST_GPIO_BASE + 0x24);
  // reg |= 0x30;
  // __raw_writel(reg, AST_GPIO_BASE + 0x24);

  // VGA MUX
  // enable GPIOJ[3:0] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0xAC);
  reg |= 0xF00;
  __raw_writel(reg, AST_GPIO_BASE + 0xAC);
  // set VGA MUX location
  reg = __raw_readl(AST_GPIO_BASE + 0x70);
  if (loc < MAX_NODES) {
    reg = (reg & ~0xC00) | (loc << 10);
  }
  __raw_writel(reg, AST_GPIO_BASE + 0x70);
  // set GPIOJ[3:0] as output
  reg = __raw_readl(AST_GPIO_BASE + 0x74);
  reg |= 0xF00;
  __raw_writel(reg, AST_GPIO_BASE + 0x74);

  // PCIe Clk/Rst buffer
  // enable GPIOB[7:4] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x1C);
  reg |= 0xF000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1C);
  // set PCIe Clk/Rst buffer
  reg = __raw_readl(AST_GPIO_BASE + 0x00);
  __raw_writel(reg, AST_GPIO_BASE + 0x00);
  // set GPIOB[7:4] as output
  reg = __raw_readl(AST_GPIO_BASE + 0x04);
  reg |= 0xF000;
  __raw_writel(reg, AST_GPIO_BASE + 0x04);

  // BIOS mux
  // enable GPIOM[5:3:2] WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x0FC);
  reg |= 0x2C;
  __raw_writel(reg, AST_GPIO_BASE + 0x0FC);

  reg = __raw_readl(AST_GPIO_BASE + 0x7C);
  reg &= 0x0C;
  if (reg != 0x0C){
      //set GPIOM[3:2] to output
      reg = __raw_readl(AST_GPIO_BASE + 0x7C);
      reg |= 0x0C;
      __raw_writel(reg, AST_GPIO_BASE + 0x7C);
  }

  //change GPIOM5:P12V_EN to output
  reg = __raw_readl(AST_GPIO_BASE + 0x7C);
  reg &= 0x20;
  if (reg != 0x20){
      reg = __raw_readl(AST_GPIO_BASE + 0x7C);
      reg |= 0x20;
      __raw_writel(reg, AST_GPIO_BASE + 0x7C);
  }

  return 0;
}

static int slot_12V_init(void)
{
  u32 slot_present_reg;
  u32 slot_12v_reg;
  u32 dir_reg;
  u32 toler_reg;
  uint8_t val_prim;
  uint8_t val_ext;
  int i;

  //Read GPIOZ0~Z3 and AA0~AA3
  slot_present_reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  //Read GPIOO4~O7
  slot_12v_reg = __raw_readl(AST_GPIO_BASE + 0x078);

  for (i = 0; i < MAX_NODES; i++) {
    val_ext = (slot_present_reg >> (2*MAX_NODES+i)) & 0x1;
    val_prim = (slot_present_reg >> (4*MAX_NODES+i)) & 0x1;

    if (val_prim || val_ext) {
      slot_12v_reg &= ~(1<<(5*MAX_NODES+i));
    } else {
      slot_12v_reg |= (1<<(5*MAX_NODES+i));
    }
  }

  //Set GPIOO4~O7 Watchdog reset tolerance
  toler_reg = __raw_readl(AST_GPIO_BASE + 0x0FC);
  toler_reg |= 0xF00000;
  __raw_writel(toler_reg, AST_GPIO_BASE + 0x0FC);

  //Configure GPIOO4~O7
  __raw_writel(slot_12v_reg, AST_GPIO_BASE + 0x078);
  dir_reg = __raw_readl(AST_GPIO_BASE + 0x07C);
  dir_reg |= 0xF00000;
  __raw_writel(dir_reg, AST_GPIO_BASE + 0x07C);
  __raw_writel(slot_12v_reg, AST_GPIO_BASE + 0x078);


  /*Copy GPIOO4 behavior to GPIOZ0*/
  // read GPIOZ0 value
  slot_12v_reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  slot_12v_reg &= ~0x100; //GPIOZ0 value low , XG1 P12V_STBY_MB_EN_N is low enable
  __raw_writel(slot_12v_reg, AST_GPIO_BASE + 0x1E0);

  // read GPIOZ0 direction
  dir_reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  dir_reg |= 0x100; //GPIOZ0  output direction
  __raw_writel(dir_reg, AST_GPIO_BASE + 0x1E4);

  // set GPIOZ0 Watchdog reset tolerance
  toler_reg = __raw_readl(AST_GPIO_BASE + 0x18C);
  toler_reg |= 0x100;
  __raw_writel(toler_reg, AST_GPIO_BASE + 0x18C);

  return 0;
}

static int slot_led_init(void)
{
  u32 reg;
  //u32 fan_latch;
  // u32 slot_present_reg;
  // uint8_t val_prim;
  // uint8_t val_ext;
  // int i;

  // disable GPIOAC0-7
  reg = __raw_readl(AST_SCU_BASE + 0xAC);
  reg &= ~0xFF;
  __raw_writel(reg, AST_SCU_BASE + 0xAC);

  // // read GPIOH5
  // fan_latch = __raw_readl(AST_GPIO_BASE + 0x020) & 0x20000000;
  //
  // //Read GPIOZ0~Z3 and AA0~AA3
  // slot_present_reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  //
  // // configure GPIOAC0~AC3, and AC7
  // reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
  //
  // for (i = 0; i < MAX_NODES; i++) {
  //   val_ext = (slot_present_reg >> (2*MAX_NODES+i)) & 0x1;
  //   val_prim = (slot_present_reg >> (4*MAX_NODES+i)) & 0x1;
  //
  //   if (val_prim || val_ext)
  //     reg &= ~(1 << i);
  //   else
  //     reg |= (1 << i);
  // }
  //
  // reg = (fan_latch) ? (reg | 0x80) : (reg & ~0x8F);
  //
  // dir_reg = __raw_readl(AST_GPIO_BASE + 0x1EC) | 0x8F;
  // __raw_writel(dir_reg, AST_GPIO_BASE + 0x1EC);
  // __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

  return 0;
}

static int spi_init(void)
{
    u32 reg;
    // enable SPI2 function pin
    reg = __raw_readl(AST_SCU_BASE + 0x88);
    reg |= 0x3C000000;
    __raw_writel(reg, AST_SCU_BASE + 0x88);
    return 0;
}
#endif


#if defined(CONFIG_FBAL) || defined(CONFIG_FBSP) || defined(CONFIG_FBEP) || defined(CONFIG_FBY3)
static void fix_mmc_hold_time_fail(void)
{
  u32 reg;

  reg = __raw_readl(AST_SCU_BASE + 0x08);
  reg &= ~(7 << 12);
  reg |= (0x01 << 12);
  __raw_writel(reg, AST_SCU_BASE + 0x08);
}
#endif

#if defined(CONFIG_FBAL) || defined(CONFIG_FBSP)
static void disable_snoop_interrupt(void)
{
  u32 reg;

  reg = __raw_readl(AST_LPC_BASE + 0x80);
  reg &= ~0xA;
  __raw_writel(reg, AST_LPC_BASE + 0x80);

  __raw_writel(0x0, AST_LPC_BASE + 0x130);
}
#endif

#if defined(CONFIG_FBAL)
static void enable_nic_mux(void)
{
  u32 reg;

  reg = __raw_readl(AST_SCU_BASE + 0xA0);
  reg |= (1 << 4);
  __raw_writel(reg, AST_SCU_BASE + 0xA0);

  // enable GPIOT4 WDT reset tolerance
  reg = __raw_readl(AST_GPIO_BASE + 0x12C);
  reg |= 0x10000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x12C);

  reg = __raw_readl(AST_GPIO_BASE + 0x84);
  reg |= 0x10000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x84);

  reg = __raw_readl(AST_GPIO_BASE + 0x80);
  reg |= 0x10000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x80); 
}

static int get_fbal_pwrok(void)
{
  // GPIOY2
  return ((__raw_readl(AST_GPIO_BASE + 0x1E0) >> 2) & 1);
}

static void set_fbal_pwrbtn(int level)
{
  u32 reg;
  // GPIOE3
  // output
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  __raw_writel(reg | (1 << 3), AST_GPIO_BASE + 0x24);

  reg = __raw_readl(AST_GPIO_BASE + 0x20);
  if (level) //high
    reg |= (1<<3);
  else // low
    reg &= ~(1<<3);
  __raw_writel(reg, AST_GPIO_BASE + 0x20);
}

static void policy_init(void)
{
  u32 reg;
  char *policy = NULL;
  char *last_state = NULL;
  char *result;
  int to_pwr_on = 0;

  // BMC's SCU3C: System Reset Control/Status Register
  reg = __raw_readl(AST_SCU_BASE + 0x3c);
  // Power on reset flag(SCU3C[0])
  // POR flag bit will be cleared at Linux init
  if (reg & 0x1) {
    // getenv return the same buffer,
    // duplicate result before call it again.
    result = env_get("por_policy");
    policy = (result) ? strdup(result) : NULL;
    // printf("%X por_policy:%s\n", policy, policy?policy:"null");

    result = env_get("por_ls");
    last_state = (result) ? strdup(result) : NULL;
    // printf("%X por_ls:%s\n", last_state, last_state?last_state:"null");

    if (policy && last_state){
      if ((!strcmp(policy, "on")) ||
          (!strcmp(policy, "lps") && !strcmp(last_state, "on"))
      ) {
        to_pwr_on = 1;
      }
    } else {
      // default power on if no por config
      to_pwr_on = 1;
    }
  }
  printf("to_pwr_on: %d, policy:%s, ls:%s, scu3c:%08X\n",
    to_pwr_on,
    policy ? policy : "null",
    last_state ? last_state : "null",
    reg);

  // Host Server should power on
  if (to_pwr_on == 1) {
    // Host Server is not on
    if (!get_fbal_pwrok()) {
      set_fbal_pwrbtn(0);
      udelay(1000*1000);
      set_fbal_pwrbtn(1);
      udelay(1000*1000);
      if (!get_fbal_pwrok())
        printf("!!!! Power On failed !!!!\n");
    }
  }

  // free duplicated string buffer
  if (policy)
    free(policy);
  if (last_state)
    free(last_state);
}
#endif

#ifdef CONFIG_FBSP
static void fan_init(void)
{
  u32 reg;

  // enable PWM0 and PWM1 function pin
  reg = __raw_readl(AST_SCU_BASE + 0x88);
  reg |= 0x03;
  __raw_writel(reg, AST_SCU_BASE + 0x88);

  reg = __raw_readl(AST_SCU_BASE + 0x04);
  reg &= ~0x200;
  __raw_writel(reg, AST_SCU_BASE + 0x04);

  // set PWM0 and PWM1 to 70%
  __raw_writel(0x09435F05, AST_PWM_BASE + 0x04);
  __raw_writel(0x43004300, AST_PWM_BASE + 0x08);
  __raw_writel(0x00000301, AST_PWM_BASE + 0x00);
}

static int get_fbsp_pwrok(void)
{
  // GPIOY2
  return ((__raw_readl(AST_GPIO_BASE + 0x1E0) >> 2) & 1);
}

static void set_fbsp_pwrbtn(int level)
{
  u32 reg;
  // GPIOE3
  // output
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  __raw_writel(reg | (1 << 3), AST_GPIO_BASE + 0x24);

  reg = __raw_readl(AST_GPIO_BASE + 0x20);
  if (level) //high
    reg |= (1<<3);
  else // low
    reg &= ~(1<<3);
  __raw_writel(reg, AST_GPIO_BASE + 0x20);
}

static void policy_init(void)
{
  u32 reg;
  char *policy = NULL;
  char *last_state = NULL;
  char *result;
  int to_pwr_on = 0;

  // BMC's SCU3C: System Reset Control/Status Register
  reg = __raw_readl(AST_SCU_BASE + 0x3c);
  // Power on reset flag(SCU3C[0])
  // POR flag bit will be cleared at Linux init
  if (reg & 0x1) {
    // getenv return the same buffer,
    // duplicate result before call it again.
    result = env_get("por_policy");
    policy = (result) ? strdup(result) : NULL;

    result = env_get("por_ls");
    last_state = (result) ? strdup(result) : NULL;

    if (policy && last_state){
      if ((!strcmp(policy, "on")) ||
          (!strcmp(policy, "lps") && !strcmp(last_state, "on"))
      ) {
        to_pwr_on = 1;
      }
    } else {
      // default power on if no por config
      to_pwr_on = 1;
    }
  }
  printf("to_pwr_on: %d, policy:%s, ls:%s, scu3c:%08X\n",
    to_pwr_on,
    policy ? policy : "null",
    last_state ? last_state : "null",
    reg);

  // Host Server should power on
  if (to_pwr_on == 1) {
    // Host Server is not on
    if (!get_fbsp_pwrok()) {
      set_fbsp_pwrbtn(0);
      udelay(1000*1000);
      set_fbsp_pwrbtn(1);
      udelay(1000*1000);
      if (!get_fbsp_pwrok())
        printf("!!!! Power On failed !!!!\n");
    }
  }

  // free duplicated string buffer
  if (policy)
    free(policy);
  if (last_state)
    free(last_state);
}

//disable GPIOE pass-through
static void disable_gpioe_pass_through(void) {
  u32 reg;
  reg = __raw_readl(AST_SCU_BASE + 0x70);
  reg &= 0xFFBFFFFF;
  __raw_writel(reg, AST_SCU_BASE + 0x70);
}
#endif

#if defined(CONFIG_FBEP)
static void spi_init(void)
{
  // Disable SPI interface of SPI1
  __raw_writel((0x3 << 12), 0x1e6e207c);
}

static void led_init(void)
{
  u32 reg;
  // ID_LED (GPIOM1) output-high
  reg = __raw_readl(AST_GPIO_BASE + 0x78);
  reg |= (0x1 << 1);
  __raw_writel(reg, AST_GPIO_BASE + 0x78);
  reg = __raw_readl(AST_GPIO_BASE + 0x7C);
  reg |= (0x1 << 1);
  __raw_writel(reg, AST_GPIO_BASE + 0x7C);
}
#endif

int board_init(void)
{
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
	vboot_check_enforce();

#if defined(CONFIG_FBTP) || defined(CONFIG_PWNEPTUNE)
  fan_init();
  policy_init();
  disable_bios_debug();
  disable_snoop_dma_interrupt();
#endif

#ifdef CONFIG_FBY2
  fan_init();
  mux_init();
  slot_12V_init();
  slot_led_init();
#endif

#ifdef CONFIG_MINILAKETB
  fan_init();
  mux_init();
  slot_12V_init();
  slot_led_init();
  spi_init();
#endif

#if defined(CONFIG_FBAL)
  policy_init();
  disable_snoop_interrupt();
  enable_nic_mux();
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBSP)
  fan_init();
  policy_init();
  disable_gpioe_pass_through();
  disable_snoop_interrupt();
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBEP)
  spi_init();
  led_init();
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBY3)
  fix_mmc_hold_time_fail();
#endif

  gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

  return 0;
}

int dram_init(void)
{
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = dram - vga;
#ifdef CONFIG_DRAM_ECC
	gd->ram_size -= gd->ram_size >> 3; /* need 1/8 for ECC */
#endif
	return 0;
}

#ifdef CONFIG_FTGMAC100
int board_eth_init(bd_t *bd)
{
  return ftgmac100_initialize(bd);
}
#endif

#ifdef CONFIG_ASPEEDNIC
int board_eth_init(bd_t *bd)
{
  return aspeednic_initialize(bd);
}
#endif
