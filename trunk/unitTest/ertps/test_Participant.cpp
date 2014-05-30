/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_Participant.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "gtest/gtest.h"
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/dds/DomainParticipant.h"

using namespace eprosima;
namespace eprosima{
namespace rtps{


class ParticipantTest:public ::testing::Test
{
protected:

	ParticipantParams_t param;
	Participant* p;
	void SetUp()
	{
		param.domainId = 80;
		p = DomainParticipant::createParticipant(param);
	}
	void TearDown()
	{
		dds::DomainParticipant *dp = dds::DomainParticipant::getInstance();
		dp->removeParticipant(p);
		//	dp->removeParticipant();
	}
};

}
}

//TEST_F(ParticipantTest, Constructor)
//{
//	EXPECT_TRUE(p->m_guid.entityId == ENTITYID_PARTICIPANT);
//}




