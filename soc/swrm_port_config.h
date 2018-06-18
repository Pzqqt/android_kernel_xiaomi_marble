/* Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _SWRM_PORT_CONFIG
#define _SWRM_PORT_CONFIG

#define WSA_MSTR_PORT_MASK 0xFF

struct port_params wsa_frame_superset[SWR_MSTR_PORT_LEN] = {
		{7, 1, 0},
		{31, 2, 0},
		{63, 12, 31},
		{7, 6, 0},
		{31, 18, 0},
		{63, 13, 31},
		{15, 7, 0},
		{15, 10, 0},
};

#endif /* _SWRM_REGISTERS_H */
