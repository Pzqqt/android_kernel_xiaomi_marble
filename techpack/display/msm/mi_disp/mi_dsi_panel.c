/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2020 XiaoMi, Inc. All rights reserved.
 */

#define pr_fmt(fmt)	"mi-dsi-panel:[%s:%d] " fmt, __func__, __LINE__
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/pm_wakeup.h>
#include <video/mipi_display.h>

#include "dsi_panel.h"
#include "dsi_display.h"
#include "dsi_ctrl_hw.h"
#include "dsi_parser.h"
#include "../../../../kernel/irq/internals.h"
#include "sde_connector.h"
#include "sde_trace.h"
#include "mi_dsi_panel.h"
#include "mi_disp_feature.h"
#include "mi_disp_print.h"
#include "mi_disp_parser.h"
#include "mi_dsi_display.h"
#include "mi_disp_lhbm.h"
#include "mi_panel_id.h"
#include "mi_disp_nvt_alpha_data.h"

#define to_dsi_display(x) container_of(x, struct dsi_display, host)

static int mi_dsi_update_lhbm_cmd_87reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl);
void mi_dsi_update_backlight_in_aod(struct dsi_panel *panel,
			bool restore_backlight);

static u64 g_panel_id[MI_DISP_MAX];

typedef int (*mi_display_pwrkey_callback)(int);
extern void mi_display_pwrkey_callback_set(mi_display_pwrkey_callback);


struct dsi_panel *g_panel;

static int mi_panel_id_init(struct dsi_panel *panel)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	g_panel = panel;
	g_panel_id[mi_get_disp_id(panel->type)] = panel->mi_cfg.mi_panel_id;

	return 0;
}

enum mi_project_panel_id mi_get_panel_id_by_dsi_panel(struct dsi_panel *panel)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return PANEL_ID_INVALID;
	}

	return mi_get_panel_id(panel->mi_cfg.mi_panel_id);
}

enum mi_project_panel_id mi_get_panel_id_by_disp_id(int disp_id)
{
	if (!is_support_disp_id(disp_id)) {
		DISP_ERROR("Unsupported display id\n");
		return PANEL_ID_INVALID;
	}

	return mi_get_panel_id(g_panel_id[disp_id]);
}

int mi_dsi_panel_init(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	mi_cfg->dsi_panel = panel;
	mutex_init(&mi_cfg->doze_lock);

	mi_cfg->disp_wakelock = wakeup_source_register(NULL, "disp_wakelock");
	if (!mi_cfg->disp_wakelock) {
		DISP_ERROR("doze_wakelock wake_source register failed");
		return -ENOMEM;
	}

	mi_dsi_panel_parse_config(panel);
	mi_panel_id_init(panel);
	atomic_set(&mi_cfg->real_brightness_clone, 0);
	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M16T_PANEL_PA ||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M16T_PANEL_PB||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M80_PANEL_PA ||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PA ||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB){
		mi_display_pwrkey_callback_set(mi_display_powerkey_callback);
	}

	return 0;
}

int mi_dsi_panel_deinit(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	if (mi_cfg->disp_wakelock) {
		wakeup_source_unregister(mi_cfg->disp_wakelock);
	}
	return 0;
}

int mi_dsi_acquire_wakelock(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	if (mi_cfg->disp_wakelock) {
		__pm_stay_awake(mi_cfg->disp_wakelock);
	}

	return 0;
}

int mi_dsi_release_wakelock(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	if (mi_cfg->disp_wakelock) {
		__pm_relax(mi_cfg->disp_wakelock);
	}

	return 0;

}

bool is_aod_and_panel_initialized(struct dsi_panel *panel)
{
	if ((panel->power_mode == SDE_MODE_DPMS_LP1 ||
		panel->power_mode == SDE_MODE_DPMS_LP2) &&
		dsi_panel_initialized(panel)){
		return true;
	} else {
		return false;
	}
}

int mi_dsi_panel_esd_irq_ctrl(struct dsi_panel *panel,
				bool enable)
{
	int ret  = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	ret = mi_dsi_panel_esd_irq_ctrl_locked(panel, enable);
	mutex_unlock(&panel->panel_lock);

	return ret;
}

int mi_dsi_panel_esd_irq_ctrl_locked(struct dsi_panel *panel,
				bool enable)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	struct irq_desc *desc;

	if (!panel || !panel->panel_initialized) {
		DISP_ERROR("Panel not ready!\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;
	if (gpio_is_valid(mi_cfg->esd_err_irq_gpio)) {
		if (mi_cfg->esd_err_irq) {
			if (enable) {
				if (!mi_cfg->esd_err_enabled) {
					desc = irq_to_desc(mi_cfg->esd_err_irq);
					if (!irq_settings_is_level(desc))
						desc->istate &= ~IRQS_PENDING;
					enable_irq_wake(mi_cfg->esd_err_irq);
					enable_irq(mi_cfg->esd_err_irq);
					mi_cfg->esd_err_enabled = true;
					DISP_INFO("[%s] esd irq is enable\n", panel->type);
				}
			} else {
				if (mi_cfg->esd_err_enabled) {
					disable_irq_wake(mi_cfg->esd_err_irq);
					disable_irq_nosync(mi_cfg->esd_err_irq);
					mi_cfg->esd_err_enabled = false;
					DISP_INFO("[%s] esd irq is disable\n", panel->type);
				}
			}
		}
	} else {
		DISP_INFO("[%s] esd irq gpio invalid\n", panel->type);
	}
	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M80_PANEL_PA){
		if (gpio_is_valid(mi_cfg->esd_err_irq_gpio_second)) {
			if (mi_cfg->esd_err_irq_second) {
				if (enable) {
					if (!mi_cfg->esd_err_enabled_second) {
						desc = irq_to_desc(mi_cfg->esd_err_irq_second);
						if (!irq_settings_is_level(desc))
							desc->istate &= ~IRQS_PENDING;
						enable_irq_wake(mi_cfg->esd_err_irq_second);
						enable_irq(mi_cfg->esd_err_irq_second);
						mi_cfg->esd_err_enabled_second = true;
						DISP_INFO("[%s] esd irq second is enable\n", panel->type);
					}
				} else {
					if (mi_cfg->esd_err_enabled_second) {
						disable_irq_wake(mi_cfg->esd_err_irq_second);
						disable_irq_nosync(mi_cfg->esd_err_irq_second);
						mi_cfg->esd_err_enabled_second = false;
						DISP_INFO("[%s] esd irq second is disable\n", panel->type);
					}
				}
			}
		} else {
			DISP_INFO("[%s] esd irq second gpio invalid\n", panel->type);
		}

	}
	
	return 0;
}

static void mi_disp_set_dimming_delayed_work_handler(struct kthread_work *work)
{
	struct disp_delayed_work *delayed_work = container_of(work,
					struct disp_delayed_work, delayed_work.work);
	struct dsi_panel *panel = (struct dsi_panel *)(delayed_work->data);
	struct disp_feature_ctl ctl;

	memset(&ctl, 0, sizeof(struct disp_feature_ctl));
	ctl.feature_id = DISP_FEATURE_DIMMING;
	ctl.feature_val = FEATURE_ON;

	DISP_INFO("[%s] panel set backlight dimming on\n", panel->type);
	mi_dsi_acquire_wakelock(panel);
	mi_dsi_panel_set_disp_param(panel, &ctl);
	mi_dsi_release_wakelock(panel);

	kfree(delayed_work);
}

int mi_dsi_panel_tigger_dimming_delayed_work(struct dsi_panel *panel)
{
	int disp_id = 0;
	struct disp_feature *df = mi_get_disp_feature();
	struct disp_display *dd_ptr;
	struct disp_delayed_work *delayed_work;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	delayed_work = kzalloc(sizeof(*delayed_work), GFP_KERNEL);
	if (!delayed_work) {
		DISP_ERROR("failed to allocate delayed_work buffer\n");
		return -ENOMEM;
	}

	disp_id = mi_get_disp_id(panel->type);
	dd_ptr = &df->d_display[disp_id];

	kthread_init_delayed_work(&delayed_work->delayed_work,
			mi_disp_set_dimming_delayed_work_handler);
	delayed_work->dd_ptr = dd_ptr;
	delayed_work->wq = &dd_ptr->pending_wq;
	delayed_work->data = panel;

	return kthread_queue_delayed_work(dd_ptr->worker, &delayed_work->delayed_work,
				msecs_to_jiffies(panel->mi_cfg.panel_on_dimming_delay));
}

int mi_dsi_update_switch_cmd(struct dsi_panel *panel)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u8 gamma_cfg = 0;
	u32 cmd_update_index = 0;
	bool need_update_gamma_zero = false;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	cmd_update_index = DSI_CMD_SET_TIMING_SWITCH_UPDATA;
	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	need_update_gamma_zero = panel->mi_cfg.flat_update_gamma_zero;

	if (panel->mi_cfg.feature_val[DISP_FEATURE_FLAT_MODE] == FEATURE_ON)
		gamma_cfg = cur_mode->priv_info->flat_on_gamma;
	else
		gamma_cfg = cur_mode->priv_info->flat_off_gamma;

	if (gamma_cfg || need_update_gamma_zero) {
		mi_dsi_panel_update_cmd_set(panel, cur_mode,
			DSI_CMD_SET_TIMING_SWITCH, info,
			&gamma_cfg, sizeof(gamma_cfg));
	} else {
		DISP_INFO("[%s] gamma cfg is 0, can not update switch cmd", panel->type);
	}
	return 0;
}

int mi_dsi_update_switch_cmd_several_gamma(struct dsi_panel *panel)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 gamma_cfg[2] = {0, 0};
	int j = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (panel->mi_cfg.feature_val[DISP_FEATURE_FLAT_MODE] == FEATURE_ON) {
		gamma_cfg[0] = cur_mode->priv_info->flat_on_gamma;
		gamma_cfg[1] = cur_mode->priv_info->flat_on_lhbm_gamma;
	} else {
		gamma_cfg[0] = cur_mode->priv_info->flat_off_gamma;
		gamma_cfg[1] = cur_mode->priv_info->flat_off_lhbm_gamma;
	}

	if (!gamma_cfg[0] || !gamma_cfg[1]) {
		DISP_INFO("[%s] gamma cfg is 0, can not update switch cmd", panel->type);
		return 0;
	}

	cmd_update_index = DSI_CMD_SET_TIMING_SWITCH_UPDATA;
	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];

	for (j = 0; j < cmd_update_count; j++) {
		mi_dsi_panel_update_cmd_set(panel, cur_mode,
			DSI_CMD_SET_TIMING_SWITCH, info,
			&gamma_cfg[j], sizeof(gamma_cfg[j]));
		info++;
	}

	return 0;
}

bool is_hbm_fod_on(struct dsi_panel *panel)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;
	int feature_val;
	bool is_fod_on = false;

	feature_val = mi_cfg->feature_val[DISP_FEATURE_LOCAL_HBM];
	switch (feature_val) {
	case LOCAL_HBM_NORMAL_WHITE_1000NIT:
	case LOCAL_HBM_NORMAL_WHITE_750NIT:
	case LOCAL_HBM_NORMAL_WHITE_500NIT:
	case LOCAL_HBM_NORMAL_WHITE_110NIT:
	case LOCAL_HBM_NORMAL_GREEN_500NIT:
	case LOCAL_HBM_HLPM_WHITE_1000NIT:
	case LOCAL_HBM_HLPM_WHITE_110NIT:
		is_fod_on = true;
		break;
	default:
		break;
	}

	if (mi_cfg->feature_val[DISP_FEATURE_HBM_FOD] == FEATURE_ON) {
		is_fod_on = true;
	}

	return is_fod_on;
}

bool is_dc_on_skip_backlight(struct dsi_panel *panel, u32 bl_lvl)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;

	if (!mi_cfg->dc_feature_enable)
		return false;
	if (mi_cfg->feature_val[DISP_FEATURE_DC] == FEATURE_OFF)
		return false;
	if (mi_cfg->dc_type == TYPE_CRC_SKIP_BL &&
		bl_lvl < mi_cfg->dc_threshold &&
		mi_cfg->feature_val[DISP_FEATURE_HBM] == FEATURE_OFF)
		return true;
	else
		return false;
}

bool is_backlight_set_skip(struct dsi_panel *panel, u32 bl_lvl)
{
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;
	int feature_val = -1;

	if (mi_cfg->in_fod_calibration || is_hbm_fod_on(panel)) {
		if (bl_lvl != 0) {
			if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L2_PANEL_PA ||
				mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L1_PANEL_PA ||
				mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L2S_PANEL_PA ||
				mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PA ||
				mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB) {
				DISP_INFO("[%s] update 51 reg to %d even LHBM is on,",
					panel->type, bl_lvl);
				return false;
			}
			DISP_INFO("[%s] skip set backlight %d due to LHBM is on,"
				"but update alpha\n", panel->type, bl_lvl);
			feature_val = mi_cfg->feature_val[DISP_FEATURE_LOCAL_HBM];
			if (feature_val== LOCAL_HBM_NORMAL_WHITE_1000NIT ||
				feature_val == LOCAL_HBM_HLPM_WHITE_1000NIT) {
				mi_dsi_update_lhbm_cmd_87reg(panel,
					DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, bl_lvl);
				dsi_panel_tx_cmd_set(panel,
					DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT);
			} else if (feature_val == LOCAL_HBM_NORMAL_WHITE_110NIT ||
				feature_val == LOCAL_HBM_HLPM_WHITE_110NIT) {
				mi_dsi_update_lhbm_cmd_87reg(panel,
					DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT, bl_lvl);
				dsi_panel_tx_cmd_set(panel,
					DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT);
			} else {
				DISP_INFO("[%s] skip set backlight %d due to fod hbm "
						"or fod calibration\n", panel->type, bl_lvl);
			 }
		} else {
			DISP_INFO("[%s] skip set backlight %d due to fod hbm "
					"or fod calibration\n", panel->type, bl_lvl);
		}
		return true;
	} else if (bl_lvl != 0 && is_dc_on_skip_backlight(panel, bl_lvl)) {
		DISP_INFO("[%s] skip set backlight %d due to DC on\n", panel->type, bl_lvl);
		return true;
	} else if (panel->power_mode == SDE_MODE_DPMS_LP1 && bl_lvl == 0) {
		DISP_INFO("[%s] skip set backlight 0 due to LP1 on\n", panel->type);
		return true;
	} else if (!panel->mi_cfg.bl_enable) {
		DISP_INFO("[%s] skip set backlight %d due to aod on\n", panel->type, bl_lvl);
		return true;
	} else {
		return false;
	}
}

bool is_target_fps_support_dc(struct dsi_panel *panel)
{
	bool rst = true;
	struct dsi_display_mode *cur_mode = panel->cur_mode;

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3_PANEL_PA ||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3S_PANEL_PA) {
		if (cur_mode->timing.refresh_rate != 60)
			rst = false;
	}

	return rst;
}

void mi_dsi_panel_update_last_bl_level(struct dsi_panel *panel, int brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return;
	}

	mi_cfg = &panel->mi_cfg;

	if ((mi_cfg->last_bl_level == 0 || mi_cfg->dimming_state == STATE_DIM_RESTORE) &&
		brightness > 0 && !is_hbm_fod_on(panel) && !mi_cfg->in_fod_calibration &&
		panel->mi_cfg.bl_enable) {
		if (!panel->mi_cfg.dimming_need_update_speed)
			mi_dsi_panel_tigger_dimming_delayed_work(panel);
		else {
			mi_cfg->feature_val[DISP_FEATURE_DIMMING] = FEATURE_ON;
			DISP_INFO("[%s] enable dimming setting.\n", panel->type);
		}

		if (mi_cfg->dimming_state == STATE_DIM_RESTORE)
			mi_cfg->dimming_state = STATE_NONE;
	}

	if (mi_cfg->last_bl_level == 0 && brightness && ( mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M80_PANEL_PA)){
		DSI_INFO("disable insert black sreen\n");
		dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISABLE_INSERT_BLACK);
	}

	mi_cfg->last_bl_level = brightness;
	if (brightness != 0)
		mi_cfg->last_no_zero_bl_level = brightness;

	return;
}

void mi_dsi_panel_update_dimming_frame(struct dsi_panel *panel, u8 dimming_switch, u8 frame)
{
	static u8 frame_state = DIMMING_SPEED_0FRAME;
	static u8 dimming_switch_state = DIMMING_SPEED_SWITCH_DEFAULT;

	if (frame_state == frame && dimming_switch_state == dimming_switch) {
		DISP_ERROR("don't need to update dimming state\n");
		return;
	}

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return;
	}

	if (dimming_switch_state != dimming_switch) {
		if (dimming_switch == DIMMING_SPEED_SWITCH_ON) {
			dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMINGON);
			DISP_DEBUG("[%s] panel set dimming on.\n", panel->type);
		} else if (dimming_switch == DIMMING_SPEED_SWITCH_OFF) {
			dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMINGOFF);
			DISP_DEBUG("[%s] panel set dimming off.\n", panel->type);
		}
		dimming_switch_state = dimming_switch;
	}

	if (dimming_switch_state == DIMMING_SPEED_SWITCH_ON) {
		if (frame == DIMMING_SPEED_8FRAME && frame_state != frame) {
			dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMING_8FRAME);
			DISP_DEBUG("[%s] panel set dimming frame %d.\n", panel->type, frame);
		} else if (frame == DIMMING_SPEED_4FRAME && frame_state != frame) {
			dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMING_4FRAME);
			DISP_DEBUG("[%s] panel set dimming frame %d.\n", panel->type, frame);
		}
	}
	frame_state = frame;

	return;
}

