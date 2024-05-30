/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef _IPA_LINUX_TESTS_HR_TESTS_H_
#define _IPA_LINUX_TESTS_HR_TESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "Constants.h"
#include "linux/msm_ipa.h"
#include "TestsUtils.h"
#include "HeaderRemovalTestFixture.h"

/*This test will send a DMUX with IP packet and check it TOS field thus
 *validating the the header was removed.
 */
class HeaderRemovalTOSCheck:public HeaderRemovalTestFixture
{
public:
	/*This Constructor will be use to specify some test description.*/
	HeaderRemovalTOSCheck();

	/*This method will send a an IP packet with
	 * DMUX header and create a rule
	 * */
	virtual bool Run();
};

/*This test will send a DMUX with IP packet and see if it filtered
 *by meta data contained in link layer header as expected
 */
class HeaderRemovalMetaDataFiltering:HeaderRemovalTestFixture
{
public:
	/* his Constructor will be use to specify
	 * some test description.*/
	HeaderRemovalMetaDataFiltering();

	virtual bool Run();

};

#endif
