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

#ifndef __CFG_POLICY_MGR
#define __CFG_POLICY_MGR
#include "qdf_types.h"

/*
 * <ini>
 * gWlanMccToSccSwitchMode - Control SAP channel.
 * @Min: 0
 * @Max: 5
 * @Default: 0
 *
 * This ini is used to override SAP channel.
 * If gWlanMccToSccSwitchMode = 0: disabled.
 * If gWlanMccToSccSwitchMode = 1: Enable switch.
 * If gWlainMccToSccSwitchMode = 2: Force switch with SAP restart.
 * If gWlainMccToSccSwitchMode = 3: Force switch without SAP restart.
 * If gWlainMccToSccSwitchMode = 4: Switch using
 * 					fav channel(s)without SAP restart.
 * If gWlainMccToSccSwitchMode = 5: Force switch without SAP restart.MCC allowed
 *					in exceptional cases.
 * If gWlainMccToSccSwitchMode = 6: Force Switch without SAP restart only in
					user preffered band.
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MCC_TO_SCC_SWITCH CFG_INI_UINT(\
					"gWlanMccToSccSwitchMode", \
					QDF_MCC_TO_SCC_SWITCH_DISABLE, \
					QDF_MCC_TO_SCC_SWITCH_MAX - 1, \
					QDF_MCC_TO_SCC_SWITCH_DISABLE, \
					CFG_VALUE_OR_DEFAULT, \
					"Provides MCC to SCC switch mode")
/*
 * <ini>
 * gSystemPref - Configure wlan system preference for PCL.
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is used to configure wlan system preference option to help
 * policy manager decide on Preferred Channel List for a new connection.
 * For possible values refer to enum hdd_conc_priority_mode
 *
 * Related: None.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_CONC_SYS_PREF CFG_INI_UINT(\
					"gSystemPref", 0, 2, 0, \
					CFG_VALUE_OR_DEFAULT, \
					"System preference to predict PCL")
/*
 * <ini>
 * gMaxConcurrentActiveSessions - Maximum number of concurrent connections.
 * @Min: 1
 * @Max: 4
 * @Default: 3
 *
 * This ini is used to configure the maximum number of concurrent connections.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_MAX_CONC_CXNS CFG_INI_UINT(\
					"gMaxConcurrentActiveSessions", \
					1, 4, 3, \
					CFG_VALUE_OR_DEFAULT, \
					"Config max num allowed connections")
/*
 * <ini>
 * channel_select_logic_conc - Set channel selection logic
 * for different concurrency combinations to DBS or inter band
 * MCC. Default is DBS for STA+STA and STA+P2P.
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000000
 *
 * 0 - inter-band MCC
 * 1 - DBS
 *
 * BIT 0: STA+STA
 * BIT 1: STA+P2P
 * BIT 2-31: Reserved
 *
 * Supported Feature: STA+STA, STA+P2P
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHNL_SELECT_LOGIC_CONC CFG_INI_UINT(\
						"channel_select_logic_conc",\
						0x00000000, \
						0xFFFFFFFF, \
						0x00000003, \
						CFG_VALUE_OR_DEFAULT, \
						"Set channel selection policy for various concurrency")
/*
 * <ini>
 * dbs_selection_policy - Configure dbs selection policy.
 * @Min: 0
 * @Max: 3
 * @Default: 0
 *
 *  set band preference or Vdev preference.
 *      bit[0] = 0: 5G 2x2 preferred to select 2x2 5G + 1x1 2G DBS mode.
 *      bit[0] = 1: 2G 2x2 preferred to select 2x2 2G + 1x1 5G DBS mode.
 *      bit[1] = 1: vdev priority enabled. The INI "vdev_priority_list" will
 * specify the vdev priority.
 *      bit[1] = 0: vdev priority disabled.
 * This INI only take effect for Genoa dual DBS hw.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DBS_SELECTION_PLCY CFG_INI_UINT(\
					    "dbs_selection_policy", \
					    0, 3, 0, \
					    CFG_VALUE_OR_DEFAULT, \
					    "Configure dbs selection policy")
/*
 * <ini>
 * vdev_priority_list - Configure vdev priority list.
 * @Min: 0
 * @Max: 0x4444
 * @Default: 0x4321
 *
 * @vdev_priority_list: vdev priority list
 *      bit[0-3]: pri_id (policy_mgr_pri_id) of highest priority
 *      bit[4-7]: pri_id (policy_mgr_pri_id) of second priority
 *      bit[8-11]: pri_id (policy_mgr_pri_id) of third priority
 *      bit[12-15]: pri_id (policy_mgr_pri_id) of fourth priority
 *      example: 0x4321 - CLI < GO < SAP < STA
 *      vdev priority id mapping:
 *        PM_STA_PRI_ID = 1,
 *        PM_SAP_PRI_ID = 2,
 *        PM_P2P_GO_PRI_ID = 3,
 *        PM_P2P_CLI_PRI_ID = 4,
 * When the previous INI "dbs_selection_policy" bit[1]=1, which means
 * the vdev 2x2 prioritization enabled. Then this INI will be used to
 * specify the vdev type priority list. For example :
 * dbs_selection_policy=0x2
 * vdev_priority_list=0x4312
 * means: default preference 2x2 band is 5G, vdev 2x2 prioritization enabled.
 * And the priority list is CLI < GO < STA < SAP
 *
 * This INI only take effect for Genoa dual DBS hw.
 *
 * Supported Feature: DBS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_VDEV_CUSTOM_PRIORITY_LIST CFG_INI_UINT(\
					"vdev_priority_list", \
					0, 0x4444, 0x4321, \
					CFG_VALUE_OR_DEFAULT, \
					"Configure vdev priority list")
/*
 * <ini>
 * gEnableCustomConcRule1 - Enable custom concurrency rule1.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable custom concurrency rule1.
 * If SAP comes up first and STA comes up later then SAP needs to follow STA's
 * channel.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_CONC_RULE1 CFG_INI_UINT(\
					"gEnableCustomConcRule1", \
					0, 1, 0, \
					CFG_VALUE_OR_DEFAULT, \
					"Enable custom concurrency rule 1")
/*
 * <ini>
 * gEnableCustomConcRule2 - Enable custom concurrency rule2.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable custom concurrency rule2.
 * If P2PGO comes up first and STA comes up later then P2PGO need to follow
 * STA's channel in 5Ghz. In following if condition we are just adding sanity
 * check to make sure that by this time P2PGO's channel is same as STA's
 * channel.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_CONC_RULE2 CFG_INI_UINT(\
					"gEnableCustomConcRule2", \
					0, 1, 0, \
					CFG_VALUE_OR_DEFAULT, \
					"Enable custom concurrency rule 2")
/*
 * <ini>
 * gEnableMCCAdaptiveScheduler - MCC Adaptive Scheduler feature.
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable MCC Adaptive Scheduler feature.
 *
 * Related: None.
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_MCC_ADATIVE_SCH_ENABLED_NAME CFG_INI_UINT(\
					"gEnableMCCAdaptiveScheduler", \
					0, 1, 1, \
					CFG_VALUE_OR_DEFAULT, \
					"Enable/Disable MCC Adaptive Scheduler")

/*
 * <ini>
 * gEnableStaConnectionIn5Ghz - To enable/disable STA connection in 5G
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable STA connection in 5G band
 *
 * Related: STA
 *
 * Supported Feature: Concurrency
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_STA_CONNECTION_IN_5GHZ CFG_INI_UINT(\
					"gEnableStaConnectionIn5Ghz", \
					0, 1, 1, \
					CFG_VALUE_OR_DEFAULT, \
					"Enable/Disable STA connection in 5G")

#define CFG_POLICY_MGR_ALL \
		CFG(CFG_MCC_TO_SCC_SWITCH) \
		CFG(CFG_CONC_SYS_PREF) \
		CFG(CFG_MAX_CONC_CXNS) \
		CFG(CFG_DBS_SELECTION_PLCY) \
		CFG(CFG_VDEV_CUSTOM_PRIORITY_LIST) \
		CFG(CFG_CHNL_SELECT_LOGIC_CONC) \
		CFG(CFG_ENABLE_CONC_RULE1) \
		CFG(CFG_ENABLE_CONC_RULE2) \
		CFG(CFG_ENABLE_MCC_ADATIVE_SCH_ENABLED_NAME)\
		CFG(CFG_ENABLE_STA_CONNECTION_IN_5GHZ)
#endif
