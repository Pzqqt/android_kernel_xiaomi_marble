// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 */
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <soc/qcom/secure_buffer.h>
#include "msm_cvp_core.h"
#include "msm_cvp.h"
#include "cvp_hfi.h"

struct cvp_dsp_apps gfa_cv;
static int hlosVM[HLOS_VM_NUM] = {VMID_HLOS};
static int dspVM[DSP_VM_NUM] = {VMID_HLOS, VMID_CDSP_Q6};
static int dspVMperm[DSP_VM_NUM] = { PERM_READ | PERM_WRITE | PERM_EXEC,
				PERM_READ | PERM_WRITE | PERM_EXEC };
static int hlosVMperm[HLOS_VM_NUM] = { PERM_READ | PERM_WRITE | PERM_EXEC };

static int cvp_reinit_dsp(void);

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
	cmd.ddr_type = of_fdt_get_ddrtype();
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

	mutex_lock(&me->lock);
	me->chan = rpdev;
	me->state = DSP_PROBED;
	complete(&me->completions[CPU2DSP_MAX_CMD]);
	mutex_unlock(&me->lock);

	return ret;
}

static void cvp_dsp_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct cvp_dsp_apps *me = &gfa_cv;

	dprintk(CVP_WARN, "%s: CDSP SSR triggered\n", __func__);

	mutex_lock(&me->lock);
	cvp_hyp_assign_from_dsp();

	me->chan = NULL;
	me->state = DSP_UNINIT;
	mutex_unlock(&me->lock);
	/* kernel driver needs clean all dsp sessions */

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

	mutex_lock(&me->lock);
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
			mutex_unlock(&me->lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->lock);
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
	mutex_unlock(&me->lock);
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

	mutex_lock(&me->lock);
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
	mutex_unlock(&me->lock);
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

	mutex_lock(&me->lock);
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
			mutex_unlock(&me->lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->lock);
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
	mutex_unlock(&me->lock);
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

	mutex_lock(&me->lock);
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
			mutex_unlock(&me->lock);
			retried = true;
			rc = cvp_reinit_dsp();
			mutex_lock(&me->lock);
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
	mutex_unlock(&me->lock);
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
	mutex_lock(&me->lock);
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
	mutex_unlock(&me->lock);
	return rc;
}

static int cvp_reinit_dsp(void)
{
	int rc;
	struct cvp_dsp_apps *me = &gfa_cv;

	rc = __reinit_dsp();
	if (rc)	{
		mutex_lock(&me->lock);
		me->state = DSP_INVALID;
		cvp_hyp_assign_from_dsp();
		mutex_unlock(&me->lock);
	}
	return rc;
}

#ifdef FASTRPC_DRIVER_AVAILABLE
static struct cvp_dsp_fastrpc_driver_entry *cvp_find_fastrpc_node_with_handle(
			uint32_t handle)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct list_head *ptr = NULL, *next = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;

	mutex_lock(&me->fastrpc_driver_list.lock);
	list_for_each_safe(ptr, next, &me->fastrpc_driver_list.list) {
		frpc_node = list_entry(ptr,
				struct cvp_dsp_fastrpc_driver_entry, list);
		if (handle == frpc_node->handle) {
			dprintk(CVP_DSP, "Find frpc_node with handle 0x%x\n",
				handle);
			break;
		}
	}
	mutex_unlock(&me->fastrpc_driver_list.lock);

	return frpc_node;
}

static void eva_fastrpc_driver_unregister(struct msm_cvp_inst *inst,
			uint32_t handle,
			bool force_exit);

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

	return 0;
}


