/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_EDCA__PARAM_H
#define __CFG_MLME_EDCA__PARAM_H

#define STR_EDCA_ANI_ACBK_LOCAL "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_ANI_ACBK_LOCAL_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACBK_LOCAL CFG_STRING( \
		"edca_ani_acbk_local", \
		0, \
		STR_EDCA_ANI_ACBK_LOCAL_LEN, \
		STR_EDCA_ANI_ACBK_LOCAL, \
		"EDCA ANI ACBK LOCAL")

#define STR_EDCA_ANI_ACBE_LOCAL "0x0, 0x2, 0x0, 0xf, 0x3, 0xff, 0x64, 0x0, 0x1f, 0x3, 0xff, 0x64, 0x0, 0xf, 0x3, 0xff, 0x64"
#define STR_EDCA_ANI_ACBE_LOCAL_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACBE_LOCAL CFG_STRING( \
		"edca_ani_acbe_local", \
		0, \
		STR_EDCA_ANI_ACBE_LOCAL_LEN, \
		STR_EDCA_ANI_ACBE_LOCAL, \
		"EDCA ANI ACBE LOCAL")

#define STR_EDCA_ANI_ACVI_LOCAL "0x0, 0x2, 0x0, 0x7, 0x0, 0xf, 0xc8, 0x0, 0xf, 0x0, 0x1f, 0xbc, 0x0, 0x7, 0x0, 0xf, 0xc8"
#define STR_EDCA_ANI_ACVI_LOCAL_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACVI_LOCAL CFG_STRING( \
		"edca_ani_acvi_local",\
		0, \
		STR_EDCA_ANI_ACVI_LOCAL_LEN, \
		STR_EDCA_ANI_ACVI_LOCAL, \
		"EDCA ANI ACVI LOCAL")

#define STR_EDCA_ANI_ACVO_LOCAL "0x0, 0x2, 0x0, 0x3, 0x0, 0x7, 0x64, 0x0, 0x7, 0x0, 0xf, 0x66, 0x0, 0x3, 0x0, 0x7, 0x64"
#define STR_EDCA_ANI_ACVO_LOCAL_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACVO_LOCAL CFG_STRING( \
		"edca_ani_acvo_local", \
		0, \
		STR_EDCA_ANI_ACVO_LOCAL_LEN, \
		STR_EDCA_ANI_ACVO_LOCAL, \
		"EDCA ANI ACVO LOCAL")

#define STR_EDCA_ANI_ACBK "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_ANI_ACBK_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACBK CFG_STRING( \
		"edca_ani_acbk", \
		0, \
		STR_EDCA_ANI_ACBK_LEN, \
		STR_EDCA_ANI_ACBK, \
		"EDCA ANI ACBK BROADCAST")

#define STR_EDCA_ANI_ACBE "0x0, 0x2, 0x0, 0xf, 0x3, 0xff, 0x64, 0x0, 0x1f, 0x3, 0xff, 0x64, 0x0, 0xf, 0x3, 0xff, 0x64"
#define STR_EDCA_ANI_ACBE_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACBE CFG_STRING( \
		"edca_ani_acbe", \
		0, \
		STR_EDCA_ANI_ACBE_LEN, \
		STR_EDCA_ANI_ACBE, \
		"EDCA ANI ACBE BROADCAST")

#define STR_EDCA_ANI_ACVI "0x0, 0x2, 0x0, 0x7, 0x0, 0xf, 0xc8, 0x0, 0xf, 0x0, 0x1f, 0xbc, 0x0, 0x7, 0x0, 0xf, 0xc8"
#define STR_EDCA_ANI_ACVI_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACVI CFG_STRING( \
		"edca_ani_acvi", \
		0, \
		STR_EDCA_ANI_ACVI_LEN, \
		STR_EDCA_ANI_ACVI, \
		"EDCA ANI ACVI BROADCAST")

#define STR_EDCA_ANI_ACVO "0x0, 0x2, 0x0, 0x3, 0x0, 0x7, 0x64, 0x0, 0x7, 0x0, 0xf, 0x66, 0x0, 0x3, 0x0, 0x7, 0x64"
#define STR_EDCA_ANI_ACVO_LEN (sizeof(STR_EDCA_ANI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ANI_ACVO CFG_STRING( \
		"edca_ani_acvo", \
		0, \
		STR_EDCA_ANI_ACVO_LEN, \
		STR_EDCA_ANI_ACVO, \
		"EDCA ANI ACVO BROADCAST")

