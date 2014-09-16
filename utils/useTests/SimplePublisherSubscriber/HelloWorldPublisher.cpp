/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"


HelloWorldPublisher::HelloWorldPublisher()
{

	m_Hello.index(0);
	m_Hello.message("HelloWorld");
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.builtin.domainId = 80;
	TIME_INFINITE(PParam.builtin.leaseDuration);
		PParam.sendSocketBufferSize = 8712;
		PParam.listenSocketBufferSize = 17424;
	PParam.name = "participant1";
	mp_participant = DomainParticipant::createParticipant(PParam);

	PublisherAttributes Wparam;
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicDataType = "HelloWorldType";
	Wparam.topic.topicName = "HelloWorldTopic";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 20;
	Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.resourceLimitsQos.allocated_samples = 20;
	Wparam.times.heartbeatPeriod.seconds = 2;
	Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_publisher = DomainParticipant::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
}

HelloWorldPublisher::~HelloWorldPublisher()
{
	// TODO Auto-generated destructor stub
}

void HelloWorldPublisher::PubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		n_matched++;
		cout << "Publisher matched"<<endl;
	}
	else
	{
		n_matched--;
		cout << "Publisher unmatched"<<endl;
	}
}

bool HelloWorldPublisher::publish()
{
	if(m_listener.n_matched>0)
	{
		m_Hello.index(m_Hello.index()+1);
		mp_publisher->write((void*)&m_Hello);
		cout << "Message: "<<m_Hello.message()<< " "<< m_Hello.index()<< " SENT"<<endl;
		return true;
	}
	return false;
}