static struct fastrpc_driver cvp_fastrpc_client = {
	.probe = cvp_fastrpc_probe,
	.callback = cvp_fastrpc_callback,
	.driver = {
		.name = "qcom,fastcv",
	},
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
		rc = fastrpc_driver_invoke(frpc_device, FASTRPC_DEV_MAP_DMA,
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
		rc = fastrpc_driver_invoke(frpc_device, FASTRPC_DEV_UNMAP_DMA,
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

static int eva_fastrpc_driver_register(uint32_t handle)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	int rc = 0;
	//struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;

	frpc_node = cvp_find_fastrpc_node_with_handle(handle);

	if (frpc_node == NULL) {
		frpc_node = kzalloc(sizeof(*frpc_node), GFP_KERNEL);
		if (!frpc_node) {
			dprintk(CVP_DSP, "%s allocate frpc node fail\n",
				__func__);
			return -EINVAL;
		}

		memset(frpc_node, 0, sizeof(*frpc_node));

		/* Init completion */
		init_completion(&frpc_node->fastrpc_probe_completion);

		mutex_lock(&me->fastrpc_driver_list.lock);
		dprintk(CVP_DSP, "Add frpc node 0x%x to list\n", frpc_node);
		list_add_tail(&frpc_node->list, &me->fastrpc_driver_list.list);
		mutex_unlock(&me->fastrpc_driver_list.lock);

		/* register fastrpc device to this session */
		frpc_node->handle = handle;
		frpc_node->cvp_fastrpc_driver = cvp_fastrpc_client;
		frpc_node->cvp_fastrpc_driver.handle = handle;
		rc = fastrpc_driver_register(&frpc_node->cvp_fastrpc_driver);
		if (rc) {
			dprintk(CVP_ERR, "%s fastrpc driver reg fail err %d\n",
				__func__, rc);
			goto fail_fastrpc_driver_register;
		}

		/* signal wait reuse dsp timeout setup for now */
		if (!wait_for_completion_timeout(
				&frpc_node->fastrpc_probe_completion,
				msecs_to_jiffies(CVP_DSP_RESPONSE_TIMEOUT))) {
			dprintk(CVP_ERR, "%s fastrpc driver_register timeout\n",
				__func__);
			goto fail_fastrpc_driver_timeout;
		}

		/* initialize dspbuf list */
		INIT_MSM_CVP_LIST(&frpc_node->dspbufs);
	}

	frpc_node->session_cnt++;

	return rc;

fail_fastrpc_driver_timeout:
	/* remove list if this is the last session */
	mutex_lock(&me->fastrpc_driver_list.lock);
	list_del(&frpc_node->list);
	mutex_unlock(&me->fastrpc_driver_list.lock);
	fastrpc_driver_unregister(&frpc_node->cvp_fastrpc_driver);
fail_fastrpc_driver_register:
	kfree(frpc_node);
	return -EINVAL;
}

static void eva_fastrpc_driver_unregister(struct msm_cvp_inst *inst,
			uint32_t handle,
			bool force_exit)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct list_head *ptr = NULL, *next = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct msm_cvp_list *buf_list = NULL;
	struct cvp_internal_buf *buf = NULL;
	struct fastrpc_device *frpc_device = NULL;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;
	int rc = 0;

	dprintk(CVP_DSP, "%s Unregister fastrpc driver handle 0x%x, force %d\n",
		__func__, handle, (uint32_t)force_exit);

	/* Foundd fastrpc node */
	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);

	if (frpc_node == NULL)
		return;

	frpc_node->session_cnt--;

