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
#include <asm/arch/vbs.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>

#include "tpm-spl.h"
#include "util.h"
#include <i2c.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

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

#ifdef CONFIG_FBY3
static int slot_12V_init(void)
{
  int ret = -1;
  int retry = 2;
  int i = 0;
  int node = 0;
  int board_id = 0;
  int slots_present_status = 0;
  uint8_t present = 0;
  struct udevice *bus, *dev;
  struct gpio_desc desc[4];
  // struct gpio_desc desctest[4];

  node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "board_id");
  if (node < 0) {
    return -1;
  }
  ret = gpio_request_list_by_name_nodev(offset_to_ofnode(node),
                "board-id-gpios", desc,
                ARRAY_SIZE(desc), GPIOD_IS_IN);
  if (ret < 0) {
    return ret;
  }
  board_id = dm_gpio_get_values_as_int(desc, ret);
  debug("board_id = %x\n", board_id);

  // if config C, do nothing
  // GPIOF[0:3] = BOARD_ID[3:0] = 1001 -------> NIC Expansion Card
  if (board_id == 9) {
    return 0;
  }

  node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "slots-present");
  if (node < 0) {
    return -1;
  }
  ret = gpio_request_list_by_name_nodev(offset_to_ofnode(node),
              "slots-present-gpios", desc,
              ARRAY_SIZE(desc), GPIOD_IS_IN);
  if (ret < 0) {
    return ret;
  }
  slots_present_status = dm_gpio_get_values_as_int(desc, ret);

  do {
    ret = uclass_get_device_by_name(UCLASS_I2C, CONFIG_BB_CPLD_BUS, &bus);
    if (ret) {
      break;
    }

    ret = i2c_get_chip(bus, CONFIG_BB_CPLD_ADDR, 1, &dev);
    if (ret) {
      break;
    }

    for (i = 0; i < MAX_NODES; i++) {
      present = (slots_present_status >> i) & 0x01;
      if (present == 0) {
        retry = 2;
        do {
          ret = dm_i2c_reg_write(dev, 0x09+i, 0x01);
          if (ret && retry) {
            udelay(10000);
          }
        } while (ret && (retry-- > 0));
      }
    }
  } while (0);

  return 0;
}
#endif //CONFIG_FBY3 end

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

//Config GPIOE pass-through
static void config_gpioe_pass_through(void) {
  u32 reg;

  //disable GPIOE Pass-throuth
  reg = 0x00400000;
  __raw_writel(reg, AST_SCU_BASE + 0x7C);

  //Enable Reset Button
  reg = 1 << 12;
  __raw_writel(reg, AST_SCU_BASE + 0x8C);
}
#endif

#if defined(CONFIG_FBAL)
static void led_init(void)
{
  u32 reg;

  // POSTCODE LED (GPIOH)
  reg = __raw_readl(AST_GPIO_BASE + 0x24);
  reg |= 0xFF000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x24);
  reg = __raw_readl(AST_GPIO_BASE + 0x20);
  reg &= ~0xFF000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x20);

  reg = __raw_readl(AST_GPIO_BASE + 0x68);
  reg |= 0x01000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x68);
  reg = __raw_readl(AST_GPIO_BASE + 0x6C);
  reg &= ~0x01000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x6C);
}

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
  // GPIOZ1 CPU power good
  return ((__raw_readl(AST_GPIO_BASE + 0x1E0) >> 9) & 1);
}

