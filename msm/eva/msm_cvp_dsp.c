// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <soc/qcom/secure_buffer.h>
#include "msm_cvp_core.h"
#include "msm_cvp.h"
#include "cvp_hfi.h"
#include "cvp_dump.h"

struct cvp_dsp_apps gfa_cv;
static int hlosVM[HLOS_VM_NUM] = {VMID_HLOS};
static int dspVM[DSP_VM_NUM] = {VMID_HLOS, VMID_CDSP_Q6};
static int dspVMperm[DSP_VM_NUM] = { PERM_READ | PERM_WRITE | PERM_EXEC,
				PERM_READ | PERM_WRITE | PERM_EXEC };
static int hlosVMperm[HLOS_VM_NUM] = { PERM_READ | PERM_WRITE | PERM_EXEC };

static int cvp_reinit_dsp(void);

static int __fastrpc_driver_register(struct fastrpc_driver *driver)
{
#ifdef CVP_FASTRPC_ENABLED
	return fastrpc_driver_register(driver);
#else
	return -ENODEV;
#endif
}

static void __fastrpc_driver_unregister(struct fastrpc_driver *driver)
{
#ifdef CVP_FASTRPC_ENABLED
	return fastrpc_driver_unregister(driver);
#endif
}

static int __fastrpc_driver_invoke(struct fastrpc_device *dev,
				enum fastrpc_driver_invoke_nums invoke_num,
				unsigned long invoke_param)
{
#ifdef CVP_FASTRPC_ENABLED
	return fastrpc_driver_invoke(dev, invoke_num, invoke_param);
#else
	return -ENODEV;
#endif
}

static int cvp_dsp_send_cmd(struct cvp_dsp_cmd_msg *cmd, uint32_t len)
{
	int rc = 0;
	struct cvp_dsp_apps *me = &gfa_cv;

	dprintk(CVP_DSP, "%s: cmd = %d\n", __func__, cmd->type);

	if (IS_ERR_OR_NULL(me->chan)) {
		dprintk(CVP_ERR, "%s: DSP GLink is not ready\n", __func__);
		rc = -EINVAL;
		goto exit;
	}
	rc = rpmsg_send(me->chan->ept, cmd, len);
	if (rc) {
		dprintk(CVP_ERR, "%s: DSP rpmsg_send failed rc=%d\n",
			__func__, rc);
		goto exit;
	}

exit:
	return rc;
}

static int cvp_dsp_send_cmd_sync(struct cvp_dsp_cmd_msg *cmd,
		uint32_t len, struct cvp_dsp_rsp_msg *rsp)
{
	int rc = 0;
	struct cvp_dsp_apps *me = &gfa_cv;

	dprintk(CVP_DSP, "%s: cmd = %d\n", __func__, cmd->type);

	me->pending_dsp2cpu_rsp.type = cmd->type;
	rc = cvp_dsp_send_cmd(cmd, len);
	if (rc) {
		dprintk(CVP_ERR, "%s: cvp_dsp_send_cmd failed rc=%d\n",
			__func__, rc);
		goto exit;
	}

	if (!wait_for_completion_timeout(&me->completions[cmd->type],
			msecs_to_jiffies(CVP_DSP_RESPONSE_TIMEOUT))) {
		dprintk(CVP_ERR, "%s cmd %d timeout\n", __func__, cmd->type);
		rc = -ETIMEDOUT;
		goto exit;
	}

exit:
	rsp->ret = me->pending_dsp2cpu_rsp.ret;
	rsp->dsp_state = me->pending_dsp2cpu_rsp.dsp_state;
	me->pending_dsp2cpu_rsp.type = CVP_INVALID_RPMSG_TYPE;
	return rc;
}

static int cvp_dsp_send_cmd_hfi_queue(phys_addr_t *phys_addr,
				uint32_t size_in_bytes,
				struct cvp_dsp_rsp_msg *rsp)
{
	int rc = 0;
	struct cvp_dsp_cmd_msg cmd;

	cmd.type = CPU2DSP_SEND_HFI_QUEUE;
	cmd.msg_ptr = (uint64_t)phys_addr;
	cmd.msg_ptr_len = size_in_bytes;
	cmd.ddr_type = cvp_of_fdt_get_ddrtype();
	if (cmd.ddr_type < 0) {
		dprintk(CVP_WARN,
			"%s: Incorrect DDR type value %d, use default %d\n",
			__func__, cmd.ddr_type, DDR_TYPE_LPDDR5);
		/*return -EINVAL;*/
		cmd.ddr_type =  DDR_TYPE_LPDDR5;
	}

	dprintk(CVP_DSP,
		"%s: address of buffer, PA=0x%pK  size_buff=%d ddr_type=%d\n",
		__func__, phys_addr, size_in_bytes, cmd.ddr_type);

	rc = cvp_dsp_send_cmd_sync(&cmd, sizeof(struct cvp_dsp_cmd_msg), rsp);
	if (rc) {
		dprintk(CVP_ERR,
			"%s: cvp_dsp_send_cmd failed rc = %d\n",
			__func__, rc);
		goto exit;
	}
exit:
	return rc;
}

static int cvp_hyp_assign_to_dsp(uint64_t addr, uint32_t size)
{
	int rc = 0;
	struct cvp_dsp_apps *me = &gfa_cv;

	if (!me->hyp_assigned) {
		rc = hyp_assign_phys(addr, size, hlosVM, HLOS_VM_NUM, dspVM,
			dspVMperm, DSP_VM_NUM);
		if (rc) {
			dprintk(CVP_ERR, "%s failed. rc=%d\n", __func__, rc);
			return rc;
		}
		me->addr = addr;
		me->size = size;
		me->hyp_assigned = true;
	}

	return rc;
}

static int cvp_hyp_assign_from_dsp(void)
{
	int rc = 0;
	struct cvp_dsp_apps *me = &gfa_cv;

	if (me->hyp_assigned) {
		rc = hyp_assign_phys(me->addr, me->size, dspVM, DSP_VM_NUM,
				hlosVM, hlosVMperm, HLOS_VM_NUM);
		if (rc) {
			dprintk(CVP_ERR, "%s failed. rc=%d\n", __func__, rc);
			return rc;
		}
		me->addr = 0;
		me->size = 0;
		me->hyp_assigned = false;
	}

	return rc;
}

static int cvp_dsp_rpmsg_probe(struct rpmsg_device *rpdev)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	const char *edge_name = NULL;
	int ret = 0;

	ret = of_property_read_string(rpdev->dev.parent->of_node,
			"label", &edge_name);
	if (ret) {
		dprintk(CVP_ERR, "glink edge 'label' not found in node\n");
		return ret;
	}

	if (strcmp(edge_name, "cdsp")) {
		dprintk(CVP_ERR,
			"%s: Failed to probe rpmsg device.Node name:%s\n",
			__func__, edge_name);
		return -EINVAL;
	}

	mutex_lock(&me->tx_lock);
	me->chan = rpdev;
	me->state = DSP_PROBED;
	mutex_unlock(&me->tx_lock);
	complete(&me->completions[CPU2DSP_MAX_CMD]);

	return ret;
}

static int eva_fastrpc_dev_unmap_dma(
		struct fastrpc_device *frpc_device,
		struct cvp_internal_buf *buf);

static int delete_dsp_session(struct msm_cvp_inst *inst,
		struct cvp_dsp_fastrpc_driver_entry *frpc_node)
{
	struct msm_cvp_list *buf_list = NULL;
	struct list_head *ptr_dsp_buf = NULL, *next_dsp_buf = NULL;
	struct cvp_internal_buf *buf = NULL;
	struct task_struct *task = NULL;
	struct cvp_hfi_device *hdev;
	int rc;

	if (!inst)
		return -EINVAL;

	buf_list = &inst->cvpdspbufs;

	mutex_lock(&buf_list->lock);
	ptr_dsp_buf = &buf_list->list;
	list_for_each_safe(ptr_dsp_buf, next_dsp_buf, &buf_list->list) {
		buf = list_entry(ptr_dsp_buf, struct cvp_internal_buf, list);
		if (buf) {
			dprintk(CVP_DSP, "fd in list 0x%x\n", buf->fd);

			if (!buf->smem) {
				dprintk(CVP_DSP, "Empyt smem\n");
				continue;
			}

			dprintk(CVP_DSP, "%s find device addr 0x%x\n",
				__func__, buf->smem->device_addr);

			rc = eva_fastrpc_dev_unmap_dma(
					frpc_node->cvp_fastrpc_device,
					buf);
			if (rc)
				dprintk(CVP_WARN,
					"%s Failed to unmap buffer 0x%x\n",
					__func__, rc);

			rc = cvp_release_dsp_buffers(inst, buf);
			if (rc)
				dprintk(CVP_ERR,
					"%s Failed to free buffer 0x%x\n",
					__func__, rc);

			list_del(&buf->list);

			kmem_cache_free(cvp_driver->buf_cache, buf);
		}
	}