	if ((frpc_node->session_cnt == 0) || force_exit) {
		dprintk(CVP_DSP, "%s session cnt %d, force %d\n",
		__func__, frpc_node->session_cnt, (uint32_t)force_exit);
		//Free any left over buffers
		ptr = NULL;
		next = NULL;
		buf_list = &frpc_node->dspbufs;
		mutex_lock(&buf_list->lock);
		list_for_each_safe(ptr, next, &buf_list->list) {
			buf = list_entry(ptr, struct cvp_internal_buf, list);

			if (!buf->smem) {
				dprintk(CVP_DSP, "%s Empyt smem\n", __func__);
				list_del(&buf->list);
				kfree(buf);
				continue;
			}

			rc = eva_fastrpc_dev_unmap_dma(frpc_device, buf);
			if (rc) {
				dprintk(CVP_ERR,
					"%s Fail to unmap buffer 0x%x\n",
						__func__, rc);
				continue;
			}

			rc = cvp_release_dsp_buffers(inst, buf);
			if (rc) {
				dprintk(CVP_ERR,
					"%s Fail to free buffer 0x%x\n",
						__func__, rc);
				continue;
			}

			list_del(&buf->list);
			kfree(buf);
		}
		mutex_unlock(&buf_list->lock);

		DEINIT_MSM_CVP_LIST(&frpc_node->dspbufs);

		/* remove list if this is the last session */
		mutex_lock(&me->fastrpc_driver_list.lock);
		list_del(&frpc_node->list);
		mutex_unlock(&me->fastrpc_driver_list.lock);

		fastrpc_driver_unregister(&frpc_node->cvp_fastrpc_driver);
		kfree(frpc_node);
	}
}

#endif

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
	mutex_lock(&me->lock);

	if (!device->dsp_iface_q_table.align_virtual_addr) {
		dprintk(CVP_ERR, "%s: DSP HFI queue released\n", __func__);
		mutex_unlock(&me->lock);
		mutex_unlock(&device->lock);
		return;
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
		mutex_unlock(&me->lock);
		if (cvp_reinit_dsp()) {
			dprintk(CVP_ERR, "%s reinit dsp fail\n", __func__);
			mutex_unlock(&device->lock);
			return;
		}
		mutex_lock(&me->lock);
	}

	dprintk(CVP_DSP, "%s: dsp initialized\n", __func__);
	me->state = DSP_READY;

exit:
	mutex_unlock(&me->lock);
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

static int msm_cvp_register_buffer_dsp(struct msm_cvp_inst *inst,
		struct eva_kmd_buffer *buf,
		int32_t pid,
		uint32_t *iova)
{
	struct cvp_hfi_device *hdev;
	struct cvp_hal_session *session;
	struct msm_cvp_inst *s;
	int rc = 0;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!buf->index)
		return 0;

	s = cvp_get_inst_validate(inst->core, inst);
	if (!s)
		return -ECONNRESET;

	inst->cur_cmd_type = EVA_KMD_REGISTER_BUFFER;
	session = (struct cvp_hal_session *)inst->session;
	if (!session) {
		dprintk(CVP_ERR, "%s: invalid session\n", __func__);
		rc = -EINVAL;
		goto exit;
	}
	hdev = inst->core->device;
	print_client_buffer(CVP_HFI, "register", inst, buf);

	rc = msm_cvp_map_buf_dsp_new(inst, buf, pid, iova);
	dprintk(CVP_DSP, "%s: fd %d, iova 0x%x\n", __func__, buf->fd, *iova);

exit:
	inst->cur_cmd_type = 0;
	cvp_put_inst(s);
	return rc;
}

static int msm_cvp_unregister_buffer_dsp(struct msm_cvp_inst *inst,
		struct eva_kmd_buffer *buf)
{
	struct msm_cvp_inst *s;
	int rc = 0;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!buf->index)
		return 0;

	s = cvp_get_inst_validate(inst->core, inst);
	if (!s)
		return -ECONNRESET;

	inst->cur_cmd_type = EVA_KMD_UNREGISTER_BUFFER;
	print_client_buffer(CVP_HFI, "unregister", inst, buf);

	rc = msm_cvp_unmap_buf_dsp_new(inst, buf);
	inst->cur_cmd_type = 0;
	cvp_put_inst(s);
	return rc;
}

static void __dsp_cvp_sess_create(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst = NULL;
	uint64_t inst_handle = 0;
	struct eva_kmd_arg *kmd;
	struct eva_kmd_sys_properties *sys_prop = NULL;
	struct eva_kmd_session_control *sys_ctrl = NULL;
	int rc = 0;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess Type %d Mask %d Prio %d Sec %d pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_type,
		dsp2cpu_cmd->kernel_mask,
		dsp2cpu_cmd->session_prio,
		dsp2cpu_cmd->is_secure,
		dsp2cpu_cmd->pid);

	kmd = kzalloc(sizeof(*kmd), GFP_KERNEL);
        if (!kmd) {
		dprintk(CVP_ERR, "%s kzalloc failure\n", __func__);
		goto fail_frpc_driver_reg;
	}

