/*
 * (C) Copyright 2004-Present
 * Dan Zhang <zhdaniel@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef __FACEBOOK_CONFIG_H
#define __FACEBOOK_CONFIG_H

#define STR(s)       #s
#define FB_CMN_H_STR(v) STR(facebook_common_v##v.h)
#define FB_CMN_H(v) FB_CMN_H_STR(v)
#define FACEBOOK_COMMON_V_H FB_CMN_H(CONFIG_FBOBMC_IMG_LAYOUT_VER)

#include FACEBOOK_COMMON_V_H

#endif /* __FACEBOOK_CONFIG_H */