int mi_dsi_print_51_backlight_log(struct dsi_panel *panel,
		struct dsi_cmd_desc *cmd)
{
	u8 *buf = NULL;
	u32 bl_lvl = 0;
	int i = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	static int use_count = 20;

	if (!panel || !cmd) {
		DISP_ERROR("Invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;
	buf = (u8 *)cmd->msg.tx_buf;
	if (buf && buf[0] == MIPI_DCS_SET_DISPLAY_BRIGHTNESS) {
		if (cmd->msg.tx_len >= 3) {
			if (panel->bl_config.bl_inverted_dbv)
				bl_lvl = (buf[1] << 8) | buf[2];
			else
				bl_lvl = buf[1] | (buf[2] << 8);

			if (use_count-- > 0 && mi_cfg->last_bl_level != bl_lvl)
				DISP_TIME_INFO("[%s] set 51 backlight %d\n", panel->type, bl_lvl);

			if ((!mi_cfg->last_bl_level && bl_lvl) ||
				(mi_cfg->last_bl_level && !bl_lvl)) {
					use_count = 20;
			}
		}

		if (mi_get_backlight_log_mask() & BACKLIGHT_LOG_ENABLE) {
			DISP_INFO("[%s] [0x51 backlight debug] tx_len = %d\n",
					panel->type, cmd->msg.tx_len);
			for (i = 0; i < cmd->msg.tx_len; i++) {
				DISP_INFO("[%s] [0x51 backlight debug] tx_buf[%d] = 0x%02X\n",
					panel->type, i, buf[i]);
			}

			if (mi_get_backlight_log_mask() & BACKLIGHT_LOG_DUMP_STACK)
				dump_stack();
		}
	}

	return 0;
}

int mi_dsi_panel_parse_cmd_sets_update(struct dsi_panel *panel,
		struct dsi_display_mode *mode)
{
	int rc = 0;
	int i = 0, j = 0, k = 0;
	u32 length = 0;
	u32 count = 0;
	u32 size = 0;
	u32 *arr_32 = NULL;
	const u32 *arr;
	struct dsi_parser_utils *utils = &panel->utils;
	struct dsi_cmd_update_info *info;
	int type = -1;

	if (!mode || !mode->priv_info) {
		DISP_ERROR("invalid arguments\n");
		return -EINVAL;
	}

	DISP_INFO("WxH: %dx%d, FPS: %d\n", mode->timing.h_active,
			mode->timing.v_active, mode->timing.refresh_rate);

	for (i = 0; i < DSI_CMD_UPDATE_MAX; i++) {
		arr = utils->get_property(utils->data, cmd_set_update_map[i],
			&length);

		if (!arr) {
			DISP_DEBUG("[%s] commands not defined\n", cmd_set_update_map[i]);
			continue;
		}

		if (length & 0x1) {
			DISP_ERROR("[%s] syntax error\n", cmd_set_update_map[i]);
			rc = -EINVAL;
			goto error;
		}

		DISP_INFO("%s length = %d\n", cmd_set_update_map[i], length);

		for (j = DSI_CMD_SET_PRE_ON; j < DSI_CMD_SET_MAX; j++) {
			if (strstr(cmd_set_update_map[i], cmd_set_prop_map[j])) {
				type = j;
				DISP_INFO("find type(%d) is [%s] commands\n", type,
					cmd_set_prop_map[type]);
				break;
			}
		}
		if (type < 0 || j == DSI_CMD_SET_MAX) {
			rc = -EINVAL;
			goto error;
		}

		length = length / sizeof(u32);
		size = length * sizeof(u32);

		arr_32 = kzalloc(size, GFP_KERNEL);
		if (!arr_32) {
			rc = -ENOMEM;
			goto error;
		}

		rc = utils->read_u32_array(utils->data, cmd_set_update_map[i],
						arr_32, length);
		if (rc) {
			DISP_ERROR("[%s] read failed\n", cmd_set_update_map[i]);
			goto error_free_arr_32;
		}

		count = length / 3;
		size = count * sizeof(*info);
		info = kzalloc(size, GFP_KERNEL);
		if (!info) {
			rc = -ENOMEM;
			goto error_free_arr_32;
		}

		mode->priv_info->cmd_update[i] = info;
		mode->priv_info->cmd_update_count[i] = count;

		for (k = 0; k < length; k += 3) {
			info->type = type;
			info->mipi_address= arr_32[k];
			info->index= arr_32[k + 1];
			info->length= arr_32[k + 2];
			DISP_INFO("update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				cmd_set_update_map[i], info->mipi_address,
				info->index, info->length);
			info++;
		}
	}

error_free_arr_32:
	kfree(arr_32);
error:
	return rc;
}

int mi_dsi_panel_update_cmd_set(struct dsi_panel *panel,
			struct dsi_display_mode *mode, enum dsi_cmd_set_type type,
			struct dsi_cmd_update_info *info, u8 *payload, u32 size)
{
	int rc = 0;
	int i = 0;
	struct dsi_cmd_desc *cmds = NULL;
	u32 count;
	u8 *tx_buf = NULL;
	size_t tx_len;

	if (!panel || !mode || !mode->priv_info)
	{
		DISP_ERROR("invalid panel or cur_mode params\n");
		return -EINVAL;
	}

	if (!info || !payload) {
		DISP_ERROR("invalid info or payload params\n");
		return -EINVAL;
	}

	if (type != info->type || size != info->length) {
		DISP_ERROR("please check type(%d, %d) or update size(%d, %d)\n",
			type, info->type, info->length, size);
		return -EINVAL;
	}

	cmds = mode->priv_info->cmd_sets[type].cmds;
	count = mode->priv_info->cmd_sets[type].count;
	if (count == 0) {
		DISP_ERROR("[%s] No commands to be sent\n", cmd_set_prop_map[type]);
		return -EINVAL;
	}

	DISP_DEBUG("WxH: %dx%d, FPS: %d\n", mode->timing.h_active,
			mode->timing.v_active, mode->timing.refresh_rate);

	DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
	for (i = 0; i < size; i++) {
		DISP_DEBUG("[%s] payload[%d] = 0x%02X\n", panel->type, i, payload[i]);
	}

	if (cmds && count > info->index) {
		tx_buf = (u8 *)cmds[info->index].msg.tx_buf;
		tx_len = cmds[info->index].msg.tx_len;
		if (tx_buf && tx_buf[0] == info->mipi_address && tx_len >= info->length) {
			memcpy(&tx_buf[1], payload, info->length);
			for (i = 0; i < tx_len; i++) {
				DISP_DEBUG("[%s] tx_buf[%d] = 0x%02X\n",
					panel->type, i, tx_buf[i]);
			}
		} else {
			if (tx_buf) {
				DISP_ERROR("[%s] %s mipi address(0x%02X != 0x%02X)\n",
					panel->type, cmd_set_prop_map[type],
					tx_buf[0], info->mipi_address);
			} else {
				DISP_ERROR("[%s] panel tx_buf is NULL pointer\n",
					panel->type);
			}
			rc = -EINVAL;
		}
	} else {
		DISP_ERROR("[%s] panel cmd[%s] index error\n",
			panel->type, cmd_set_prop_map[type]);
		rc = -EINVAL;
	}

	return rc;
}

int mi_dsi_panel_parse_gamma_config(struct dsi_panel *panel,
		struct dsi_display_mode *mode)
{
	int rc;
	struct dsi_display_mode_priv_info *priv_info;
	struct dsi_parser_utils *utils = &panel->utils;
	u32 gamma_cfg[2] = {0, 0};
	u32 lhbm_gamma_cfg[2] = {0, 0};

	priv_info = mode->priv_info;

	rc = utils->read_u32_array(utils->data, "mi,mdss-flat-status-control-gamma-cfg",
			gamma_cfg, 2);
	if (rc) {
		DISP_DEBUG("mi,mdss-flat-status-control-gamma-cfg not defined rc=%d\n", rc);
	} else {
		DISP_INFO("FPS: %d, gamma cfg: 0x%02X, 0x%02X\n",
			mode->timing.refresh_rate, gamma_cfg[0], gamma_cfg[1]);
		priv_info->flat_on_gamma = gamma_cfg[0];
		priv_info->flat_off_gamma = gamma_cfg[1];
	}

	rc = utils->read_u32_array(utils->data, "mi,mdss-flat-status-control-lhbm-gamma-cfg",
			lhbm_gamma_cfg, 2);
	if (rc) {
		DISP_DEBUG("mi,mdss-flat-status-control-lhbm-gamma-cfg not defined rc=%d\n", rc);
	} else {
		DISP_INFO("FPS: %d, lhbm gamma cfg: 0x%02X, 0x%02X\n",
			mode->timing.refresh_rate,lhbm_gamma_cfg[0], lhbm_gamma_cfg[1]);
		priv_info->flat_on_lhbm_gamma = lhbm_gamma_cfg[0];
		priv_info->flat_off_lhbm_gamma = lhbm_gamma_cfg[1];
	}

	return 0;
}

int mi_dsi_panel_write_cmd_set(struct dsi_panel *panel,
				struct dsi_panel_cmd_set *cmd_sets)
{
	int rc = 0, i = 0;
	ssize_t len;
	struct dsi_cmd_desc *cmds;
	u32 count;
	enum dsi_cmd_set_state state;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cmds = cmd_sets->cmds;
	count = cmd_sets->count;
	state = cmd_sets->state;

	if (count == 0) {
		DISP_DEBUG("[%s] No commands to be sent for state\n", panel->type);
		goto error;
	}

	for (i = 0; i < count; i++) {
		cmds->ctrl_flags = 0;

		if (state == DSI_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		len = dsi_host_transfer_sub(panel->host, cmds);
		if (len < 0) {
			rc = len;
			DISP_ERROR("failed to set cmds, rc=%d\n", rc);
			goto error;
		}
		if (cmds->post_wait_ms)
			usleep_range(cmds->post_wait_ms * 1000,
					((cmds->post_wait_ms * 1000) + 10));
		cmds++;
	}
error:
	return rc;
}

int mi_dsi_panel_read_batch_number(struct dsi_panel *panel)
{
	int rc = 0;
	unsigned long mode_flags_backup = 0;
	u8 rdbuf[8];
	ssize_t read_len = 0;
	u8 read_batch_number = 0;

	int i = 0;
	struct panel_batch_info info[] = {
		{0x00, "P0.0"},
		{0x01, "P0.1"},
		{0x10, "P1.0"},
		{0x11, "P1.1"},
		{0x12, "P1.2"},
		{0x13, "P1.2"},
		{0x20, "P2.0"},
		{0x21, "P2.1"},
		{0x30, "MP"},
	};

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	mutex_lock(&panel->panel_lock);

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L2_PANEL_PA ||
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L2S_PANEL_PA) {
		mode_flags_backup = panel->mipi_device.mode_flags;
		panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
		read_len = mipi_dsi_dcs_read(&panel->mipi_device, 0xDA, rdbuf, 1);
		panel->mipi_device.mode_flags = mode_flags_backup;
		if (read_len > 0) {
			read_batch_number = rdbuf[0];
			panel->mi_cfg.panel_batch_number = read_batch_number;
			for (i = 0; i < ARRAY_SIZE(info); i++) {
				if (read_batch_number == info[i].batch_number) {
					DISP_INFO("panel batch is %s\n", info[i].batch_name);
					break;
				}
			}
			rc = 0;
			panel->mi_cfg.panel_batch_number_read_done = true;
		} else {
			DISP_ERROR("failed to read panel batch number\n");
			panel->mi_cfg.panel_batch_number = 0;
			rc = -EAGAIN;
			panel->mi_cfg.panel_batch_number_read_done = false;
		}
	}

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int mi_dsi_panel_write_dsi_cmd_set(struct dsi_panel *panel,
			int type)
{
	int rc = 0;
	int i = 0, j = 0;
	u8 *tx_buf = NULL;
	u8 *buffer = NULL;
	int buf_size = 1024;
	u32 cmd_count = 0;
	int buf_count = 1024;
	struct dsi_cmd_desc *cmds;
	enum dsi_cmd_set_state state;
	struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode || type < 0 || type >= DSI_CMD_SET_MAX) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	buffer = kzalloc(buf_size, GFP_KERNEL);
	if (!buffer) {
		return -ENOMEM;
	}

	mutex_lock(&panel->panel_lock);

	mode = panel->cur_mode;
	cmds = mode->priv_info->cmd_sets[type].cmds;
	cmd_count = mode->priv_info->cmd_sets[type].count;
	state = mode->priv_info->cmd_sets[type].state;

	if (cmd_count == 0) {
		DISP_ERROR("[%s] No commands to be sent\n", cmd_set_prop_map[type]);
		rc = -EAGAIN;
		goto error;
	}

	DISP_INFO("set cmds [%s], count (%d), state(%s)\n",
		cmd_set_prop_map[type], cmd_count,
		(state == DSI_CMD_SET_STATE_LP) ? "dsi_lp_mode" : "dsi_hs_mode");

	for (i = 0; i < cmd_count; i++) {
		memset(buffer, 0, buf_size);
		buf_count = snprintf(buffer, buf_size, "%02X", cmds->msg.tx_len);
		tx_buf = (u8 *)cmds->msg.tx_buf;
		for (j = 0; j < cmds->msg.tx_len ; j++) {
			buf_count += snprintf(buffer + buf_count,
					buf_size - buf_count, " %02X", tx_buf[j]);
		}
		DISP_DEBUG("[%d] %s\n", i, buffer);
		cmds++;
	}

	rc = dsi_panel_tx_cmd_set(panel, type);

error:
	mutex_unlock(&panel->panel_lock);
	kfree(buffer);
	return rc;
}

ssize_t mi_dsi_panel_show_dsi_cmd_set_type(struct dsi_panel *panel,
			char *buf, size_t size)
{
	ssize_t count = 0;
	int type = 0;

	if (!panel || !buf) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	count = snprintf(buf, size, "%s: dsi cmd_set name\n", "id");

	for (type = DSI_CMD_SET_PRE_ON; type < DSI_CMD_SET_MAX; type++) {
		count += snprintf(buf + count, size - count, "%02d: %s\n",
				     type, cmd_set_prop_map[type]);
	}

	return count;
}

int mi_dsi_panel_update_doze_cmd_locked(struct dsi_panel *panel, u8 value)
{
	int rc = 0;
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	int i = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	cmd_update_index = DSI_CMD_SET_MI_DOZE_LBM_UPDATE;
	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (i = 0; i < cmd_update_count; i++){
		DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		rc = mi_dsi_panel_update_cmd_set(panel, cur_mode,
			DSI_CMD_SET_MI_DOZE_LBM, info,
			&value, sizeof(value));
		info++;
	}

	return rc;
}

int mi_dsi_panel_set_doze_brightness(struct dsi_panel *panel,
			u32 doze_brightness)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	unsigned long mode_flags = 0;
	u8 m16t_pb_cmd = 0x51;
	u8 m16t_pb_data_lbm[2] = {0x00, 0x10};
	u8 m16t_pb_data_hbm[2] = {0x00, 0xe5};

	mutex_lock(&panel->panel_lock);

	mi_cfg = &panel->mi_cfg;

	if (!panel->panel_initialized) {
		DISP_ERROR("panel is not initialized!\n");
		mutex_unlock(&panel->panel_lock);
		return -EINVAL;
	}

	if (is_hbm_fod_on(panel)) {
		mi_cfg->doze_brightness = doze_brightness;
		DISP_INFO("Skip! [%s] set doze brightness %d due to FOD_HBM_ON\n",
			panel->type, doze_brightness);
	} else if (panel->mi_cfg.aod_to_normal_status == true) {
		mi_cfg->doze_brightness = doze_brightness;
		mi_dsi_update_backlight_in_aod(panel, false);
	} else if (panel->mi_cfg.panel_state == PANEL_STATE_ON
		|| mi_cfg->doze_brightness != doze_brightness) {
		if (doze_brightness == DOZE_BRIGHTNESS_HBM) {
			panel->mi_cfg.panel_state = PANEL_STATE_DOZE_HIGH;
			if (mi_get_panel_id_by_dsi_panel(panel) == M16T_PANEL_PB &&
				panel->cur_mode->timing.refresh_rate != 30) {
				DISP_INFO("[%s] skip send DOZE_HBM cmd, refresh rate=%d\n",
					  panel->type, panel->cur_mode->timing.refresh_rate);
				mode_flags = panel->mipi_device.mode_flags;
				panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
				rc = mipi_dsi_dcs_write(&panel->mipi_device, m16t_pb_cmd,
					m16t_pb_data_hbm, sizeof(m16t_pb_data_hbm));
				panel->mipi_device.mode_flags = mode_flags;
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_HBM);
			}
			if (rc) {
				DISP_ERROR("[%s] failed to send DOZE_HBM cmd, rc=%d\n",
					panel->type, rc);
			} else {
				panel->mi_cfg.aod_to_normal_status = false;
			}
		} else if (doze_brightness == DOZE_BRIGHTNESS_LBM) {
			panel->mi_cfg.panel_state = PANEL_STATE_DOZE_LOW;
			if (mi_get_panel_id_by_dsi_panel(panel) == L2_PANEL_PA &&
				panel->mi_cfg.panel_batch_number < 0x20)
				mi_dsi_panel_update_doze_cmd_locked(panel, 0x25);
			if (mi_get_panel_id_by_dsi_panel(panel) == M16T_PANEL_PB &&
				panel->cur_mode->timing.refresh_rate != 30) {
				DISP_INFO("[%s] skip send DOZE_LBM cmd, refresh rate=%d\n",
					  panel->type, panel->cur_mode->timing.refresh_rate);
				mode_flags = panel->mipi_device.mode_flags;
				panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
				rc = mipi_dsi_dcs_write(&panel->mipi_device, m16t_pb_cmd,
					m16t_pb_data_lbm, sizeof(m16t_pb_data_lbm));
				panel->mipi_device.mode_flags = mode_flags;
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM);
			}
			if (rc) {
				DISP_ERROR("[%s] failed to send DOZE_LBM cmd, rc=%d\n",
					panel->type, rc);
			} else {
				panel->mi_cfg.aod_to_normal_status = false;
			}
		} else if (doze_brightness == DOZE_TO_NORMAL
				&& (panel->power_mode == SDE_MODE_DPMS_LP1
					|| panel->power_mode == SDE_MODE_DPMS_LP2)){

			if (mi_get_panel_id_by_dsi_panel(panel) != N16_PANEL_PA
				&& mi_get_panel_id_by_dsi_panel(panel) != N16_PANEL_PB) {
				mi_cfg->panel_state = PANEL_STATE_DOZE_TO_NORMAL;
			}

			if (mi_get_panel_id_by_dsi_panel(panel) == L10_PANEL_PA) {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM);
				if (rc) {
					DISP_ERROR("[%s] failed to send DOZE_LBM cmd, rc=%d\n",
						panel->type, rc);
				}
			}
		}
		mi_cfg->doze_brightness = doze_brightness;
		DISP_TIME_INFO("[%s] set doze brightness to %s\n",
			panel->type, get_doze_brightness_name(doze_brightness));
	} else {
		DISP_INFO("[%s] %s has been set, skip\n", panel->type,
			get_doze_brightness_name(doze_brightness));
	}

	mutex_unlock(&panel->panel_lock);

	return rc;
}

