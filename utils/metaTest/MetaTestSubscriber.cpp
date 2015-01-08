/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestSubscriber.cpp
 *
 */

#include "MetaTestSubscriber.h"

namespace eprosima {

MetaTestSubscriber::MetaTestSubscriber():mp_par(nullptr),
		mp_pub(nullptr),
		mp_sub(nullptr)
{
	// TODO Auto-generated constructor stub

}

MetaTestSubscriber::~MetaTestSubscriber() {
	// TODO Auto-generated destructor stub
}

bool MetaTestSubscriber::init()
{
	ParticipantAttributes Patt;
	Patt.rtps.builtin.domainId = 50;
	Patt.rtps.builtin.leaseDuration = c_TimeInfinite;
	Patt.rtps.setName("MetaPublisher");

	mp_par = Domain::createParticipant(Patt);

	PublisherAttributes Wparam;
	Wparam.topic.topicName = "metaTest_S2P";
	Wparam.topic.topicDataType = "MetaTestInfo";

	mp_pub = Domain::createPublisher(mp_par,Wparam,&m_publisten);


	SubscriberAttributes Rparam;
	Rparam.topic.topicName = "metaTest_P2S";
	Rparam.topic.topicDataType = "MetaTestInfo";

	mp_sub = Domain::createSubscriber(mp_par,Rparam,&m_sublisten);

	if(mp_par == nullptr ||mp_pub == nullptr || mp_sub == nullptr)
		return false;
	return true;
}

void MetaTestSubscriber::run()
{


}

} /* namespace eprosima */
