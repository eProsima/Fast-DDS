/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputSubscriber.h
 *
 *  Created on: Jun 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include "eprosimartps/rtps_all.h"

class ThroughputSubscriber
{
public:
	ThroughputSubscriber();
	virtual ~ThroughputSubscriber(){DomainParticipant::stopAll();}
	Participant* mp_par;
	Subscriber* mp_datasub;
	Publisher* mp_commandpub;
	Subscriber* mp_commandsub;
	eClock m_Clock;

	Time_t m_t1,m_t2;
	uint64_t m_overhead;
	boost::interprocess::interprocess_semaphore sema;
	class ThrouputSubscriberListener:public SubscriberListener
	{
		void onSubscriptionMatched()
		{
			cout << B_RED << "Data Subscription Matched"<<DEF<<endl;
			sema.post()
		}
	}m_DataSubListener;

	class ThrouputPublisherCommandListener:public SubscriberListener
	{
		void onSubscriptionMatched()
		{
			cout << B_MAGENTA << "Subscription Matched"<<DEF<<endl;
			sema.post();
		}
	}m_CommandSubListener;
	bool ready;

	void run();
};

ThroughputSubscriber::ThroughputSubscriber():
		sema(0),ready(true)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant2";
	mp_par = DomainParticipant::createParticipant(PParam);
	if(mp_par == NULL)
	{
		cout << "ERROR"<<endl;
		ready = false;
		return;
	}
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (Time2MicroSec(m_t2)-Time2MicroSec(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
	//PUBLISHER
	SubscriberAttributes Sparam;
	Sparam.historyMaxSize = 10000;
	Sparam.topic.topicDataType = "LatencyType";
	Sparam.topic.topicKind = NO_KEY;
	Sparam.topic.topicName = "LatencyUp";
	mp_datasub = DomainParticipant::createSubscriber(mp_par,Sparam,(SubscriberListener*)&this->m_DataSubListener);
	//COMMAND
	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = 20;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommandUp";
	mp_commandsub = DomainParticipant::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);
	PublisherAttributes Wparam;
	Wparam.historyMaxSize = 20;
	Wparam.topic.topicDataType = "ThroughputCommand";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "ThroughputCommandDown";
	mp_commandpub = DomainParticipant::createPublisher(mp_par,Wparam,NULL);

	if(mp_datasub == NULL || mp_commandsub == NULL || mp_commandpub == NULL)
		ready = false;
}

void ThroughputSubscriber::run()
{
	if(!ready)
		return;

	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
}




#endif /* THROUGHPUTSUBSCRIBER_H_ */
