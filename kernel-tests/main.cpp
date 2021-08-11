/*
 * Copyright (c) 2017-2018,2020 The Linux Foundation. All rights reserved.
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

#include <getopt.h>

#include "Logger.h"
#include "TestManager.h"
#include "TestsUtils.h"
#include <stdio.h>
#include <iostream>
#include <set>
///////////////////////////////////////////////////////////

Logger g_Logger(LOG_ERROR);
#define CHOOSER_MODE   "--chooser"
#define SHOW_TEST_FLAG "--show_tests"
#define SHOW_SUIT_FLAG "--show_suites"
#define RUN_TEST_FLAG  "--test"
#define RUN_SUIT_FLAG  "--suite"
string sFormat = "ip_accelerator <control_flag> <suit/name>, ..., <suit/name>\n"
							"contorl_flag = "   RUN_TEST_FLAG  " or "  RUN_SUIT_FLAG "\n"
							"ip_accelerator " SHOW_TEST_FLAG  "\n"
							"ip_accelerator " SHOW_SUIT_FLAG  "\n"
							"or ip_accelerator --chooser "
							"for menu chooser interface\n";
#define MAX_SUITES 17

#undef strcasesame
#define strcasesame(x, y) \
	(! strcasecmp((x), (y)))

#undef legal_nat_mem_type
#define legal_nat_mem_type(mt) \
	( strcasesame(mt, "DDR") || \
	  strcasesame(mt, "SRAM") || \
	  strcasesame(mt, "HYBRID") )

TestManager *testmanager = NULL;

enum ipa_test_type{
	TEST = 1,
	SUITE,
	EXIT,
	MAX_TYPE
};

const char *ipa_hw_type_name[] = {
	"None",
	"1.0",
	"1.1",
	"2.0",
	"2.1",
	"2.5/2.6",
	"2.6L",
	"Unused",
	"Unused",
	"Unused",
	"3.0",
	"3.1",
	"3.5",
	"3.5.1",
	"4.0",
	"4.1",
	"4.2",
	"4.5",
	"4.9",
	"5.0",
	"5.1",
	"MAX"
};

void BuildRegressionTestSuite()
{
	TestBase *test;

	for (unsigned int i = 0; i < testmanager->m_testList.size(); i++) {
		test = testmanager->m_testList[i];
		if (test->m_runInRegression) {
			test->m_testSuiteName.push_back("Regression");
		}
	}
}


///////////////////////////////////////////////////////////
void showTests()
{
	TestBase *test = testmanager->m_testList[0];

	for (unsigned  i = 0; i < testmanager->m_testList.size(); i++) {
		test = testmanager->m_testList[i];
		string name = test->m_name, index = test->m_testSuiteName[0];

		printf("%d) %s (part of %s suite" ,i+1, name.c_str(), index.c_str());
		for (unsigned j = 0; j < test->m_testSuiteName.size(); ++j) {
			if ( test->m_testSuiteName[j] == index)
				continue;
			printf(", %s suite",
				 test->m_testSuiteName[j].c_str());
			index = test->m_testSuiteName[j];
		}
		printf("), (%s <= HW Version <= %s)\n",
		       ipa_hw_type_name[test->m_minIPAHwType],
		       ipa_hw_type_name[test->m_maxIPAHwType]);
	}

	// Example:
	// 15) DmaModeMBIMggregationLoopTest (part of DmaMbim16Agg suite), (1.0 <= HW Version <= 2.1)
}

void showSuits()
{
	TestBase *test;
	std::set<string> suiteSet;
	int suiteIndex = 1;

	test = testmanager->m_testList[0];

	for (unsigned  i = 0; i < testmanager->m_testList.size(); i++) {
		test = testmanager->m_testList[i];
		for (unsigned j = 0; j < test->m_testSuiteName.size() ; j++)
			suiteSet.insert(test->m_testSuiteName[j]);
	}

	for (std::set<string>::iterator i = suiteSet.begin();
			i != suiteSet.end(); suiteIndex++) {
		printf("%d) %s\n", suiteIndex, (*i).c_str());
		++i;
	}
}

void preparTests(int argc, char* argv[],
		vector<string>& list)
{
	for (int i = 2; i < argc; i++)
		list.push_back(argv[i]);
}

const char* getSuite(int suite_num) {
	int count = 0, suiteIndex = 1;
	string result = "error";
	TestBase *test = testmanager->m_testList[0];
	set<string> suiteSet;

	if (testmanager->m_testList.size() < 1)
		return NULL;

	if (count == suite_num)
		return testmanager->m_testList[0]->m_testSuiteName[0].c_str();

	for (unsigned  i = 0; i < testmanager->m_testList.size(); i++) {
		test = testmanager->m_testList[i];
		for (unsigned j = 0; j < test->m_testSuiteName.size() ; j++)
			suiteSet.insert(test->m_testSuiteName[j]);
	}

	for (std::set<string>::iterator i = suiteSet.begin();
			i != suiteSet.end(); suiteIndex++) {
		printf("%d) %s\n", suiteIndex, (*i).c_str());
		if (suiteIndex == suite_num)
			return (*i).c_str();
		++i;
	}
	return NULL;
}

int chooserMode() {
	vector<string> testSuiteList;
	vector<string> testNameList;
	unsigned int test_num;
	int suite_num;
	int type;
	TestBase *test;
	const char* res;
	int result = 0;
	char input_str[4];
	char* temp;

	printf("Welcome to the ip_accelerator\nChoose an option:\n");
	printf("1) Run tests\n2) Run suites\n3) Exit\nChoose an option: ");
	temp = fgets(input_str, sizeof(input_str), stdin);
	if (!temp) {
		printf("Error: fgets returned nullptr !!");
		return -1;
	}
	type = atoi(input_str);
	switch((enum ipa_test_type)type) {
	case TEST:
		BuildRegressionTestSuite();
		showTests();
		printf("Choose which test you wish to run: \n");
		fflush(stdin);
		temp = fgets(input_str, sizeof(input_str), stdin);
		if (!temp) {
			printf("Error: fgets returned nullptr !!");
			return -1;
		}
		test_num = atoi(input_str);
		if ( test_num > testmanager->m_testList.size()) {
			printf("Invalid test number. Try again\n");
			result = -1;
			break;
		}
		test = testmanager->m_testList[test_num-1];
		printf("Running Test %s\n",
				test->m_name.
				c_str());
		testNameList.push_back(test->m_name.c_str());
		BuildRegressionTestSuite();
		testmanager->Run(testSuiteList, testNameList);
		testNameList.clear();
		break;
	case SUITE:
		BuildRegressionTestSuite();
		showSuits();
		printf("Choose which suite you wish to run: \n");
		fflush(stdin);
		temp = fgets(input_str, sizeof(input_str), stdin);
		if (!temp) {
			printf("Error: fgets returned nullptr !!");
			return -1;
		}
		suite_num = atoi(input_str);
		if (suite_num < 0 || suite_num > MAX_SUITES) {
			printf("Invalid test number. Try again\n");
			result = -1;
			break;
		}
		res = getSuite(suite_num);
		if	(!res) {
			printf("Error in getSuite. Exiting\n");
			result = -1;
			break;
		}
		testSuiteList.push_back(res);
		testmanager->Run(testSuiteList, testNameList);
		testSuiteList.clear();
		break;
	default:
		printf("Exiting\n");
		break;
	}
	return result;
}

int scriptMode(int argc, char* argv[]) {
	vector<string> testSuiteList;
	vector<string> testNameList;
	string sControlFlag;

	if (argc < 2) {
		printf("please use correct format:\n%s", sFormat.c_str());
		return -1;
	}

	sControlFlag = argv[1];

	if (sControlFlag.find("--") == string::npos) {
		printf("please use correct format:\n%s", sFormat.c_str());
		return -1;
	}

	BuildRegressionTestSuite();

	if (sControlFlag.compare(SHOW_TEST_FLAG) == 0) {
		showTests();
		return 0;
	} else if (sControlFlag.compare(SHOW_SUIT_FLAG) == 0) {
		showSuits();
		return 0;
	}

	if (sControlFlag.compare(RUN_TEST_FLAG) == 0) {
		preparTests(argc, argv, testNameList);
	} else if (sControlFlag.compare(RUN_SUIT_FLAG) == 0) {
		preparTests(argc, argv, testSuiteList);
	} else {
		printf("please use correct format:\n%s", sFormat.c_str());
		return -1;
	}

	testmanager->Run(testSuiteList, testNameList);
	return 0;
}

int main(int argc, char* argv[])
{
	string nat_mem_type = DFLT_NAT_MEM_TYPE;

	int c, result = 0, what = 0;

	int opt_idx = 0;

	struct option opts[] = {
		/* These options set a flag. */
		{"chooser",     no_argument,       &what, 1},
		{"show_tests",  no_argument,       &what, 2},
		{"show_suites", no_argument,       &what, 3},
		{"test",        no_argument,       &what, 4},
		{"suite",       no_argument,       &what, 5},
		{"mem",         required_argument, 0,    'm'},
		{0, 0, 0, 0}
	};

	if (argc <= 1) {
		printf("please use correct format:\n%s", sFormat.c_str());
		return -1;
	}

	while ( (c = getopt_long(argc, argv, "", opts, &opt_idx)) != -1 )
	{
		switch ( c )
		{
		case 0:
			break;
		case 'm':
			if ( legal_nat_mem_type(optarg) )
			{
				nat_mem_type = optarg;
			}
			else
			{
				fprintf(stderr, "Illegal: --mem %s\n", optarg);
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "Illegal command line argument passed\n");
			printf("please use correct format:\n%s", sFormat.c_str());
			exit(1);
		}
	}

	if ( what == 0 ) {
		printf("please use correct format:\n%s", sFormat.c_str());
		return -1;
	}

	argc = 2;

	switch ( what )
	{
	case 1:
		argv[1] = (char*) CHOOSER_MODE;
		break;
	case 2:
		argv[1] = (char*) SHOW_TEST_FLAG;
		break;
	case 3:
		argv[1] = (char*) SHOW_SUIT_FLAG;
		break;
	case 4:
		argv[1] = (char*) RUN_TEST_FLAG;
		break;
	case 5:
		argv[1] = (char*) RUN_SUIT_FLAG;
		break;
	default:
		printf("please use correct format:\n%s", sFormat.c_str());
		exit(1);
	}

	testmanager = TestManager::GetInstance(nat_mem_type.c_str());

	string sControlFlag = argv[1];

	if (sControlFlag.compare(CHOOSER_MODE) == 0) {
		result = chooserMode();
	} else {
		result = scriptMode(argc, argv);
	}
	return result;
}//main

////////////////////////////////////////////////////////////////////

