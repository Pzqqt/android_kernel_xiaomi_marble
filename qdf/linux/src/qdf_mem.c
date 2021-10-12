/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_mem
 * This file provides OS dependent memory management APIs
 */

#include "qdf_debugfs.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "qdf_lock.h"
#include "qdf_mc_timer.h"
#include "qdf_module.h"
#include <qdf_trace.h>
#include "qdf_str.h"
#include "qdf_talloc.h"
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <qdf_list.h>

#if IS_ENABLED(CONFIG_WCNSS_MEM_PRE_ALLOC)
#include <net/cnss_prealloc.h>
#endif

#if defined(MEMORY_DEBUG) || defined(NBUF_MEMORY_DEBUG)
static bool mem_debug_disabled;
qdf_declare_param(mem_debug_disabled, bool);
qdf_export_symbol(mem_debug_disabled);
#endif

#ifdef MEMORY_DEBUG
static bool is_initial_mem_debug_disabled;
#endif

/* Preprocessor Definitions and Constants */
#define QDF_MEM_MAX_MALLOC (4096 * 1024) /* 4 Mega Bytes */
#define QDF_MEM_WARN_THRESHOLD 300 /* ms */
#define QDF_DEBUG_STRING_SIZE 512

/**
 * struct __qdf_mem_stat - qdf memory statistics
 * @kmalloc: total kmalloc allocations
 * @dma: total dma allocations
 * @skb: total skb allocations
 * @skb_total: total skb allocations in host driver
 * @dp_tx_skb: total Tx skb allocations in datapath
 * @dp_rx_skb: total Rx skb allocations in datapath
 * @skb_mem_max: high watermark for skb allocations
 * @dp_tx_skb_mem_max: high watermark for Tx DP skb allocations
 * @dp_rx_skb_mem_max: high watermark for Rx DP skb allocations
 * @dp_tx_skb_count: DP Tx buffer count
 * @dp_tx_skb_count_max: High watermark for DP Tx buffer count
 * @dp_rx_skb_count: DP Rx buffer count
 * @dp_rx_skb_count_max: High watermark for DP Rx buffer count
 * @tx_descs_outstanding: Current pending Tx descs count
 * @tx_descs_max: High watermark for pending Tx descs count
 */
static struct __qdf_mem_stat {
	qdf_atomic_t kmalloc;
	qdf_atomic_t dma;
	qdf_atomic_t skb;
	qdf_atomic_t skb_total;
	qdf_atomic_t dp_tx_skb;
	qdf_atomic_t dp_rx_skb;
	int32_t skb_mem_max;
	int32_t dp_tx_skb_mem_max;
	int32_t dp_rx_skb_mem_max;
	qdf_atomic_t dp_tx_skb_count;
	int32_t dp_tx_skb_count_max;
	qdf_atomic_t dp_rx_skb_count;
	int32_t dp_rx_skb_count_max;
	qdf_atomic_t tx_descs_outstanding;
	int32_t tx_descs_max;
} qdf_mem_stat;

#ifdef MEMORY_DEBUG
#include "qdf_debug_domain.h"

enum list_type {
	LIST_TYPE_MEM = 0,
	LIST_TYPE_DMA = 1,
	LIST_TYPE_NBUF = 2,
	LIST_TYPE_MAX,
};

/**
 * major_alloc_priv: private data registered to debugfs entry created to list
 *                   the list major allocations
 * @type:            type of the list to be parsed
 * @threshold:       configured by user by overwriting the respective debugfs
 *                   sys entry. This is to list the functions which requested
 *                   memory/dma allocations more than threshold nubmer of times.
 */
struct major_alloc_priv {
	enum list_type type;
	uint32_t threshold;
};

static qdf_list_t qdf_mem_domains[QDF_DEBUG_DOMAIN_COUNT];
static qdf_spinlock_t qdf_mem_list_lock;

static qdf_list_t qdf_mem_dma_domains[QDF_DEBUG_DOMAIN_COUNT];
static qdf_spinlock_t qdf_mem_dma_list_lock;

static inline qdf_list_t *qdf_mem_list_get(enum qdf_debug_domain domain)
{
	return &qdf_mem_domains[domain];
}

static inline qdf_list_t *qdf_mem_dma_list(enum qdf_debug_domain domain)
{
	return &qdf_mem_dma_domains[domain];
}

/**
 * struct qdf_mem_header - memory object to dubug
 * @node: node to the list
 * @domain: the active memory domain at time of allocation
 * @freed: flag set during free, used to detect double frees
 *	Use uint8_t so we can detect corruption
 * @func: name of the function the allocation was made from
 * @line: line number of the file the allocation was made from
 * @size: size of the allocation in bytes
 * @caller: Caller of the function for which memory is allocated
 * @header: a known value, used to detect out-of-bounds access
 * @time: timestamp at which allocation was made
 */
struct qdf_mem_header {
	qdf_list_node_t node;
	enum qdf_debug_domain domain;
	uint8_t freed;
	char func[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t line;
	uint32_t size;
	void *caller;
	uint64_t header;
	uint64_t time;
};

/* align the qdf_mem_header to 8 bytes */
#define QDF_DMA_MEM_HEADER_ALIGN 8

static uint64_t WLAN_MEM_HEADER = 0x6162636465666768;
static uint64_t WLAN_MEM_TRAILER = 0x8081828384858687;

static inline struct qdf_mem_header *qdf_mem_get_header(void *ptr)
{
	return (struct qdf_mem_header *)ptr - 1;
}

/* make sure the header pointer is 8bytes aligned */
static inline struct qdf_mem_header *qdf_mem_dma_get_header(void *ptr,
							    qdf_size_t size)
{
	return (struct qdf_mem_header *)
				qdf_roundup((size_t)((uint8_t *)ptr + size),
					    QDF_DMA_MEM_HEADER_ALIGN);
}

static inline uint64_t *qdf_mem_get_trailer(struct qdf_mem_header *header)
{
	return (uint64_t *)((void *)(header + 1) + header->size);
}

static inline void *qdf_mem_get_ptr(struct qdf_mem_header *header)
{
	return (void *)(header + 1);
}

/* number of bytes needed for the qdf memory debug information */
#define QDF_MEM_DEBUG_SIZE \
	(sizeof(struct qdf_mem_header) + sizeof(WLAN_MEM_TRAILER))

/* number of bytes needed for the qdf dma memory debug information */
#define QDF_DMA_MEM_DEBUG_SIZE \
	(sizeof(struct qdf_mem_header) + QDF_DMA_MEM_HEADER_ALIGN)

static void qdf_mem_trailer_init(struct qdf_mem_header *header)
{
	QDF_BUG(header);
	if (!header)
		return;
	*qdf_mem_get_trailer(header) = WLAN_MEM_TRAILER;
}

static void qdf_mem_header_init(struct qdf_mem_header *header, qdf_size_t size,
				const char *func, uint32_t line, void *caller)
{
	QDF_BUG(header);
	if (!header)
		return;

	header->domain = qdf_debug_domain_get();
	header->freed = false;

	qdf_str_lcopy(header->func, func, QDF_MEM_FUNC_NAME_SIZE);

	header->line = line;
	header->size = size;
	header->caller = caller;
	header->header = WLAN_MEM_HEADER;
	header->time = qdf_get_log_timestamp();
}

enum qdf_mem_validation_bitmap {
	QDF_MEM_BAD_HEADER = 1 << 0,
	QDF_MEM_BAD_TRAILER = 1 << 1,
	QDF_MEM_BAD_SIZE = 1 << 2,
	QDF_MEM_DOUBLE_FREE = 1 << 3,
	QDF_MEM_BAD_FREED = 1 << 4,
	QDF_MEM_BAD_NODE = 1 << 5,
	QDF_MEM_BAD_DOMAIN = 1 << 6,
	QDF_MEM_WRONG_DOMAIN = 1 << 7,
};

static enum qdf_mem_validation_bitmap
qdf_mem_trailer_validate(struct qdf_mem_header *header)
{
	enum qdf_mem_validation_bitmap error_bitmap = 0;

	if (*qdf_mem_get_trailer(header) != WLAN_MEM_TRAILER)
		error_bitmap |= QDF_MEM_BAD_TRAILER;
	return error_bitmap;
}

static enum qdf_mem_validation_bitmap
qdf_mem_header_validate(struct qdf_mem_header *header,
			enum qdf_debug_domain domain)
{
	enum qdf_mem_validation_bitmap error_bitmap = 0;

	if (header->header != WLAN_MEM_HEADER)
		error_bitmap |= QDF_MEM_BAD_HEADER;

	if (header->size > QDF_MEM_MAX_MALLOC)
		error_bitmap |= QDF_MEM_BAD_SIZE;

	if (header->freed == true)
		error_bitmap |= QDF_MEM_DOUBLE_FREE;
	else if (header->freed)
		error_bitmap |= QDF_MEM_BAD_FREED;

	if (!qdf_list_node_in_any_list(&header->node))
		error_bitmap |= QDF_MEM_BAD_NODE;

	if (header->domain < QDF_DEBUG_DOMAIN_INIT ||
	    header->domain >= QDF_DEBUG_DOMAIN_COUNT)
		error_bitmap |= QDF_MEM_BAD_DOMAIN;
	else if (header->domain != domain)
		error_bitmap |= QDF_MEM_WRONG_DOMAIN;