#define STR_EDCA_WME_ACBK_LOCAL "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_WME_ACBK_LOCAL_LEN (sizeof(STR_EDCA_WME_ACBK_LOCAL) - 1)

#define CFG_EDCA_WME_ACBK_LOCAL CFG_STRING( \
		"edca_wme_acbk_local", \
		0, \
		STR_EDCA_WME_ACBK_LOCAL_LEN, \
		STR_EDCA_WME_ACBK_LOCAL, \
		"EDCA WME ACBK LOCAL")

#define STR_EDCA_WME_ACBE_LOCAL "0x0, 0x3, 0x0, 0xf, 0x0, 0x3f, 0x0, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x0, 0x3f, 0x0"
#define STR_EDCA_WME_ACBE_LOCAL_LEN (sizeof(STR_EDCA_WME_ACBE_LOCAL) - 1)

#define CFG_EDCA_WME_ACBE_LOCAL CFG_STRING( \
		"edca_wme_acbe_local", \
		0, \
		STR_EDCA_WME_ACBE_LOCAL_LEN, \
		STR_EDCA_WME_ACBE_LOCAL, \
		"EDCA WME ACBE LOCAL")

#define STR_EDCA_WME_ACVI_LOCAL "0x0, 0x1, 0x0, 0x7, 0x0, 0xf, 0x5e, 0x0, 0x7, 0x0, 0xf, 0xbc, 0x0, 0x7, 0x0, 0xf, 0x5e"
#define STR_EDCA_WME_ACVI_LOCAL_LEN (sizeof(STR_EDCA_WME_ACVI_LOCAL) - 1)

#define CFG_EDCA_WME_ACVI_LOCAL CFG_STRING( \
		"edca_wme_acvi_local", \
		0, \
		STR_EDCA_WME_ACVI_LOCAL_LEN, \
		STR_EDCA_WME_ACVI_LOCAL, \
		"EDCA WME ACVI LOCAL")

#define STR_EDCA_WME_ACVO_LOCAL "0x0, 0x1, 0x0, 0x3, 0x0, 0x7, 0x2f, 0x0, 0x3, 0x0, 0x7, 0x66, 0x0, 0x3, 0x0, 0x7, 0x2f"
#define STR_EDCA_WME_ACVO_LOCAL_LEN (sizeof(STR_EDCA_WME_ACVO_LOCAL) - 1)

#define CFG_EDCA_WME_ACVO_LOCAL CFG_STRING( \
		"edca_wme_acvo_local", \
		0, \
		STR_EDCA_WME_ACVO_LOCAL_LEN, \
		STR_EDCA_WME_ACVO_LOCAL, \
		"EDCA WME ACVO LOCAL")

#define STR_EDCA_WME_ACBK "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_WME_ACBK_LEN (sizeof(STR_EDCA_WME_ACBK) - 1)

#define CFG_EDCA_WME_ACBK CFG_STRING( \
		"edca_wme_acbk", \
		0, \
		STR_EDCA_WME_ACBK_LEN, \
		STR_EDCA_WME_ACBK, \
		"EDCA WME ACBK BROADCAST")

#define STR_EDCA_WME_ACBE "0x0, 0x3, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_WME_ACBE_LEN (sizeof(STR_EDCA_WME_ACBE) - 1)

#define CFG_EDCA_WME_ACBE CFG_STRING( \
		"edca_wme_acbe", \
		0, \
		STR_EDCA_WME_ACBE_LEN, \
		STR_EDCA_WME_ACBE, \
		"EDCA WME ACBE BROADCAST")

#define STR_EDCA_WME_ACVI "0x0, 0x2, 0x0, 0x7, 0x0, 0xf, 0x5e, 0x0, 0x7, 0x0, 0xf, 0xbc, 0x0, 0x7, 0x0, 0xf, 0x5e"
#define STR_EDCA_WME_ACVI_LEN (sizeof(STR_EDCA_WME_ACVI) - 1)

