// SPDX-License-Identifier: GPL-2.0
/*
 * pwrkey-sync.c
 *
 * Copyright (c) 2024 Juhyung Park
 *
 * This software driver listens to power button events and performs sync when
 * it's held down for long.
 *
 * This is to minimize data corruption when the system is not responding and
 * the user wants to initiate a forced reboot.
 *
 * Additionally, it can forcefully trigger a kernel panic to help debugging a
 * kernel issue further by storing its log to something like pstore before the
 * hardware resets itself.
 */

#define pr_fmt(fmt) "pwrkey-sync: " fmt

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/sysrq.h>
#include <linux/input.h>

// Temporarily enable all sysrq
static void call_sysrq(int key)
{
	int mask;

	pr_info("%s: %c\n", __func__, key);
	mask = sysrq_mask();
	sysrq_toggle_support(1);
	handle_sysrq(key);
	sysrq_toggle_support(mask);
}

static void pwrkey_sync_work(struct work_struct *unused)
{
	pr_info("%s triggered\n", __func__);

	/* Perform an emergency sync and a full sync */
	call_sysrq('s');
	msleep(100);
	ksys_sync_helper();
}

static void pwrkey_sync_panic_work(struct work_struct *unused)
{
	pr_info("%s triggered\n", __func__);

	pwrkey_sync_work(NULL);	// Perform a sync
	call_sysrq('u');	// Will attempt to remount all mounted filesystems read-only
	pwrkey_sync_work(NULL);	// Perform a sync again

	// Print useful info
	call_sysrq('d');	// Shows all locks that are held
	call_sysrq('l');	// Shows a stack backtrace for all active CPUs
	call_sysrq('w');	// Will dump a list of current tasks and their information to your console

	// Panic!
	panic("Power key is held down for " __stringify(CONFIG_INPUT_PWRKEY_SYNC_PANIC_DELAY) "ms");
}

static DECLARE_DELAYED_WORK(pwrkey_sync_worker, pwrkey_sync_work);
static DECLARE_DELAYED_WORK(pwrkey_sync_panic_worker, pwrkey_sync_panic_work);

static void pwrkey_sync_input_event(struct input_handle *handle,
				    unsigned int type, unsigned int code, int value)
{
	if (type != EV_KEY)
		return;

	switch (value) {
	case 0:
		/* UP */
		cancel_delayed_work(&pwrkey_sync_worker);
#ifdef CONFIG_INPUT_PWRKEY_SYNC_PANIC
		cancel_delayed_work(&pwrkey_sync_panic_worker);
#endif
		break;
	case 1:
		/* DOWN */
		queue_delayed_work(system_highpri_wq, &pwrkey_sync_worker,
				   msecs_to_jiffies(CONFIG_INPUT_PWRKEY_SYNC_DELAY));
#ifdef CONFIG_INPUT_PWRKEY_SYNC_PANIC
		queue_delayed_work(system_highpri_wq, &pwrkey_sync_panic_worker,
				   msecs_to_jiffies(CONFIG_INPUT_PWRKEY_SYNC_PANIC_DELAY));
#endif
		break;
	}
}

static int pwrkey_sync_input_connect(struct input_handler *handler,
				     struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

#ifdef CONFIG_INPUT_PWRKEY_SYNC_DEVNAME
	if (strcmp(dev->name, CONFIG_INPUT_PWRKEY_SYNC_DEVNAME_STR)) {
		return -ENODEV;
	}
#endif

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "pwrkey_sync";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;
	pr_info("%s found and connected!\n", dev->name);
	return 0;
 err1:
	input_unregister_handle(handle);
 err2:
	kfree(handle);
	return error;
}

static void pwrkey_sync_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id pwrkey_sync_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
		.evbit = { BIT_MASK(EV_KEY) },
		.keybit = { [BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER) },
	},
	{},
};

static struct input_handler pwrkey_sync_input_handler = {
	.event = pwrkey_sync_input_event,
	.connect = pwrkey_sync_input_connect,
	.disconnect = pwrkey_sync_input_disconnect,
	.name = "pwrkey_sync_handler",
	.id_table = pwrkey_sync_ids,
};

static int __init pwrkey_sync_init(void)
{
	int ret;

	ret = input_register_handler(&pwrkey_sync_input_handler);
	if (ret) {
		pr_err("Failed to register input listener: %d\n", ret);
		return ret;
	}

	return 0;
}

static void __exit pwrkey_sync_exit(void)
{
	input_unregister_handler(&pwrkey_sync_input_handler);
}

module_init(pwrkey_sync_init);
module_exit(pwrkey_sync_exit);

MODULE_DESCRIPTION("Trigger a sync on power button events");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juhyung Park");