int mi_dsi_panel_get_doze_brightness(struct dsi_panel *panel,
			u32 *doze_brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	mi_cfg = &panel->mi_cfg;
	*doze_brightness =  mi_cfg->doze_brightness;

	mutex_unlock(&panel->panel_lock);

	return 0;
}

int mi_dsi_panel_get_brightness(struct dsi_panel *panel,
			u32 *brightness)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	mi_cfg = &panel->mi_cfg;
	*brightness =  mi_cfg->last_bl_level;

	mutex_unlock(&panel->panel_lock);

	return 0;
}

int mi_dsi_panel_write_dsi_cmd(struct dsi_panel *panel,
			struct dsi_cmd_rw_ctl *ctl)
{
	struct dsi_panel_cmd_set cmd_sets = {0};
	u32 packet_count = 0;
	u32 dlen = 0;
	int rc = 0;

	mutex_lock(&panel->panel_lock);

	if (!panel || !panel->panel_initialized) {
		DISP_ERROR("Panel not initialized!\n");
		rc = -EAGAIN;
		goto exit_unlock;
	}

	if (!ctl->tx_len || !ctl->tx_ptr) {
		DISP_ERROR("[%s] invalid params\n", panel->type);
		rc = -EINVAL;
		goto exit_unlock;
	}

	rc = dsi_panel_get_cmd_pkt_count(ctl->tx_ptr, ctl->tx_len, &packet_count);
	if (rc) {
		DISP_ERROR("[%s] write dsi commands failed, rc=%d\n",
			panel->type, rc);
		goto exit_unlock;
	}

	DISP_DEBUG("[%s] packet-count=%d\n", panel->type, packet_count);

	rc = dsi_panel_alloc_cmd_packets(&cmd_sets, packet_count);
	if (rc) {
		DISP_ERROR("[%s] failed to allocate cmd packets, rc=%d\n",
			panel->type, rc);
		goto exit_unlock;
	}

	rc = dsi_panel_create_cmd_packets(ctl->tx_ptr, dlen, packet_count,
				cmd_sets.cmds);
	if (rc) {
		DISP_ERROR("[%s] failed to create cmd packets, rc=%d\n",
			panel->type, rc);
		goto exit_free1;
	}

	if (ctl->tx_state == MI_DSI_CMD_LP_STATE) {
		cmd_sets.state = DSI_CMD_SET_STATE_LP;
	} else if (ctl->tx_state == MI_DSI_CMD_HS_STATE) {
		cmd_sets.state = DSI_CMD_SET_STATE_HS;
	} else {
		DISP_ERROR("[%s] command state unrecognized-%s\n",
			panel->type, cmd_sets.state);
		goto exit_free1;
	}

	rc = mi_dsi_panel_write_cmd_set(panel, &cmd_sets);
	if (rc) {
		DISP_ERROR("[%s] failed to send cmds, rc=%d\n", panel->type, rc);
		goto exit_free2;
	}

exit_free2:
	if (ctl->tx_len && ctl->tx_ptr)
		dsi_panel_destroy_cmd_packets(&cmd_sets);
exit_free1:
	if (ctl->tx_len && ctl->tx_ptr)
		dsi_panel_dealloc_cmd_packets(&cmd_sets);
exit_unlock:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int mi_dsi_panel_set_brightness_clone(struct dsi_panel *panel,
			u32 brightness_clone)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg;
	int disp_id = MI_DISP_PRIMARY;
	static int log_count = 20;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	mi_cfg = &panel->mi_cfg;

	mi_cfg->user_brightness_clone = brightness_clone;

	if (!mi_cfg->thermal_dimming_enabled) {
		if (brightness_clone > mi_cfg->thermal_max_brightness_clone)
			brightness_clone = mi_cfg->thermal_max_brightness_clone;
	}

	if (brightness_clone > mi_cfg->max_brightness_clone)
		brightness_clone = mi_cfg->max_brightness_clone;

	if (!brightness_clone)
		log_count = 20;

	if (log_count-- > 0)
		DISP_TIME_INFO("[%s] set brightness clone to %d\n",
				panel->type, brightness_clone);

	atomic_set(&mi_cfg->real_brightness_clone, brightness_clone);

	disp_id = mi_get_disp_id(panel->type);
	mi_disp_feature_event_notify_by_type(disp_id,
			MI_DISP_EVENT_BRIGHTNESS_CLONE,
			sizeof(mi_cfg->real_brightness_clone),
			atomic_read(&mi_cfg->real_brightness_clone));

	mutex_unlock(&panel->panel_lock);

	return rc;
}

int mi_dsi_panel_get_brightness_clone(struct dsi_panel *panel,
			u32 *brightness_clone)
{
	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	*brightness_clone =  atomic_read(&panel->mi_cfg.real_brightness_clone);

	return 0;
}

int mi_dsi_panel_get_max_brightness_clone(struct dsi_panel *panel,
			u32 *max_brightness_clone)
{
	struct mi_dsi_panel_cfg *mi_cfg;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;
	*max_brightness_clone =  mi_cfg->max_brightness_clone;

	return 0;
}

int mi_dsi_panel_switch_page_locked(struct dsi_panel *panel, u8 page_index)
{
	int rc = 0;
	u8 tx_buf[5] = {0x55, 0xAA, 0x52, 0x08, page_index};
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	int i = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!panel->panel_initialized) {
		DISP_ERROR("[%s] panel not initialized\n", panel->type);
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	cmd_update_index = DSI_CMD_SET_MI_SWITCH_PAGE_UPDATE;
	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (i = 0; i < cmd_update_count; i++){
		DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		mi_dsi_panel_update_cmd_set(panel, cur_mode,
			DSI_CMD_SET_MI_SWITCH_PAGE, info,
			tx_buf, sizeof(tx_buf));
		info++;
	}

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_SWITCH_PAGE);
	if (rc) {
		DISP_ERROR("[%s] failed to send switch page(%d) cmd\n",
			panel->type, page_index);
	}

	return rc;
}

int mi_dsi_panel_switch_page(struct dsi_panel *panel, u8 page_index)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = mi_dsi_panel_switch_page_locked(panel, page_index);

	mutex_unlock(&panel->panel_lock);

	return rc;
}

int mi_dsi_panel_read_and_update_flat_param(struct dsi_panel *panel)
{
	int ret = 0;
	mutex_lock(&panel->panel_lock);
	ret = mi_dsi_panel_read_and_update_flat_param_locked(panel);
	mutex_unlock(&panel->panel_lock);
	return ret;
}


int mi_dsi_panel_read_and_update_flat_param_locked(struct dsi_panel *panel)
{

	int rc = 0;
	struct flat_mode_cfg *flat_cfg = NULL;
	struct dsi_display *display;
	u32 num_display_modes;
	struct dsi_display_mode *cur_mode = NULL;
	struct dsi_display_mode_priv_info *priv_info;
	struct dsi_cmd_update_info *info = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 rdbuf[10];
	uint request_rx_len = 0;
	unsigned long mode_flags_backup = 0;
	ssize_t read_len = 0;
	int i = 0, j = 0;

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!panel->mi_cfg.flat_update_flag) {
		DISP_DEBUG("[%s] flat_update_flag is not configed\n", panel->type);
		return 0;
	}

	flat_cfg = &panel->mi_cfg.flat_cfg;
	if (flat_cfg->update_done) {
		DISP_DEBUG("flat param already updated\n");
		rc = 0;
		goto exit;
	}

	DISP_INFO("[%s] flat param update start\n", panel->type);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_PREPARE_READ_FLAT);
	if (rc) {
		DISP_ERROR("[%s] failed to send PREPARE_READ_FLAT cmd\n",
			panel->type);
		goto exit;
	}

	if (sizeof(rdbuf) >= sizeof(flat_cfg->flat_on_data) ) {
		request_rx_len = sizeof(flat_cfg->flat_on_data);
	} else {
		DISP_ERROR("please check 0xB8 read_buf size, must > or = %d\n",
			sizeof(flat_cfg->flat_on_data));
		rc = -EAGAIN;
		goto exit;
	}

	mode_flags_backup = panel->mipi_device.mode_flags;
	panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
	read_len = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8,
		rdbuf, request_rx_len);
	panel->mipi_device.mode_flags = mode_flags_backup;

	if (read_len == request_rx_len) {
		memcpy(flat_cfg->flat_on_data, rdbuf, read_len);
		for (i = 0; i < sizeof(flat_cfg->flat_on_data); i++)
			DISP_INFO("flat read 0xB8, flat_on_data[%d] = 0x%02X\n",
				i, flat_cfg->flat_on_data[i]);
	} else {
		DISP_INFO("read flat 0xB8 failed (%d)\n", read_len);
		rc = -EAGAIN;
		goto exit;
	}

	num_display_modes = panel->num_display_modes;
	display = to_dsi_display(panel->host);
	if (!display || !display->modes) {
		DISP_ERROR("invalid display or display->modes ptr\n");
		rc = -EINVAL;
		goto exit;
	}

	for (i = 0; i < num_display_modes; i++) {
		cur_mode = &display->modes[i];
		DISP_INFO("[%s] update %d fps flat cmd\n", panel->type,
				cur_mode->timing.refresh_rate);

		priv_info = cur_mode->priv_info;
		cmd_update_index = DSI_CMD_SET_MI_FLAT_MODE_ON_UPDATE;
		info = priv_info->cmd_update[cmd_update_index];
		cmd_update_count = priv_info->cmd_update_count[cmd_update_index];
		for (j = 0; j < cmd_update_count; j++){
			DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
				DSI_CMD_SET_MI_FLAT_MODE_ON, info,
				flat_cfg->flat_on_data, sizeof(flat_cfg->flat_on_data));
			info++;
		}
	}

	if(mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L10_PANEL_PA &&
	   mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L9S_PANEL_PA){
			goto update_done;
	}

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_PREPARE_READ_FLAT_OFF);
	if (rc) {
		DISP_ERROR("[%s] failed to send PREPARE_READ_FLAT_OFF cmd\n",
			panel->type);
		goto exit;
	}

	if (sizeof(rdbuf) >= sizeof(flat_cfg->flat_off_data) ) {
		request_rx_len = sizeof(flat_cfg->flat_off_data);
	} else {
		DISP_ERROR("please check 0xB8 read_buf size, must > or = %d\n",
			sizeof(flat_cfg->flat_off_data));
		rc = -EAGAIN;
		goto exit;
	}

	mode_flags_backup = panel->mipi_device.mode_flags;
	panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
	read_len = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8,
		rdbuf, request_rx_len);
	panel->mipi_device.mode_flags = mode_flags_backup;

	if (read_len == request_rx_len) {
		memcpy(flat_cfg->flat_off_data, rdbuf, read_len);
		for (i = 0; i < sizeof(flat_cfg->flat_off_data); i++)
			DISP_INFO("flat off read 0xB8, flat_off_data[%d] = 0x%02X\n",
				i, flat_cfg->flat_off_data[i]);
	} else {
		DISP_INFO("read flat off 0xB8 failed (%d)\n", read_len);
		rc = -EAGAIN;
		goto exit;
	}

	for (i = 0; i < num_display_modes; i++) {
		cur_mode = &display->modes[i];
		DISP_INFO("[%s] update %d fps flat off cmd\n", panel->type,
				cur_mode->timing.refresh_rate);

		priv_info = cur_mode->priv_info;
		cmd_update_index = DSI_CMD_SET_MI_FLAT_MODE_OFF_UPDATE;
		info = priv_info->cmd_update[cmd_update_index];
		cmd_update_count = priv_info->cmd_update_count[cmd_update_index];
		for (j = 0; j < cmd_update_count; j++){
			DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
				DSI_CMD_SET_MI_FLAT_MODE_OFF, info,
				flat_cfg->flat_off_data, sizeof(flat_cfg->flat_off_data));
			info++;
		}
	}

update_done:
	flat_cfg->update_done = true;
	DISP_INFO("[%s] flat param update end\n", panel->type);
exit:

	return rc;
}

int mi_dsi_panel_read_and_update_dc_param(struct dsi_panel *panel)
{
	int ret = 0;
	mutex_lock(&panel->panel_lock);
	ret = mi_dsi_panel_read_and_update_dc_param_locked(panel);
	mutex_unlock(&panel->panel_lock);
	return ret;
}


int mi_dsi_panel_read_and_update_dc_param_locked(struct dsi_panel *panel)
{
	int rc = 0;
	struct dc_lut_cfg *dc_cfg = NULL;
	struct dsi_display *display;
	struct dsi_display_mode *cur_mode = NULL;
	unsigned long mode_flags_backup = 0;
	u32 num_display_modes;
	struct dsi_cmd_update_info *info = NULL;
	u8 read_dc_reg = 0;
	u8 read_count = 0;
	ssize_t read_len = 0;
	uint request_rx_len = 0;
	u8 read_buf[80] = {0};
	int i = 0, j = 0;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!panel->mi_cfg.dc_update_flag) {
		DISP_DEBUG("[%s] dc_update_flag is not configed\n", panel->type);
		return 0;
	}

	dc_cfg = &panel->mi_cfg.dc_cfg;
	if (dc_cfg->update_done) {
		DISP_DEBUG("DC param already updated\n");
		rc = 0;
		goto exit;
	}

	if (!panel->panel_initialized) {
		DISP_ERROR("[%s] Panel not initialized\n", panel->type);
		rc = -EINVAL;
		goto exit;
	}

	DISP_INFO("[%s] DC param update start\n", panel->type);

	/* switch page 4 */
	rc = mi_dsi_panel_switch_page_locked(panel, 4);
	if (rc) {
		DISP_ERROR("[%s] failed to send SWITCH_PAGE cmd\n",
			panel->type);
		goto exit;
	}

	for (read_count = DC_LUT_60HZ; read_count < DC_LUT_MAX; read_count++) {
		if (read_count == DC_LUT_60HZ) {
			read_dc_reg = 0xD2;
		} else {
			read_dc_reg = 0xD4;
		}

		if (sizeof(read_buf) >= sizeof(dc_cfg->exit_dc_lut[read_count])) {
			request_rx_len = sizeof(dc_cfg->exit_dc_lut[read_count]);
		} else {
			DISP_ERROR("please check 0x%02X read_buf size, must > or = %d\n",
				read_dc_reg, sizeof(dc_cfg->exit_dc_lut[read_count]));
			rc = -EAGAIN;
			goto exit;
		}
		DISP_INFO("[%s]read DC 0x%02X paramter length is %d\n",
			panel->type, read_dc_reg, request_rx_len);

		mode_flags_backup = panel->mipi_device.mode_flags;
		panel->mipi_device.mode_flags |= MIPI_DSI_MODE_LPM;
		read_len = mipi_dsi_dcs_read(&panel->mipi_device, read_dc_reg,
			read_buf, request_rx_len);
		panel->mipi_device.mode_flags = mode_flags_backup;
		if (read_len == request_rx_len) {
			memcpy(dc_cfg->exit_dc_lut[read_count], read_buf, read_len);
			for (i = 0; i < sizeof(dc_cfg->exit_dc_lut[read_count]); i++) {
				DISP_DEBUG("DC 0x%02X exit_dc_lut[%d] = 0x%02X\n",
					read_dc_reg, i, dc_cfg->exit_dc_lut[read_count][i]);
			}
		} else {
			DISP_INFO("read DC 0x%02X failed (%d)\n", read_dc_reg, read_len);
			rc = -EAGAIN;
			goto exit;
		}

		memcpy(dc_cfg->enter_dc_lut[read_count], dc_cfg->exit_dc_lut[read_count],
				sizeof(dc_cfg->exit_dc_lut[read_count]));
		for (i = 0; i < sizeof(dc_cfg->enter_dc_lut[read_count])/5; i++) {
			for (j = i * 5; j < i  * 5 + 3 ; j++) {
				dc_cfg->enter_dc_lut[read_count][j] =
					dc_cfg->exit_dc_lut[read_count][i * 5 + 2];
			}
		}
		for (i = 0; i < sizeof(dc_cfg->enter_dc_lut[read_count]); i++) {
			DISP_DEBUG("DC 0x%02X enter_dc_lut[%d] = 0x%02X\n",
				read_dc_reg, i, dc_cfg->enter_dc_lut[read_count][i]);
		}
		DISP_INFO("[%s] DC 0x%02X parameter read done\n",
			panel->type, read_dc_reg);

	}

	num_display_modes = panel->num_display_modes;
	display = to_dsi_display(panel->host);
	if (!display || !display->modes) {
		DISP_ERROR("invalid display or display->modes ptr\n");
		rc = -EINVAL;
		goto exit;
	}

	for (i = 0; i < num_display_modes; i++) {
		cur_mode = &display->modes[i];

		DISP_INFO("[%s] update %d fps DC cmd\n", panel->type,
			cur_mode->timing.refresh_rate);

		cmd_update_index = DSI_CMD_SET_MI_DC_ON_UPDATE;
		info = cur_mode->priv_info->cmd_update[cmd_update_index];
		cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
		for (j = 0; j < cmd_update_count; j++){
			DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			mi_dsi_panel_update_cmd_set(panel, cur_mode, DSI_CMD_SET_MI_DC_ON, info,
				dc_cfg->enter_dc_lut[j], sizeof(dc_cfg->enter_dc_lut[j]));
			info++;
		}

		cmd_update_index = DSI_CMD_SET_MI_DC_OFF_UPDATE;
		info = cur_mode->priv_info->cmd_update[cmd_update_index];
		cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
		for (j = 0; j < cmd_update_count; j++){
			DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			mi_dsi_panel_update_cmd_set(panel, cur_mode, DSI_CMD_SET_MI_DC_OFF,
				info, dc_cfg->exit_dc_lut[j], sizeof(dc_cfg->exit_dc_lut[j]));
			info++;
		}
	}

	dc_cfg->update_done = true;
	DISP_INFO("[%s] DC param update end\n", panel->type);

