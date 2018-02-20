 /*
  * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
  * DOC: Public Data Structures to perform debug operations
  * on object manager
  */

#ifndef _WLAN_OBJMGR_DEBUG_H_
#define _WLAN_OBJMGR_DEBUG_H_

#include <qdf_timer.h>

#ifdef WLAN_OBJMGR_DEBUG

/**
 * wlan_objmgr_notify_log_delete()- insert
 *  logically deleted object into list
 * @obj:           object to be inserted
 * @obj_type:      type of object to be inserted
 *
 * Return: void
 */
void wlan_objmgr_notify_log_delete(void *obj,
				   enum wlan_objmgr_obj_type obj_type);

/**
 * wlan_objmgr_notify_destroy() - remove
 *  logically deleted object from list
 * @obj:           object to be removed
 * @obj_type:      type of object to be removed
 *
 * Return: void
 */
void wlan_objmgr_notify_destroy(void *obj,
				enum wlan_objmgr_obj_type obj_type);

/**
 * wlan_objmgr_debug_info_init() - initialize
 *  the logically deleted list object
 * Caller need to protect with global object lock
 *
 * Return: void
 */
void wlan_objmgr_debug_info_init(void);

/**
 * wlan_objmgr_debug_info_deinit() - deinitialize
 *  the logically deleted list object
 *
 * Return: void
 */
void wlan_objmgr_debug_info_deinit(void);


#else

static inline void
wlan_objmgr_notify_log_delete(void *obj,
			      enum wlan_objmgr_obj_type obj_type)
{
}

static inline void
wlan_objmgr_notify_destroy(void *obj,
			   enum wlan_objmgr_obj_type obj_typ)
{
}

static inline void
wlan_objmgr_debug_info_init(void)
{
}

static inline void
wlan_objmgr_debug_info_deinit(void)
{
}

#endif /*WLAN_OBJMGR_DEBUG*/

#endif /*_WLAN_OBJMGR_DEBUG_H_*/
