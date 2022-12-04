// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <exports.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <fdtdec.h>
#include <asm/io.h>

#include "dptest.h"

/* Version info*/
#define MAINVER		0
#define SUBVER			3
#define TEMPVER			2

#define YEAR	2022
#define MONTH	07
#define DAY		11

/* Compile define */
/* #define RE_DRIVER */
/* #define INTERNAL */

/* Debug define */
#define DBG_Print

#define DBG_ERR		0x00000001	/* DBG_ERROR */
#define DBG_NOR		0x00000002	/* DBG_NORMAL */
#define DBG_A_NOR	0x00000004	/* DBG_AUTO_NORMAL */
#define DBG_A_TEST	0x00000008	/* DBG_AUTO_TEST */
#define DBG_A_SUB	0x00000010	/* DBG_AUTO_SUBFUNS */
#define DBG_A_EDID	0x00000020	/* DBG_AUTO_EDID */
#define DBG_INF		0x00000040	/* DBG_INFORMATION */
#define DBG_STAGE	0x00000040	/* DBG_STAGE */
#define DBG_AUX_R	0x00001000	/* DBG_AUX_R_VALUE */
/* #define DBG_AUX_W	0x00002000	*//*DBG_AUX_W_VALUE */

int DBG_LEVEL = 0x00000040;		/* Information and stage */
/* int DBG_LEVEL = 0x0000107F;		*//*Fully */
/* int DBG_LEVEL = 0x00000001;		*//*Error */

#ifdef DBG_Print
#define DBG(Level, format, args...) if ((Level) & DBG_LEVEL) printf(format, ##args)
#else
#define DBG(Level, format, args...)
#endif

int	PHY_Cfg_N;
int	PHY_Cfg;
int	PHY_Cfg_1;
int	TX_SSCG_Cfg;
int	DP_Rate;
int	Deemphasis_Level;
int	Deemphasis_Level_1;
int	Deemphasis_Show;
int	Deemphasis_RD;
int	Swing_Level;
int	SSCG;
int	Current_Item;
int	GFlag;
uchar	EDID[256];

int	I2C_PORT;/* I2c port */
int	I2C_BASE;
int	I2C_BUFF;
uchar	RD_VAL;

/* Record DP Sink status */
uchar bEn_Frame;
uchar Link_Aux_RD_Val;
uchar CR_EQ_Keep;
int Deemlevel[3] = {DP_DEEMP_2, DP_DEEMP_0, DP_DEEMP_1};
uchar Auto_Link_Rate;
uchar Auto_Lane_Count;
/* uchar Patch_Normal_Behavior; */

static int
do_ast_dptest(cmd_tbl_t *cmdtp, int flags, int argc, char *const argv[])
{
	char received = 0;
	char execute_test = 0;
	int flag = 0, i;
	char *Temp = NULL;

	/* Default setting */
	DP_Rate			= DP_RATE_1_62;

	Deemphasis_Show	= DP_DEEMP_0;
	Deemphasis_Level	= DP_DEEMP_0;
	Deemphasis_Level_1	= DP_DEEMP_2;
	Swing_Level		= 2;
	SSCG			= DP_SSCG_ON;

	/* Obtain the argc / argv */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'D':
			case 'd':
				Temp = (char *)&argv[i][2];
				DBG_LEVEL = (int)(strtoul(Temp, NULL, 16));
				DBG_LEVEL |= DBG_ERR;	/* Add must print. */
				printf("DBG_LEVEL change into 0x%x\n", DBG_LEVEL);
				break;
			case 'R':
			case 'r':
				Temp = (char *)&argv[i][2];
				I2C_PORT = (int)(strtoul(Temp, NULL, 16));
				printf("I2C_PORT change into 0x%x\n", I2C_PORT);
				break;
			default:
				I2C_PORT = 0x0;
				break;
			}
		}
	}

	printf("ASPEED DP Physical Layer Test Tool V %d.%d.%d\n", MAINVER, SUBVER, TEMPVER);
#ifdef INTERNAL
	printf("Internal Version\n");
