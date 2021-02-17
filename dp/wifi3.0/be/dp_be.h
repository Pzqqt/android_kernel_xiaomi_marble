/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#ifndef __DP_BE_H
#define __DP_BE_H

#include <dp_types.h>
#include <hal_be_tx.h>

/**
 * struct dp_tx_bank_profile - DP wrapper for TCL banks
 * @is_configured: flag indicating if this bank is configured
 * @ref_count: ref count indicating number of users of the bank
 * @bank_config: HAL TX bank configuration
 */
struct dp_tx_bank_profile {
	uint8_t is_configured;
	qdf_atomic_t  ref_count;
	union hal_tx_bank_config bank_config;
};

/**
 * struct dp_soc_be - Extended DP soc for BE targets
 * @soc: dp soc structure
 * @num_bank_profiles: num TX bank profiles
 * @bank_profiles: bank profiles for various TX banks
 */
struct dp_soc_be {
	struct dp_soc soc;
	uint8_t num_bank_profiles;
	qdf_mutex_t tx_bank_lock;
	struct dp_tx_bank_profile *bank_profiles;

};

/**
 * struct dp_pdev_be - Extended DP pdev for BE targets
 * @pdev: dp pdev structure
 */
struct dp_pdev_be {
	struct dp_pdev pdev;
};

/**
 * struct dp_vdev_be - Extended DP vdev for BE targets
 * @vdev: dp vdev structure
 * @bank_id: bank_id to be used for TX
 * @vdev_id_check_en: flag if HW vdev_id check is enabled for vdev
 */
struct dp_vdev_be {
	struct dp_vdev vdev;
	int8_t bank_id;
	uint8_t vdev_id_check_en;
};

/**
 * struct dp_peer_be - Extended DP peer for BE targets
 * @dp_peer: dp peer structure
 */
struct dp_peer_be {
	struct dp_peer peer;
};

/**
 * dp_get_soc_context_size_be() - get context size for target specific DP soc
 *
 * Return: value in bytes for BE specific soc structure
 */
qdf_size_t dp_get_soc_context_size_be(void);

/**
 * dp_initialize_arch_ops_be() - initialize BE specific arch ops
 * @arch_ops: arch ops pointer
 *
 * Return: none
 */
void dp_initialize_arch_ops_be(struct dp_arch_ops *arch_ops);

/**
 * dp_get_context_size_be() - get BE specific size for peer/vdev/pdev/soc
 * @arch_ops: arch ops pointer
 *
 * Return: size in bytes for the context_type
 */
qdf_size_t dp_get_context_size_be(enum dp_context_type context_type);

/**
 * dp_get_be_soc_from_dp_soc() - get dp_soc_be from dp_soc
 * @soc: dp_soc pointer
 *
 * Return: dp_soc_be pointer
 */
static inline struct dp_soc_be *dp_get_be_soc_from_dp_soc(struct dp_soc *soc)
{
	return (struct dp_soc_be *)soc;
}

/**
 * dp_get_be_pdev_from_dp_pdev() - get dp_pdev_be from dp_pdev
 * @pdev: dp_pdev pointer
 *
 * Return: dp_pdev_be pointer
 */
static inline
struct dp_pdev_be *dp_get_be_pdev_from_dp_pdev(struct dp_pdev *pdev)
{
	return (struct dp_pdev_be *)pdev;
}

/**
 * dp_get_be_vdev_from_dp_vdev() - get dp_vdev_be from dp_vdev
 * @vdev: dp_vdev pointer
 *
 * Return: dp_vdev_be pointer
 */
static inline
struct dp_vdev_be *dp_get_be_vdev_from_dp_vdev(struct dp_vdev *vdev)
{
	return (struct dp_vdev_be *)vdev;
}

/**
 * dp_get_be_peer_from_dp_peer() - get dp_peer_be from dp_peer
 * @peer: dp_peer pointer
 *
 * Return: dp_peer_be pointer
 */
static inline
struct dp_peer_be *dp_get_be_peer_from_dp_peer(struct dp_peer *peer)
{
	return (struct dp_peer_be *)peer;
}
#endif
