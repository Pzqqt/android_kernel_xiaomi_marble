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

#include <stdio.h>
#include <algorithm>	// std::find
#include <vector>	// std::vector
#include <string>
#include <errno.h>
#include <ctime>
#include <sstream>
#include "TestManager.h"
#include "TestsUtils.h"
#include <fcntl.h>
#include <unistd.h>
#include "ipa_test_module.h"
#include <sys/ioctl.h>

using namespace std;

/* Global static pointer used to ensure a single instance of the class. */
TestManager* TestManager::m_instance = NULL;


#ifdef HAVE_LIBXML

TestsXMLResult::TestsXMLResult()
{
	xmlNodePtr node;

	// initialize xml report document and add a root to node it
	m_XML_doc_ptr = xmlNewDoc(BAD_CAST "1.0");
	if (m_XML_doc_ptr == NULL){
		printf("error on allocation xml doc\n");
		exit(-1);
	}

	node = xmlNewNode(NULL, BAD_CAST "testsuites");
	if (!node) {
		printf("failed to allocate XML node\n");
		exit (-1);
	}
	xmlDocSetRootElement(m_XML_doc_ptr, node);
}

TestsXMLResult::~TestsXMLResult()
{
	if (m_XML_doc_ptr)
		xmlFreeDoc(m_XML_doc_ptr);
	xmlCleanupParser();
}

/*
 * Returns xmlPtr to testsuite element node, if doesn't exist
 * creates one by that name
 */
xmlNodePtr TestsXMLResult::GetSuiteElement(const string& suite_name)
{
	xmlNodePtr root_node, suite_node, new_child_node;

	if (!m_XML_doc_ptr) {
		printf("no xml document\n");
		return NULL;
	}

	root_node = xmlDocGetRootElement(m_XML_doc_ptr);
	suite_node = xmlFirstElementChild(root_node);
	while (suite_node)
	{
		/* get suite name */
		xmlChar *val = xmlGetProp(suite_node, BAD_CAST "name");

		/* change xmlCHar* to string */
		string node_suite_name(reinterpret_cast<char*>(val));
		xmlFree(val);			//free val allocated memory

		if (node_suite_name == suite_name)
			return suite_node;
		else suite_node = suite_node->next;
	}

	/* If we got here no suitable suite name was found,
	 * so we create a new suite element and return it
	 */
	new_child_node = xmlNewChild(root_node, NULL, BAD_CAST "testsuite", BAD_CAST "");
	if (!new_child_node) {
		printf("failed creating new XML node\n");
		return NULL;
	}
	xmlSetProp(new_child_node, BAD_CAST "name", BAD_CAST suite_name.c_str());

	return xmlGetLastChild(root_node);
}

/*
 * Creates new testcase element
 */
void TestsXMLResult::AddTestcase(const string &suite_nm, const string &test_nm,
	double runtime, bool pass)
{
	xmlNodePtr suite_node, new_testcase, fail_node;
	ostringstream runtime_str;

	if (!suite_nm.size() || !test_nm.size()) {
		printf("Input error: suite_nm size %d , test_nm size %d",
			suite_nm.size(), test_nm.size());
		exit(-1);
	}

	suite_node = GetSuiteElement(suite_nm);
	if (!suite_node) {
		printf("failed getting suite element\n");
		exit(-1);
	}

	/* Create new testcase element as son to suite element */
	new_testcase = xmlNewChild(suite_node, NULL, BAD_CAST "testcase", NULL);
	if (!new_testcase) {
		printf("failed creating XML new child for testcase\n");
		exit(-1);
	}
	xmlSetProp(new_testcase, BAD_CAST "name", BAD_CAST test_nm.c_str());

	runtime_str << runtime;
	xmlSetProp(new_testcase, BAD_CAST "time", BAD_CAST runtime_str.str().c_str());

	if (!pass) {
		fail_node = xmlNewChild(new_testcase, NULL, BAD_CAST "failure", NULL);
		if (!fail_node) {
			printf("failed creating fail node\n");
			exit(-1);
		}
	}
}

/*
 * Prints the XML tree to file
 */
void TestsXMLResult::GenerateXMLReport(void)
{
	if (!m_XML_doc_ptr) {
		printf("no xml document\n");
		return;
	}

	xmlSaveFormatFileEnc(XUNIT_REPORT_PATH_AND_NAME, m_XML_doc_ptr, "UTF-8", 1);
}

#else /* HAVE_LIBXML */

TestsXMLResult::TestsXMLResult() {}
TestsXMLResult::~TestsXMLResult() {}
void TestsXMLResult::AddTestcase(const string &suite_nm, const string &test_nm,
	double runtime, bool pass) {}
void TestsXMLResult::GenerateXMLReport(void)
{
	printf("No XML support\n");
}

#endif /* HAVE_LIBXML */

TestManager::TestManager(
    const char* nat_mem_type_ptr)
{
	m_testList.clear();
	m_failedTestsNames.clear();
	m_numTestsFailed = 0;
	m_numTestsRun = 0;
	FetchIPAHwType();
	m_nat_mem_type_ptr = nat_mem_type_ptr;
}

////////////////////////////////////////////////////////////////////////////////////////////

