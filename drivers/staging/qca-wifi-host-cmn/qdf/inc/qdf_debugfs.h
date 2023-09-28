/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_debugfs.h
 * This file provides OS abstraction for debug filesystem APIs.
 */

#ifndef _QDF_DEBUGFS_H
#define _QDF_DEBUGFS_H

#include <qdf_status.h>
#include <i_qdf_debugfs.h>
#include <qdf_atomic.h>
#include <qdf_types.h>

/* representation of qdf dentry */
typedef __qdf_dentry_t qdf_dentry_t;
typedef __qdf_debugfs_file_t qdf_debugfs_file_t;
typedef __qdf_debugfs_blob_wrap_t qdf_debugfs_blob_wrap_t;
typedef __qdf_entry_t qdf_entry_t;
typedef __qdf_file_ops_t qdf_file_ops_t;

/* qdf file modes */
#define QDF_FILE_USR_READ	00400
#define QDF_FILE_USR_WRITE	00200

#define QDF_FILE_GRP_READ	00040
#define QDF_FILE_GRP_WRITE	00020

#define QDF_FILE_OTH_READ	00004
#define QDF_FILE_OTH_WRITE	00002

/**
 * struct qdf_debugfs_fops - qdf debugfs operations
 * @show: Callback for show operation.
 *	Following functions can be used to print data in the show function,
 *	qdf_debugfs_print()
 *	qdf_debugfs_hexdump()
 *	qdf_debugfs_write()
 * @write: Callback for write operation.
 * @priv: Private pointer which will be passed in the registered callbacks.
 */
struct qdf_debugfs_fops {
	QDF_STATUS(*show)(qdf_debugfs_file_t file, void *arg);
	QDF_STATUS(*write)(void *priv, const char *buf, qdf_size_t len);
	void *priv;
};

#ifdef WLAN_DEBUGFS
/**
 * qdf_debugfs_init() - initialize debugfs
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_debugfs_init(void);

/**
 * qdf_debugfs_exit() - cleanup debugfs
 *
 * Return: None
 */
void qdf_debugfs_exit(void);

/**
 * qdf_debugfs_create_dir() - create a debugfs directory
 * @name: name of the new directory
 * @parent: parent node. If NULL, defaults to base qdf_debugfs_root
 *
 * Return: dentry structure pointer in case of success, otherwise NULL.
 *
 */
qdf_dentry_t qdf_debugfs_create_dir(const char *name, qdf_dentry_t parent);

/**
 * qdf_debugfs_create_file() - create a debugfs file
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base qdf_debugfs_root
 * @fops: file operations { .read, .write ... }
 *
 * Return: dentry structure pointer in case of success, otherwise NULL.
 *
 */
qdf_dentry_t qdf_debugfs_create_file(const char *name, uint16_t mode,
				     qdf_dentry_t parent,
				     struct qdf_debugfs_fops *fops);

/**
 * qdf_debugfs_printf() - print formated string into debugfs file
 * @file: debugfs file handle passed in fops->show() function
 * @f: the format string to use
 * @...: arguments for the format string
 */
void qdf_debugfs_printf(qdf_debugfs_file_t file, const char *f, ...);

/**
 * qdf_debugfs_hexdump() - print hexdump into debugfs file
 * @file: debugfs file handle passed in fops->show() function.
 * @buf: data
 * @len: data length
 * @rowsize: row size in bytes to dump
 * @groupsize: group size in bytes to dump
 *
 */
void qdf_debugfs_hexdump(qdf_debugfs_file_t file, const uint8_t *buf,
			 qdf_size_t len, int rowsize, int groupsize);

/**
 * qdf_debugfs_overflow() - check overflow occurrence in debugfs buffer
 * @file: debugfs file handle passed in fops->show() function.
 *
 * Return: 1 on overflow occurrence else 0
 *
 */
bool qdf_debugfs_overflow(qdf_debugfs_file_t file);

/**
 * qdf_debugfs_write() - write data into debugfs file
 * @file: debugfs file handle passed in fops->show() function.
 * @buf: data
 * @len: data length
 *
 */
void qdf_debugfs_write(qdf_debugfs_file_t file, const uint8_t *buf,
		       qdf_size_t len);