	mutex_unlock(&buf_list->lock);

	rc = msm_cvp_session_delete(inst);
	if (rc)
		dprintk(CVP_ERR, "Warning: send Delete Session failed\n");

	task = inst->task;

	spin_lock(&inst->core->resources.pm_qos.lock);
	if (inst->core->resources.pm_qos.off_vote_cnt > 0)
		inst->core->resources.pm_qos.off_vote_cnt--;
	else
		dprintk(CVP_WARN, "%s Unexpected pm_qos off vote %d\n",
			__func__,
			inst->core->resources.pm_qos.off_vote_cnt);
	spin_unlock(&inst->core->resources.pm_qos.lock);

	hdev = inst->core->device;
	call_hfi_op(hdev, pm_qos_update, hdev->hfi_device_data);

	rc = msm_cvp_close(inst);
	if (rc)
		dprintk(CVP_ERR, "Warning: Failed to close cvp instance\n");

	if (task)
		put_task_struct(task);

	dprintk(CVP_DSP, "%s DSP2CPU_DETELE_SESSION Done\n", __func__);
	return rc;
}

static int eva_fastrpc_driver_get_name(
		struct cvp_dsp_fastrpc_driver_entry *frpc_node)
{
    int i = 0;
    struct cvp_dsp_apps *me = &gfa_cv;
    for (i = 0; i < MAX_FASTRPC_DRIVER_NUM; i++) {
        if (me->cvp_fastrpc_name[i].status == DRIVER_NAME_AVAILABLE) {
            frpc_node->driver_name_idx = i;
            frpc_node->cvp_fastrpc_driver.driver.name =
			me->cvp_fastrpc_name[i].name;
            me->cvp_fastrpc_name[i].status = DRIVER_NAME_USED;
            dprintk(CVP_DSP, "%s -> handle 0x%x get name %s\n",
			__func__, frpc_node->cvp_fastrpc_driver.handle,
                frpc_node->cvp_fastrpc_driver.driver.name);
            return 0;
        }
    }

    return -1;
}

static void eva_fastrpc_driver_release_name(
		struct cvp_dsp_fastrpc_driver_entry *frpc_node)
{
    struct cvp_dsp_apps *me = &gfa_cv;
    me->cvp_fastrpc_name[frpc_node->driver_name_idx].status =
		DRIVER_NAME_AVAILABLE;
}

static void cvp_dsp_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct cvp_dsp_apps *me = &gfa_cv;

	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct list_head *ptr = NULL, *next = NULL;
	struct list_head *s = NULL, *next_s = NULL;
	struct msm_cvp_inst *inst = NULL;

	dprintk(CVP_WARN, "%s: CDSP SSR triggered\n", __func__);

	mutex_lock(&me->tx_lock);
	cvp_hyp_assign_from_dsp();

	me->chan = NULL;
	me->state = DSP_UNINIT;
	mutex_unlock(&me->tx_lock);

	ptr = &me->fastrpc_driver_list.list;
	mutex_lock(&me->fastrpc_driver_list.lock);
	list_for_each_safe(ptr, next, &me->fastrpc_driver_list.list) {
		frpc_node = list_entry(ptr,
				struct cvp_dsp_fastrpc_driver_entry, list);

		if (frpc_node) {
			s = &frpc_node->dsp_sessions.list;
			list_for_each_safe(s, next_s,
					&frpc_node->dsp_sessions.list) {
				inst = list_entry(s, struct msm_cvp_inst,
						dsp_list);
				delete_dsp_session(inst, frpc_node);
			}

			dprintk(CVP_DSP, "%s DEINIT_MSM_CVP_LIST 0x%x\n",
					__func__, frpc_node->dsp_sessions);
			DEINIT_MSM_CVP_LIST(&frpc_node->dsp_sessions);
			dprintk(CVP_DSP, "%s list_del fastrpc node 0x%x\n",
					__func__, frpc_node);
			list_del(&frpc_node->list);
			__fastrpc_driver_unregister(
				&frpc_node->cvp_fastrpc_driver);
			dprintk(CVP_DSP,
				"%s Unregistered fastrpc handle 0x%x\n",
				__func__, frpc_node->handle);
			mutex_lock(&me->driver_name_lock);
			eva_fastrpc_driver_release_name(frpc_node);
			mutex_unlock(&me->driver_name_lock);
			kfree(frpc_node);
			frpc_node = NULL;
		}
	}
	mutex_unlock(&me->fastrpc_driver_list.lock);
	dprintk(CVP_WARN, "%s: CDSP SSR handled\n", __func__);
}

static int cvp_dsp_rpmsg_callback(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 addr)
{
	struct cvp_dsp_rsp_msg *rsp = (struct cvp_dsp_rsp_msg *)data;
	struct cvp_dsp_apps *me = &gfa_cv;

	dprintk(CVP_DSP, "%s: type = 0x%x ret = 0x%x len = 0x%x\n",
		__func__, rsp->type, rsp->ret, len);

	if (rsp->type < CPU2DSP_MAX_CMD && len == sizeof(*rsp)) {
		if (me->pending_dsp2cpu_rsp.type == rsp->type) {
			memcpy(&me->pending_dsp2cpu_rsp, rsp,
				sizeof(struct cvp_dsp_rsp_msg));
			complete(&me->completions[rsp->type]);
		} else {
			dprintk(CVP_ERR, "%s: CPU2DSP resp %d, pending %d\n",
					__func__, rsp->type,
					me->pending_dsp2cpu_rsp.type);
			goto exit;
		}
	} else if (rsp->type < CVP_DSP_MAX_CMD &&
			len == sizeof(struct cvp_dsp2cpu_cmd_msg)) {
		if (me->pending_dsp2cpu_cmd.type != CVP_INVALID_RPMSG_TYPE) {
			dprintk(CVP_ERR,
				"%s: DSP2CPU cmd:%d pending %d %d expect %d\n",
					__func__, rsp->type,
				me->pending_dsp2cpu_cmd.type, len,
				sizeof(struct cvp_dsp2cpu_cmd_msg));
			goto exit;
		}
		memcpy(&me->pending_dsp2cpu_cmd, rsp,
			sizeof(struct cvp_dsp2cpu_cmd_msg));
		complete(&me->completions[CPU2DSP_MAX_CMD]);
	} else {
		dprintk(CVP_ERR, "%s: Invalid type: %d\n", __func__, rsp->type);
		return 0;
	}

	return 0;
exit:
	dprintk(CVP_ERR, "concurrent dsp cmd type = %d, rsp type = %d\n",
			me->pending_dsp2cpu_cmd.type,
			me->pending_dsp2cpu_rsp.type);
	return 0;
}

int cvp_dsp_suspend(uint32_t session_flag)
{
	int rc = 0;
	struct cvp_dsp_cmd_msg cmd;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp;
	bool retried = false;

	cmd.type = CPU2DSP_SUSPEND;

	mutex_lock(&me->tx_lock);
	if (me->state != DSP_READY)
		goto exit;

retry:
	/* Use cvp_dsp_send_cmd_sync after dsp driver is ready */
	rc = cvp_dsp_send_cmd_sync(&cmd,
			sizeof(struct cvp_dsp_cmd_msg),
			&rsp);
	if (rc) {
		dprintk(CVP_ERR,
			"%s: cvp_dsp_send_cmd failed rc = %d\n",
			__func__, rc);
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EUNAVAILABLE)
		goto fatal_exit;

	if (rsp.ret == CPU2DSP_EFATAL) {
		if (!retried) {
			mutex_unlock(&me->tx_lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->tx_lock);
			if (rc)
				goto fatal_exit;
			else
				goto retry;
		} else {
			goto fatal_exit;
		}
	}

	me->state = DSP_SUSPEND;
	goto exit;

fatal_exit:
	me->state = DSP_INVALID;
	cvp_hyp_assign_from_dsp();
	rc = -ENOTSUPP;
exit:
	mutex_unlock(&me->tx_lock);
	return rc;
}

