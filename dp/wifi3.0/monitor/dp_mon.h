/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
struct dp_mon_ops {
	QDF_STATUS (*mon_soc_cfg_init)(struct dp_soc *soc);
	QDF_STATUS (*mon_pdev_attach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_detach)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_init)(struct dp_pdev *pdev);
	QDF_STATUS (*mon_pdev_deinit)(struct dp_pdev *pdev);
};

struct dp_mon_soc {
	/* Holds all monitor related fields extracted from dp_soc */
	/* Holds pointer to monitor ops */

	struct dp_mon_ops *mon_ops;
};

struct  dp_mon_pdev {
};

struct  dp_mon_vdev {
};

struct dp_mon_peer {
};

static inline QDF_STATUS monitor_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_attach) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_attach(pdev);
}

static inline QDF_STATUS monitor_pdev_detach(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_detach) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_detach(pdev);
}

static inline QDF_STATUS monitor_pdev_init(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_init) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_init(pdev);
}

static inline QDF_STATUS monitor_pdev_deinit(struct dp_pdev *pdev)
{
	struct dp_mon_ops *monitor_ops;
	struct dp_mon_soc *mon_soc = pdev->soc->monitor_soc;

	/*
	 * mon_soc uninitialized modular support enabled
	 * monitor related attach/detach/init/deinit
	 * will be done while monitor insmod
	 */
	if (!mon_soc)
		return QDF_STATUS_SUCCESS;

	monitor_ops = mon_soc->mon_ops;
	if (!monitor_ops || !monitor_ops->mon_pdev_deinit) {
		qdf_err("callback not registered");
		return QDF_STATUS_E_FAILURE;
	}

	return monitor_ops->mon_pdev_deinit(pdev);
}
