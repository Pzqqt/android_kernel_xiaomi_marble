/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_THERMAL_TEMP_H
#define __CFG_THERMAL_TEMP_H

#define CFG_THERMAL_TEMP_MIN_LEVEL0 CFG_INI_UINT( \
			"gThermalTempMinLevel0", \
			0, \
			1000, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level0")

#define CFG_THERMAL_TEMP_MAX_LEVEL0 CFG_INI_UINT( \
			"gThermalTempMaxLevel0", \
			0, \
			1000, \
			90, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level0")

#define CFG_THERMAL_TEMP_MIN_LEVEL1 CFG_INI_UINT( \
			"gThermalTempMinLevel1", \
			0, \
			1000, \
			70, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level1")

#define CFG_THERMAL_TEMP_MAX_LEVEL1 CFG_INI_UINT( \
			"gThermalTempMaxLevel1", \
			0, \
			1000, \
			110, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level1")

#define CFG_THERMAL_TEMP_MIN_LEVEL2 CFG_INI_UINT( \
			"gThermalTempMinLevel2", \
			0, \
			1000, \
			90, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level2")

#define CFG_THERMAL_TEMP_MAX_LEVEL2 CFG_INI_UINT( \
			"gThermalTempMaxLevel2", \
			0, \
			1000, \
			125, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level2")

#define CFG_THERMAL_TEMP_MIN_LEVEL3 CFG_INI_UINT( \
			"gThermalTempMinLevel3", \
			0, \
			1000, \
			110, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level3")

#define CFG_THERMAL_TEMP_MAX_LEVEL3 CFG_INI_UINT( \
			"gThermalTempMaxLevel3", \
			0, \
			1000, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level3")

#define CFG_THERMAL_TEMP_ALL \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL0) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL0) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL1) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL1) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL2) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL2) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL3) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL3) \

#endif