int cvp_dsp_resume(uint32_t session_flag)
{
	int rc = 0;
	struct cvp_dsp_cmd_msg cmd;
	struct cvp_dsp_apps *me = &gfa_cv;

	cmd.type = CPU2DSP_RESUME;

	/*
	 * Deadlock against DSP2CPU_CREATE_SESSION in dsp_thread
	 * Probably get rid of this entirely as discussed before
	 */
	if (me->state != DSP_SUSPEND)
		goto exit;

	me->state = DSP_READY;

exit:
	return rc;
}

int cvp_dsp_shutdown(uint32_t session_flag)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	int rc = 0;
	struct cvp_dsp_cmd_msg cmd;
	struct cvp_dsp_rsp_msg rsp;

	cmd.type = CPU2DSP_SHUTDOWN;

	mutex_lock(&me->tx_lock);
	if (me->state == DSP_INVALID)
		goto exit;

	me->state = DSP_INACTIVE;
	rc = cvp_dsp_send_cmd_sync(&cmd, sizeof(struct cvp_dsp_cmd_msg), &rsp);
	if (rc) {
		dprintk(CVP_ERR,
			"%s: cvp_dsp_send_cmd failed with rc = %d\n",
			__func__, rc);
		cvp_hyp_assign_from_dsp();
		goto exit;
	}

	rc = cvp_hyp_assign_from_dsp();

exit:
	mutex_unlock(&me->tx_lock);
	return rc;
}

int cvp_dsp_register_buffer(uint32_t session_id, uint32_t buff_fd,
			uint32_t buff_fd_size, uint32_t buff_size,
			uint32_t buff_offset, uint32_t buff_index,
			uint32_t buff_fd_iova)
{
	struct cvp_dsp_cmd_msg cmd;
	int rc;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp;
	bool retried = false;

	cmd.type = CPU2DSP_REGISTER_BUFFER;
	cmd.session_id = session_id;
	cmd.buff_fd = buff_fd;
	cmd.buff_fd_size = buff_fd_size;
	cmd.buff_size = buff_size;
	cmd.buff_offset = buff_offset;
	cmd.buff_index = buff_index;
	cmd.buff_fd_iova = buff_fd_iova;

	dprintk(CVP_DSP,
		"%s: type=0x%x, buff_fd_iova=0x%x buff_index=0x%x\n",
		__func__, cmd.type, buff_fd_iova,
		cmd.buff_index);
	dprintk(CVP_DSP, "%s: buff_size=0x%x session_id=0x%x\n",
		__func__, cmd.buff_size, cmd.session_id);

	mutex_lock(&me->tx_lock);
retry:
	rc = cvp_dsp_send_cmd_sync(&cmd, sizeof(struct cvp_dsp_cmd_msg), &rsp);
	if (rc) {
		dprintk(CVP_ERR, "%s send failed rc = %d\n", __func__, rc);
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EFAIL || rsp.ret == CPU2DSP_EUNSUPPORTED) {
		dprintk(CVP_WARN, "%s, DSP return err %d\n", __func__, rsp.ret);
		rc = -EINVAL;
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EUNAVAILABLE)
		goto fatal_exit;

	if (rsp.ret == CPU2DSP_EFATAL) {
		if (!retried) {
			mutex_unlock(&me->tx_lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->tx_lock);
			if (rc)
				goto fatal_exit;
			else
				goto retry;
		} else {
			goto fatal_exit;
		}
	}

	goto exit;

fatal_exit:
	me->state = DSP_INVALID;
	cvp_hyp_assign_from_dsp();
	rc = -ENOTSUPP;
exit:
	mutex_unlock(&me->tx_lock);
	return rc;
}

int cvp_dsp_deregister_buffer(uint32_t session_id, uint32_t buff_fd,
			uint32_t buff_fd_size, uint32_t buff_size,
			uint32_t buff_offset, uint32_t buff_index,
			uint32_t buff_fd_iova)
{
	struct cvp_dsp_cmd_msg cmd;
	int rc;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp;
	bool retried = false;

	cmd.type = CPU2DSP_DEREGISTER_BUFFER;
	cmd.session_id = session_id;
	cmd.buff_fd = buff_fd;
	cmd.buff_fd_size = buff_fd_size;
	cmd.buff_size = buff_size;
	cmd.buff_offset = buff_offset;
	cmd.buff_index = buff_index;
	cmd.buff_fd_iova = buff_fd_iova;

	dprintk(CVP_DSP,
		"%s: type=0x%x, buff_fd_iova=0x%x buff_index=0x%x\n",
		__func__, cmd.type, buff_fd_iova,
		cmd.buff_index);
	dprintk(CVP_DSP, "%s: buff_size=0x%x session_id=0x%x\n",
		__func__, cmd.buff_size, cmd.session_id);

	mutex_lock(&me->tx_lock);
retry:
	rc = cvp_dsp_send_cmd_sync(&cmd, sizeof(struct cvp_dsp_cmd_msg), &rsp);
	if (rc) {
		dprintk(CVP_ERR, "%s send failed rc = %d\n", __func__, rc);
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EFAIL || rsp.ret == CPU2DSP_EUNSUPPORTED) {
		dprintk(CVP_WARN, "%s, DSP return err %d\n", __func__, rsp.ret);
		rc = -EINVAL;
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EUNAVAILABLE)
		goto fatal_exit;

	if (rsp.ret == CPU2DSP_EFATAL) {
		if (!retried) {
			mutex_unlock(&me->tx_lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->tx_lock);
			if (rc)
				goto fatal_exit;
			else
				goto retry;
		} else {
			goto fatal_exit;
		}
	}

	goto exit;

fatal_exit:
	me->state = DSP_INVALID;
	cvp_hyp_assign_from_dsp();
	rc = -ENOTSUPP;
exit:
	mutex_unlock(&me->tx_lock);
	return rc;
}

static const struct rpmsg_device_id cvp_dsp_rpmsg_match[] = {
	{ CVP_APPS_DSP_GLINK_GUID },
	{ },
};

static struct rpmsg_driver cvp_dsp_rpmsg_client = {
	.id_table = cvp_dsp_rpmsg_match,
	.probe = cvp_dsp_rpmsg_probe,
	.remove = cvp_dsp_rpmsg_remove,
	.callback = cvp_dsp_rpmsg_callback,
	.drv = {
		.name = "qcom,msm_cvp_dsp_rpmsg",
	},
};

static void cvp_dsp_set_queue_hdr_defaults(struct cvp_hfi_queue_header *q_hdr)
{
	q_hdr->qhdr_status = 0x1;
	q_hdr->qhdr_type = CVP_IFACEQ_DFLT_QHDR;
	q_hdr->qhdr_q_size = CVP_IFACEQ_QUEUE_SIZE / 4;
	q_hdr->qhdr_pkt_size = 0;
	q_hdr->qhdr_rx_wm = 0x1;
	q_hdr->qhdr_tx_wm = 0x1;
	q_hdr->qhdr_rx_req = 0x1;
	q_hdr->qhdr_tx_req = 0x0;
	q_hdr->qhdr_rx_irq_status = 0x0;
	q_hdr->qhdr_tx_irq_status = 0x0;
	q_hdr->qhdr_read_idx = 0x0;
	q_hdr->qhdr_write_idx = 0x0;
}