static int is_fbal_master(void)
{
  // GPIOO0 High:Slave Low:Master
  return (((__raw_readl(AST_GPIO_BASE + 0x078) >> 16) & 1) ? 0 : 1 );
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
    // Host Server is not on and Mode is master
    if ( !get_fbal_pwrok() && is_fbal_master() ) {
      set_fbal_pwrbtn(0);
      udelay(1000*1000);
      set_fbal_pwrbtn(1);
      udelay(1000*2000);
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
#endif

#if defined(CONFIG_FBEP)
static int init_LINKCFG_ASIC(void)
{
  struct udevice *dev, *bus;
  int retry = 2;
  int ret;
  u8 addr;

  ret = uclass_get_device_by_name(UCLASS_I2C, "i2c-bus@300", &bus);
  if (ret) {
    printf("i2c-7 is not enabled\n");
    return ret;
  }

  for (addr = 0x20; addr <= 0x27; addr++) {
    ret = i2c_get_chip(bus, addr, 1, &dev);
    if (ret) {
      printf("Can't find GPIO expander at 0x%02X\n", addr);
      break;
    }

    do {
      // LINK_CONFIG[4:0] = b'01010
      ret = dm_i2c_reg_write(dev, 0x2, 0xEA);
      if (ret && retry) {
	udelay(10000);
      }
      ret = dm_i2c_reg_write(dev, 0x6, 0xEA);
      if (ret && retry) {
	udelay(10000);
      }

    } while (ret && (retry-- > 0));
  }

  return ret;
}

static int init_PAX_FLASH(void)
{
  u32 reg;

  // SPI MUX: GPIOAA0/GPIOAA1/GPIOAA5/GPIOAA6
  // To output-low
  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  reg = reg & ~((0x1 << 16) | (0x1 << 17) | (0x1 << 21) | (0x1 << 22));
  __raw_writel(reg, AST_GPIO_BASE + 0x1E0);
  reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  reg = reg | ((0x1 << 16) | (0x1 << 17) | (0x1 << 21) | (0x1 << 22));
  __raw_writel(reg, AST_GPIO_BASE + 0x1E4);

  return 0;
}

static int setup_SKU_ID(int server_type)
{
  u32 reg;

  // Switch GPIOAC0 GPIOAC2 to GPIO function
  reg = __raw_readl(AST_SCU_BASE + 0xAC);
  reg = reg & ~((0x1 << 0) | (0x1 << 2));
  __raw_writel(reg, AST_SCU_BASE + 0xAC);

  // SKU ID0: GPIOB0/GPIOB4/GPIOAC0/GPIOAC2
  // SKU ID1: GPIOB6/GPIOB7/GPIOM0/GPIOM6
  // To ouput
  reg = __raw_readl(AST_GPIO_BASE + 0x04);
  reg = reg | ((0x1 << 8) | (0x1 << 12) | (0x1 << 14) | (0x1 << 15));
  __raw_writel(reg, AST_GPIO_BASE + 0x04);
  reg = __raw_readl(AST_GPIO_BASE + 0x7C);
  reg = reg | ((0x1 << 0) | (0x1 << 6));
  __raw_writel(reg, AST_GPIO_BASE + 0x7C);
  reg = __raw_readl(AST_GPIO_BASE + 0x1EC);
  reg = reg | ((0x1 << 0) | (0x1 << 2));
  __raw_writel(reg, AST_GPIO_BASE + 0x1EC);

  if (server_type == 8) {
    // 4S: SKU ID[0:1] = 01'b
    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg & ~((0x1 << 8) | (0x1 << 12));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);

    reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
    reg = reg & ~((0x1 << 0) | (0x1 << 2));
    __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg | ((0x1 << 14) | (0x1 << 15));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);
    reg = __raw_readl(AST_GPIO_BASE + 0x78);
    reg = reg | ((0x1 << 0) | (0x1 << 6));
    __raw_writel(reg, AST_GPIO_BASE + 0x78);

  } else if (server_type == 4) {
    // 4SEX: SKU ID[0:1] = 00'b
    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg & ~((0x1 << 8) | (0x1 << 12));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);

    reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
    reg = reg & ~((0x1 << 0) | (0x1 << 2));
    __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg & ~((0x1 << 14) | (0x1 << 15));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);
    reg = __raw_readl(AST_GPIO_BASE + 0x78);
    reg = reg & ~((0x1 << 0) | (0x1 << 6));
    __raw_writel(reg, AST_GPIO_BASE + 0x78);

  } else if (server_type == 2) {
    // 2S: SKU ID[0:1] = 10'b
    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg | ((0x1 << 8) | (0x1 << 12));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);

    reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
    reg = reg | ((0x1 << 0) | (0x1 << 2));
    __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

    reg = __raw_readl(AST_GPIO_BASE + 0x00);
    reg = reg & ~((0x1 << 14) | (0x1 << 15));
    __raw_writel(reg, AST_GPIO_BASE + 0x00);
    reg = __raw_readl(AST_GPIO_BASE + 0x78);
    reg = reg & ~((0x1 << 0) | (0x1 << 6));
    __raw_writel(reg, AST_GPIO_BASE + 0x78);

  } else {
    return -1;
  }

  return 0;
}

