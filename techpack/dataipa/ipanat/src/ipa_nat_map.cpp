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
#include <map>
#include <iterator>

#include "ipa_nat_utils.h"

#include "ipa_nat_map.h"

static std::map<uint32_t, uint32_t> map_array[MAP_NUM_MAX];

/******************************************************************************/

int ipa_nat_map_add(
	ipa_which_map which,
	uint32_t      key,
	uint32_t      val )
{
	int ret_val = 0;

	std::pair<std::map<uint32_t, uint32_t>::iterator, bool> ret;

	IPADBG("In\n");

	if ( ! VALID_IPA_USE_MAP(which) )
	{
		IPAERR("Bad arg which(%u)\n", which);
		ret_val = -1;
		goto bail;
	}

	IPADBG("[%s] key(%u) -> val(%u)\n",
		   ipa_which_map_as_str(which), key, val);

	ret = map_array[which].insert(std::pair<uint32_t, uint32_t>(key, val));

	if ( ret.second == false )
	{
		IPAERR("[%s] key(%u) already exists in map\n",
			   ipa_which_map_as_str(which),
			   key);
		ret_val = -1;
	}

bail:
	IPADBG("Out\n");

	return ret_val;
}

/******************************************************************************/

int ipa_nat_map_find(
	ipa_which_map which,
	uint32_t      key,
	uint32_t*     val_ptr )
{
	int ret_val = 0;

	std::map<uint32_t, uint32_t>::iterator it;

	IPADBG("In\n");

	if ( ! VALID_IPA_USE_MAP(which) )
	{
		IPAERR("Bad arg which(%u)\n", which);
		ret_val = -1;
		goto bail;
	}

	IPADBG("[%s] key(%u)\n",
		   ipa_which_map_as_str(which), key);

	it = map_array[which].find(key);

	if ( it == map_array[which].end() )
	{
		IPAERR("[%s] key(%u) not found in map\n",
			   ipa_which_map_as_str(which),
			   key);
		ret_val = -1;
	}
	else
	{
		if ( val_ptr )
		{
			*val_ptr = it->second;
			IPADBG("[%s] key(%u) -> val(%u)\n",
				   ipa_which_map_as_str(which),
				   key, *val_ptr);
		}
	}

bail:
	IPADBG("Out\n");

	return ret_val;
}

/******************************************************************************/

int ipa_nat_map_del(
	ipa_which_map which,
	uint32_t      key,
	uint32_t*     val_ptr )
{
	int ret_val = 0;

	std::map<uint32_t, uint32_t>::iterator it;

	IPADBG("In\n");

	if ( ! VALID_IPA_USE_MAP(which) )
	{
		IPAERR("Bad arg which(%u)\n", which);
		ret_val = -1;
		goto bail;
	}

	IPADBG("[%s] key(%u)\n",
		   ipa_which_map_as_str(which), key);

	it = map_array[which].find(key);

	if ( it == map_array[which].end() )
	{
		IPAERR("[%s] key(%u) not found in map\n",
			   ipa_which_map_as_str(which),
			   key);
		ret_val = -1;
	}
	else
	{
		if ( val_ptr )
		{
			*val_ptr = it->second;
			IPADBG("[%s] key(%u) -> val(%u)\n",
				   ipa_which_map_as_str(which),
				   key, *val_ptr);
		}
		map_array[which].erase(it);
	}

bail:
	IPADBG("Out\n");

	return ret_val;
}

int ipa_nat_map_clear(
	ipa_which_map which )
{
	int ret_val = 0;

	IPADBG("In\n");

	if ( ! VALID_IPA_USE_MAP(which) )
	{
		IPAERR("Bad arg which(%u)\n", which);
		ret_val = -1;
		goto bail;
	}

	map_array[which].clear();

bail:
	IPADBG("Out\n");

	return ret_val;
}

int ipa_nat_map_dump(
	ipa_which_map which )
{
	std::map<uint32_t, uint32_t>::iterator it;

	int ret_val = 0;

	IPADBG("In\n");

	if ( ! VALID_IPA_USE_MAP(which) )
	{
		IPAERR("Bad arg which(%u)\n", which);
		ret_val = -1;
		goto bail;
	}

	printf("Dumping: %s\n", ipa_which_map_as_str(which));

	for ( it  = map_array[which].begin();
		  it != map_array[which].end();
		  it++ )
	{
		printf("  Key[%u|0x%08X] -> Value[%u|0x%08X]\n",
			   it->first,
			   it->first,
			   it->second,
			   it->second);
	}

bail:
	IPADBG("Out\n");

	return ret_val;
}