void cvp_dsp_init_hfi_queue_hdr(struct iris_hfi_device *device)
{
	u32 i;
	struct cvp_hfi_queue_table_header *q_tbl_hdr;
	struct cvp_hfi_queue_header *q_hdr;
	struct cvp_iface_q_info *iface_q;

	for (i = 0; i < CVP_IFACEQ_NUMQ; i++) {
		iface_q = &device->dsp_iface_queues[i];
		iface_q->q_hdr = CVP_IFACEQ_GET_QHDR_START_ADDR(
			device->dsp_iface_q_table.align_virtual_addr, i);
		cvp_dsp_set_queue_hdr_defaults(iface_q->q_hdr);
	}
	q_tbl_hdr = (struct cvp_hfi_queue_table_header *)
			device->dsp_iface_q_table.align_virtual_addr;
	q_tbl_hdr->qtbl_version = 0;
	q_tbl_hdr->device_addr = (void *)device;
	strlcpy(q_tbl_hdr->name, "msm_cvp", sizeof(q_tbl_hdr->name));
	q_tbl_hdr->qtbl_size = CVP_IFACEQ_TABLE_SIZE;
	q_tbl_hdr->qtbl_qhdr0_offset =
				sizeof(struct cvp_hfi_queue_table_header);
	q_tbl_hdr->qtbl_qhdr_size = sizeof(struct cvp_hfi_queue_header);
	q_tbl_hdr->qtbl_num_q = CVP_IFACEQ_NUMQ;
	q_tbl_hdr->qtbl_num_active_q = CVP_IFACEQ_NUMQ;

	iface_q = &device->dsp_iface_queues[CVP_IFACEQ_CMDQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_HOST_TO_CTRL_CMD_Q;

	iface_q = &device->dsp_iface_queues[CVP_IFACEQ_MSGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_MSG_Q;

	iface_q = &device->dsp_iface_queues[CVP_IFACEQ_DBGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q;
	/*
	 * Set receive request to zero on debug queue as there is no
	 * need of interrupt from cvp hardware for debug messages
	 */
	q_hdr->qhdr_rx_req = 0;
}

static int __reinit_dsp(void)
{
	int rc;
	uint32_t flag = 0;
	uint64_t addr;
	uint32_t size;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp;
	struct msm_cvp_core *core;
	struct iris_hfi_device *device;

	core = list_first_entry(&cvp_driver->cores, struct msm_cvp_core, list);
	if (core && core->device)
		device = core->device->hfi_device_data;
	else
		return -EINVAL;

	if (!device) {
		dprintk(CVP_ERR, "%s: NULL device\n", __func__);
		return -EINVAL;
	}

	/* Force shutdown DSP */
	rc = cvp_dsp_shutdown(flag);
	if (rc)
		return rc;

	/* Resend HFI queue */
	mutex_lock(&me->tx_lock);
	if (!device->dsp_iface_q_table.align_virtual_addr) {
		dprintk(CVP_ERR, "%s: DSP HFI queue released\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	addr = (uint64_t)device->dsp_iface_q_table.mem_data.dma_handle;
	size = device->dsp_iface_q_table.mem_data.size;

	if (!addr || !size) {
		dprintk(CVP_DSP, "%s: HFI queue is not ready\n", __func__);
		goto exit;
	}

	rc = cvp_hyp_assign_to_dsp(addr, size);
	if (rc) {
		dprintk(CVP_ERR, "%s: cvp_hyp_assign_to_dsp. rc=%d\n",
			__func__, rc);
		goto exit;
	}

	rc = cvp_dsp_send_cmd_hfi_queue((phys_addr_t *)addr, size, &rsp);
	if (rc) {
		dprintk(CVP_WARN, "%s: Send HFI Queue failed rc = %d\n",
			__func__, rc);

		goto exit;
	}
	if (rsp.ret) {
		dprintk(CVP_ERR, "%s: DSP error %d %d\n", __func__,
				rsp.ret, rsp.dsp_state);
		rc = -ENODEV;
	}
exit:
	mutex_unlock(&me->tx_lock);
	return rc;
}

static int cvp_reinit_dsp(void)
{
	int rc;
	struct cvp_dsp_apps *me = &gfa_cv;

	rc = __reinit_dsp();
	if (rc)	{
		mutex_lock(&me->tx_lock);
		me->state = DSP_INVALID;
		cvp_hyp_assign_from_dsp();
		mutex_unlock(&me->tx_lock);
	}
	return rc;
}

static struct cvp_dsp_fastrpc_driver_entry *cvp_find_fastrpc_node_with_handle(
			uint32_t handle)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct list_head *ptr = NULL, *next = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL, *tmp_node = NULL;

	mutex_lock(&me->fastrpc_driver_list.lock);
	list_for_each_safe(ptr, next, &me->fastrpc_driver_list.list) {
		tmp_node = list_entry(ptr,
				struct cvp_dsp_fastrpc_driver_entry, list);
		if (handle == tmp_node->handle) {
			frpc_node = tmp_node;
			dprintk(CVP_DSP, "Find tmp_node with handle 0x%x\n",
				handle);
			break;
		}
	}
	mutex_unlock(&me->fastrpc_driver_list.lock);

	dprintk(CVP_DSP, "%s found fastrpc probe handle %pK pid 0x%x\n",
		__func__, frpc_node, handle);
	return frpc_node;
}

static void eva_fastrpc_driver_unregister(uint32_t handle, bool force_exit);

static int cvp_fastrpc_probe(struct fastrpc_device *rpc_dev)
{
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;

	dprintk(CVP_DSP, "%s fastrpc probe handle 0x%x\n",
		__func__, rpc_dev->handle);

	frpc_node = cvp_find_fastrpc_node_with_handle(rpc_dev->handle);
	if (frpc_node) {
		frpc_node->cvp_fastrpc_device = rpc_dev;
		// static structure with signal and pid
		complete(&frpc_node->fastrpc_probe_completion);
	}

	return 0;
}

static int cvp_fastrpc_callback(struct fastrpc_device *rpc_dev,
			enum fastrpc_driver_status fastrpc_proc_num)
{
	dprintk(CVP_DSP, "%s handle 0x%x, proc %d\n", __func__,
			rpc_dev->handle, fastrpc_proc_num);

	/* fastrpc drive down when process gone
	 * any handling can happen here, such as
	 * eva_fastrpc_driver_unregister(rpc_dev->handle, true);
	 */
	eva_fastrpc_driver_unregister(rpc_dev->handle, true);

	return 0;
}


static struct fastrpc_driver cvp_fastrpc_client = {
	.probe = cvp_fastrpc_probe,
	.callback = cvp_fastrpc_callback,
};


static int eva_fastrpc_dev_map_dma(struct fastrpc_device *frpc_device,
			struct cvp_internal_buf *buf,
			uint32_t dsp_remote_map,
			uint64_t *v_dsp_addr)
{
	struct fastrpc_dev_map_dma frpc_map_buf = {0};
	int rc = 0;

	if (dsp_remote_map == 1) {
		frpc_map_buf.buf = buf->smem->dma_buf;
		frpc_map_buf.size = buf->smem->size;
		frpc_map_buf.attrs = 0;

		dprintk(CVP_DSP,
			"%s frpc_map_buf size %d, dma_buf %pK, map %pK, 0x%x\n",
			__func__, frpc_map_buf.size, frpc_map_buf.buf,
			&frpc_map_buf, (unsigned long)&frpc_map_buf);
		rc = __fastrpc_driver_invoke(frpc_device, FASTRPC_DEV_MAP_DMA,
			(unsigned long)(&frpc_map_buf));
		if (rc) {
			dprintk(CVP_ERR,
				"%s Failed to map buffer 0x%x\n", __func__, rc);
			return rc;
		}
		buf->fd = (s32)frpc_map_buf.v_dsp_addr;
		*v_dsp_addr = frpc_map_buf.v_dsp_addr;
	} else {
		dprintk(CVP_DSP, "%s Buffer not mapped to dsp\n", __func__);
		buf->fd = 0;
	}

	return rc;
}

static int eva_fastrpc_dev_unmap_dma(struct fastrpc_device *frpc_device,
			struct cvp_internal_buf *buf)
{
	struct fastrpc_dev_unmap_dma frpc_unmap_buf = {0};
	int rc = 0;

	/* Only if buffer is mapped to dsp */
	if (buf->fd != 0) {
		frpc_unmap_buf.buf = buf->smem->dma_buf;
		rc = __fastrpc_driver_invoke(frpc_device, FASTRPC_DEV_UNMAP_DMA,
				(unsigned long)(&frpc_unmap_buf));
		if (rc) {
			dprintk(CVP_ERR, "%s Failed to unmap buffer 0x%x\n",
				__func__, rc);
			return rc;
		}
	} else {
		dprintk(CVP_DSP, "%s buffer not mapped to dsp\n", __func__);
	}

	return rc;
}

static void eva_fastrpc_driver_add_sess(
	struct cvp_dsp_fastrpc_driver_entry *frpc,
	struct msm_cvp_inst *inst)
{
	mutex_lock(&frpc->dsp_sessions.lock);
	if (inst)
		list_add_tail(&inst->dsp_list, &frpc->dsp_sessions.list);
	else
		dprintk(CVP_ERR, "%s incorrect input %pK\n", __func__, inst);
	frpc->session_cnt++;
	mutex_unlock(&frpc->dsp_sessions.lock);
	dprintk(CVP_DSP, "add dsp sess %pK fastrpc_driver %pK\n", inst, frpc);
}

int cvp_dsp_fastrpc_unmap(uint32_t process_id, struct cvp_internal_buf *buf)
{
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct fastrpc_device *frpc_device = NULL;
	int rc = 0;

	frpc_node = cvp_find_fastrpc_node_with_handle(process_id);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s no frpc node for process id %d\n",
			__func__, process_id);
		return -EINVAL;
	}
	frpc_device = frpc_node->cvp_fastrpc_device;
	rc = eva_fastrpc_dev_unmap_dma(frpc_device, buf);
	if (rc) {
		dprintk(CVP_ERR,
			"%s Fail to unmap buffer 0x%x\n",
				__func__, rc);
		return rc;
	}

	return rc;
}

int cvp_dsp_del_sess(uint32_t process_id, struct msm_cvp_inst *inst)
{
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct list_head *ptr = NULL, *next = NULL;
	struct msm_cvp_inst *sess;
	bool found = false;

	frpc_node = cvp_find_fastrpc_node_with_handle(process_id);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s no frpc node for process id %d\n",
				__func__, process_id);
		return -EINVAL;
	}
	mutex_lock(&frpc_node->dsp_sessions.lock);
	list_for_each_safe(ptr, next, &frpc_node->dsp_sessions.list) {
		sess = list_entry(ptr, struct msm_cvp_inst, dsp_list);
		if (sess == inst) {
			dprintk(CVP_DSP, "%s Find sess %pK to be deleted\n",
				__func__, inst);
			found = true;
			break;
		}
	}
	if (found) {
		list_del(&inst->dsp_list);
		frpc_node->session_cnt--;
	}

	mutex_unlock(&frpc_node->dsp_sessions.lock);

	return 0;
}

