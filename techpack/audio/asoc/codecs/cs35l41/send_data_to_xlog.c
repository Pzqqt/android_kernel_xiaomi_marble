#include "send_data_to_xlog.h"
#include <linux/debugfs.h>

char msg_format[] = "{\"name\":\"DC detection\",\"audio_event\":{\"count\":\"%d\",\"reason\":\"%s\"},\"dgt\":\"null\",\"audio_ext\":\"null\" }";

#define MAX_LEN 512

void send_DC_data_to_xlog(int dc_current_cnt, char *reason)
{
	int ret = -1;
	pr_info("%s: dc_current_cnt: %d, reason: %s", __func__, dc_current_cnt, reason);
	ret = xlog_send_int(dc_current_cnt, reason);
	if (ret < 0) {
		pr_info("%s: failed", __func__);
	} else {
		pr_info("%s: success", __func__);
	}
	j;lkj;:w

}

int xlog_send_int(int dc_current_cnt, char *reason)
{
	int ret = 0;
	char msg[512];
	pr_info("%s: cs35l41->dc_current_cnt: %d, reason: %s", __func__, dc_current_cnt, reason);
	ret = xlog_format_msg_int(msg, dc_current_cnt, reason);
	if (ret < 0) {
		return ret;
	}
	xlogchar_kwrite(msg, sizeof(msg));
	pr_info("%s: send msg: %s", __func__, msg);
	return ret;
}

int xlog_format_msg_int (char *msg, int dc_current_cnt, char *reason)
{
	if (msg == NULL) {
		pr_info("%s: the msg is NULL", __func__);
		return -EINVAL;
	}
	pr_info("%s start", __func__);
	snprintf(msg, MAX_LEN, msg_format, dc_current_cnt, reason);
	pr_info("%s end", __func__);
	return 0;
}