#endif

	printf("Build Date: %d.%d.%d\n\n", YEAR, MONTH, DAY);

	printf("PLEASE REFER USER MANUAL BEFORE TEST!!\n\n");

	/* DP TX MCU reset */
	DPTX_MCU_Reset();
	printf("DP TX MCU Initial Done!\n");

	/* DP phy set */
	DPPHY_Set();
	printf("DP Physcial Config Done!\n");

	printf("Press ESC key to leave test ...\n\n");

	// While for auto testing
	while (1) {
		if (tstc()) {
			received = getc();

			/* printf("Press %d\n",received); */

			/* Set parameters path */
			if (received >= 49 && received <= 53) {
				switch (received) {
				/* Press "1" : Set DP_Rate as 1.62 */
				case '1':
					DP_Rate = DP_RATE_1_62;
					PRINT_RATE_1_62;
					break;
				/* Press "2" : Set DP_Rate as 2.701 */
				case '2':
					DP_Rate = DP_RATE_2_70;
					PRINT_RATE_2_70;
					break;
				/* Press "3" : Set DP_Rate as 5.40 */
				/* case '3': */
				/*	DP_Rate = DP_RATE_5_40;	*/
				/*	PRINT_RATE_5_40 */
				/*	break; */
#ifdef INTERNAL
				/* Press "3" : Set Deemphasis_Level as 1 / Deemphasis_Level_1 as 2 */
				case '3':
					Deemphasis_Level	= DP_DEEMP_1;
					Deemphasis_Level_1	= DP_DEEMP_2;
					Deemphasis_Show	= DP_DEEMP_0;
					Deemphasis_RD		= Deemphasis_Show;
					PRINT_DEEMP_0;
					break;
				/* Press "4" : Set Deemphasis_Level as 0 / Deemphasis_Level_1 as 0 */
				case '4':
					Deemphasis_Level	= DP_DEEMP_0;
					Deemphasis_Level_1	= DP_DEEMP_0;
					Deemphasis_Show	= DP_DEEMP_1;
					Deemphasis_RD		= Deemphasis_Show;
					PRINT_DEEMP_1;
					break;
				/* Press "5" : Set Deemphasis_Level as 2 / Deemphasis_Level_1 as 1 */
				case '5':
					Deemphasis_Level	= DP_DEEMP_2;
					Deemphasis_Level_1	= DP_DEEMP_1;
					Deemphasis_Show	= DP_DEEMP_2;
					Deemphasis_RD		= Deemphasis_Show;
					PRINT_DEEMP_2;
					break;
#endif
				default:
					break;
				}

				if (execute_test)
					printf("The parameter is applied next measure!\n");

			} else if (received == '!') /* show config */ {
				if (execute_test)
					printf("Measurement is executed!\n");
				else
					printf("Measurement is stopped!\n");

				DPPHYTX_Show_Cfg();
			} else if (received == '0') /* change sscfg */ {
				if (SSCG == DP_SSCG_ON) {
					SSCG = DP_SSCG_OFF;
					PRINT_SSCG_OFF;
				} else {
					SSCG = DP_SSCG_ON;
					PRINT_SSCG_ON;
				}

				/* SSCG could be applied without reset device. */
				if (execute_test) {
					printf("Apply SSCG into current measurement !\n\n");
					TX_SSCG_Cfg = DP_TX_RDY_TEST;
					/* TX_SSCG_Cfg |= SSCG; */
					TX_SSCG_Cfg |= DP_SSCG_ON;
					writel(TX_SSCG_Cfg, DP_TX_PHY_SET);
				}
			} else {
				/* Check the ESC key */
				if (received == 27) {
					execute_test = 0;
					DPTX_MCU_Reset();
					printf("'ESC' is pressed!\n");
					break;
				}

				/* If the test is execute, reset it */
				if (execute_test) {
					execute_test = 0;
					DPTX_MCU_Reset();

					printf("Stop current measurement !\n\n\n\n\n\n");
				}

				/* Clear flag */
				flag = 0;

				Current_Item = received;

				switch (received) {
				case 'a':
					flag = (F_EMPHASIS_NULL | F_PAT_PRBS7);
					break;
#ifdef INTERNAL
				case 'b':
					flag = (F_EMPHASIS_1 | F_PAT_PRBS7);
					break;

				case 'c':
					flag = (F_EMPHASIS_1 | F_PAT_PLTPAT);
					break;

				case 'd': /* Non-Transition Voltage Range Measurement - PLTPAT */
					flag = (F_EMPHASIS | F_PAT_PLTPAT);
					break;

				case 'e': /* Pre-Emphasis Level Test and Pre-Emphasis Level Delta Test- PRBS7 */
					flag = (F_EMPHASIS_1 | F_PAT_PRBS7);
					break;

				case 'f': /* Non-Transition Voltage Range Measurement - PRBS7 */
					flag = (F_EMPHASIS | F_PAT_PRBS7);
					break;

				case 'g': /* Total - PRBS7 */
					flag = (F_EMPHASIS_1 | F_PAT_D10_2);
					break;

				case 'h':
					printf("Change the Swing value from request\n");
					flag = (F_EMPHASIS_1 | F_PAT_PRBS7 | F_SHOW_SWING);
					break;

				case 'i':
					printf("Change the Swing value from request\n");
					flag = (F_EMPHASIS_1 | F_PAT_PLTPAT | DP_TX_HIGH_SPEED | F_SHOW_SWING);
					break;

				case 'j':
					flag = F_PAT_AUX;
					break;
#endif
				case 'x': /* Auto hand shaking with DPR-100 */
					flag = F_EXE_AUTO;
					break;

				default:
					printf("Non - define command!\n");
					break;
				}

				// Execute testing
				if (flag) {
					execute_test = 1;
					GFlag = flag;

					if (flag & F_PAT_AUX) {
						printf("######################################\n");
						printf("#Current DP TX setting is shown below#\n");
						printf("######################################\n\n");

						DPPHYTX_Show_Item(Current_Item);
						Apply_AUX_Mesument(flag);
					} else if (flag & F_EXE_AUTO) {
						printf("#########################\n");
						printf("#Enter DP TX Auto Test!!#\n");
						printf("#########################\n\n");

						Apply_Auto_Preset(0x1);
						Apply_Auto_Mesument();
						Apply_Auto_Preset(0x0);

						printf("#########################\n");
						printf("#Leave DP TX Auto Test!!#\n");
						printf("#########################\n\n");
					} else {
						DPPHYTX_Show_Cfg();
						Apply_Main_Mesument(flag);
					}
				}
			}
		}
		mdelay(200);
	};

	printf("\n\n");
	return 0;
}

/* Temp for cording here */
void Apply_Auto_Preset(char Init)
{
	/* Fill some nessary register value for auto test */
	if (Init) {
		/* Enable MCU received interrupt */
		writel(0x00e80000, DP_TX_INT_CLEAR);

		/* Set HPD irq time interval */
		writel(0x04e300fb, DP_TX_IRQ_CFG);

		/* Set HPD event time interval */
		writel(0x000007d1, DP_TX_EVENT_CFG);
	} else {
		/* Recover */
		writel(0x0, DP_TX_INT_CLEAR);
		writel(0x0, DP_TX_IRQ_CFG);
		writel(0x0, DP_TX_EVENT_CFG);
	}
}

/* READ EDID */
void Read_EDID_128(char Offset)
{
	char AUX_Data[16] = {0};
	uchar Length = 1;
	uchar Status = 0;
	uchar i, j;

	DBG(DBG_A_EDID, "R EDID Offset %d\n", Offset);

	AUX_W(I2C_M_EA_CMD_W, 0x50, (int *)(&AUX_Data), &Length, &Status);

	AUX_Data[0] = Offset;
	AUX_W(I2C_M_CMD_W, 0x50, (int *)(&AUX_Data), &Length, &Status);

	AUX_R(I2C_M_EA_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);

	Length = 16;

	// Read 128 bytes block
	for (i = 0; i < 8; i++) {
		do {
			AUX_R(I2C_M_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_EDID, "EDID read %d!\n", i);

			udelay(100);
		} while (Status == 0x20);

		/* copy from temp into EDID */
		for (j = 0; j < 16; j++)
			EDID[Offset + i * 16 + j] = AUX_Data[j];
	}

	Length = 1;
	AUX_R(I2C_EA_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);
}

void Apply_EDID_Reading(void)
{
	char AUX_Data[16] = {0};
	uchar Length = 1;
	uchar Status = 0;

	DBG(DBG_STAGE, "Apply EDID Reading!\n");

	AUX_W(I2C_M_EA_CMD_W, 0x50, (int *)(&AUX_Data), &Length, &Status);

	AUX_W(I2C_M_CMD_W, 0x50, (int *)(&AUX_Data), &Length, &Status);

	AUX_R(I2C_M_EA_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);

	/* Check read EDID header */
	Length = 4;
	do {
		AUX_R(I2C_M_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);

		udelay(100);
	} while (Status == 0x20);

	DBG(DBG_A_EDID, "EDID First 4 is 0x%X_0x%X_0x%X_0x%X\n", AUX_Data[0], AUX_Data[1], AUX_Data[2], AUX_Data[3]);

	Length = 1;
	AUX_R(I2C_EA_CMD_R, 0x50, (int *)(&AUX_Data), &Length, &Status);

	DBG(DBG_A_EDID, "Read 128!\n");
	Read_EDID_128(0x0);

	/* If the extension bit is set, then read back next block */
	if (EDID[0x7E] == 0x1) {
		DBG(DBG_A_EDID, "Read 256!\n");
		Read_EDID_128(0x80);
	}
}

