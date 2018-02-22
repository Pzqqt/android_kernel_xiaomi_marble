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
 * DOC: qdf_str
 * QCA driver framework (QDF) string APIs.
 */

#ifndef __QDF_STR_H
#define __QDF_STR_H

#include "i_qdf_str.h"
#include "qdf_types.h"

/**
 * qdf_is_space() - check if @c is a whitespace character
 * @c: the character to check
 *
 * Whitespace characters include HT, LF, VT, FF, CR, space, and nbsp
 *
 * Return: true if @ is a whitespace character
 */
static inline bool qdf_is_space(char c)
{
	return __qdf_is_space(c);
}

/**
 * qdf_str_dup() - duplicate null-terminated string @src
 * @dest: double pointer to be populated
 * @src: the null-terminated string to be duplicated
 *
 * @dest must be freed using qdf_mem_free() to avoid memory leaks.
 *
 * Return: QDF_STATUS; @dest set to NULL on failure, a valid address on success
 */
QDF_STATUS qdf_str_dup(char **dest, const char *src);

/**
 * qdf_str_left_trim() - Trim any leading whitespace from @str
 * @str: the string to trim
 *
 * Return: A pointer to the first non-space character in @str
 */
static inline char *qdf_str_left_trim(char *str)
{
	return __qdf_str_left_trim(str);
}

/**
 * qdf_str_right_trim() - Trim any trailing whitespace from @str
 * @str: the string to trim
 *
 * Note: The first trailing whitespace character is replaced with a
 * null-terminator
 *
 * Return: None
 */
void qdf_str_right_trim(char *str);

/**
 * qdf_str_trim() - Trim any leading/trailing whitespace from @str
 * @str: the string to trim
 *
 * Note: The first trailing whitespace character is replaced with a
 * null-terminator
 *
 * Return: A pointer to the first non-space character in @str
 */
static inline char *qdf_str_trim(char *str)
{
	return __qdf_str_trim(str);
}

/**
 * qdf_str_nlen() - Get string length up to @limit characters
 * @str: the string to get the length of
 * @limit: the maximum number of characters to check
 *
 * Return: length of @str, or @limit if the end is not found
 */
static inline qdf_size_t qdf_str_nlen(const char *str, size_t limit)
{
	return __qdf_str_nlen(str, limit);
}

#endif /* __QDF_STR_H */