	return error_bitmap;
}

static void
qdf_mem_header_assert_valid(struct qdf_mem_header *header,
			    enum qdf_debug_domain current_domain,
			    enum qdf_mem_validation_bitmap error_bitmap,
			    const char *func,
			    uint32_t line)
{
	if (!error_bitmap)
		return;

	if (error_bitmap & QDF_MEM_BAD_HEADER)
		qdf_err("Corrupted memory header 0x%llx (expected 0x%llx)",
			header->header, WLAN_MEM_HEADER);

	if (error_bitmap & QDF_MEM_BAD_SIZE)
		qdf_err("Corrupted memory size %u (expected < %d)",
			header->size, QDF_MEM_MAX_MALLOC);

	if (error_bitmap & QDF_MEM_BAD_TRAILER)
		qdf_err("Corrupted memory trailer 0x%llx (expected 0x%llx)",
			*qdf_mem_get_trailer(header), WLAN_MEM_TRAILER);

	if (error_bitmap & QDF_MEM_DOUBLE_FREE)
		qdf_err("Memory has previously been freed");

	if (error_bitmap & QDF_MEM_BAD_FREED)
		qdf_err("Corrupted memory freed flag 0x%x", header->freed);

	if (error_bitmap & QDF_MEM_BAD_NODE)
		qdf_err("Corrupted memory header node or double free");

	if (error_bitmap & QDF_MEM_BAD_DOMAIN)
		qdf_err("Corrupted memory domain 0x%x", header->domain);

	if (error_bitmap & QDF_MEM_WRONG_DOMAIN)
		qdf_err("Memory domain mismatch; allocated:%s(%d), current:%s(%d)",
			qdf_debug_domain_name(header->domain), header->domain,
			qdf_debug_domain_name(current_domain), current_domain);

	QDF_MEMDEBUG_PANIC("Fatal memory error detected @ %s:%d", func, line);
}

/**
 * struct __qdf_mem_info - memory statistics
 * @func: the function which allocated memory
 * @line: the line at which allocation happened
 * @size: the size of allocation
 * @caller: Address of the caller function
 * @count: how many allocations of same type
 * @time: timestamp at which allocation happened
 */
struct __qdf_mem_info {
	char func[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t line;
	uint32_t size;
	void *caller;
	uint32_t count;
	uint64_t time;
};

/*
 * The table depth defines the de-duplication proximity scope.
 * A deeper table takes more time, so choose any optimum value.
 */
#define QDF_MEM_STAT_TABLE_SIZE 8

/**
 * qdf_mem_debug_print_header() - memory debug header print logic
 * @print: the print adapter function
 * @print_priv: the private data to be consumed by @print
 * @threshold: the threshold value set by user to list top allocations
 *
 * Return: None
 */
static void qdf_mem_debug_print_header(qdf_abstract_print print,
				       void *print_priv,
				       uint32_t threshold)
{
	if (threshold)
		print(print_priv, "APIs requested allocations >= %u no of time",
		      threshold);
	print(print_priv,
	      "--------------------------------------------------------------");
	print(print_priv,
	      " count    size     total    filename     caller    timestamp");
	print(print_priv,
	      "--------------------------------------------------------------");
}

/**
 * qdf_mem_meta_table_insert() - insert memory metadata into the given table
 * @table: the memory metadata table to insert into
 * @meta: the memory metadata to insert
 *
 * Return: true if the table is full after inserting, false otherwise
 */
static bool qdf_mem_meta_table_insert(struct __qdf_mem_info *table,
				      struct qdf_mem_header *meta)
{
	int i;

	for (i = 0; i < QDF_MEM_STAT_TABLE_SIZE; i++) {
		if (!table[i].count) {
			qdf_str_lcopy(table[i].func, meta->func,
				      QDF_MEM_FUNC_NAME_SIZE);
			table[i].line = meta->line;
			table[i].size = meta->size;
			table[i].count = 1;
			table[i].caller = meta->caller;
			table[i].time = meta->time;
			break;
		}

		if (qdf_str_eq(table[i].func, meta->func) &&
		    table[i].line == meta->line &&
		    table[i].size == meta->size &&
		    table[i].caller == meta->caller) {
			table[i].count++;
			break;
		}
	}

	/* return true if the table is now full */
	return i >= QDF_MEM_STAT_TABLE_SIZE - 1;
}

/**
 * qdf_mem_domain_print() - output agnostic memory domain print logic
 * @domain: the memory domain to print
 * @print: the print adapter function
 * @print_priv: the private data to be consumed by @print
 * @threshold: the threshold value set by uset to list top allocations
 * @mem_print: pointer to function which prints the memory allocation data
 *
 * Return: None
 */
static void qdf_mem_domain_print(qdf_list_t *domain,
				 qdf_abstract_print print,
				 void *print_priv,
				 uint32_t threshold,
				 void (*mem_print)(struct __qdf_mem_info *,
						   qdf_abstract_print,
						   void *, uint32_t))
{
	QDF_STATUS status;
	struct __qdf_mem_info table[QDF_MEM_STAT_TABLE_SIZE];
	qdf_list_node_t *node;

	qdf_mem_zero(table, sizeof(table));
	qdf_mem_debug_print_header(print, print_priv, threshold);

	/* hold lock while inserting to avoid use-after free of the metadata */
	qdf_spin_lock(&qdf_mem_list_lock);
	status = qdf_list_peek_front(domain, &node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		struct qdf_mem_header *meta = (struct qdf_mem_header *)node;
		bool is_full = qdf_mem_meta_table_insert(table, meta);

		qdf_spin_unlock(&qdf_mem_list_lock);

		if (is_full) {
			(*mem_print)(table, print, print_priv, threshold);
			qdf_mem_zero(table, sizeof(table));
		}

		qdf_spin_lock(&qdf_mem_list_lock);
		status = qdf_list_peek_next(domain, node, &node);
	}
	qdf_spin_unlock(&qdf_mem_list_lock);

	(*mem_print)(table, print, print_priv, threshold);
}

/**
 * qdf_mem_meta_table_print() - memory metadata table print logic
 * @table: the memory metadata table to print
 * @print: the print adapter function
 * @print_priv: the private data to be consumed by @print
 * @threshold: the threshold value set by user to list top allocations
 *
 * Return: None
 */
static void qdf_mem_meta_table_print(struct __qdf_mem_info *table,
				     qdf_abstract_print print,
				     void *print_priv,
				     uint32_t threshold)
{
	int i;
	char debug_str[QDF_DEBUG_STRING_SIZE];
	size_t len = 0;
	char *debug_prefix = "WLAN_BUG_RCA: memory leak detected";

	len += qdf_scnprintf(debug_str, sizeof(debug_str) - len,
			     "%s", debug_prefix);

	for (i = 0; i < QDF_MEM_STAT_TABLE_SIZE; i++) {
		if (!table[i].count)
			break;

		print(print_priv,
		      "%6u x %5u = %7uB @ %s:%u   %pS %llu",
		      table[i].count,
		      table[i].size,
		      table[i].count * table[i].size,
		      table[i].func,
		      table[i].line, table[i].caller,
		      table[i].time);
		len += qdf_scnprintf(debug_str + len,
				     sizeof(debug_str) - len,
				     " @ %s:%u %pS",
				     table[i].func,
				     table[i].line,
				     table[i].caller);
	}
	print(print_priv, "%s", debug_str);
}

static int qdf_err_printer(void *priv, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	QDF_VTRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR, (char *)fmt, args);
	va_end(args);

	return 0;
}

#endif /* MEMORY_DEBUG */

bool prealloc_disabled = 1;
qdf_declare_param(prealloc_disabled, bool);
qdf_export_symbol(prealloc_disabled);

int qdf_mem_malloc_flags(void)
{
	if (in_interrupt() || irqs_disabled() || in_atomic())
		return GFP_ATOMIC;

	return GFP_KERNEL;
}

qdf_export_symbol(qdf_mem_malloc_flags);

/**
 * qdf_prealloc_disabled_config_get() - Get the user configuration of
 *                                       prealloc_disabled
 *
 * Return: value of prealloc_disabled qdf module argument
 */
bool qdf_prealloc_disabled_config_get(void)
{
	return prealloc_disabled;
}

qdf_export_symbol(qdf_prealloc_disabled_config_get);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
/**
 * qdf_prealloc_disabled_config_set() - Set prealloc_disabled
 * @str_value: value of the module param
 *
 * This function will set qdf module param prealloc_disabled
 *
 * Return: QDF_STATUS_SUCCESS on Success
 */
QDF_STATUS qdf_prealloc_disabled_config_set(const char *str_value)
{
	QDF_STATUS status;

	status = qdf_bool_parse(str_value, &prealloc_disabled);
	return status;
}
#endif

#if defined WLAN_DEBUGFS

/* Debugfs root directory for qdf_mem */
static struct dentry *qdf_mem_debugfs_root;

#ifdef MEMORY_DEBUG
static int seq_printf_printer(void *priv, const char *fmt, ...)
{
	struct seq_file *file = priv;
	va_list args;

	va_start(args, fmt);
	seq_vprintf(file, fmt, args);
	seq_puts(file, "\n");
	va_end(args);

	return 0;
}

/**
 * qdf_print_major_alloc() - memory metadata table print logic
 * @table: the memory metadata table to print
 * @print: the print adapter function
 * @print_priv: the private data to be consumed by @print
 * @threshold: the threshold value set by uset to list top allocations
 *
 * Return: None
 */
static void qdf_print_major_alloc(struct __qdf_mem_info *table,
				  qdf_abstract_print print,
				  void *print_priv,
				  uint32_t threshold)
{
	int i;

	for (i = 0; i < QDF_MEM_STAT_TABLE_SIZE; i++) {
		if (!table[i].count)
			break;
		if (table[i].count >= threshold)
			print(print_priv,
			      "%6u x %5u = %7uB @ %s:%u   %pS %llu",
			      table[i].count,
			      table[i].size,
			      table[i].count * table[i].size,
			      table[i].func,
			      table[i].line, table[i].caller,
			      table[i].time);
	}
}

/**
 * qdf_mem_seq_start() - sequential callback to start
 * @seq: seq_file handle
 * @pos: The start position of the sequence
 *
 * Return: iterator pointer, or NULL if iteration is complete
 */
static void *qdf_mem_seq_start(struct seq_file *seq, loff_t *pos)
{
	enum qdf_debug_domain domain = *pos;

	if (!qdf_debug_domain_valid(domain))
		return NULL;

	/* just use the current position as our iterator */
	return pos;
}

/**
 * qdf_mem_seq_next() - next sequential callback
 * @seq: seq_file handle
 * @v: the current iterator
 * @pos: the current position
 *
 * Get the next node and release previous node.
 *
 * Return: iterator pointer, or NULL if iteration is complete
 */
static void *qdf_mem_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;

	return qdf_mem_seq_start(seq, pos);
}

/**
 * qdf_mem_seq_stop() - stop sequential callback
 * @seq: seq_file handle
 * @v: current iterator
 *
 * Return: None
 */
static void qdf_mem_seq_stop(struct seq_file *seq, void *v) { }

/**
 * qdf_mem_seq_show() - print sequential callback
 * @seq: seq_file handle
 * @v: current iterator
 *
 * Return: 0 - success
 */
static int qdf_mem_seq_show(struct seq_file *seq, void *v)
{
	enum qdf_debug_domain domain_id = *(enum qdf_debug_domain *)v;

	seq_printf(seq, "\n%s Memory Domain (Id %d)\n",
		   qdf_debug_domain_name(domain_id), domain_id);
	qdf_mem_domain_print(qdf_mem_list_get(domain_id),
			     seq_printf_printer,
			     seq,
			     0,
			     qdf_mem_meta_table_print);

	return 0;
}

/* sequential file operation table */
static const struct seq_operations qdf_mem_seq_ops = {
	.start = qdf_mem_seq_start,
	.next  = qdf_mem_seq_next,
	.stop  = qdf_mem_seq_stop,
	.show  = qdf_mem_seq_show,
};


static int qdf_mem_debugfs_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &qdf_mem_seq_ops);
}

/**
 * qdf_major_alloc_show() - print sequential callback
 * @seq: seq_file handle
 * @v: current iterator
 *
 * Return: 0 - success
 */