exit:
	return rc;
}

void mi_dsi_update_backlight_in_aod(struct dsi_panel *panel, bool restore_backlight)
{
	int bl_lvl = 0;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;
	struct mipi_dsi_device *dsi = &panel->mipi_device;

	if (restore_backlight) {
		bl_lvl = mi_cfg->last_bl_level;
	} else {
		switch (mi_cfg->doze_brightness) {
			case DOZE_BRIGHTNESS_HBM:
				bl_lvl = mi_cfg->doze_hbm_dbv_level;
				break;
			case DOZE_BRIGHTNESS_LBM:
				bl_lvl = mi_cfg->doze_lbm_dbv_level;
				break;
			default:
				return;
		}
	}
	DISP_INFO("[%s] mi_dsi_update_backlight_in_aod bl_lvl=%d\n",
			panel->type, bl_lvl);
	if (panel->bl_config.bl_inverted_dbv)
		bl_lvl = (((bl_lvl & 0xff) << 8) | (bl_lvl >> 8));
	mipi_dsi_dcs_set_display_brightness(dsi, bl_lvl);

	return;
}

int mi_dsi_update_51_mipi_cmd(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 bl_buf[2] = {0};
	int j = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	DISP_INFO("[%s] bl_lvl = %d\n", panel->type, bl_lvl);

	bl_buf[0] = (bl_lvl >> 8) & 0xff;
	bl_buf[1] = bl_lvl & 0xff;

	switch (type) {
	case DSI_CMD_SET_NOLP:
		cmd_update_index = DSI_CMD_SET_NOLP_UPDATE;
		break;
	case DSI_CMD_SET_MI_HBM_ON:
		cmd_update_index = DSI_CMD_SET_MI_HBM_ON_UPDATA;
		break;
	case DSI_CMD_SET_MI_HBM_OFF:
		cmd_update_index = DSI_CMD_SET_MI_HBM_OFF_UPDATA;
		break;
	case DSI_CMD_SET_MI_HBM_FOD_ON:
		cmd_update_index = DSI_CMD_SET_MI_HBM_FOD_ON_UPDATA;
		break;
	case DSI_CMD_SET_MI_HBM_FOD_OFF:
		cmd_update_index = DSI_CMD_SET_MI_HBM_FOD_OFF_UPDATA;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_HBM:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_HBM_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		break;
	default:
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		if (info && info->mipi_address != 0x51) {
			DISP_DEBUG("[%s] error mipi address (0x%02X)\n", panel->type, info->mipi_address);
			info++;
			continue;
		} else {
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
					type, info, bl_buf, sizeof(bl_buf));
			break;
		}
	}

	return 0;
}

static int mi_dsi_update_lhbm_cmd_87reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 alpha_buf[3] = {0};
	int j = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (!panel->mi_cfg.lhbm_alpha_ctrlaa) {
		DISP_DEBUG("local hbm can't use alpha control AA area brightness\n");
		return 0;
	}

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L12_PANEL_PA) {
		if (bl_lvl > 2047) {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl, 0xFFF);
			alpha_buf[0] = 0x10 | 0x0F ;
			alpha_buf[1] = 0xFF;
		} else {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl,  aa_alpha_L12_PANEL_PA[bl_lvl]);
			alpha_buf[0] = 0x10 | ((aa_alpha_L12_PANEL_PA[bl_lvl] >> 8) & 0xFF);
			alpha_buf[1] = aa_alpha_L12_PANEL_PA[bl_lvl] & 0xFF;
		}
	} else if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L12_PANEL_PB) {
		if (bl_lvl > 327) {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl, 0xFFF);
			alpha_buf[0] = 0x25;
			alpha_buf[1] = 0x0F;
			alpha_buf[2] = 0xFF;
		} else {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl,  aa_alpha_L12_PANEL_PB[bl_lvl]);
			alpha_buf[0] = 0x25;
			alpha_buf[1] = (aa_alpha_L12_PANEL_PB[bl_lvl] >> 8) & 0xFF;
			alpha_buf[2] = aa_alpha_L12_PANEL_PB[bl_lvl] & 0xFF;
		}
	} else if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L2S_PANEL_PB) {
		if(bl_lvl < 2047) {
			alpha_buf[0] = (aa_alpha_l2s_pb_set[bl_lvl] >> 8) & 0xFF;
			alpha_buf[1] = aa_alpha_l2s_pb_set[bl_lvl] & 0xFF;
		} else {
			alpha_buf[0] = 0x0B;
			alpha_buf[1] = 0xAC;
		}
		DISP_INFO("[%s] pb bl_lvl = %d, alpha = 0x%x%x\n",
                                        panel->type, bl_lvl, alpha_buf[0], alpha_buf[1]);
	} else if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L9S_PANEL_PA) {
		DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
				panel->type, bl_lvl,  aa_alpha_L9S_PANEL_PA[bl_lvl]);

		alpha_buf[0] = (aa_alpha_L9S_PANEL_PA[bl_lvl] >> 8) & 0x0f;
		alpha_buf[1] = aa_alpha_L9S_PANEL_PA[bl_lvl] & 0xff;
	} else if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L9S_PANEL_PB) {
		if (bl_lvl > 327) {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl, 0x1FFF);
			alpha_buf[0] = 0x1F;
			alpha_buf[1] = 0xFF;
		} else {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl,  aa_alpha_L9S_PANEL_PB[bl_lvl]);
			alpha_buf[0] = (aa_alpha_L9S_PANEL_PB[bl_lvl] >> 8) & 0xff;
			alpha_buf[1] = aa_alpha_L9S_PANEL_PB[bl_lvl] & 0xff;
		}
	} else if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M11A_PANEL_PA) {
		if (bl_lvl > 327) {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl, 0xFFF);
			alpha_buf[0] = 0x10 | 0x0F ;
			alpha_buf[1] = 0xFF;
		} else {
			DISP_INFO("[%s] bl_lvl = %d, alpha = 0x%x\n",
					panel->type, bl_lvl,  aa_alpha_M11A_PANEL_PA[bl_lvl]);
			alpha_buf[0] = 0x10 | ((aa_alpha_M11A_PANEL_PA[bl_lvl] >> 8) & 0xFF);
			alpha_buf[1] = aa_alpha_M11A_PANEL_PA[bl_lvl] & 0xFF;
		}
	} else {
		DISP_INFO("[%s] bl_lvl = %d, alpha = %d\n",
				panel->type, bl_lvl,  aa_alpha_set[bl_lvl]);

		alpha_buf[0] = (aa_alpha_set[bl_lvl] >> 8) & 0x0f;
		alpha_buf[1] = aa_alpha_set[bl_lvl] & 0xff;
	}

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		if (info && info->mipi_address != 0x87) {
			DISP_ERROR("[%s] error mipi address (0x%02X)\n", panel->type, info->mipi_address);
			info++;
			continue;
		} else {
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
					type, info, alpha_buf, info->length);
			break;
		}
	}

	return 0;
}

static int mi_dsi_update_lhbm_cmd_N16_PB(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_display_mode_priv_info *priv_info = NULL;
	struct dsi_cmd_update_info *info = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	int j = 0;
	u8 d1_reg_buf[6] = {0};
	u8 a9_reg_buf[14] = {0x02,0x00,0xB5,0x2C,0x2C,0x00,0x01,0x00,0x87,0x00,0x02,0x25,0x20,0x67};

	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;
	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE] == FEATURE_ON || mi_cfg->aod_to_normal_status)
	{
		if (bl_lvl <= 2047)
		{
			a9_reg_buf [12] = (aa_alpha_n16_pb_gir_on_set[bl_lvl] >> 8) & 0xFF;
			a9_reg_buf [13] = aa_alpha_n16_pb_gir_on_set[bl_lvl] & 0xFF;
		} else {
			a9_reg_buf [12] = (aa_alpha_n16_pb_gir_on_set[2048] >> 8) & 0xFF;
			a9_reg_buf [13] = aa_alpha_n16_pb_gir_on_set[2048] & 0xFF;
		}
	} else {
		if (bl_lvl <= 2047)
		{
			a9_reg_buf [12] = (aa_alpha_n16_pb_gir_off_set[bl_lvl] >> 8) & 0xFF;
			a9_reg_buf [13] = aa_alpha_n16_pb_gir_off_set[bl_lvl] & 0xFF;
		} else {
			a9_reg_buf [12] = (aa_alpha_n16_pb_gir_off_set[2048] >> 8) & 0xFF;
			a9_reg_buf [13] = aa_alpha_n16_pb_gir_off_set[2048] & 0xFF;
		}
	}

	DISP_INFO("[%s] bl_lvl = %d, a9 reg alpha= 0x%02X 0x%02X \n",
			panel->type, bl_lvl, a9_reg_buf[12], a9_reg_buf[13]);

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		memcpy(&d1_reg_buf[0], &panel->mi_cfg.lhbm_rgb_param[0],sizeof(u8)*6);
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		memcpy(&d1_reg_buf[0], &panel->mi_cfg.lhbm_rgb_param[0],sizeof(u8)*6);
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		memcpy(&d1_reg_buf[0], &panel->mi_cfg.lhbm_rgb_param[6],sizeof(u8)*6);
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		memcpy(&d1_reg_buf[0], &panel->mi_cfg.lhbm_rgb_param[12],sizeof(u8)*6);
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		memcpy(&d1_reg_buf[0], &panel->mi_cfg.lhbm_rgb_param[12],sizeof(u8)*6);
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}
	
	priv_info = panel->cur_mode->priv_info;
	info = priv_info->cmd_update[cmd_update_index];
	cmd_update_count = priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		if (info) {
			DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			if(info->mipi_address == 0xD1) {
				mi_dsi_panel_update_cmd_set(panel, panel->cur_mode,
						type, info, d1_reg_buf, sizeof(d1_reg_buf));
			} else if (info->mipi_address == 0xA9) {
				mi_dsi_panel_update_cmd_set(panel, panel->cur_mode,
						type, info, a9_reg_buf, sizeof(a9_reg_buf));
			} 
			info++;
		}
	}

	return 0;
}

static int mi_dsi_update_lhbm_cmd_DF_reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 df_reg_buf[2] = {0};
	int j = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (!panel->mi_cfg.lhbm_ctrl_df_reg) {
		DISP_DEBUG("local hbm can't use control DF reg\n");
		return 0;
	}

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M11A_PANEL_PA) {
		if (bl_lvl > 327) {
			df_reg_buf[0] = ((bl_lvl * 4) >> 8) & 0xFF;
			df_reg_buf[1] = (bl_lvl * 4) & 0xFF;
		} else {
			df_reg_buf[0] = 0x05;
			df_reg_buf[1] = 0x1C;
		}
	} else {
		if (bl_lvl > 328) {
			df_reg_buf[0] = ((bl_lvl * 4) >> 8) & 0xFF;
			df_reg_buf[1] = (bl_lvl * 4) & 0xFF;
		} else {
			df_reg_buf[0] = 0x05;
			df_reg_buf[1] = 0x20;
		}
	}

	DISP_INFO("[%s] bl_lvl = %d, df reg = 0x%x 0x%x\n",
			panel->type, bl_lvl, df_reg_buf[0], df_reg_buf[1]);

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		if (info && info->mipi_address != 0xDF) {
			DISP_ERROR("[%s] error mipi address (0x%02X)\n", panel->type, info->mipi_address);
			info++;
			continue;
		} else {
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
					type, info, df_reg_buf, sizeof(df_reg_buf));
			break;
		}
	}

	return 0;
}

static int mi_dsi_update_lhbm_cmd_63_C5_reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u32 rgb_gamma_num = 0;
	u8 reg_63_buf[4] = {0};
	u8 reg_C5_buf[18] = {0};
	u8 reg_51_buf[2] = {0};
	int i = 0;
	enum {
		lhbm_white_normal = 1,
		lhbm_green_normal = 2,
		lhbm_white_low = 3,
		lhbm_max = 4,
  	};

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (!panel->mi_cfg.lhbm_ctrl_63_C5_reg) {
		DISP_DEBUG("local hbm can't use control 63 C5 reg\n");
		return 0;
	}

	if (bl_lvl >= 327) {
		if(bl_lvl > 2047){
			DISP_INFO("[%s] backlight %d in local hbm can't exceed 2047\n", panel->type, bl_lvl);
			bl_lvl = 2047;
		}
		reg_63_buf[0] = 0x10;
		reg_63_buf[1] = 0x00;
		reg_63_buf[2] = (bl_lvl >> 8) & 0xFF;
		reg_63_buf[3] = bl_lvl & 0xFF;
	} else {
		if(bl_lvl == 0){
			DISP_ERROR("backlight is zero, return\n");
			return -EINVAL;
		}
		reg_63_buf[0] = (lhbm_alpha_gain_pa[bl_lvl - 1] >> 8) & 0xFF;;
		reg_63_buf[1] = lhbm_alpha_gain_pa[bl_lvl - 1] & 0xFF;
		reg_63_buf[2] = 0x01;
		reg_63_buf[3] = 0x47;
	}

	reg_51_buf[0] = (bl_lvl >> 8) & 0xFF;
	reg_51_buf[1] = bl_lvl & 0xFF;

	DISP_DEBUG("[%s] bl_lvl = %d, 63 reg = 0x%02X 0x%02X 0x%02X 0x%02X\n",
			panel->type, bl_lvl, reg_63_buf[0], reg_63_buf[1], reg_63_buf[2], reg_63_buf[3]);

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		rgb_gamma_num = lhbm_white_normal;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		rgb_gamma_num = lhbm_white_normal;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		rgb_gamma_num = lhbm_green_normal;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		rgb_gamma_num = lhbm_white_low;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		rgb_gamma_num = lhbm_white_low;
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	memcpy(&reg_C5_buf[0], &panel->mi_cfg.lhbm_rgb_param[6 * (rgb_gamma_num - 1)], sizeof(u8) * 6);

	memcpy(&reg_C5_buf[6], &panel->mi_cfg.lhbm_rgb_param[6 * (rgb_gamma_num - 1)], sizeof(u8) * 6);

	memcpy(&reg_C5_buf[12], &panel->mi_cfg.lhbm_rgb_param[6 * (rgb_gamma_num - 1)], sizeof(u8) * 6);

	DISP_DEBUG("[%s] C5 reg = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
			panel->type, reg_C5_buf[0], reg_C5_buf[1], reg_C5_buf[2], reg_C5_buf[3], reg_C5_buf[4], reg_C5_buf[5]);

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (i = 0; i < cmd_update_count; i++) {
		if(info){
			DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
				panel->type, cmd_set_prop_map[info->type],
				info->mipi_address, info->index, info->length);
			if(info->mipi_address == 0x63) {
				mi_dsi_panel_update_cmd_set(panel, cur_mode,
						type, info, reg_63_buf, sizeof(reg_63_buf));
			} else if (info->mipi_address == 0xC5) {
				mi_dsi_panel_update_cmd_set(panel, cur_mode,
						type, info, reg_C5_buf, sizeof(reg_C5_buf));
			} else if (info->mipi_address == 0x51) {
				mi_dsi_panel_update_cmd_set(panel, cur_mode,
						type, info, reg_51_buf, sizeof(reg_51_buf));
			}
			info++;
		}
	}

	return 0;
}

