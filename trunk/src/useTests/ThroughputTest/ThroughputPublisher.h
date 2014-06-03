/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputPublisher.h
 *
 *  Created on: Jun 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include "eprosimartps/rtps_all.h"

class ThroughputPublisher
{
public:
	ThroughputPublisher();
	virtual ~ThroughputPublisher(){DomainParticipant::stopAll();}
	Participant* mp_par;
	Publisher* mp_datapub;
	Subscriber* mp_commandsub;
	eClock m_Clock;

	Time_t m_t1,m_t2;
	uint64_t m_overhead;
	boost::interprocess::interprocess_semaphore sema;
	class ThrouputPublisherListener:public PublisherListener
	{
		void onPublicationMatched()
		{
			cout << B_RED << "Publication Matched"<<DEF<<endl;
		}
	}m_DataPubListener;

	class ThrouputPublisherCommandListener:public SubscriberListener
	{
		void onSubscriptionMatched()
		{
			cout << B_MAGENTA << "Subscription Matched"<<DEF<<endl;
		}
	}m_CommandSubListener;
	void run();
};

ThroughputPublisher::ThroughputPublisher():
		sema(0)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant1";
	mp_par = DomainParticipant::createParticipant(PParam);

	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (Time2MicroSec(m_t2)-Time2MicroSec(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
	//PUBLISHER
	PublisherAttributes Wparam;
	Wparam.historyMaxSize = 1100;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	mp_datapub = DomainParticipant::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_DataPubListener);
	//SUBSCRIBER
	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = 1100;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommand";
	mp_commandsub = DomainParticipant::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);
}

void ThroughputPublisher::run()
{

}








#endif /* THROUGHPUTPUBLISHER_H_ */