/**
 * qdf_debugfs_create_u8() - create a debugfs file for a u8 variable
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @value: pointer to a u8 variable (global/static)
 *
 * Return: None
 */
void qdf_debugfs_create_u8(const char *name, uint16_t mode,
			   qdf_dentry_t parent, u8 *value);

/**
 * qdf_debugfs_create_u16() - create a debugfs file for a u16 variable
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @value: pointer to a u16 variable (global/static)
 *
 * Return: None
 */
void qdf_debugfs_create_u16(const char *name, uint16_t mode,
			    qdf_dentry_t parent, u16 *value);

/**
 * qdf_debugfs_create_u32() - create a debugfs file for a u32 variable
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @value: pointer to a u32 variable (global/static)
 *
 * Return: None
 */
void qdf_debugfs_create_u32(const char *name, uint16_t mode,
			    qdf_dentry_t parent, u32 *value);

/**
 * qdf_debugfs_create_u64() - create a debugfs file for a u64 variable
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @value: pointer to a u64 variable (global/static)
 *
 * Return: None
 */
void qdf_debugfs_create_u64(const char *name, uint16_t mode,
			    qdf_dentry_t parent, u64 *value);

/**
 * qdf_debugfs_create_atomic() - create a debugfs file for an atomic variable
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @value: pointer to an atomic variable (global/static)
 *
 * Return: None
 */
void qdf_debugfs_create_atomic(const char *name, uint16_t mode,
			       qdf_dentry_t parent,
			       qdf_atomic_t *value);

/**
 * qdf_debugfs_create_string() - create a debugfs file for a string
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @str: a pointer to NULL terminated string (global/static).
 *
 * Return: dentry for the file; NULL in case of failure.
 *
 */
qdf_dentry_t qdf_debugfs_create_string(const char *name, uint16_t mode,
				       qdf_dentry_t parent, char *str);

/**
 * qdf_debugfs_remove_dir_recursive() - remove directory recursively
 * @d: debugfs node
 *
 * This function will recursively removes a dreictory in debugfs that was
 * previously createed with a call to qdf_debugfs_create_file() or it's
 * variant functions.
 */
void qdf_debugfs_remove_dir_recursive(qdf_dentry_t d);

/**
 * qdf_debugfs_remove_dir() - remove debugfs directory
 * @d: debugfs node
 *
 */
void qdf_debugfs_remove_dir(qdf_dentry_t d);

/**
 * qdf_debugfs_remove_file() - remove debugfs file
 * @d: debugfs node
 *
 */
void qdf_debugfs_remove_file(qdf_dentry_t d);

/**
 * qdf_debugfs_create_file_simplified() - Create a simple debugfs file
 * where a single function call produces all the desired output
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base 'qdf_debugfs_root'
 * @fops: file operations { .show, .write , .priv... }
 *
 * Users just have to define the show() function and pass it via @fops.show()
 * argument. When the output time comes, the show() will be called once.
 * The show() function must do everything that is needed to write the data,
 * all in one function call.
 * This is useful either for writing small amounts of data to debugfs or
 * for cases in which the output is not iterative.
 * The private data can be passed via @fops.priv, which will be available
 * inside the show() function as the 'private' filed of the qdf_debugfs_file_t.
 *
 * Return: dentry structure pointer in case of success, otherwise NULL.
 *
 */

qdf_dentry_t qdf_debugfs_create_file_simplified(const char *name, uint16_t mode,
						qdf_dentry_t parent,
						struct qdf_debugfs_fops *fops);

/**
 * qdf_debugfs_printer() - Print formated string into debugfs file
 * @priv: The private data
 * @fmt: Format string
 * @...: arguments for the format string
 *
 * This function prints a new line character after printing the formatted
 * string into the debugfs file.
 * This function can be passed when the argument is of type qdf_abstract_print
 */
int qdf_debugfs_printer(void *priv, const char *fmt, ...);

/**
 * qdf_debugfs_create_blob() - create a debugfs file that is used to read
 * a binary blob
 * @name: a pointer to a string containing the name of the file to create.
 * @mode: the permission that the file should have
 * @parent: a pointer to the parent dentry for this file.  This should be a
 *          directory dentry if set.  If this parameter is %NULL, then the
 *          file will be created in the root of the debugfs filesystem.
 * @blob: a pointer to a qdf_debugfs_blob_wrap_t which contains a pointer
 *        to the blob data and the size of the data.
 *
 * Return: dentry structure pointer on success, NULL otherwise.
 */
