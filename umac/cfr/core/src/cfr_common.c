/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include <cfr_defs_i.h>
#include <qdf_types.h>
#include <osif_private.h>
#include <ol_if_athvar.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_cfr_tgt_api.h>
#include <qal_streamfs.h>
#include <relay.h>
#include <debugfs.h>
#include <target_if.h>

QDF_STATUS
wlan_cfr_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct psoc_cfr *cfr_sc = NULL;

	cfr_sc = (struct psoc_cfr *)qdf_mem_malloc(sizeof(struct psoc_cfr));
	if (NULL == cfr_sc) {
		cfr_err("Failed to allocate cfr_ctx object\n");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(cfr_sc, sizeof(struct psoc_cfr));
	cfr_sc->psoc_obj = psoc;

	wlan_objmgr_psoc_component_obj_attach(psoc, WLAN_UMAC_COMP_CFR,
					      (void *)cfr_sc,
					      QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct psoc_cfr *cfr_sc = NULL;

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);
	if (NULL != cfr_sc) {
		wlan_objmgr_psoc_component_obj_detach(psoc, WLAN_UMAC_COMP_CFR,
						      (void *)cfr_sc);
		qdf_mem_free(cfr_sc);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cfr *pa = NULL;

	if (NULL == pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pa = (struct pdev_cfr *)qdf_mem_malloc(sizeof(struct pdev_cfr));
	if (NULL == pa) {
		cfr_err("Failed to allocate pdev_cfr object\n");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(pa, sizeof(struct pdev_cfr));
	pa->pdev_obj = pdev;

	wlan_objmgr_pdev_component_obj_attach(pdev, WLAN_UMAC_COMP_CFR,
					      (void *)pa, QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cfr *pa = NULL;

	if (NULL == pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (NULL != pa) {
		wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_CFR,
						      (void *)pa);
		qdf_mem_free(pa);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cfr *pe = NULL;

	if (NULL == peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pe = (struct peer_cfr *)qdf_mem_malloc(sizeof(struct peer_cfr));
	if (NULL == pe) {
		cfr_err("Failed to allocate peer_cfr object\n");
		return QDF_STATUS_E_FAILURE;
	}

	pe->peer_obj = peer;

	/* Remaining will be populated when we give CFR capture command */
	wlan_objmgr_peer_component_obj_attach(peer, WLAN_UMAC_COMP_CFR,
					      (void *)pe, QDF_STATUS_SUCCESS);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cfr *pe = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct pdev_cfr *pa = NULL;

	if (NULL == peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_peer_get_vdev(peer);
	if (vdev)
		pdev = wlan_vdev_get_pdev(vdev);

	if (pdev)
		pa = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							   WLAN_UMAC_COMP_CFR);

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);

	if (pa && pe) {
		if (pe->period && pe->request)
			pa->cfr_current_sta_count--;
	}

	if (NULL != pe) {
		wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_CFR,
						      (void *)pe);
		qdf_mem_free(pe);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * create_buf_file_handler() - Create streamfs buffer file
 *  @filename: base name of files to create, NULL for buffering only
 *  @parent: dentry of parent directory, NULL for root directory
 *  @mode: filemode
 *  @rchan_buf: streamfs channel buf
 *
 *  Returns dentry if successful, NULL otherwise.
 */
static struct dentry *create_buf_file_handler(const char *filename,
					      struct dentry *parent,
					      umode_t mode,
					      struct rchan_buf *buf,
					      int *is_global)
{
	struct qal_dentry_t *buf_file;
	*is_global = 1;
	buf_file = qal_streamfs_create_file(filename, mode,
					(struct qal_dentry_t *)parent,
					(struct qal_streamfs_chan_buf *)buf);

	if (!buf_file) {
		cfr_err("Chan buffer creation failed\n");
		return NULL;
	}

	return (struct dentry *)buf_file;
}

/**
 * remove_buf_file_handler() - Remove streamfs buffer file
 *  @dentry:dentry
 */
static int remove_buf_file_handler(struct dentry *dentry)
{
	qal_streamfs_remove_file((struct qal_dentry_t *)dentry);

	return 0;
}

static struct rchan_callbacks cfr_streamfs_cb = {
	.create_buf_file = create_buf_file_handler,
	.remove_buf_file = remove_buf_file_handler,
};

QDF_STATUS cfr_streamfs_init(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa = NULL;
	char folder[32];
	struct net_device *pdev_netdev;
	struct ol_ath_softc_net80211 *scn;
	struct target_pdev_info *tgt_hdl;

	if (pdev == NULL) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	tgt_hdl = wlan_pdev_get_tgt_if_handle(pdev);

	if (!tgt_hdl) {
		cfr_err("target_pdev_info is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	scn = target_pdev_get_feature_ptr(tgt_hdl);
	pdev_netdev = scn->netdev;
	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);

	if (pa == NULL) {
		cfr_err("pdev_cfr is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pa->is_cfr_capable) {
		cfr_err("CFR IS NOT SUPPORTED\n");
		return QDF_STATUS_E_FAILURE;
	}

	snprintf(folder, sizeof(folder), "cfr%s", pdev_netdev->name);

	pa->dir_ptr = qal_streamfs_create_dir((const char *)folder, NULL);

	if (!pa->dir_ptr) {
		cfr_err("Directory create failed");
		return QDF_STATUS_E_FAILURE;
	}

	pa->chan_ptr = qal_streamfs_open("cfr_dump", pa->dir_ptr,
			pa->subbuf_size, pa->num_subbufs,
			(struct qal_streamfs_chan_callbacks *)&cfr_streamfs_cb,
			NULL);

	if (!pa->chan_ptr) {
		cfr_err("Chan create failed");
		qal_streamfs_remove_dir_recursive(pa->dir_ptr);
		pa->dir_ptr = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_remove(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa = NULL;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa) {
		if (pa->chan_ptr) {
			qal_streamfs_close(pa->chan_ptr);
			pa->chan_ptr = NULL;
		}

		if (pa->dir_ptr) {
			qal_streamfs_remove_dir_recursive(pa->dir_ptr);
			pa->dir_ptr = NULL;
		}

	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_write(struct pdev_cfr *pa, const void *write_data,
			      size_t write_len)
{
	if (pa->chan_ptr) {

	/* write to channel buffer */
		qal_streamfs_write(pa->chan_ptr, (const void *)write_data,
				write_len);
	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_flush(struct pdev_cfr *pa)
{
	if (pa->chan_ptr) {

	/* Flush the data write to channel buffer */
		qal_streamfs_flush(pa->chan_ptr);
	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
