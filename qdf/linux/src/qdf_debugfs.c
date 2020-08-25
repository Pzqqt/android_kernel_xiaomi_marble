/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_debugfs
 * This file provides QDF debug file system APIs
 */

#include <qdf_debugfs.h>
#include <i_qdf_debugfs.h>
#include <qdf_mem.h>
#include <qdf_trace.h>
#include <qdf_module.h>

/* A private structure definition to qdf sequence */
struct qdf_debugfs_seq_priv {
	bool stop;
};

/* entry for root debugfs directory*/
static qdf_dentry_t qdf_debugfs_root;

QDF_STATUS qdf_debugfs_init(void)
{
	qdf_debugfs_root = debugfs_create_dir(KBUILD_MODNAME, NULL);

	if (!qdf_debugfs_root)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_debugfs_init);

void qdf_debugfs_exit(void)
{
	if (!qdf_debugfs_root)
		return;

	debugfs_remove_recursive(qdf_debugfs_root);
	qdf_debugfs_root = NULL;
}
qdf_export_symbol(qdf_debugfs_exit);

qdf_dentry_t qdf_debugfs_get_root(void)
{
	return qdf_debugfs_root;
}
qdf_export_symbol(qdf_debugfs_get_root);

umode_t qdf_debugfs_get_filemode(uint16_t mode)
{
	umode_t ret = 0;

	if (mode & QDF_FILE_USR_READ)
		ret |= 0400;
	if (mode & QDF_FILE_USR_WRITE)
		ret |= 0200;

	if (mode & QDF_FILE_GRP_READ)
		ret |= 0040;
	if (mode & QDF_FILE_GRP_WRITE)
		ret |= 0020;

	if (mode & QDF_FILE_OTH_READ)
		ret |= 0004;
	if (mode & QDF_FILE_OTH_WRITE)
		ret |= 0002;

	return ret;
}

/**
 * ---------------------- Implementation note ---------------------------------
 *
 * A read in debugfs file triggers seq_read() which calls seq_read api. A
 * sequence begins with the call of the function start(). If the return is a non
 * NULL value, the function next() is called. This function is an iterator, the
 * goal is to go though all the data. Each time next() is called, the function
 * show() is also called. It writes data values in the buffer read by the user.
 * The function next() is called until it returns NULL. The sequence ends when
 * next() returns NULL, then the function stop() is called.
 *
 * NOTE: When a sequence is finished, another one starts. That means that
 * at the end of function stop(), the function start() is called again. This
 * loop finishes when the function start() returns NULL.
 * ----------------------------------------------------------------------------
 */

/* .seq_start() */
static void *qdf_debugfs_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct qdf_debugfs_seq_priv *priv;

	priv = qdf_mem_malloc(sizeof(*priv));
	if (!priv)
		return NULL;

	priv->stop = false;

	return priv;
}

/* .seq_next() */
static void *qdf_debugfs_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct qdf_debugfs_seq_priv *priv = v;

	if (priv)
		++*pos;

	if (priv && priv->stop) {
		qdf_mem_free(priv);
		priv = NULL;
	}

	return priv;
}

/* .seq_stop() */
static void qdf_debugfs_seq_stop(struct seq_file *seq, void *v)
{
	qdf_mem_free(v);
}

/* .seq_show() */
static int qdf_debugfs_seq_show(struct seq_file *seq, void *v)
{
	struct qdf_debugfs_seq_priv *priv = v;
	struct qdf_debugfs_fops *fops;
	QDF_STATUS status;

	fops = seq->private;

	if (fops && fops->show) {
		status = fops->show(seq, fops->priv);

		if (priv && (status != QDF_STATUS_E_AGAIN))
			priv->stop = true;
	}

	return 0;
}

void qdf_debugfs_printf(qdf_debugfs_file_t file, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	seq_vprintf(file, f, args);
	va_end(args);
}

qdf_export_symbol(qdf_debugfs_printf);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))

void qdf_debugfs_hexdump(qdf_debugfs_file_t file, const uint8_t *buf,
			 qdf_size_t len, int rowsize, int groupsize)
{
	seq_hex_dump(file, "", DUMP_PREFIX_OFFSET, rowsize, groupsize, buf, len,
		     false);
}

qdf_export_symbol(qdf_debugfs_hexdump);
#else

