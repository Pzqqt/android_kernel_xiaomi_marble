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

#ifndef __CFG_LEGACY_DP
#define __CFG_LEGACY_DP

#include "cfg_define.h"
#include "cfg_converged.h"
#include "qdf_types.h"

/*
 * <ini>
 * gEnableFlowSteering - Enable rx traffic flow steering
 * @Default: false
 *
 * Enable Rx traffic flow steering to enable Rx interrupts on multiple CEs based
 * on the flows. Different CEs<==>different IRQs<==>probably different CPUs.
 * Parallel Rx paths.
 * 1 - enable  0 - disable
 *
 * Usage: Internal
 *
 * </ini>
 */
 #define CFG_DP_FLOW_STEERING_ENABLED \
		CFG_INI_BOOL( \
		"gEnableFlowSteering", \
		false, \
		"")

#define CFG_DP_CE_CLASSIFY_ENABLE \
		CFG_INI_BOOL("gCEClassifyEnable", \
		true, "enable CE classify")

#define CFG_LEGACY_DP_ALL \
	CFG(CFG_DP_FLOW_STEERING_ENABLED) \
	CFG(CFG_DP_CE_CLASSIFY_ENABLE)

#endif
