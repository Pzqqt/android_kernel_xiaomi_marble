#ifndef SEND_DATA_TO_XLOG
#define SEND_DATA_TO_XLOG

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/ratelimit.h>
#include <asm/current.h>
#include <asm/div64.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/poll.h>

extern ssize_t xlogchar_kwrite(const char __user *buf, size_t count);

void send_DC_data_to_xlog(int dc_current_cnt, char *reason);
int xlog_send_int(int dc_current_cnt, char *reason);
int xlog_format_msg_int (char *msg, int dc_current_cnt, char *reason);

#endif

