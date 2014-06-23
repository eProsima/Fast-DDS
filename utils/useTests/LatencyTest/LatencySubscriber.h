/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencySubscriber.h
 *
 */

#ifndef LATENCYSUBSCRIBER_H_
#define LATENCYSUBSCRIBER_H_

#include "eprosimartps/rtps_all.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class LatencySubscriber : public SubscriberListener
{
public:
	LatencySubscriber();
	~LatencySubscriber(){};
	Participant* m_part;
	Publisher* m_pub;
	Subscriber* m_sub;
	LatencyType* m_latency;
	SampleInfo_t m_info;
	bool test(uint32_t datasize,uint32_t n_samples);
	uint32_t n_received;
	uint32_t n_samples;
	boost::interprocess::interprocess_semaphore sema;

	void onNewDataMessage()
	{
		m_sub->readNextData((void*)m_latency,&m_info);
		//cout << "R: "<<m_latency->seqnum<< "|";
		m_pub->write((void*)m_latency);
		n_received++;
		if(n_received == n_samples)
			sema.post();
	}
	void onSubscriptionMatched()
	{
		cout << B_RED << "SUBSCRIPTION MATCHED" <<DEF << endl;
		sema.post();
	}

	class LatencySubscriber_PubListener:public PublisherListener
	{
	public:
		LatencySubscriber_PubListener(boost::interprocess::interprocess_semaphore* sem):
			mp_sema(sem){};
		virtual ~LatencySubscriber_PubListener(){};
		boost::interprocess::interprocess_semaphore* mp_sema;
		void onPublicationMatched()
		{
			mp_sema->post();
			cout <<B_MAGENTA<< "Publication Matched"<<DEF <<endl;
		}
	}m_PubListener;
};


LatencySubscriber::LatencySubscriber():
		m_latency(NULL),n_received(0),n_samples(0),sema(0),
		m_PubListener(&sema)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant2";
	m_part = DomainParticipant::createParticipant(PParam);

	PublisherAttributes Wparam;
	Wparam.historyMaxSize = NSAMPLES+100;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyDown";
	m_pub = DomainParticipant::createPublisher(m_part,Wparam,(PublisherListener*)&this->m_PubListener);


	SubscriberAttributes Rparam;
	Rparam.historyMaxSize = NSAMPLES+100;
	Rparam.topic.topicDataType = std::string("LatencyType");
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "LatencyUp";
	m_sub = DomainParticipant::createSubscriber(m_part,Rparam,(SubscriberListener*)this);


}


bool LatencySubscriber::test(uint32_t datasize,uint32_t n_samples_in)
{
	m_latency = new LatencyType(datasize);
	n_samples = n_samples_in;
	n_received = 0;
	cout << "Waiting ... for latencytype of size "<< (m_latency->data.size()+4) <<endl;
	sema.wait();
	size_t removed;
	cout << "Removing ";
	m_pub->removeAllChange(&removed);
	cout << removed << endl;
	cout << m_sub->getHistoryElementsNumber() << endl;
//	std::cin >> removed
	while(m_sub->getHistoryElementsNumber()>0)
	{
		if(!m_sub->takeNextData((void*)m_latency,&m_info))
			cout << "ERROR ";
	}
	//std::cin >> removed;
	delete(m_latency);
	return true;
}






#endif /* LATENCYSUBSCRIBER_H_ */
