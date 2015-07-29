/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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