static int qdf_major_alloc_show(struct seq_file *seq, void *v)
{
	enum qdf_debug_domain domain_id = *(enum qdf_debug_domain *)v;
	struct major_alloc_priv *priv;
	qdf_list_t *list;

	priv = (struct major_alloc_priv *)seq->private;
	seq_printf(seq, "\n%s Memory Domain (Id %d)\n",
		   qdf_debug_domain_name(domain_id), domain_id);

	switch (priv->type) {
	case LIST_TYPE_MEM:
		list = qdf_mem_list_get(domain_id);
		break;
	case LIST_TYPE_DMA:
		list = qdf_mem_dma_list(domain_id);
		break;
	default:
		list = NULL;
		break;
	}

	if (list)
		qdf_mem_domain_print(list,
				     seq_printf_printer,
				     seq,
				     priv->threshold,
				     qdf_print_major_alloc);

	return 0;
}

/* sequential file operation table created to track major allocs */
static const struct seq_operations qdf_major_allocs_seq_ops = {
	.start = qdf_mem_seq_start,
	.next = qdf_mem_seq_next,
	.stop = qdf_mem_seq_stop,
	.show = qdf_major_alloc_show,
};

static int qdf_major_allocs_open(struct inode *inode, struct file *file)
{
	void *private = inode->i_private;
	struct seq_file *seq;
	int rc;

	rc = seq_open(file, &qdf_major_allocs_seq_ops);
	if (rc == 0) {
		seq = file->private_data;
		seq->private = private;
	}
	return rc;
}

static ssize_t qdf_major_alloc_set_threshold(struct file *file,
					     const char __user *user_buf,
					     size_t count,
					     loff_t *pos)
{
	char buf[32];
	ssize_t buf_size;
	uint32_t threshold;
	struct seq_file *seq = file->private_data;
	struct major_alloc_priv *priv = (struct major_alloc_priv *)seq->private;

	buf_size = min(count, (sizeof(buf) - 1));
	if (buf_size <= 0)
		return 0;
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = '\0';
	if (!kstrtou32(buf, 10, &threshold))
		priv->threshold = threshold;
	return buf_size;
}

/**
 * qdf_print_major_nbuf_allocs() - output agnostic nbuf print logic
 * @threshold: the threshold value set by uset to list top allocations
 * @print: the print adapter function
 * @print_priv: the private data to be consumed by @print
 * @mem_print: pointer to function which prints the memory allocation data
 *
 * Return: None
 */
static void
qdf_print_major_nbuf_allocs(uint32_t threshold,
			    qdf_abstract_print print,
			    void *print_priv,
			    void (*mem_print)(struct __qdf_mem_info *,
					      qdf_abstract_print,
					      void *, uint32_t))
{
	uint32_t nbuf_iter;
	unsigned long irq_flag = 0;
	QDF_NBUF_TRACK *p_node;
	QDF_NBUF_TRACK *p_prev;
	struct __qdf_mem_info table[QDF_MEM_STAT_TABLE_SIZE];
	struct qdf_mem_header meta;
	bool is_full;

	qdf_mem_zero(table, sizeof(table));
	qdf_mem_debug_print_header(print, print_priv, threshold);

	if (is_initial_mem_debug_disabled)
		return;

	qdf_rl_info("major nbuf print with threshold %u", threshold);

	for (nbuf_iter = 0; nbuf_iter < QDF_NET_BUF_TRACK_MAX_SIZE;
	     nbuf_iter++) {
		qdf_nbuf_acquire_track_lock(nbuf_iter, irq_flag);
		p_node = qdf_nbuf_get_track_tbl(nbuf_iter);
		while (p_node) {
			meta.line = p_node->line_num;
			meta.size = p_node->size;
			meta.caller = NULL;
			meta.time = p_node->time;
			qdf_str_lcopy(meta.func, p_node->func_name,
				      QDF_MEM_FUNC_NAME_SIZE);

			is_full = qdf_mem_meta_table_insert(table, &meta);

			if (is_full) {
				(*mem_print)(table, print,
					     print_priv, threshold);
				qdf_mem_zero(table, sizeof(table));
			}

			p_prev = p_node;
			p_node = p_node->p_next;
		}
		qdf_nbuf_release_track_lock(nbuf_iter, irq_flag);
	}

	(*mem_print)(table, print, print_priv, threshold);

	qdf_rl_info("major nbuf print end");
}

/**
 * qdf_major_nbuf_alloc_show() - print sequential callback
 * @seq: seq_file handle
 * @v: current iterator
 *
 * Return: 0 - success
 */
static int qdf_major_nbuf_alloc_show(struct seq_file *seq, void *v)
{
	struct major_alloc_priv *priv = (struct major_alloc_priv *)seq->private;

	if (!priv) {
		qdf_err("priv is null");
		return -EINVAL;
	}

	qdf_print_major_nbuf_allocs(priv->threshold,
				    seq_printf_printer,
				    seq,
				    qdf_print_major_alloc);

	return 0;
}

/**
 * qdf_nbuf_seq_start() - sequential callback to start
 * @seq: seq_file handle
 * @pos: The start position of the sequence
 *
 * Return: iterator pointer, or NULL if iteration is complete
 */
static void *qdf_nbuf_seq_start(struct seq_file *seq, loff_t *pos)
{
	enum qdf_debug_domain domain = *pos;

	if (domain > QDF_DEBUG_NBUF_DOMAIN)
		return NULL;

	return pos;
}

/**
 * qdf_nbuf_seq_next() - next sequential callback
 * @seq: seq_file handle
 * @v: the current iterator
 * @pos: the current position
 *
 * Get the next node and release previous node.
 *
 * Return: iterator pointer, or NULL if iteration is complete
 */
static void *qdf_nbuf_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;

	return qdf_nbuf_seq_start(seq, pos);
}

/**
 * qdf_nbuf_seq_stop() - stop sequential callback
 * @seq: seq_file handle
 * @v: current iterator
 *
 * Return: None
 */
static void qdf_nbuf_seq_stop(struct seq_file *seq, void *v) { }

/* sequential file operation table created to track major skb allocs */
static const struct seq_operations qdf_major_nbuf_allocs_seq_ops = {
	.start = qdf_nbuf_seq_start,
	.next = qdf_nbuf_seq_next,
	.stop = qdf_nbuf_seq_stop,
	.show = qdf_major_nbuf_alloc_show,
};

static int qdf_major_nbuf_allocs_open(struct inode *inode, struct file *file)
{
	void *private = inode->i_private;
	struct seq_file *seq;
	int rc;

	rc = seq_open(file, &qdf_major_nbuf_allocs_seq_ops);
	if (rc == 0) {
		seq = file->private_data;
		seq->private = private;
	}
	return rc;
}

static ssize_t qdf_major_nbuf_alloc_set_threshold(struct file *file,
						  const char __user *user_buf,
						  size_t count,
						  loff_t *pos)
{
	char buf[32];
	ssize_t buf_size;
	uint32_t threshold;
	struct seq_file *seq = file->private_data;
	struct major_alloc_priv *priv = (struct major_alloc_priv *)seq->private;

	buf_size = min(count, (sizeof(buf) - 1));
	if (buf_size <= 0)
		return 0;
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = '\0';
	if (!kstrtou32(buf, 10, &threshold))
		priv->threshold = threshold;
	return buf_size;
}

/* file operation table for listing major allocs */
static const struct file_operations fops_qdf_major_allocs = {
	.owner = THIS_MODULE,
	.open = qdf_major_allocs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
	.write = qdf_major_alloc_set_threshold,
};

/* debugfs file operation table */
static const struct file_operations fops_qdf_mem_debugfs = {
	.owner = THIS_MODULE,
	.open = qdf_mem_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/* file operation table for listing major allocs */
static const struct file_operations fops_qdf_nbuf_major_allocs = {
	.owner = THIS_MODULE,
	.open = qdf_major_nbuf_allocs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
	.write = qdf_major_nbuf_alloc_set_threshold,
};

static struct major_alloc_priv mem_priv = {
	/* List type set to mem */
	LIST_TYPE_MEM,
	/* initial threshold to list APIs which allocates mem >= 50 times */
	50
};

static struct major_alloc_priv dma_priv = {
	/* List type set to DMA */
	LIST_TYPE_DMA,
	/* initial threshold to list APIs which allocates dma >= 50 times */
	50
};

static struct major_alloc_priv nbuf_priv = {
	/* List type set to NBUF */
	LIST_TYPE_NBUF,
	/* initial threshold to list APIs which allocates nbuf >= 50 times */
	50
};

static QDF_STATUS qdf_mem_debug_debugfs_init(void)
{
	if (is_initial_mem_debug_disabled)
		return QDF_STATUS_SUCCESS;

	if (!qdf_mem_debugfs_root)
		return QDF_STATUS_E_FAILURE;

	debugfs_create_file("list",
			    S_IRUSR,
			    qdf_mem_debugfs_root,
			    NULL,
			    &fops_qdf_mem_debugfs);

	debugfs_create_file("major_mem_allocs",
			    0600,
			    qdf_mem_debugfs_root,
			    &mem_priv,
			    &fops_qdf_major_allocs);

	debugfs_create_file("major_dma_allocs",
			    0600,
			    qdf_mem_debugfs_root,
			    &dma_priv,
			    &fops_qdf_major_allocs);

	debugfs_create_file("major_nbuf_allocs",
			    0600,
			    qdf_mem_debugfs_root,
			    &nbuf_priv,
			    &fops_qdf_nbuf_major_allocs);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_mem_debug_debugfs_exit(void)
{
	return QDF_STATUS_SUCCESS;
}

#else /* MEMORY_DEBUG */

static QDF_STATUS qdf_mem_debug_debugfs_init(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static QDF_STATUS qdf_mem_debug_debugfs_exit(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}

#endif /* MEMORY_DEBUG */


static void qdf_mem_debugfs_exit(void)
{
	debugfs_remove_recursive(qdf_mem_debugfs_root);
	qdf_mem_debugfs_root = NULL;
}

static QDF_STATUS qdf_mem_debugfs_init(void)
{
	struct dentry *qdf_debugfs_root = qdf_debugfs_get_root();

	if (!qdf_debugfs_root)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_debugfs_root = debugfs_create_dir("mem", qdf_debugfs_root);

	if (!qdf_mem_debugfs_root)
		return QDF_STATUS_E_FAILURE;


	debugfs_create_atomic_t("kmalloc",
				S_IRUSR,
				qdf_mem_debugfs_root,
				&qdf_mem_stat.kmalloc);

	debugfs_create_atomic_t("dma",
				S_IRUSR,
				qdf_mem_debugfs_root,
				&qdf_mem_stat.dma);

	debugfs_create_atomic_t("skb",
				S_IRUSR,
				qdf_mem_debugfs_root,
				&qdf_mem_stat.skb);

	return QDF_STATUS_SUCCESS;
}

#else /* WLAN_DEBUGFS */

static QDF_STATUS qdf_mem_debugfs_init(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}
static void qdf_mem_debugfs_exit(void) {}


static QDF_STATUS qdf_mem_debug_debugfs_init(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static QDF_STATUS qdf_mem_debug_debugfs_exit(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}

#endif /* WLAN_DEBUGFS */

void qdf_mem_kmalloc_inc(qdf_size_t size)
{
	qdf_atomic_add(size, &qdf_mem_stat.kmalloc);
}

static void qdf_mem_dma_inc(qdf_size_t size)
{
	qdf_atomic_add(size, &qdf_mem_stat.dma);
}

#ifdef CONFIG_WLAN_SYSFS_MEM_STATS
void qdf_mem_skb_inc(qdf_size_t size)
{
	qdf_atomic_add(size, &qdf_mem_stat.skb);
}

void qdf_mem_skb_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.skb);
}

void qdf_mem_skb_total_inc(qdf_size_t size)
{
	int32_t skb_mem_max = 0;

	qdf_atomic_add(size, &qdf_mem_stat.skb_total);
	skb_mem_max = qdf_atomic_read(&qdf_mem_stat.skb_total);
	if (qdf_mem_stat.skb_mem_max < skb_mem_max)
		qdf_mem_stat.skb_mem_max = skb_mem_max;
}

void qdf_mem_skb_total_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.skb_total);
}

