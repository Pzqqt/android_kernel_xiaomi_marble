/* SPDX-License-Identifier: BSD-2-Clause */
// LZ4 compatibility wrapper for Linux kernel

#ifndef __LINUX_LZ4_H__
#define __LINUX_LZ4_H__

#include "../../lib/lz4/lz4.h"
#include "../../lib/lz4/lz4hc.h"

#define LZ4_MEM_COMPRESS	LZ4_STREAM_MINSIZE
#define LZ4HC_MEM_COMPRESS	LZ4_STREAMHC_MINSIZE

#define LZ4HC_MIN_CLEVEL	LZ4HC_CLEVEL_MIN
#define LZ4HC_MAX_CLEVEL	LZ4HC_CLEVEL_MAX

#endif
