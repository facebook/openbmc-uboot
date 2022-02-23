// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>
#include "util.h"
#include "ast-g6.h"

DECLARE_GLOBAL_DATA_PTR;

void el_port80_init(void){

  uint32_t value = 0;

  //GPION Settings
  value  = readl(AST6_GPIO_BASE | 0x7C);
  value |= 0x0000FF00;  //GPIO N Output
  writel(value,  AST6_GPIO_BASE | 0x7C);

  //GPIOM/N/O/P Command Source 0
  value  = readl(AST6_GPIO_BASE | 0x0E0);
  value |= 1<<8;    //Port GPION[7:0] Command Source 0 = 1 (LPC)
  writel(value,  AST6_GPIO_BASE | 0x0E0);

  //GPIOM/N/O/P Command Source 1
  value  = readl(AST6_GPIO_BASE | 0x0E4);
  value &= ~(1<<8); //Port GPION[7:0] Command Source 1 = 0 (LPC)
  writel(value,  AST6_GPIO_BASE | 0x0E4);

  //SNPWADR(0x90): LPC Snoop Address Register
  value  = readl(AST6_LPC_BASE | LPC_SNPWADR);
  value &= 0xFFFF0000;  //BIT[15:0] - clear;
  value |= 0x00000080;  //BIT[15:0] - set;
  writel(value,  AST6_LPC_BASE | LPC_SNPWADR);

  //HICR5(0x80): Host Interface Control Register 5
  value  = readl(AST6_LPC_BASE | LPC_HICR5);
  value |= 1;       //Enable snooping address #0
  value &= ~0xA;
  writel(value,  AST6_LPC_BASE | LPC_HICR5);

  //HICRB(0x100): Host Interface Control Register B
  value  = readl(AST6_LPC_BASE | LPC_HICRB);
  value |= 1<<14;//EnSNP0D: Enable ACCEPT response code for snoop #0 commands, defined in HICR5[0], in eSPI mode.
  writel(value,  AST6_LPC_BASE | LPC_HICRB);

  writel(0, LPC_PCCR0);
}


void el_espi_init(void){
  uint32_t value=0;

  //SCU514: SCU514: Hardware Strap2 Clear Register (default)
  value = readl(AST6_SCU_BASE | 0x514);
  value |= 1<<6; //eSPI Mode
  writel(value,  AST6_SCU_BASE | 0x514);

  //SCU434: Multi-function Pin Control #9 (default)
  value  = readl(AST6_SCU_BASE | 0x434);
  value |= 0xFF<<16;
  writel(value,  AST6_SCU_BASE | 0x434);

  //SCU454: Multi-function Pin Control #15
  value  = readl(AST6_SCU_BASE | 0x454);
  value &= 0x00FFFFFF;  //Clear BIT[31:24]
  value |= 0xAA000000;  //BIT[31:24] = LAD3ESPID3~0 Driving Strength
  writel(value,  AST6_SCU_BASE | 0x454);

  //ESPI000: Engine Control
  value  = readl(AST6_ESPI_BASE);
  value |= 1<<4;   //OOB Channel Ready.
  writel(value,  AST6_ESPI_BASE);
}

void el_superio_decoder(uint8_t addr) {
  uint32_t value=0;
  //Super IO Settings
  //SCU514: Hardware Strap2 'Clear' Register
  value  = 0;
  value |= 1<<3;   //Enable LPC to decode SuperIO 0x2E/0x4E address
  writel(value,  AST6_SCU_BASE | 0x514);

  //Super IO Settings
  //SCU510: Hardware Strap2 Register
  //SuperIO configuration address selection (0 = 0x2E(Default) / 1 = 0x4E)
  if (addr == 0x4E ) {
    value  = readl(AST6_SCU_BASE | 0x510);
    value |= 1<<2; 
    writel(value,  AST6_SCU_BASE | 0x510);
  }
}


void debugMsg_espi(void){

  uint32_t value = 0;

  //Debug Message
  printf("\n[S9S] eSPI Settings\n");
  value  = readl(AST6_SCU_BASE | 0x510);
  printf("SCU510 = 0x%.8X , eSPI Mode[6](0 = eSPI)\n" , value);
  value  = readl(AST6_SCU_BASE | 0x434);
  printf("SCU434 = 0x%.8X , eSPI Pins[23:16]\n" , value);
  value  = readl(AST6_SCU_BASE | 0x454);
  printf("SCU454 = 0x%.8X , eSPI Driving Strength[31:24]\n" , value);
  value  = readl(AST6_ESPI_BASE);
  printf("ESPI000 = 0x%.8X , OOB Channel Ready[4]\n" , value);

  //end
  printf("\n");
}

#ifdef CONFIG_FBGC
static void system_status_led_init(void)
{
	/* GPIO088: GPIO_U/V/W/X Data Value Register
	 * 31:24 Port GPIOX[7:0] data register
	 * 23:16 Port GPIOW[7:0] data register
	 * 15:8 Port GPIOV[7:0] data register
	 * 7:0 Port GPIOU[7:0] data register
	 */
	u32 value = readl(0x1e780088);
	/* GPIO08C: GPIO_U/V/W/X Direction Value Register
	 * 31:24 Port GPIOX[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 23:16 Port GPIOW[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 15:8 Port GPIOV[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 7:0 Reserved
	 * GPIOU is input only
	 */
	u32 direction = readl(0x1e78008C);

	/* GPIOV4: BMC_LED_STATUS_BLUE_EN_R
	 * 0: LED OFF 1:LED ON BLUE
	 * GPIOV5: BMC_LED_STATUS_YELLOW_EN_R
	 * 0: LED ON YELLOW 1:LED OFF
	 */
	direction |= 0x3000; // set GPIOV4 & GPIOV5 direction output
	value &= 0xFFFFCFFF; // set GPIOV4 & GPIOV5 value 0
	writel(direction, 0x1e78008C);
	writel(value, 0x1e780088);
}

#endif /* CONFIG_FBGC */

#ifdef CONFIG_ELBERTVBOOT
extern void configureBcm53134(void);
#endif

int board_init(void)
{
	struct udevice *dev;
	int i;
	int ret;
	u64 rev_id;
	u32 tmp_val;

	/* disable address remapping for A1 to prevent secure boot reboot failure */
	rev_id = readl(ASPEED_REVISION_ID0);
	rev_id = ((u64)readl(ASPEED_REVISION_ID1) << 32) | rev_id;

	if (rev_id == 0x0501030305010303 || rev_id == 0x0501020305010203) {
		if ((readl(ASPEED_SB_STS) & BIT(6))) {
			tmp_val = readl(0x1e60008c) & (~BIT(0));
			writel(0xaeed1a03, 0x1e600000);
			writel(tmp_val, 0x1e60008c);
			writel(0x1, 0x1e600000);
		}
	}

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if CONFIG_IS_ENABLED(ASPEED_ENABLE_DUAL_BOOT_WATCHDOG)
	dual_boot_watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#else
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif

	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	vboot_check_enforce();

#ifdef CONFIG_FBGC
	system_status_led_init();
#endif /* CONFIG_FBGC */

#ifdef CONFIG_ELBERVBOOT
	configureBcm53134();
#endif /* ELBERT specific */

#ifdef CONFIG_FBGT
        el_port80_init();
        el_espi_init();
        el_superio_decoder(0x4E);
        debugMsg_espi();
#endif /* FBGT specific */
	return 0;
}
