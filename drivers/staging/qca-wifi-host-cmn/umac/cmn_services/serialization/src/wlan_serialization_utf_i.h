/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Defines the data structures used by the unit test framework for
 * serialization module
 */

#ifndef _WLAN_SERIALIZATION_UTF_I_H_
#define _WLAN_SERIALIZATION_UTF_I_H_

#define WLAN_SER_UTF_MAX_VDEVS 4
#define WLAN_SER_UTF_SCAN_CMD_TESTS 33
#define WLAN_SER_UTF_TIMER_TIMEOUT_MS 5000
#define WLAN_SER_UTF_TEST_CMD_TIMEOUT_MS 30000

/* Sample string: SER_Vxx_Cxx */
#define WLAN_SER_UTF_STR_SIZE 15
#define WLAN_SER_DATA_STR(_s, _v, _i) \
	scnprintf(_s, WLAN_SER_UTF_STR_SIZE, "SER_V%u_C%u", _v, _i)

#define SER_UTF_BLOCK_STR(_x) (_x ? "BLOCK" : "NON-BLOCK")

char *wlan_serialization_status_strings[] = {
	"WLAN_SER_CMD_PENDING",
	"WLAN_SER_CMD_ACTIVE",
	"WLAN_SER_CMD_DENIED_RULES_FAILED",
	"WLAN_SER_CMD_DENIED_LIST_FULL",
	"WLAN_SER_CMD_DENIED_UNSPECIFIED",
};

/**
 * enum wlan_ser_utf_tc_id - Test case id
 * @SER_UTF_TC_DEINIT: Deinit UTF
 * @SER_UTF_TC_INIT: Init UTF
 * @SER_UTF_TC_ADD: Add a custom cmd to queue
 * @SER_UTF_TC_REMOVE: Remove a custom cmd from queue
 * @SER_UTF_TC_CANCEL: Cancel a custom cmd from queue
 * @SER_UTF_TC_SINGLE_SCAN: Add and remove a single scan cmd
 * @SER_UTF_TC_MULTI_SCAN: Add and remove a multiple scan cmd
 * @SER_UTF_TC_MAX_SCAN: Add and remove a maximum scan cmd
 * @SER_UTF_TC_SINGLE_NONSCAN: Add and remove a single nonscan cmd
 * @SER_UTF_TC_MULTI_NONSCAN: Add and remove a multiple nonscan cmd
 * @SER_UTF_TC_MAX_NONSCAN: Add and remove a maximum nonscan cmd
 * @SER_UTF_TC_MULTI_VDEV_NONSCAN: Add nonscan cmd across multiple vdev
 * @SER_UTF_TC_CANCEL_SCAN_AC_SINGLE: Cancel single scan from active queue
 * @SER_UTF_TC_CANCEL_SCAN_AC_PDEV: Cancel pdev scan from active queue
 * @SER_UTF_TC_CANCEL_SCAN_AC_VDEV: Cancel vdev scan from active queue
 * @SER_UTF_TC_CANCEL_SCAN_PD_SINGLE: Cancel single scan from pending queue
 * @SER_UTF_TC_CANCEL_SCAN_PD_PDEV: Cancel pdev scan from pending queue
 * @SER_UTF_TC_CANCEL_SCAN_PD_VDEV: Cancel vdev scan from pending queue
 * @SER_UTF_TC_CANCEL_NONSCAN_AC_SINGLE: Cancel single nonscan from active queue
 * @SER_UTF_TC_CANCEL_NONSCAN_AC_PDEV: Cancel pdev nonscan from active queue
 * @SER_UTF_TC_CANCEL_NONSCAN_AC_VDEV: Cancel vdev nonscan from active queue
 * @SER_UTF_TC_CANCEL_NONSCAN_PD_SINGLE: Cancel nonscan from pending queue
 * @SER_UTF_TC_CANCEL_NONSCAN_PD_PDEV: Cancel pdev nonscan from pending queue
 * @SER_UTF_TC_CANCEL_NONSCAN_PD_VDEV: Cancel vdev nonscan from pending queue
 * @SER_UTF_TC_START_BSS_FILTERING: Test start_bss filtering logic
 * @SER_UTF_TC_STOP_BSS_FILTERING: Test stop_bss filtering logic
 * @SER_UTF_TC_ADD_BLOCKING_NONSCAN_AC_1: Add blocking cmd to active queue
 * @SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_1: Add blocking cmd to pending queue with
 *			blocking cmd in active queue
 * @SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_2: Add blocking cmd to pending queue with
 *			non-blocking cmd in active queue
 * @SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_3: Add blocking cmd to tail of
 *			pending queue with non-blocking cmd in active queue
 * @SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_4: Add blocking cmd to pending between
 *			non-blocking cmd in pending and active queue
 * @SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_1: Add blocking nonscan cmd
 *			across multiple vdev
 * @SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_2: Add blocking nonscan cmd
 *		to a pending queue of vdev with non-blocking across multi vdev
 * @SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_3: Add blocking nonscan cmd
 *		to a active queue of vdev with non-blocking across multiple vdev
 * @SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_4: Add blocking nonscan cmd to the
 *		pending queue of multi vdev with non-blocking across multi vdev
 * @SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_5: Add blocking nonscan cmd to the
 *		pending queue of multi vdev with non-blocking across multi vdev
 *		in pending and active queue
 * @SER_UTF_TC_HIGH_PRIO_NONSCAN_WO_BL: Add high priority nonscan cmd
 *		to the tail of pending queue
 * @SER_UTF_TC_HIGH_PRIO_NONSCAN_W_BL: Add high priority nonscan cmd
 *		to the pending queue between normal priority command
 * @SER_UTF_TC_HIGH_PRIO_BL_NONSCAN: Add high priority blocking
 *		nonscan cmd to the tail of pending queue
 */
