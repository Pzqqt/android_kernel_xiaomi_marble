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

/**
 * DOC: qld_api.h
 * QLD: This file provides public exposed functions
 */

#ifndef _QLD_API_H_
#define _QLD_API_H_

#define QLD_MAX_NAME    48

/**
 * struct qld_entry - Individual entry in qld_event
 * @addr: Start address of object to dump
 * @size: Size of memory dump
 * @name: Name of memory dump
 */
struct qld_entry {
	uint64_t addr;
	size_t size;
	char name[QLD_MAX_NAME];
};

/**
 * typedef qld_iter_func - qld callback function
 * @req: opaque pointer
 * @qld_entry: qld_entry
 *
 * Return: 0 - OK -EINVAL - On failure
 */
typedef int (*qld_iter_func)(void *req, struct qld_entry *entry);

/**
 * qld_iterate_list() - qld list iteration routine
 * @gen_table: callback function to genrate table
 * @req: opaque request
 *
 * Return: 0 - OK -EINVAL - On failure
 */
int qld_iterate_list(qld_iter_func gen_table, void *req);

/**
 * qld_register() - Register qld for the given address
 * @addr: starting address the dump
 * @size: size of memory to dump
 * @name: name identifier of dump
 *
 * Return: 0 - OK -EINVAL -ENOMEM - On failure
 */
int qld_register(void *addr, size_t size, char *name);

/**
 * qld_unregister() - Un-register qld for the given address
 * @addr: starting address the dump
 *
 * Return: 0 - OK -EINVAL - On failure
 */
int qld_unregister(void *addr);

/**
 * qld_list_init() - Initialize qld list
 * @max_list: maximum size list supports
 *
 * Return: 0 - OK -EINVAL -ENOMEM - On failure
 */
int qld_list_init(uint32_t max_list);

/**
 * qld_list_delete() - empty qld list
 *
 * Return: 0 - OK -EINVAL - On failure
 */
int qld_list_delete(void);

/**
 * qld_list_deinit() - De-initialize qld list
 *
 * Return: 0 - OK -EINVAL - On failure
 */
int qld_list_deinit(void);

/**
 * qld_get_list_count () - get size of qld list
 * @list_count: list_count to set
 *
 * Return: 0 - OK -EINVAL - On failure
 */
int qld_get_list_count(uint32_t *list_count);

/**
 * is_qld_enable() - check if qld feature is set
 *
 * Return: true on success, false on failure
 */
bool is_qld_enable(void);

#endif /* _QLD_API_H_ */
