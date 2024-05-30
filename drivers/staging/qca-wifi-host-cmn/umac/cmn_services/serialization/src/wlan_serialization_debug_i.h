/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_debug_i.h
 * This file defines the prototypes for the debug functions
 * for the serialization component.
 */

#ifndef __WLAN_SERIALIZATION_DEBUG_I_H
#define __WLAN_SERIALIZATION_DEBUG_I_H

#ifdef WLAN_SER_DEBUG

#define SER_MAX_HISTORY_CMDS 50

#define WLAN_SER_LINE "--------------------"\
		      "--------------------"

#define WLAN_SER_HISTORY_HEADER "CMD_TYPE|CMD_ID|VDEV_ID|"\
				"BLOCKING|PRIORITY|ACTION|"\
				"  QUEUE|           REASON|"

enum ser_queue_type {
	SER_PDEV_QUEUE_TYPE,
	SER_VDEV_QUEUE_TYPE,
};

struct ser_data {
		/*
		 * Serialization Actions that modifies the serialization queues
		 * 0: SER_REQUEST
		 * 1: SER_REMOVE
		 * 2: SER_CANCEL
		 * 3: SER_TIMEOUT
		 * 4: SER_ACTIVATION_FAILED
		 * 5: SER_PENDING_TO_ACTIVE
		 */
		uint32_t cmd_type:6,	/* max 2^6 = 64 types of commands */
		cmd_id:16,		/* max cmd_id = 2^16  */
		is_blocking:1,
		is_high_priority:1,
		add_remove:1,
		active_pending:1,
		data_updated:1,
		ser_reason:5;

		uint16_t vdev_id;
};

struct ser_history {
	struct ser_data data[SER_MAX_HISTORY_CMDS];
	uint16_t index;
};
#endif /* WLAN_SER_DEBUG */
#endif