/* LINK TRAIN */
uchar Adjust_CR_EQ_Train(int TrainPat, uchar ADJStatus)
{
	char AUX_Data[16] = {0};
	uchar Ret = 0;
	uchar Length = 1;
	uchar Status = 0;
	uchar AUX_R_Level = 0;
	uchar AUX_W_Level = 0;
	uchar DE_Level = 0x0;
	uchar CR_Status = 0;
	int value = 0;
	uchar bFirst = 1;
	uchar tempkeep = 0;

	/* Set the main link pattern with TPS1 / TPS2 for Lanex_CR_Done */
	value = ((readl(DP_TX_MAIN_PAT) & ~(0x30000000)) | TrainPat);
	writel(value, DP_TX_MAIN_PAT);
	DBG(DBG_A_SUB, "1.A_C_E Set Link Pattern\n");

	/* AST phy 0xE4 Bit28 (0x1000000) / DP 0x102 Bit0 : TPS1 */
	/* AST phy 0xE4 Bit29 (0x2000000) / DP 0x102 Bit1 : TPS2 */
	if (TrainPat == 0x10000000)
		AUX_Data[0]  = 0x21;
	else
		AUX_Data[0]  = 0x22;

	AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "2.A_C_E W 0x102 set 0x%x\n", AUX_Data[0]);

	/* First */
	if (bFirst) {
		Length = 4;

		AUX_Data[0]  = CR_EQ_Keep;
		AUX_Data[1]  = CR_EQ_Keep;
		AUX_Data[2]  = CR_EQ_Keep;
		AUX_Data[3]  = CR_EQ_Keep;
		tempkeep = CR_EQ_Keep;

		do {
			AUX_W(AUX_CMD_W, 0x103, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_SUB, "3.A_C_E W 0x103 - 0x106 set\n");
		} while (Status == 0x20);

		Length = 6;

		udelay(1200);

		AUX_R(AUX_CMD_R, 0x200, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_SUB, "4.A_C_E R 0x200 - 0x205 read back\n");

		if ((AUX_Data[2] & ADJStatus) == ADJStatus) {
			CR_EQ_Keep = tempkeep;
			return Ret;
		}

		bFirst = 0;
	}

	/* loop for CR_lock */
	do {
		Length = 1;

		AUX_R(AUX_CMD_R, 0x206, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_SUB, "5.A_C_E R 0x206 read back\n");
		AUX_R_Level = AUX_Data[0];

		AUX_R(AUX_CMD_R, 0x207, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_SUB, "6.A_C_E R 0x207 read back\n");

		/* Update SW Level */
		switch (AUX_R_Level & 0x33) {
		case 0x00:
			AUX_W_Level = 00;
			break;
		case 0x11:
			AUX_W_Level = 01;
			break;
		default:
			AUX_W_Level = 06;
			break;
		}

		/* Update SW Level */
		switch (AUX_R_Level & 0xCC) {
		case 0x00:
			/* AUX_W_Level |= 00; */
			DE_Level = 0;
			break;
		case 0x44:
			AUX_W_Level  |= 0x08;
			DE_Level = 1;
			break;
		default:
			AUX_W_Level  |= 0x30;
			DE_Level = 2;
			break;
		}

		/* Set the de-emphsis value */
		value = ((readl(DP_TX_PHY_CFG) & ~(0x33000000)) | Deemlevel[DE_Level]);
		writel(value, DP_TX_PHY_CFG);
		DBG(DBG_A_SUB, "6.A_C_E Set Phy config %d\n", DE_Level);

		DBG(DBG_A_SUB, "Link AUX_W_Level is 0x%x\n", AUX_W_Level);

		Length = 4;

		AUX_Data[0]  = AUX_W_Level;
		AUX_Data[1]  = AUX_W_Level;
		AUX_Data[2]  = AUX_W_Level;
		AUX_Data[3]  = AUX_W_Level;
		tempkeep = AUX_W_Level;

		do {
			AUX_W(AUX_CMD_W, 0x103, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_SUB, "7.A_C_E W 0x103 - 0x106 set\n");
		} while (Status == 0x20);

		udelay(1380);

		Length = 6;
		AUX_R(AUX_CMD_R, 0x200, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_SUB, "8.A_C_E R 0x200 - 0x205 read back\n");

		CR_Status = AUX_Data[2] & ADJStatus;

		/* Decrease speed because the deemphasis level reach max value */
		if (AUX_W_Level == 0x36) {
			Ret = 1;
			break;
		}

	} while (CR_Status != ADJStatus);

	CR_EQ_Keep = tempkeep;

	return Ret;
}

uchar Link_Train_Flow(char Link_Level)
{
	char AUX_Data[16] = {0};
	uchar Length = 1;
	uchar Status = 0;
	uchar Ret = 0;
	int value = 0;

	DBG(DBG_STAGE, "Link train flow! Level : %d\n", Link_Level);

	AUX_R(AUX_CMD_R, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "1.Link R 0x101 read back\n");

	/* Normal / Test case */
	if (Auto_Lane_Count)
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | Auto_Lane_Count);
	else
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | 0x2);

	AUX_W(AUX_CMD_W, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "2.Link W 0x101 clear\n");

	/* Set different clock bit rate */
	value = ((readl(DP_TX_MAIN_SET) & ~(0x300)) | (Link_Level << 8));
	writel(value, DP_TX_MAIN_SET);

	switch (Link_Level) {
	case 0x1: /* 2.7 */
		AUX_Data[0] = 0x0a;
		break;

	default: /* 1.62 */
		AUX_Data[0] = 0x06;
		break;
	}

	AUX_W(AUX_CMD_W, 0x100, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "3.Link W 0x100 set\n");

	AUX_R(AUX_CMD_R, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "4.Link R 0x101 read back\n");

	/* Normal / Test case */
	if (Auto_Lane_Count)
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | Auto_Lane_Count);
	else
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | 0x2);

	AUX_W(AUX_CMD_W, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "5.Link W 0x101 clear\n");

	/* Set the 2 lanes and enhance frame by checking AUX 0x2 bit 7 */
	value = ((readl(DP_TX_MAIN_SET) & ~(0x1070)) | 0x20);

	if (bEn_Frame)
		value |= 0x1000;

	writel(value, DP_TX_MAIN_SET);

	value = ((readl(DP_TX_PHY_CFG) & ~(0x3000)) | 0x3000);

	writel(value, DP_TX_PHY_CFG);

	AUX_R(AUX_CMD_R, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "6.Link R 0x101 read back\n");

	/* Normal / Test case */
	if (Auto_Lane_Count)
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | Auto_Lane_Count);
	else
		AUX_Data[0] = ((AUX_Data[0] & 0xE0) | 0x2);

	AUX_W(AUX_CMD_W, 0x101, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_SUB, "7.Link W 0x101 clear\n");

	/* Set the main link control on */
	value = ((readl(DP_TX_PHY_SET) & ~(0x100)) | 0x100);

	udelay(1000);
	udelay(1500);

	writel(value, DP_TX_PHY_SET);

	do {
		value = (readl(DP_TX_PHY_SET) & 0x200);
	} while (value != 0x200);
	DBG(DBG_A_SUB, "8.Link Main Link Ready\n");

	/* Adjust for CR */
	if (Adjust_CR_EQ_Train(0x10000000, 0x11)) {
		Ret = 1;
		return Ret;
	}

	/* Adjust for EQ */
	if (Adjust_CR_EQ_Train(0x20000000, 0x77)) {
		Ret = 1;
		return Ret;
	}

	return Ret;
}

