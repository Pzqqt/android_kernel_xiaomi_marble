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
 * DOC: wbuff.c
 * wbuff buffer management APIs
 */

#include <wbuff.h>
#include "i_wbuff.h"

/**
 * Allocation holder array for all wbuff registered modules
 */
struct wbuff_holder wbuff;

/**
 * wbuff_get_pool_slot_from_len() - get pool_slot from length
 * @len: length of the buffer
 *
 * Return: pool slot
 */
static uint8_t wbuff_get_pool_slot_from_len(uint16_t len)
{
	if ((len > 0) && (len <= WBUFF_LEN_POOL0))
		return WBUFF_POOL_0;
	else if ((len > WBUFF_LEN_POOL0) && (len <= WBUFF_LEN_POOL1))
		return WBUFF_POOL_1;
	else if ((len > WBUFF_LEN_POOL1) && (len <= WBUFF_LEN_POOL2))
		return WBUFF_POOL_2;
	else
		return WBUFF_POOL_3;
}

/**
 * wbuff_get_len_from_pool_slot() - get len from pool slot
 * @pool_slot: wbuff pool_slot
 *
 * Return: nbuf length from pool slot
 */
static uint32_t wbuff_get_len_from_pool_slot(uint16_t pool_slot)
{
	uint32_t len = 0;

	switch (pool_slot) {
	case 0:
		len = WBUFF_LEN_POOL0;
		break;
	case 1:
		len = WBUFF_LEN_POOL1;
		break;
	case 2:
		len = WBUFF_LEN_POOL2;
		break;
	case 3:
		len = WBUFF_LEN_POOL3;
		break;
	default:
		len = 0;
	}

	return len;
}

/**
 * wbuff_get_free_mod_slot() - get free module slot
 *
 * Return: module slot
 */
static uint8_t wbuff_get_free_mod_slot(void)
{
	uint8_t mslot = 0;

	for (mslot = 0; mslot < WBUFF_MAX_MODULES; mslot++) {
		qdf_spin_lock_bh(&wbuff.mod[mslot].lock);
		if (!wbuff.mod[mslot].registered) {
			wbuff.mod[mslot].registered = true;
			qdf_spin_unlock_bh(&wbuff.mod[mslot].lock);
			break;
		}
		qdf_spin_unlock_bh(&wbuff.mod[mslot].lock);
	}

	return mslot;
}

/**
 * wbuff_is_valid_alloc_req() - validate alloc  request
 * @req: allocation request from registered module
 * @num: number of pools required
 *
 * Return: true if valid wbuff_alloc_request
 *         false if invalid wbuff_alloc_request
 */
static bool wbuff_is_valid_alloc_req(struct wbuff_alloc_request *req,
				     uint8_t num)
{
	uint16_t psize = 0;
	uint8_t alloc = 0, pslot = 0;

	for (alloc = 0; alloc < num; alloc++) {
		pslot = req[alloc].slot;
		psize = req[alloc].size;
		if ((pslot > WBUFF_MAX_POOLS - 1) ||
		    (psize > wbuff_alloc_max[pslot]))
			return false;
	}

	return true;
}

/**
 * wbuff_prepare_nbuf() - allocate nbuf
 * @mslot: module slot
 * @pslot: pool slot
 * @len: length of the buffer
 * @reserve: nbuf headroom to start with
 * @align: alignment for the nbuf
 *
 * Return: nbuf if success
 *         NULL if failure
 */
static qdf_nbuf_t wbuff_prepare_nbuf(uint8_t mslot, uint8_t pslot,
				     uint32_t len, int reserve, int align)
{
	qdf_nbuf_t buf;
	unsigned long dev_scratch = 0;

	buf = qdf_nbuf_alloc(NULL, roundup(len + reserve, align), reserve,
			     align, false);
	if (!buf)
		return NULL;
	dev_scratch = mslot;
	dev_scratch <<= WBUFF_MSLOT_SHIFT;
	dev_scratch |= ((pslot << WBUFF_PSLOT_SHIFT) | 1);
	qdf_nbuf_set_dev_scratch(buf, dev_scratch);

	return buf;
}

/**
 * wbuff_is_valid_handle() - validate wbuff handle
 * @handle: wbuff handle passed by module
 *
 * Return: true - valid wbuff_handle
 *         false - invalid wbuff_handle
 */
static bool wbuff_is_valid_handle(struct wbuff_handle *handle)
{
	if ((handle) && (handle->id < WBUFF_MAX_MODULES) &&
	    (wbuff.mod[handle->id].registered))
		return true;

	return false;
}