void qdf_debugfs_hexdump(qdf_debugfs_file_t file, const uint8_t *buf,
			 qdf_size_t len, int rowsize, int groupsize)
{
	char *dst;
	size_t dstlen, readlen, remaining = len;
	int prefix = 0;
	size_t commitlen;

	while (remaining > 0 && (file->size > file->count)) {
		seq_printf(file, "%.8x: ", prefix);

		readlen = qdf_min(remaining, (qdf_size_t)rowsize);
		dstlen = seq_get_buf(file, &dst);
		hex_dump_to_buffer(buf, readlen, rowsize, groupsize, dst,
				   dstlen, false);
		commitlen = strnlen(dst, dstlen);
		seq_commit(file, commitlen);
		seq_putc(file, '\n');

		remaining = (remaining > rowsize) ? remaining - rowsize : 0;
		buf += readlen;
		prefix += rowsize;
	}
}

qdf_export_symbol(qdf_debugfs_hexdump);
#endif

bool qdf_debugfs_overflow(qdf_debugfs_file_t file)
{
	return seq_has_overflowed(file);
}

qdf_export_symbol(qdf_debugfs_overflow);

void qdf_debugfs_write(qdf_debugfs_file_t file, const uint8_t *buf,
		       qdf_size_t len)
{
	seq_write(file, buf, len);
}

/* sequential file operation table */
static const struct seq_operations __qdf_debugfs_seq_ops = {
	.start = qdf_debugfs_seq_start,
	.next = qdf_debugfs_seq_next,
	.stop = qdf_debugfs_seq_stop,
	.show = qdf_debugfs_seq_show,
};

/* .open() */
static int qdf_seq_open(struct inode *inode, struct file *file)
{
	void *private = inode->i_private;
	struct seq_file *seq;
	int rc;

	/**
	 * Note: seq_open() will allocate a struct seq_file and store its
	 * pointer in @file->private_data. It warns if private_data is not NULL.
	 */

	rc = seq_open(file, &__qdf_debugfs_seq_ops);

	if (rc == 0) {
		seq = file->private_data;
		seq->private = private;
	}

	return rc;
}

/* .write() */
static ssize_t qdf_seq_write(struct file *filp, const char __user *ubuf,
			     size_t len, loff_t *ppos)
{
	struct qdf_debugfs_fops *fops;
	struct seq_file *seq;
	u8 *buf;
	ssize_t rc = 0;

	if (len == 0)
		return 0;

	seq = filp->private_data;
	fops = seq->private;
	if (fops && fops->write) {
		buf = qdf_mem_malloc(len + 1);
		if (buf) {
			buf[len] = '\0';
			rc = simple_write_to_buffer(buf, len, ppos, ubuf, len);
			fops->write(fops->priv, buf, len + 1);
			qdf_mem_free(buf);
		}
	}

	return rc;
}

/* debugfs file operation table */
static const struct file_operations __qdf_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = qdf_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
	.write = qdf_seq_write,
};

qdf_dentry_t qdf_debugfs_create_dir(const char *name, qdf_dentry_t parent)
{
	qdf_dentry_t dir;

	if (!name)
		return NULL;
	if (!parent)
		parent = qdf_debugfs_get_root();

	dir = debugfs_create_dir(name, parent);

	if (IS_ERR_OR_NULL(dir)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s creation failed", name);
		dir = NULL;
	}

	return dir;
}
qdf_export_symbol(qdf_debugfs_create_dir);

qdf_dentry_t qdf_debugfs_create_file(const char *name, uint16_t mode,
				     qdf_dentry_t parent,
				     struct qdf_debugfs_fops *fops)
{
	qdf_dentry_t file;
	umode_t filemode;

	if (!name || !fops)
		return NULL;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	file = debugfs_create_file(name, filemode, parent, fops,
				   &__qdf_debugfs_fops);

	if (IS_ERR_OR_NULL(file)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s creation failed 0x%pK", name, file);
		file = NULL;
	}

	return file;
}
qdf_export_symbol(qdf_debugfs_create_file);

void qdf_debugfs_create_u8(const char *name, uint16_t mode,
			   qdf_dentry_t parent, u8 *value)
{
	umode_t filemode;

	if (!name)
		return;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	debugfs_create_u8(name, filemode, parent, value);
}