void Apply_HPD_Normal(void)
{
	char AUX_Data[16] = {0};
	uchar Length = 1;
	uchar Status = 0;
	int value = 0;

	DBG(DBG_STAGE, "HPD Normal set!\n");

	AUX_Data[0] = 0x01;
	AUX_W(AUX_CMD_W, 0x600, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "1.HPD_N W 0x600 set power!\n");

	AUX_R(AUX_CMD_R, 0x0, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "2.HPD_N R 0x0 read back is 0x%X!\n", AUX_Data[0]);

	Length = 8;
	AUX_R(AUX_CMD_R, 0x500, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "3.HPD_N R 0x500 - 0x508 read back\n");

	Length = 14;
	AUX_R(AUX_CMD_R, 0x0, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "4.HPD_N R 0x0 - 0xD read back\n");

	bEn_Frame = AUX_Data[2] & 0x80;
	Link_Aux_RD_Val = AUX_Data[14];

	if (bEn_Frame)
		DBG(DBG_NOR, "4.HPD_N R 0x2 En_Frame_Cap\n");

	Length = 1;
	AUX_R(AUX_CMD_R, 0xe, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "5.HPD_N R 0xE read back\n");

	/* Read EDID */
	DBG(DBG_NOR, "6.HPD_N Apply_EDID_Reading Enter\n");

	Apply_EDID_Reading();

	DBG(DBG_NOR, "6.HPD_N Apply_EDID_Reading Leave\n");

	Length = 2;
	AUX_R(AUX_CMD_R, 0x200, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "7.HPD_N R 0x200 - 0x201 read back.\n");

	Length = 1;
	AUX_R(AUX_CMD_R, 0x68028, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "8.HPD_N R 0x68028 read back.\n");

	AUX_R(AUX_CMD_R, 0x68028, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "9.HPD_N R 0x68028 read back.\n");

	AUX_R(AUX_CMD_R, 0x600, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "10.HPD_N R 0x600 read back.\n");

	AUX_Data[0] = 0x01;
	AUX_W(AUX_CMD_W, 0x600, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "11.HPD_N W 0x600 set power!\n");

	DBG(DBG_NOR, "12.HPD_N Link_Train_Flow 0x1 Enter\n");

	Status = Link_Train_Flow(0x1);

	DBG(DBG_NOR, "12.HPD_N Link_Train_Flow 0x1 Leave\n");

	if (Status) {
		AUX_Data[0] = 0x20;
		AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_ERR, "!!HPD_N W 0x102 set Train Flow 0x1 Fail!!\n");

		DBG(DBG_NOR, "13.HPD_N Link_Train_Flow 0x0 Enter\n");

		Status = Link_Train_Flow(0x0);

		DBG(DBG_NOR, "13.HPD_NLink_Train_Flow 0x0 Leave\n");

		if (Status) {
			AUX_Data[0] = 0x20;
			AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_ERR, "!!HPD_N W 0x102 set Train Flow 0x0 Fail!!\n");

			DBG(DBG_ERR, "### CAUTION!! LINK TRAN FAIL!! ###\n");

			return;
		}
	}

	/* Link successful */
	AUX_Data[0] = 0x00;
	AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "14.HPD_N W 0x102 clear!\n");

	/* Fill idle pattern */
	value = ((readl(DP_TX_MAIN_PAT) & ~(0x31000000)) | 0x01000000);
	writel(value, DP_TX_MAIN_PAT);
	DBG(DBG_NOR, "15.HPD_N Fill idle pattern!\n");

	Length = 1;
	AUX_R(AUX_CMD_R, 0x68028, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "16.HPD_N R 0x68028 read back.\n");

	Length = 5;
	AUX_R(AUX_CMD_R, 0x68000, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "17.HPD_N R 0x68000 - 0x68004 read back.\n");

	Length = 1;
	AUX_R(AUX_CMD_R, 0x68028, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "18.HPD_N R 0x68028 read back.\n");

	AUX_R(AUX_CMD_R, 0x68028, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "19.HPD_N R 0x68028 read back.\n");

	AUX_R(AUX_CMD_R, 0x0, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_NOR, "20.HPD_N R 0x0 read back.\n");
}

