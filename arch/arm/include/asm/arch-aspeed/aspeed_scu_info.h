#ifndef __ASPEED_SCU_INFO_H_INCLUDED
#define __ASPEED_SCU_INFO_H_INCLUDED

extern int aspeed_get_mac_phy_interface(u8 num);

extern void aspeed_print_soc_id(void);
extern void aspeed_print_security_info(void);
extern void aspeed_print_sysrst_info(void);
extern void aspeed_print_dram_initializer(void);
extern void aspeed_print_2nd_wdt_mode(void);
extern void aspeed_print_spi_strap_mode(void);
extern void aspeed_print_espi_mode(void);
extern void aspeed_print_mac_info(void);

#endif