void qdf_mem_dp_tx_skb_inc(qdf_size_t size)
{
	int32_t curr_dp_tx_skb_mem_max = 0;

	qdf_atomic_add(size, &qdf_mem_stat.dp_tx_skb);
	curr_dp_tx_skb_mem_max = qdf_atomic_read(&qdf_mem_stat.dp_tx_skb);
	if (qdf_mem_stat.dp_tx_skb_mem_max < curr_dp_tx_skb_mem_max)
		qdf_mem_stat.dp_tx_skb_mem_max = curr_dp_tx_skb_mem_max;
}

void qdf_mem_dp_tx_skb_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.dp_tx_skb);
}

void qdf_mem_dp_rx_skb_inc(qdf_size_t size)
{
	int32_t curr_dp_rx_skb_mem_max = 0;

	qdf_atomic_add(size, &qdf_mem_stat.dp_rx_skb);
	curr_dp_rx_skb_mem_max = qdf_atomic_read(&qdf_mem_stat.dp_rx_skb);
	if (qdf_mem_stat.dp_rx_skb_mem_max < curr_dp_rx_skb_mem_max)
		qdf_mem_stat.dp_rx_skb_mem_max = curr_dp_rx_skb_mem_max;
}

void qdf_mem_dp_rx_skb_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.dp_rx_skb);
}

void qdf_mem_dp_tx_skb_cnt_inc(void)
{
	int32_t curr_dp_tx_skb_count_max = 0;

	qdf_atomic_add(1, &qdf_mem_stat.dp_tx_skb_count);
	curr_dp_tx_skb_count_max =
		qdf_atomic_read(&qdf_mem_stat.dp_tx_skb_count);
	if (qdf_mem_stat.dp_tx_skb_count_max < curr_dp_tx_skb_count_max)
		qdf_mem_stat.dp_tx_skb_count_max = curr_dp_tx_skb_count_max;
}

void qdf_mem_dp_tx_skb_cnt_dec(void)
{
	qdf_atomic_sub(1, &qdf_mem_stat.dp_tx_skb_count);
}

void qdf_mem_dp_rx_skb_cnt_inc(void)
{
	int32_t curr_dp_rx_skb_count_max = 0;

	qdf_atomic_add(1, &qdf_mem_stat.dp_rx_skb_count);
	curr_dp_rx_skb_count_max =
		qdf_atomic_read(&qdf_mem_stat.dp_rx_skb_count);
	if (qdf_mem_stat.dp_rx_skb_count_max < curr_dp_rx_skb_count_max)
		qdf_mem_stat.dp_rx_skb_count_max = curr_dp_rx_skb_count_max;
}

void qdf_mem_dp_rx_skb_cnt_dec(void)
{
	qdf_atomic_sub(1, &qdf_mem_stat.dp_rx_skb_count);
}
#endif

void qdf_mem_kmalloc_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.kmalloc);
}

static inline void qdf_mem_dma_dec(qdf_size_t size)
{
	qdf_atomic_sub(size, &qdf_mem_stat.dma);
}

/**
 * __qdf_mempool_init() - Create and initialize memory pool
 *
 * @osdev: platform device object
 * @pool_addr: address of the pool created
 * @elem_cnt: no. of elements in pool
 * @elem_size: size of each pool element in bytes
 * @flags: flags
 *
 * return: Handle to memory pool or NULL if allocation failed
 */
int __qdf_mempool_init(qdf_device_t osdev, __qdf_mempool_t *pool_addr,
		       int elem_cnt, size_t elem_size, u_int32_t flags)
{
	__qdf_mempool_ctxt_t *new_pool = NULL;
	u_int32_t align = L1_CACHE_BYTES;
	unsigned long aligned_pool_mem;
	int pool_id;
	int i;

	if (prealloc_disabled) {
		/* TBD: We can maintain a list of pools in qdf_device_t
		 * to help debugging
		 * when pre-allocation is not enabled
		 */
		new_pool = (__qdf_mempool_ctxt_t *)
			kmalloc(sizeof(__qdf_mempool_ctxt_t), GFP_KERNEL);
		if (!new_pool)
			return QDF_STATUS_E_NOMEM;

		memset(new_pool, 0, sizeof(*new_pool));
		/* TBD: define flags for zeroing buffers etc */
		new_pool->flags = flags;
		new_pool->elem_size = elem_size;
		new_pool->max_elem = elem_cnt;
		*pool_addr = new_pool;
		return 0;
	}

	for (pool_id = 0; pool_id < MAX_MEM_POOLS; pool_id++) {
		if (!osdev->mem_pool[pool_id])
			break;
	}

	if (pool_id == MAX_MEM_POOLS)
		return -ENOMEM;

	new_pool = osdev->mem_pool[pool_id] = (__qdf_mempool_ctxt_t *)
		kmalloc(sizeof(__qdf_mempool_ctxt_t), GFP_KERNEL);
	if (!new_pool)
		return -ENOMEM;

	memset(new_pool, 0, sizeof(*new_pool));
	/* TBD: define flags for zeroing buffers etc */
	new_pool->flags = flags;
	new_pool->pool_id = pool_id;

	/* Round up the element size to cacheline */
	new_pool->elem_size = roundup(elem_size, L1_CACHE_BYTES);
	new_pool->mem_size = elem_cnt * new_pool->elem_size +
				((align)?(align - 1):0);

	new_pool->pool_mem = kzalloc(new_pool->mem_size, GFP_KERNEL);
	if (!new_pool->pool_mem) {
			/* TBD: Check if we need get_free_pages above */
		kfree(new_pool);
		osdev->mem_pool[pool_id] = NULL;
		return -ENOMEM;
	}

	spin_lock_init(&new_pool->lock);

	/* Initialize free list */
	aligned_pool_mem = (unsigned long)(new_pool->pool_mem) +
			((align) ? (unsigned long)(new_pool->pool_mem)%align:0);
	STAILQ_INIT(&new_pool->free_list);

	for (i = 0; i < elem_cnt; i++)
		STAILQ_INSERT_TAIL(&(new_pool->free_list),
			(mempool_elem_t *)(aligned_pool_mem +
			(new_pool->elem_size * i)), mempool_entry);


	new_pool->free_cnt = elem_cnt;
	*pool_addr = new_pool;
	return 0;
}
qdf_export_symbol(__qdf_mempool_init);

/**
 * __qdf_mempool_destroy() - Destroy memory pool
 * @osdev: platform device object
 * @Handle: to memory pool
 *
 * Returns: none
 */
void __qdf_mempool_destroy(qdf_device_t osdev, __qdf_mempool_t pool)
{
	int pool_id = 0;

	if (!pool)
		return;

	if (prealloc_disabled) {
		kfree(pool);
		return;
	}

	pool_id = pool->pool_id;

	/* TBD: Check if free count matches elem_cnt if debug is enabled */
	kfree(pool->pool_mem);
	kfree(pool);
	osdev->mem_pool[pool_id] = NULL;
}
qdf_export_symbol(__qdf_mempool_destroy);

/**
 * __qdf_mempool_alloc() - Allocate an element memory pool
 *
 * @osdev: platform device object
 * @Handle: to memory pool
 *
 * Return: Pointer to the allocated element or NULL if the pool is empty
 */
void *__qdf_mempool_alloc(qdf_device_t osdev, __qdf_mempool_t pool)
{
	void *buf = NULL;

	if (!pool)
		return NULL;

	if (prealloc_disabled)
		return  qdf_mem_malloc(pool->elem_size);

	spin_lock_bh(&pool->lock);

	buf = STAILQ_FIRST(&pool->free_list);
	if (buf) {
		STAILQ_REMOVE_HEAD(&pool->free_list, mempool_entry);
		pool->free_cnt--;
	}

	/* TBD: Update free count if debug is enabled */
	spin_unlock_bh(&pool->lock);

	return buf;
}
qdf_export_symbol(__qdf_mempool_alloc);

/**
 * __qdf_mempool_free() - Free a memory pool element
 * @osdev: Platform device object
 * @pool: Handle to memory pool
 * @buf: Element to be freed
 *
 * Returns: none
 */
void __qdf_mempool_free(qdf_device_t osdev, __qdf_mempool_t pool, void *buf)
{
	if (!pool)
		return;


	if (prealloc_disabled)
		return qdf_mem_free(buf);

	spin_lock_bh(&pool->lock);
	pool->free_cnt++;

	STAILQ_INSERT_TAIL
		(&pool->free_list, (mempool_elem_t *)buf, mempool_entry);
	spin_unlock_bh(&pool->lock);
}
qdf_export_symbol(__qdf_mempool_free);

#if IS_ENABLED(CONFIG_WCNSS_MEM_PRE_ALLOC)
static bool qdf_might_be_prealloc(void *ptr)
{
	if (ksize(ptr) > WCNSS_PRE_ALLOC_GET_THRESHOLD)
		return true;
	else
		return false;
}

/**
 * qdf_mem_prealloc_get() - conditionally pre-allocate memory
 * @size: the number of bytes to allocate
 *
 * If size if greater than WCNSS_PRE_ALLOC_GET_THRESHOLD, this function returns
 * a chunk of pre-allocated memory. If size if less than or equal to
 * WCNSS_PRE_ALLOC_GET_THRESHOLD, or an error occurs, NULL is returned instead.
 *
 * Return: NULL on failure, non-NULL on success
 */
static void *qdf_mem_prealloc_get(size_t size)
{
	void *ptr;

	if (size <= WCNSS_PRE_ALLOC_GET_THRESHOLD)
		return NULL;

	ptr = wcnss_prealloc_get(size);
	if (!ptr)
		return NULL;

	memset(ptr, 0, size);

	return ptr;
}

static inline bool qdf_mem_prealloc_put(void *ptr)
{
	return wcnss_prealloc_put(ptr);
}
#else
static bool qdf_might_be_prealloc(void *ptr)
{
	return false;
}

static inline void *qdf_mem_prealloc_get(size_t size)
{
	return NULL;
}

