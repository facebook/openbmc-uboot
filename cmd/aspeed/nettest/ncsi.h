/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef NCSI_H
#define NCSI_H


#define NCSI_RxDMA_PakSize                       2048
#define NCSI_RxDMA_BASE                          ( DMA_BASE + 0x00100000 )

//---------------------------------------------------------
// Define
//---------------------------------------------------------
#define MAX_PACKAGE_NUM                          8      // 1 ~ 8
#define MAX_CHANNEL_NUM                          4      // 1 ~ 32

//---------------------------------------------------------
// Function
//---------------------------------------------------------
#define SENT_RETRY_COUNT                         1
#define NCSI_RxDESNum                            2048

#define NCSI_Skip_RxCRCData
//#define NCSI_Skip_Phase1_DeSelectPackage
#define NCSI_Skip_DeSelectPackage
//#define NCSI_Skip_DiSChannel
//#define NCSI_EnableDelay_DeSelectPackage
//#define NCSI_EnableDelay_GetLinkStatus
//#define NCSI_EnableDelay_EachPackage
//#define NCSI_VERBOSE_TEST
//#define Print_Version_ID
//#define Print_PackageName
#define Print_DetailFrame

//---------------------------------------------------------
// Delay (ms)
//---------------------------------------------------------
#define Delay_EachPackage                        1000
#define Delay_DeSelectPackage                    50
#define Delay_GetLinkStatus                      50

//---------------------------------------------------------
// PCI DID/VID & Manufacturer ID
//---------------------------------------------------------
#define ManufacturerID_Intel                     0x00000157     //343
#define ManufacturerID_Broadcom                  0x0000113d     //4413
#define ManufacturerID_Mellanox                  0x000002c9     //713
#define ManufacturerID_Mellanox1                 0x00008119     //33049
#define ManufacturerID_Emulex                    0x0000006c     //108

//PCI VID: [163c]intel
//PCI VID: [8086]Intel Corporation
//PCI VID: [8087]Intel
//PCI VID: [14e4]Broadcom Corporation
//PCI VID: [15b3]Mellanox
//PCI VID: [10df]Emulex
#define PCI_DID_VID_Intel_82574L                 0x10d38086     // IntelR 82574L Gigabit Ethernet Controller
#define PCI_DID_VID_Intel_82575_10d6             0x10d68086     // 82566 DM-2-gigabyte
#define PCI_DID_VID_Intel_82575_10a7             0x10a78086     // 82575EB Gigabit Network Connection
#define PCI_DID_VID_Intel_82575_10a9             0x10a98086     // 82575EB Gigabit Network Connection
#define PCI_DID_VID_Intel_82576_10c9             0x10c98086     //*82576 Gigabit ET Dual Port Server Adapter
#define PCI_DID_VID_Intel_82576_10e6             0x10e68086     // 82576 Gigabit Network Connection
#define PCI_DID_VID_Intel_82576_10e7             0x10e78086     // 82576 Gigabit Network Connection
#define PCI_DID_VID_Intel_82576_10e8             0x10e88086     // E64750-xxx Intel Gigabit ET Quad Port Server Adapter
#define PCI_DID_VID_Intel_82576_1518             0x15188086     // 82576NS SerDes Gigabit Network Connectio
#define PCI_DID_VID_Intel_82576_1526             0x15268086     // Intel Gigabit ET2 Quad Port Server Adapter
#define PCI_DID_VID_Intel_82576_150a             0x150a8086     // 82576NS Gigabit Ethernet Controller
#define PCI_DID_VID_Intel_82576_150d             0x150d8086     // 82576 Gigabit Backplane Connection
#define PCI_DID_VID_Intel_82599_10fb             0x10fb8086     // 10 Gb Ethernet controller
#define PCI_DID_VID_Intel_82599_1557             0x15578086     // 82599EN
#define PCI_DID_VID_Intel_I210_1533              0x15338086     //
#define PCI_DID_VID_Intel_I210_1537              0x15378086     //???
#define PCI_DID_VID_Intel_I350_1521              0x15218086     //
#define PCI_DID_VID_Intel_I350_1523              0x15238086     //
#define PCI_DID_VID_Intel_X540                   0x15288086     //
#define PCI_DID_VID_Intel_X550                   0x15638086     //
#define PCI_DID_VID_Intel_Broadwell_DE           0x15ab8086     //PCH
#define PCI_DID_VID_Intel_X722_37d0              0x37d08086     //
#define PCI_DID_VID_Broadcom_BCM5718             0x165614e4     //
#define PCI_DID_VID_Broadcom_BCM5719             0x165714e4     //
#define PCI_DID_VID_Broadcom_BCM5720             0x165f14e4     //
#define PCI_DID_VID_Broadcom_BCM5725             0x164314e4     //
#define PCI_DID_VID_Broadcom_BCM57810S           0x168e14e4     //
#define PCI_DID_VID_Broadcom_Cumulus             0x16ca14e4     //
#define PCI_DID_VID_Broadcom_BCM57302            0x16c914e4     //
#define PCI_DID_VID_Broadcom_BCM957452           0x16f114e4     //
#define PCI_DID_VID_Mellanox_ConnectX_3_1003     0x100315b3     //*
#define PCI_DID_VID_Mellanox_ConnectX_3_1007     0x100715b3     //ConnectX-3 Pro
#define PCI_DID_VID_Mellanox_ConnectX_4          0x101515b3     //*
#define PCI_DID_VID_Emulex_40G                   0x072010df     //

