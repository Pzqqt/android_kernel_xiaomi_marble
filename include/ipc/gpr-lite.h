/* Copyright (c) 2011-2017, 2019-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2018, Linaro Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __QCOM_GPR_H_
#define __QCOM_GPR_H_

#include <linux/spinlock.h>
#include <linux/device.h>
#include "soc/audio_mod_devicetable.h"
#include <dt-bindings/sound/qcom,gpr.h>

extern struct bus_type gprbus;

enum gpr_subsys_state {
        GPR_SUBSYS_DOWN,
        GPR_SUBSYS_UP,
        GPR_SUBSYS_LOADED,
};


struct gpr_q6 {
 //void *pil;
 atomic_t q6_state;
 atomic_t modem_state;
 struct mutex lock;
};


/* Version */
#define GPR_PKT_VER		0x0

/*
 * Header format
 * Bits 3-0  : version
 * Bits 7-4  : header_size
 * Bits 31-8 : packet_size
 */

#define GPR_PKT_INIT_PORT_V ( ( uint32_t ) 0 )

#define GPR_PKT_INIT_RESERVED_V ( ( uint16_t ) 0 )

#define GPR_PKT_INIT_DOMAIN_ID_V ( ( uint8_t ) 0 )

#define GPR_IDS_DOMAIN_ID_MODEM_V    ( 1 ) /**< Modem domain. */
#define GPR_IDS_DOMAIN_ID_ADSP_V     ( 2 ) /**< aDSP domain. */
#define GPR_IDS_DOMAIN_ID_APPS_V     ( 3 ) /**< Application domain. */

/* V2.0 header size in number of 32-bit words. */
#define GPR_PKT_HEADER_WORD_SIZE_V ( ( sizeof( struct gpr_pkt ) + 3 ) >> 2 )

/* V2.0 header size in number of bytes. */
#define GPR_PKT_HEADER_BYTE_SIZE_V ( GPR_PKT_HEADER_WORD_SIZE_V << 2 )

/* Undefined value for where a valid GUID is expected. */
#define GPR_UNDEFINED_ID_V ( ( uint32_t ) 0xFFFFFFFF )

#define GPR_PKT_INIT_CLIENT_DATA_V ( ( uint8_t ) 0 )

/***********************************************************/
/**     GPRv2 Packet Bitfield Macros                     ***/
/***********************************************************/

/* Bitmask and bit shift of the version field. */
#define GPR_PKT_VERSION_MASK ( 0x0000000F )
#define GPR_PKT_VERSION_SHFT ( 0 )

/* Bitmask and bit shift of the header size field. */
#define GPR_PKT_HEADER_SIZE_MASK ( 0x000000F0 )
#define GPR_PKT_HEADER_SIZE_SHFT ( 4 )

/* Bitmask and bit shift of the reserved field.
 * Includes 4 reserved bits from client data field */
#define GPR_PKT_RESERVED_MASK ( 0xFFF00000 )
#define GPR_PKT_RESERVED_SHFT ( 20 )

/* Bitmask and bit shift of the packet size field. */
#define GPR_PKT_PACKET_SIZE_MASK ( 0xFFFFFF00 )
#define GPR_PKT_PACKET_SIZE_SHFT ( 8 )

/**********************************************************************/
/**       Packet Helper Macros                                      ***/
/**********************************************************************/

/* Gets the value of a field, including the specified mask and shift. */
#define GPR_GET_BITMASK( mask, shift, value ) \
  ( ( ( value ) & ( mask ) ) >> ( shift ) )

/* Sets a value in a field with a specified mask and shift. */
#define GPR_SET_BITMASK( mask, shift, value ) \
  ( ( ( value ) << ( shift ) ) & ( mask ) )

