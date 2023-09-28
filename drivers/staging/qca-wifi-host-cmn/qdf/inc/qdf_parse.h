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
 * DOC: Text parsing related abstractions, not related to a specific type
 */

#ifndef __QDF_PARSE_H
#define __QDF_PARSE_H

#include "qdf_status.h"

typedef QDF_STATUS (*qdf_ini_section_cb)(void *context, const char *name);
typedef QDF_STATUS (*qdf_ini_item_cb)(void *context,
				      const char *key,
				      const char *value);

/**
 * qdf_ini_parse() - parse an ini file
 * @ini_path: The full file path of the ini file to parse
 * @context: The caller supplied context to pass into callbacks
 * @item_cb: Ini item (key/value pair) handler callback function
 *	Return QDF_STATUS_SUCCESS to continue parsing, else to abort
 * @section_cb: Ini section header handler callback function
 *	Return QDF_STATUS_SUCCESS to continue parsing, else to abort
 *
 * The *.ini file format is a simple format consisting of a list of key/value
 * pairs (items), separated by an '=' character. Comments are initiated with
 * a '#' character. Sections are also supported, using '[' and ']' around the
 * section name. e.g.
 *
 *	# comments are started with a '#' character
 *	# items are key/value string pairs, separated by the '=' character
 *	someKey1=someValue1
 *	someKey2=someValue2 # this is also a comment
 *
 *	# section headers are enclosed in square brackets
 *	[some section header] # new section begins
 *	someKey3=someValue3
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
qdf_ini_parse(const char *ini_path, void *context,
	      qdf_ini_item_cb item_cb, qdf_ini_section_cb section_cb);

#endif /* __QDF_PARSE_H */

