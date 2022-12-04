/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef SWFUNC_H
#define SWFUNC_H

#include <config.h> // for uboot system

//---------------------------------------------------------
// Program information
//---------------------------------------------------------
/* PHY in Normal mode */
#define VER_NAME "Ver 0.77 version @2017/12/20 1310"

/* ========================================================== */
#define NETESTCMD_MAX_ARGS CONFIG_SYS_MAXARGS

/* == Step 4:==========   Select PHY    ================== */

/* ======================== Program flow control ======================== */
#define RUN_STEP                                 5
// 1: parameter setup
// 2: mdc/mdio pinmux,
// 4: Data Initial
// 5: ALL

/* ====================== Switch print debug message ====================== */
//#define   DbgPrn_Enable_Debug_pause                //[off]
//#define DBG_LOG_FUNC_NAME
#define DBG_PRINT_ERR_FLAG		0
#define DbgPrn_BufAdr			0
#define DbgPrn_Bufdat			0
#define DbgPrn_BufdatDetail		0
#define DbgPrn_PHYRW			0
#define DbgPrn_PHYInit			0
#define DbgPrn_DumpMACCnt		0
#define DbgPrn_Info			    0
#define DbgPrn_FRAME_LEN		0

#ifdef DBG_LOG_FUNC_NAME
#define nt_log_func_name()				\
			do{printf("%s\n", __func__); debug_pause();}while(0)
#else
#define nt_log_func_name(...)
#endif
/* ============ Enable or Disable Check item of the descriptor ============ */
    #define CheckRxOwn
    #define CheckRxLen
    #define CheckRxErr
    #define CheckCRC
    #define CheckFTL
    #define CheckRunt
//    #define CheckOddNibble
    #define CheckRxFIFOFull
  

//    #define CheckRxbufUNAVA
    #define CheckRPktLost
//    #define CheckNPTxbufUNAVA
    #define CheckTPktLost
    #define CheckRxBuf
    //#define CHECK_RX_DATA


/* error mask of the RX descriptor */
#define RXDES_EM_RXERR			BIT(18)
#define RXDES_EM_CRC			BIT(19)
#define RXDES_EM_FTL			BIT(20)
#define RXDES_EM_RUNT			BIT(21)
#define RXDES_EM_ODD_NB			BIT(22)
#define RXDES_EM_FIFO_FULL		BIT(23)
#define RXDES_EM_ALL                                                           \
	(RXDES_EM_RXERR | RXDES_EM_CRC | RXDES_EM_FTL | RXDES_EM_RUNT |        \
	 RXDES_EM_ODD_NB | RXDES_EM_FIFO_FULL)

#endif // SWFUNC_H