static inline bool qdf_mem_prealloc_put(void *ptr)
{
	return false;
}
#endif /* CONFIG_WCNSS_MEM_PRE_ALLOC */

/* External Function implementation */
#ifdef MEMORY_DEBUG
/**
 * qdf_mem_debug_config_get() - Get the user configuration of mem_debug_disabled
 *
 * Return: value of mem_debug_disabled qdf module argument
 */
#ifdef DISABLE_MEM_DBG_LOAD_CONFIG
bool qdf_mem_debug_config_get(void)
{
	/* Return false if DISABLE_LOAD_MEM_DBG_CONFIG flag is enabled */
	return false;
}
#else
bool qdf_mem_debug_config_get(void)
{
	return mem_debug_disabled;
}
#endif /* DISABLE_MEM_DBG_LOAD_CONFIG */

/**
 * qdf_mem_debug_disabled_set() - Set mem_debug_disabled
 * @str_value: value of the module param
 *
 * This function will se qdf module param mem_debug_disabled
 *
 * Return: QDF_STATUS_SUCCESS on Success
 */
#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
QDF_STATUS qdf_mem_debug_disabled_config_set(const char *str_value)
{
	QDF_STATUS status;

	status = qdf_bool_parse(str_value, &mem_debug_disabled);
	return status;
}
#endif

/**
 * qdf_mem_debug_init() - initialize qdf memory debug functionality
 *
 * Return: none
 */
static void qdf_mem_debug_init(void)
{
	int i;

	is_initial_mem_debug_disabled = qdf_mem_debug_config_get();

	if (is_initial_mem_debug_disabled)
		return;

	/* Initalizing the list with maximum size of 60000 */
	for (i = 0; i < QDF_DEBUG_DOMAIN_COUNT; ++i)
		qdf_list_create(&qdf_mem_domains[i], 60000);
	qdf_spinlock_create(&qdf_mem_list_lock);

	/* dma */
	for (i = 0; i < QDF_DEBUG_DOMAIN_COUNT; ++i)
		qdf_list_create(&qdf_mem_dma_domains[i], 0);
	qdf_spinlock_create(&qdf_mem_dma_list_lock);
}

static uint32_t
qdf_mem_domain_check_for_leaks(enum qdf_debug_domain domain,
			       qdf_list_t *mem_list)
{
	if (is_initial_mem_debug_disabled)
		return 0;

	if (qdf_list_empty(mem_list))
		return 0;

	qdf_err("Memory leaks detected in %s domain!",
		qdf_debug_domain_name(domain));
	qdf_mem_domain_print(mem_list,
			     qdf_err_printer,
			     NULL,
			     0,
			     qdf_mem_meta_table_print);

	return mem_list->count;
}

static void qdf_mem_domain_set_check_for_leaks(qdf_list_t *domains)
{
	uint32_t leak_count = 0;
	int i;

	if (is_initial_mem_debug_disabled)
		return;

	/* detect and print leaks */
	for (i = 0; i < QDF_DEBUG_DOMAIN_COUNT; ++i)
		leak_count += qdf_mem_domain_check_for_leaks(i, domains + i);

	if (leak_count)
		QDF_MEMDEBUG_PANIC("%u fatal memory leaks detected!",
				   leak_count);
}

/**
 * qdf_mem_debug_exit() - exit qdf memory debug functionality
 *
 * Return: none
 */
static void qdf_mem_debug_exit(void)
{
	int i;

	if (is_initial_mem_debug_disabled)
		return;

	/* mem */
	qdf_mem_domain_set_check_for_leaks(qdf_mem_domains);
	for (i = 0; i < QDF_DEBUG_DOMAIN_COUNT; ++i)
		qdf_list_destroy(qdf_mem_list_get(i));

	qdf_spinlock_destroy(&qdf_mem_list_lock);

	/* dma */
	qdf_mem_domain_set_check_for_leaks(qdf_mem_dma_domains);
	for (i = 0; i < QDF_DEBUG_DOMAIN_COUNT; ++i)
		qdf_list_destroy(&qdf_mem_dma_domains[i]);
	qdf_spinlock_destroy(&qdf_mem_dma_list_lock);
}

void *qdf_mem_malloc_debug(size_t size, const char *func, uint32_t line,
			   void *caller, uint32_t flag)
{
	QDF_STATUS status;
	enum qdf_debug_domain current_domain = qdf_debug_domain_get();
	qdf_list_t *mem_list = qdf_mem_list_get(current_domain);
	struct qdf_mem_header *header;
	void *ptr;
	unsigned long start, duration;

	if (is_initial_mem_debug_disabled)
		return __qdf_mem_malloc(size, func, line);

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_err("Cannot malloc %zu bytes @ %s:%d", size, func, line);
		return NULL;
	}

	ptr = qdf_mem_prealloc_get(size);
	if (ptr)
		return ptr;

	if (!flag)
		flag = qdf_mem_malloc_flags();

	start = qdf_mc_timer_get_system_time();
	header = kzalloc(size + QDF_MEM_DEBUG_SIZE, flag);
	duration = qdf_mc_timer_get_system_time() - start;

	if (duration > QDF_MEM_WARN_THRESHOLD)
		qdf_warn("Malloc slept; %lums, %zuB @ %s:%d",
			 duration, size, func, line);

	if (!header) {
		qdf_warn("Failed to malloc %zuB @ %s:%d", size, func, line);
		return NULL;
	}

	qdf_mem_header_init(header, size, func, line, caller);
	qdf_mem_trailer_init(header);
	ptr = qdf_mem_get_ptr(header);

	qdf_spin_lock_irqsave(&qdf_mem_list_lock);
	status = qdf_list_insert_front(mem_list, &header->node);
	qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_err("Failed to insert memory header; status %d", status);

	qdf_mem_kmalloc_inc(ksize(header));

	return ptr;
}
qdf_export_symbol(qdf_mem_malloc_debug);

void qdf_mem_free_debug(void *ptr, const char *func, uint32_t line)
{
	enum qdf_debug_domain current_domain = qdf_debug_domain_get();
	struct qdf_mem_header *header;
	enum qdf_mem_validation_bitmap error_bitmap;

	if (is_initial_mem_debug_disabled) {
		__qdf_mem_free(ptr);
		return;
	}

	/* freeing a null pointer is valid */
	if (qdf_unlikely(!ptr))
		return;

	if (qdf_mem_prealloc_put(ptr))
		return;

	if (qdf_unlikely((qdf_size_t)ptr <= sizeof(*header)))
		QDF_MEMDEBUG_PANIC("Failed to free invalid memory location %pK",
				   ptr);

	qdf_talloc_assert_no_children_fl(ptr, func, line);

	qdf_spin_lock_irqsave(&qdf_mem_list_lock);
	header = qdf_mem_get_header(ptr);
	error_bitmap = qdf_mem_header_validate(header, current_domain);
	error_bitmap |= qdf_mem_trailer_validate(header);

	if (!error_bitmap) {
		header->freed = true;
		qdf_list_remove_node(qdf_mem_list_get(header->domain),
				     &header->node);
	}
	qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);

	qdf_mem_header_assert_valid(header, current_domain, error_bitmap,
				    func, line);

	qdf_mem_kmalloc_dec(ksize(header));
	kfree(header);
}
qdf_export_symbol(qdf_mem_free_debug);

void qdf_mem_check_for_leaks(void)
{
	enum qdf_debug_domain current_domain = qdf_debug_domain_get();
	qdf_list_t *mem_list = qdf_mem_list_get(current_domain);
	qdf_list_t *dma_list = qdf_mem_dma_list(current_domain);
	uint32_t leaks_count = 0;

	if (is_initial_mem_debug_disabled)
		return;

	leaks_count += qdf_mem_domain_check_for_leaks(current_domain, mem_list);
	leaks_count += qdf_mem_domain_check_for_leaks(current_domain, dma_list);

	if (leaks_count)
		QDF_MEMDEBUG_PANIC("%u fatal memory leaks detected!",
				   leaks_count);
}

/**
 * qdf_mem_multi_pages_alloc_debug() - Debug version of
 * qdf_mem_multi_pages_alloc
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @element_size: Each element size
 * @element_num: Total number of elements should be allocated
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 * @func: Caller of this allocator
 * @line: Line number of the caller
 * @caller: Return address of the caller
 *
 * This function will allocate large size of memory over multiple pages.
 * Large size of contiguous memory allocation will fail frequently, then
 * instead of allocate large memory by one shot, allocate through multiple, non
 * contiguous memory and combine pages when actual usage
 *
 * Return: None
 */
void qdf_mem_multi_pages_alloc_debug(qdf_device_t osdev,
				     struct qdf_mem_multi_page_t *pages,
				     size_t element_size, uint16_t element_num,
				     qdf_dma_context_t memctxt, bool cacheable,
				     const char *func, uint32_t line,
				     void *caller)
{
	uint16_t page_idx;
	struct qdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages = NULL;
	uint16_t i;

	if (!pages->page_size)
		pages->page_size = qdf_page_size;

	pages->num_element_per_page = pages->page_size / element_size;
	if (!pages->num_element_per_page) {
		qdf_print("Invalid page %d or element size %d",
			  (int)pages->page_size, (int)element_size);
		goto out_fail;
	}

	pages->num_pages = element_num / pages->num_element_per_page;
	if (element_num % pages->num_element_per_page)
		pages->num_pages++;

	if (cacheable) {
		/* Pages information storage */
		pages->cacheable_pages = qdf_mem_malloc_debug(
			pages->num_pages * sizeof(pages->cacheable_pages),
			func, line, caller, 0);
		if (!pages->cacheable_pages)
			goto out_fail;

		cacheable_pages = pages->cacheable_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			cacheable_pages[page_idx] = qdf_mem_malloc_debug(
				pages->page_size, func, line, caller, 0);
			if (!cacheable_pages[page_idx])
				goto page_alloc_fail;
		}
		pages->dma_pages = NULL;
	} else {
		pages->dma_pages = qdf_mem_malloc_debug(
			pages->num_pages * sizeof(struct qdf_mem_dma_page_t),
			func, line, caller, 0);
		if (!pages->dma_pages)
			goto out_fail;

		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			dma_pages->page_v_addr_start =
				qdf_mem_alloc_consistent_debug(
					osdev, osdev->dev, pages->page_size,
					&dma_pages->page_p_addr,
					func, line, caller);
			if (!dma_pages->page_v_addr_start) {
				qdf_print("dmaable page alloc fail pi %d",
					  page_idx);
				goto page_alloc_fail;
			}
			dma_pages->page_v_addr_end =
				dma_pages->page_v_addr_start + pages->page_size;
			dma_pages++;
		}
		pages->cacheable_pages = NULL;
	}
	return;

page_alloc_fail:
	if (cacheable) {
		for (i = 0; i < page_idx; i++)
			qdf_mem_free_debug(pages->cacheable_pages[i],
					   func, line);
		qdf_mem_free_debug(pages->cacheable_pages, func, line);
	} else {
		dma_pages = pages->dma_pages;
		for (i = 0; i < page_idx; i++) {
			qdf_mem_free_consistent_debug(
				osdev, osdev->dev,
				pages->page_size, dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt, func, line);
			dma_pages++;
		}
		qdf_mem_free_debug(pages->dma_pages, func, line);
	}

