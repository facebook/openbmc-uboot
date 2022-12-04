/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef MAC_H
#define MAC_H


#include <common.h>
#include <command.h>

#include "ncsi.h"
#include <asm/io.h>

// --------------------------------------------------------------
// Define
// --------------------------------------------------------------
//#define Enable_MAC_SWRst               //[off]
#define   Enable_No_IOBoundary         //[ON]
//#define Enable_Dual_Mode               //[off]

//#define Enable_Runt
//#define Enable_ShowBW

#define TX_DELAY_SCALING			2
#define RX_DELAY_SCALING			2

//#define SelectSimpleBoundary                                    //[off] Using in debug
//#define SelectSimpleData                                        //[off] Using in debug
//#define SelectSimpleLength                       1514           //[off] 60(0x3c) ~ 1514(0x5ea); 1512(0x5e8)
//#define SelectSimpleDA                                          //[off] Using in debug
//#define SelectSimpleDes                                         //[off]
//#define SelectLengthInc                                         //[off] Using in debug

#define   SimpleData_Fix                                        //[ON] Using in debug
#define     SimpleData_FixNum                    12
#define     SimpleData_FixVal00                  0x00000000     //[0]no SelectSimpleDA: (60: 0412 8908)(1512: e20d e9da)
#define     SimpleData_FixVal01                  0xffffffff     //[0]no SelectSimpleDA: (60: f48c f14d)(1512: af05 260c)
#define     SimpleData_FixVal02                  0x55555555     //[0]no SelectSimpleDA: (60: 5467 5ecb)(1512: d90a 5368)
#define     SimpleData_FixVal03                  0xaaaaaaaa     //[0]no SelectSimpleDA: (60: a4f9 268e)(1512: 9402 9cbe)
#define     SimpleData_FixVal04                  0x5a5a5a5a     //[1]no SelectSimpleDA: (60: 7f01 e22d)(1512: 4fd3 8012)
#define     SimpleData_FixVal05                  0xc3c3c3c3     //[1]no SelectSimpleDA: (60: 5916 02d5)(1512: 99f1 6127)
#define     SimpleData_FixVal06                  0x96969696     //[1]no SelectSimpleDA: (60: 0963 d516)(1512: a2f6 db95)
#define     SimpleData_FixVal07                  0xf0f0f0f0     //[1]no SelectSimpleDA: (60: dfea 4dab)(1512: 39dc f576)
#define     SimpleData_FixVal08                  0x5555aaaa     //[2]no SelectSimpleDA: (60: b61b 5777)(1512: 4652 ddb0)
#define     SimpleData_FixVal09                  0xffff0000     //[2]no SelectSimpleDA: (60: 16f0 f8f1)(1512: 305d a8d4)
#define     SimpleData_FixVal10                  0x5a5aa5a5     //[2]no SelectSimpleDA: (60: 9d7d eb91)(1512: d08b 0eca)
#define     SimpleData_FixVal11                  0xc3c33c3c     //[2]no SelectSimpleDA: (60: bb6a 0b69)(1512: 06a9 efff)

#define   SimpleData_XORVal                      0x00000000
//#define   SimpleData_XORVal                    0xffffffff

#define   SelectSimpleDA_Dat0                    0x67052301
#define   SelectSimpleDA_Dat1                    0xe0cda089
#define   SelectSimpleDA_Dat2                    0x98badcfe

#define   SelectWOLDA_DatH                       0x206a
#define   SelectWOLDA_DatL                       0x8a374d9b

/* MByte per second to move data */
#define MOVE_DATA_MB_SEC			800

//---------------------------------------------------------
// Frame size
//---------------------------------------------------------
#define ENABLE_RAND_SIZE                         0
#define   RAND_SIZE_SED                          0xffccd
#define   RAND_SIZE_SIMPLE                       0
#define   RAND_SIZE_MIN                          60
#define   RAND_SIZE_MAX                          1514

#define FRAME_SELH_PERD                          7
  #ifdef SelectSimpleLength
//    #define FRAME_LENH                           ( SelectSimpleLength + 1 )
//    #define FRAME_LENL                           ( SelectSimpleLength     )
    #define FRAME_LENH                           SelectSimpleLength
    #define FRAME_LENL                           SelectSimpleLength
  #else
//    #define FRAME_LENH                           1514           //max:1514
//    #define FRAME_LENL                           1513           //max:1514
    #define FRAME_LENH                           1514           //max:1514
    #define FRAME_LENL                           1514           //max:1514
  #endif

#endif // MAC_H
