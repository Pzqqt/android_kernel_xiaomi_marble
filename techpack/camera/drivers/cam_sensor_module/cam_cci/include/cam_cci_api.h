/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#ifndef _CAM_CCI_API_H_
#define _CAM_CCI_API_H_

#include <media/v4l2-subdev.h>
#include "cam_cci_dev.h"

/**
 * cam_cci_client_ops()
 *
 * @brief:      Direct kernel-internal API for CCI operations.
 *              Calls cam_cci_core_cfg() directly, bypassing the ioctl
 *              dispatch path and video_usercopy entirely.
 *
 * @sd:         V4L2 sub-device pointer for the CCI subdev
 * @cmd:        CCI command identifier
 * @cci_ctrl:   Pointer to a kernel-owned cam_cci_ctrl struct
 */

int32_t cam_cci_client_ops(struct v4l2_subdev *sd, unsigned int cmd,
	struct cam_cci_ctrl *cci_ctrl);

#endif /* _CAM_CCI_API_H_ */