static int init_SKU_ID_PAX(void)
{
  struct udevice *dev, *bus;
  int server_type = -1;
  int retry = 2;
  int ret;
  char mode[8] = {0};

  ret = uclass_get_device_by_name(UCLASS_I2C, "i2c-bus@1c0", &bus);
  if (ret) {
    printf("i2c-6 is not enabled\n");
    return ret;
  }

  ret = i2c_get_chip(bus, 0x54, 2, &dev);
  if (ret) {
    printf("Can't find MB EEPROM\n");
    return ret;
  }

  do {
    server_type = dm_i2c_reg_read(dev, 1030);
    if (server_type < 0 && retry) {
      udelay(10000);
    }
  } while (server_type < 0 && (retry-- > 0));

  printf("Server type: ");
  if (server_type == 2) {
    snprintf(mode, sizeof(mode), "2S");
  } else if (server_type == 4) {
    snprintf(mode, sizeof(mode), "4SEX");
  } else if (server_type == 8) {
    snprintf(mode, sizeof(mode), "4S");
  } else {
    printf("Unknown\n");
    return -1;
  }
  printf("%s Socket mode\n", mode);
  return setup_SKU_ID(server_type);
}

static void init_ASIC_PAX(void)
{
  u32 reg;

  // Release AST SMBus reset
  reg = __raw_readl(AST_SCU_BASE + 0x04);
  reg &= ~(0x1 << 2);
  __raw_writel(reg, AST_SCU_BASE + 0x04);

  if (init_LINKCFG_ASIC() < 0 ||
      init_SKU_ID_PAX() < 0 ||
      init_PAX_FLASH() < 0)
    return;

  // Assume server is present
  // GPIOM1 (PWR_CTRL) to output-low
  reg = __raw_readl(AST_GPIO_BASE + 0x7C);
  reg |= (0x1 << 1);
  __raw_writel(reg, AST_GPIO_BASE + 0x7C);
  reg = __raw_readl(AST_GPIO_BASE + 0x78);
  reg &= ~(0x1 << 1);
  __raw_writel(reg, AST_GPIO_BASE + 0x78);

  // GPIOB2 (BMC_READY) to output-low
  reg = __raw_readl(AST_GPIO_BASE + 0x04);
  reg |= (0x1 << 10);
  __raw_writel(reg, AST_GPIO_BASE + 0x04);

  reg = __raw_readl(AST_GPIO_BASE + 0x00);
  reg &= ~(0x1 << 10);
  __raw_writel(reg, AST_GPIO_BASE + 0x00);
}
#endif

#if defined (CONFIG_FBCC)
static void init_SEL_FLASH_PAX(void)
{
  u32 reg;
  // set GPIOAA0 AA1 AA5 AA6 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  reg |= 0x00630000;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E4);
  // set GPIOAA0 AA1 AA5 AA6 value 0
  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  reg &= 0xFF9CFFFF;
  __raw_writel(reg, AST_GPIO_BASE + 0x0D8);
}

