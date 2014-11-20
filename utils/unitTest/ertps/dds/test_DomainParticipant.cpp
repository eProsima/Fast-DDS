/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_DomainRTPSParticipant.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */



#include "gtest/gtest.h"
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/DomainRTPSParticipant.h"

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


