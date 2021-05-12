/*
 * Copyright (c) 2017,2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef _FILTERING_TABLE_
#define _FILTERING_TABLE_

#include "Constants.h"
#include "Filtering.h"

/*This Class Encapsulate Filtering Table and Filtering Rules.
 *It allows the user to easily manipulate rules and Tables.
 */
class IPAFilteringTable {
    public:
	IPAFilteringTable();
	~IPAFilteringTable();

	bool Init(ipa_ip_type ipFamily, ipa_client_type pipeNo,
		  uint8_t isGlobal, uint8_t numOfRulesInTable,
		  uint8_t commit = true);

	/*This Function Frees the Filtering Table and all it's content.
	 *This Function will always return TRUE;
	 */
	void Destructor();

	bool GeneratePresetRule(uint8_t preset,
				ipa_flt_rule_add &flt_rule_entry);
	bool GeneratePresetRule(uint8_t preset,
				ipa_flt_rule_add_v2 &flt_rule_entry);
	uint8_t AddRuleToTable(ipa_flt_rule_add flt_rule_entry);
	uint8_t AddRuleToTable(ipa_flt_rule_add_v2 flt_rule_entry);
	bool WriteRuleToEndOfTable(const ipa_flt_rule_add *flt_rule_entry);

	/*Warning!!!
	 *Take care when using this function.
	 *The Returned pointer existence is guaranteed only as
	 *long as no other method of this class is called.
	 */
	const ipa_flt_rule_add *ReadRuleFromTable(uint8_t index);

	/*Warning!!!
	 *Take care when using this function
	 *The Returned pointer existence is guaranteed only
	 *as long as no other method of this class is called.
	 */
	const ipa_ioc_add_flt_rule *GetFilteringTable()
	{
		return m_pFilteringTable;
	}

	uint8_t size()
	{
		return nextRuleIndex;
	}

    private:
	void ReportError(char *message)
	{
		printf("%s\n", message);
	}
	ipa_ioc_add_flt_rule *m_pFilteringTable;
	uint8_t nextRuleIndex;
};

class IPAFilteringTable_v2 {
    public:
	IPAFilteringTable_v2();
	~IPAFilteringTable_v2();

	bool Init(ipa_ip_type ipFamily, ipa_client_type pipeNo,
		  uint8_t isGlobal, uint8_t numOfRulesInTable,
		  uint8_t commit = true);

	/*This Function Frees the Filtering Table and all it's content.
	 *This Function will always return TRUE;
	 */
	void Destructor();

	bool GeneratePresetRule(uint8_t preset,
				ipa_flt_rule_add_v2 &flt_rule_entry);
	uint8_t AddRuleToTable(ipa_flt_rule_add_v2 flt_rule_entry);

	/*Warning!!!
	 *Take care when using this function.
	 *The Returned pointer existence is guaranteed only as
	 *long as no other method of this class is called.
	 */
	const ipa_flt_rule_add_v2 *ReadRuleFromTable(uint8_t index);

	/*Warning!!!
	 *Take care when using this function
	 *The Returned pointer existence is guaranteed only
	 *as long as no other method of this class is called.
	 */
	const ipa_ioc_add_flt_rule_v2 *GetFilteringTable()
	{
		return m_pFilteringTable_v2;
	}

    private:
	void ReportError(char *message)
	{
		printf("%s\n", message);
	}
	ipa_ioc_add_flt_rule_v2 *m_pFilteringTable_v2;
	uint8_t nextRuleIndex;
};

#endif
