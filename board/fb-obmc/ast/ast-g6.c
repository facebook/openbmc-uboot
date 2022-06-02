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
//ESPI SETTING
void el_espi_init(void){
	uint32_t value=0;

	//SCU514: SCU514: Hardware Strap2 Clear Register (default)
	//SCU510 [6]:0 eSPI mode
	setbits_le32(SCU_HW_STRAP2_CLR_REG, BIT(6));

	//SCU434: Multi-function Pin Control #9 (default)
	setbits_le32(SCU_MUTI_FN_PIN_CTRL9, 0xFF<<16);

	//SCU454: Multi-function Pin Control #15
	value  = readl(SCU_MUTI_FN_PIN_CTRL15);
	value &= 0x00FFFFFF;  //Clear BIT[31:24]
	value |= 0xAA000000;  //BIT[31:24] = LAD3ESPID3~0 Driving Strength
	writel(value, SCU_MUTI_FN_PIN_CTRL15);

	//ESPI000: Engine Control
	setbits_le32(AST6_ESPI_BASE, 1<<4);
}


//Super IO Settings
void el_superio_decoder(uint8_t addr) {
	//Enable LPC to decode SuperIO 0x2E/0x4E address
	setbits_le32(SCU_HW_STRAP2_CLR_REG, BIT(3));

	//SuperIO configuration address selection (0 = 0x2E(Default) / 1 = 0x4E)
	if ( addr == 0x4E )
		setbits_le32(SCU_HW_STRAP2_SET_REG, BIT(2));
}

void debugMsg_espi(void){

	uint32_t value = 0;

	//Debug Message
	printf("\n[S9S] eSPI Settings\n");
	value  = readl(SCU_HW_STRAP2_SET_REG);
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

// SNOOP SET
void el_port80_init(uint32_t reg_dir, uint32_t reg_val,
		uint32_t cmd_source0, uint32_t cmd_source1 ){

	uint32_t value = 0;

	//GPIO Output Settings
	setbits_le32(reg_dir, reg_val);

	//Command Source0 = 1
	//Command Source1 = 0
	setbits_le32(cmd_source0, BIT(8));
	clrbits_le32(cmd_source1, BIT(8));

	//SNPWADR(0x90): LPC Snoop Address Register
	value  = readl(LPC_SNPWADR);
	value &= 0xFFFF0000;  //BIT[15:0] - clear;
	value |= 0x00000080;  //BIT[15:0] - set;
	writel(value, LPC_SNPWADR);
	//HICR5(0x80): Host Interface Control Register 5
	value  = readl(LPC_HICR5);
	value |= 1;       //Enable snooping address #0
	value &= ~0xA;
	writel(value, LPC_HICR5);

	//HICRB(0x100): Host Interface Control Register B
        //EnSNP0D: Enable ACCEPT response code for snoop #0 commands, defined in HICR5[0], in eSPI mode.
	setbits_le32(LPC_HICRB, BIT(14));

	writel(0, LPC_PCCR0);
}

// SGPIO SETTING
void enable_sgpiom1(uint16_t sgpio_clk_div, uint8_t sgpio_byte) {
	uint32_t value = 0;

	//Clk=2M 16byte
	clrbits_le32(SGPIO1_CFG_REG, 0xFFFFFFFF);
	value = SGPIO_CLK_DIV(sgpio_clk_div) | SGPIO_BYTES(sgpio_byte) | SGPIO_ENABLE;
	setbits_le32(SGPIO1_CFG_REG, value);

	setbits_le32(SCU_MUTI_FN_PIN_CTRL5, SCU_SGPM_ENABLE);
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
	clrbits_le32(SCU_HW_STRAP3_REG, ENABLE_GPIO_PASSTHROUGH);
	el_port80_init(GPIO_MNOP_DIR_REG, GPIO_GROUP('N', 0xFF),
	GPIO_MNOP_CMD_SOURCE0, GPIO_MNOP_CMD_SOURCE1);
	el_espi_init();
	el_superio_decoder(0x4E);
	enable_sgpiom1(254, 16);
//	debugMsg_espi();
#endif /* FBGT specific */
	return 0;
}