#define CFG_EDCA_WME_ACVI CFG_STRING( \
		"edca_wme_acvi", \
		0, \
		STR_EDCA_WME_ACVI_LEN, \
		STR_EDCA_WME_ACVI, \
		"EDCA WME ACVI BROADCAST")

#define STR_EDCA_WME_ACVO "0x0, 0x2, 0x0, 0x3, 0x0, 0x7, 0x2f, 0x0, 0x3, 0x0, 0x7, 0x66, 0x0, 0x3, 0x0, 0x7, 0x2f"
#define STR_EDCA_WME_ACVO_LEN (sizeof(STR_EDCA_WME_ACVO) - 1)

#define CFG_EDCA_WME_ACVO CFG_STRING( \
		"edca_wme_acvo", \
		0, \
		STR_EDCA_WME_ACVO_LEN, \
		STR_EDCA_WME_ACVO, \
		"EDCA WME ACVO BROADCAST")

#define STR_EDCA_ETSI_ACBK_LOCAL "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0xbb, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_ETSI_ACBK_LOCAL_LEN (sizeof(STR_EDCA_ETSI_ACBK_LOCAL) - 1)

#define CFG_EDCA_ETSI_ACBK_LOCAL CFG_STRING( \
		"edca_etsi_acbk_local", \
		0, \
		STR_EDCA_ETSI_ACBK_LOCAL_LEN, \
		STR_EDCA_ETSI_ACBK_LOCAL, \
		"EDCA ETSI ACBK LOCAL")

#define STR_EDCA_ETSI_ACBE_LOCAL "0x0, 0x3, 0x0, 0xf, 0x0, 0x3f, 0xbb, 0x0, 0x1f, 0x3, 0xff, 0x0, 0x0, 0xf, 0x0, 0x3f, 0x0"
#define STR_EDCA_ETSI_ACBE_LOCAL_LEN (sizeof(STR_EDCA_ETSI_ACBE_LOCAL) - 1)

#define CFG_EDCA_ETSI_ACBE_LOCAL CFG_STRING( \
		"edca_etsi_acbe_local", \
		0, \
		STR_EDCA_ETSI_ACBE_LOCAL_LEN, \
		STR_EDCA_ETSI_ACBE_LOCAL, \
		"EDCA ETSI ACBE LOCAL")

#define STR_EDCA_ETSI_ACVI_LOCAL "0x0, 0x1, 0x0, 0x7, 0x0, 0xf, 0x7d, 0x0, 0x7, 0x0, 0xf, 0xbc, 0x0, 0x7, 0x0, 0xf, 0x5e"
#define STR_EDCA_ETSI_ACVI_LOCAL_LEN (sizeof(STR_EDCA_ETSI_ACVI_LOCAL) - 1)

#define CFG_EDCA_ETSI_ACVI_LOCAL CFG_STRING( \
		"edca_etsi_acvi_local", \
		0, \
		STR_EDCA_ETSI_ACVI_LOCAL_LEN, \
		STR_EDCA_ETSI_ACVI_LOCAL, \
		"EDCA ETSI ACVI LOCAL")

#define STR_EDCA_ETSI_ACVO_LOCAL "0x0, 0x1, 0x0, 0x3, 0x0, 0x7, 0x3e, 0x0, 0x3, 0x0, 0x7, 0x66, 0x0, 0x3, 0x0, 0x7, 0x2f"
#define STR_EDCA_ETSI_ACVO_LOCAL_LEN (sizeof(STR_EDCA_ETSI_ACVO_LOCAL) - 1)

#define CFG_EDCA_ETSI_ACVO_LOCAL CFG_STRING( \
		"edca_etsi_acvo_local", \
		0, \
		STR_EDCA_ETSI_ACVO_LOCAL_LEN, \
		STR_EDCA_ETSI_ACVO_LOCAL, \
		"EDCA ETSI ACVO LOCAL")

#define STR_EDCA_ETSI_ACBK "0x0, 0x7, 0x0, 0xf, 0x3, 0xff, 0xbb, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_ETSI_ACBK_LEN (sizeof(STR_EDCA_ETSI_ACBK) - 1)

