/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

/*
 *
 * This file lim_ser_des_utils.cc contains the serializer/deserializer
 * utility functions LIM uses while communicating with upper layer
 * software entities
 * Author:        Chandra Modumudi
 * Date:          10/20/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "ani_system_defs.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_ser_des_utils.h"


/**---------------------------------------------------------------
   \fn     lim_get_session_info
   \brief  This function returns the sessionId and transactionId
 \       of a message. This assumes that the message structure
 \       is of format:
 \          uint16_t   messageType
 \          uint16_t   messageLength
 \          uint8_t    sessionId
 \          uint16_t   transactionId
   \param  mac          - mac global structure
   \param  *pBuf         - pointer to the message buffer
   \param  sessionId     - returned session id value
   \param  transactionId - returned transaction ID value
   \return None
   ------------------------------------------------------------------*/
void
lim_get_session_info(struct mac_context *mac, uint8_t *pBuf, uint8_t *sessionId,
		     uint16_t *transactionId)
{
	if (!pBuf) {
		pe_err("NULL ptr received");
		return;
	}

	pBuf += sizeof(uint16_t);       /* skip message type */
	pBuf += sizeof(uint16_t);       /* skip message length */

	*sessionId = *pBuf;     /* get sessionId */
	pBuf++;
	*transactionId = lim_get_u16(pBuf);       /* get transactionId */

	return;
}
