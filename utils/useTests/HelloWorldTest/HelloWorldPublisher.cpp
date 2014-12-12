/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/Domain.h"



HelloWorldPublisher::HelloWorldPublisher()
{

	m_Hello.index(0);
	m_Hello.message("HelloWorld");
	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10042;
	PParam.rtps.use_IP6_to_send = true;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.rtps.builtin.domainId = 80;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
	PParam.rtps.sendSocketBufferSize = 8712;
	PParam.rtps.listenSocketBufferSize = 17424;
	PParam.rtps.setName("Participant_pub");
	mp_participant = Domain::createParticipant(PParam);

	//REGISTER THE TYPE

	Domain::registerType(mp_participant,&m_type);

	//CREATE THE PUBLISHER
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
	mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
}

HelloWorldPublisher::~HelloWorldPublisher()
{
	// TODO Auto-generated destructor stub
}

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* pub,MatchingInfo info)
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
		cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT, press enter to send another one."<<endl;
		std::cin.ignore();
		return true;
	}
	return false;
}

