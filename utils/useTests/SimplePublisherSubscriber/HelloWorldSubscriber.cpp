/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"



HelloWorldSubscriber::HelloWorldSubscriber() {

	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10043;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.builtin.domainId = 80;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "participant2";
	mp_participant = DomainParticipant::createParticipant(PParam);

	SubscriberAttributes Rparam;
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicDataType = "HelloWorldType";
	Rparam.topic.topicName = "HelloWorldTopic";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 20;
	Rparam.topic.resourceLimitsQos.max_samples = 50;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

	mp_subscriber = DomainParticipant::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);
	m_listener.mp_sub = mp_subscriber;

}

HelloWorldSubscriber::~HelloWorldSubscriber() {
	// TODO Auto-generated destructor stub
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(MatchingInfo info)
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

void HelloWorldSubscriber::SubListener::onNewDataMessage()
{
	if(mp_sub!=NULL)
	{
		while(mp_sub->takeNextData((void*)&m_Hello,&m_info))
		{
			if(m_info.sampleKind == ALIVE)
			{
				cout << "Message "<<m_Hello.message()<< " "<< m_Hello.index()<< " RECEIVED"<<endl;
			}
		}
	}
}

