/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#if !defined(_IPA_NATI_MAP_H_)
# define _IPA_NATI_MAP_H_

#include <stdint.h>

# ifdef __cplusplus
extern "C"
{
# endif /* __cplusplus */

/* Used below */
#define MAKE_AS_STR_CASE(v) case v: return #v

/*
 * The following is used to describe which map to use.
 *
 * PLEASE KEEP THE FOLLOWING IN SYNC WITH ipa_which_map_as_str()
 * BELOW.
 */
typedef enum
{
	MAP_NUM_00 = 0,
	MAP_NUM_01 = 1,
	MAP_NUM_02 = 2,
	MAP_NUM_03 = 3,

	MAP_NUM_99 = 4,

	MAP_NUM_MAX
} ipa_which_map;

#define VALID_IPA_USE_MAP(w) \
	( (w) >= MAP_NUM_00 || (w) < MAP_NUM_MAX )

/* KEEP THE FOLLOWING IN SYNC WITH ABOVE. */
static inline const char* ipa_which_map_as_str(
	ipa_which_map w )
{
	switch ( w )
	{
		MAKE_AS_STR_CASE(MAP_NUM_00);
		MAKE_AS_STR_CASE(MAP_NUM_01);
		MAKE_AS_STR_CASE(MAP_NUM_02);
		MAKE_AS_STR_CASE(MAP_NUM_03);

		MAKE_AS_STR_CASE(MAP_NUM_99);
	default:
		break;
	}

	return "???";
}

int ipa_nat_map_add(
	ipa_which_map which,
	uint32_t      key,
	uint32_t      val );

int ipa_nat_map_find(
	ipa_which_map which,
	uint32_t      key,
	uint32_t*     val_ptr );

int ipa_nat_map_del(
	ipa_which_map which,
	uint32_t      key,
	uint32_t*     val_ptr );

int ipa_nat_map_clear(
	ipa_which_map which );

int ipa_nat_map_dump(
	ipa_which_map which );

# ifdef __cplusplus
}
# endif /* __cplusplus */

#endif /* #if !defined(_IPA_NATI_MAP_H_) */
