#ifndef __ASPEED_SCU_INFO_H_INCLUDED
#define __ASPEED_SCU_INFO_H_INCLUDED

int aspeed_get_mac_phy_interface(u8 num);

void __weak aspeed_print_fmc_aux_ctrl(void);
void __weak aspeed_print_spi1_abr_mode(void);
void __weak aspeed_print_spi1_aux_ctrl(void);

void aspeed_print_soc_id(void);
void aspeed_print_security_info(void);
void aspeed_print_sysrst_info(void);
void aspeed_print_dram_initializer(void);
void aspeed_print_2nd_wdt_mode(void);
void aspeed_print_spi_strap_mode(void);
void aspeed_print_espi_mode(void);
void aspeed_print_mac_info(void);

#endif
