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

#include "IPAFilteringTable.h"
#include <cstring>
#include "TestsUtils.h"

IPAFilteringTable::IPAFilteringTable () : // C'tor
			m_pFilteringTable(NULL),
				nextRuleIndex(0) {}

bool IPAFilteringTable::Init(
	ipa_ip_type     ipFamily,
	ipa_client_type pipeNo,
	uint8_t         isGlobal,
	uint8_t         numOfRulesInTable,
	uint8_t         commit)
{
	if (NULL != m_pFilteringTable) {
		char message[256] = {0};
		snprintf(message, sizeof(message), "Error in Function %s, m_pFilteringTable==0x%p, must be NULL, Please call D'tor prior to calling () %s.",
				__FUNCTION__,m_pFilteringTable,__FUNCTION__);
		ReportError(message);
		return false;
	}

	if (numOfRulesInTable < 1) {
		char message[256] = {0};
		snprintf(message, sizeof(message),"Error in Function %s, numberOfRulesInTable==%d must be  > 0",
				__FUNCTION__,numOfRulesInTable);
		ReportError(message);
		return false;
	}

	m_pFilteringTable = (struct ipa_ioc_add_flt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_flt_rule) +
				numOfRulesInTable *sizeof(struct ipa_flt_rule_add));

	if (NULL ==  m_pFilteringTable) {
		char message[256] = {0};
		snprintf(message, sizeof(message),"Error in Function %s, Failed to allocate %d filter rules in Filtering Table",__FUNCTION__,numOfRulesInTable);
		ReportError(message);
		return false;
	}

	m_pFilteringTable->commit    = commit;
	m_pFilteringTable->ep        = pipeNo;
	m_pFilteringTable->global    = isGlobal;
	m_pFilteringTable->ip        = ipFamily;
	m_pFilteringTable->num_rules = (uint8_t)(numOfRulesInTable);

	return true;
}

bool IPAFilteringTable::GeneratePresetRule(uint8_t preset,ipa_flt_rule_add &flt_rule_entry)
{
	memset(&flt_rule_entry,0,sizeof(ipa_flt_rule_add)); // Zero All Fields

	switch (preset)
	{
	case 0: // in Preset 0 the Filtering Rule is matches all (bypass)
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		break;
	case 1:
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		//flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		break;
	default:
		char message[256] = {0};
		snprintf(message, sizeof(message),"Error in Function %s, preset=%d, is not supported.",__FUNCTION__,preset);
		ReportError(message);
		return false;
	}
	return true;
}

uint8_t IPAFilteringTable::AddRuleToTable(ipa_flt_rule_add flt_rule_entry)
{
	if (NULL == m_pFilteringTable) {
		char message[256] = {0};
		snprintf(message, sizeof(message),"Error in Function %s, m_pFilteringTable==NULL, Please call Init() prior to calling %s().",__FUNCTION__,__FUNCTION__);
		ReportError(message);
		return -1;
	}

	if (nextRuleIndex >= m_pFilteringTable->num_rules) {
		char message[256] = {0};
		snprintf(message, sizeof(message),"Error in Function %s, ruleIindex==%d while, No. of Rules in Filtering Table is %d. Please use IPAFilteringTable::WriteRule().",
				__FUNCTION__,nextRuleIndex,m_pFilteringTable->num_rules);
		ReportError(message);
		return -1;
	}
	struct ipa_flt_rule_add *pFilteringRule = &(m_pFilteringTable->rules[nextRuleIndex]);
	memcpy(pFilteringRule,&flt_rule_entry,sizeof(ipa_flt_rule_add));
	nextRuleIndex++;
	return(nextRuleIndex-1);
}

const ipa_flt_rule_add * IPAFilteringTable::ReadRuleFromTable(uint8_t index)
{
	if (index < nextRuleIndex)
		return (&(m_pFilteringTable->rules[index]));
	return NULL;
}

bool IPAFilteringTable::WriteRuleToEndOfTable(const ipa_flt_rule_add *flt_rule_entry) {
	ipa_ioc_add_flt_rule *prev_flt;

	if (NULL == m_pFilteringTable) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, m_pFilteringTable==NULL, Please call Init() prior to calling %s().", __FUNCTION__, __FUNCTION__);
		ReportError(message);
		return false;
	}

	prev_flt = m_pFilteringTable;
	m_pFilteringTable = NULL;
	m_pFilteringTable = (struct ipa_ioc_add_flt_rule *)calloc(
		1, sizeof(struct ipa_ioc_add_flt_rule) +
		((prev_flt->num_rules + 1) * sizeof(struct ipa_flt_rule_add)));

	if (NULL == m_pFilteringTable) {
		char message[256] = { 0 };
		snprintf(
			message, sizeof(message),
			"Error in Function %s, Failed to allocate %d filter rules in Filtering Table",
			__FUNCTION__, prev_flt->num_rules + 1);
		ReportError(message);
		return false;
	}

	m_pFilteringTable->commit = prev_flt->commit;
	m_pFilteringTable->ep = prev_flt->ep;
	m_pFilteringTable->global = prev_flt->global;
	m_pFilteringTable->ip = prev_flt->ip;
	m_pFilteringTable->num_rules = prev_flt->num_rules + 1;

	for (int i = 0; i < prev_flt->num_rules; i++) {
			memcpy(&(m_pFilteringTable->rules[i]), &(prev_flt->rules[i]),
				sizeof(ipa_flt_rule_add));
	}
	free(prev_flt);

	struct ipa_flt_rule_add *pFilteringRule = &(m_pFilteringTable->rules[nextRuleIndex]);
	memcpy(pFilteringRule, flt_rule_entry, sizeof(ipa_flt_rule_add));
	nextRuleIndex++;
	return true;

}

