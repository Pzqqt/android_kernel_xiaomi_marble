/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include "qdf_mem.h"
#include "qdf_list.h"
#include "qdf_event.h"
#include "wlan_cfg80211.h"
#include "wlan_osif_request_manager.h"

/* arbitrary value */
#define MAX_NUM_REQUESTS 20

static bool is_initialized;
static qdf_list_t requests;
static qdf_spinlock_t spinlock;
static void *cookie;

struct osif_request {
	qdf_list_node_t node;
	void *cookie;
	uint32_t reference_count;
	struct osif_request_params params;
	qdf_event_t completed;
};

/* must be called with spinlock held */
static void osif_request_unlink(struct osif_request *request)
{
	qdf_list_remove_node(&requests, &request->node);
}

static void osif_request_destroy(struct osif_request *request)
{
	struct osif_request_params *params;

	params = &request->params;
	if (params->dealloc) {
		void *priv = osif_request_priv(request);

		params->dealloc(priv);
	}
	qdf_event_destroy(&request->completed);
	qdf_mem_free(request);
}

/* must be called with spinlock held */
static struct osif_request *osif_request_find(void *cookie)
{
	QDF_STATUS status;
	struct osif_request *request;
	qdf_list_node_t *node;

	status = qdf_list_peek_front(&requests, &node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		request = qdf_container_of(node, struct osif_request, node);
		if (request->cookie == cookie)
			return request;
		status = qdf_list_peek_next(&requests, node, &node);
	}

	return NULL;
}

struct osif_request *osif_request_alloc(const struct osif_request_params *params)
{
	size_t length;
	struct osif_request *request;

	if (!is_initialized) {
		osif_err("invoked when not initialized");
		return NULL;
	}

	length = sizeof(*request) + params->priv_size;
	request = qdf_mem_malloc(length);
	if (!request)
		return NULL;

	request->reference_count = 1;
	request->params = *params;
	qdf_event_create(&request->completed);
	qdf_spin_lock_bh(&spinlock);
	request->cookie = cookie++;
	qdf_list_insert_back(&requests, &request->node);
	qdf_spin_unlock_bh(&spinlock);

	return request;
}

void *osif_request_priv(struct osif_request *request)
{
	/* private data area immediately follows the struct osif_request */
	return request + 1;
}

void *osif_request_cookie(struct osif_request *request)
{
	return request->cookie;
}

struct osif_request *osif_request_get(void *cookie)
{
	struct osif_request *request;

	if (!is_initialized) {
		osif_err("invoked when not initialized");
		return NULL;
	}
	qdf_spin_lock_bh(&spinlock);
	request = osif_request_find(cookie);
	if (request)
		request->reference_count++;
	qdf_spin_unlock_bh(&spinlock);

	return request;
}

void osif_request_put(struct osif_request *request)
{
	bool unlinked = false;

	qdf_spin_lock_bh(&spinlock);
	request->reference_count--;
	if (0 == request->reference_count) {
		osif_request_unlink(request);
		unlinked = true;
	}
	qdf_spin_unlock_bh(&spinlock);
	if (unlinked)
		osif_request_destroy(request);
}

int osif_request_wait_for_response(struct osif_request *request)
{
	QDF_STATUS status;

	status = qdf_wait_for_event_completion(&request->completed,
				       request->params.timeout_ms);

	return qdf_status_to_os_return(status);
}

void osif_request_complete(struct osif_request *request)
{
	(void) qdf_event_set(&request->completed);
}

void osif_request_manager_init(void)
{
	if (is_initialized)
		return;

	qdf_list_create(&requests, MAX_NUM_REQUESTS);
	qdf_spinlock_create(&spinlock);
	is_initialized = true;
}

/*
 * osif_request_manager_deinit implementation note:
 * It is intentional that we do not destroy the list or the spinlock.
 * This allows threads to still access the infrastructure even when it
 * has been deinitialized. Since neither lists nor spinlocks consume
 * resources this does not result in a resource leak.
 */
void osif_request_manager_deinit(void)
{
	is_initialized = false;
}
