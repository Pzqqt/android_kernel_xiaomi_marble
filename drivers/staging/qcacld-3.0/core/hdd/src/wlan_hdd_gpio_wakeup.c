/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "wlan_hdd_main.h"
#include <linux/gpio.h>
#include "wlan_hdd_gpio_wakeup.h"

static int32_t gpio_wakeup_irq_num = -1;

static uint32_t
hdd_gpio_wakeup_mode_pmo_to_linux(enum pmo_gpio_wakeup_mode mode)
{
	uint32_t irq_flag;

	switch (mode) {
	case PMO_GPIO_WAKEUP_MODE_RISING:
		irq_flag = IRQF_TRIGGER_RISING;
		break;
	case PMO_GPIO_WAKEUP_MODE_FALLING:
		irq_flag = IRQF_TRIGGER_FALLING;
		break;
	case PMO_GPIO_WAKEUP_MODE_HIGH:
		irq_flag = IRQF_TRIGGER_HIGH;
		break;
	case PMO_GPIO_WAKEUP_MODE_LOW:
		irq_flag = IRQF_TRIGGER_LOW;
		break;
	default:
		irq_flag = IRQF_TRIGGER_NONE;
		break;
	}

	return irq_flag;
}

static irqreturn_t hdd_gpio_wakeup_isr(int irq, void *dev)
{
	hdd_debug("gpio_wakeup_isr");

	return IRQ_HANDLED;
}

int wlan_hdd_gpio_wakeup_init(struct hdd_context *hdd_ctx)
{
	uint32_t gpio_wakeup_pin;
	enum pmo_gpio_wakeup_mode gpio_wakeup_mode;
	int32_t ret;
	uint32_t irq_flag;

	if (!ucfg_pmo_is_gpio_wakeup_enabled(hdd_ctx->psoc)) {
		hdd_debug("gpio wakeup is not enabled");
		return 0;
	}

	gpio_wakeup_pin = ucfg_pmo_get_gpio_wakeup_pin(hdd_ctx->psoc);

	ret = gpio_request(gpio_wakeup_pin, "gpio_wakeup");
	if (ret) {
		hdd_err("failed to request gpio%d", gpio_wakeup_pin);
		return -EIO;
	}

	ret = gpio_direction_input(gpio_wakeup_pin);
	if (ret) {
		hdd_err("failed to set input direction");
		goto fail_free_gpio;
	}

	gpio_wakeup_irq_num = gpio_to_irq(gpio_wakeup_pin);
	if (gpio_wakeup_irq_num < 0) {
		hdd_err("failed to get irq num");
		goto fail_free_gpio;
	}

	gpio_wakeup_mode = ucfg_pmo_get_gpio_wakeup_mode(hdd_ctx->psoc);
	if (gpio_wakeup_mode == PMO_GPIO_WAKEUP_MODE_INVALID) {
		hdd_err("failed to get invalid wakeup mode");
		goto fail_free_gpio;
	}

	irq_flag = hdd_gpio_wakeup_mode_pmo_to_linux(gpio_wakeup_mode);
	ret = request_irq(gpio_wakeup_irq_num, hdd_gpio_wakeup_isr, irq_flag,
			  "gpio_wakeup_irq", hdd_ctx);
	if (ret) {
		hdd_err("failed to request irq %d", ret);
		goto fail_free_gpio;
	}

	ret = enable_irq_wake(gpio_wakeup_irq_num);
	if (ret) {
		hdd_err("failed to enable irq wake %d", ret);
		goto fail_free_irq;
	}

	hdd_debug("succeed to set gpio wakeup");

	return 0;

fail_free_irq:
	free_irq(gpio_wakeup_irq_num, hdd_ctx);
	gpio_wakeup_irq_num = -1;
fail_free_gpio:
	gpio_free(gpio_wakeup_pin);

	return -EIO;
}

int wlan_hdd_gpio_wakeup_deinit(struct hdd_context *hdd_ctx)
{
	uint32_t gpio_wakeup_pin;

	if (!ucfg_pmo_is_gpio_wakeup_enabled(hdd_ctx->psoc)) {
		hdd_debug("gpio wakeup is not enabled");
		return 0;
	}

	if (gpio_wakeup_irq_num < 0) {
		hdd_debug("gpio wakeup irq is not enabled");
		return 0;
	}

	free_irq(gpio_wakeup_irq_num, hdd_ctx);
	gpio_wakeup_irq_num = -1;

	gpio_wakeup_pin = ucfg_pmo_get_gpio_wakeup_pin(hdd_ctx->psoc);
	gpio_free(gpio_wakeup_pin);

	return 0;
}
