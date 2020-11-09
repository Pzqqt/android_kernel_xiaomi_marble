/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef WLAN_PMO_GPIO_WAKEUP_CFG_H__
#define WLAN_PMO_GPIO_WAKEUP_CFG_H__

#ifdef WLAN_ENABLE_GPIO_WAKEUP
/*
 * <ini>
 * genable_gpio_wakeup - Enable gpio wakeup
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Enable gpio wakeup
 *
 * Supported Feature: gpio wakeup
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_ENABLE_GPIO_WAKEUP CFG_INI_BOOL("genable_gpio_wakeup", \
						0, \
						"Enable gpio wakeup")

/*
 * <ini>
 * ggpio_wakeup_pin - Wakeup gpio pin of host platform
 * @Min: 0
 * @Max: 255
 * @Default: 255
 *
 * Wakeup gpio pin of host platform
 *
 * Supported Feature: gpio wakeup
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_GPIO_WAKEUP_PIN CFG_INI_UINT("ggpio_wakeup_pin", \
					     0, 255, 255, \
					     CFG_VALUE_OR_DEFAULT, \
					     "Wakeup gpio pin of host platform")

/*
 * <ini>
 * ggpio_wakeup_mode - Wakeup gpio mode
 * @Min: 0
 * @Max: 4
 * @Default: 0
 *
 * Wakeup gpio mode
 * 1 indicates rising trigger
 * 2 indicates failing trigger
 * 3 indicates high trigger
 * 4 indicates low trigger
 *
 * Supported Feature: gpio wakeup
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PMO_GPIO_WAKEUP_MODE CFG_INI_UINT("ggpio_wakeup_mode", \
					      0, 4, 0, \
					      CFG_VALUE_OR_DEFAULT, \
					      "Wakeup gpio mode")

#define CFG_GPIO_WAKEUP_ALL \
	CFG(CFG_PMO_ENABLE_GPIO_WAKEUP) \
	CFG(CFG_PMO_GPIO_WAKEUP_PIN) \
	CFG(CFG_PMO_GPIO_WAKEUP_MODE)
#else
#define CFG_GPIO_WAKEUP_ALL
#endif /* WLAN_ENABLE_GPIO_WAKEUP */
#endif /* WLAN_PMO_GPIO_WAKEUP_CFG_H__ */
