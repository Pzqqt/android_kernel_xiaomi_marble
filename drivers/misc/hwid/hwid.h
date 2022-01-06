/*
 * Copyright (C) 2021 XiaoMi, Inc.
 *               2022 The LineageOS Project
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __HWID_H__
#define __HWID_H__

#include <linux/types.h>

#define HARDWARE_PROJECT_UNKNOWN    0
#define HARDWARE_PROJECT_L1         HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L1A        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L2         1
#define HARDWARE_PROJECT_L2S        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L3         2
#define HARDWARE_PROJECT_L3S        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L9S        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L10        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L12        HARDWARE_PROJECT_UNKNOWN
#define HARDWARE_PROJECT_L18        HARDWARE_PROJECT_UNKNOWN

typedef enum {
	CountryCN = 0x00,
	CountryGlobal = 0x01,
	CountryIndia = 0x02,
	CountryJapan = 0x03,
	INVALID = 0x04,
	CountryIDMax = 0x7FFFFFFF
} CountryType;

uint32_t get_hw_version_platform(void);
uint32_t get_hw_id_value(void);
uint32_t get_hw_country_version(void);
uint32_t get_hw_version_major(void);
uint32_t get_hw_version_minor(void);
uint32_t get_hw_version_build(void);

#endif /* __HWID_H__ */
