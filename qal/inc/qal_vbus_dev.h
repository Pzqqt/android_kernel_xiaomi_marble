/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
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
 * DOC: qal_vbus_dev
 * QCA abstraction layer (QAL) virtual bus management APIs
 */

#if !defined(__QAL_VBUS_DEV_H)
#define __QAL_VBUS_DEV_H

/* Include Files */
#include <qdf_types.h>
#include <i_qal_vbus_dev.h>

struct qdf_vbus_resource;
struct qdf_vbus_rstctl;
struct qdf_dev_clk;
struct qdf_pfm_hndl;
struct qdf_pfm_drv;

#ifdef ENHANCED_OS_ABSTRACTION
/**
 * qal_vbus_get_iorsc() - acquire io resource
 * @devnum: Device Number
 * @flag: Property bitmap for the io resource
 * @devname: Device name string
 *
 * This function will allocate the io resource for a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_get_iorsc(int devnum, uint32_t flag, char *devname);

/**
 * qdf_vbus_release_iorsc() - release io resource
 * @devnum: Device Number
 *
 * This function will release the io resource attached to a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_release_iorsc(int devnum);

/**
 * qal_vbus_enable_devclk() - enable device clock
 * @clk: Device clock
 *
 * This function will enable the clock for a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_enable_devclk(struct qdf_dev_clk *clk);

/**
 * qal_vbus_disable_devclk() - disable device clock
 * @clk: Device clock
 *
 * This function will disable the clock for a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_disable_devclk(struct qdf_dev_clk *clk);

/**
 * qal_vbus_acquire_dev_rstctl() - get device reset control
 * @pfhndl: Device handle
 * @state: Device state information
 * @rstctl: Device reset control handle
 *
 * This function will acquire the control to reset the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_get_dev_rstctl(struct qdf_pfm_hndl *pfhndl, const char *state,
			struct qdf_vbus_rstctl **rstctl);

/**
 * qal_vbus_release_dev_rstctl() - release device reset control
 * @pfhndl: Device handle
 * @rstctl: Device reset control handle
 *
 * This function will release the control to reset the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_release_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			    struct qdf_vbus_rstctl *rstctl);

/**
 * qal_vbus_activate_dev_rstctl() - activate device reset control
 * @pfhndl: Device handle
 * @rstctl: Device reset control handle
 *
 * This function will activate the reset control for the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_activate_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			     struct qdf_vbus_rstctl *rstctl);

/**
 * qal_vbus_deactivate_dev_rstctl() - deactivate device reset control
 * @pfhndl: Device handle
 * @rstctl: Device reset control handle
 *
 * This function will deactivate the reset control for the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_deactivate_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			       struct qdf_vbus_rstctl *rstctl);

/**
 * qal_vbus_get_resource() - get resource
 * @pfhndl: Device handle
 * @rsc: Resource handle
 * @restype: Resource type
 * @residx: Resource index
 *
 * This function will acquire a particular resource and attach it to the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_get_resource(struct qdf_pfm_hndl *pfhndl,
		      struct qdf_vbus_resource **rsc, uint32_t restype,
		      uint32_t residx);

/**
 * qal_vbus_get_irq() - get irq
 * @pfhndl: Device handle
 * @str: Device identifier
 * @irq: irq number
 *
 * This function will acquire an irq for the device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_get_irq(struct qdf_pfm_hndl *pfhndl, const char *str, int *irq);

/**
 * qal_vbus_register_driver() - register driver
 * @pfdev: Device handle
 *
 * This function will initialize a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_register_driver(struct qdf_pfm_drv *pfdev);

/**
 * qal_vbus_deregister_driver() - deregister driver
 * @pfdev: Device handle
 *
 * This function will deregister the driver for a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_deregister_driver(struct qdf_pfm_drv *pfdev);

/**
 * qal_vbus_gpio_set_value_cansleep() - assign a gpio's value
 * @gpio: gpio whose value will be assigned
 * @value: value to assign
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_gpio_set_value_cansleep(unsigned int gpio, int value);

/**
 * rcu_read_lock() - mark the beginning of an RCU read-side critical section
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_rcu_read_lock(void);

/**
 * rcu_read_unlock() - mark the end of an RCU read-side critical section
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qal_vbus_rcu_read_unlock(void);
#else
static inline QDF_STATUS
qal_vbus_get_iorsc(int devnum, uint32_t flag, char *devname)
{
	return __qal_vbus_get_iorsc(devnum, flag, devname);
}

static inline QDF_STATUS
qal_vbus_release_iorsc(int devnum)
{
	return __qal_vbus_release_iorsc(devnum);
}

static inline QDF_STATUS
qal_vbus_enable_devclk(struct qdf_dev_clk *clk)
{
	return __qal_vbus_enable_devclk(clk);
}

static inline QDF_STATUS
qal_vbus_disable_devclk(struct qdf_dev_clk *clk)
{
	return __qal_vbus_disable_devclk(clk);
}

static inline QDF_STATUS
qal_vbus_get_dev_rstctl(struct qdf_pfm_hndl *pfhndl, const char *state,
			struct qdf_vbus_rstctl **rstctl)
{
	return __qal_vbus_get_dev_rstctl(pfhndl, state, rstctl);
}

static inline QDF_STATUS
qal_vbus_release_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			    struct qdf_vbus_rstctl *rstctl)
{
	return __qal_vbus_release_dev_rstctl(pfhndl, rstctl);
}

static inline QDF_STATUS
qal_vbus_activate_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			     struct qdf_vbus_rstctl *rstctl)
{
	return __qal_vbus_activate_dev_rstctl(pfhndl, rstctl);
}

static inline QDF_STATUS
qal_vbus_deactivate_dev_rstctl(struct qdf_pfm_hndl *pfhndl,
			       struct qdf_vbus_rstctl *rstctl)
{
	return __qal_vbus_deactivate_dev_rstctl(pfhndl, rstctl);
}

static inline QDF_STATUS
qal_vbus_get_resource(struct qdf_pfm_hndl *pfhndl,
		      struct qdf_vbus_resource **rsc, uint32_t restype,
		      uint32_t residx)
{
	return __qal_vbus_get_resource(pfhndl, rsc, restype, residx);
}

static inline QDF_STATUS
qal_vbus_get_irq(struct qdf_pfm_hndl *pfhndl, const char *str, int *irq)
{
	return __qal_vbus_get_irq(pfhndl, str, irq);
}

static inline QDF_STATUS
qal_vbus_register_driver(struct qdf_pfm_drv *pfdev)
{
	return __qal_vbus_register_driver(pfdev);
}

static inline QDF_STATUS
qal_vbus_deregister_driver(struct qdf_pfm_drv *pfdev)
{
	return __qal_vbus_deregister_driver(pfdev);
}

static inline QDF_STATUS
qal_vbus_gpio_set_value_cansleep(unsigned int gpio, int value)
{
	return __qal_vbus_gpio_set_value_cansleep(gpio, value);
}

static inline QDF_STATUS
qal_vbus_rcu_read_lock(void)
{
	return __qal_vbus_rcu_read_lock();
}

static inline QDF_STATUS
qal_vbus_rcu_read_unlock(void)
{
	return __qal_vbus_rcu_read_unlock();
}
#endif

#endif  /* __QAL_VBUS_DEV_H */