void Apply_HPD_Auto_Test(void)
{
	char AUX_Data[16];
	uchar Length = 0;
	uchar Status = 0;
	uchar clear_auto_test = 0;
	uchar auto_test_link = 0;
	uchar auto_test_phy = 0;
	uchar swing0 = 0, preemphasis0 = 0;
	uchar swing1 = 0, preemphasis1 = 0;
	int flag = 0;
	char temp0 = 0, temp1 = 0;
	char temp206 = 0;

	DBG(DBG_STAGE, "HPD Auto test set!\n");

	/* Set power D0 */
	AUX_Data[0] = 0x01;
	Length = 1;
	AUX_W(AUX_CMD_W, 0x600, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_TEST, "1.HP_I W 0x600 done!\n");

	Length = 6;
	AUX_R(AUX_CMD_R, 0x200, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_TEST, "2.HP_I R 0x200 - 0x206 done!\n");

	Length = 1;
	AUX_R(AUX_CMD_R, 0x201, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_TEST, "3.HP_I R 0x201 done!\n");

	/* Obtain auto test */
	if (AUX_Data[0] & 0x2)
		clear_auto_test = 1;
	else
		clear_auto_test = 0;

	/* Read dummy */
	Length = 3;
	AUX_R(AUX_CMD_R, 0x202, (int *)(&AUX_Data), &Length, &Status);
	Length = 1;
	AUX_R(AUX_CMD_R, 0x101, (int *)(&AUX_Data), &Length, &Status);
	AUX_R(AUX_CMD_R, 0x200, (int *)(&AUX_Data), &Length, &Status);
	Length = 3;
	AUX_R(AUX_CMD_R, 0x202, (int *)(&AUX_Data), &Length, &Status);
	Length = 1;
	AUX_R(AUX_CMD_R, 0x205, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_TEST, "4. HP_I R Dummy!\n");

	AUX_R(AUX_CMD_R, 0x219, (int *)(&AUX_Data), &Length, &Status);
	DBG(DBG_A_TEST, "5. HP_I Link Rate R 0x219 : 0x%x\n", AUX_Data[0]);

	if (AUX_Data[0] == 0x06) {
		Auto_Link_Rate = AUX_Data[0];
		DP_Rate = DP_RATE_1_62;
		PRINT_RATE_1_62;
	} else if (AUX_Data[0] == 0x0a) {
		Auto_Link_Rate = AUX_Data[0];
		DP_Rate = DP_RATE_2_70;
		PRINT_RATE_2_70;
	} else if (AUX_Data[0] == 0x14) {
		DBG(DBG_ERR, "!!DON'T SET 5.4 bps !!\n");
		return;
	}

	if (clear_auto_test) {
		AUX_Data[0] = 0x02;
		AUX_W(AUX_CMD_W, 0x201, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_TEST, "1.HP_I CA W 0x201 clear auto!\n");
		clear_auto_test = 0;

		/* Fetch Testing data */
		AUX_R(AUX_CMD_R, 0x218, (int *)(&AUX_Data), &Length, &Status);
		DBG(DBG_A_TEST, "2.HP_I CA R 0x218 done!\n");

		/* Check auto test link flag */
		if (AUX_Data[0] & 0x1)
			auto_test_link = 1;
		else
			auto_test_link = 0;

		/* Check auto test phy flag */
		if (AUX_Data[0] & 0x8)
			auto_test_phy = 1;
		else
			auto_test_phy = 0;

		if (auto_test_link) {
			AUX_R(AUX_CMD_R, 0x219, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "1.HP_I TL R 0x219 : 0x%x\n", AUX_Data[0]);

			if (AUX_Data[0] == 0x06) {
				Auto_Link_Rate = AUX_Data[0];
				DP_Rate = DP_RATE_1_62;
				PRINT_RATE_1_62;
			} else if (AUX_Data[0] == 0x0a) {
				Auto_Link_Rate = AUX_Data[0];
				DP_Rate = DP_RATE_2_70;
				PRINT_RATE_2_70;
			} else if (AUX_Data[0] == 0x14) {
				DBG(DBG_ERR, "!!DON'T SET 5.4 bps !!\n");
				return;
			}

			AUX_R(AUX_CMD_R, 0x220, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "2.HP_I TL R 0x220 : 0x%x\n", AUX_Data[0]);
			Auto_Lane_Count = AUX_Data[0]  & 0x1F;

			AUX_Data[0] = 0x01;
			AUX_W(AUX_CMD_W, 0x260, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "3.HP_I TL W 0x260 test ACK\n");

			mdelay(95);

			/* Set power D0 */
			AUX_Data[0] = 0x01;
			AUX_W(AUX_CMD_W, 0x600, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "3.1 HP_I W 0x600 done!\n");

			switch (Auto_Link_Rate) {
			case 0x06:
				DBG(DBG_A_TEST, "4.HP_I TL Link_Train_Flow 1.62bps Enter\n");
				Status = Link_Train_Flow(0x0);
				DBG(DBG_A_TEST, "4.HP_I TL Link_Train_Flow 1.62bps Leave\n");
				break;

			case 0x0a:
				DBG(DBG_A_TEST, "4.HP_I TL Link_Train_Flow 2.70bps Enter\n");
				Status = Link_Train_Flow(0x1);
				DBG(DBG_A_TEST, "4.HP_I TL Link_Train_Flow 2.70bps Leave\n");
				break;

			default:
				DBG(DBG_ERR, "!!BAD LINK RATE!!\n");
				return;
			}

			if (Status) {
				DBG(DBG_ERR, "!!AUTO TEST LINK FAIL!!\n");
				return;
			}

			/* Link successful */
			AUX_Data[0] = 0x00;
			AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "5.HP_I TL Link clear!\n");

			auto_test_link = 0;
		}

		if (auto_test_phy) {
			Length = 1;
			flag = 0;
			temp206 = 0;

			AUX_R(AUX_CMD_R, 0x248, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "1.HP_I TP R 0x248 : 0x%x!\n", AUX_Data[0]);

			if (AUX_Data[0] == 0x01) {
				flag |= F_PAT_D10_2;
				DBG(DBG_A_TEST, "HP_I TP D10.2!\n");
			} else if (AUX_Data[0] == 0x03) {
				flag |= F_PAT_PRBS7;
				DBG(DBG_A_TEST, "HP_I TP PRBS7!\n");
			}

			AUX_R(AUX_CMD_R, 0x206, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "2.HP_I TP R 0x206 : 0x%x!\n", AUX_Data[0]);

			/* Temp for verified */
			DBG(DBG_INF, "Read value 0x206 : 0x%x!\n", AUX_Data[0]);

			/* Check Swing */
			temp0 = (AUX_Data[0] & 0x03);
			temp1 = (AUX_Data[0] & 0x30);

			/* Check Swing0 */
			switch (temp0) {
			case 0x2:
				swing0 = 0x2;
				temp206 |= 6;
				break;
			case 0x1:
				swing0 = 0x1;
				temp206 |= 1;
				break;
			case 0x0:
				swing0 = 0x0;
				break;
			default:
				DBG(DBG_ERR, "HP_I TP 0x206 other swing0 val %x!\n", temp0);
				break;
			}

			/* Check Swing1 */
			switch (temp1) {
			case 0x20:
				swing1 = 0x2;
				temp206 |= 6;
				break;
			case 0x10:
				swing1 = 0x1;
				temp206 |= 1;
				break;
			case 0x00:
				swing1 = 0x0;
				break;
			default:
				DBG(DBG_ERR, "HP_I TP 0x206 other swing1 val %x!\n", temp1);
				break;
			}

			if (swing0 != swing1)
				DBG(DBG_ERR, "Swing 0 / 1 diff val %x!\n", AUX_Data[0]);

			/* Check Pre-emphasis */
			temp0 = (AUX_Data[0] & 0x0C);
			temp1 = (AUX_Data[0] & 0xC0);

			/* Check Pre-emphasis0 */
			switch (temp0) {
			case 0x8:
				preemphasis0 = 0x2;
				temp206 |= 0x30;
				break;
			case 0x4:
				preemphasis0 = 0x1;
				temp206 |= 0x08;
				break;
			case 0x0:
				preemphasis0 = 0x0;
				break;
			default:
				DBG(DBG_ERR, "HP_I TP 0x206 other Pre-emphasis0 val %x!\n", temp0);
				break;
			}

			/* Check Pre-emphasis1 */
			switch (temp1) {
			case 0x80:
				preemphasis1 = 0x2;
				temp206 |= 0x30;
				break;
			case 0x40:
				preemphasis1 = 0x1;
				temp206 |= 0x08;
				break;
			case 0x00:
				preemphasis1 = 0x0;
				break;
			default:
				DBG(DBG_ERR, "HP_I TP 0x206 other Pre-emphasis1 val %x!\n", temp1);
				break;
			}

			if (preemphasis0 != preemphasis1)
				DBG(DBG_ERR, "Preemphasis 0 / 1 diff val %x!\n", AUX_Data[0]);

			/* Judgement */
			if (swing0 == 0x2 || swing1 == 0x2)
				Swing_Level = 0x2;
			else if (swing0 == 1 || swing1 == 0x1)
				Swing_Level = 0x1;
			else if (swing0 == 0x0 || swing1 == 0x0)
				Swing_Level = 0x0;

			if (preemphasis0 == 0x2 || preemphasis1 == 0x2) {
				Deemphasis_RD = DP_DEEMP_2;
				Deemphasis_Level_1 = DP_DEEMP_1;
				DBG(DBG_ERR, "!!De-type 1 P_2 !!\n");
			} else if (preemphasis0 == 0x1 || preemphasis1 == 0x1) {
				Deemphasis_RD = DP_DEEMP_1;
				Deemphasis_Level_1 = DP_DEEMP_0;
				DBG(DBG_ERR, "!!De-type 0 P_1 !!\n");
			} else if (preemphasis0 == 0x0 || preemphasis1 == 0x0) {
				Deemphasis_RD = DP_DEEMP_0;
				Deemphasis_Level_1 = DP_DEEMP_2;
				DBG(DBG_ERR, "!!De-type 2 P_0 !!\n");
			}

			DBG(DBG_INF, "!!Swing %d / Pre-emphasis %d !!\n", swing0, preemphasis0);

			flag |= F_EMPHASIS_1;

			AUX_R(AUX_CMD_R, 0x102, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "3.HP_I TP R 0x102 done!\n");
			temp0 = AUX_Data[0];

			Length = 2;
			AUX_R(AUX_CMD_R, 0x10b, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "4.HP_I TP R 0x10b done!\n");

			AUX_W(AUX_CMD_W, 0x10b, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "5.HP_I TP W 0x10b done!\n");

			AUX_Data[0] = temp0 | 0x20;
			Length = 1;
			AUX_W(AUX_CMD_W, 0x102, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "6.HP_I TP W 0x102 done!\n");

			AUX_Data[0] = temp206;
			AUX_Data[1] = temp206;
			Length = 2;

			do {
				AUX_W(AUX_CMD_W, 0x103, (int *)(&AUX_Data), &Length, &Status);
				DBG(DBG_A_TEST, "7.HP_I TP W 0x103 - 0x104 done!\n");
			} while (Status == 0x20);

			DPPHYTX_Show_Cfg();
			Apply_Main_Mesument(flag);

			Length = 1;
			AUX_Data[0] = 0x01;
			AUX_W(AUX_CMD_W, 0x260, (int *)(&AUX_Data), &Length, &Status);
			DBG(DBG_A_TEST, "8.HP_I TP W 0x260 done!\n");

			DBG(DBG_STAGE, "Leave Apply Auto Test\n");

			auto_test_phy = 0;
		}
		clear_auto_test = 0;
	}
}