#ifdef FASTRPC_DRIVER_AVAILABLE
	rc = eva_fastrpc_driver_register(dsp2cpu_cmd->pid);
#endif
	if (rc) {
		dprintk(CVP_ERR, "%s Register fastrpc driver fail\n", __func__);
		goto fail_frpc_driver_reg;
	}

	inst = msm_cvp_open(MSM_CORE_CVP, MSM_CVP_USER);
	if (!inst) {
		dprintk(CVP_ERR, "%s Failed create instance\n", __func__);
		goto fail_msm_cvp_open;
	}

	kmd->type = EVA_KMD_SET_SYS_PROPERTY;
	sys_prop = (struct eva_kmd_sys_properties *)&kmd->data.sys_properties;
	sys_prop->prop_num = 5;

	sys_prop->prop_data[0].prop_type = EVA_KMD_PROP_SESSION_KERNELMASK;
	sys_prop->prop_data[0].data = dsp2cpu_cmd->kernel_mask;
	sys_prop->prop_data[1].prop_type = EVA_KMD_PROP_SESSION_TYPE;
	sys_prop->prop_data[1].data = dsp2cpu_cmd->session_type;
	sys_prop->prop_data[2].prop_type = EVA_KMD_PROP_SESSION_PRIORITY;
	sys_prop->prop_data[2].data = dsp2cpu_cmd->session_prio;
	sys_prop->prop_data[3].prop_type = EVA_KMD_PROP_SESSION_SECURITY;
	sys_prop->prop_data[3].data = dsp2cpu_cmd->is_secure;
	sys_prop->prop_data[4].prop_type = EVA_KMD_PROP_SESSION_DSPMASK;
	sys_prop->prop_data[4].data = dsp2cpu_cmd->dsp_access_mask;

	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to set sys property\n", __func__);
		goto fail_set_sys_property;
	}
	dprintk(CVP_DSP, "%s set sys property done\n", __func__);


	/* EVA_KMD_SESSION_CONTROL from DSP */
	memset(kmd, 0, sizeof(struct eva_kmd_arg));
	kmd->type = EVA_KMD_SESSION_CONTROL;
	sys_ctrl = (struct eva_kmd_session_control *)&kmd->data.session_ctrl;
	sys_ctrl->ctrl_type = SESSION_CREATE;

	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "Warning: send Session Create failed\n");
		goto fail_session_create;
	}
	dprintk(CVP_DSP, "%s send Session Create done\n", __func__);


	/* Get session id */
	memset(kmd, 0, sizeof(struct eva_kmd_arg));
	kmd->type = EVA_KMD_GET_SESSION_INFO;
	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "Warning: get session index failed\n");
		goto fail_get_session_info;
	}
	cmd->session_id = kmd->data.session.session_id;

	inst_handle = (uint64_t)inst;
	cmd->session_cpu_high = (uint32_t)((inst_handle & HIGH32) >> 32);
	cmd->session_cpu_low = (uint32_t)(inst_handle & LOW32);

	dprintk(CVP_DSP,
		"%s CREATE_SESS id 0x%x, cpu_low 0x%x, cpu_high 0x%x\n",
		__func__, cmd->session_id, cmd->session_cpu_low,
		cmd->session_cpu_high);

	kfree(kmd);
	return;

fail_get_session_info:
fail_session_create:
fail_set_sys_property:
fail_msm_cvp_open:
	/* unregister fastrpc driver */
fail_frpc_driver_reg:
	cmd->ret = -1;
	kfree(kmd);
}

