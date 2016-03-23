/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include "qdf_types.h"
#include "dummy.h"
#include "hif_debug.h"

/**
 * hif_dummy_bus_prevent_linkdown() - prevent linkdown
 * @hif_ctx: hif context
 * @flag: weather to keep the bus alive or not
 *
 * Dummy function for busses and platforms that do not support
 * link down.  This may need to be replaced with a wakelock.
 */
void hif_dummy_bus_prevent_linkdown(struct hif_softc *scn, bool flag)
{
	HIF_ERROR("wlan: %s pcie power collapse ignored",
			(flag ? "disable" : "enable"));
}

/**
 * hif_reset_soc(): reset soc
 *
 * this function resets soc
 *
 * @hif_ctx: HIF context
 *
 * Return: void
 */
/* Function to reset SoC */
void hif_dummy_reset_soc(struct hif_softc *hif_ctx)
{
}

/**
 * hif_dummy_suspend() - suspend the bus
 * @hif_ctx: hif context
 *
 * dummy for busses that don't need to suspend.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_suspend(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_resume() - hif resume API
 *
 * This function resumes the bus. but snoc doesn't need to resume.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_dummy_bus_resume(struct hif_softc *hif_ctx)
{
	return 0;
}

/**
 * hif_dummy_target_sleep_state_adjust() - api to adjust state of target
 * @scn: hif context
 * @sleep_ok: allow or deny target to go to sleep
 * @wait_for_it: ensure target has change
 */
int hif_dummy_target_sleep_state_adjust(struct hif_softc *scn,
						bool sleep_ok, bool wait_for_it)
{
	return 0;
}

/**
 * hif_dummy_enable_power_management - dummy call
 * hif_ctx: hif context
 * is_packet_log_enabled: true if packet log is enabled
 */
void hif_dummy_enable_power_management(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled)
{}

/**
 * hif_dummy_disable_power_management - dummy call
 * hif_ctx: hif context
 * is_packet_log_enabled: true if packet log is enabled
 */
void hif_dummy_disable_power_management(struct hif_softc *hif_ctx)
{}
