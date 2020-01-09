/*
 * Copyright (c) 2012-2018,2020 The Linux Foundation. All rights reserved.
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

/*
 * <ini>
 * gThermalTempMinLevel0 - Set Thermal Temp Min Level0
 * @Min: 0
 * @Max: 1000
 * @Default: 0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MIN_LEVEL0 CFG_INI_UINT( \
			"gThermalTempMinLevel0", \
			0, \
			1000, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level0")

/*
 * <ini>
 * gThermalTempMaxLevel0 - Set Thermal Temp Max Level0
 * @Min: 0
 * @Max: 1000
 * @Default: 90
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MAX_LEVEL0 CFG_INI_UINT( \
			"gThermalTempMaxLevel0", \
			0, \
			1000, \
			90, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level0")

/*
 * <ini>
 * gThermalTempMinLevel1 - Set Thermal Temp Min Level1
 * @Min: 0
 * @Max: 1000
 * @Default: 70
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MIN_LEVEL1 CFG_INI_UINT( \
			"gThermalTempMinLevel1", \
			0, \
			1000, \
			70, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level1")

/*
 * <ini>
 * gThermalTempMaxLevel1 - Set Thermal Temp Max Level1
 * @Min: 0
 * @Max: 1000
 * @Default: 110
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MAX_LEVEL1 CFG_INI_UINT( \
			"gThermalTempMaxLevel1", \
			0, \
			1000, \
			110, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level1")

/*
 * <ini>
 * gThermalTempMinLevel2 - Set Thermal Temp Min Level2
 * @Min: 0
 * @Max: 1000
 * @Default: 90
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MIN_LEVEL2 CFG_INI_UINT( \
			"gThermalTempMinLevel2", \
			0, \
			1000, \
			90, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level2")

/*
 * <ini>
 * gThermalTempMaxLevel2 - Set Thermal Temp Max Level2
 * @Min: 0
 * @Max: 1000
 * @Default: 125
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MAX_LEVEL2 CFG_INI_UINT( \
			"gThermalTempMaxLevel2", \
			0, \
			1000, \
			125, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level2")

/*
 * <ini>
 * gThermalTempMinLevel3 - Set Thermal Temp Min Level3
 * @Min: 0
 * @Max: 1000
 * @Default: 110
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MIN_LEVEL3 CFG_INI_UINT( \
			"gThermalTempMinLevel3", \
			0, \
			1000, \
			110, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Min Level3")

/*
 * <ini>
 * gThermalTempMaxLevel3 - Set Thermal Temp Max Level3
 * @Min: 0
 * @Max: 1000
 * @Default: 0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_TEMP_MAX_LEVEL3 CFG_INI_UINT( \
			"gThermalTempMaxLevel3", \
			0, \
			1000, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal Temp Max Level3")

/*
 * <ini>
 * gThermalMitigationEnable - Set Thermal mitigation feature control
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THERMAL_MITIGATION_ENABLE CFG_INI_BOOL( \
			"gThermalMitigationEnable", \
			0, \
			"Thermal mitigation feature control")

/*
 * <ini>
 * gThrottlePeriod - Set Thermal mitigation throttle period
 * @Min: 10
 * @Max: 10000
 * @Default: 4000
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_PERIOD CFG_INI_UINT( \
			"gThrottlePeriod", \
			10, \
			10000, \
			4000, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle period")

/*
 * <ini>
 * gThrottleDutyCycleLevel0 - Set Thermal mitigation throttle duty cycle level0
 * @Min: 0
 * @Max: 0
 * @Default: 0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL0 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel0", \
			0, \
			0, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level0")

/*
 * <ini>
 * gThrottleDutyCycleLevel1 - Set Thermal mitigation throttle duty cycle level1
 * @Min: 0
 * @Max: 100
 * @Default: 10
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL1 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel1", \
			0, \
			100, \
			10, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level1")

/*
 * <ini>
 * gThrottleDutyCycleLevel2 - Set Thermal mitigation throttle duty cycle level2
 * @Min: 0
 * @Max: 100
 * @Default: 30
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL2 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel2", \
			0, \
			100, \
			30, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level2")

/*
 * <ini>
 * gThrottleDutyCycleLevel3 - Set Thermal mitigation throttle duty cycle level3
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL3 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel3", \
			0, \
			100, \
			50, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level3")

/*
 * <ini>
 * gThrottleDutyCycleLevel4 - Set Thermal mitigation throttle duty cycle level4
 * @Min: 0
 * @Max: 100
 * @Default: 70
 *
 * This ini will apply the thermal throttle duty cycle value in FW
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL4 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel4", \
			0, \
			100, \
			70, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level4")

/*
 * <ini>
 * gThrottleDutyCycleLevel5 - Set Thermal mitigation throttle duty cycle level5
 * @Min: 0
 * @Max: 100
 * @Default: 90
 *
 * This ini will apply the thermal throttle duty cycle value in FW
 * Usage: External
 *
 * </ini>
 */
#define CFG_THROTTLE_DUTY_CYCLE_LEVEL5 CFG_INI_UINT( \
			"gThrottleDutyCycleLevel5", \
			0, \
			100, \
			90, \
			CFG_VALUE_OR_DEFAULT, \
			"Thermal mitigation throttle duty cycle level5")

/* <ini>
 *gThermalSamplingTime - Configure the thermal mitigation sampling time in ms.
 *
 * @Min: 10
 * @Max: 100
 * @Default: 100
 *
 * This ini will control the sampling time that the thermal mitigation in FW
 * will consider while applying the duty cycle.
 *
 * Usage: External
 *
 * Supported features: Thermal Mitigation
 *
 *</ini>
 */
#define CFG_THERMAL_SAMPLING_TIME CFG_INI_UINT( \
				"gThermalSamplingTime", \
				10, \
				100, \
				100, \
				CFG_VALUE_OR_DEFAULT, \
				"Thermal mitigation sampling time")

#define CFG_THERMAL_TEMP_ALL \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL0) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL0) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL1) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL1) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL2) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL2) \
	CFG(CFG_THERMAL_TEMP_MIN_LEVEL3) \
	CFG(CFG_THERMAL_TEMP_MAX_LEVEL3) \
	CFG(CFG_THERMAL_MITIGATION_ENABLE) \
	CFG(CFG_THROTTLE_PERIOD) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL0) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL1) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL2) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL3) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL4) \
	CFG(CFG_THROTTLE_DUTY_CYCLE_LEVEL5) \
	CFG(CFG_THERMAL_SAMPLING_TIME)

#endif