/* TEST SECTION */
void Apply_Auto_Mesument(void)
{
	char auto_received = 0;
	int temp = 0, wdata = 0;
	int breakcount = 0;

	while (1) {
		breakcount = 0;

		/* Wait HPD */
		do {
			temp = (readl(DP_TX_INT_STATUS) & 0x3800);
			breakcount++;

			if (breakcount == 0x96000) {
				/* A simple break for esc press received */
				break;
			}

		} while (!(temp & 0x3800));

		if (temp) {
			/* Clear AUX write interrupt status */
			wdata = (readl(DP_TX_INT_CLEAR) | (temp >> 8));
			writel(wdata, DP_TX_INT_CLEAR);
		}

		/* Interrupt occur */
		if (temp & 0x2800) {
			/* Initial global parameter */
			Auto_Link_Rate = 0;
			Auto_Lane_Count = 0;
			bEn_Frame = 0;
			CR_EQ_Keep = 0;

			if (temp & 0x2000) {
				printf("DP HPD event is detected!\n");
				Apply_HPD_Normal();
			}

			if (temp & 0x800) {
				printf("DP HPD irq is detected!\n");
				Apply_HPD_Auto_Test();
			}
		}

		/* Leave auto test if the 'ESC' is pressed */
		if (tstc()) {
			auto_received = getc();

			/* Check the ESC key */
			if (auto_received == 27) {
				printf("'ESC' is pressed under auto test!\n\n");
				return;
			}

			printf("DP TX auto test is executed!\n");
		}
	}
}

void Apply_Main_Mesument(int flag)
{
	DPTX_MCU_Reset();

	/* Emphasis setting */
	if (flag & F_EMPHASIS_NULL)
		writel(PHY_Cfg_N, DP_TX_PHY_CFG);
	else if (flag & F_EMPHASIS)
		writel(PHY_Cfg, DP_TX_PHY_CFG);
	else if (flag & F_EMPHASIS_1)
		writel(PHY_Cfg_1, DP_TX_PHY_CFG);

	if (flag & F_RES_HIGH)
		writel(DP_TX_HIGH_SPEED, DP_TX_RES_CFG);
	else
		writel(DP_TX_NOR_SPEED, DP_TX_RES_CFG);

	writel(PHY_Cfg_1, DP_TX_PHY_CFG);

	DPPHY_Set();

	if (flag & F_PAT_PRBS7)
		Set_PRBS7();
	else if (flag & F_PAT_PLTPAT)
		Set_PLTPAT();
	else if (flag & F_PAT_HBR2CPAT)
		Set_HBR2CPAT();
	else if (flag & F_PAT_D10_2)
		Set_D10_1();

	/* ssc special patch */
	if (flag & F_PAT_D10_2) {
		/*Apply special patch*/
		writel(0x00000400, DP_TX_RES_CFG);
		TX_SSCG_Cfg |= DP_SSCG_ON;
	} else {
		/*Recover into original setting*/
		writel(0x00000000, DP_TX_RES_CFG);
	}

	writel(TX_SSCG_Cfg, DP_TX_PHY_SET);
}