#define CFG_EDCA_ETSI_ACBK CFG_STRING( \
		"edca_etsi_acbk", \
		0, \
		STR_EDCA_ETSI_ACBK_LEN, \
		STR_EDCA_ETSI_ACBK, \
		"EDCA ETSI ACBK BROADCAST")

#define STR_EDCA_ETSI_ACBE "0x0, 0x3, 0x0, 0xf, 0x3, 0xff, 0xbb, 0x0, 0xf, 0x3, 0xff, 0x0, 0x0, 0xf, 0x3, 0xff, 0x0"
#define STR_EDCA_ETSI_ACBE_LEN (sizeof(STR_EDCA_ETSI_ACBE) - 1)

#define CFG_EDCA_ETSI_ACBE CFG_STRING( \
		"edca_etsi_acbe", \
		0, \
		STR_EDCA_ETSI_ACBE_LEN, \
		STR_EDCA_ETSI_ACBE, \
		"EDCA ETSI ACBE BROADCAST")

#define STR_EDCA_ETSI_ACVI "0x0, 0x2, 0x0, 0x7, 0x0, 0xf, 0x7d, 0x0, 0x7, 0x0, 0xf, 0xbc, 0x0, 0x7, 0x0, 0xf, 0x5e"
#define STR_EDCA_ETSI_ACVI_LEN (sizeof(STR_EDCA_ETSI_ACVI) - 1)

#define CFG_EDCA_ETSI_ACVI CFG_STRING( \
		"edca_etsi_acvi", \
		0, \
		STR_EDCA_ETSI_ACVI_LEN, \
		STR_EDCA_ETSI_ACVI, \
		"EDCA ETSI ACVI BROADCAST")

#define STR_EDCA_ETSI_ACVO "0x0, 0x2, 0x0, 0x3, 0x0, 0x7, 0x3e, 0x0, 0x3, 0x0, 0x7, 0x66, 0x0, 0x3, 0x0, 0x7, 0x2f"
#define STR_EDCA_ETSI_ACVO_LEN (sizeof(STR_EDCA_ETSI_ACVO) - 1)

#define CFG_EDCA_ETSI_ACVO CFG_STRING( \
		"edca_etsi_acvo", \
		0, \
		STR_EDCA_ETSI_ACVO_LEN, \
		STR_EDCA_ETSI_ACVO, \
		"EDCA ETSI ACVO BROADCAST")

#define CFG_EDCA_PARAMS_ALL \
	CFG(CFG_EDCA_ANI_ACBK_LOCAL) \
	CFG(CFG_EDCA_ANI_ACBE_LOCAL) \
	CFG(CFG_EDCA_ANI_ACVI_LOCAL) \
	CFG(CFG_EDCA_ANI_ACVO_LOCAL) \
	CFG(CFG_EDCA_ANI_ACBK) \
	CFG(CFG_EDCA_ANI_ACBE) \
	CFG(CFG_EDCA_ANI_ACVI) \
	CFG(CFG_EDCA_ANI_ACVO) \
	CFG(CFG_EDCA_WME_ACBK_LOCAL) \
	CFG(CFG_EDCA_WME_ACBE_LOCAL) \
	CFG(CFG_EDCA_WME_ACVI_LOCAL) \
	CFG(CFG_EDCA_WME_ACVO_LOCAL) \
	CFG(CFG_EDCA_WME_ACBK) \
	CFG(CFG_EDCA_WME_ACBE) \
	CFG(CFG_EDCA_WME_ACVI) \
	CFG(CFG_EDCA_WME_ACVO) \
	CFG(CFG_EDCA_ETSI_ACBK_LOCAL) \
	CFG(CFG_EDCA_ETSI_ACBE_LOCAL) \
	CFG(CFG_EDCA_ETSI_ACVI_LOCAL) \
	CFG(CFG_EDCA_ETSI_ACVO_LOCAL) \
	CFG(CFG_EDCA_ETSI_ACBK) \
	CFG(CFG_EDCA_ETSI_ACBE) \
	CFG(CFG_EDCA_ETSI_ACVI) \
	CFG(CFG_EDCA_ETSI_ACVO)

#endif  /* __CFG_MLME_EDCA__PARAM_H */