static int mi_dsi_update_lhbm_cmd_B2_reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	u8 b2_reg_buf[2] = {0};
	int j = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (!panel->mi_cfg.lhbm_ctrl_b2_reg) {
		DISP_DEBUG("local hbm can't use control B2 reg\n");
		return 0;
	}

	if(bl_lvl > 327) {
		b2_reg_buf[0] = (bl_lvl >> 8) & 0xFF;
		b2_reg_buf[1] = bl_lvl & 0xFF;
	}else {
		b2_reg_buf[0] = 0x01;
		b2_reg_buf[1] = 0x48;
	}
	DISP_INFO("[%s] bl_lvl = %d, b2 reg = 0x%x 0x%x\n",
			panel->type, bl_lvl, b2_reg_buf[0], b2_reg_buf[1]);

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		DISP_INFO("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		if (info && info->mipi_address != 0xB2) {
			DISP_ERROR("[%s] error mipi address (0x%02X)\n", panel->type, info->mipi_address);
			info++;
			continue;
		} else {
			mi_dsi_panel_update_cmd_set(panel, cur_mode,
					type, info, b2_reg_buf, sizeof(b2_reg_buf));
			break;
		}
	}

	return 0;
}

int mi_dsi_update_nolp_cmd_B2reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type)
{
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u8 set_buf[1] = {0};

	panel->mi_cfg.panel_state = PANEL_STATE_ON;

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L3_PANEL_PA &&
		mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L3S_PANEL_PA)
		return 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (panel->mi_cfg.feature_val[DISP_FEATURE_DC] == FEATURE_OFF)
		set_buf[0] = 0x18;
	else
		set_buf[0] = 0x98;

	switch (type) {
	case DSI_CMD_SET_NOLP:
		cmd_update_index = DSI_CMD_SET_NOLP_UPDATE;
		break;
	case DSI_CMD_SET_MI_DOZE_HBM_NOLP:
		cmd_update_index = DSI_CMD_SET_MI_DOZE_HBM_NOLP_UPDATE;
		break;
	case DSI_CMD_SET_MI_DOZE_LBM_NOLP:
		cmd_update_index = DSI_CMD_SET_MI_DOZE_LBM_NOLP_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_B2_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_B2_UPDATE;
		break;
	default :
		DISP_ERROR("[%s] unsupport cmd %s\n",
				panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	if (info && info->mipi_address != 0xB2) {
		DISP_ERROR("error mipi address (0x%02X)\n", info->mipi_address);
		return -EINVAL;
	}
	mi_dsi_panel_update_cmd_set(panel, cur_mode,
			type, info, set_buf, sizeof(set_buf));

	return 0;
}

/* Note: Factory version need flat cmd send out immediately,
 * do not care it may lead panel flash.
 * Dev version need flat cmd send out send with te
 */
static int mi_dsi_send_flat_sync_with_te_locked(struct dsi_panel *panel,
			bool enable)
{
	int rc = 0;

#ifdef DISPLAY_FACTORY_BUILD
	if (enable) {
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_ON);
		rc |= dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_SEC_ON);
	} else {
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_OFF);
		rc |= dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_SEC_OFF);
	}
	DISP_INFO("send flat %s cmd immediately", enable ? "ON" : "OFF");
#else
	/*flat cmd will be send out at mi_sde_connector_flat_fence*/
	DISP_DEBUG("flat cmd should send out sync with te");
#endif
	return rc;
}

int mi_dsi_panel_read_manufacturer_info(struct dsi_panel *panel,
		u32 manufacturer_info_addr, char *rdbuf, int rdlen)
{
	int ret = 0;

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	mutex_lock(&panel->panel_lock);
	ret = mi_dsi_panel_read_manufacturer_info_locked(panel, manufacturer_info_addr, rdbuf, rdlen);
	mutex_unlock(&panel->panel_lock);
	return ret;
}
int mi_dsi_panel_read_manufacturer_info_locked(struct dsi_panel *panel,
		u32 manufacturer_info_addr, char *rdbuf, int rdlen)
{
	int rc = 0;
	u8 tesk_key_enable[2] = {0x5A,0x5A};
	u8 tesk_key_disable[2] = {0xA5,0xA5};
	u8 cmd = 0xF0;

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}
	if (mi_get_panel_id_by_dsi_panel(panel) == L18_PANEL_PA){
		rc = mipi_dsi_dcs_write(&panel->mipi_device, cmd, tesk_key_enable, sizeof(tesk_key_enable));
		if(rc < 0){
			DISP_ERROR("[%s] wrie tesk_key_enable failed, rc=%d\n", panel->type, rc);
			goto exit;
		}
	}
	rc = mipi_dsi_dcs_read(&panel->mipi_device, manufacturer_info_addr, rdbuf, rdlen);
	if (rc < 0) {
		DISP_ERROR("[%s] read manufacturer_info param failed, rc=%d\n", panel->type, rc);
		goto exit;
	}
	if (rc != rdlen) {
		DISP_INFO("read manufacturer_info failed (%d)\n", rc);
		goto exit;
	}
	if (mi_get_panel_id_by_dsi_panel(panel) == L18_PANEL_PA){
		rc = mipi_dsi_dcs_write(&panel->mipi_device, cmd, tesk_key_disable, sizeof(tesk_key_disable));
		if(rc < 0){
			DISP_ERROR("[%s] wrie tesk_key_enable failed, rc=%d\n", panel->type, rc);
			goto exit;
		}
	}
exit:
	return rc;
}

int mi_dsi_panel_read_lhbm_white_param(struct dsi_panel *panel)
{
	int ret = 0;
	mutex_lock(&panel->panel_lock);
	ret = mi_dsi_panel_read_lhbm_white_param_locked(panel);
	mutex_unlock(&panel->panel_lock);
	return ret;
}


int mi_dsi_panel_read_lhbm_white_param_locked(struct dsi_panel *panel)
{
	int rc = 0;
	u8 r_buf[20] = {0};
	u8 g_buf[20] = {0};
	u8 b_buf[20] = {0};
	int rdlength = 0;
	struct mi_dsi_panel_cfg *mi_cfg = &panel->mi_cfg;
	int j=0;

	if (!panel || !panel->host) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!mi_cfg->lhbm_w1000_update_flag && !mi_cfg->lhbm_w110_update_flag) {
		DISP_DEBUG("[%s] don't need read back white rgb config\n", panel->type);
		return 0;
	}

	if (!panel->panel_initialized) {
		DISP_ERROR("[%s] Panel not initialized\n", panel->type);
		rc = -EINVAL;
		goto exit;
	}

	if (mi_cfg->uefi_read_lhbm_success ||
		(mi_cfg->lhbm_w1000_readbackdone && mi_cfg->lhbm_w110_readbackdone)) {
		DISP_DEBUG("lhbm white rgb read back done\n");
		rc = 0;
		goto exit;
	}

	DISP_INFO("[%s] LHBM white read cmd start\n", panel->type);

	if (mi_cfg->lhbm_w1000_update_flag) {
		rdlength = 10;
		rc = dsi_panel_tx_cmd_set(panel,
			DSI_CMD_SET_MI_LOCAL_HBM_WHITE_1000NIT_GIRON_PRE_READ);
		if (rc) {
			DISP_ERROR("[%s] failed to send HBM_WHITE_1000NIT_GIRON_PRE_READ cmds, rc=%d\n",
				panel->type, rc);
			rc = -EINVAL;
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB2, r_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_on red param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB5, g_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_on green param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8, b_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_on blue param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		mi_cfg->whitebuf_1000_gir_on[0] = r_buf[8];
		mi_cfg->whitebuf_1000_gir_on[1] = r_buf[9];
		mi_cfg->whitebuf_1000_gir_on[2] = g_buf[8];
		mi_cfg->whitebuf_1000_gir_on[3] = g_buf[9];
		mi_cfg->whitebuf_1000_gir_on[4] = b_buf[8];
		mi_cfg->whitebuf_1000_gir_on[5] = b_buf[9];

		memset(r_buf, 0, sizeof(r_buf));
		memset(g_buf, 0, sizeof(g_buf));
		memset(b_buf, 0, sizeof(b_buf));

		rc = dsi_panel_tx_cmd_set(panel,
			DSI_CMD_SET_MI_LOCAL_HBM_WHITE_1000NIT_GIROFF_PRE_READ);
		if (rc) {
			DISP_ERROR("[%s] failed to send HBM_WHITE_1000NIT_GIROFF_PRE_READ cmds, rc=%d\n",
				panel->type, rc);
			rc = -EINVAL;
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB2, r_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_off red param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB5, g_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_off green param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8, b_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w1000_off blue param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		mi_cfg->whitebuf_1000_gir_off[0] =r_buf[8];
		mi_cfg->whitebuf_1000_gir_off[1] =r_buf[9];
		mi_cfg->whitebuf_1000_gir_off[2] =g_buf[8];
		mi_cfg->whitebuf_1000_gir_off[3] =g_buf[9];
		mi_cfg->whitebuf_1000_gir_off[4] =b_buf[8];
		mi_cfg->whitebuf_1000_gir_off[5] =b_buf[9];

		mi_cfg->lhbm_w1000_readbackdone = true;

		for (j = 0; j < sizeof(mi_cfg->whitebuf_1000_gir_on); j++)
			DISP_INFO("read from 0xB2,0xB5,0xB8 whitebuf_1000_gir_on[%d] = 0x%02X\n", j, mi_cfg->whitebuf_1000_gir_on[j]);

		for (j = 0; j < sizeof(mi_cfg->whitebuf_1000_gir_off); j++)
			DISP_INFO("read from 0xB2,0xB5, 0xB8,whitebuf_1000_gir_off[%d] = 0x%02X\n", j, mi_cfg->whitebuf_1000_gir_off[j]);
	} else {
		mi_cfg->lhbm_w1000_readbackdone = true;
	}

	if (mi_cfg->lhbm_w110_update_flag) {
		rdlength = 4;

		rc = dsi_panel_tx_cmd_set(panel,
			DSI_CMD_SET_MI_LOCAL_HBM_WHITE_110NIT_GIRON_PRE_READ);
		if (rc) {
			DISP_ERROR("[%s] failed to send HBM_WHITE_110NIT_GIRON_PRE_READ cmds, rc=%d\n",
				panel->type, rc);
			rc = -EINVAL;
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB2, r_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_on red param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB5, g_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_on green param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8, b_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_on blue param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		mi_cfg->whitebuf_110_gir_on[0] = r_buf[2];
		mi_cfg->whitebuf_110_gir_on[1] = r_buf[3];
		mi_cfg->whitebuf_110_gir_on[2] = g_buf[2];
		mi_cfg->whitebuf_110_gir_on[3] = g_buf[3];
		mi_cfg->whitebuf_110_gir_on[4] = b_buf[2];
		mi_cfg->whitebuf_110_gir_on[5] = b_buf[3];

		memset(r_buf, 0, sizeof(r_buf));
		memset(g_buf, 0, sizeof(g_buf));
		memset(b_buf, 0, sizeof(b_buf));

		rc = dsi_panel_tx_cmd_set(panel,
			DSI_CMD_SET_MI_LOCAL_HBM_WHITE_110NIT_GIROFF_PRE_READ);
		if (rc) {
			DISP_ERROR("[%s] failed to send HBM_WHITE_110NIT_GIROFF_PRE_READ cmds, rc=%d\n",
				panel->type, rc);
			rc = -EINVAL;
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB2, r_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_off red param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB5, g_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_off green param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		rc = mipi_dsi_dcs_read(&panel->mipi_device, 0xB8, b_buf, rdlength);
		if (rc < 0) {
			DISP_ERROR("[%s] read back w110_off blue param failed, rc=%d\n", panel->type, rc);
			goto exit;
		}

		mi_cfg->whitebuf_110_gir_off[0] = r_buf[2];
		mi_cfg->whitebuf_110_gir_off[1] = r_buf[3];
		mi_cfg->whitebuf_110_gir_off[2] = g_buf[2];
		mi_cfg->whitebuf_110_gir_off[3] = g_buf[3];
		mi_cfg->whitebuf_110_gir_off[4] = b_buf[2];
		mi_cfg->whitebuf_110_gir_off[5] = b_buf[3];

		mi_cfg->lhbm_w110_readbackdone = true;

		for (j = 0; j < sizeof(mi_cfg->whitebuf_110_gir_on); j++)
			DISP_INFO("read from 0xB2,0xB5, 0xB8,whitebuf_110_gir_on[%d] = 0x%02X\n", j, mi_cfg->whitebuf_110_gir_on[j]);

		for (j = 0; j < sizeof(mi_cfg->whitebuf_110_gir_off); j++)
			DISP_INFO("read from 0xB2,0xB5, 0xB8,whitebuf_110_gir_off[%d] = 0x%02X\n", j, mi_cfg->whitebuf_110_gir_off[j]);
	} else {
		mi_cfg->lhbm_w110_readbackdone = true;
	}
	DISP_INFO("[%s] LHBM white param read back end\n", panel->type);

exit:
	return rc;
}

static int mi_dsi_panel_update_lhbm_white_param(struct dsi_panel * panel,
		enum dsi_cmd_set_type type,int flat_mode)
{
	int rc=0;
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u32 cmd_update_index = 0;
	u32 cmd_update_count = 0;
	int i= 0, j = 0, level = 1000;
	u8 *whitebuf = NULL;
	u8 coefficient = 0;
	u32 r = 0, g = 0, b = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	if (!panel->mi_cfg.lhbm_w1000_update_flag && !panel->mi_cfg.lhbm_w110_update_flag && !panel->mi_cfg.lhbm_g500_update_flag) {
		DISP_DEBUG("[%s] don't need update white rgb config\n", panel->type);
		return 0;
	}

	if (!panel->panel_initialized) {
		DISP_ERROR("[%s] Panel not initialized\n", panel->type);
		rc = -EINVAL;
		goto exit;
	}

	if (panel->mi_cfg.uefi_read_lhbm_success &&
		(!panel->mi_cfg.lhbm_w1000_readbackdone || !panel->mi_cfg.lhbm_w110_readbackdone)) {
		for (i = 0; i < sizeof(panel->mi_cfg.lhbm_rgb_param); i++) {
			if (i < 6)
				panel->mi_cfg.whitebuf_1000_gir_on[i] = panel->mi_cfg.lhbm_rgb_param[i];
			else if (i < 12)
					panel->mi_cfg.whitebuf_1000_gir_off[i-6] = panel->mi_cfg.lhbm_rgb_param[i];
			else if (i < 18)
				panel->mi_cfg.whitebuf_110_gir_on[i-12] = panel->mi_cfg.lhbm_rgb_param[i];
			else if (i < 24)
				panel->mi_cfg.whitebuf_110_gir_off[i-18] = panel->mi_cfg.lhbm_rgb_param[i];
		}

		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L9S_PANEL_PA) {
			for (i = 0; i < sizeof(panel->mi_cfg.lhbm_rgb_param); i++) {
				if (i < 6) {
					panel->mi_cfg.whitebuf_1000_gir_on[i] = panel->mi_cfg.lhbm_rgb_param[i];
					panel->mi_cfg.whitebuf_1000_gir_off[i] = panel->mi_cfg.lhbm_rgb_param[i];
				} else if (i < 12) {
					panel->mi_cfg.whitebuf_110_gir_off[i-6] = panel->mi_cfg.lhbm_rgb_param[i];
				} else if (i < 18) {
					panel->mi_cfg.whitebuf_110_gir_on[i-12] = panel->mi_cfg.lhbm_rgb_param[i];
				} else if (i < 24) {
					panel->mi_cfg.greenbuf_500nit[i-18] = panel->mi_cfg.lhbm_rgb_param[i];
				}
			}
			for(i = 2;i < 4;i++){
				panel->mi_cfg.greenbuf_500nit[i-2] = 0;
				panel->mi_cfg.greenbuf_500nit[i+2] = 0;
			}
		}

		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L9S_PANEL_PB) {
			for (i = 0; i < 18; i++) {
				if (i < 6) {
					panel->mi_cfg.whitebuf_1000_gir_on[i] = panel->mi_cfg.lhbm_rgb_param[i];
					panel->mi_cfg.whitebuf_1000_gir_off[i] = panel->mi_cfg.lhbm_rgb_param[i];
				} else if (i < 12) {
					panel->mi_cfg.whitebuf_110_gir_on[i-6] = panel->mi_cfg.lhbm_rgb_param[i];
					panel->mi_cfg.whitebuf_110_gir_off[i-6] = panel->mi_cfg.lhbm_rgb_param[i];
				} else if (i < 18) {
					panel->mi_cfg.greenbuf_500nit[i-12] = panel->mi_cfg.lhbm_rgb_param[i];
				}
			}
			for(i = 2;i < 4;i++){
				panel->mi_cfg.greenbuf_500nit[i-2] = 0;
				panel->mi_cfg.greenbuf_500nit[i+2] = 0;
			}
		}

		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L12_PANEL_PB) {
			/*  900nit GAMMA R=R[11:0]*1.09; 900nit GAMMA G=G[11:0]*1.08; 900nit GAMMA B=B[11:0]*1.09
				110nit GAMMA R=R[11:0]*1.03; 110nit GAMMA G=G[11:0]*1.03; 110nit GAMMA B=B[11:0]*1.03
			*/
			for (i = 0; i < 4; i++) {
				if (i == 0) {
					coefficient = 109;
					whitebuf = panel->mi_cfg.whitebuf_1000_gir_on;
				} else if (i == 1) {
					coefficient = 109;
					whitebuf = panel->mi_cfg.whitebuf_1000_gir_off;
				} else if (i == 2) {
					coefficient = 103;
					whitebuf = panel->mi_cfg.whitebuf_110_gir_on;
				} else if (i == 3) {
					coefficient = 103;
					whitebuf = panel->mi_cfg.whitebuf_110_gir_off;
				}
				r = ((whitebuf[0] << 8) + whitebuf[1]) * coefficient;
				if (i == 0 || i == 1)
					g = ((whitebuf[2] << 8) + whitebuf[3]) * (coefficient - 1);
				else if(i == 2 || i == 3)
					g = ((whitebuf[2] << 8) + whitebuf[3]) * coefficient;

				b = ((whitebuf[4] << 8) + whitebuf[5]) * coefficient;
				r = r / 100 + (r % 100 /10 > 4 ? 1 : 0);
				g = g / 100 + (g % 100 /10 > 4 ? 1 : 0);
				b = b / 100 + (b % 100 /10 > 4 ? 1 : 0);

				whitebuf[0] = (r >> 8) & 0xFF;
				whitebuf[1] = r & 0xFF;
				whitebuf[2] =(g >> 8) & 0xFF;
				whitebuf[3] = g & 0xFF;
				whitebuf[4] = (b >> 8) & 0xFF;
				whitebuf[5] = b & 0xFF;
			}
		}

		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == M11A_PANEL_PA) {
			/*  250nit GAMMA R=R[11:0]+70; 250nit GAMMA G=G[11:0]+70; 250nit GAMMA B=B[11:0]+70
			*/
			for (i = 0; i < 2; i++) {
				if (i == 0)
					whitebuf = panel->mi_cfg.whitebuf_110_gir_on;
				else if (i == 1)
					whitebuf = panel->mi_cfg.whitebuf_110_gir_off;

				r = (whitebuf[0] << 8) + whitebuf[1] + 70;
				g = (whitebuf[2] << 8) + whitebuf[3] + 70;
				b = (whitebuf[4] << 8) + whitebuf[5] + 70;

				whitebuf[0] = (r >> 8) & 0xFF;
				whitebuf[1] = r & 0xFF;
				whitebuf[2] =(g >> 8) & 0xFF;
				whitebuf[3] = g & 0xFF;
				whitebuf[4] = (b >> 8) & 0xFF;
				whitebuf[5] = b & 0xFF;
			}
		}

		panel->mi_cfg.lhbm_w1000_readbackdone = true;
		panel->mi_cfg.lhbm_w110_readbackdone = true;
	}

	switch (type) {
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
		level = 1000;
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
		level = 1000;
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT:
		level = 110;
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT:
		level = 110;
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT_UPDATE;
		break;
	case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT:
		level = 500;
		cmd_update_index = DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT_UPDATE;
		break;
	default:
		DISP_ERROR("[%s] unsupport cmd %s\n", panel->type, cmd_set_prop_map[type]);
		return -EINVAL;
	}

	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	cmd_update_count = cur_mode->priv_info->cmd_update_count[cmd_update_index];
	for (j = 0; j < cmd_update_count; j++) {
		DISP_DEBUG("[%s] update [%s] mipi_address(0x%02X) index(%d) lenght(%d)\n",
			panel->type, cmd_set_prop_map[info->type],
			info->mipi_address, info->index, info->length);
		if (info && info->mipi_address != 0xD0) {
			DISP_ERROR("error mipi address (0x%02X)\n", info->mipi_address);
			info++;
			continue;
		} else {
			if (level == 110) {
				if(flat_mode!= 0) {
					mi_dsi_panel_update_cmd_set(panel, cur_mode, type, info,
					panel->mi_cfg.whitebuf_110_gir_on, sizeof(panel->mi_cfg.whitebuf_110_gir_on));
				} else {
					mi_dsi_panel_update_cmd_set(panel, cur_mode, type, info,
					panel->mi_cfg.whitebuf_110_gir_off, sizeof(panel->mi_cfg.whitebuf_110_gir_off));
				}
			} else if (level == 500) {
				mi_dsi_panel_update_cmd_set(panel, cur_mode, type, info,
					panel->mi_cfg.greenbuf_500nit, sizeof(panel->mi_cfg.greenbuf_500nit));
			} else {
				if(flat_mode!= 0) {
					mi_dsi_panel_update_cmd_set(panel, cur_mode, type, info,
					panel->mi_cfg.whitebuf_1000_gir_on, sizeof(panel->mi_cfg.whitebuf_1000_gir_on));
				} else {
					mi_dsi_panel_update_cmd_set(panel, cur_mode, type, info,
					panel->mi_cfg.whitebuf_1000_gir_off, sizeof(panel->mi_cfg.whitebuf_1000_gir_off));
				}
			}
		}
	}

