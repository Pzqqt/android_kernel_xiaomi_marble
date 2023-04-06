
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



#ifndef _TLV_HDR_H_

#define _TLV_HDR_H_

#if !defined(__ASSEMBLER__)

#endif

struct tlv_usr_16_hdr {

   volatile uint16_t             tlv_cflg_reserved   :   1,

                                 tlv_tag             :   5,

                                 tlv_len             :   4,

                                 tlv_usrid           :   6;

};

struct tlv_16_hdr {

   volatile uint16_t             tlv_cflg_reserved   :   1,

                                 tlv_tag             :   5,

                                 tlv_len             :   4,

                                 tlv_reserved        :   6;

};

struct tlv_usr_32_hdr {

   volatile uint32_t             tlv_cflg_reserved   :   1,

                                 tlv_tag             :   9,

                                 tlv_len             :  16,

                                 tlv_usrid           :   6;

};

struct tlv_32_hdr {

   volatile uint32_t             tlv_cflg_reserved   :   1,

                                 tlv_tag             :   9,

                                 tlv_len             :  16,

                                 tlv_reserved        :   6;

};

struct tlv_usr_42_hdr {

   volatile uint64_t             tlv_compression     :   1,

                                 tlv_tag             :   9,

                                 tlv_len             :  16,

                                 tlv_usrid           :   6,

                                 tlv_reserved        :  10,

                                 pad_42to64_bit      :  22;

};

struct tlv_42_hdr {

   volatile uint64_t             tlv_compression     :   1,

                                 tlv_tag             :   9,

                                 tlv_len             :  16,

                                 tlv_reserved        :  16,

                                 pad_42to64_bit      :  22;

};

struct tlv_usr_c_42_hdr {

   volatile uint64_t             tlv_compression     :   1,

                                 tlv_ctag            :   3,

                                 tlv_usrid           :   6,

                                 tlv_cdata           :  32,

                                 pad_42to64_bit      :  22;

};

#endif