//This Function Frees the Filtering Table and all it's content.
//This Function will always return TRUE;
void IPAFilteringTable::Destructor()
{
	if (NULL != m_pFilteringTable) {
		free (m_pFilteringTable);
		printf("Filtering Table Freed\n");
	}
	m_pFilteringTable = NULL;
	nextRuleIndex = 0;
}

IPAFilteringTable::~IPAFilteringTable()
{
	Destructor();
}

/* V2 */

IPAFilteringTable_v2::IPAFilteringTable_v2() : // C'tor
	m_pFilteringTable_v2(NULL),
	nextRuleIndex(0)
{
}

bool IPAFilteringTable_v2::Init(ipa_ip_type ipFamily, ipa_client_type pipeNo, uint8_t isGlobal, uint8_t numOfRulesInTable, uint8_t commit)
{
	if (NULL != m_pFilteringTable_v2) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, m_pFilteringTable_v2==0x%p, must be NULL, Please call D'tor prior to calling () %s.",
			__FUNCTION__, m_pFilteringTable_v2, __FUNCTION__);
		ReportError(message);
		return false;
	}

	if (numOfRulesInTable < 1) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, numberOfRulesInTable==%d must be  > 0",
			__FUNCTION__, numOfRulesInTable);
		ReportError(message);
		return false;
	}

	m_pFilteringTable_v2 = (struct ipa_ioc_add_flt_rule_v2 *)
		calloc(1, sizeof(struct ipa_ioc_add_flt_rule_v2));
	m_pFilteringTable_v2->rules = (uint64_t)calloc(numOfRulesInTable, sizeof(struct ipa_flt_rule_add_v2));

	if (NULL == m_pFilteringTable_v2) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, Failed to allocate %d filter rules in Filtering Table V2", __FUNCTION__, numOfRulesInTable);
		ReportError(message);
		return false;
	}

	m_pFilteringTable_v2->commit = commit;
	m_pFilteringTable_v2->ep = pipeNo;
	m_pFilteringTable_v2->global = isGlobal;
	m_pFilteringTable_v2->ip = ipFamily;
	m_pFilteringTable_v2->num_rules = (uint8_t)(numOfRulesInTable);
	m_pFilteringTable_v2->flt_rule_size = sizeof(struct ipa_flt_rule_add_v2);
	return true;
}

bool IPAFilteringTable_v2::GeneratePresetRule(uint8_t preset, ipa_flt_rule_add_v2 &flt_rule_entry)
{
	memset(&flt_rule_entry, 0, sizeof(ipa_flt_rule_add_v2)); // Zero All Fields

	switch (preset) {
	case 0: // in Preset 0 the Filtering Rule is matches all (bypass)
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		break;
	case 1:
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		//flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		break;
	default:
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, preset=%d, is not supported.", __FUNCTION__, preset);
		ReportError(message);
		return false;
	}
	return true;
}

uint8_t IPAFilteringTable_v2::AddRuleToTable(ipa_flt_rule_add_v2 flt_rule_entry)
{
	if (NULL == m_pFilteringTable_v2) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, m_pFilteringTable_v2==NULL, Please call Init() prior to calling %s().", __FUNCTION__, __FUNCTION__);
		ReportError(message);
		return -1;
	}

	if (nextRuleIndex >= m_pFilteringTable_v2->num_rules) {
		char message[256] = { 0 };
		snprintf(message, sizeof(message), "Error in Function %s, ruleIindex==%d while, No. of Rules in Filtering Table is %d. Please use IPAFilteringTable::WriteRule().",
			__FUNCTION__, nextRuleIndex, m_pFilteringTable_v2->num_rules);
		ReportError(message);
		return -1;
	}
	struct ipa_flt_rule_add_v2 *pFilteringRule = &(((struct ipa_flt_rule_add_v2 *)(m_pFilteringTable_v2->rules))[nextRuleIndex]);
	memcpy(pFilteringRule, &flt_rule_entry, sizeof(ipa_flt_rule_add_v2));
	nextRuleIndex++;
	return(nextRuleIndex - 1);
}

const ipa_flt_rule_add_v2 *IPAFilteringTable_v2::ReadRuleFromTable(uint8_t index)
{
	if (index < nextRuleIndex)
		return (&(((struct ipa_flt_rule_add_v2*)m_pFilteringTable_v2->rules)[index]));
	return NULL;
}

//This Function Frees the Filtering Table and all it's content.
//This Function will always return TRUE;
void IPAFilteringTable_v2::Destructor()
{
	if (NULL != m_pFilteringTable_v2) {
		free(m_pFilteringTable_v2);
		printf("Filtering Table Freed\n");
	}
	m_pFilteringTable_v2 = NULL;
	nextRuleIndex = 0;
}

IPAFilteringTable_v2::~IPAFilteringTable_v2()
{
	Destructor();
}

