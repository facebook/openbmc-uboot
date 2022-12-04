/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) ASPEED Technology Inc.
 */

/* Define register */
/* MCU */
#define SYS_REST		0x1e6e2040
#define SYS_REST_CLR		0x1e6e2044

/* DP TX */
#define DP_TX_INT_CLEAR		0x1e6eb040
#define DP_TX_INT_STATUS	0x1e6eb044

#define DP_TX_IRQ_CFG		0x1e6eb080
#define DP_TX_EVENT_CFG		0x1e6eb084

#define DP_AUX_REQ_CFG		0x1e6eb088
#define DP_AUX_ADDR_LEN		0x1e6eb08c
#define DP_AUX_STATUS		0x1e6eb0b0

#define DP_AUX_W_D_0		0x1e6eb090
#define DP_AUX_W_D_4		0x1e6eb094
#define DP_AUX_W_D_8		0x1e6eb098
#define DP_AUX_W_D_C		0x1e6eb09c

#define DP_AUX_R_D_0		0x1e6eb0a0
#define DP_AUX_R_D_4		0x1e6eb0a4
#define DP_AUX_R_D_8		0x1e6eb0a8
#define DP_AUX_R_D_C		0x1e6eb0ac

#define DP_TX_MAIN_SET		0x1e6eb0c0
#define DP_TX_MAIN_PAT		0x1e6eb0e4
#define DP_TX_PHY_PAT		0x1e6eb0e8
#define DP_TX_CUS_PAT_0		0x1e6eb0ec
#define DP_TX_CUS_PAT_1		0x1e6eb0f0
#define DP_TX_CUS_PAT_2		0x1e6eb0f4
#define DP_TX_MAIN_CFG		0x1e6eb0fc
#define DP_TX_PHY_SET		0x1e6eb104
#define DP_TX_PHY_CFG		0x1e6eb108
#define DP_TX_RES_CFG		0x1e6eb118

/* i2c control */
#define MP_SCU410		0x1e6e2410
#define MP_SCU414		0x1e6e2414
#define MP_SCU418		0x1e6e2418
#define MP_SCU4b0		0x1e6e24b0
#define MP_SCU4b4		0x1e6e24b4
#define MP_SCU4b8		0x1e6e24b8

#define I2C_GBASE		0x1e78a000
#define I2C_DEV_OFFSET		0x80

#define I2C0_BASE		0x1e78a080
#define I2C0_TIMMING_O		0x04
#define I2C0_COUNT_O		0x0c
#define I2C0_INT_O		0x10
#define I2C0_INT_STATUS_O	0x14
#define I2C0_EXECUTE_O		0x18

#define I2C0_BUFF		0x1e78ac00
#define I2C_BUFF_OFFSET		0x20

/* Re-driver setting  */
#define RD_EQ3			0x80
#define RD_EQ2			0x40
#define RD_EQ1			0x20
#define RD_EQ0			0x10
#define RD_FG1			0x08
#define RD_FG0			0x04
#define RD_SW1			0x02
#define RD_SW0			0x01

/* Aux write command */
#define AUX_CMD_W		0x80000010

/* Aux read command */
#define AUX_CMD_R		0x90000010

/* I2C write over AUX command with MOT */
#define I2C_M_CMD_W		0x40000010
/* I2C write over AUX command with MOT and stop by addr */
#define I2C_M_EA_CMD_W	0x40010010

/* I2C read over AUX command with MOT */
#define I2C_M_CMD_R		0x50000010
/* I2C read over AUX command with MOT and stop by addr */
#define I2C_M_EA_CMD_R		0x50010010

/* I2C write over AUX command */
#define I2C_CMD_W		0x00000010
/* I2C write over AUX command stop by addr */
#define I2C_EA_CMD_W		0x00010010

/* I2C read over AUX command */
#define I2C_CMD_R		0x10000010
/* I2C read over AUX command stop by addr */
#define I2C_EA_CMD_R		0x10010010