static void __dsp_cvp_sess_delete(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	struct eva_kmd_arg *kmd;
	struct eva_kmd_session_control *sys_ctrl;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x low 0x%x high 0x%x, pid 0x%x\n",
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

#ifdef FASTRPC_DRIVER_AVAILABLE
	/* unregister fastrpc driver */
	eva_fastrpc_driver_unregister(inst, dsp2cpu_cmd->pid, false);
#endif

	kmd->type = EVA_KMD_SESSION_CONTROL;
	sys_ctrl = (struct eva_kmd_session_control *)&kmd->data.session_ctrl;

	/* Session delete does nothing here */
	sys_ctrl->ctrl_type = SESSION_DELETE;

	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "Warning: send Delete Session failed\n");
		cmd->ret = -1;
		goto dsp_fail_delete;
	}

	rc = msm_cvp_close(inst);
	if (rc) {
		dprintk(CVP_ERR, "Warning: Failed to close cvp instance\n");
		cmd->ret = -1;
		goto dsp_fail_delete;
	}

	dprintk(CVP_DSP, "%s DSP2CPU_DETELE_SESSION Done\n", __func__);
dsp_fail_delete:
	kfree(kmd);
}

static void __dsp_cvp_power_req(struct cvp_dsp_cmd_msg *cmd)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	struct msm_cvp_inst *inst;
	struct eva_kmd_arg *kmd;
	struct eva_kmd_sys_properties *sys_prop;
	int rc;
	struct cvp_dsp2cpu_cmd_msg *dsp2cpu_cmd = &me->pending_dsp2cpu_cmd;

	cmd->ret = 0;
	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high);

	kmd = kzalloc(sizeof(*kmd), GFP_KERNEL);
        if (!kmd) {
		dprintk(CVP_ERR, "%s kzalloc failure\n", __func__);
		cmd->ret = -1;
		return;
	}

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	print_power(&dsp2cpu_cmd->power_req);

	/* EVA_KMD_SET_SYS_PROPERTY
	 * Total 14 properties, 8 max once
	 * Need to do 2 rounds
	 */
	kmd->type = EVA_KMD_SET_SYS_PROPERTY;
	sys_prop = (struct eva_kmd_sys_properties *)&kmd->data.sys_properties;
	sys_prop->prop_num = 7;

	sys_prop->prop_data[0].prop_type = EVA_KMD_PROP_PWR_FDU;
	sys_prop->prop_data[0].data =
			dsp2cpu_cmd->power_req.clock_fdu;
	sys_prop->prop_data[1].prop_type = EVA_KMD_PROP_PWR_ICA;
	sys_prop->prop_data[1].data =
			dsp2cpu_cmd->power_req.clock_ica;
	sys_prop->prop_data[2].prop_type = EVA_KMD_PROP_PWR_OD;
	sys_prop->prop_data[2].data =
			dsp2cpu_cmd->power_req.clock_od;
	sys_prop->prop_data[3].prop_type = EVA_KMD_PROP_PWR_MPU;
	sys_prop->prop_data[3].data =
			dsp2cpu_cmd->power_req.clock_mpu;
	sys_prop->prop_data[4].prop_type = EVA_KMD_PROP_PWR_FW;
	sys_prop->prop_data[4].data =
			dsp2cpu_cmd->power_req.clock_fw;
	sys_prop->prop_data[5].prop_type = EVA_KMD_PROP_PWR_DDR;
	sys_prop->prop_data[5].data =
			dsp2cpu_cmd->power_req.bw_ddr;
	sys_prop->prop_data[6].prop_type = EVA_KMD_PROP_PWR_SYSCACHE;
	sys_prop->prop_data[6].data =
			dsp2cpu_cmd->power_req.bw_sys_cache;

	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to set sys property\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_power_req;
	}
	dprintk(CVP_DSP, "%s set sys property done part 1\n", __func__);

	/* EVA_KMD_SET_SYS_PROPERTY Round 2 */
	memset(kmd, 0, sizeof(struct eva_kmd_arg));
	kmd->type = EVA_KMD_SET_SYS_PROPERTY;
	sys_prop = (struct eva_kmd_sys_properties *)&kmd->data.sys_properties;
	sys_prop->prop_num  = 7;

	sys_prop->prop_data[0].prop_type = EVA_KMD_PROP_PWR_FDU_OP;
	sys_prop->prop_data[0].data =
			dsp2cpu_cmd->power_req.op_clock_fdu;
	sys_prop->prop_data[1].prop_type = EVA_KMD_PROP_PWR_ICA_OP;
	sys_prop->prop_data[1].data =
			dsp2cpu_cmd->power_req.op_clock_ica;
	sys_prop->prop_data[2].prop_type = EVA_KMD_PROP_PWR_OD_OP;
	sys_prop->prop_data[2].data =
			dsp2cpu_cmd->power_req.op_clock_od;
	sys_prop->prop_data[3].prop_type = EVA_KMD_PROP_PWR_MPU_OP;
	sys_prop->prop_data[3].data =
			dsp2cpu_cmd->power_req.op_clock_mpu;
	sys_prop->prop_data[4].prop_type = EVA_KMD_PROP_PWR_FW_OP;
	sys_prop->prop_data[4].data =
			dsp2cpu_cmd->power_req.op_clock_fw;
	sys_prop->prop_data[5].prop_type = EVA_KMD_PROP_PWR_DDR_OP;
	sys_prop->prop_data[5].data =
			dsp2cpu_cmd->power_req.op_bw_ddr;
	sys_prop->prop_data[6].prop_type = EVA_KMD_PROP_PWR_SYSCACHE_OP;
	sys_prop->prop_data[6].data =
			dsp2cpu_cmd->power_req.op_bw_sys_cache;

	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to set sys property\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_power_req;
	}
	dprintk(CVP_DSP, "%s set sys property done part 2\n", __func__);

	memset(kmd, 0, sizeof(struct eva_kmd_arg));
	kmd->type = EVA_KMD_UPDATE_POWER;
	rc = msm_cvp_handle_syscall(inst, kmd);
	if (rc) {
		/* May need to define more error types
		 * Check UMD implementation here:
		 * https://opengrok.qualcomm.com/source/xref/LA.UM.9.14/vendor/qcom/proprietary/cv-noship/cvp/cpurev/src/cvpcpuRev_skel_imp_cvp2.cpp#380
		 */
		dprintk(CVP_ERR, "%s Failed to send update power numbers\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_power_req;
	}

	dprintk(CVP_DSP, "%s DSP2CPU_POWER_REQUEST Done\n", __func__);
dsp_fail_power_req:
	kfree(kmd);
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

	rc = msm_cvp_register_buffer_dsp(inst, kmd_buf,
			dsp2cpu_cmd->pid, &cmd->sbuf.iova);
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to register buffer\n", __func__);
		cmd->ret = -1;
		goto dsp_fail_buf_reg;
	}
	dprintk(CVP_DSP, "%s register buffer done\n", __func__);

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

	rc = msm_cvp_unregister_buffer_dsp(inst, kmd_buf);
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