void Apply_AUX_Mesument(int flag)
{
	DPTX_MCU_Reset();
	DPPHY_Set();

	writel(0x0F000000, DP_AUX_ADDR_LEN);
	writel(0x80000010, DP_AUX_REQ_CFG);
}

/* FUNCTION SECTION */
/* i2c set */
#ifdef RE_DRIVER
void Set_Redriver(void)
{
	int value = 0x0;
	uchar offset = 0x0;
	uchar *set_table = &set_table0;

	if (Deemphasis_RD == DP_DEEMP_1)
		set_table = &set_table1;
	else if (Deemphasis_RD == DP_DEEMP_2)
		set_table = &set_table2;

	RD_VAL = set_table[Swing_Level];

	printf("RD_VAL is 0x%x\n", RD_VAL);

	writel(0x600, I2C_BASE + I2C0_COUNT_O);
	value = (0x0000f0f0 | (RD_VAL << 24));
	writel(value, I2C_BUFF);
	printf("value0 is 0x%x\n", value);
	value = (RD_VAL | (RD_VAL << 8) | (RD_VAL << 16) | (RD_VAL << 24));
	writel(value, (I2C_BUFF + 0x4));
	printf("value1 is 0x%x\n", value);
	writel(0x70010063, (I2C_BASE + I2C0_EXECUTE_O));
	mdelay(1000);
}

/* i2c single initial */
void I2C_L_Initial(void)
{
	I2C_BASE = (I2C0_BASE + (I2C_DEV_OFFSET * I2C_PORT));
	I2C_BUFF = (I2C0_BUFF + (I2C_BUFF_OFFSET * I2C_PORT));

	writel(0x0, I2C_BASE);
	mdelay(1);
	writel(0x28001, I2C_BASE);
	writel(0x344001, I2C_BASE + I2C0_TIMMING_O);
	writel(0xFFFFFFFF, I2C_BASE + I2C0_INT_STATUS_O);
	writel(0x0, I2C_BASE + I2C0_INT_O);
	mdelay(10);
}

/* i2c golbal iniitial */
void I2C_G_Initial(void)
{
	/* i2c multi-function */
	writel(0x0FFF3000, MP_SCU410);
	writel(0x0F00FF00, MP_SCU414);
	writel(0xCFFF001F, MP_SCU418);
	writel(0xF00000FF, MP_SCU4b0);
	writel(0xF0FF00FF, MP_SCU4b4);
	writel(0x0000FF00, MP_SCU4b8);

	/* I2c control */
	writel(0x16, (I2C_GBASE + 0xC));
	writel(0x041230C6, (I2C_GBASE + 0x10));

	mdelay(1000);
}
#endif

void DPTX_MCU_Reset(void)
{
	/* Reset DPMCU & Release DPTX */
	writel(0x20000000, SYS_REST);
	writel(0x10000000, SYS_REST_CLR);

	/* Wait for apply setting */
	mdelay(1000);
}

void DPPHY_Set(void)
{
	int value = 0, count = 0;

	/* Clear power on reset */
	writel(0x10000000, DP_TX_PHY_SET);
	mdelay(1);

	/* Clear DPTX reset */
	writel(0x11000000, DP_TX_PHY_SET);
	mdelay(1);

	/* Turn on Main link / Aux channel */
	writel(0x11001100, DP_TX_PHY_SET);
	mdelay(1);

	while (value != DP_TX_RDY_TEST) {
		value =  readl(DP_TX_PHY_SET);
		mdelay(1);
		count++;
	}
}

char DPPHYTX_Show_Cfg(void)
{
	char SetFail = 0;

	PHY_Cfg		= DP_PHY_INIT_CFG;
	PHY_Cfg_1	= DP_PHY_INIT_CFG;
	TX_SSCG_Cfg	= DP_TX_RDY_TEST;

	/* Show the setting */

	printf("######################################\n");
	printf("#Current DP TX setting is shown below#\n");
	printf("######################################\n\n");

	DPPHYTX_Show_Item(Current_Item);

	switch (DP_Rate) {
	case DP_RATE_1_62:
		PRINT_RATE_1_62;
		break;

	case DP_RATE_2_70:
		PRINT_RATE_2_70;
		break;

	case DP_RATE_5_40:
		PRINT_RATE_5_40;
		break;

	default:
		PRINT_INVALID;
		printf("DP Rate\n");
		SetFail = 1;
		break;
	}

	switch (Deemphasis_Show) {
	case DP_DEEMP_0:
		if (GFlag & F_SHOW_SWING)
			PRINT_SWING_0;
		else
			PRINT_EMPVAL_0;
		break;

	case DP_DEEMP_1:
		if (GFlag & F_SHOW_SWING)
			PRINT_SWING_1;
		else
			PRINT_EMPVAL_1;
		break;

	case DP_DEEMP_2:
		if (GFlag & F_SHOW_SWING)
			PRINT_SWING_2;
		else
			PRINT_EMPVAL_2;
		break;

	default:
		PRINT_INVALID;
		printf("Deemphasis Level\n");
		SetFail = 1;
		break;
	}

	switch (Swing_Level) {
	case 0:
		PRINT_SWING_0;
		break;

	case 1:
		PRINT_SWING_1;
		break;

	case 2:
		PRINT_SWING_2;
		break;

	default:
		PRINT_INVALID;
		printf("Swing Level\n");
		SetFail = 1;
		break;
	}

	PHY_Cfg		= DP_PHY_INIT_CFG | (DP_Rate | Deemphasis_Level);
	PHY_Cfg_1	= DP_PHY_INIT_CFG | (DP_Rate | Deemphasis_Level_1);
	PHY_Cfg_N	= DP_PHY_INIT_CFG | (DP_Rate | DP_DEEMP_2);

	switch (SSCG) {
	case DP_SSCG_ON:
		/*PRINT_SSCG_ON;*/
		break;

	case DP_SSCG_OFF:
		/*PRINT_SSCG_OFF;*/
		break;

	default:
		PRINT_INVALID;
		printf("SSCG\n");
		SetFail = 1;
		break;
	}
	/* TX_SSCG_Cfg |= SSCG; */
	TX_SSCG_Cfg |= DP_SSCG_ON;

	printf("\n");

	return SetFail;
}