#define AUX_CMD_TIMEOUT		0x8000
#define AUX_CMD_DONE		0x4000

/* DP Rate / Lane */
#define DP_RATE_1_62		0x0000
#define DP_RATE_2_70		0x0100
#define DP_RATE_5_40		0x0200

/* DP Deemphasis setting */
#define DP_DEEMP_0		0x00000000
#define DP_DEEMP_1		0x11000000
#define DP_DEEMP_2		0x22000000

/* DP Main link SSCG setting */
#define DP_SSCG_ON		0x00000010
#define DP_SSCG_OFF		0x00000000

/* Status */
#define DP_TX_RDY_TEST		0x11003300
#define DP_PHY_INIT_CFG		0x00113021
#define DP_TX_PAT_HBR2		0x00050100
#define DP_TX_PAT_TPS1		0x10000100

#define DP_TX_PLTPAT_0		0x3e0f83e0
#define DP_TX_PLTPAT_1		0x0f83e0f8
#define DP_TX_PLTPAT_2		0x0000f83e

#define DP_PY_PAT		0x00000642
#define DP_PY_PAT_PRB7		0x01000000
#define DP_PY_PAT_SCRB		0x00001000
#define DP_PY_PAT_CUS		0x10000000

#define DP_TX_MAIN_NOR		0x00000020
#define DP_TX_MAIN_ADV		0x00001020
#define DP_TX_PY_RESET		0x00000001
#define DP_TX_MAIN_TRA		0x03000000
#define DP_TX_RDY_25201		0x00000200

#define DP_TX_HIGH_SPEED	0x000C0000
#define DP_TX_NOR_SPEED		0x00000000

#define DP_TX_D_INT_CFG		0x00C00000
#define DP_TX_A_INT_CFG		0x00F80000

/* Flag */
#define F_EMPHASIS_NULL		0x00000001	// No emphasis	: 2
#define F_EMPHASIS		0x00000002	// Emphasis	: 1-0-2
#define F_EMPHASIS_1		0x00000004	// Emphasis 1	: 2-0-1
#define F_RES_HIGH		0x00000008
#define F_PAT_PRBS7		0x00000010	// PRBS7 pattern
#define F_PAT_PLTPAT		0x00000020	// PLTPAT pattern
#define F_PAT_HBR2CPAT		0x00000040	// HBR2CPAT pattern
#define F_PAT_D10_2		0x00000080	// D10_2 and TPS1 pattern
#define F_PAT_AUX		0x00000100	// Aux testing pattern
#define F_SHOW_SWING		0x00000200	// Show swing level
#define F_EXE_AUTO		0x00000400	// Enter auto mode

/* Print message define */
#define PRINT_RATE_1_62		printf("DP Rate 1.62 Gbps !\n")
#define PRINT_RATE_2_70		printf("DP Rate 2.70 Gbps !\n")
#define PRINT_RATE_5_40		printf("DP Rate 5.40 Gbps !\n")

#define PRINT_SWING_0		printf("DP Swing Level 0!\n")
#define PRINT_SWING_1		printf("DP Swing Level 1!\n")
#define PRINT_SWING_2		printf("DP Swing Level 2!\n")

#define PRINT_DEEMP_0		printf("DP Pre - Emphasis Level 0!\n")
#define PRINT_DEEMP_1		printf("DP Pre - Emphasis Level 1!\n")
#define PRINT_DEEMP_2		printf("DP Pre - Emphasis Level 2!\n")

#define PRINT_EMPVAL_0		printf("DP Pre - Emphasis Level 0 !\n")
#define PRINT_EMPVAL_1		printf("DP Pre - Emphasis Level 1 !\n")
#define PRINT_EMPVAL_2		printf("DP Pre - Emphasis Level 2 !\n")

#define PRINT_SSCG_ON		printf("DP SSCG ON !\n")
#define PRINT_SSCG_OFF		printf("DP SSCG OFF !\n")

#define PRINT_INVALID		printf("This parameter is invalid : \t")