static int eva_fastrpc_driver_register(uint32_t handle)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	int rc = 0;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	bool skip_deregister = true;

	dprintk(CVP_DSP, "%s -> cvp_find_fastrpc_node_with_handle pid 0x%x\n",
			__func__, handle);
	frpc_node = cvp_find_fastrpc_node_with_handle(handle);

	if (frpc_node == NULL) {
		dprintk(CVP_DSP, "%s new fastrpc node pid 0x%x\n",
				__func__, handle);
		frpc_node = kzalloc(sizeof(*frpc_node), GFP_KERNEL);
		if (!frpc_node) {
			dprintk(CVP_DSP, "%s allocate frpc node fail\n",
				__func__);
			return -EINVAL;
		}

		memset(frpc_node, 0, sizeof(*frpc_node));

		/* Setup fastrpc_node */
		frpc_node->handle = handle;
		frpc_node->cvp_fastrpc_driver = cvp_fastrpc_client;
		frpc_node->cvp_fastrpc_driver.handle = handle;
		mutex_lock(&me->driver_name_lock);
		rc = eva_fastrpc_driver_get_name(frpc_node);
		mutex_unlock(&me->driver_name_lock);
		if (rc) {
			dprintk(CVP_ERR, "%s fastrpc get name fail err %d\n",
				__func__, rc);
			goto fail_fastrpc_driver_get_name;
		}

		/* Init completion */
		init_completion(&frpc_node->fastrpc_probe_completion);

		mutex_lock(&me->fastrpc_driver_list.lock);
		dprintk(CVP_DSP, "Add frpc node 0x%x to list\n", frpc_node);
		list_add_tail(&frpc_node->list, &me->fastrpc_driver_list.list);
		mutex_unlock(&me->fastrpc_driver_list.lock);
		INIT_MSM_CVP_LIST(&frpc_node->dsp_sessions);

		/* register fastrpc device to this session */
		rc = __fastrpc_driver_register(&frpc_node->cvp_fastrpc_driver);
		if (rc) {
			dprintk(CVP_ERR, "%s fastrpc driver reg fail err %d\n",
				__func__, rc);
			skip_deregister = true;
			goto fail_fastrpc_driver_register;
		}

		/* signal wait reuse dsp timeout setup for now */
		if (!wait_for_completion_timeout(
				&frpc_node->fastrpc_probe_completion,
				msecs_to_jiffies(CVP_DSP_RESPONSE_TIMEOUT))) {
			dprintk(CVP_ERR, "%s fastrpc driver_register timeout\n",
				__func__);
			skip_deregister = false;
			goto fail_fastrpc_driver_register;
		}
	} else {
		dprintk(CVP_DSP, "%s fastrpc probe hndl %pK pid 0x%x\n",
			__func__, frpc_node, handle);
	}

	return rc;

fail_fastrpc_driver_register:
	/* remove list if this is the last session */
	mutex_lock(&me->fastrpc_driver_list.lock);
	list_del(&frpc_node->list);
	mutex_unlock(&me->fastrpc_driver_list.lock);

	if (!skip_deregister)
		__fastrpc_driver_unregister(&frpc_node->cvp_fastrpc_driver);

	mutex_lock(&me->driver_name_lock);
	eva_fastrpc_driver_release_name(frpc_node);
	mutex_unlock(&me->driver_name_lock);
fail_fastrpc_driver_get_name:
	kfree(frpc_node);
	return -EINVAL;
}

static void eva_fastrpc_driver_unregister(uint32_t handle, bool force_exit)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	dprintk(CVP_DSP, "%s Unregister fastrpc driver handle 0x%x, force %d\n",
		__func__, handle, (uint32_t)force_exit);

	/* Foundd fastrpc node */
	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);

	if (frpc_node == NULL) {
		dprintk(CVP_DSP, "%s fastrpc handle 0x%x unregistered\n",
			__func__, handle);
		return;
	}

	if ((frpc_node->session_cnt == 0) || force_exit) {
		dprintk(CVP_DSP, "%s session cnt %d, force %d\n",
		__func__, frpc_node->session_cnt, (uint32_t)force_exit);

		DEINIT_MSM_CVP_LIST(&frpc_node->dsp_sessions);

		/* remove list if this is the last session */
		mutex_lock(&me->fastrpc_driver_list.lock);
		list_del(&frpc_node->list);
		mutex_unlock(&me->fastrpc_driver_list.lock);

		__fastrpc_driver_unregister(&frpc_node->cvp_fastrpc_driver);
		mutex_lock(&me->driver_name_lock);
		eva_fastrpc_driver_release_name(frpc_node);
		mutex_unlock(&me->driver_name_lock);
		kfree(frpc_node);
	}
}

void cvp_dsp_send_debug_mask(void)
{
	struct cvp_dsp_cmd_msg cmd;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp;
	int rc;

	cmd.type = CPU2DSP_SET_DEBUG_LEVEL;
	cmd.eva_dsp_debug_mask = me->debug_mask;

	dprintk(CVP_DSP,
		"%s: debug mask 0x%x\n",
		__func__, cmd.eva_dsp_debug_mask);

	rc = cvp_dsp_send_cmd_sync(&cmd, sizeof(struct cvp_dsp_cmd_msg), &rsp);
	if (rc)
		dprintk(CVP_ERR,
			"%s: cvp_dsp_send_cmd failed rc = %d\n",
			__func__, rc);
}

