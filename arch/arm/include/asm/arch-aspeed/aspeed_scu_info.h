#ifndef __ASPEED_SCU_INFO_H_INCLUDED
#define __ASPEED_SCU_INFO_H_INCLUDED

extern void aspeed_get_revision_id(void);
extern int aspeed_get_mac_phy_interface(u8 num);
extern void aspeed_security_info(void);
extern void aspeed_sys_reset_info(void);
extern void aspeed_who_init_dram(void);
extern void aspeed_2nd_wdt_mode(void);
extern void aspeed_espi_mode(void);
extern void aspeed_spi_strap_mode(void);

#endif
