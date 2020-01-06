/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

#ifndef _TARGET_TYPE_H_
#define _TARGET_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Header files */
#include "bmi_msg.h"

/* TARGET definition needs to be abstracted in fw common
 * header files, below is the placeholder till WIN codebase
 * moved to latest copy of fw common header files.
 */
/* For Adrastea target */
#define TARGET_TYPE_ADRASTEA  19
#ifndef TARGET_TYPE_QCA8074
#define TARGET_TYPE_QCA8074   20
#endif
#ifndef TARGET_TYPE_QCA6290
#define TARGET_TYPE_QCA6290   21
#endif
#ifndef TARGET_TYPE_QCN7605
#define TARGET_TYPE_QCN7605   22
#endif
#ifndef TARGET_TYPE_QCA6390
#define TARGET_TYPE_QCA6390   23
#endif
#ifndef TARGET_TYPE_QCA8074V2
#define TARGET_TYPE_QCA8074V2 24
#endif
/* For Cypress */
#ifndef TARGET_TYPE_QCA6018
#define TARGET_TYPE_QCA6018      25
#endif
#ifndef TARGET_TYPE_QCN9000
#define TARGET_TYPE_QCN9000      26
#endif
/* HastingsPrime */
#ifndef TARGET_TYPE_QCA6490
#define TARGET_TYPE_QCA6490   27
#endif

/* Moselle */
#ifndef TARGET_TYPE_QCA6750
#define TARGET_TYPE_QCA6750 28
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TARGET_TYPE_H_ */
