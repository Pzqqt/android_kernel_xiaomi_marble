/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_vfs
 * This file provides OS dependent virtual fiesystem APIs
 */

#include "qdf_vfs.h"
#include "qdf_util.h"
#include "qdf_module.h"
#include <linux/string.h>
#include <linux/kobject.h>

QDF_STATUS
qdf_vfs_set_file_attributes(struct qdf_dev_obj *devobj,
			    struct qdf_vfs_attr *attr)
{
	int ret;

	if (!devobj || !attr)
		return QDF_STATUS_E_INVAL;

	ret = sysfs_create_group((struct kobject *)devobj,
				 (struct attribute_group *)attr);

	return qdf_status_from_os_return(ret);
}

qdf_export_symbol(qdf_vfs_set_file_attributes);

QDF_STATUS
qdf_vfs_clear_file_attributes(struct qdf_dev_obj *devobj,
			      struct qdf_vfs_attr *attr)
{
	if (!devobj || !attr)
		return QDF_STATUS_E_INVAL;

	sysfs_remove_group((struct kobject *)devobj,
			   (struct attribute_group *)attr);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_vfs_clear_file_attributes);

QDF_STATUS
qdf_vfs_create_binfile(struct qdf_dev_obj *devobj, struct qdf_vf_bin_attr *attr)
{
	int ret;

	if (!devobj || !attr)
		return QDF_STATUS_E_INVAL;

	ret = sysfs_create_bin_file((struct kobject *)devobj,
				    (struct bin_attribute *)attr);

	return qdf_status_from_os_return(ret);
}

qdf_export_symbol(qdf_vfs_create_binfile);

QDF_STATUS
qdf_vfs_delete_binfile(struct qdf_dev_obj *devobj, struct qdf_vf_bin_attr *attr)
{
	if (!devobj || !attr)
		return QDF_STATUS_E_INVAL;

	sysfs_remove_bin_file((struct kobject *)devobj,
			      (struct bin_attribute *)attr);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_vfs_delete_binfile);
