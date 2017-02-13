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

#include <asm/arch/ast_scu.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/arch/vbs.h>
#include <asm/io.h>

#define AST_WDT_CLK (1*1000*1000) /* 1M clock source */

DECLARE_GLOBAL_DATA_PTR;

void watchdog_init(void)
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
  u32 reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
  /* set the reload value */
  __raw_writel(reload, AST_WDT_BASE + 0x04);
  /* magic word to reload */
  __raw_writel(0x4755, AST_WDT_BASE + 0x08);
  /* start the watchdog with 1M clk src and reset whole chip */
  u32 val = 0x33; /* Full | Clear after | Enable */
  /* Some boards may request the reset to trigger the EXT reset GPIO.
   * On Linux this is defined as WDT_CTRL_B_EXT.
   */
#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
  val |= 0x08; /* Ext */
#endif
  __raw_writel(val, AST_WDT_BASE + 0x0c);
  printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

static void vbs_handoff(void)
{
  /* Clean the handoff marker from ROM. */
  struct vbs *vbs = (struct vbs*)AST_SRAM_VBS_BASE;
  vbs->rom_handoff = 0x0;
}

#ifdef CONFIG_FBTP
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
  // If BMC is reset by watch dog#1, #2(SCU3C[2:3]) or
  // external reset pin(SCU3C[1])
  if (!(reg & 0xe)) {
    // getenv return the same buffer,
    // duplicate result before call it again.
    result = getenv("por_policy");
    policy = (result) ? strdup(result) : NULL;
    // printf("%X por_policy:%s\n", policy, policy?policy:"null");

    result = getenv("por_ls");
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

int board_init(void)
{
	watchdog_init();
	vbs_handoff();

#ifdef CONFIG_FBTP
  fan_init();
  policy_init();
  disable_bios_debug();
  disable_snoop_dma_interrupt();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = dram - vga - (62*1024*1024); // ECC used = (dram(512) - vga(16M))/8 = 62M

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
