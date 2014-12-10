/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/Domain.h"


HelloWorldSubscriber::HelloWorldSubscriber() {

	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10043;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.rtps.builtin.domainId = 80;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
	PParam.rtps.sendSocketBufferSize = 8712;
	PParam.rtps.listenSocketBufferSize = 17424;
	PParam.rtps.setName("Participant_sub");
	mp_participant = Domain::createParticipant(PParam);
	//REGISTER THE TYPE

	Domain::registerType(mp_participant,&m_type);
	//CREATE THE SUBSCRIBER
	SubscriberAttributes Rparam;
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicDataType = "HelloWorldType";
	Rparam.topic.topicName = "HelloWorldTopic";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 20;
	Rparam.topic.resourceLimitsQos.max_samples = 50;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Locator_t loc;
	loc.port = 11111;
	Rparam.unicastLocatorList.push_back(loc);
	loc.port = 11112;
	Rparam.unicastLocatorList.push_back(loc);
	mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);

}

HelloWorldSubscriber::~HelloWorldSubscriber() {
	// TODO Auto-generated destructor stub
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		n_matched++;
		cout << "Subscriber matched"<<endl;
	}
	else
	{
		n_matched--;
		cout << "Subscriber unmatched"<<endl;
	}
}

void HelloWorldSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
	while(sub->takeNextData((void*)&m_Hello,&m_info))
	{
		if(m_info.sampleKind == ALIVE)
		{
			cout << "Message "<<m_Hello.message()<< " "<< m_Hello.index()<< " RECEIVED"<<endl;
		}
	}

}