/* Gets the value of a field. */
#define GPR_GET_FIELD( field, value ) \
  GPR_GET_BITMASK( ( field##_MASK ), ( field##_SHFT ), ( value ) )

/* Sets a value in a field. */
#define GPR_SET_FIELD( field, value ) \
  GPR_SET_BITMASK( ( field##_MASK ), ( field##_SHFT ), ( value ) )

/* Returns an 8-bit aligned pointer to a base address pointer plus an
   offset, in bytes.*/
#define GPR_PTR_END_OF( base_ptr, offset ) \
  ( ( ( uint8_t* ) base_ptr ) + ( offset ) )

/** Given the packet header, returns the packet's current size in bytes.
    The current packet byte size is the sum of the base packet structure and
    the used portion of the payload.
*/
#define GPR_PKT_GET_PACKET_BYTE_SIZE( header ) \
  ( GPR_GET_FIELD( GPR_PKT_PACKET_SIZE, header ) )

/* Given the packet header, returns the header's current size in bytes. */
#define GPR_PKT_GET_HEADER_BYTE_SIZE( header ) \
  ( GPR_GET_FIELD( GPR_PKT_HEADER_SIZE, header ) << 2 )

/** Given the packet header, returns the payload's current size in bytes.
    The current payload byte size is the difference between the packet size and
    the header size.
*/
#define GPR_PKT_GET_PAYLOAD_BYTE_SIZE( header ) \
  ( GPR_PKT_GET_PACKET_BYTE_SIZE( header ) - \
    GPR_PKT_GET_HEADER_BYTE_SIZE( header ) )

/** Given the packet, returns a pointer to the beginning of the packet's
    payload. */
#define GPR_PKT_GET_PAYLOAD( type, packet_ptr ) \
  ( ( type* ) GPR_PTR_END_OF( packet_ptr, \
                GPR_PKT_GET_HEADER_BYTE_SIZE( \
                  ( ( struct gpr_hdr* ) packet_ptr )->header ) ) )

#define GPR_HDR_FIELD(msg_type, hdr_len, ver)\
	(((msg_type & 0x3) << 8) | ((hdr_len & 0xF) << 4) | (ver & 0xF))

#define GPR_HDR_SIZE sizeof(struct gpr_hdr)
#define GPR_SEQ_CMD_HDR_FIELD GPR_HDR_FIELD(GPR_MSG_TYPE_SEQ_CMD, \
					    GPR_HDR_LEN(GPR_HDR_SIZE), \
					    GPR_PKT_VER)

/**********************************************************************/
/**       SPF GUID definitions                                    ***/
/**********************************************************************/
/* An empty macro to mark non-GUIDs so that GUID script doesn't mistake
   the ID for a GUID */
#define MM_NON_GUID(x)                    x

/* GUID owner is QTI. */
#define MM_GUID_OWNER_QTI                  0x0

/* GUID owner is not-QTI. */
#define MM_GUID_OWNER_NON_QTI              0x1

/* Mask and Shift for Owner */
#define MM_GUID_OWNER_MASK                MM_NON_GUID(0xF0000000)
#define MM_GUID_OWNER_SHIFT               28

/************************************************/
/*       GUID types                           ***/
/************************************************/

/* Reserved */
#define MM_GUID_TYPE_RESERVED             0x0

/* Control commands */
#define MM_GUID_TYPE_CONTROL_CMD          0x1

/* Responses of control commands */
#define MM_GUID_TYPE_CONTROL_CMD_RSP      0x2

/* Control events */
#define MM_GUID_TYPE_CONTROL_EVENT        0x3

/* Data commands */
#define MM_GUID_TYPE_DATA_CMD             0x4

/* Data command responses */
#define MM_GUID_TYPE_DATA_CMD_RSP         0x5

/* Data events */
#define MM_GUID_TYPE_DATA_EVENT           0x6

/* Module-id */
#define MM_GUID_TYPE_MODULE_ID            0x7

/* Module's param & event-id */
#define MM_GUID_TYPE_PARAM_EVENT_ID       0x8

/* Media format-id. */
#define MM_GUID_TYPE_FORMAT_ID            0x9

/* CAPI */
#define MM_GUID_TYPE_CAPI                 0xA

/* MAX */
#define MM_GUID_TYPE_MAX                  0xB
/** Rest are reserved */

/* Zero is an invalid ID.
 * To be used like NULL value for pointers */
#define MM_GUID_INVALID                   0

/* Mask and shift for GUID type */
#define MM_GUID_TYPE_MASK                 MM_NON_GUID(0x0F000000)
#define MM_GUID_TYPE_SHIFT                24

struct gpr_hdr {
        uint32_t header;
        uint8_t dst_domain_id;
        uint8_t src_domain_id;
        uint8_t client_data;
        uint8_t reserved;
        uint32_t src_port;
        uint32_t dst_port;
        uint32_t token;
        uint32_t opcode;
} __packed;

struct gpr_pkt {
       struct gpr_hdr hdr;
       uint8_t payload[0];
};

#define GPR_IBASIC_RSP_RESULT 0x02001005

/* Bits 0 to 15 -- Minor version,  Bits 16 to 31 -- Major version */
#define GPR_SVC_MAJOR_VERSION(v)	((v >> 16) & 0xFF)
#define GPR_SVC_MINOR_VERSION(v)	(v & 0xFF)

struct gpr_device {
	struct device	dev;
	uint16_t	svc_id;
	uint16_t	domain_id;
	uint32_t	version;
	char name[GPR_NAME_SIZE];
	spinlock_t	lock;
	struct list_head node;
};

#define to_gpr_device(d) container_of(d, struct gpr_device, dev)

struct gpr_driver {
	int	(*probe)(struct gpr_device *sl);
	int	(*remove)(struct gpr_device *sl);
	int	(*callback)(struct gpr_device *a,
			    void *d);
	struct device_driver		driver;
	const struct gpr_device_id	*id_table;
};

#define to_gpr_driver(d) container_of(d, struct gpr_driver, driver)

/*
 * use a macro to avoid include chaining to get THIS_MODULE
 */
#define gpr_driver_register(drv) __gpr_driver_register(drv, THIS_MODULE)

int __gpr_driver_register(struct gpr_driver *drv, struct module *owner);
void gpr_driver_unregister(struct gpr_driver *drv);

/**
 * module_gpr_driver() - Helper macro for registering a gprbus driver
 * @__gprbus_driver: gprbus_driver struct
 *
 * Helper macro for gprbus drivers which do not do anything special in
 * module init/exit. This eliminates a lot of boilerplate. Each module
 * may only use this macro once, and calling it replaces module_init()
 * and module_exit()
 */
#define module_gpr_driver(__gpr_driver) \
	module_driver(__gpr_driver, gpr_driver_register, \
			gpr_driver_unregister)

int gpr_send_pkt(struct gpr_device *adev, struct gpr_pkt *pkt);

enum gpr_subsys_state gpr_get_modem_state(void);
#endif /* __QCOM_GPR_H_ */
