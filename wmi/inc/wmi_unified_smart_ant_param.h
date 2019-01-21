/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_SMART_ANT_PARAM_H_
#define _WMI_UNIFIED_SMART_ANT_PARAM_H_

/**
 * struct ant_switch_tbl_params - Antenna switch table params
 * @ant_ctrl_common1: ANtenna control common param 1
 * @ant_ctrl_common2: Antenna control commn param 2
 */
struct ant_switch_tbl_params {
	uint32_t ant_ctrl_common1;
	uint32_t ant_ctrl_common2;
	uint32_t pdev_id;
	uint32_t antCtrlChain;
};

/**
 * struct smart_ant_enable_params - Smart antenna params
 * @enable: Enable/Disable
 * @mode: SA mode
 * @rx_antenna: RX antenna config
 * @gpio_pin : GPIO pin config
 * @gpio_func : GPIO function config
 */
struct smart_ant_enable_params {
	uint32_t enable;
	uint32_t mode;
	uint32_t rx_antenna;
	uint32_t gpio_pin[WMI_HAL_MAX_SANTENNA];
	uint32_t gpio_func[WMI_HAL_MAX_SANTENNA];
	uint32_t pdev_id;
};

/**
 * struct smart_ant_rx_ant_params - RX antenna params
 * @antenna: RX antenna
 */
struct smart_ant_rx_ant_params {
	uint32_t antenna;
	uint32_t pdev_id;
};

/**
 * struct smart_ant_tx_ant_params - TX antenna param
 * @antenna_array: Antenna arry
 * @vdev_id: VDEV id
 */
struct smart_ant_tx_ant_params {
	uint32_t *antenna_array;
	uint8_t vdev_id;
};

/**
 * struct smart_ant_training_info_params - SA training params
 * @vdev_id: VDEV id
 * @rate_array: Rates array
 * @antenna_array: Antenna array
 * @numpkts: num packets for training
 */
struct smart_ant_training_info_params {
	uint8_t vdev_id;
	uint32_t *rate_array;
	uint32_t *antenna_array;
	uint32_t numpkts;
};

/**
 * struct smart_ant_node_config_params - SA node config params
 * @vdev_id: VDEV id
 * @cmd_id: Command id
 * @args_count: Arguments count
 */
struct smart_ant_node_config_params {
	uint8_t vdev_id;
	uint32_t cmd_id;
	uint16_t args_count;
	uint32_t *args_arr;
};

#endif /* _WMI_UNIFIED_SMART_ANT_PARAM_H_ */