#ifdef FASTRPC_DRIVER_AVAILABLE
	struct fastrpc_device *frpc_device = NULL;
	struct cvp_dsp_fastrpc_driver_entry *frpc_node = NULL;
	struct msm_cvp_list *buf_list = NULL;
#endif

	cmd->ret = 0;

	dprintk(CVP_DSP,
		"%s sess id 0x%x, low 0x%x, high 0x%x, pid 0x%x\n",
		__func__, dsp2cpu_cmd->session_id,
		dsp2cpu_cmd->session_cpu_low,
		dsp2cpu_cmd->session_cpu_high,
		dsp2cpu_cmd->pid);

#ifdef FASTRPC_DRIVER_AVAILABLE
	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s Failed to find fastrpc node 0x%x\n",
				__func__, dsp2cpu_cmd->pid);
		goto fail_fastrpc_node;
	}
	frpc_device = frpc_node->cvp_fastrpc_device;
#endif

	inst = (struct msm_cvp_inst *)ptr_dsp2cpu(
			dsp2cpu_cmd->session_cpu_high,
			dsp2cpu_cmd->session_cpu_low);

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf)
		goto fail_kzalloc_buf;

	rc = cvp_allocate_dsp_bufs(inst, buf,
			dsp2cpu_cmd->sbuf.size,
			dsp2cpu_cmd->sbuf.type);
	if (rc != 0)
		goto fail_allocate_dsp_buf;

