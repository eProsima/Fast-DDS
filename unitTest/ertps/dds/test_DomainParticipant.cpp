/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_DomainParticipant.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */



#include "gtest/gtest.h"
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/DomainParticipant.h"

using namespace eprosima;
namespace eprosima{

using namespace rtps;

namespace dds{


class DomainParticipantTest:public ::testing::Test
{
protected:

	ParticipantParams_t param;
	WriterParams_t wparam;
	ReaderParams_t rparam;
	Participant* p;
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
		DomainParticipant::removePublisher(p,pub);
		DomainParticipant::removeSubscriber(p,sub);
		DomainParticipant::removeParticipant(p);
	}
};

}
}

TEST_F(DomainParticipantTest, Participant)
{
	param.name = "participant1";
	p = DomainParticipant::createParticipant(param);
	EXPECT_TRUE(p!=NULL);
	EXPECT_TRUE(p->m_guid.entityId == ENTITYID_PARTICIPANT);

	Participant* p2 = DomainParticipant::createParticipant(param);
	EXPECT_TRUE(p2==NULL) << "Two parameters with the same name were allowed to be created"<<endl;
	EXPECT_FALSE(DomainParticipant::removeParticipant(p2));

	EXPECT_TRUE(DomainParticipant::removeParticipant(p));

}