out_fail:
	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
}

qdf_export_symbol(qdf_mem_multi_pages_alloc_debug);

/**
 * qdf_mem_multi_pages_free_debug() - Debug version of qdf_mem_multi_pages_free
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 * @func: Caller of this allocator
 * @line: Line number of the caller
 *
 * This function will free large size of memory over multiple pages.
 *
 * Return: None
 */
void qdf_mem_multi_pages_free_debug(qdf_device_t osdev,
				    struct qdf_mem_multi_page_t *pages,
				    qdf_dma_context_t memctxt, bool cacheable,
				    const char *func, uint32_t line)
{
	unsigned int page_idx;
	struct qdf_mem_dma_page_t *dma_pages;

	if (!pages->page_size)
		pages->page_size = qdf_page_size;

	if (cacheable) {
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++)
			qdf_mem_free_debug(pages->cacheable_pages[page_idx],
					   func, line);
		qdf_mem_free_debug(pages->cacheable_pages, func, line);
	} else {
		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			qdf_mem_free_consistent_debug(
				osdev, osdev->dev, pages->page_size,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt, func, line);
			dma_pages++;
		}
		qdf_mem_free_debug(pages->dma_pages, func, line);
	}

	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
}

qdf_export_symbol(qdf_mem_multi_pages_free_debug);

#else
static void qdf_mem_debug_init(void) {}

static void qdf_mem_debug_exit(void) {}

void *qdf_mem_malloc_atomic_fl(size_t size, const char *func, uint32_t line)
{
	void *ptr;

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_nofl_err("Cannot malloc %zu bytes @ %s:%d", size, func,
			     line);
		return NULL;
	}

	ptr = qdf_mem_prealloc_get(size);
	if (ptr)
		return ptr;

	ptr = kzalloc(size, GFP_ATOMIC);
	if (!ptr) {
		qdf_nofl_warn("Failed to malloc %zuB @ %s:%d",
			      size, func, line);
		return NULL;
	}

	qdf_mem_kmalloc_inc(ksize(ptr));

	return ptr;
}
qdf_export_symbol(qdf_mem_malloc_atomic_fl);

/**
 * qdf_mem_multi_pages_alloc() - allocate large size of kernel memory
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @element_size: Each element size
 * @element_num: Total number of elements should be allocated
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will allocate large size of memory over multiple pages.
 * Large size of contiguous memory allocation will fail frequently, then
 * instead of allocate large memory by one shot, allocate through multiple, non
 * contiguous memory and combine pages when actual usage
 *
 * Return: None
 */
void qdf_mem_multi_pages_alloc(qdf_device_t osdev,
			       struct qdf_mem_multi_page_t *pages,
			       size_t element_size, uint16_t element_num,
			       qdf_dma_context_t memctxt, bool cacheable)
{
	uint16_t page_idx;
	struct qdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages = NULL;
	uint16_t i;

	if (!pages->page_size)
		pages->page_size = qdf_page_size;

	pages->num_element_per_page = pages->page_size / element_size;
	if (!pages->num_element_per_page) {
		qdf_print("Invalid page %d or element size %d",
			  (int)pages->page_size, (int)element_size);
		goto out_fail;
	}

	pages->num_pages = element_num / pages->num_element_per_page;
	if (element_num % pages->num_element_per_page)
		pages->num_pages++;

	if (cacheable) {
		/* Pages information storage */
		pages->cacheable_pages = qdf_mem_malloc(
			pages->num_pages * sizeof(pages->cacheable_pages));
		if (!pages->cacheable_pages)
			goto out_fail;

		cacheable_pages = pages->cacheable_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			cacheable_pages[page_idx] =
				qdf_mem_malloc(pages->page_size);
			if (!cacheable_pages[page_idx])
				goto page_alloc_fail;
		}
		pages->dma_pages = NULL;
	} else {
		pages->dma_pages = qdf_mem_malloc(
			pages->num_pages * sizeof(struct qdf_mem_dma_page_t));
		if (!pages->dma_pages)
			goto out_fail;

		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			dma_pages->page_v_addr_start =
				qdf_mem_alloc_consistent(osdev, osdev->dev,
					 pages->page_size,
					&dma_pages->page_p_addr);
			if (!dma_pages->page_v_addr_start) {
				qdf_print("dmaable page alloc fail pi %d",
					page_idx);
				goto page_alloc_fail;
			}
			dma_pages->page_v_addr_end =
				dma_pages->page_v_addr_start + pages->page_size;
			dma_pages++;
		}
		pages->cacheable_pages = NULL;
	}
	return;

page_alloc_fail:
	if (cacheable) {
		for (i = 0; i < page_idx; i++)
			qdf_mem_free(pages->cacheable_pages[i]);
		qdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (i = 0; i < page_idx; i++) {
			qdf_mem_free_consistent(
				osdev, osdev->dev, pages->page_size,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		qdf_mem_free(pages->dma_pages);
	}

out_fail:
	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}
qdf_export_symbol(qdf_mem_multi_pages_alloc);

/**
 * qdf_mem_multi_pages_free() - free large size of kernel memory
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will free large size of memory over multiple pages.
 *
 * Return: None
 */
void qdf_mem_multi_pages_free(qdf_device_t osdev,
			      struct qdf_mem_multi_page_t *pages,
			      qdf_dma_context_t memctxt, bool cacheable)
{
	unsigned int page_idx;
	struct qdf_mem_dma_page_t *dma_pages;

	if (!pages->page_size)
		pages->page_size = qdf_page_size;

	if (cacheable) {
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++)
			qdf_mem_free(pages->cacheable_pages[page_idx]);
		qdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			qdf_mem_free_consistent(
				osdev, osdev->dev, pages->page_size,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		qdf_mem_free(pages->dma_pages);
	}

	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}
qdf_export_symbol(qdf_mem_multi_pages_free);
#endif

void qdf_mem_multi_pages_zero(struct qdf_mem_multi_page_t *pages,
			      bool cacheable)
{
	unsigned int page_idx;
	struct qdf_mem_dma_page_t *dma_pages;

	if (!pages->page_size)
		pages->page_size = qdf_page_size;

	if (cacheable) {
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++)
			qdf_mem_zero(pages->cacheable_pages[page_idx],
				     pages->page_size);
	} else {
		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			qdf_mem_zero(dma_pages->page_v_addr_start,
				     pages->page_size);
			dma_pages++;
		}
	}
}

qdf_export_symbol(qdf_mem_multi_pages_zero);

void __qdf_mem_free(void *ptr)
{
	if (!ptr)
		return;

	if (qdf_might_be_prealloc(ptr)) {
		if (qdf_mem_prealloc_put(ptr))
			return;
	}

	qdf_mem_kmalloc_dec(ksize(ptr));

	kfree(ptr);
}

qdf_export_symbol(__qdf_mem_free);

void *__qdf_mem_malloc(size_t size, const char *func, uint32_t line)
{
	void *ptr;

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_nofl_err("Cannot malloc %zu bytes @ %s:%d", size, func,
			     line);
		return NULL;
	}

	ptr = qdf_mem_prealloc_get(size);
	if (ptr)
		return ptr;

	ptr = kzalloc(size, qdf_mem_malloc_flags());
	if (!ptr)
		return NULL;

	qdf_mem_kmalloc_inc(ksize(ptr));

	return ptr;
}

qdf_export_symbol(__qdf_mem_malloc);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
void __qdf_untracked_mem_free(void *ptr)
{
	if (!ptr)
		return;

	kfree(ptr);
}

void *__qdf_untracked_mem_malloc(size_t size, const char *func, uint32_t line)
{
	void *ptr;

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_nofl_err("Cannot malloc %zu bytes @ %s:%d", size, func,
			     line);
		return NULL;
	}

	ptr = kzalloc(size, qdf_mem_malloc_flags());
	if (!ptr)
		return NULL;

	return ptr;
}
#endif

void *qdf_aligned_malloc_fl(uint32_t *size,
			    void **vaddr_unaligned,
				qdf_dma_addr_t *paddr_unaligned,
				qdf_dma_addr_t *paddr_aligned,
				uint32_t align,
			    const char *func, uint32_t line)
{
	void *vaddr_aligned;
	uint32_t align_alloc_size;

	*vaddr_unaligned = qdf_mem_malloc_fl((qdf_size_t)*size, func,
			line);
	if (!*vaddr_unaligned) {
		qdf_warn("Failed to alloc %uB @ %s:%d", *size, func, line);
		return NULL;
	}

	*paddr_unaligned = qdf_mem_virt_to_phys(*vaddr_unaligned);

	/* Re-allocate additional bytes to align base address only if
	 * above allocation returns unaligned address. Reason for
	 * trying exact size allocation above is, OS tries to allocate
	 * blocks of size power-of-2 pages and then free extra pages.
	 * e.g., of a ring size of 1MB, the allocation below will
	 * request 1MB plus 7 bytes for alignment, which will cause a
	 * 2MB block allocation,and that is failing sometimes due to
	 * memory fragmentation.
	 */
	if ((unsigned long)(*paddr_unaligned) & (align - 1)) {
		align_alloc_size = *size + align - 1;

		qdf_mem_free(*vaddr_unaligned);
		*vaddr_unaligned = qdf_mem_malloc_fl(
				(qdf_size_t)align_alloc_size, func, line);
		if (!*vaddr_unaligned) {
			qdf_warn("Failed to alloc %uB @ %s:%d",
				 align_alloc_size, func, line);
			return NULL;
		}

		*paddr_unaligned = qdf_mem_virt_to_phys(
				*vaddr_unaligned);
		*size = align_alloc_size;
	}

	*paddr_aligned = (qdf_dma_addr_t)qdf_align
		((unsigned long)(*paddr_unaligned), align);

	vaddr_aligned = (void *)((unsigned long)(*vaddr_unaligned) +
			((unsigned long)(*paddr_aligned) -
			 (unsigned long)(*paddr_unaligned)));

	return vaddr_aligned;
}

qdf_export_symbol(qdf_aligned_malloc_fl);

/**
 * qdf_mem_multi_page_link() - Make links for multi page elements
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @elem_size: Single element size
 * @elem_count: elements count should be linked
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will make links for multi page allocated structure
 *
 * Return: 0 success
 */
