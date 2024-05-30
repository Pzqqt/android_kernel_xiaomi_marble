/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_OSIF_REQUEST_MANAGER_H__
#define __WLAN_OSIF_REQUEST_MANAGER_H__

/**
 * DOC: WLAN OSIF REQUEST MANAGER
 *
 * Many operations within the wlan driver occur in an asynchronous
 * manner. Requests are received by OSIF via one of the kernel
 * interfaces (ioctl, nl80211, virtual file system, etc.). The
 * requests are translated to an internal format and are then passed
 * to lower layers, usually via SME, for processing. For requests
 * which require a response, that response comes up from the lower
 * layers in a separate thread of execution, ultimately resulting in a
 * call to a callback function that was provided by OSIF as part of the
 * initial request. So a mechanism is needed to synchronize the
 * request and response. This framework provides that mechanism.
 *
 * Once the framework has been initialized, the typical sequence of
 * events is as follows:
 *
 * Request Thread:
 * 1. Create a &struct osif_request_params which describes the request.
 * 2. Call osif_request_alloc() to allocate a &struct osif_request.
 * 3. Call osif_request_priv() to get a pointer to the private data.
 * 4. Place any information which must be shared with the Response
 *    Callback in the private data area.
 * 5. Call osif_request_cookie() to get the unique cookie assigned
 *    to the request.
 * 6. Call the underlying request handling API, passing the cookie
 *    as the callback's private context.
 * 7. Call osif_request_wait_for_response() to wait for the response
 *    (or for the request to time out).
 * 8. Use the return status to see if the request was successful. If
 *    it was, retrieve any response information from the private
 *    structure and prepare a response for userspace.
 * 9. Call osif_request_put() to relinquish access to the request.
 * 10. Return status to the caller.
 *
 * Response Callback:
 * 1. Call osif_request_get() with the provided cookie to see if the
 *    request structure is still valid.  If it returns %NULL then
 *    return since this means the request thread has already timed
 *    out.
 * 2. Call osif_request_priv() to get access to the private data area.
 * 3. Write response data into the private data area.
 * 4. Call osif_request_complete() to indicate that the response is
 *    ready to be processed by the request thread.
 * 5. Call osif_request_put() to relinquish the callback function's
 *    reference to the request.
 */

/* this is opaque to clients */
struct osif_request;

/**
 * typedef osif_request_dealloc - Private data deallocation function
 */
typedef void (*osif_request_dealloc)(void *priv);

/**
 * struct osif_request_params - OSIF request parameters
 * @priv_size: Size of the private data area required to pass
 *      information between the request thread and the response callback.
 * @timeout_ms: The amount of time to wait for a response in milliseconds.
 * @dealloc: Function to be called when the request is destroyed to
 *      deallocate any allocations made in the private area of the
 *      request struct. Can be %NULL if no private allocations are
 *      made.
 */
struct osif_request_params {
	uint32_t priv_size;
	uint32_t timeout_ms;
	osif_request_dealloc dealloc;
};

/**
 * osif_request_alloc() - Allocate a request struct
 * @params: parameter block that specifies the attributes of the
 *      request
 *
 * This function will attempt to allocate a &struct osif_request with
 * the specified @params. If successful, the caller can then use
 * request struct to make an asynchronous request. Once the request is
 * no longer needed, the reference should be relinquished via a call
 * to osif_request_put().
 *
 * Return: A pointer to an allocated &struct osif_request (which also
 * contains room for the private buffer) if the allocation is
 * successful, %NULL if the allocation fails.
 */
struct osif_request *osif_request_alloc(const struct osif_request_params *params);

/**
 * osif_request_priv() - Get pointer to request private data
 * @request: The request struct that contains the private data
 *
 * This function will return a pointer to the private data area that
 * is part of the request struct. The caller must already have a valid
 * reference to @request from either osif_request_alloc() or
 * osif_request_get().
 *
 * Returns: pointer to the private data area. Note that this pointer
 * will always be an offset from the input @request pointer and hence
 * this function will never return %NULL.
 */
void *osif_request_priv(struct osif_request *request);

/**
 * osif_request_cookie() - Get cookie of a request
 * @request: The request struct associated with the request
 *
 * This function will return the unique cookie that has been assigned
 * to the request. This cookie can subsequently be passed to
 * osif_request_get() to retrieve the request.
 *
 * Note that the cookie is defined as a void pointer as it is intended
 * to be passed as an opaque context pointer from OSIF to underlying
 * layers when making a request, and subsequently passed back to OSIF
 * as an opaque pointer in an asynchronous callback.
 *
 * Returns: The cookie assigned to the request.
 */
void *osif_request_cookie(struct osif_request *request);

/**
 * osif_request_get() - Get a reference to a request struct
 * @cookie: The cookie of the request struct that needs to be
 *      referenced
 *
 * This function will use the cookie to determine if the associated
 * request struct is valid, and if so, will increment the reference
 * count of the struct. This means the caller is guaranteed that the
 * request struct is valid and the underlying private data can be
 * dereferenced.
 *
 * Returns: The pointer to the request struct associated with @cookie
 * if the request is still valid, %NULL if the underlying request
 * struct is no longer valid.
 */
struct osif_request *osif_request_get(void *cookie);

/**
 * osif_request_put() - Release a reference to a request struct
 * @request: The request struct that no longer needs to be referenced
 *
 * This function will decrement the reference count of the struct, and
 * will clean up the request if this is the last reference. The caller
 * must already have a valid reference to @request, either from
 * osif_request_alloc() or osif_request_get().
 *
 * Returns: Nothing
 */
void osif_request_put(struct osif_request *request);

/**
 * osif_request_wait_for_response() - Wait for a response
 * @request: The request struct associated with the request
 *
 * This function will wait until either a response is received and
 * communicated via osif_request_complete(), or until the request
 * timeout period expires.
 *
 * Returns: 0 if a response was received, -ETIMEDOUT if the response
 * timed out.
 */
int osif_request_wait_for_response(struct osif_request *request);

/**
 * osif_request_complete() - Complete a request
 * @request: The request struct associated with the request
 *
 * This function is used to indicate that a response has been received
 * and that any information required by the request thread has been
 * copied into the private data area of the request struct. This will
 * unblock any osif_request_wait_for_response() that is pending on this
 * @request.
 *
 * Returns: Nothing
 */
void osif_request_complete(struct osif_request *request);

/**
 * osif_request_manager_init() - Initialize the OSIF Request Manager
 *
 * This function must be called during system initialization to
 * initialize the OSIF Request Manager.
 *
 * Returns: Nothing
 */
void osif_request_manager_init(void);

/**
 * osif_request_manager_deinit() - Deinitialize the OSIF Request Manager
 *
 * This function must be called during system shutdown to deinitialize
 * the OSIF Request Manager.
 *
 * Returns: Nothing
 */
void osif_request_manager_deinit(void);

#endif /* __WLAN_OSIF_REQUEST_MANAGER_H__ */
