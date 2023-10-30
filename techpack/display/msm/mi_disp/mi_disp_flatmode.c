#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "mi_disp_print.h"
#include <linux/soc/qcom/smem.h>
#include "dsi_parser.h"
#include "dsi_panel.h"
#include "mi_dsi_panel.h"
#include "mi_dsi_display.h"
#include "mi_disp_flatmode.h"

static void mi_dsi_panel_flatmode_freemem(struct mi_panel_flatmode_config *config)
{
	if (config) {
		dsi_panel_destroy_cmd_packets(&config->status_cmd);
		dsi_panel_dealloc_cmd_packets(&config->status_cmd);

		kfree(config->status_buf);
		kfree(config->return_buf);
		kfree(config->status_value);
		kfree(config->status_cmds_rlen);

	}
}

static bool mi_flatmode_validate_status(struct dsi_display *display,
		struct mi_panel_flatmode_config *config)
{
	int i, count = 0, start = 0, len = 0, *lenp;
	struct dsi_cmd_desc *cmds;
	u16 flags = 0;

	lenp = config->status_cmds_rlen;
	count = config->status_cmd.count;
	cmds = config->status_cmd.cmds;
	memset(config->return_buf, 0x0, sizeof(*config->return_buf));

	if (config->status_cmd.state == DSI_CMD_SET_STATE_LP)
		flags = MIPI_DSI_MSG_USE_LPM;

	for (i = 0; i < count; ++i) {
		memset(config->status_buf, 0x0, 256);
		cmds[i].msg.flags |= flags;
		if(mi_dsi_display_cmd_read(display, cmds[i], config->status_buf, lenp[i]) <= 0) {
			DISP_ERROR("mi_dsi_display_cmd_read fail\n");
			goto error;
		}

		memcpy(config->return_buf+start, config->status_buf, lenp[i]);
		start += lenp[i];
	}

	for (i = 0; i < count; i++)
		len += lenp[i];

	for (i = 0; i < len; ++i) {
		DISP_INFO(" flat mode return_buff[%d] = 0x%x, status_value[%d] = 0x%x",
				i, config->return_buf[i], i, config->status_value[i]);

		if (config->return_buf[i] != config->status_value[i])
			goto error;
	}

	return true;

error:
	return false;
}

int mi_dsi_panel_flatmode_validate_status(struct dsi_display *display, bool *status)
{
	int rc = 0;
	struct mi_panel_flatmode_config flatmode_config;
	bool flatmode_check_enabled;
	struct mi_dsi_panel_cfg *mi_cfg;
	struct dsi_panel *panel;

	panel = display->panel;
	mi_cfg = &panel->mi_cfg;

	if (!display || !panel || !mi_cfg || !status) {
		DISP_ERROR("Invalid ptr\n");
		return -EINVAL;
	}

	flatmode_check_enabled = mi_cfg->flatmode_check_enabled;
	if (!flatmode_check_enabled) {
		DISP_INFO("flatmode check not enabled\n");
		return rc;
	}

	rc = mi_dsi_panel_parse_flatmode_configs(panel);
	if (rc) {
		DSI_ERR("[%s] failed to get flatmode check read infos, rc=%d\n",
				panel->name, rc);
		return rc;
	}

	memset(&flatmode_config, 0, sizeof(flatmode_config));
	flatmode_config = mi_cfg->flatmode_check_config;

	*status = mi_flatmode_validate_status(display, &flatmode_config);

	mi_dsi_panel_flatmode_freemem(&flatmode_config);

	return rc;
}