exit:
	return rc;
}

int mi_dsi_panel_set_round_corner_locked(struct dsi_panel *panel,
			bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (panel->mi_cfg.ddic_round_corner_enabled) {
		if (enable)
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_ROUND_CORNER_ON);
		else
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_ROUND_CORNER_OFF);

		if (rc)
			DISP_ERROR("[%s] failed to send ROUND_CORNER(%s) cmds, rc=%d\n",
				panel->type, enable ? "On" : "Off", rc);
	} else {
		DISP_INFO("[%s] ddic round corner feature not enabled\n", panel->type);
	}

	return rc;
}

int mi_dsi_panel_set_round_corner(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = mi_dsi_panel_set_round_corner_locked(panel, enable);

	mutex_unlock(&panel->panel_lock);

	return rc;
}

int mi_dsi_panel_set_flat_mode(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = mi_dsi_panel_set_flat_mode_locked(panel, enable);

	if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
		panel->mi_cfg.aod_to_normal_status = false;

	mutex_unlock(&panel->panel_lock);

	return rc;
}

int mi_dsi_panel_set_flat_mode_locked(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (enable)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_ON);
	else
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_OFF);

	if (rc)
		DISP_ERROR("failed to set flat mode: %d\n", enable);
	else
		DISP_INFO("success set flat mode: %d\n", enable);

	return rc;
}

int mi_dsi_panel_set_dc_mode(struct dsi_panel *panel, bool enable)
{
	int rc = 0;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = mi_dsi_panel_set_dc_mode_locked(panel, enable);

	mutex_unlock(&panel->panel_lock);

	return rc;
}


int mi_dsi_panel_set_dc_mode_locked(struct dsi_panel *panel, bool enable)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg  = NULL;

	if (!panel) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	if (mi_cfg->dc_feature_enable) {
		if (enable) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DC_ON);
			mi_cfg->real_dc_state = FEATURE_ON;
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DC_OFF);
			mi_cfg->real_dc_state = FEATURE_OFF;
		}

		if (rc)
			DISP_ERROR("failed to set DC mode: %d\n", enable);
		else
			DISP_INFO("DC mode: %s\n", enable ? "On" : "Off");
	} else {
		DISP_INFO("DC mode: TODO\n");
	}

	return rc;
}

static int mi_dsi_panel_set_lhbm_fod_locked(struct dsi_panel *panel,
		struct disp_feature_ctl *ctl)
{
	int rc = 0;
	int update_bl = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel || !ctl || ctl->feature_id != DISP_FEATURE_LOCAL_HBM) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	DISP_TIME_INFO("[%s] Local HBM: %s\n", panel->type,
			get_local_hbm_state_name(ctl->feature_val));

	switch (ctl->feature_val) {
	case LOCAL_HBM_OFF_TO_NORMAL:
		mi_dsi_update_backlight_in_aod(panel, true);
		if (mi_cfg->feature_val[DISP_FEATURE_HBM] == FEATURE_ON) {
			DISP_INFO("LOCAL_HBM_OFF_TO_HBM\n");
			mi_cfg->dimming_state = STATE_DIM_BLOCK;
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_HBM,
					mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_HBM);
		} else {
			DISP_INFO("LOCAL_HBM_OFF_TO_NORMAL\n");
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL,
					mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
			mi_dsi_panel_update_last_bl_level(panel, mi_cfg->last_bl_level);
		}
		break;
	case LOCAL_HBM_OFF_TO_NORMAL_BACKLIGHT:
		DISP_INFO("LOCAL_HBM_OFF_TO_NORMAL_BACKLIGHT\n");
		mi_dsi_update_backlight_in_aod(panel, false);
		update_bl = mi_cfg->last_bl_level;
		/* display backlight value should equal AOD brightness */
		if (is_aod_and_panel_initialized(panel)) {
			switch (mi_cfg->doze_brightness) {
			case DOZE_BRIGHTNESS_HBM:
				if (mi_cfg->last_no_zero_bl_level < mi_cfg->doze_hbm_dbv_level
					&& mi_cfg->feature_val[DISP_FEATURE_FP_STATUS] == AUTH_STOP) {
					update_bl = mi_cfg->last_no_zero_bl_level;
				} else {
					update_bl = mi_cfg->doze_hbm_dbv_level;
				}
				break;
			case DOZE_BRIGHTNESS_LBM:
				if (mi_cfg->last_no_zero_bl_level < mi_cfg->doze_lbm_dbv_level) {
					update_bl = mi_cfg->last_no_zero_bl_level;
				} else {
					update_bl = mi_cfg->doze_lbm_dbv_level;
				}
				break;
			default:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			}
		}

		DISP_INFO("LOCAL_HBM_OFF_TO_NORMAL_BACKLIGHT update_bl=%d\n", update_bl);

		mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL, update_bl);

		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL);
		mi_cfg->dimming_state = STATE_DIM_RESTORE;
		break;
	case LOCAL_HBM_OFF_TO_NORMAL_BACKLIGHT_RESTORE:
		/* display backlight value should equal unlock brightness */
		DISP_INFO("LOCAL_HBM_OFF_TO_NORMAL_BACKLIGHT_RESTORE\n");
		if ((mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PA ||
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB) && !is_hbm_fod_on(panel)) {
				DISP_INFO("local hbm is off,no need send backlight restore\n");
				break;
		} else {
			mi_dsi_update_backlight_in_aod(panel, true);
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL,
						mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_NORMAL);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
			mi_cfg->doze_brightness = DOZE_TO_NORMAL;
			break;
		}
	case LOCAL_HBM_NORMAL_WHITE_1000NIT:
		mi_cfg->dimming_state = STATE_DIM_BLOCK;

		update_bl = mi_cfg->last_bl_level;

		if (mi_cfg->feature_val[DISP_FEATURE_HBM] == FEATURE_ON &&
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L12_PANEL_PA &&
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) != L12_PANEL_PB) {
			DISP_INFO("LOCAL_HBM_NORMAL_WHITE_1000NIT in HBM\n");
			update_bl = panel->bl_config.bl_max_level;
		} else {
			if (is_aod_and_panel_initialized(panel)) {
				switch (mi_cfg->doze_brightness) {
				case DOZE_BRIGHTNESS_HBM:
					update_bl = mi_cfg->doze_hbm_dbv_level;
					break;
				case DOZE_BRIGHTNESS_LBM:
					update_bl = mi_cfg->doze_lbm_dbv_level;
					break;
				default:
					update_bl = mi_cfg->doze_hbm_dbv_level;
					break;
				}
			}
			DISP_INFO("LOCAL_HBM_NORMAL_WHITE_1000NIT update_bl = %d\n", update_bl);
		}

		mi_dsi_update_51_mipi_cmd(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		mi_dsi_update_lhbm_cmd_87reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		mi_dsi_update_lhbm_cmd_63_C5_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
			mi_dsi_update_lhbm_cmd_N16_PB(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		mi_dsi_update_lhbm_cmd_DF_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		mi_dsi_update_lhbm_cmd_B2_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT, update_bl);

		mi_dsi_panel_update_lhbm_white_param(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT,
				mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE]);
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT);
		break;
	case LOCAL_HBM_NORMAL_WHITE_750NIT:
		DISP_INFO("LOCAL_HBM_NORMAL_WHITE_750NIT\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_750NIT);
		break;
	case LOCAL_HBM_NORMAL_WHITE_500NIT:
		DISP_INFO("LOCAL_HBM_NORMAL_WHITE_500NIT\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_500NIT);
		break;
	case LOCAL_HBM_NORMAL_WHITE_110NIT:
		DISP_INFO("LOCAL_HBM_NORMAL_WHITE_110NIT\n");
		update_bl = mi_cfg->last_bl_level;
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PA && is_aod_and_panel_initialized(panel)) {
			switch (mi_cfg->doze_brightness) {
			case DOZE_BRIGHTNESS_HBM:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			case DOZE_BRIGHTNESS_LBM:
				update_bl = mi_cfg->doze_lbm_dbv_level;
				break;
			default:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			}
		}
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
			mi_dsi_update_lhbm_cmd_N16_PB(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
					update_bl);
		mi_dsi_update_lhbm_cmd_63_C5_reg(panel,DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
				update_bl);
		mi_dsi_update_lhbm_cmd_87reg(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
				update_bl);
		mi_dsi_update_51_mipi_cmd(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
				update_bl);
		mi_dsi_update_lhbm_cmd_B2_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
				update_bl);
		mi_dsi_panel_update_lhbm_white_param(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT,
				mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE]);
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_110NIT);
		break;
	case LOCAL_HBM_NORMAL_GREEN_500NIT:
		DISP_INFO("LOCAL_HBM_NORMAL_GREEN_500NIT\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		mi_dsi_update_51_mipi_cmd(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT, mi_cfg->last_bl_level);
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
			mi_dsi_update_lhbm_cmd_N16_PB(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT,
					mi_cfg->last_bl_level);
		mi_dsi_update_lhbm_cmd_63_C5_reg(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT,
				mi_cfg->last_bl_level);
		mi_dsi_update_lhbm_cmd_DF_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT, mi_cfg->last_bl_level);
		mi_dsi_update_lhbm_cmd_87reg(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT,
				mi_cfg->last_bl_level);
		mi_dsi_update_lhbm_cmd_B2_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT,
				mi_cfg->last_bl_level);
		mi_dsi_panel_update_lhbm_white_param(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT,
				mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE]);
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_GREEN_500NIT);
		break;
	case LOCAL_HBM_HLPM_WHITE_1000NIT:
		DISP_INFO("LOCAL_HBM_HLPM_WHITE_1000NIT\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		update_bl = mi_cfg->last_bl_level;
		mi_dsi_update_nolp_cmd_B2reg(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT);
		if (is_aod_and_panel_initialized(panel)) {
			switch (mi_cfg->doze_brightness) {
			case DOZE_BRIGHTNESS_HBM:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			case DOZE_BRIGHTNESS_LBM:
				update_bl = mi_cfg->doze_lbm_dbv_level;
				break;
			default:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			}
			DISP_INFO("LOCAL_HBM_HLPM_WHITE_1000NIT update_bl=%d\n", update_bl);
		}

		mi_dsi_update_51_mipi_cmd(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT, update_bl);
		mi_dsi_update_lhbm_cmd_87reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT, update_bl);
		mi_dsi_update_lhbm_cmd_B2_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT, update_bl);
		mi_dsi_update_lhbm_cmd_63_C5_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT, update_bl);
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
			mi_dsi_update_lhbm_cmd_N16_PB(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT, update_bl);

		mi_dsi_update_backlight_in_aod(panel, false);

		mi_dsi_panel_update_lhbm_white_param(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT,
				mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE]);

		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT);
		panel->mi_cfg.panel_state = PANEL_STATE_ON;
		panel->mi_cfg.aod_to_normal_status = true;
		break;
	case LOCAL_HBM_HLPM_WHITE_110NIT:
		DISP_INFO("LOCAL_HBM_HLPM_WHITE_110NIT\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		update_bl = mi_cfg->last_bl_level;
		mi_dsi_update_nolp_cmd_B2reg(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT);
		if (is_aod_and_panel_initialized(panel)) {
			switch (mi_cfg->doze_brightness) {
			case DOZE_BRIGHTNESS_HBM:
				update_bl = mi_cfg->doze_hbm_dbv_level;
				break;
			case DOZE_BRIGHTNESS_LBM:
				update_bl = mi_cfg->doze_lbm_dbv_level;
				break;
			default:
				update_bl = mi_cfg->doze_lbm_dbv_level;
				break;
			}
		}
		DISP_INFO("LOCAL_HBM_HLPM_WHITE_110NIT update_bl=%d\n", update_bl);
		mi_dsi_update_51_mipi_cmd(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT, update_bl);
		mi_dsi_update_lhbm_cmd_87reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT, update_bl);
		mi_dsi_update_lhbm_cmd_B2_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT, update_bl);
		mi_dsi_update_lhbm_cmd_63_C5_reg(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT, update_bl);
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
			mi_dsi_update_lhbm_cmd_N16_PB(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT, update_bl);

		mi_dsi_update_backlight_in_aod(panel, false);
		mi_dsi_panel_update_lhbm_white_param(panel,
				DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT,
				mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE]);
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_110NIT);
		panel->mi_cfg.panel_state = PANEL_STATE_ON;
		panel->mi_cfg.aod_to_normal_status = true;
		break;
	case LOCAL_HBM_OFF_TO_HLPM:
		DISP_INFO("LOCAL_HBM_OFF_TO_HLPM\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		panel->mi_cfg.panel_state = PANEL_STATE_DOZE_HIGH;
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_HLPM);
		break;
	case LOCAL_HBM_OFF_TO_LLPM:
		DISP_INFO("LOCAL_HBM_OFF_TO_LLPM\n");
		mi_cfg->dimming_state = STATE_DIM_BLOCK;
		panel->mi_cfg.panel_state = PANEL_STATE_DOZE_LOW;
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_LOCAL_HBM_OFF_TO_LLPM);
		break;
	default:
		DISP_ERROR("invalid local hbm value\n");
		break;
	}

	return rc;
}

