/*
 * Copyright (c) 2013-2014, 2016 The Linux Foundation. All rights reserved.
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
 * Every Product Line or chipset or team can have its own Whitelist table.
 * The following is a list of versions that the present software can support
 * even though its versions are incompatible. Any entry here means that the
 * indicated version does not break WMI compatibility even though it has
 * a minor version change.
 */
#ifndef _WMI_VERSION_WHITELIST_H_
#define _WMI_VERSION_WHITELIST_H_
static wmi_whitelist_version_info version_whitelist[] = {
	{0, 0, 0x5F414351, 0x00004C4D, 0, 0}
	/* Placeholder: Major=0, Minor=0, Namespace="QCA_ML" (Dummy entry) */
};
#endif /* _WMI_VERSION_WHITELIST_H_ */