#ifdef FASTRPC_DRIVER_AVAILABLE
	rc = eva_fastrpc_dev_map_dma(frpc_device, buf,
			dsp2cpu_cmd->sbuf.dsp_remote_map,
			&v_dsp_addr);
#endif
	if (rc) {
		dprintk(CVP_ERR, "%s Failed to map buffer 0x%x\n", __func__,
			rc);
		goto fail_fastrpc_dev_map_dma;
	}
#ifdef FASTRPC_DRIVER_AVAILABLE
	buf_list = &frpc_node->dspbufs;
	mutex_lock(&buf_list->lock);
	list_add_tail(&buf->list, &buf_list->list);
	mutex_unlock(&buf_list->lock);
#endif
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
	kfree(buf);
fail_kzalloc_buf:
#ifdef FASTRPC_DRIVER_AVAILABLE
fail_fastrpc_node:
#endif
	cmd->ret = -1;
	return;

}

static void __dsp_cvp_mem_free(struct cvp_dsp_cmd_msg *cmd)
{
#ifdef FASTRPC_DRIVER_AVAILABLE
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

	frpc_node = cvp_find_fastrpc_node_with_handle(dsp2cpu_cmd->pid);
	if (!frpc_node) {
		dprintk(CVP_ERR, "%s Failed to find fastrpc node 0x%x\n",
				__func__, dsp2cpu_cmd->pid);
		cmd->ret = -1;
		return;
	}
	frpc_device = frpc_node->cvp_fastrpc_device;

	buf_list = &frpc_node->dspbufs;
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

			kfree(buf);
			break;
		}
	}

fail_release_buf:
fail_fastrpc_dev_unmap_dma:
	mutex_unlock(&buf_list->lock);
#endif
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
		mutex_lock(&me->lock);
		switch (me->pending_dsp2cpu_cmd.type) {
		case DSP2CPU_POWERON:
		{
			if (me->state == DSP_READY) {
				cmd.ret = 0;
				break;
			}

			mutex_unlock(&me->lock);
			old_state = me->state;
			me->state = DSP_READY;
			rc = call_hfi_op(hdev, resume, hdev->hfi_device_data);
			if (rc) {
				dprintk(CVP_WARN, "%s Failed to resume cvp\n",
						__func__);
				mutex_lock(&me->lock);
				me->state = old_state;
				cmd.ret = 1;
				break;
			}
			mutex_lock(&me->lock);
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
		mutex_unlock(&me->lock);
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
}

int cvp_dsp_device_init(void)
{
	struct cvp_dsp_apps *me = &gfa_cv;
	char tname[16];
	int rc;
	int i;

	mutex_init(&me->lock);
	me->state = DSP_INVALID;
	me->hyp_assigned = false;

	for (i = 0; i <= CPU2DSP_MAX_CMD; i++)
		init_completion(&me->completions[i]);

	me->pending_dsp2cpu_cmd.type = CVP_INVALID_RPMSG_TYPE;
	me->pending_dsp2cpu_rsp.type = CVP_INVALID_RPMSG_TYPE;

	INIT_MSM_CVP_LIST(&me->fastrpc_driver_list);

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

	mutex_lock(&me->lock);
	me->state = DSP_INVALID;
	mutex_unlock(&me->lock);

	DEINIT_MSM_CVP_LIST(&me->fastrpc_driver_list);

	for (i = 0; i <= CPU2DSP_MAX_CMD; i++)
		complete_all(&me->completions[i]);

	mutex_destroy(&me->lock);
	unregister_rpmsg_driver(&cvp_dsp_rpmsg_client);
}