static void init_SKU_ID_PAX(void)
{
  u32 reg;
  // set GPIOB0 B4 AA5 B6 B7 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x004);
  reg |= 0x0000D100;
  __raw_writel(reg, AST_GPIO_BASE + 0x004);

  reg = __raw_readl(AST_GPIO_BASE + 0x000);
  reg &= 0xFFFF3EFF;
  // __raw_writel(reg, AST_GPIO_BASE + 0x0C0);
  __raw_writel(reg, AST_GPIO_BASE + 0x000);

  reg = __raw_readl(AST_GPIO_BASE + 0x000);
  reg |= 0x00001000;
  // __raw_writel(reg, AST_GPIO_BASE + 0x0C0);
  __raw_writel(reg, AST_GPIO_BASE + 0x000);

  // set GPIOM0 M6 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x07C);
  reg |= 0x00000041;
  __raw_writel(reg, AST_GPIO_BASE + 0x07C);

  reg = __raw_readl(AST_GPIO_BASE + 0x078);
  reg |= 0x00000041;
  // __raw_writel(reg, AST_GPIO_BASE + 0x0CC);
  __raw_writel(reg, AST_GPIO_BASE + 0x078);

  // set GPIOAC0 AC2 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x1EC);
  reg |= 0x5;
  __raw_writel(reg, AST_GPIO_BASE + 0x1EC);

  reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
  reg &= 0xFFFFFFF0;
  // __raw_writel(reg, AST_GPIO_BASE + 0x0DC);
  __raw_writel(reg, AST_GPIO_BASE + 0x1E8);

  reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
  reg |= 0x00000004;
  // __raw_writel(reg, AST_GPIO_BASE + 0x0DC);
  __raw_writel(reg, AST_GPIO_BASE + 0x1E8);
}

static void init_MUX_RESET_PIN(void)
{
  u32 reg;
  // set GPIOL5 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x074);
  reg |= 0x20000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x074);
  // set GPIOL5 value 1
  reg = __raw_readl(AST_GPIO_BASE + 0x070);
  reg |= 0x20000000;
  __raw_writel(reg, AST_GPIO_BASE + 0x070);

    // set GPIOY0 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x1E4);
  reg |= 0x00000001;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E4);
  // set GPIOY0 value 1
  reg = __raw_readl(AST_GPIO_BASE + 0x1E0);
  reg |= 0x00000001;
  __raw_writel(reg, AST_GPIO_BASE + 0x1E0);
}

static void init_CPLD_POWER_ON(void)
{
  u32 reg;

  // enable GPIOAC1
  reg = __raw_readl(AST_SCU_BASE + 0xAC);
  reg &= ~0x02;
  __raw_writel(reg, AST_SCU_BASE + 0xAC);

  // set GPIOAC1 direction output
  reg = __raw_readl(AST_GPIO_BASE + 0x1EC);
  reg |= 0x00000002;
  __raw_writel(reg, AST_GPIO_BASE + 0x1EC);

  reg = __raw_readl(AST_GPIO_BASE + 0x1E8);
  reg &= 0xFFFFFFFD;
  // set GPIOAC1 value 0
  __raw_writel(reg, AST_GPIO_BASE + 0x1E8);
}

#endif

#if defined (CONFIG_FBWEDGE400)
enum board_id {
  wedge400_EVT3 = 0x0004,
  wedge400_DVT1 = 0x0006,
  wedge400_PVT2 = 0x0007,
  wedge400_PVT4 = 0x000c,
  wedge400_MP = 0x000d,
  wedge400_MP_respin = 0x010e,

  wedge400C_EVT1 = 0x0000,
  wedge400C_EVT2 = 0x0001,
  wedge400C_DVT1 = 0x0002,
  wedge400C_DVT2 = 0x0003,
  wedge400C_MP_respin = 0x0108,
};

/*
 * Wedge400/Wedge400C Board ID GPIO MAP:
 * low_byte[bit0~bit4] = GPIOG4~GPIOG7
 * high_byte[bit0~bit1] = GPION6~GPION7
 * board_id = high_byte+low_byte
 */
static uint16_t get_board_id(void)
{
	int ret, node, board_id;
	struct gpio_desc desc[6];

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "board_id");
	if (node < 0) {
		return -1;
	}
	ret = gpio_request_list_by_name_nodev(offset_to_ofnode(node),
					      "board-id-gpios", desc,
					      ARRAY_SIZE(desc), GPIOD_IS_IN);
	if (ret < 0) {
		return ret;
	}
	ret = dm_gpio_get_values_as_int(desc, ret);
	board_id = ((ret & 0x30) << 4 ) | (ret & 0xF);
	printf("board_id = %x\n", board_id);

	return board_id;
}