void qdf_debugfs_create_u16(const char *name, uint16_t mode,
			    qdf_dentry_t parent, u16 *value)
{
	umode_t filemode;

	if (!name)
		return;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	debugfs_create_u16(name, filemode, parent, value);
}

qdf_export_symbol(qdf_debugfs_create_u16);

void qdf_debugfs_create_u32(const char *name,
			    uint16_t mode,
			    qdf_dentry_t parent, u32 *value)
{
	umode_t filemode;

	if (!name)
		return;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	debugfs_create_u32(name, filemode, parent, value);
}

qdf_export_symbol(qdf_debugfs_create_u32);

void qdf_debugfs_create_u64(const char *name, uint16_t mode,
			    qdf_dentry_t parent, u64 *value)
{
	umode_t filemode;

	if (!name)
		return;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	debugfs_create_u64(name, filemode, parent, value);
}

qdf_export_symbol(qdf_debugfs_create_u64);

void qdf_debugfs_create_atomic(const char *name, uint16_t mode,
			       qdf_dentry_t parent, qdf_atomic_t *value)
{
	umode_t filemode;

	if (!name)
		return;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	debugfs_create_atomic_t(name, filemode, parent, value);
}

qdf_export_symbol(qdf_debugfs_create_atomic);

static int qdf_debugfs_string_show(struct seq_file *seq, void *pos)
{
	char *str = seq->private;

	seq_puts(seq, str);
	seq_putc(seq, '\n');

	return 0;
}

static int qdf_debugfs_string_open(struct inode *inode, struct file *file)
{
	return single_open(file, qdf_debugfs_string_show, inode->i_private);
}

static const struct file_operations qdf_string_fops = {
	.owner = THIS_MODULE,
	.open = qdf_debugfs_string_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release
};

qdf_dentry_t qdf_debugfs_create_string(const char *name, uint16_t mode,
				       qdf_dentry_t parent, char *str)
{
	umode_t filemode;

	if (!name)
		return NULL;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	return debugfs_create_file(name, filemode, parent, str,
				   &qdf_string_fops);
}
qdf_export_symbol(qdf_debugfs_create_string);

void qdf_debugfs_remove_dir_recursive(qdf_dentry_t d)
{
	debugfs_remove_recursive(d);
}
qdf_export_symbol(qdf_debugfs_remove_dir_recursive);

void qdf_debugfs_remove_dir(qdf_dentry_t d)
{
	debugfs_remove(d);
}
qdf_export_symbol(qdf_debugfs_remove_dir);

void qdf_debugfs_remove_file(qdf_dentry_t d)
{
	debugfs_remove(d);
}
qdf_export_symbol(qdf_debugfs_remove_file);

static int qdf_debugfs_single_show(struct seq_file *seq, void *v)
{
	struct qdf_debugfs_fops *fops = seq->private;

	if (fops && fops->show)
		fops->show(seq, fops->priv);

	return 0;
}

/* .open() */
static int qdf_debugfs_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, qdf_debugfs_single_show,
			   inode->i_private);
}

/* File operations for the simplified version */
static const struct file_operations qdf_debugfs_fops_simple = {
	.owner          = THIS_MODULE,
	.open           = qdf_debugfs_single_open,
	.release        = single_release,
	.read           = seq_read,
	.llseek         = seq_lseek,
};

qdf_dentry_t qdf_debugfs_create_file_simplified(
	const char *name, uint16_t mode,
	qdf_dentry_t parent, struct qdf_debugfs_fops *fops)
{
	qdf_dentry_t file;
	umode_t filemode;

	if (!name || !fops)
		return NULL;

	if (!parent)
		parent = qdf_debugfs_get_root();

	filemode = qdf_debugfs_get_filemode(mode);
	file = debugfs_create_file(name, filemode, parent, fops,
				   &qdf_debugfs_fops_simple);

	if (IS_ERR_OR_NULL(file)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s creation failed 0x%pK", name, file);
		file = NULL;
	}

	return file;
}
qdf_export_symbol(qdf_debugfs_create_file_simplified);

int qdf_debugfs_printer(void *priv, const char *fmt, ...)
{
	struct seq_file *file = priv;
	va_list args;

	va_start(args, fmt);
	seq_vprintf(file, fmt, args);
	seq_puts(file, "\n");
	va_end(args);

	return 0;
}
qdf_export_symbol(qdf_debugfs_printer);
