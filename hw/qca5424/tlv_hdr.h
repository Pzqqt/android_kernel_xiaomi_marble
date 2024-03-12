
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 

#ifndef _TLV_HDR_H_
#define _TLV_HDR_H_
#if !defined(__ASSEMBLER__)
#endif

#define _TLV_USERID_WIDTH_      6
#define _TLV_DATA_WIDTH_        32
#define _TLV_TAG_WIDTH_         9

#define _TLV_MRV_EN_LEN_WIDTH_  9
#define _TLV_MRV_DIS_LEN_WIDTH_ 12

#define _TLV_16_DATA_WIDTH_     16
#define _TLV_16_TAG_WIDTH_      5
#define _TLV_16_LEN_WIDTH_      4
#define _TLV_CTAG_WIDTH_        5
#define _TLV_44_DATA_WIDTH_     44
#define _TLV_64_DATA_WIDTH_     64
#define _TLV_76_DATA_WIDTH_     64
#define _TLV_CDATA_WIDTH_       32
#define _TLV_CDATA_76_WIDTH_    64

struct tlv_usr_16_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint16_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_16_TAG_WIDTH_,
                                tlv_len             :   _TLV_16_LEN_WIDTH_,
                                tlv_usrid           :   _TLV_USERID_WIDTH_;
#else
           uint16_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_len             :   _TLV_16_LEN_WIDTH_,
                                tlv_tag             :   _TLV_16_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

struct tlv_16_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint16_t             tlv_cflg_reserved   :   1,
                                tlv_len             :   _TLV_16_LEN_WIDTH_,
                                tlv_tag             :   _TLV_16_TAG_WIDTH_,
                                tlv_reserved        :   6;
#else
           uint16_t             tlv_reserved        :   6,
                                tlv_tag             :   _TLV_16_TAG_WIDTH_,
                                tlv_len             :   _TLV_16_LEN_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

 
 
 
 
 

struct tlv_mlo_usr_32_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

struct tlv_mlo_32_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   6;
#else
           uint32_t             tlv_reserved        :   6,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

struct tlv_mlo_usr_64_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
#endif
                                tlv_reserved        :   32;
};

struct tlv_mlo_64_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   38;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
                                tlv_reserved        :   32;
#endif
};

struct tlv_mlo_usr_44_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_reserved        :   10,
                                pad_44to64_bit      :   22;
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                pad_44to64_bit      :   22,
                                tlv_reserved        :   10;
#endif
};

struct tlv_mlo_44_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   16,
                                pad_44to64_bit      :   22;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                pad_44to64_bit      :   22,
                                tlv_reserved        :   10;
#endif
};

struct tlv_mlo_usr_76_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
#endif
                                tlv_reserved        :   32;
           uint64_t             pad_64to128_bit     :   64;
};

struct tlv_mlo_76_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   38;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                tlv_reserved        :   32;
#endif
           uint64_t             pad_64to128_bit     :   64;
};






struct tlv_mac_usr_32_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

struct tlv_mac_32_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   6;
#else
           uint32_t             tlv_reserved        :   6,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
};

struct tlv_mac_usr_64_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
#endif
                                tlv_reserved        :   32;
};

struct tlv_mac_64_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   38;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
                                tlv_reserved        :   32;
#endif
};

struct tlv_mac_usr_44_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_reserved        :   10,
                                pad_44to64_bit      :   22;
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                pad_44to64_bit      :   22,
                                tlv_reserved        :   10;
#endif
};

struct tlv_mac_44_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   16,
                                pad_44to64_bit      :   22;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                pad_44to64_bit      :   22,
                                tlv_reserved        :   10;
#endif
};

struct tlv_mac_usr_76_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
#else
           uint64_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
#endif
                                tlv_reserved        :   32;
           uint64_t             pad_64to128_bit     :   64;
};

struct tlv_mac_76_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   38;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_compression     :   1,
                                tlv_reserved        :   32;
#endif
           uint64_t             pad_64to128_bit     :   64;
};

 
 
 

struct tlv_usr_c_44_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_cdata           :   _TLV_CDATA_WIDTH_,
                                pad_44to64_bit      :   20;
#else
           uint64_t             tlv_cdata_lower_20  :   20,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_compression     :   1,
                                pad_44to64_bit      :   20,
                                tlv_cdata_upper_12  :   12;
#endif
};

struct tlv_usr_c_76_tlword_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_compression     :   1,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_cdata_lower_52  :   52; 
           uint64_t             tlv_cdata_upper_12  :   12,
                                pad_76to128_bit     :   52;
#else
           uint64_t             tlv_cdata_lower_20  :   20,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_compression     :   1,
                                tlv_cdata_middle_32 :   32;
           uint64_t             pad_76to96_bit      :   20,
                                tlv_cdata_upper_12  :   12,
                                pad_96to128_bit     :   32;
#endif
};


 
 
 
 
 
struct tlv_usr_32_hdr {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
#else
           uint32_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
#endif
                                tlv_reserved        :   32;
};

struct tlv_32_hdr {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint64_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   38;
#else
           uint64_t             tlv_usrid_reserved  :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1,
                                tlv_reserved        :   32;
#endif
};
 

 
 
 
 
 

struct tlv_mlo_usr_64_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
           uint32_t             pad_32to64_bit      :   32;
};

struct tlv_mlo_64_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_dst_linkid      :   3,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_reserved        :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_dst_linkid      :   3,
                                tlv_len             :   _TLV_MRV_EN_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
           uint32_t             pad_32to64_bit      :   32;
};

struct tlv_mac_usr_64_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_usrid           :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
           uint32_t             pad_32to64_bit      :   32;
};

struct tlv_mac_64_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_cflg_reserved   :   1,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_src_linkid      :   3,
                                tlv_mrv             :   1,
                                tlv_reserved        :   _TLV_USERID_WIDTH_;
#else
           uint32_t             tlv_reserved        :   _TLV_USERID_WIDTH_,
                                tlv_mrv             :   1,
                                tlv_src_linkid      :   3,
                                tlv_len             :   _TLV_MRV_DIS_LEN_WIDTH_,
                                tlv_tag             :   _TLV_TAG_WIDTH_,
                                tlv_cflg_reserved   :   1;
#endif
           uint32_t             pad_32to64_bit      :   32;
};

 
 
 

struct tlv_usr_c_44_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_compression     :   1,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_cdata_lower_20  :   20;
           uint32_t             tlv_cdata_upper_12  :   12,
                                pad_44to64_bit      :   20;
#else
           uint32_t             tlv_cdata_lower_20  :   20,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_compression     :   1;
           uint32_t             pad_44to64_bit      :   20,
                                tlv_cdata_upper_12  :   12;
#endif
};

struct tlv_usr_c_76_tlw32_t {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
           uint32_t             tlv_compression     :   1,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_cdata_lower_20  :   20;
           uint32_t             tlv_cdata_middle_32 :   32;
           uint32_t             tlv_cdata_upper_12  :   12,
                                pad_76to96_bit      :   20;
           uint32_t             pad_96to128_bit     :   32;
#else
           uint32_t             tlv_cdata_lower_20  :   20,
                                tlv_usrid           :   _TLV_USERID_WIDTH_,
                                tlv_ctag            :   _TLV_CTAG_WIDTH_,
                                tlv_compression     :   1;
           uint32_t             tlv_cdata_middle_32 :   32;
           uint32_t             pad_76to96_bit      :   20,
                                tlv_cdata_upper_12  :   12;
           uint32_t             pad_96to128_bit     :   32;
#endif
};
 


#endif  