int mi_dsi_panel_aod_to_normal_optimize_locked(struct dsi_panel *panel,
		bool enable)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel || !panel->panel_initialized) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;

	if (!mi_cfg->need_fod_animal_in_normal || panel->power_mode == SDE_MODE_DPMS_ON)
		return 0;

	DISP_TIME_DEBUG("[%s] fod aod_to_normal: %d\n", panel->type, enable);

	if (is_hbm_fod_on(panel)) {
		/*fod on is in normal state*/
		mi_cfg->aod_to_normal_status = true;
		DISP_INFO("fod hbm on, skip fod aod_to_normal: %d\n", enable);
		return 0;
	}

	if (enable && mi_cfg->panel_state != PANEL_STATE_ON) {
		switch (mi_cfg->doze_brightness) {
		case DOZE_BRIGHTNESS_HBM:
			DISP_INFO("enter DOZE HBM NOLP\n");
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_HBM_NOLP);
			if (rc) {
				DISP_ERROR("[%s] failed to send MI_DOZE_HBM_NOLP cmd, rc=%d\n",
					panel->type, rc);
			} else {
				panel->mi_cfg.panel_state = PANEL_STATE_ON;
				mi_cfg->aod_to_normal_status = true;
			}
			break;
		case DOZE_BRIGHTNESS_LBM:
			DISP_INFO("enter DOZE LBM NOLP\n");
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM_NOLP);
			if (rc) {
				DISP_ERROR("[%s] failed to send MI_DOZE_LBM_NOLP cmd, rc=%d\n",
					panel->type, rc);
			} else {
				panel->mi_cfg.panel_state = PANEL_STATE_ON;
				mi_cfg->aod_to_normal_status = true;
			}
			break;
		default:
			break;
		}
	} else if (!enable && mi_cfg->panel_state == PANEL_STATE_ON &&
		panel->power_mode != SDE_MODE_DPMS_ON &&
		mi_cfg->feature_val[DISP_FEATURE_FP_STATUS] != AUTH_STOP) {
		switch (mi_cfg->doze_brightness) {
		case DOZE_BRIGHTNESS_HBM:
			DISP_INFO("enter DOZE HBM\n");
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_DOZE_HBM,
					mi_cfg->doze_hbm_dbv_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_HBM);
			if (rc) {
				DISP_ERROR("[%s] failed to send MI_DOZE_HBM_NOLP cmd, rc=%d\n",
					panel->type, rc);
			}
			panel->mi_cfg.panel_state = PANEL_STATE_DOZE_HIGH;
			mi_cfg->aod_to_normal_status = false;
			break;
		case DOZE_BRIGHTNESS_LBM:
			DISP_INFO("enter DOZE LBM\n");
			if (mi_get_panel_id_by_dsi_panel(panel) == L2_PANEL_PA &&
				panel->mi_cfg.panel_batch_number < 0x20)
				mi_dsi_panel_update_doze_cmd_locked(panel, 0x25);
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_DOZE_LBM,
					mi_cfg->doze_lbm_dbv_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM);
			if (rc) {
				DISP_ERROR("[%s] failed to send MI_DOZE_LBM_NOLP cmd, rc=%d\n",
					panel->type, rc);
			}
			panel->mi_cfg.panel_state = PANEL_STATE_DOZE_LOW;
			mi_cfg->aod_to_normal_status = false;
			break;
		default:
			break;
		}
	} else {
		rc = -EAGAIN;
	}

	return rc;
}

bool mi_dsi_panel_is_need_tx_cmd(u32 feature_id)
{
	switch (feature_id) {
	case DISP_FEATURE_SENSOR_LUX:
	case DISP_FEATURE_LOW_BRIGHTNESS_FOD:
	case DISP_FEATURE_FP_STATUS:
	case DISP_FEATURE_FOLD_STATUS:
	case DISP_FEATURE_POWERSTATUS:
	case DISP_FEATURE_SYSTEM_BUILD_VERSION:
		return false;
	default:
		return true;
	}
}

static int mi_dsi_panel_gray_by_temperature(struct dsi_panel *panel, int temp_level)
{
	int rc = 0;
	struct dsi_cmd_update_info *info = NULL;
	struct dsi_display_mode *cur_mode = NULL;
	u8 gray_3d_lut_cfg = 0;
	u32 cmd_update_index = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	cur_mode = panel->cur_mode;

	cmd_update_index = DSI_CMD_SET_MI_3D_LUT_MODE_UPDATE;
	info = cur_mode->priv_info->cmd_update[cmd_update_index];
	if (temp_level == TEMP_INDEX1_36)
		gray_3d_lut_cfg = 0x82;
	else if (temp_level == TEMP_INDEX2_32)
		gray_3d_lut_cfg = 0x81;
	else if (temp_level == TEMP_INDEX3_28)
		gray_3d_lut_cfg = 0x80;
	else if (temp_level == TEMP_INDEX4_off)
		gray_3d_lut_cfg = 0x00;

	mi_dsi_panel_update_cmd_set(panel, cur_mode,
			DSI_CMD_SET_MI_3D_LUT_MODE, info,
			&gray_3d_lut_cfg, sizeof(gray_3d_lut_cfg));

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_3D_LUT_MODE);
	if (rc) {
		DISP_ERROR("[%s] failed to send 3d lut mode(%d) cmd\n",
			panel->type, gray_3d_lut_cfg);
	}

	return rc;
}

int mi_dsi_panel_set_disp_param(struct dsi_panel *panel, struct disp_feature_ctl *ctl)
{
	int rc = 0;
	struct mi_dsi_panel_cfg *mi_cfg = NULL;

	if (!panel || !ctl) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	DISP_TIME_INFO("[%s] feature: %s, value: %d\n", panel->type,
			get_disp_feature_id_name(ctl->feature_id), ctl->feature_val);


	mi_cfg = &panel->mi_cfg;

	if (!panel->panel_initialized &&
		mi_dsi_panel_is_need_tx_cmd(ctl->feature_id)) {
		if (ctl->feature_id == DISP_FEATURE_DC)
			mi_cfg->feature_val[DISP_FEATURE_DC] = ctl->feature_val;
		DISP_WARN("[%s] panel not initialized!\n", panel->type);
		rc = -ENODEV;
		goto exit;
	}


	switch (ctl->feature_id) {
	case DISP_FEATURE_DIMMING:
		if (panel->power_mode == SDE_MODE_DPMS_ON) {
			if (mi_cfg->dimming_state != STATE_DIM_BLOCK) {
				if (ctl->feature_val == FEATURE_ON)
					rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMINGON);
				else
					rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMINGOFF);
				mi_cfg->feature_val[DISP_FEATURE_DIMMING] = ctl->feature_val;
			} else {
				DISP_INFO("skip dimming %s\n", ctl->feature_val ? "on" : "off");
			}
		} else {
			DISP_INFO("skip dimming due to power_mode is not ON\n");
		}
		break;
	case DISP_FEATURE_HBM:
		mi_cfg->feature_val[DISP_FEATURE_HBM] = ctl->feature_val;
#ifdef DISPLAY_FACTORY_BUILD
		if (ctl->feature_val == FEATURE_ON) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_ON);
			mi_cfg->dimming_state = STATE_DIM_BLOCK;
		} else {
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_HBM_OFF,
					mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_OFF);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
		}
#else
		if (mi_get_panel_id_by_dsi_panel(panel) == L1_PANEL_PA ||
			mi_get_panel_id_by_dsi_panel(panel) == L2_PANEL_PA ||
			mi_get_panel_id_by_dsi_panel(panel) == L2S_PANEL_PA ||
			mi_get_panel_id_by_dsi_panel(panel) == L2S_PANEL_PB ||
			mi_get_panel_id_by_dsi_panel(panel) == L18_PANEL_PA ||
			mi_get_panel_id_by_dsi_panel(panel) == L18_PANEL_SA ||
			mi_get_panel_id_by_dsi_panel(panel) == M16T_PANEL_PB) {
			if (panel->power_mode == SDE_MODE_DPMS_ON) {
				if (ctl->feature_val == FEATURE_ON) {
					rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_ON);
					mi_cfg->dimming_state = STATE_DIM_BLOCK;
				} else {
					rc  = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_OFF);
					mi_cfg->dimming_state = STATE_DIM_RESTORE;
				}
			}
		}
#endif
		mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->type),
				MI_DISP_EVENT_HBM, sizeof(ctl->feature_val), ctl->feature_val);
		break;
	case DISP_FEATURE_HBM_FOD:
		if (ctl->feature_val == FEATURE_ON) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_FOD_ON);
			mi_cfg->dimming_state = STATE_DIM_BLOCK;
		} else {
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_HBM_FOD_OFF,
					mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_FOD_OFF);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
		}
		mi_cfg->feature_val[DISP_FEATURE_HBM_FOD] = ctl->feature_val;
		break;
	case DISP_FEATURE_DOZE_BRIGHTNESS:
#ifdef DISPLAY_FACTORY_BUILD
		if (dsi_panel_initialized(panel) &&
			is_aod_brightness(ctl->feature_val)) {
			if (ctl->feature_val == DOZE_BRIGHTNESS_HBM)
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_HBM);
			else
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM);
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NOLP);
		}
#else
		if (is_aod_and_panel_initialized(panel) &&
			is_aod_brightness(ctl->feature_val)) {
			if (ctl->feature_val == DOZE_BRIGHTNESS_HBM) {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_HBM);
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DOZE_LBM);
			}
		} else {
			mi_dsi_update_nolp_cmd_B2reg(panel, DSI_CMD_SET_NOLP);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NOLP);
		}
#endif
		mi_cfg->feature_val[DISP_FEATURE_DOZE_BRIGHTNESS] = ctl->feature_val;
		break;
	case DISP_FEATURE_FOD_CALIBRATION_BRIGHTNESS:
		if (ctl->feature_val == -1) {
			DISP_INFO("FOD calibration brightness restore last_bl_level=%d\n",
				mi_cfg->last_bl_level);
			dsi_panel_update_backlight(panel, mi_cfg->last_bl_level);
			mi_cfg->in_fod_calibration = false;
		} else {
			if (ctl->feature_val >= 0 &&
				ctl->feature_val <= panel->bl_config.bl_max_level) {
				mi_cfg->in_fod_calibration = true;
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_DIMMINGOFF);
				dsi_panel_update_backlight(panel, ctl->feature_val);
				mi_cfg->dimming_state = STATE_NONE;
			} else {
				mi_cfg->in_fod_calibration = false;
				DISP_ERROR("FOD calibration invalid brightness level:%d\n",
						ctl->feature_val);
			}
		}
		mi_cfg->feature_val[DISP_FEATURE_FOD_CALIBRATION_BRIGHTNESS] = ctl->feature_val;
		break;
	case DISP_FEATURE_FOD_CALIBRATION_HBM:
		if (ctl->feature_val == -1) {
			DISP_INFO("FOD calibration HBM restore last_bl_level=%d\n",
					mi_cfg->last_bl_level);
			mi_dsi_update_51_mipi_cmd(panel, DSI_CMD_SET_MI_HBM_FOD_OFF,
					mi_cfg->last_bl_level);
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_FOD_OFF);
			mi_cfg->dimming_state = STATE_DIM_RESTORE;
			mi_cfg->in_fod_calibration = false;
		} else {
			mi_cfg->in_fod_calibration = true;
			mi_cfg->dimming_state = STATE_DIM_BLOCK;
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_HBM_FOD_ON);
		}
		mi_cfg->feature_val[DISP_FEATURE_FOD_CALIBRATION_HBM] = ctl->feature_val;
		break;
	case DISP_FEATURE_FLAT_MODE:
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3_PANEL_PA ||
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L10_PANEL_PA ||
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3S_PANEL_PA ||
			mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L9S_PANEL_PA) {
			mi_dsi_panel_read_and_update_flat_param_locked(panel);
		}
		if (!mi_cfg->flat_sync_te) {
			if (ctl->feature_val == FEATURE_ON) {
				DISP_INFO("flat mode on\n");
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_ON);
				rc |= dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_SEC_ON);
			}
			else {
				DISP_INFO("flat mode off\n");
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_OFF);
				rc |= dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_MI_FLAT_MODE_SEC_OFF);
			}
			mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->type),
				MI_DISP_EVENT_FLAT_MODE, sizeof(ctl->feature_val), ctl->feature_val);
		} else {
			rc = mi_dsi_send_flat_sync_with_te_locked(panel,
					ctl->feature_val == FEATURE_ON);
		}
		mi_cfg->feature_val[DISP_FEATURE_FLAT_MODE] = ctl->feature_val;
		break;
	case DISP_FEATURE_CRC:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DC:
		DISP_INFO("DC mode state:%d\n", ctl->feature_val);
		if (mi_cfg->dc_feature_enable) {
			if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3_PANEL_PA ||
				mi_get_panel_id(panel->mi_cfg.mi_panel_id) == L3S_PANEL_PA) {
				mi_dsi_panel_read_and_update_dc_param_locked(panel);
			}
			rc = mi_dsi_panel_set_dc_mode_locked(panel, ctl->feature_val == FEATURE_ON);
		}
		mi_cfg->feature_val[DISP_FEATURE_DC] = ctl->feature_val;
		mi_disp_feature_event_notify_by_type(mi_get_disp_id(panel->type),
				MI_DISP_EVENT_DC, sizeof(ctl->feature_val), ctl->feature_val);
		break;
	case DISP_FEATURE_LOCAL_HBM:
		rc = mi_dsi_panel_set_lhbm_fod_locked(panel, ctl);
		mi_cfg->feature_val[DISP_FEATURE_LOCAL_HBM] = ctl->feature_val;
		break;
	case DISP_FEATURE_SENSOR_LUX:
		DISP_DEBUG("DISP_FEATURE_SENSOR_LUX=%d\n", ctl->feature_val);
		mi_cfg->feature_val[DISP_FEATURE_SENSOR_LUX] = ctl->feature_val;
		break;
	case DISP_FEATURE_FP_STATUS:
		mi_cfg->feature_val[DISP_FEATURE_FP_STATUS] = ctl->feature_val;
		if (ctl->feature_val == ENROLL_STOP ||
			ctl->feature_val == AUTH_STOP ||
			ctl->feature_val == HEART_RATE_STOP) {
			if (is_aod_and_panel_initialized(panel)) {
				mi_dsi_update_backlight_in_aod(panel, true);
			}
		}
		break;
	case DISP_FEATURE_FOLD_STATUS:
		DISP_INFO("DISP_FEATURE_FOLD_STATUS=%d\n", ctl->feature_val);
		mi_cfg->feature_val[DISP_FEATURE_FOLD_STATUS] = ctl->feature_val;
		break;
	case DISP_FEATURE_NATURE_FLAT_MODE:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_SPR_RENDER:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_AOD_TO_NORMAL:
		rc = mi_dsi_panel_aod_to_normal_optimize_locked(panel,
				ctl->feature_val == FEATURE_ON);
		mi_cfg->feature_val[DISP_FEATURE_AOD_TO_NORMAL] = ctl->feature_val;
		break;
	case DISP_FEATURE_COLOR_INVERT:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DC_BACKLIGHT:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_GIR:
		DISP_INFO("TODO\n");
		break;
	case DISP_FEATURE_DBI:
		if (mi_get_panel_id(panel->mi_cfg.mi_panel_id) == N16_PANEL_PB)
		{
			if (panel->mi_cfg.id_config.build_id != 0x00 && panel->mi_cfg.id_config.build_id != 0x10) {
				mi_cfg->feature_val[DISP_FEATURE_DBI] = ctl->feature_val;
				rc = mi_dsi_panel_gray_by_temperature(panel, ctl->feature_val);
			}
		}
		break;
	case DISP_FEATURE_DDIC_ROUND_CORNER:
		DISP_INFO("DDIC round corner state:%d\n", ctl->feature_val);
		rc = mi_dsi_panel_set_round_corner_locked(panel, ctl->feature_val == FEATURE_ON);
		mi_cfg->feature_val[DISP_FEATURE_DDIC_ROUND_CORNER] = ctl->feature_val;
		break;
	case DISP_FEATURE_HBM_BACKLIGHT:
		DISP_INFO("hbm backlight:%d\n", ctl->feature_val);
		panel->mi_cfg.last_bl_level = ctl->feature_val;
		if (panel->power_mode == SDE_MODE_DPMS_ON) {
			dsi_panel_update_backlight(panel, panel->mi_cfg.last_bl_level);
		}
		break;
	//add cabc
	case DISP_FEATURE_CABC:
		DISP_INFO("feature :CABC ,cabc val = %d  !!\n",ctl->feature_val );
		if (ctl->feature_val == LCD_CABC_OFF) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_CABC_OFF);
			DISP_INFO("m80 cabc off !!\n");
		}
		if (ctl->feature_val == LCD_CABC_UI_ON) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_CABC_UI_ON);
			DISP_INFO("m80 cabc on !!\n");
		}
		if (ctl->feature_val == LCD_CABC_MOVIE_ON) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_CABC_MOVIE_ON);
			DISP_INFO("m80 cabc movie on !!\n");
		}
		if (ctl->feature_val == LCD_CABC_STILL_ON) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_CABC_STILL_ON);
			DISP_INFO("m80 cabc still on !!\n");
		}
		break;
	case DISP_FEATURE_POWERSTATUS:
		DISP_DEBUG("power get:%d, last power mode = %d\n", ctl->feature_val, panel->power_mode);
		break;
	case DISP_FEATURE_SYSTEM_BUILD_VERSION:
		DISP_INFO("system build version:%s\n", ctl->tx_ptr);
		break;
	default:
		DISP_ERROR("invalid feature argument: %d\n", ctl->feature_id);
		break;
	}
