/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: mlo_global_h_shmem_arena.h
 *  This file contains APIs that are exposed by the the MLO
 *  global shared memory arena.
 */

#include<qdf_types.h>

#ifndef _MLO_GLOBAL_H_SHMEM_ARENA_API_H_
#define _MLO_GLOBAL_H_SHMEM_ARENA_API_H_
/**
 * wlan_host_mlo_glb_h_shmem_arena_ctx_init() - Initialize MLO Global shared
 * memory arena context on Host
 * @arena_vaddr: Virtual address of the MLO Global shared memory arena
 * @arena_len: Length (in bytes) of the MLO Global shared memory arena
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
wlan_host_mlo_glb_h_shmem_arena_ctx_init(void *arena_vaddr,
					 size_t arena_len);

/**
 * wlan_host_mlo_glb_h_shmem_arena_deinit() - De-initialize MLO Global shared
 * memory arena context on Host
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS wlan_host_mlo_glb_h_shmem_arena_ctx_deinit(void);
#endif
