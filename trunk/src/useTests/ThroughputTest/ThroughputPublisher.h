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
#include <vector>
using namespace std;

#include "eprosimartps/rtps_all.h"
#include "ThroughputTypes.h"
class ThroughputPublisher
{
public:
	ThroughputPublisher();
	virtual ~ThroughputPublisher(){DomainParticipant::stopAll();}
	Participant* mp_par;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
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
			sema.post();
		}
	}m_DataPubListener;

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

ThroughputPublisher::ThroughputPublisher():
		sema(0),ready(true)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant1";
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
	PublisherAttributes Wparam;
	Wparam.historyMaxSize = 10000;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	mp_datapub = DomainParticipant::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_DataPubListener);
	//COMMAND
	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = 20;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommandDown";
	mp_commandsub = DomainParticipant::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);

	Wparam.historyMaxSize = 20;
	Wparam.topic.topicDataType = "ThroughputCommand";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "ThroughputCommandUp";
	mp_commandpub = DomainParticipant::createPublisher(mp_par,Wparam,NULL);

	if(mp_datapub == NULL || mp_commandsub == NULL || mp_commandpub == NULL)
		ready = false;
}

void ThroughputPublisher::run()
{
	if(!ready)
		return;
	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
	uint32_t demands[] = {100,200,300,400,500,600,700,800,900};
	vector<uint32_t> demand (demands, demands + sizeof(demands) / sizeof(uint32_t) );
	LatencyType latency(1024);
	for(std::vector<uint32_t>::iterator it=demand.begin();it!=demand.end();++it)
	{
		m_Clock.setTimeNow(&m_t2);
		uint64_t timewait_us=0;
		uint32_t samples=0;
		m_Clock.setTimeNow(&m_t1);
		while(Time2MicroSec(m_t2)-Time2MicroSec(m_t1)<TESTTIME*1000000)
		{
			for(uint32_t sample=0;sample<*it;sample++)
			{
				latency.seqnum = sample;
				mp_datapub->write((void*)&latency);
			}
			samples+=*it;
			m_Clock.setTimeNow(&m_t2);
			eClock::my_sleep(10);
			timewait_us+=10000+m_overhead;
		}
		uint64_t totaltime = Time2MicroSec(m_t2)-Time2MicroSec(m_t1)-timewait_us;
	}

}




#endif /* THROUGHPUTPUBLISHER_H_ */