void cvp_dsp_send_hfi_queue(void)
{
	struct msm_cvp_core *core;
	struct iris_hfi_device *device;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_rsp_msg rsp = {0};
	uint64_t addr;
	uint32_t size;
	int rc;

	core = list_first_entry(&cvp_driver->cores, struct msm_cvp_core, list);
	if (core && core->device)
		device = core->device->hfi_device_data;
	else
		return;

	if (!device) {
		dprintk(CVP_ERR, "%s: NULL device\n", __func__);
		return;
	}

	dprintk(CVP_DSP, "Entering %s\n", __func__);

	mutex_lock(&device->lock);
	mutex_lock(&me->tx_lock);

	if (!device->dsp_iface_q_table.align_virtual_addr) {
		dprintk(CVP_ERR, "%s: DSP HFI queue released\n", __func__);
		goto exit;
	}

	addr = (uint64_t)device->dsp_iface_q_table.mem_data.dma_handle;
	size = device->dsp_iface_q_table.mem_data.size;

	if (!addr || !size) {
		dprintk(CVP_DSP, "%s: HFI queue is not ready\n", __func__);
		goto exit;
	}

	if (me->state != DSP_PROBED && me->state != DSP_INACTIVE)
		goto exit;

	rc = cvp_hyp_assign_to_dsp(addr, size);
	if (rc) {
		dprintk(CVP_ERR, "%s: cvp_hyp_assign_to_dsp. rc=%d\n",
			__func__, rc);
		goto exit;
	}

	if (me->state == DSP_PROBED) {
		cvp_dsp_init_hfi_queue_hdr(device);
		dprintk(CVP_WARN,
			"%s: Done init of HFI queue headers\n", __func__);
	}

	rc = cvp_dsp_send_cmd_hfi_queue((phys_addr_t *)addr, size, &rsp);
	if (rc) {
		dprintk(CVP_WARN, "%s: Send HFI Queue failed rc = %d\n",
			__func__, rc);

		goto exit;
	}

	if (rsp.ret == CPU2DSP_EUNSUPPORTED) {
		dprintk(CVP_WARN, "%s unsupported cmd %d\n",
			__func__, rsp.type);
		goto exit;
	}

	if (rsp.ret == CPU2DSP_EFATAL || rsp.ret == CPU2DSP_EUNAVAILABLE) {
		dprintk(CVP_ERR, "%s fatal error returned %d\n",
				__func__, rsp.dsp_state);
		me->state = DSP_INVALID;
		cvp_hyp_assign_from_dsp();
		goto exit;
	} else if (rsp.ret == CPU2DSP_EINVALSTATE) {
		dprintk(CVP_ERR, "%s dsp invalid state %d\n",
				__func__, rsp.dsp_state);
		mutex_unlock(&me->tx_lock);
		if (cvp_reinit_dsp()) {
			dprintk(CVP_ERR, "%s reinit dsp fail\n", __func__);
			mutex_unlock(&device->lock);
			return;
		}
		mutex_lock(&me->tx_lock);
	}

	dprintk(CVP_DSP, "%s: dsp initialized\n", __func__);
	me->state = DSP_READY;

exit:
	mutex_unlock(&me->tx_lock);
	mutex_unlock(&device->lock);
}
/* 32 or 64 bit CPU Side Ptr <-> 2 32 bit DSP Pointers. Dirty Fix. */
static void *ptr_dsp2cpu(uint32_t session_cpu_high, uint32_t session_cpu_low)
{
	void *inst;

	if ((session_cpu_high == 0) && (sizeof(void *) == BITPTRSIZE32)) {
		inst = (void *)((uintptr_t)session_cpu_low);
	} else if ((session_cpu_high != 0) && (sizeof(void *) == BITPTRSIZE64)) {
		inst = (void *)((uintptr_t)(((uint64_t)session_cpu_high) << 32
							| session_cpu_low));
	} else {
		dprintk(CVP_ERR,
			"%s Invalid _cpu_high = 0x%x _cpu_low = 0x%x\n",
				__func__, session_cpu_high, session_cpu_low);
		inst = NULL;
	}
	return inst;
}

static void print_power(const struct eva_power_req *pwr_req)
{
	if (pwr_req) {
		dprintk(CVP_DSP, "Clock: Fdu %d Ica %d Od %d Mpu %d Fw %d",
				pwr_req->clock_fdu, pwr_req->clock_ica,
				pwr_req->clock_od, pwr_req->clock_mpu,
				pwr_req->clock_fw);
		dprintk(CVP_DSP, "OpClock: Fdu %d Ica %d Od %d Mpu %d Fw %d",
				pwr_req->op_clock_fdu, pwr_req->op_clock_ica,
				pwr_req->op_clock_od, pwr_req->op_clock_mpu,
				pwr_req->op_clock_fw);
		dprintk(CVP_DSP, "Actual Bw: Ddr %d, SysCache %d",
				pwr_req->bw_ddr, pwr_req->bw_sys_cache);
		dprintk(CVP_DSP, "OpBw: Ddr %d, SysCache %d",
				pwr_req->op_bw_ddr, pwr_req->op_bw_sys_cache);
	}
}

static void __dsp_cvp_sess_create(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst = NULL;
	uint64_t inst_handle = 0;
	int rc = 0;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct pid *pid_s = NULL;
	struct task_struct *task = NULL;
	struct cvp_hfi_device *hdev;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess Type %d Mask %d Prio %d Sec %d pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_type,
		dsp2cpu_cmd->kernel_mask,
		dsp2cpu_cmd->session_prio,
		dsp2cpu_cmd->is_secure,
		dsp2cpu_cmd->pid);

	rc = eva_fastrpc_driver_register(dsp2cpu_cmd->pid);
	if (rc) {
		dprintk(CVP_ERR, "%s Register fastrpc driver fail\n", __func__);
		cmd->ret = -1;
		return;
	}

	inst = msm_cvp_open(MSM_CORE_CVP, MSM_CVP_DSP);
	if (!inst) {
		dprintk(CVP_ERR, "%s Failed create instance\n", __func__);
		goto fail_msm_cvp_open;
	}

	inst->process_id = dsp2cpu_cmd->pid;
	inst->prop.kernel_mask = dsp2cpu_cmd->kernel_mask;
	inst->prop.type =  dsp2cpu_cmd->session_type;
	inst->prop.priority = dsp2cpu_cmd->session_prio;
	inst->prop.is_secure = dsp2cpu_cmd->is_secure;
	inst->prop.dsp_mask = dsp2cpu_cmd->dsp_access_mask;

	rc = msm_cvp_session_create(inst);
	if (rc) {
		dprintk(CVP_ERR, "Warning: send Session Create failed\n");
		goto fail_session_create;
	} else {
		dprintk(CVP_DSP, "%s DSP Session Create done\n", __func__);
	}

	/* Get session id */
	rc = msm_cvp_get_session_info(inst, &cmd->session_id);
	if (rc) {
		dprintk(CVP_ERR, "Warning: get session index failed %d\n", rc);
		goto fail_get_session_info;
	}

	inst_handle = (uint64_t)inst;
	cmd->session_cpu_high = (uint32_t)((inst_handle & HIGH32) >> 32);
	cmd->session_cpu_low = (uint32_t)(inst_handle & LOW32);

	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (frpc_node)
		eva_fastrpc_driver_add_sess(frpc_node, inst);

	pid_s = find_get_pid(inst->process_id);
	if (pid_s == NULL) {
		dprintk(CVP_WARN, "%s incorrect pid\n", __func__);
		goto fail_get_pid;
	}
	dprintk(CVP_DSP, "%s get pid_s 0x%x from pidA 0x%x\n", __func__,
			pid_s, inst->process_id);

	task = get_pid_task(pid_s, PIDTYPE_TGID);
	if (!task) {
		dprintk(CVP_WARN, "%s task doesn't exist\n", __func__);
		goto fail_get_task;
	}

	inst->task = task;
	dprintk(CVP_DSP,
		"%s CREATE_SESS id 0x%x, cpu_low 0x%x, cpu_high 0x%x\n",
		__func__, cmd->session_id, cmd->session_cpu_low,
		cmd->session_cpu_high);

	spin_lock(&inst->core->resources.pm_qos.lock);
	inst->core->resources.pm_qos.off_vote_cnt++;
	spin_unlock(&inst->core->resources.pm_qos.lock);
	hdev = inst->core->device;
	call_hfi_op(hdev, pm_qos_update, hdev->hfi_device_data);

	return;

fail_get_pid:
fail_get_task:
fail_get_session_info:
fail_session_create:
	msm_cvp_close(inst);
fail_msm_cvp_open:
	/* unregister fastrpc driver */
	eva_fastrpc_driver_unregister(dsp2cpu_cmd->pid, false);
	cmd->ret = -1;
}

static void __dsp_cvp_sess_delete(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct task_struct *task = NULL;
	struct cvp_hfi_device *hdev;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x low 0x%x high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s pid 0x%x not registered with fastrpc\n",
			__func__, dsp2cpu_cmd->pid);
		cmd->ret = -1;
		return;
	}

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);
	if (!inst || !is_cvp_inst_valid(inst)) {
		dprintk(CVP_ERR, "%s incorrect session ID\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_delete;
	}

	rc = msm_cvp_session_delete(inst);
	if (rc) {
		dprintk(CVP_ERR, "Warning: send Delete Session failed\n");
		cmd->ret = -1;
		goto dsp_fail_delete;
	}

	task = inst->task;

	spin_lock(&inst->core->resources.pm_qos.lock);
	if (inst->core->resources.pm_qos.off_vote_cnt > 0)
		inst->core->resources.pm_qos.off_vote_cnt--;
	else
		dprintk(CVP_WARN, "%s Unexpected pm_qos off vote %d\n",
			__func__,
			inst->core->resources.pm_qos.off_vote_cnt);
	spin_unlock(&inst->core->resources.pm_qos.lock);

	hdev = inst->core->device;
	call_hfi_op(hdev, pm_qos_update, hdev->hfi_device_data);

	rc = msm_cvp_close(inst);
	if (rc) {
		dprintk(CVP_ERR, "Warning: Failed to close cvp instance\n");
		cmd->ret = -1;
		goto dsp_fail_delete;
	}

	/* unregister fastrpc driver */
	eva_fastrpc_driver_unregister(dsp2cpu_cmd->pid, false);

	if (task)
		put_task_struct(task);

	dprintk(CVP_DSP, "%s DSP2CPU_DETELE_SESSION Done\n", __func__);
dsp_fail_delete:
	return;
}