//---------------------------------------------------------
// NCSI Parameter
//---------------------------------------------------------
//Command and Response Type
#define CLEAR_INITIAL_STATE                      0x00           //M
#define SELECT_PACKAGE                           0x01           //M
#define DESELECT_PACKAGE                         0x02           //M
#define ENABLE_CHANNEL                           0x03           //M
#define DISABLE_CHANNEL                          0x04           //M
#define RESET_CHANNEL                            0x05           //M
#define ENABLE_CHANNEL_NETWORK_TX                0x06           //M
#define DISABLE_CHANNEL_NETWORK_TX               0x07           //M
#define AEN_ENABLE                               0x08
#define SET_LINK                                 0x09           //M
#define GET_LINK_STATUS                          0x0A           //M
#define SET_VLAN_FILTER                          0x0B           //M
#define ENABLE_VLAN                              0x0C           //M
#define DISABLE_VLAN                             0x0D           //M
#define SET_MAC_ADDRESS                          0x0E           //M
#define ENABLE_BROADCAST_FILTERING               0x10           //M
#define DISABLE_BROADCAST_FILTERING              0x11           //M
#define ENABLE_GLOBAL_MULTICAST_FILTERING        0x12
#define DISABLE_GLOBAL_MULTICAST_FILTERING       0x13
#define SET_NCSI_FLOW_CONTROL                    0x14
#define GET_VERSION_ID                           0x15           //M
#define GET_CAPABILITIES                         0x16           //M
#define GET_PARAMETERS                           0x17           //M
#define GET_CONTROLLER_PACKET_STATISTICS         0x18
#define GET_NCSI_STATISTICS                      0x19
#define GET_NCSI_PASS_THROUGH_STATISTICS         0x1A

//Standard Response Code
#define COMMAND_COMPLETED                        0x00
#define COMMAND_FAILED                           0x01
#define COMMAND_UNAVAILABLE                      0x02
#define COMMAND_UNSUPPORTED                      0x03

//Standard Reason Code
#define NO_ERROR                                 0x0000
#define INTERFACE_INITIALIZATION_REQUIRED        0x0001
#define PARAMETER_IS_INVALID                     0x0002
#define CHANNEL_NOT_READY                        0x0003
#define PACKAGE_NOT_READY                        0x0004
#define INVALID_PAYLOAD_LENGTH                   0x0005
#define UNKNOWN_COMMAND_TYPE                     0x7FFF

//SET_MAC_ADDRESS
#define UNICAST                                  ( 0x00 << 5 )
#define MULTICAST                                ( 0x01 << 5 )
#define DISABLE_MAC_ADDRESS_FILTER               0x00
#define ENABLE_MAC_ADDRESS_FILTER                0x01

//GET_LINK_STATUS
#define LINK_DOWN                                0
#define LINK_UP                                  1

#endif // NCSI_H