int qdf_mem_multi_page_link(qdf_device_t osdev,
		struct qdf_mem_multi_page_t *pages,
		uint32_t elem_size, uint32_t elem_count, uint8_t cacheable)
{
	uint16_t i, i_int;
	void *page_info;
	void **c_elem = NULL;
	uint32_t num_link = 0;

	for (i = 0; i < pages->num_pages; i++) {
		if (cacheable)
			page_info = pages->cacheable_pages[i];
		else
			page_info = pages->dma_pages[i].page_v_addr_start;

		if (!page_info)
			return -ENOMEM;

		c_elem = (void **)page_info;
		for (i_int = 0; i_int < pages->num_element_per_page; i_int++) {
			if (i_int == (pages->num_element_per_page - 1)) {
				if ((i + 1) == pages->num_pages)
					break;
				if (cacheable)
					*c_elem = pages->
						cacheable_pages[i + 1];
				else
					*c_elem = pages->
						dma_pages[i + 1].
							page_v_addr_start;
				num_link++;
				break;
			} else {
				*c_elem =
					(void *)(((char *)c_elem) + elem_size);
			}
			num_link++;
			c_elem = (void **)*c_elem;

			/* Last link established exit */
			if (num_link == (elem_count - 1))
				break;
		}
	}

	if (c_elem)
		*c_elem = NULL;

	return 0;
}
qdf_export_symbol(qdf_mem_multi_page_link);

void qdf_mem_copy(void *dst_addr, const void *src_addr, uint32_t num_bytes)
{
	/* special case where dst_addr or src_addr can be NULL */
	if (!num_bytes)
		return;

	QDF_BUG(dst_addr);
	QDF_BUG(src_addr);
	if (!dst_addr || !src_addr)
		return;

	memcpy(dst_addr, src_addr, num_bytes);
}
qdf_export_symbol(qdf_mem_copy);

qdf_shared_mem_t *qdf_mem_shared_mem_alloc(qdf_device_t osdev, uint32_t size)
{
	qdf_shared_mem_t *shared_mem;
	qdf_dma_addr_t dma_addr, paddr;
	int ret;

	shared_mem = qdf_mem_malloc(sizeof(*shared_mem));
	if (!shared_mem)
		return NULL;

	shared_mem->vaddr = qdf_mem_alloc_consistent(osdev, osdev->dev,
				size, qdf_mem_get_dma_addr_ptr(osdev,
						&shared_mem->mem_info));
	if (!shared_mem->vaddr) {
		qdf_err("Unable to allocate DMA memory for shared resource");
		qdf_mem_free(shared_mem);
		return NULL;
	}

	qdf_mem_set_dma_size(osdev, &shared_mem->mem_info, size);
	size = qdf_mem_get_dma_size(osdev, &shared_mem->mem_info);

	qdf_mem_zero(shared_mem->vaddr, size);
	dma_addr = qdf_mem_get_dma_addr(osdev, &shared_mem->mem_info);
	paddr = qdf_mem_paddr_from_dmaaddr(osdev, dma_addr);

	qdf_mem_set_dma_pa(osdev, &shared_mem->mem_info, paddr);
	ret = qdf_mem_dma_get_sgtable(osdev->dev, &shared_mem->sgtable,
				      shared_mem->vaddr, dma_addr, size);
	if (ret) {
		qdf_err("Unable to get DMA sgtable");
		qdf_mem_free_consistent(osdev, osdev->dev,
					shared_mem->mem_info.size,
					shared_mem->vaddr,
					dma_addr,
					qdf_get_dma_mem_context(shared_mem,
								memctx));
		qdf_mem_free(shared_mem);
		return NULL;
	}

	qdf_dma_get_sgtable_dma_addr(&shared_mem->sgtable);

	return shared_mem;
}

qdf_export_symbol(qdf_mem_shared_mem_alloc);

/**
 * qdf_mem_copy_toio() - copy memory
 * @dst_addr: Pointer to destination memory location (to copy to)
 * @src_addr: Pointer to source memory location (to copy from)
 * @num_bytes: Number of bytes to copy.
 *
 * Return: none
 */
void qdf_mem_copy_toio(void *dst_addr, const void *src_addr, uint32_t num_bytes)
{
	if (0 == num_bytes) {
		/* special case where dst_addr or src_addr can be NULL */
		return;
	}

	if ((!dst_addr) || (!src_addr)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%pK destination:%pK",
			  __func__, src_addr, dst_addr);
		QDF_ASSERT(0);
		return;
	}
	memcpy_toio(dst_addr, src_addr, num_bytes);
}

qdf_export_symbol(qdf_mem_copy_toio);

/**
 * qdf_mem_set_io() - set (fill) memory with a specified byte value.
 * @ptr: Pointer to memory that will be set
 * @value: Byte set in memory
 * @num_bytes: Number of bytes to be set
 *
 * Return: None
 */
void qdf_mem_set_io(void *ptr, uint32_t num_bytes, uint32_t value)
{
	if (!ptr) {
		qdf_print("%s called with NULL parameter ptr", __func__);
		return;
	}
	memset_io(ptr, value, num_bytes);
}

qdf_export_symbol(qdf_mem_set_io);

void qdf_mem_set(void *ptr, uint32_t num_bytes, uint32_t value)
{
	QDF_BUG(ptr);
	if (!ptr)
		return;

	memset(ptr, value, num_bytes);
}
qdf_export_symbol(qdf_mem_set);

void qdf_mem_move(void *dst_addr, const void *src_addr, uint32_t num_bytes)
{
	/* special case where dst_addr or src_addr can be NULL */
	if (!num_bytes)
		return;

	QDF_BUG(dst_addr);
	QDF_BUG(src_addr);
	if (!dst_addr || !src_addr)
		return;

	memmove(dst_addr, src_addr, num_bytes);
}
qdf_export_symbol(qdf_mem_move);

int qdf_mem_cmp(const void *left, const void *right, size_t size)
{
	QDF_BUG(left);
	QDF_BUG(right);

	return memcmp(left, right, size);
}
qdf_export_symbol(qdf_mem_cmp);

#if defined(A_SIMOS_DEVHOST) || defined(HIF_SDIO) || defined(HIF_USB)
/**
 * qdf_mem_dma_alloc() - allocates memory for dma
 * @osdev: OS device handle
 * @dev: Pointer to device handle
 * @size: Size to be allocated
 * @phy_addr: Physical address
 *
 * Return: pointer of allocated memory or null if memory alloc fails
 */
static inline void *qdf_mem_dma_alloc(qdf_device_t osdev, void *dev,
				      qdf_size_t size,
				      qdf_dma_addr_t *phy_addr)
{
	void *vaddr;

	vaddr = qdf_mem_malloc(size);
	*phy_addr = ((uintptr_t) vaddr);
	/* using this type conversion to suppress "cast from pointer to integer
	 * of different size" warning on some platforms
	 */
	BUILD_BUG_ON(sizeof(*phy_addr) < sizeof(vaddr));
	return vaddr;
}

#elif defined(CONFIG_WIFI_EMULATION_WIFI_3_0) && defined(BUILD_X86) && \
	!defined(QCA_WIFI_QCN9000)

#define QCA8074_RAM_BASE 0x50000000
#define QDF_MEM_ALLOC_X86_MAX_RETRIES 10
void *qdf_mem_dma_alloc(qdf_device_t osdev, void *dev, qdf_size_t size,
			qdf_dma_addr_t *phy_addr)
{
	void *vaddr = NULL;
	int i;

	*phy_addr = 0;

	for (i = 0; i < QDF_MEM_ALLOC_X86_MAX_RETRIES; i++) {
		vaddr = dma_alloc_coherent(dev, size, phy_addr,
					   qdf_mem_malloc_flags());

		if (!vaddr) {
			qdf_err("%s failed , size: %zu!", __func__, size);
			return NULL;
		}

		if (*phy_addr >= QCA8074_RAM_BASE)
			return vaddr;

		dma_free_coherent(dev, size, vaddr, *phy_addr);
	}

	return NULL;
}

#else
static inline void *qdf_mem_dma_alloc(qdf_device_t osdev, void *dev,
				      qdf_size_t size, qdf_dma_addr_t *paddr)
{
	return dma_alloc_coherent(dev, size, paddr, qdf_mem_malloc_flags());
}
#endif

#if defined(A_SIMOS_DEVHOST) || defined(HIF_SDIO) || defined(HIF_USB)
static inline void
qdf_mem_dma_free(void *dev, qdf_size_t size, void *vaddr, qdf_dma_addr_t paddr)
{
	qdf_mem_free(vaddr);
}
#else

static inline void
qdf_mem_dma_free(void *dev, qdf_size_t size, void *vaddr, qdf_dma_addr_t paddr)
{
	dma_free_coherent(dev, size, vaddr, paddr);
}
#endif

#ifdef MEMORY_DEBUG
void *qdf_mem_alloc_consistent_debug(qdf_device_t osdev, void *dev,
				     qdf_size_t size, qdf_dma_addr_t *paddr,
				     const char *func, uint32_t line,
				     void *caller)
{
	QDF_STATUS status;
	enum qdf_debug_domain current_domain = qdf_debug_domain_get();
	qdf_list_t *mem_list = qdf_mem_dma_list(current_domain);
	struct qdf_mem_header *header;
	void *vaddr;

	if (is_initial_mem_debug_disabled)
		return __qdf_mem_alloc_consistent(osdev, dev,
						  size, paddr,
						  func, line);

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_err("Cannot malloc %zu bytes @ %s:%d", size, func, line);
		return NULL;
	}

	vaddr = qdf_mem_dma_alloc(osdev, dev, size + QDF_DMA_MEM_DEBUG_SIZE,
				   paddr);

	if (!vaddr) {
		qdf_warn("Failed to malloc %zuB @ %s:%d", size, func, line);
		return NULL;
	}

	header = qdf_mem_dma_get_header(vaddr, size);
	/* For DMA buffers we only add trailers, this function will init
	 * the header structure at the tail
	 * Prefix the header into DMA buffer causes SMMU faults, so
	 * do not prefix header into the DMA buffers
	 */
	qdf_mem_header_init(header, size, func, line, caller);

	qdf_spin_lock_irqsave(&qdf_mem_dma_list_lock);
	status = qdf_list_insert_front(mem_list, &header->node);
	qdf_spin_unlock_irqrestore(&qdf_mem_dma_list_lock);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_err("Failed to insert memory header; status %d", status);

	qdf_mem_dma_inc(size);

	return vaddr;
}
qdf_export_symbol(qdf_mem_alloc_consistent_debug);

