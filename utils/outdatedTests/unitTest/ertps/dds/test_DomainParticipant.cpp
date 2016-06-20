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
 * @file test_DomainRTPSParticipant.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */



#include "gtest/gtest.h"
#include "fastrtps/rtps_all.h"
#include "fastrtps/dds/DomainRTPSParticipant.h"

using namespace eprosima;
namespace eprosima{

using namespace rtps;

namespace dds{


class DomainRTPSParticipantTest:public ::testing::Test
{
protected:

	RTPSParticipantParams_t param;
	WriterParams_t wparam;
	ReaderParams_t rparam;
	RTPSParticipant* p;
	Publisher* pub;
	Subscriber* sub;
	void SetUp()
	{
		param.domainId = 80;
	}
	void TearDown()
	{

	}

	void removeAll()
	{
		DomainRTPSParticipant::removePublisher(p,pub);
		DomainRTPSParticipant::removeSubscriber(p,sub);
		DomainRTPSParticipant::removeRTPSParticipant(p);
	}
};

}
}

TEST_F(DomainRTPSParticipantTest, RTPSParticipant)
{
	param.name = "RTPSParticipant1";
	p = DomainRTPSParticipant::createRTPSParticipant(param);
	EXPECT_TRUE(p!=NULL);
	EXPECT_TRUE(p->m_guid.entityId == ENTITYID_RTPSParticipant);

	RTPSParticipant* p2 = DomainRTPSParticipant::createRTPSParticipant(param);
	EXPECT_TRUE(p2==NULL) << "Two parameters with the same name were allowed to be created"<<endl;
	EXPECT_FALSE(DomainRTPSParticipant::removeRTPSParticipant(p2));

	EXPECT_TRUE(DomainRTPSParticipant::removeRTPSParticipant(p));

}


