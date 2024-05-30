
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _UNIFORM_REO_CMD_HEADER_H_
#define _UNIFORM_REO_CMD_HEADER_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER 1

struct uniform_reo_cmd_header {
             uint32_t reo_cmd_number                  : 16,
                      reo_status_required             :  1,
                      reserved_0a                     : 15;
};

#define UNIFORM_REO_CMD_HEADER_0_REO_CMD_NUMBER_OFFSET               0x00000000
#define UNIFORM_REO_CMD_HEADER_0_REO_CMD_NUMBER_LSB                  0
#define UNIFORM_REO_CMD_HEADER_0_REO_CMD_NUMBER_MASK                 0x0000ffff

#define UNIFORM_REO_CMD_HEADER_0_REO_STATUS_REQUIRED_OFFSET          0x00000000
#define UNIFORM_REO_CMD_HEADER_0_REO_STATUS_REQUIRED_LSB             16
#define UNIFORM_REO_CMD_HEADER_0_REO_STATUS_REQUIRED_MASK            0x00010000

#define UNIFORM_REO_CMD_HEADER_0_RESERVED_0A_OFFSET                  0x00000000
#define UNIFORM_REO_CMD_HEADER_0_RESERVED_0A_LSB                     17
#define UNIFORM_REO_CMD_HEADER_0_RESERVED_0A_MASK                    0xfffe0000

#endif