enum wlan_ser_utf_tc_id {
	SER_UTF_TC_DEINIT,
	SER_UTF_TC_INIT,
	SER_UTF_TC_ADD,
	SER_UTF_TC_REMOVE,
	SER_UTF_TC_CANCEL,
	SER_UTF_TC_SINGLE_SCAN,
	SER_UTF_TC_MULTI_SCAN,
	SER_UTF_TC_MAX_SCAN,
	SER_UTF_TC_SINGLE_NONSCAN,
	SER_UTF_TC_MULTI_NONSCAN,
	SER_UTF_TC_MAX_NONSCAN,
	SER_UTF_TC_MULTI_VDEV_NONSCAN,
	SER_UTF_TC_CANCEL_SCAN_AC_SINGLE,
	SER_UTF_TC_CANCEL_SCAN_AC_PDEV,
	SER_UTF_TC_CANCEL_SCAN_AC_VDEV,
	SER_UTF_TC_CANCEL_SCAN_PD_SINGLE,
	SER_UTF_TC_CANCEL_SCAN_PD_PDEV,
	SER_UTF_TC_CANCEL_SCAN_PD_VDEV,
	SER_UTF_TC_CANCEL_NONSCAN_AC_SINGLE,
	SER_UTF_TC_CANCEL_NONSCAN_AC_PDEV,
	SER_UTF_TC_CANCEL_NONSCAN_AC_VDEV,
	SER_UTF_TC_CANCEL_NONSCAN_PD_SINGLE,
	SER_UTF_TC_CANCEL_NONSCAN_PD_PDEV,
	SER_UTF_TC_CANCEL_NONSCAN_PD_VDEV,
	SER_UTF_TC_START_BSS_FILTERING,
	SER_UTF_TC_STOP_BSS_FILTERING,
	SER_UTF_TC_ADD_BLOCKING_NONSCAN_AC_1,
	SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_1,
	SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_2,
	SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_3,
	SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_4,
	SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_1,
	SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_2,
	SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_3,
	SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_4,
	SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_5,
	SER_UTF_TC_HIGH_PRIO_NONSCAN_WO_BL,
	SER_UTF_TC_HIGH_PRIO_NONSCAN_W_BL,
	SER_UTF_TC_HIGH_PRIO_BL_NONSCAN,
};

/**
 * struct wlan_ser_utf_data - Test data
 * @id - Test variable
 * @str - String tag associated with the command
 */
struct wlan_ser_utf_data {
	uint8_t id;
	char str[WLAN_SER_UTF_STR_SIZE + 1];
};

/**
 * struct wlan_ser_utf_vdev_info - Information used by the vdevs
 * @vdev: Vdev object manager information
 * @ser_count: Serialization command count for the vdev
 * @utf_scan_timer: Timer for scan commands
 * @utf_nonscan_timer: Timer for non-scan commands
 */
struct wlan_ser_utf_vdev_info {
	struct wlan_objmgr_vdev *vdev;
	uint8_t ser_count;
	qdf_timer_t utf_scan_timer[WLAN_SER_UTF_SCAN_CMD_TESTS];
	qdf_timer_t utf_nonscan_timer[WLAN_SER_UTF_SCAN_CMD_TESTS];
};

/**
 * wlan_ser_utf_cb() - Serialization callback function
 * @cmd: Serialization command info
 * @reason: Serialization reason for callback execution
 *
 * Return: Status of callback execution
 */
QDF_STATUS wlan_ser_utf_cb(struct wlan_serialization_command *cmd,
			   enum wlan_serialization_cb_reason reason);

#endif /* _WLAN_SERIALIZATION_UTF_I_H_ */

