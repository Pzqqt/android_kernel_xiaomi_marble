/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_talloc.h
 *
 * Linux-specific definitions for use by QDF talloc APIs
 */

#ifndef __I_QDF_TALLOC_H
#define __I_QDF_TALLOC_H

#include "asm/page.h"
#include "linux/irqflags.h"
#include "linux/preempt.h"
#include "linux/slab.h"

#define __can_sleep() \
	(!in_interrupt() && !irqs_disabled() && !in_atomic())

#define __zalloc_sleeps(size) kmalloc(size, GFP_KERNEL)
#define __zalloc_atomic(size) kmalloc(size, GFP_ATOMIC)
#define __zalloc_auto(size) \
	kmalloc(size, __can_sleep() ? GFP_KERNEL : GFP_ATOMIC)

#define __free(ptr) kfree(ptr)

#define __alloc_size(ptr) ksize(ptr)

#endif /* __I_QDF_TALLOC_H */

