/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */
uint32_t mac_reg_read(MAC_ENGINE *p_eng, uint32_t addr);
void mac_reg_write(MAC_ENGINE *p_eng, uint32_t addr, uint32_t data);
void mac_set_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d);
void mac_get_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d);
void mac_set_driving_strength(MAC_ENGINE *p_eng, uint32_t strength);
uint32_t mac_get_driving_strength(MAC_ENGINE *p_eng);
void mac_set_rmii_50m_output_enable(MAC_ENGINE *p_eng);
int mac_set_scan_boundary(MAC_ENGINE *p_eng);
void mac_set_addr(MAC_ENGINE *p_eng);
void mac_set_interal_loopback(MAC_ENGINE *p_eng);

void PrintIO_Line(MAC_ENGINE *p_eng, uint8_t option);
void PrintIO_LineS(MAC_ENGINE *p_eng, uint8_t option);
void FPri_End(MAC_ENGINE *eng, uint8_t option);
void FPri_RegValue(MAC_ENGINE *eng, uint8_t option);