void qdf_mem_free_consistent_debug(qdf_device_t osdev, void *dev,
				   qdf_size_t size, void *vaddr,
				   qdf_dma_addr_t paddr,
				   qdf_dma_context_t memctx,
				   const char *func, uint32_t line)
{
	enum qdf_debug_domain domain = qdf_debug_domain_get();
	struct qdf_mem_header *header;
	enum qdf_mem_validation_bitmap error_bitmap;

	if (is_initial_mem_debug_disabled) {
		__qdf_mem_free_consistent(
					  osdev, dev,
					  size, vaddr,
					  paddr, memctx);
		return;
	}

	/* freeing a null pointer is valid */
	if (qdf_unlikely(!vaddr))
		return;

	qdf_talloc_assert_no_children_fl(vaddr, func, line);

	qdf_spin_lock_irqsave(&qdf_mem_dma_list_lock);
	/* For DMA buffers we only add trailers, this function will retrieve
	 * the header structure at the tail
	 * Prefix the header into DMA buffer causes SMMU faults, so
	 * do not prefix header into the DMA buffers
	 */
	header = qdf_mem_dma_get_header(vaddr, size);
	error_bitmap = qdf_mem_header_validate(header, domain);
	if (!error_bitmap) {
		header->freed = true;
		qdf_list_remove_node(qdf_mem_dma_list(header->domain),
				     &header->node);
	}
	qdf_spin_unlock_irqrestore(&qdf_mem_dma_list_lock);

	qdf_mem_header_assert_valid(header, domain, error_bitmap, func, line);

	qdf_mem_dma_dec(header->size);
	qdf_mem_dma_free(dev, size + QDF_DMA_MEM_DEBUG_SIZE, vaddr, paddr);
}
qdf_export_symbol(qdf_mem_free_consistent_debug);
#endif /* MEMORY_DEBUG */

void __qdf_mem_free_consistent(qdf_device_t osdev, void *dev,
			       qdf_size_t size, void *vaddr,
			       qdf_dma_addr_t paddr, qdf_dma_context_t memctx)
{
	qdf_mem_dma_dec(size);
	qdf_mem_dma_free(dev, size, vaddr, paddr);
}

qdf_export_symbol(__qdf_mem_free_consistent);

void *__qdf_mem_alloc_consistent(qdf_device_t osdev, void *dev,
				 qdf_size_t size, qdf_dma_addr_t *paddr,
				 const char *func, uint32_t line)
{
	void *vaddr;

	if (!size || size > QDF_MEM_MAX_MALLOC) {
		qdf_nofl_err("Cannot malloc %zu bytes @ %s:%d",
			     size, func, line);
		return NULL;
	}

	vaddr = qdf_mem_dma_alloc(osdev, dev, size, paddr);

	if (vaddr)
		qdf_mem_dma_inc(size);

	return vaddr;
}

qdf_export_symbol(__qdf_mem_alloc_consistent);

void *qdf_aligned_mem_alloc_consistent_fl(
	qdf_device_t osdev, uint32_t *size,
	void **vaddr_unaligned, qdf_dma_addr_t *paddr_unaligned,
	qdf_dma_addr_t *paddr_aligned, uint32_t align,
	const char *func, uint32_t line)
{
	void *vaddr_aligned;
	uint32_t align_alloc_size;

	*vaddr_unaligned = qdf_mem_alloc_consistent(
			osdev, osdev->dev, (qdf_size_t)*size, paddr_unaligned);
	if (!*vaddr_unaligned) {
		qdf_warn("Failed to alloc %uB @ %s:%d",
			 *size, func, line);
		return NULL;
	}

	/* Re-allocate additional bytes to align base address only if
	 * above allocation returns unaligned address. Reason for
	 * trying exact size allocation above is, OS tries to allocate
	 * blocks of size power-of-2 pages and then free extra pages.
	 * e.g., of a ring size of 1MB, the allocation below will
	 * request 1MB plus 7 bytes for alignment, which will cause a
	 * 2MB block allocation,and that is failing sometimes due to
	 * memory fragmentation.
	 */
	if ((unsigned long)(*paddr_unaligned) & (align - 1)) {
		align_alloc_size = *size + align - 1;

		qdf_mem_free_consistent(osdev, osdev->dev, *size,
					*vaddr_unaligned,
					*paddr_unaligned, 0);

		*vaddr_unaligned = qdf_mem_alloc_consistent(
				osdev, osdev->dev, align_alloc_size,
				paddr_unaligned);
		if (!*vaddr_unaligned) {
			qdf_warn("Failed to alloc %uB @ %s:%d",
				 align_alloc_size, func, line);
			return NULL;
		}

		*size = align_alloc_size;
	}

	*paddr_aligned = (qdf_dma_addr_t)qdf_align(
			(unsigned long)(*paddr_unaligned), align);

	vaddr_aligned = (void *)((unsigned long)(*vaddr_unaligned) +
				 ((unsigned long)(*paddr_aligned) -
				  (unsigned long)(*paddr_unaligned)));

	return vaddr_aligned;
}
qdf_export_symbol(qdf_aligned_mem_alloc_consistent_fl);

/**
 * qdf_mem_dma_sync_single_for_device() - assign memory to device
 * @osdev: OS device handle
 * @bus_addr: dma address to give to the device
 * @size: Size of the memory block
 * @direction: direction data will be DMAed
 *
 * Assign memory to the remote device.
 * The cache lines are flushed to ram or invalidated as needed.
 *
 * Return: none
 */
void qdf_mem_dma_sync_single_for_device(qdf_device_t osdev,
					qdf_dma_addr_t bus_addr,
					qdf_size_t size,
					enum dma_data_direction direction)
{
	dma_sync_single_for_device(osdev->dev, bus_addr,  size, direction);
}
qdf_export_symbol(qdf_mem_dma_sync_single_for_device);

/**
 * qdf_mem_dma_sync_single_for_cpu() - assign memory to CPU
 * @osdev: OS device handle
 * @bus_addr: dma address to give to the cpu
 * @size: Size of the memory block
 * @direction: direction data will be DMAed
 *
 * Assign memory to the CPU.
 *
 * Return: none
 */
void qdf_mem_dma_sync_single_for_cpu(qdf_device_t osdev,
				     qdf_dma_addr_t bus_addr,
				     qdf_size_t size,
				     enum dma_data_direction direction)
{
	dma_sync_single_for_cpu(osdev->dev, bus_addr,  size, direction);
}
qdf_export_symbol(qdf_mem_dma_sync_single_for_cpu);

void qdf_mem_init(void)
{
	qdf_mem_debug_init();
	qdf_net_buf_debug_init();
	qdf_frag_debug_init();
	qdf_mem_debugfs_init();
	qdf_mem_debug_debugfs_init();
}
qdf_export_symbol(qdf_mem_init);

void qdf_mem_exit(void)
{
	qdf_mem_debug_debugfs_exit();
	qdf_mem_debugfs_exit();
	qdf_frag_debug_exit();
	qdf_net_buf_debug_exit();
	qdf_mem_debug_exit();
}
qdf_export_symbol(qdf_mem_exit);

/**
 * qdf_ether_addr_copy() - copy an Ethernet address
 *
 * @dst_addr: A six-byte array Ethernet address destination
 * @src_addr: A six-byte array Ethernet address source
 *
 * Please note: dst & src must both be aligned to u16.
 *
 * Return: none
 */
void qdf_ether_addr_copy(void *dst_addr, const void *src_addr)
{
	if ((!dst_addr) || (!src_addr)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%pK destination:%pK",
			  __func__, src_addr, dst_addr);
		QDF_ASSERT(0);
		return;
	}
	ether_addr_copy(dst_addr, src_addr);
}
qdf_export_symbol(qdf_ether_addr_copy);

int32_t qdf_dma_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.dma);
}

qdf_export_symbol(qdf_dma_mem_stats_read);

int32_t qdf_heap_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.kmalloc);
}

qdf_export_symbol(qdf_heap_mem_stats_read);

int32_t qdf_skb_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.skb);
}

qdf_export_symbol(qdf_skb_mem_stats_read);

int32_t qdf_skb_total_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.skb_total);
}

qdf_export_symbol(qdf_skb_total_mem_stats_read);

int32_t qdf_skb_max_mem_stats_read(void)
{
	return qdf_mem_stat.skb_mem_max;
}

qdf_export_symbol(qdf_skb_max_mem_stats_read);

int32_t qdf_dp_tx_skb_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.dp_tx_skb);
}

qdf_export_symbol(qdf_dp_tx_skb_mem_stats_read);

int32_t qdf_dp_rx_skb_mem_stats_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.dp_rx_skb);
}

qdf_export_symbol(qdf_dp_rx_skb_mem_stats_read);

int32_t qdf_mem_dp_tx_skb_cnt_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.dp_tx_skb_count);
}

qdf_export_symbol(qdf_mem_dp_tx_skb_cnt_read);

int32_t qdf_mem_dp_tx_skb_max_cnt_read(void)
{
	return qdf_mem_stat.dp_tx_skb_count_max;
}

qdf_export_symbol(qdf_mem_dp_tx_skb_max_cnt_read);

int32_t qdf_mem_dp_rx_skb_cnt_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.dp_rx_skb_count);
}

qdf_export_symbol(qdf_mem_dp_rx_skb_cnt_read);

int32_t qdf_mem_dp_rx_skb_max_cnt_read(void)
{
	return qdf_mem_stat.dp_rx_skb_count_max;
}

qdf_export_symbol(qdf_mem_dp_rx_skb_max_cnt_read);

int32_t qdf_dp_tx_skb_max_mem_stats_read(void)
{
	return qdf_mem_stat.dp_tx_skb_mem_max;
}

qdf_export_symbol(qdf_dp_tx_skb_max_mem_stats_read);

int32_t qdf_dp_rx_skb_max_mem_stats_read(void)
{
	return qdf_mem_stat.dp_rx_skb_mem_max;
}

qdf_export_symbol(qdf_dp_rx_skb_max_mem_stats_read);

int32_t qdf_mem_tx_desc_cnt_read(void)
{
	return qdf_atomic_read(&qdf_mem_stat.tx_descs_outstanding);
}

qdf_export_symbol(qdf_mem_tx_desc_cnt_read);

int32_t qdf_mem_tx_desc_max_read(void)
{
	return qdf_mem_stat.tx_descs_max;
}

qdf_export_symbol(qdf_mem_tx_desc_max_read);

void qdf_mem_tx_desc_cnt_update(qdf_atomic_t pending_tx_descs,
				int32_t tx_descs_max)
{
	qdf_mem_stat.tx_descs_outstanding = pending_tx_descs;
	qdf_mem_stat.tx_descs_max = tx_descs_max;
}

qdf_export_symbol(qdf_mem_tx_desc_cnt_update);

void qdf_mem_stats_init(void)
{
	qdf_mem_stat.skb_mem_max = 0;
	qdf_mem_stat.dp_tx_skb_mem_max = 0;
	qdf_mem_stat.dp_rx_skb_mem_max = 0;
	qdf_mem_stat.dp_tx_skb_count_max = 0;
	qdf_mem_stat.dp_rx_skb_count_max = 0;
	qdf_mem_stat.tx_descs_max = 0;
}

qdf_export_symbol(qdf_mem_stats_init);

void *__qdf_mem_valloc(size_t size, const char *func, uint32_t line)
{
	void *ptr;

	if (!size) {
		qdf_err("Valloc called with 0 bytes @ %s:%d", func, line);
		return NULL;
	}

	ptr = vzalloc(size);

	return ptr;
}

qdf_export_symbol(__qdf_mem_valloc);

void __qdf_mem_vfree(void *ptr)
{
	if (qdf_unlikely(!ptr))
		return;

	vfree(ptr);
}

qdf_export_symbol(__qdf_mem_vfree);