exit:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int mi_dsi_panel_get_disp_param(struct dsi_panel *panel,
			struct disp_feature_ctl *ctl)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	int i = 0;

	if (!panel || !ctl) {
		DISP_ERROR("invalid params\n");
		return -EINVAL;
	}

	if (!is_support_disp_feature_id(ctl->feature_id)) {
		DISP_ERROR("unsupported disp feature id\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	mi_cfg = &panel->mi_cfg;
	for (i = DISP_FEATURE_DIMMING; i < DISP_FEATURE_MAX; i++) {
		if (i == ctl->feature_id) {
			ctl->feature_val =  mi_cfg->feature_val[i];
			DISP_INFO("%s: %d\n", get_disp_feature_id_name(ctl->feature_id),
				ctl->feature_val);
		}
	}
	mutex_unlock(&panel->panel_lock);

	return 0;
}

ssize_t mi_dsi_panel_show_disp_param(struct dsi_panel *panel,
			char *buf, size_t size)
{
	struct mi_dsi_panel_cfg *mi_cfg;
	ssize_t count = 0;
	int i = 0;

	if (!panel || !buf || !size) {
		DISP_ERROR("invalid params\n");
		return -EAGAIN;
	}

	count = snprintf(buf, size, "%040s: feature vaule\n", "feature name[feature id]");

	mutex_lock(&panel->panel_lock);
	mi_cfg = &panel->mi_cfg;
	for (i = DISP_FEATURE_DIMMING; i < DISP_FEATURE_MAX; i++) {
		count += snprintf(buf + count, size - count, "%036s[%02d]: %d\n",
				get_disp_feature_id_name(i), i, mi_cfg->feature_val[i]);

	}
	mutex_unlock(&panel->panel_lock);

	return count;
}

int mi_dsi_panel_parse_build_id_read_config(struct dsi_panel *panel)
{
	struct mi_drm_panel_build_id_config *id_config;
	struct dsi_parser_utils *utils = &panel->utils;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	id_config = &panel->mi_cfg.id_config;
	if (!id_config)
		return -EINVAL;

	id_config->build_id = 0;
	rc = utils->read_u32(utils->data, "mi,panel-build-id-read-length",
                                &id_config->id_cmds_rlen);
	if (rc) {
		id_config->id_cmds_rlen = 0;
		return -EINVAL;
	}

	dsi_panel_parse_cmd_sets_sub(&id_config->id_cmd,
			DSI_CMD_SET_MI_PANEL_BUILD_ID, utils);
	if (!(id_config->id_cmd.count)) {
		DISP_ERROR("panel build id read command parsing failed\n");
		return -EINVAL;
	}

	return 0;
}

static bool mi_disp_parse_flatmode_status_len(struct dsi_parser_utils *utils,
	char *prop_key, u32 **target, u32 cmd_cnt)
{
	int tmp;

	if (!utils->find_property(utils->data, prop_key, &tmp))
		return false;

	tmp /= sizeof(u32);
	if (tmp != cmd_cnt) {
		DISP_ERROR("request property(%d) do not match cmd count(%d)\n",
				tmp, cmd_cnt);
		return false;
	}

	*target = kcalloc(tmp, sizeof(u32), GFP_KERNEL);
	if (IS_ERR_OR_NULL(*target)) {
		DISP_ERROR("Error allocating memory for property\n");
		return false;
	}

	if (utils->read_u32_array(utils->data, prop_key, *target, tmp)) {
		DISP_ERROR("cannot get values from dts\n");
		kfree(*target);
		*target = NULL;
		return false;
	}

	return true;
}

int mi_dsi_panel_parse_flatmode_configs(struct dsi_panel *panel)
{
	int rc = 0;
	u32 tmp;
	u32 i, status_len;
	struct property *data;
	struct dsi_parser_utils *utils;
	struct mi_panel_flatmode_config *config;

	config = &panel->mi_cfg.flatmode_check_config;

	if (!panel || !config) {
		DISP_ERROR("invalid param!\n");
		rc = -EAGAIN;
		return rc;
	}

	utils = &(panel->utils);

	dsi_panel_parse_cmd_sets_sub(&config->status_cmd,
				DSI_CMD_SET_MI_FLATMODE_STATUS, utils);
	if (!config->status_cmd.count) {
		DISP_ERROR("flat mode status command parsing failed\n");
		rc = -EINVAL;
		goto error;
	}

	if (!mi_disp_parse_flatmode_status_len(utils,
		"mi,mdss-dsi-panel-flatmode-status-read-length",
			&(config->status_cmds_rlen),
				config->status_cmd.count)) {
		DISP_ERROR("Invalid flat mode status read length\n");
		rc = -EINVAL;
		goto error1;
	}

	status_len = 0;
	for (i = 0; i < config->status_cmd.count; ++i)
		status_len += config->status_cmds_rlen[i];
	if (!status_len) {
		rc = -EINVAL;
		goto error2;
	}

	data = utils->find_property(utils->data,
			"mi,mdss-dsi-panel-flatmode-on-status-value", &tmp);
	tmp /= sizeof(u32);
	if (IS_ERR_OR_NULL(data) || tmp != status_len) {
		DISP_ERROR("error parse panel-status-value\n");
		rc = -EINVAL;
		goto error2;
	}

	config->status_value = kzalloc(sizeof(u32) * status_len, GFP_KERNEL);
	if (!(config->status_value)) {
		rc = -ENOMEM;
		goto error2;
	}
	config->return_buf = kcalloc(status_len, sizeof(unsigned char), GFP_KERNEL);
	if (!config->return_buf) {
		rc = -ENOMEM;
		goto error3;
	}
	config->status_buf = kzalloc(256, GFP_KERNEL);
	if (!config->status_buf) {
		rc = -ENOMEM;
		goto error4;
	}

	rc = utils->read_u32_array(utils->data,
		"mi,mdss-dsi-panel-flatmode-on-status-value",
		config->status_value, status_len);
	if (rc) {
		DISP_ERROR("error reading flat mode status values\n");
		memset(config->status_value, 0, status_len);
		goto error5;
	}

	return rc;

error5:
	kfree(config->status_buf);
error4:
	kfree(config->return_buf);
error3:
	kfree(config->status_value);
error2:
	kfree(config->status_cmds_rlen);
error1:
	dsi_panel_destroy_cmd_packets(&config->status_cmd);
	dsi_panel_dealloc_cmd_packets(&config->status_cmd);
error:
	panel->mi_cfg.flatmode_check_enabled = false;
	return rc;
}

int mi_dsi_panel_match_fps_pen_setting(struct dsi_panel *panel,
				struct dsi_display_mode *adj_mode)
{
	int rc =0;
	int retval = 0;
	struct dsi_display_mode_priv_info *priv_info;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info || !adj_mode) {
		pr_err("invalid params\n");
		return -EAGAIN;
	}

	priv_info = panel->cur_mode->priv_info;

	if (!priv_info->cmd_sets[DSI_CMD_SET_DISP_PEN_120HZ].count) {
		pr_debug("DSI_CMD_SET_DISP_PEN_120HZ not defined, return\n");
		return 0;
	}

	/* match fps(90/50/48/30Hz) pen seeting cmd */
	if (adj_mode->timing.refresh_rate == 120)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_120HZ);
	else if (adj_mode->timing.refresh_rate == 60)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_60HZ);
	else if (adj_mode->timing.refresh_rate == 30)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_30HZ);
	else if (adj_mode->timing.refresh_rate == 90)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_90HZ);
	else if (adj_mode->timing.refresh_rate == 48)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_48HZ);
	else if (adj_mode->timing.refresh_rate == 50)
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DISP_PEN_50HZ);

	if (rc) {
		pr_err("Failed to send DSI_CMD_SET_DISP_PEN_120HZ command\n");
		retval = -EAGAIN;
		goto error;
	}else
		pr_info("%s: refresh_rate[%d]\n", __func__, adj_mode->timing.refresh_rate);

error:
	return retval;
}
ssize_t mi_dsi_panel_lockdown_info_read( unsigned char *plockdowninfo)
{

	int rc = 0;
	int i = 0;

	if (!g_panel->panel_initialized) {
		DISP_ERROR("[%s] Panel not initialized\n", g_panel->type);
		rc = -EINVAL;
	}

	if (!g_panel || !plockdowninfo) {
		pr_err("invalid params\n");
		return -EINVAL;
	}

	for(i = 0; i < 8; i++) {
		pr_err("dgy 0x%x",  g_panel->mi_cfg.lockdown_cfg.lockdown_param[i]);
		plockdowninfo[i] = g_panel->mi_cfg.lockdown_cfg.lockdown_param[i];
	}

	rc = plockdowninfo[0];

	return rc;
}
EXPORT_SYMBOL(mi_dsi_panel_lockdown_info_read);

int mi_dsi_pwr_enable_vregs(struct dsi_regulator_info *regs, bool enable, int index)
{
	int rc = 0;
	struct dsi_vreg *vreg;
	int num_of_v = 0;
	u32 pre_on_ms, post_on_ms;
	u32 pre_off_ms, post_off_ms;
	if (enable) {
		vreg = &regs->vregs[index];
		pre_on_ms = vreg->pre_on_sleep;
		post_on_ms = vreg->post_on_sleep;
		DSI_ERR("M80 Setting optimum mode for %s, enable:%d\n",vreg->vreg_name,enable,index);
		if (vreg->pre_on_sleep)
			usleep_range((pre_on_ms * 1000),
					(pre_on_ms * 1000) + 10);

		rc = regulator_set_load(vreg->vreg,
					vreg->enable_load);
		if (rc < 0) {
			DSI_ERR("Setting optimum mode failed for %s\n",
					vreg->vreg_name);
			goto error;
		}
		num_of_v = regulator_count_voltages(vreg->vreg);
		if (num_of_v > 0) {
			rc = regulator_set_voltage(vreg->vreg,
							vreg->min_voltage,
							vreg->max_voltage);
			if (rc) {
				DSI_ERR("Set voltage(%s) fail, rc=%d\n",
						vreg->vreg_name, rc);
				goto error_disable_opt_mode;
			}
		}

		rc = regulator_enable(vreg->vreg);
		if (rc) {
			DSI_ERR("enable failed for %s, rc=%d\n",
					vreg->vreg_name, rc);
			goto error_disable_voltage;
		}

		if (vreg->post_on_sleep)
			usleep_range((post_on_ms * 1000),
					(post_on_ms * 1000) + 10);
	} else {
		vreg = &regs->vregs[index];
		pre_off_ms = vreg->pre_off_sleep;
		post_off_ms = vreg->post_off_sleep;
		DSI_ERR("M80 Setting optimum mode  for %s, enable:%d\n",vreg->vreg_name,enable,index);
		if (pre_off_ms)
			usleep_range((pre_off_ms * 1000),
					(pre_off_ms * 1000) + 10);

		if (regs->vregs[index].off_min_voltage)
			(void)regulator_set_voltage(regs->vregs[index].vreg,
					regs->vregs[index].off_min_voltage,
					regs->vregs[index].max_voltage);

		(void)regulator_set_load(regs->vregs[index].vreg,
					regs->vregs[index].disable_load);
		(void)regulator_disable(regs->vregs[index].vreg);

		if (post_off_ms)
			usleep_range((post_off_ms * 1000),
					(post_off_ms * 1000) + 10);
	}

	return 0;
error_disable_opt_mode:
	(void)regulator_set_load(regs->vregs[index].vreg,
				 regs->vregs[index].disable_load);

error_disable_voltage:
	if (num_of_v > 0)
		(void)regulator_set_voltage(regs->vregs[index].vreg,
					    0, regs->vregs[index].max_voltage);
error:
	vreg = &regs->vregs[index];
	pre_off_ms = vreg->pre_off_sleep;
	post_off_ms = vreg->post_off_sleep;

	if (pre_off_ms)
		usleep_range((pre_off_ms * 1000),
				(pre_off_ms * 1000) + 10);

	(void)regulator_set_load(regs->vregs[index].vreg,
					regs->vregs[index].disable_load);

	num_of_v = regulator_count_voltages(regs->vregs[index].vreg);
	if (num_of_v > 0)
		(void)regulator_set_voltage(regs->vregs[index].vreg,
					0, regs->vregs[index].max_voltage);

	(void)regulator_disable(regs->vregs[index].vreg);

	if (post_off_ms)
		usleep_range((post_off_ms * 1000),
				(post_off_ms * 1000) + 10);

	return rc;
}

int dsi_panel_parse_wp_reg_read_config(struct dsi_panel *panel)
{
	struct drm_panel_wp_config *wp_config;
	struct dsi_parser_utils *utils = &panel->utils;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	wp_config = &panel->wp_config;
	if (!wp_config)
		return -EINVAL;

	dsi_panel_parse_cmd_sets_sub(&wp_config->wp_cmd,
			DSI_CMD_SET_PANEL_WP_READ, utils);
	if (!wp_config->wp_cmd.count) {
		DISP_ERROR("wp_info read command parsing failed\n");
		return -EINVAL;
	}

	dsi_panel_parse_cmd_sets_sub(&wp_config->pre_tx_cmd,
			DSI_CMD_SET_PANEL_WP_READ_PRE_TX, utils);
	if (!wp_config->pre_tx_cmd.count)
		DISP_INFO("wp_info pre command parsing failed\n");

	rc = utils->read_u32(utils->data, "mi,mdss-dsi-panel-wp-read-length",
				&wp_config->wp_cmds_rlen);
	if (rc || !wp_config->wp_cmds_rlen) {
		wp_config->wp_cmds_rlen = 0;
		return -EINVAL;
	}

	rc = utils->read_u32(utils->data, "mi,mdss-dsi-panel-wp-read-index",
				&wp_config->wp_read_info_index);
	if (rc || !wp_config->wp_read_info_index) {
		wp_config->wp_read_info_index = 0;
	}

	wp_config->return_buf = kcalloc(wp_config->wp_cmds_rlen,
		sizeof(unsigned char), GFP_KERNEL);
	if (!wp_config->return_buf) {
		return -ENOMEM;
	}
	return 0;
}

int dsi_panel_parse_cell_id_read_config(struct dsi_panel *panel)
{
	struct drm_panel_cell_id_config *cell_id_config;
	struct dsi_parser_utils *utils = &panel->utils;
	int rc = 0;

	if (!panel) {
		DISP_ERROR("Invalid Params\n");
		return -EINVAL;
	}

	cell_id_config = &panel->cell_id_config;
	if (!cell_id_config)
		return -EINVAL;

	dsi_panel_parse_cmd_sets_sub(&cell_id_config->cell_id_cmd,
			DSI_CMD_SET_PANEL_CELL_ID_READ, utils);
	if (!cell_id_config->cell_id_cmd.count) {
		DISP_ERROR("cell_id_info read command parsing failed\n");
		return -EINVAL;
	}

	dsi_panel_parse_cmd_sets_sub(&cell_id_config->pre_tx_cmd,
			DSI_CMD_SET_PANEL_CELL_ID_READ_PRE_TX, utils);
	if (!cell_id_config->pre_tx_cmd.count)
		DISP_INFO("cell_id_info pre command parsing failed\n");

	rc = utils->read_u32(utils->data, "mi,mdss-dsi-panel-cell-id-read-length",
				&cell_id_config->cell_id_cmds_rlen);
	if (rc || !cell_id_config->cell_id_cmds_rlen) {
		cell_id_config->cell_id_cmds_rlen = 0;
		return -EINVAL;
	}

	rc = utils->read_u32(utils->data, "mi,mdss-dsi-panel-cell-id-read-index",
				&cell_id_config->cell_id_read_info_index);
	if (rc || !cell_id_config->cell_id_read_info_index) {
		cell_id_config->cell_id_read_info_index = 0;
	}

	cell_id_config->return_buf = kcalloc(cell_id_config->cell_id_cmds_rlen,
		sizeof(unsigned char), GFP_KERNEL);
	if (!cell_id_config->return_buf) {
		return -ENOMEM;
	}
	return 0;
}

void dsi_panel_doubleclick_enable(bool on)
{
	g_panel->mi_cfg.tddi_doubleclick_flag = on;
}
EXPORT_SYMBOL(dsi_panel_doubleclick_enable);