static void __dsp_cvp_power_req(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;
	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high);

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	if (!inst || !is_cvp_inst_valid(inst)) {
		dprintk(CVP_ERR, "%s incorrect session ID %llx\n", __func__, inst);
		cmd->ret = -1;
		goto dsp_fail_power_req;
	}

	print_power(&dsp2cpu_cmd->power_req);

	inst->prop.fdu_cycles = dsp2cpu_cmd->power_req.clock_fdu;
	inst->prop.ica_cycles =	dsp2cpu_cmd->power_req.clock_ica;
	inst->prop.od_cycles =	dsp2cpu_cmd->power_req.clock_od;
	inst->prop.mpu_cycles =	dsp2cpu_cmd->power_req.clock_mpu;
	inst->prop.fw_cycles = dsp2cpu_cmd->power_req.clock_fw;
	inst->prop.ddr_bw = dsp2cpu_cmd->power_req.bw_ddr;
	inst->prop.ddr_cache = dsp2cpu_cmd->power_req.bw_sys_cache;
	inst->prop.fdu_op_cycles = dsp2cpu_cmd->power_req.op_clock_fdu;
	inst->prop.ica_op_cycles = dsp2cpu_cmd->power_req.op_clock_ica;
	inst->prop.od_op_cycles = dsp2cpu_cmd->power_req.op_clock_od;
	inst->prop.mpu_op_cycles = dsp2cpu_cmd->power_req.op_clock_mpu;
	inst->prop.fw_op_cycles = dsp2cpu_cmd->power_req.op_clock_fw;
	inst->prop.ddr_op_bw = dsp2cpu_cmd->power_req.op_bw_ddr;
	inst->prop.ddr_op_cache = dsp2cpu_cmd->power_req.op_bw_sys_cache;

	rc = msm_cvp_update_power(inst);
	if (rc) {
		/*
		 *May need to define more error types
		 * Check UMD implementation
		 */
		dprintk(CVP_ERR, "%s Failed update power\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_power_req;
	}

	dprintk(CVP_DSP, "%s DSP2CPU_POWER_REQUEST Done\n", __func__);
dsp_fail_power_req:
	return;
}

static void __dsp_cvp_buf_register(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	struct eva_kmd_arg *kmd;
	struct eva_kmd_buffer *kmd_buf;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

	kmd = kzalloc(sizeof(*kmd), GFP_KERNEL);
        if (!kmd) {
		dprintk(CVP_ERR, "%s kzalloc failure\n", __func__);
		cmd->ret = -1;
		return;
	}

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	kmd->type = EVA_KMD_REGISTER_BUFFER;
	kmd_buf = (struct eva_kmd_buffer *)&(kmd->data.regbuf);
	kmd_buf->type = EVA_KMD_BUFTYPE_INPUT;
	kmd_buf->index = dsp2cpu_cmd->sbuf.index;
	kmd_buf->fd = dsp2cpu_cmd->sbuf.fd;
	kmd_buf->size = dsp2cpu_cmd->sbuf.size;
	kmd_buf->offset = dsp2cpu_cmd->sbuf.offset;
	kmd_buf->pixelformat = 0;
	kmd_buf->flags = EVA_KMD_FLAG_UNSECURE;

	rc = msm_cvp_register_buffer(inst, kmd_buf);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to register buffer\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_buf_reg;
	}
	dprintk(CVP_DSP, "%s register buffer done\n", __func__);

	cmd->sbuf.iova = kmd_buf->reserved[0];
	cmd->sbuf.size = kmd_buf->size;
	cmd->sbuf.fd = kmd_buf->fd;
	cmd->sbuf.index = kmd_buf->index;
	cmd->sbuf.offset = kmd_buf->offset;
	dprintk(CVP_DSP, "%s: fd %d, iova 0x%x\n", __func__,
			cmd->sbuf.fd, cmd->sbuf.iova);
dsp_fail_buf_reg:
	kfree(kmd);
}

static void __dsp_cvp_buf_deregister(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	struct eva_kmd_arg *kmd;
	struct eva_kmd_buffer *kmd_buf;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s : sess id 0x%x, low 0x%x, high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

	kmd = kzalloc(sizeof(*kmd), GFP_KERNEL);
        if (!kmd) {
		dprintk(CVP_ERR, "%s kzalloc failure\n", __func__);
		cmd->ret = -1;
		return;
	}

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	kmd->type = EVA_KMD_UNREGISTER_BUFFER;
	kmd_buf = (struct eva_kmd_buffer *)&(kmd->data.regbuf);
	kmd_buf->type = EVA_KMD_UNREGISTER_BUFFER;

	kmd_buf->type = EVA_KMD_BUFTYPE_INPUT;
	kmd_buf->index = dsp2cpu_cmd->sbuf.index;
	kmd_buf->fd = dsp2cpu_cmd->sbuf.fd;
	kmd_buf->size = dsp2cpu_cmd->sbuf.size;
	kmd_buf->offset = dsp2cpu_cmd->sbuf.offset;
	kmd_buf->pixelformat = 0;
	kmd_buf->flags = EVA_KMD_FLAG_UNSECURE;

	rc = msm_cvp_unregister_buffer(inst, kmd_buf);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to deregister buffer\n", __func__);
		cmd->ret = -1;
		goto fail_dsp_buf_dereg;
	}

	dprintk(CVP_DSP, "%s deregister buffer done\n", __func__);
fail_dsp_buf_dereg:
	kfree(kmd);
}

static void __dsp_cvp_mem_alloc(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	int rc;
	struct cvp_internal_buf *buf = NULL;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;
	uint64_t v_dsp_addr = 0;

	struct fastrpc_device *frpc_device = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s Failed to find fastrpc node 0x%x\n",
				__func__, dsp2cpu_cmd->pid);
		goto fail_fastrpc_node;
	}
	frpc_device = frpc_node->cvp_fastrpc_device;

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	buf = kmem_cache_zalloc(cvp_driver->buf_cache, GFP_KERNEL);
	if (!buf)
		goto fail_kzalloc_buf;

	rc = cvp_allocate_dsp_bufs(inst, buf,
			dsp2cpu_cmd->sbuf.size,
			dsp2cpu_cmd->sbuf.type);
	if (rc)
		goto fail_allocate_dsp_buf;

	rc = eva_fastrpc_dev_map_dma(frpc_device, buf,
			dsp2cpu_cmd->sbuf.dsp_remote_map,
			&v_dsp_addr);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to map buffer 0x%x\n", __func__,
			rc);
		goto fail_fastrpc_dev_map_dma;
	}

	mutex_lock(&inst->cvpdspbufs.lock);
	list_add_tail(&buf->list, &inst->cvpdspbufs.list);
	mutex_unlock(&inst->cvpdspbufs.lock);

	dprintk(CVP_DSP, "%s allocate buffer done, addr 0x%llx\n",
		__func__, v_dsp_addr);

	cmd->sbuf.size = buf->smem->size;
	cmd->sbuf.fd = buf->fd;
	cmd->sbuf.offset = 0;
	cmd->sbuf.iova = buf->smem->device_addr;
	cmd->sbuf.v_dsp_addr = v_dsp_addr;
	dprintk(CVP_DSP, "%s: size %d, iova 0x%x, v_dsp_addr 0x%llx\n",
		__func__, cmd->sbuf.size, cmd->sbuf.iova,
		cmd->sbuf.v_dsp_addr);

	return;

fail_fastrpc_dev_map_dma:
	cvp_release_dsp_buffers(inst, buf);
fail_allocate_dsp_buf:
	kmem_cache_free(cvp_driver->buf_cache, buf);
fail_kzalloc_buf:
fail_fastrpc_node:
	cmd->ret = -1;
	return;

}