qdf_dentry_t qdf_debugfs_create_blob(const char *name, umode_t mode,
				     qdf_dentry_t parent,
				     qdf_debugfs_blob_wrap_t blob);

/**
 * qdf_debugfs_create_entry() - create a debugfs file for read or write
 * something
 * @name: name of the file
 * @mode: qdf file mode
 * @parent: parent node. If NULL, defaults to base qdf_debugfs_root
 * @data: Something data that caller want to read or write
 * @fops: file operations { .read, .write ... }
 *
 * Return: dentry structure pointer on success, NULL otherwise.
 */
qdf_dentry_t qdf_debugfs_create_entry(const char *name, uint16_t mode,
				      qdf_dentry_t parent,
				      qdf_entry_t data,
				      const qdf_file_ops_t fops);

#else /* WLAN_DEBUGFS */

static inline QDF_STATUS qdf_debugfs_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void qdf_debugfs_exit(void) { }

static inline qdf_dentry_t qdf_debugfs_create_dir(const char *name,
						  qdf_dentry_t parent)
{
	return NULL;
}

static inline qdf_dentry_t
qdf_debugfs_create_file(const char *name, uint16_t mode, qdf_dentry_t parent,
			struct qdf_debugfs_fops *fops)
{
	return NULL;
}

static inline void qdf_debugfs_printf(qdf_debugfs_file_t file, const char *f,
				      ...)
{
}

static inline void qdf_debugfs_hexdump(qdf_debugfs_file_t file,
				       const uint8_t *buf, qdf_size_t len,
				       int rowsize, int groupsize)
{
}

static inline bool qdf_debugfs_overflow(qdf_debugfs_file_t file)
{
	return 0;
}

static inline void qdf_debugfs_write(qdf_debugfs_file_t file,
				     const uint8_t *buf, qdf_size_t len)
{
}

static inline void qdf_debugfs_create_u8(const char *name,
					 uint16_t mode,
					 qdf_dentry_t parent, u8 *value)
{
}

static inline void qdf_debugfs_create_u16(const char *name,
					  uint16_t mode,
					  qdf_dentry_t parent,
					  u16 *value)
{
}

static inline void qdf_debugfs_create_u32(const char *name,
					  uint16_t mode,
					  qdf_dentry_t parent,
					  u32 *value)
{
}

static inline void qdf_debugfs_create_u64(const char *name,
					  uint16_t mode,
					  qdf_dentry_t parent,
					  u64 *value)
{
}

static inline void qdf_debugfs_create_atomic(const char *name,
					     uint16_t mode,
					     qdf_dentry_t parent,
					     qdf_atomic_t *value)
{
}

static inline qdf_dentry_t debugfs_create_string(const char *name,
						 uint16_t mode,
						 qdf_dentry_t parent, char *str)
{
	return NULL;
}

static inline void qdf_debugfs_remove_dir_recursive(qdf_dentry_t d) {}
static inline void qdf_debugfs_remove_dir(qdf_dentry_t d) {}
static inline void qdf_debugfs_remove_file(qdf_dentry_t d) {}

static inline
qdf_dentry_t qdf_debugfs_create_file_simplified(const char *name, uint16_t mode,
						qdf_dentry_t parent,
						struct qdf_debugfs_fops *fops)
{
	return NULL;
}

static inline
int qdf_debugfs_printer(void *priv, const char *fmt, ...)
{
	return 0;
}

static inline
qdf_dentry_t qdf_debugfs_create_blob(const char *name, umode_t mode,
				     qdf_dentry_t parent,
				     qdf_debugfs_blob_wrap_t blob)
{
	return NULL;
}

static inline
qdf_dentry_t qdf_debugfs_create_entry(const char *name, uint16_t mode,
				      qdf_dentry_t parent,
				      qdf_entry_t data,
				      const qdf_file_ops_t fops)
{
	return NULL;
}
#endif /* WLAN_DEBUGFS */
#endif /* _QDF_DEBUGFS_H */
