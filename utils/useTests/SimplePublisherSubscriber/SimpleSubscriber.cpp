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

#include "SimpleSubscriber.h"



SimpleSubscriber::SimpleSubscriber():
	mp_participant(NULL),
	mp_subscriber(NULL)
{

}

SimpleSubscriber::~SimpleSubscriber() {
	// TODO Auto-generated destructor stub
}

bool SimpleSubscriber::init()
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10043;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true; //This is the default value but you can change this if you want
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true; //This is the default value but you can change this if you want
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true; //This is the default value but you can change this if you want
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true; //This is the default value but you can change this if you want
	PParam.builtin.domainId = 80; //MUST BE THE SAME AS IN THE PUBLISHER
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "participant2";
	mp_participant = DomainParticipant::createParticipant(PParam);
	if(mp_participant == NULL)
			return false;
	SubscriberAttributes Rparam;
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicDataType = "SimplePubSubType"; //Must be registered before the creation of the subscriber
	Rparam.topic.topicName = "SimplePubSubTopic";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 20;
	Rparam.topic.resourceLimitsQos.max_samples = 50;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

	mp_subscriber = DomainParticipant::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);
	if(mp_subscriber == NULL)
		return false;
	m_listener.mp_sub = mp_subscriber; //We copy the pointer to the subscriber to be able to read and take data from the listener directly
	return true;
}


void SimpleSubscriber::SubListener::onSubscriptionMatched(MatchingInfo info)
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

void SimpleSubscriber::SubListener::onNewDataMessage()
{
	if(mp_sub!=NULL)
	{
		//YOUR CODE HERE:
		//READ OR TAKE DATA
	}
}

void SimpleSubscriber::run()
{
	cout << "Type something and press enter to stop the subscriber: ";
	int aux;
	std::cin >> aux;

}