static void __dsp_cvp_mem_free(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	int rc;
	struct cvp_internal_buf *buf = NULL;
	struct list_head *ptr = NULL, *next = NULL;
	struct msm_cvp_list *buf_list = NULL;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	struct fastrpc_device *frpc_device = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);
	if (!inst) {
		dprintk(CVP_ERR, "%s Failed to get inst\n",
			__func__);
		cmd->ret = -1;
		return;
	}

	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s Failed to find fastrpc node 0x%x\n",
				__func__, dsp2cpu_cmd->pid);
		cmd->ret = -1;
		return;
	}
	frpc_device = frpc_node->cvp_fastrpc_device;

	buf_list = &inst->cvpdspbufs;
	mutex_lock(&buf_list->lock);
	list_for_each_safe(ptr, next, &buf_list->list) {
		buf = list_entry(ptr, struct cvp_internal_buf, list);
		dprintk(CVP_DSP, "fd in list 0x%x, fd from dsp 0x%x\n",
					buf->fd, dsp2cpu_cmd->sbuf.fd);

		if (!buf->smem) {
			dprintk(CVP_DSP, "Empyt smem\n");
			continue;
		}

		/* Verify with device addr */
		if (buf->smem->device_addr == dsp2cpu_cmd->sbuf.iova) {
			dprintk(CVP_DSP, "%s find device addr 0x%x\n",
				__func__, buf->smem->device_addr);

			rc = eva_fastrpc_dev_unmap_dma(frpc_device, buf);
			if (rc) {
				dprintk(CVP_ERR,
					"%s Failed to unmap buffer 0x%x\n",
					__func__, rc);
				cmd->ret = -1;
				goto fail_fastrpc_dev_unmap_dma;
			}

			rc = cvp_release_dsp_buffers(inst, buf);
			if (rc) {
				dprintk(CVP_ERR,
					"%s Failed to free buffer 0x%x\n",
					__func__, rc);
				cmd->ret = -1;
				goto fail_release_buf;
			}

			list_del(&buf->list);

			kmem_cache_free(cvp_driver->buf_cache, buf);
			break;
		}
	}

fail_release_buf:
fail_fastrpc_dev_unmap_dma:
	mutex_unlock(&buf_list->lock);
}

static int cvp_dsp_thread(void *data)
{
	int rc = 0, old_state;
	struct cvp_dsp_apps *me = &gfa_cv;
	struct cvp_dsp_cmd_msg cmd;
	struct cvp_hfi_device *hdev;
	struct msm_cvp_core *core;

	core = list_first_entry(&cvp_driver->cores, struct msm_cvp_core, list);
	if (!core) {
		dprintk(CVP_ERR, "%s: Failed to find core\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	hdev = (struct cvp_hfi_device *)core->device;
	if (!hdev) {
		dprintk(CVP_ERR, "%s Invalid device handle\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

wait_dsp:
	rc = wait_for_completion_interruptible(
			&me->completions[CPU2DSP_MAX_CMD]);

	if (me->state == DSP_INVALID)
		goto exit;

	if (me->state == DSP_UNINIT)
		goto wait_dsp;

	if (me->state == DSP_PROBED) {
		cvp_dsp_send_hfi_queue();
		goto wait_dsp;
	}

	cmd.type = me->pending_dsp2cpu_cmd.type;

	if (rc == -ERESTARTSYS) {
		dprintk(CVP_WARN, "%s received interrupt signal\n", __func__);
	} else {
		mutex_lock(&me->rx_lock);
		switch (me->pending_dsp2cpu_cmd.type) {
		case DSP2CPU_POWERON:
		{
			if (me->state == DSP_READY) {
				cmd.ret = 0;
				break;
			}

			mutex_lock(&me->tx_lock);
			old_state = me->state;
			me->state = DSP_READY;
			rc = call_hfi_op(hdev, resume, hdev->hfi_device_data);
			if (rc) {
				dprintk(CVP_WARN, "%s Failed to resume cvp\n",
						__func__);
				me->state = old_state;
				mutex_unlock(&me->tx_lock);
				cmd.ret = 1;
				break;
			}
			mutex_unlock(&me->tx_lock);
			cmd.ret = 0;
			break;
		}
		case DSP2CPU_POWEROFF:
		{
			me->state = DSP_SUSPEND;
			cmd.ret = 0;
			break;
		}
		case DSP2CPU_CREATE_SESSION:
		{
			__dsp_cvp_sess_create(&cmd);

			break;
		}
		case DSP2CPU_DETELE_SESSION:
		{
			__dsp_cvp_sess_delete(&cmd);

			break;
		}
		case DSP2CPU_POWER_REQUEST:
		{
			__dsp_cvp_power_req(&cmd);

			break;
		}
		case DSP2CPU_REGISTER_BUFFER:
		{
			__dsp_cvp_buf_register(&cmd);

			break;
		}
		case DSP2CPU_DEREGISTER_BUFFER:
		{
			__dsp_cvp_buf_deregister(&cmd);

			break;
		}
		case DSP2CPU_MEM_ALLOC:
		{
			__dsp_cvp_mem_alloc(&cmd);

			break;
		}
		case DSP2CPU_MEM_FREE:
		{
			__dsp_cvp_mem_free(&cmd);

			break;
		}
		default:
			dprintk(CVP_ERR, "unrecognaized dsp cmds: %d\n",
					me->pending_dsp2cpu_cmd.type);
			break;
		}
		me->pending_dsp2cpu_cmd.type = CVP_INVALID_RPMSG_TYPE;
		mutex_unlock(&me->rx_lock);
	}
	/* Responds to DSP */
	rc = cvp_dsp_send_cmd(&cmd, sizeof(struct cvp_dsp_cmd_msg));
	if (rc)
		dprintk(CVP_ERR,
			"%s: cvp_dsp_send_cmd failed rc = %d cmd type=%d\n",
			__func__, rc, cmd.type);
	goto wait_dsp;
exit:
	dprintk(CVP_DBG, "dsp thread exit\n");
	do_exit(rc);
	return rc;
}

int cvp_dsp_device_init(void)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	char tname[16];
	int rc;
	int i;
	char name[CVP_FASTRPC_DRIVER_NAME_SIZE] = "qcom,fastcv0\0";

	add_va_node_to_list(CVP_DBG_DUMP, &gfa_cv, sizeof(struct cvp_dsp_apps),
        "cvp_dsp_apps-gfa_cv", false);

	mutex_init(&me->tx_lock);
	mutex_init(&me->rx_lock);
	me->state = DSP_INVALID;
	me->hyp_assigned = false;

	for (i = 0; i <= CPU2DSP_MAX_CMD; i++)
		init_completion(&me->completions[i]);

	me->pending_dsp2cpu_cmd.type = CVP_INVALID_RPMSG_TYPE;
	me->pending_dsp2cpu_rsp.type = CVP_INVALID_RPMSG_TYPE;

	INIT_MSM_CVP_LIST(&me->fastrpc_driver_list);

	mutex_init(&me->driver_name_lock);
	for (i = 0; i < MAX_FASTRPC_DRIVER_NUM; i++) {
		me->cvp_fastrpc_name[i].status = DRIVER_NAME_AVAILABLE;
		snprintf(me->cvp_fastrpc_name[i].name, sizeof(name), name);
		name[11]++;
	}

	rc = register_rpmsg_driver(&cvp_dsp_rpmsg_client);
	if (rc) {
		dprintk(CVP_ERR,
			"%s : register_rpmsg_driver failed rc = %d\n",
			__func__, rc);
		goto register_bail;
	}
	snprintf(tname, sizeof(tname), "cvp-dsp-thread");
	me->state = DSP_UNINIT;
	me->dsp_thread = kthread_run(cvp_dsp_thread, me, tname);
	if (!me->dsp_thread) {
		dprintk(CVP_ERR, "%s create %s fail", __func__, tname);
		rc = -ECHILD;
		me->state = DSP_INVALID;
		goto register_bail;
	}
	return 0;

register_bail:
	return rc;
}

void cvp_dsp_device_exit(void)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	int i;

	mutex_lock(&me->tx_lock);
	me->state = DSP_INVALID;
	mutex_unlock(&me->tx_lock);

	DEINIT_MSM_CVP_LIST(&me->fastrpc_driver_list);

	for (i = 0; i <= CPU2DSP_MAX_CMD; i++)
		complete_all(&me->completions[i]);

	mutex_destroy(&me->tx_lock);
	mutex_destroy(&me->rx_lock);
	mutex_destroy(&me->driver_name_lock);
	unregister_rpmsg_driver(&cvp_dsp_rpmsg_client);
}