TestManager::~TestManager()
{
	m_testList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////

TestManager* TestManager::GetInstance(
	const char* nat_mem_type_ptr)
{
	if (!m_instance)   // Only allow one instance of class to be generated.
		m_instance = new TestManager(nat_mem_type_ptr);

	return m_instance;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TestManager::Register(TestBase &test)
{
	m_testList.push_back(&test);
}

////////////////////////////////////////////////////////////////////////////////////////////

bool TestManager::Run(vector<string> testSuiteList, vector<string> testNameList)
{
	TestBase *test = NULL;
	bool pass = true;
	vector<string>::iterator testIter;
	vector<string>::iterator testSuiteIter;
	bool runTest = false;
	clock_t begin_test_clk, end_test_clk;
	double test_runtime_sec = 0, total_time_sec = 0;
	TestsXMLResult xml_res;

	if (m_testList.size() == 0)
		return false;

	/* PrintRegisteredTests(); */

	for (unsigned int i = 0 ; i < m_testList.size() ; i++ , runTest = false) {
		pass = true;
		test = m_testList[i];

		// Run only tests from the list of test suites which is stated in the command
		// line. In case the list is empty, run all tests.
		if (testSuiteList.size() > 0) {
			for (unsigned int j = 0; j < test->m_testSuiteName.size(); j++) {
				testSuiteIter = find(testSuiteList.begin(), testSuiteList.end(), test->m_testSuiteName[j]);
				if (testSuiteIter != testSuiteList.end()) {
					runTest = true;
				}
			}
		}

		// We also support test by name
		if (testNameList.size() > 0) {
			testIter = find(testNameList.begin(), testNameList.end(), test->m_name);
			if (testIter != testNameList.end())
				runTest = true;
		}

		// Run the test only if it's applicable to the current IPA HW type / version
		if (runTest) {
			if (!(m_IPAHwType >= test->m_minIPAHwType && m_IPAHwType <= test->m_maxIPAHwType))
				runTest = false;
		}

		if (!runTest)
			continue;

		printf("\n\nExecuting test %s\n", test->m_name.c_str());
		printf("Description: %s\n", test->m_description.c_str());

		printf("Setup()\n");
		begin_test_clk = clock();
		test->SetMemType(GetMemType());
		pass &= test->Setup();

		//In case the test's setup did not go well it will be a bad idea to try and run it.
		if (true == pass)
		{
			printf("Run()\n");
			pass &= test->Run();
		}

		printf("Teardown()\n");
		pass &= test->Teardown();

		end_test_clk = clock();
		test_runtime_sec = double(end_test_clk - begin_test_clk) / CLOCKS_PER_SEC;
		total_time_sec += test_runtime_sec;

		if (pass)
		{
			m_numTestsRun++;
			PrintSeparator(test->m_name.size());
			printf("Test %s PASSED ! time:%g\n", test->m_name.c_str(), test_runtime_sec);
			PrintSeparator(test->m_name.size());
		}
		else
		{
			m_numTestsRun++;
			m_numTestsFailed++;
			m_failedTestsNames.push_back(test->m_name);
			PrintSeparator(test->m_name.size());
			printf("Test %s FAILED ! time:%g\n", test->m_name.c_str(), test_runtime_sec);
			PrintSeparator(test->m_name.size());
		}

		xml_res.AddTestcase(test->m_testSuiteName[0], test->m_name, test_runtime_sec, pass);
	} // for

	// Print summary
	printf("\n\n");
	printf("==================== RESULTS SUMMARY ========================\n");
	printf("%zu tests were run, %zu failed, total time:%g.\n", m_numTestsRun, m_numTestsFailed, total_time_sec);
	if (0 != m_numTestsFailed) {
		printf("Failed tests list:\n");
		for (size_t i = 0; i < m_numTestsFailed; i++) {
			printf("        %s\n", m_failedTestsNames[i].c_str());
			m_failedTestsNames.pop_back();
		}
	}
	printf("=============================================================\n");
	xml_res.GenerateXMLReport();

	return pass;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TestManager::PrintSeparator(size_t len)
{
	string separator;

	for (size_t i = 0; i < len + 15; i++) {
		separator += "-";
	}

	printf("%s\n", separator.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////

TestManager::TestManager(TestManager const&)
{

}

////////////////////////////////////////////////////////////////////////////////////////////

TestManager& TestManager::operator=(TestManager const&)
{
	return *m_instance;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TestManager::PrintRegisteredTests()
{
	printf("Test list: (%zu registered)\n", m_testList.size());
	for (unsigned int i = 0; i < m_testList.size(); i++) {
		printf("%d) name = %s, suite name = %s, regression = %d\n", i, m_testList[i]->m_name.c_str(),
		       m_testList[i]->m_testSuiteName[0].c_str(), m_testList[i]->m_runInRegression);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void TestManager::FetchIPAHwType()
{
	int fd;

	// Open ipa_test device node
	fd = open("/dev/ipa_test" , O_RDONLY);
	if (fd < 0) {
		printf("Failed opening %s. errno %d: %s\n", "/dev/ipa_test", errno, strerror(errno));
		m_IPAHwType = IPA_HW_None;
		return;
	}

	printf("%s(), fd is %d\n", __FUNCTION__, fd);

	m_IPAHwType = (enum ipa_hw_type)ioctl(fd, IPA_TEST_IOC_GET_HW_TYPE);
	if (-1 == m_IPAHwType) {
		printf("%s(), IPA_TEST_IOC_GET_HW_TYPE ioctl failed\n", __FUNCTION__);
		m_IPAHwType = IPA_HW_None;
	}

	printf("%s(), IPA HW type (version) = %d\n", __FUNCTION__, m_IPAHwType);
	close(fd);
}


////////////////////////////////////////////////////////////////////////////////////////////
