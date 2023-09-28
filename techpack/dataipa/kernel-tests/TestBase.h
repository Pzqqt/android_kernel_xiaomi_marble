/*
 * Copyright (c) 2017,2020 The Linux Foundation. All rights reserved.
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

#ifndef _TEST_BASE_H_
#define _TEST_BASE_H_

#include <string>
#include <vector>

#include <string.h>

#define DFLT_NAT_MEM_TYPE "HYBRID"

using namespace std;

class TestBase
{
public:
	virtual bool Setup();
	/* Empty default implementation,
	 * a test does not have to implement Setup()
	 */
	virtual bool Run() = 0;
	/* A test must implement Run() */
	virtual bool Teardown();
	/* Empty default implementation,
	 * a test does not have to implement Teardown()
	 */
	void Register(TestBase & test);
	virtual ~TestBase();
	TestBase();
	void SetMemType(
		const char* mem_type = DFLT_NAT_MEM_TYPE)
	{
		m_mem_type = mem_type;
	}

	void print_buffer(
		void *data,
		size_t size,
		const char* preamble) {

		uint8_t bytes_in_line = 16;
		uint i, j, num_lines;
		char str[1024];

		num_lines = size / bytes_in_line;
		if (size % bytes_in_line > 0)
			num_lines++;

		printf(
			"%s [buffer@(0x%p) with size=(%zu)]:\n",
			(preamble) ? preamble : "Printing",
			data,
			size);

		for (i = 0 ; i < num_lines; i++) {
			str[0] = '\0';
			for (j = 0; (j < bytes_in_line) && ((i * bytes_in_line + j) < size); j++) {
				snprintf(
					str         + strlen(str),
					sizeof(str) - strlen(str),
					"%02x ",
					((unsigned char*)data)[i * bytes_in_line + j]);
			}
			printf("%s\n", str);
		}
	}

	const char* m_mem_type;
	string m_name;
	string m_description;
	vector < string > m_testSuiteName;
	/* Every test can belong to multiple test suites */
	bool m_runInRegression;
	/* Should this test be run in a regression test ? (Default is yes) */
	int m_minIPAHwType;
	/* The minimal IPA HW version which this test can run on */
	int m_maxIPAHwType;
	/* The maximal IPA HW version which this test can run on */
};
#endif