void DPPHYTX_Show_Item(char received)
{
	switch (received) {
	case 'a':
		PRINT_ITEM_A;
		break;

	case 'b':
		PRINT_ITEM_B;
		break;

	case 'c':
		PRINT_ITEM_C;
		break;

	case 'd':
		PRINT_ITEM_D;
		break;

	case 'e':
		PRINT_ITEM_E;
		break;

	case 'f':
		PRINT_ITEM_F;
		break;

	case 'g':
		PRINT_ITEM_G;
		break;

	case 'h':
		PRINT_ITEM_H;
		break;

	case 'i':
		PRINT_ITEM_I;
		break;

	case 'j':
		PRINT_ITEM_J;
		break;

	case 'x':
		PRINT_ITEM_X;
		break;

	default:
		break;
	}

	printf("\n");
}

void Set_PRBS7(void)
{
	writel(DP_TX_MAIN_NOR, DP_TX_MAIN_SET);
	writel((DP_PY_PAT | DP_PY_PAT_PRB7), DP_TX_PHY_PAT);
}

void Set_HBR2CPAT(void)
{
	int value = 0, count = 0;

	writel(DP_TX_MAIN_ADV, DP_TX_MAIN_SET);

	writel(DP_TX_PAT_HBR2, DP_TX_MAIN_PAT);
	writel((DP_PY_PAT | DP_PY_PAT_SCRB), DP_TX_PHY_PAT);

	writel(DP_TX_MAIN_TRA, DP_TX_MAIN_CFG);
	mdelay(1);

	while (value != DP_TX_RDY_25201) {
		value =  (readl(DP_TX_MAIN_CFG) & 0xFFF);
		mdelay(1);
		count++;
	}

	/* Reset for signal apply */
	writel((DP_TX_MAIN_ADV | DP_TX_PY_RESET), DP_TX_MAIN_SET);
}

void Set_PLTPAT(void)
{
	writel(DP_TX_MAIN_NOR, DP_TX_MAIN_SET);

	writel(DP_TX_PLTPAT_0, DP_TX_CUS_PAT_0);
	writel(DP_TX_PLTPAT_1, DP_TX_CUS_PAT_1);
	writel(DP_TX_PLTPAT_2, DP_TX_CUS_PAT_2);

	writel((DP_PY_PAT | DP_PY_PAT_CUS), DP_TX_PHY_PAT);
}

void Set_D10_1(void)
{
	writel(DP_TX_MAIN_NOR, DP_TX_MAIN_SET);

	writel(DP_TX_PAT_TPS1, DP_TX_MAIN_PAT);
	writel(DP_PY_PAT, DP_TX_PHY_PAT);
}

uchar AUX_R(int aux_cmd, int aux_addr, int *aux_r_data, uchar *length, uchar *status)
{
	int wdata = 0,  temp = 0;
	uchar len = *length;

	/* Check valid length */
	if (len >= 1)
		len -= 1;
	else
		return 1;

	/* Prepare AUX write address and data */
	wdata = (int)((len << 24) | aux_addr);
	writel(wdata, DP_AUX_ADDR_LEN);

	DBG(DBG_AUX_R, "AUX Read on 0x%x with %d bytes.\n", aux_addr, *length);

	/* Fire AUX read */
	writel(aux_cmd, DP_AUX_REQ_CFG);

	/* Wait AUX read finish or timeout */
	do {
		temp = (readl(DP_TX_INT_STATUS) & 0xC000);
	} while (!(temp & 0xC000));

	/* Clear AUX write interrupt status */
	wdata = (readl(DP_TX_INT_CLEAR) | (temp >> 8));
	writel(wdata, DP_TX_INT_CLEAR);

	if (temp & AUX_CMD_DONE) {
		/* Read back data count */
		*aux_r_data = readl(DP_AUX_R_D_0);

		DBG(DBG_AUX_R, "Data on 0x0 is 0x%x.\n", *aux_r_data);

		if ((*length) > 0x4) {
			aux_r_data++;
			*aux_r_data = readl(DP_AUX_R_D_4);
			DBG(DBG_AUX_R, "Data on 0x4 is 0x%x.\n", *aux_r_data);
		}

		if ((*length) > 0x8) {
			aux_r_data++;
			*aux_r_data = readl(DP_AUX_R_D_8);
			DBG(DBG_AUX_R, "Data on 0x8 is 0x%x.\n", *aux_r_data);
		}

		if ((*length) > 0xC) {
			aux_r_data++;
			*aux_r_data = readl(DP_AUX_R_D_C);
			DBG(DBG_AUX_R, "Data on 0xC is 0x%x.\n", *aux_r_data);
		}

		(*status) = (uchar)(readl(DP_AUX_STATUS) & 0xFF);
		return 0;
	}	else {
		return 1;
	}
}

uchar AUX_W(int aux_cmd, int aux_addr, int *aux_w_data, uchar *length, uchar *status)
{
	int wdata = 0, temp = 0;
	uchar len = *length;

	/* Check valid length */
	if (len >= 1)
		len -= 1;
	else
		return 1;

	/* Prepare AUX write address and data */
	wdata = (int)((len << 24) | aux_addr);
	writel(wdata, DP_AUX_ADDR_LEN);

	writel(*aux_w_data, DP_AUX_W_D_0);

	if ((*length) > 0x4) {
		aux_w_data++;
		writel(*aux_w_data, DP_AUX_W_D_4);
	}

	if ((*length) > 0x8) {
		aux_w_data++;
		writel(*aux_w_data, DP_AUX_W_D_8);
	}

	if ((*length) > 0xC) {
		aux_w_data++;
		writel(*aux_w_data, DP_AUX_W_D_C);
	}

	/* Fire AUX write */
	writel(aux_cmd, DP_AUX_REQ_CFG);

	/* Wait AUX write finish or timeout */
	do {
		temp = (readl(DP_TX_INT_STATUS) & 0xC000);
	} while (!(temp & 0xC000));

	/* Clear AUX write interrupt status */
	wdata = (readl(DP_TX_INT_CLEAR) | (temp >> 8));
	writel(wdata, DP_TX_INT_CLEAR);

	if (temp & AUX_CMD_DONE) {
		(*status) = (uchar)(readl(DP_AUX_STATUS) & 0xFF);
		return 0;
	} else {
		return 1;
	}
}

U_BOOT_CMD(dptest, 2, 0, do_ast_dptest, "ASPEED Display Port phy test", "ASPEED DP test v.0.2.9");