#define PRINT_ITEM_A		printf("3.1 Eye Diagram Test - PRBS7\n");\
				printf("3.4 Inter Pair Skew Test -PRBS7\n");\
				printf("3.11 Non ISI Jitter Test -PRBS7\n");\
				printf("3.12 Total Jitter Test -PRBS7\n")\

#define PRINT_ITEM_B		printf("3.3 Peak to Peak Voltage Test - PRBS7\n")

#define PRINT_ITEM_C		printf("3.3 Peak to Peak Voltage Test -PLTPAT\n");\
				printf("3.3 Pre-Emphasis Level Test and Pre-Emphasis Level Delta Test -PLTPAT\n")\

#define PRINT_ITEM_D		printf("3.3  Non-Transition Voltage Range Measurement -PLTPAT\n")

#define PRINT_ITEM_E		printf("3.3 Pre-Emphasis Level Test and Pre-Emphasis Level Delta Test -PRBS7\n")

#define PRINT_ITEM_F		printf("3.3 Non-Transition Voltage Range Measurement - PRBS7\n")

#define PRINT_ITEM_G		printf("3.12 Total Jitter Test with No Cable Model -D10.2\n");\
				printf("3.12 Total Jitter Test -D10.2\n");\
				printf("3.12 Deterministic Jitter Test with No Cable Model -D10.2\n");\
				printf("3.12 Deterministic Jitter Test -D10.2\n");\
				printf("3.12 Random Jitter Test with No Cable Model -D10.2\n");\
				printf("3.12 Random Jitter Test -D10.2\n");\
				printf("3.14 Main Link Frequency Compliance -D10.2\n");\
				printf("3.15 SSC Modulation Frequency Test -D10.2\n");\
				printf("3.16 SSC Modulation Deviation Test -D10.2\n")\

#define PRINT_ITEM_H		printf("3.2 Non Pre-Emphasis Level Test - PRBS7\n")

#define PRINT_ITEM_I		printf("3.2 Non Pre-Emphasis Level Test - PLTPAT\n")

#define PRINT_ITEM_J		printf("8.1 AUX Channel Measurement\n")

#define PRINT_ITEM_K		printf("3.1 Eye Diagram Test with No Cable Mode - HBR2CPAT\n");\
				printf("3.1 Eye Diagram Test - HBR2CPAT\n");\
				printf("3.12 Total Jitter Test with No Cable Model -HBR2CPAT\n");\
				printf("3.12 Total Jitter Test -HBR2CPAT\n");\
				printf("3.12 Deterministic Jitter Test with No Cable Model -HBR2CPAT\n");\
				printf("3.12 Deterministic Jitter Test -HBR2CPAT\n")\

#define PRINT_ITEM_X		printf("Auto DP Measurement\n")

/* Function prototype */
void DPTX_MCU_Reset(void);
void DPPHY_Set(void);
char DPPHYTX_Show_Cfg(void);
void DPPHYTX_Show_Item(char received);

/* I2c set */
void I2C_G_Initial(void);
void I2C_L_Initial(void);
void Set_Redriver(void);

void Set_PRBS7(void);
void Set_HBR2CPAT(void);
void Set_PLTPAT(void);
void Set_D10_1(void);

uchar AUX_R(int aux_cmd, int aux_addr, int *aux_r_data, uchar *length, uchar *status);
uchar AUX_W(int aux_cmd, int aux_addr, int *aux_w_data, uchar *length, uchar *status);

void Apply_HPD_Normal(void);
void Apply_HPD_Auto_Test(void);

uchar Link_Train_Flow(char Link_Level);
uchar Adjust_CR_EQ_Train(int TrainPat, uchar ADJStatus);

void Apply_EDID_Reading(void);
void Read_EDID_128(char Offset);

void Apply_Auto_Preset(char Init);

void Apply_Auto_Mesument(void);
void Apply_Main_Mesument(int flag);
void Apply_AUX_Mesument(int flag);