QDF_STATUS wbuff_module_init(void)
{
	struct wbuff_module *mod = NULL;
	uint8_t mslot = 0, pslot = 0;

	if (!qdf_nbuf_is_dev_scratch_supported()) {
		wbuff.initialized = false;
		return QDF_STATUS_E_NOSUPPORT;
	}

	for (mslot = 0; mslot < WBUFF_MAX_MODULES; mslot++) {
		mod = &wbuff.mod[mslot];
		qdf_spinlock_create(&mod->lock);
		for (pslot = 0; pslot < WBUFF_MAX_POOLS; pslot++)
			mod->pool[pslot] = NULL;
		mod->registered = false;
	}
	wbuff.initialized = true;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wbuff_module_deinit(void)
{
	struct wbuff_module *mod = NULL;
	uint8_t mslot = 0;

	if (!wbuff.initialized)
		return QDF_STATUS_E_INVAL;

	wbuff.initialized = false;
	for (mslot = 0; mslot < WBUFF_MAX_MODULES; mslot++) {
		mod = &wbuff.mod[mslot];
		if (mod->registered)
			wbuff_module_deregister((struct wbuff_mod_handle *)
						&mod->handle);
		qdf_spinlock_destroy(&mod->lock);
	}

	return QDF_STATUS_SUCCESS;
}

struct wbuff_mod_handle *
wbuff_module_register(struct wbuff_alloc_request *req, uint8_t num,
		      int reserve, int align)
{
	struct wbuff_module *mod = NULL;
	qdf_nbuf_t buf = NULL;
	uint32_t len = 0;
	uint16_t idx = 0, psize = 0;
	uint8_t alloc = 0, mslot = 0, pslot = 0;

	if (!wbuff.initialized)
		return NULL;

	if ((num == 0) || (num > WBUFF_MAX_POOLS))
		return NULL;

	if (!wbuff_is_valid_alloc_req(req, num))
		return NULL;

	mslot = wbuff_get_free_mod_slot();
	if (mslot == WBUFF_MAX_MODULES)
		return NULL;

	mod = &wbuff.mod[mslot];

	mod->handle.id = mslot;

	for (alloc = 0; alloc < num; alloc++) {
		pslot = req[alloc].slot;
		psize = req[alloc].size;
		len = wbuff_get_len_from_pool_slot(pslot);
		/**
		 * Allocate pool_cnt number of buffers for
		 * the pool given by pslot
		 */
		for (idx = 0; idx < psize; idx++) {
			buf = wbuff_prepare_nbuf(mslot, pslot, len, reserve,
						 align);
			if (!buf)
				continue;
			if (!mod->pool[pslot]) {
				qdf_nbuf_set_next(buf, NULL);
				mod->pool[pslot] = buf;
			} else {
				qdf_nbuf_set_next(buf, mod->pool[pslot]);
				mod->pool[pslot] = buf;
			}
		}
	}
	mod->reserve = reserve;
	mod->align = align;

	return (struct wbuff_mod_handle *)&mod->handle;
}

QDF_STATUS wbuff_module_deregister(struct wbuff_mod_handle *hdl)
{
	struct wbuff_handle *handle;
	struct wbuff_module *mod = NULL;
	uint8_t mslot = 0, pslot = 0;
	qdf_nbuf_t first = NULL, buf = NULL;

	handle = (struct wbuff_handle *)hdl;

	if ((!wbuff.initialized) || (!wbuff_is_valid_handle(handle)))
		return QDF_STATUS_E_INVAL;

	mslot = handle->id;
	mod = &wbuff.mod[mslot];

	qdf_spin_lock_bh(&mod->lock);
	for (pslot = 0; pslot < WBUFF_MAX_POOLS; pslot++) {
		first = mod->pool[pslot];
		while (first) {
			buf = first;
			first = qdf_nbuf_next(buf);
			qdf_nbuf_free(buf);
		}
	}
	mod->registered = false;
	qdf_spin_unlock_bh(&mod->lock);

	return QDF_STATUS_SUCCESS;
}

qdf_nbuf_t wbuff_buff_get(struct wbuff_mod_handle *hdl, uint32_t len,
			  const char *func_name, uint32_t line_num)
{
	struct wbuff_handle *handle;
	struct wbuff_module *mod = NULL;
	uint8_t mslot = 0;
	uint8_t pslot = 0;
	qdf_nbuf_t buf = NULL;

	handle = (struct wbuff_handle *)hdl;

	if ((!wbuff.initialized) || (!wbuff_is_valid_handle(handle)) || !len ||
	    (len > WBUFF_MAX_BUFFER_SIZE))
		return NULL;

	mslot = handle->id;
	pslot = wbuff_get_pool_slot_from_len(len);
	mod = &wbuff.mod[mslot];

	qdf_spin_lock_bh(&mod->lock);
	if (mod->pool[pslot]) {
		buf = mod->pool[pslot];
		mod->pool[pslot] = qdf_nbuf_next(buf);
		mod->pending_returns++;
	}
	qdf_spin_unlock_bh(&mod->lock);
	if (buf) {
		qdf_nbuf_set_next(buf, NULL);
		qdf_net_buf_debug_update_node(buf, func_name, line_num);
	}

	return buf;
}

qdf_nbuf_t wbuff_buff_put(qdf_nbuf_t buf)
{
	qdf_nbuf_t buffer = buf;
	unsigned long slot_info = 0;
	uint8_t mslot = 0, pslot = 0;

	if (!wbuff.initialized)
		return buffer;

	slot_info = qdf_nbuf_get_dev_scratch(buf);
	if (!slot_info)
		return buffer;

	mslot = (slot_info & WBUFF_MSLOT_BITMASK) >> WBUFF_MSLOT_SHIFT;
	pslot = (slot_info & WBUFF_PSLOT_BITMASK) >> WBUFF_PSLOT_SHIFT;
	qdf_nbuf_reset(buffer, wbuff.mod[mslot].reserve, wbuff.mod[mslot].
		       align);
	qdf_spin_lock_bh(&wbuff.mod[mslot].lock);
	if (wbuff.mod[mslot].registered) {
		qdf_nbuf_set_next(buffer, wbuff.mod[mslot].pool[pslot]);
		wbuff.mod[mslot].pool[pslot] = buffer;
		wbuff.mod[mslot].pending_returns--;
		buffer = NULL;
	}
	qdf_spin_unlock_bh(&wbuff.mod[mslot].lock);

	return buffer;
}