int init_mac_TXRX_delay(void)
{
  uint16_t board_id=0xffff;
  uint8_t mac2_TX_delay = 0;
  uint8_t mac2_RX_delay = 0;

  board_id = get_board_id();
  switch(board_id) {
    case wedge400_MP_respin:
    case wedge400C_MP_respin:
      mac2_TX_delay = 10;
      mac2_RX_delay = 4;
    break;
    case wedge400_EVT3:
    case wedge400_DVT1:
    case wedge400_PVT2:
    case wedge400_PVT4:
    case wedge400_MP:
    case wedge400C_EVT1:
    case wedge400C_EVT2:
    case wedge400C_DVT1:
    case wedge400C_DVT2:
      mac2_TX_delay = 8;
      mac2_RX_delay = 2;
    break;
    default:
      printf("Not supported board id: 0x%04x\n", board_id);
    break;
  }

  /* Setup mac2 TX-delay and RX-delay */
  __raw_writel((0x80000000|(mac2_RX_delay<<18)|(mac2_TX_delay<<6)), AST_SCU_BASE + 0x48);

  return(0);
}

int init_SDIO_divider_clk(void){
  /*
   * SCU08  Clock Selection Register
   * bit 15     SCDCLK clock running enable
   *              0 stop clock
   *              1 Enable clock
   * bit 14:12  SDCCLK divider
   *            000 SDCLK = H-PLL/4
   *            001 SDCLK = H-PLL/8
   *                  ...
   *            110 SDCLK = H-PLL/28
   *            111 SDCLK = H-PLL/32
   */
  #define AST2500_SCDCLK_MASK    (0xf << 12)
  #define AST2500_SCDCLK_ENABLE  (0x1 << 15)
  #define AST2500_SCDCLK_HPLL_4  (0x0 << 12)
  #define AST2500_SCDCLK_HPLL_8  (0x1 << 12)
  #define AST2500_SCDCLK_HPLL_12  (0x2 << 12)
  #define AST2500_SCDCLK_HPLL_16  (0x3 << 12)
  #define AST2500_SCDCLK_HPLL_20  (0x4 << 12)
  #define AST2500_SCDCLK_HPLL_24  (0x5 << 12)
  #define AST2500_SCDCLK_HPLL_28  (0x6 << 12)
  #define AST2500_SCDCLK_HPLL_32  (0x7 << 12)
  u32 reg = __raw_readl(AST_SCU_BASE + 0x08);
  reg &= ~ AST2500_SCDCLK_MASK;
  reg |= AST2500_SCDCLK_ENABLE | AST2500_SCDCLK_HPLL_8;
  __raw_writel(reg, AST_SCU_BASE + 0x08);
  return(0);
}

#endif

int board_init(void)
{
#if CONFIG_IS_ENABLED(ASPEED_ENABLE_DUAL_BOOT_WATCHDOG)
	dual_boot_watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#else
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
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

#ifdef CONFIG_FBY3
  slot_12V_init();
#endif

#ifdef CONFIG_MINILAKETB
  fan_init();
  mux_init();
  slot_12V_init();
  slot_led_init();
  spi_init();
#endif

#if defined(CONFIG_FBAL)
  config_gpioe_pass_through();
  policy_init();
  disable_snoop_interrupt();
  led_init();
  enable_nic_mux();
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBSP)
  fan_init();
  config_gpioe_pass_through();
  policy_init();
  disable_snoop_interrupt();
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBEP)
  fix_mmc_hold_time_fail();
  init_ASIC_PAX();
#endif

#if defined(CONFIG_FBY3)
  fix_mmc_hold_time_fail();
#endif

#if defined(CONFIG_FBCC)
  init_SEL_FLASH_PAX();
  init_SKU_ID_PAX();
  init_MUX_RESET_PIN();
  init_CPLD_POWER_ON();
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

#ifdef CONFIG_ASPEEDNIC
int board_eth_init(bd_t *bd)
{
  return aspeednic_initialize(bd);
}
#endif
