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
 * qdf_str_cmp - Compare two strings
 * @str1: First string
 * @str2: Second string
 * Return:
 *	 0 - strings are equal
 *	<0 - str1 sorts lexicographically before str2
 *	>0 - str1 sorts lexicographically after str2
 */
static inline int32_t qdf_str_cmp(const char *str1, const char *str2)
{
	return __qdf_str_cmp(str1, str2);
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
 * qdf_str_eq - compare two null-terminated strings for equality
 * @left: the string left of the equality
 * @right: the string right of the equality
 *
 * This is a thin wrapper over `if (strcmp(left, right) == 0)` for clarity.
 *
 * Return: true if strings are equal
 */
static inline bool qdf_str_eq(const char *left, const char *right)
{
	return qdf_str_cmp(left, right) == 0;
}

/**
 * qdf_str_lcopy - Bounded copy from one string to another
 * @dest: destination string
 * @src: source string
 * @dest_size: max number of bytes to copy (incl. null terminator)
 *
 * If the return value is >= @dest_size, @dest has been truncated.
 *
 * Return: length of @src
 */
static inline qdf_size_t
qdf_str_lcopy(char *dest, const char *src, uint32_t dest_size)
{
	return __qdf_str_lcopy(dest, src, dest_size);
}

/**
 * qdf_str_left_trim() - Trim any leading whitespace from @str
 * @str: the string to trim
 *
 * Return: A pointer to the first non-space character in @str
 */
static inline const char *qdf_str_left_trim(const char *str)
{
	return __qdf_str_left_trim(str);
}

/**
 * qdf_str_len() - returns the length of a null-terminated string
 * @str: input string
 *
 * Return: length of @str (without null terminator)
 */
static inline qdf_size_t qdf_str_len(const char *str)
{
	return __qdf_str_len(str);
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
 * Return: the less of @limit or the length of @str (without null terminator)
 */
static inline qdf_size_t qdf_str_nlen(const char *str, qdf_size_t limit)
{
	return __qdf_str_nlen(str, limit);
}

#endif /* __QDF_STR_H */
