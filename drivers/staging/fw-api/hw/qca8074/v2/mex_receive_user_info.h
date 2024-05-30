/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#pragma once

#include "../mex_tlv_intf.h"
#include "receive_user_info.h"

template <typename TlvType>
struct MexParseTlv<TlvType, typename std::enable_if<std::is_base_of<receive_user_info, TlvType>::value>::type>
    : MexParseTlvBase<TlvType> {

  std::string tlv_name{"receive_user_info"};

  void operator()() {
    MEX_PARSE_TLV_FIELD(phy_ppdu_id);
    MEX_PARSE_TLV_FIELD(user_rssi);
    MEX_PARSE_TLV_FIELD(pkt_type);
    MEX_PARSE_TLV_FIELD(stbc);
    MEX_PARSE_TLV_FIELD(reception_type);
    MEX_PARSE_TLV_FIELD(rate_mcs);
    MEX_PARSE_TLV_FIELD(sgi);
    MEX_PARSE_TLV_FIELD(receive_bandwidth);
    MEX_PARSE_TLV_FIELD(mimo_ss_bitmap);
    MEX_PARSE_TLV_FIELD(ofdma_ru_allocation);
    MEX_PARSE_TLV_FIELD(ofdma_user_index);
    MEX_PARSE_TLV_FIELD(ofdma_content_channel);
    MEX_PARSE_TLV_FIELD(ldpc);
    MEX_PARSE_TLV_FIELD(reserved_2a);
  }
};


