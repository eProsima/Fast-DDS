// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file test_RTPSParticipant.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "gtest/gtest.h"
#include "fastrtps/rtps_all.h"
#include "fastrtps/RTPSParticipant.h"
#include "fastrtps/dds/DomainRTPSParticipant.h"

using namespace eprosima;
namespace eprosima{
namespace rtps{


class RTPSParticipantTest:public ::testing::Test
{
protected:

	RTPSParticipantParams_t param;
	RTPSParticipant* p;
	void SetUp()
	{
		param.domainId = 80;
		p = DomainRTPSParticipant::createRTPSParticipant(param);
	}
	void TearDown()
	{
		dds::DomainRTPSParticipant *dp = dds::DomainRTPSParticipant::getInstance();
		dp->removeRTPSParticipant(p);
		//	dp->removeRTPSParticipant();
	}
};

}
}

//TEST_F(RTPSParticipantTest, Constructor)
//{
//	EXPECT_TRUE(p->m_guid.entityId == ENTITYID_RTPSParticipant);
//}




