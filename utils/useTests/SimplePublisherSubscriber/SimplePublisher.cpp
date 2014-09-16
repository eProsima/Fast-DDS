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

#include "SimplePublisher.h"
#include "SimplePubSubType.h"


SimplePublisher::SimplePublisher():
	mp_participant(NULL),
	mp_publisher(NULL)
{

}

SimplePublisher::~SimplePublisher()
{
	// TODO Auto-generated destructor stub
}

bool SimplePublisher::init()
{
	//CREATE PARTICIPANT
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true; //This is the default value but you can change this if you want
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true; //This is the default value but you can change this if you want
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true; //This is the default value but you can change this if you want
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true; //This is the default value but you can change this if you want
	PParam.builtin.domainId = 80;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "participant_publisher";  //You can put here the name you want
	mp_participant = DomainParticipant::createParticipant(PParam);
	if(mp_participant == NULL)
		return false;
	//CREATE PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicDataType = "SimplePubSubType";  //This type MUST be registered
	Wparam.topic.topicName = "SimplePubSubTopic";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 20;
	Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.resourceLimitsQos.allocated_samples = 20;
	Wparam.times.heartbeatPeriod.seconds = 2;
	Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_publisher = DomainParticipant::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
	if(mp_publisher == NULL)
		return false;
	return true;
}


void SimplePublisher::PubListener::onPublicationMatched(MatchingInfo info)
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

void SimplePublisher::run()
{
	while(m_listener.n_matched==0)
	{
		eClock::my_sleep(250); //Sleep 250 ms
	}

	//YOUR CODE GOES HERE, PUBLISH WHAT YOU WANT
	MyType mtype;
	mytype.index(0);
	mytype.message("My own message");
	mytype.price(3.14);
	for(int i =0;i<10;++i)
	{
		mytype.index(i);
		mytype.price(3.14*i);
		mp_publisher->write(&mytype)
	}

}

