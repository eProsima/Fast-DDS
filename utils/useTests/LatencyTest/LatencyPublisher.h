/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyPublisher.h
 *
 */

#ifndef LATENCYPUBLISHER_H_
#define LATENCYPUBLISHER_H_

#include <numeric>
#include <cmath>
#include <cstdint>

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"

inline double round( double d )
{
    return floor( d + 0.5 );
}


struct TimeStats{
	uint64_t nbytes;
	uint64_t min,max,mean,p50,p90,p99,p9999;
	uint64_t stdev;
};





class LatencyPublisher:public SubscriberListener
{
public:
	LatencyPublisher();
	~LatencyPublisher(){};
	Participant* m_part;
	Publisher* m_pub;
	Subscriber* m_sub;
	LatencyType* m_latency_out;
	LatencyType* m_latency_in;
	SampleInfo_t m_info;
	Time_t m_t1;
	Time_t m_t2;
	
	eClock clock;
	double overhead_value;
	std::vector<double> m_times;
	std::vector<TimeStats> m_stats;
	boost::interprocess::interprocess_semaphore sema;
	bool test(uint32_t datasize,uint32_t n_samples);
	void analyzeTimes(uint32_t datasize);
	void printStat(TimeStats& TS);
	void onNewDataMessage()
	{
		m_sub->takeNextData((void*)m_latency_in,&m_info);
		cout << m_latency_in->seqnum << " ## ";
		clock.setTimeNow(&m_t2);
		cout << (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-overhead_value) << " # ";
		cout << TimeConv::Time_t2MicroSecondsDouble(m_t2) << " # "<<TimeConv::Time_t2MicroSecondsDouble(m_t1)<<endl;
		m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-overhead_value);
		//m_sub->takeNextData((void*)m_latency_in,&m_info);
		sema.post();
	}
	void onSubscriptionMatched(MatchingInfo info)
	{
		cout << RTPS_B_RED << "SUBSCRIPTION MATCHED" <<RTPS_DEF << endl;
		sema.post();
	}

	class LatencyPublisher_PubListener: public PublisherListener
	{
	public:
		LatencyPublisher_PubListener(boost::interprocess::interprocess_semaphore* sem):
			mp_sema(sem){};
		virtual ~LatencyPublisher_PubListener(){};
		boost::interprocess::interprocess_semaphore* mp_sema;
		void onPublicationMatched(MatchingInfo info)
		{
			mp_sema->post();
			cout << RTPS_B_MAGENTA <<"Publication Matched" <<RTPS_DEF<< endl;
		}
	} m_PubListener;
};



LatencyPublisher::LatencyPublisher():
				m_latency_out(NULL),m_latency_in(NULL),sema(0),
				m_PubListener(&sema)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant1";
	m_part = DomainParticipant::createParticipant(PParam);

	clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		clock.setTimeNow(&m_t2);
	overhead_value = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << overhead_value << endl;
	//PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	Wparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	m_pub = DomainParticipant::createPublisher(m_part,Wparam,(PublisherListener*)&this->m_PubListener);
	//SUBSCRIBER
	SubscriberAttributes Rparam;
	Rparam.topic.topicDataType = std::string("LatencyType");
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "LatencyDown";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 100;
	Rparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	Rparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	m_sub = DomainParticipant::createSubscriber(m_part,Rparam,(SubscriberListener*)this);

//	m_part->announceParticipantState();


}

bool LatencyPublisher::test(uint32_t datasize,uint32_t n_samples)
{
	m_latency_in = new LatencyType(datasize);
	m_latency_out = new LatencyType(datasize);
	m_times.clear();
	//Sleep to allow subscriber to remove its elements
	int aux;
	eClock::my_sleep(1000);
	cout << "Begin test of size: " << datasize+4 << ", enter number to start: ";
	std::cin >> aux;
	for(uint32_t i =0;i<n_samples;++i)
	{
		m_latency_out->seqnum++;
		clock.setTimeNow(&m_t1);
		m_pub->write((void*)m_latency_out);
		sema.wait();
		if(!(*m_latency_out == *m_latency_in))
		{
			cout << "PROBLEM"<<endl;
			return false;
		}
		m_latency_in->seqnum = -1;
	}
	size_t removed=0;
	m_pub->removeAllChange(&removed);
//	cout << "Removed " << removed << endl;
//	cout << "Sub element number " << m_sub->getHistoryElementsNumber() << endl;
	while(m_sub->getHistoryElementsNumber()>0)
	{
		if(!m_sub->takeNextData((void*)m_latency_in,&m_info))
			cout << "ERR ";
	}
	cout << endl;
	//cout << "Number of elements in Reader History: "<< m_sub->getHistoryElementsNumber()<<endl;
	//cout << "Number of elements in Writer History: "<< m_pub->getHistoryElementsNumber()<<endl;
	analyzeTimes(datasize);
	delete(m_latency_in);
	delete(m_latency_out);
	return true;
}



void LatencyPublisher::analyzeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.min = *std::min_element(m_times.begin(),m_times.end());
	TS.max = *std::max_element(m_times.begin(),m_times.end());
	TS.mean = std::accumulate(m_times.begin(),m_times.end(),0)/m_times.size();
	double auxstdev=0;
	for(std::vector<double>::iterator tit=m_times.begin();tit!=m_times.end();++tit)
	{
		//cout << *tit<< "/"<< TS.mean<< "///";
		auxstdev += pow(((*tit)-TS.mean),2);
	}
	auxstdev = sqrt(auxstdev/m_times.size());
	TS.stdev = (uint64_t)round(auxstdev);
	std::sort(m_times.begin(),m_times.end());
	double x= 0;
	double elem,dec;
	x = m_times.size()*0.5;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p50 = (uint64_t)((m_times.at(elem)+m_times.at(elem+1))/2);
	else
		TS.p50 = m_times.at(elem+1);
	x = m_times.size()*0.9;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p90 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p90 = m_times.at(elem);
	x = m_times.size()*0.99;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p99 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p99 = m_times.at(elem);
	x = m_times.size()*0.9999;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p9999 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p9999 = m_times.at(elem);


	printStat(TS);
	m_stats.push_back(TS);

}

void LatencyPublisher::printStat(TimeStats& TS)
{
	printf("%6lu,%6lu,%6lu,%6lu,%6lu,%6lu,%6lu,%6lu,%6lu \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.max);
}



#endif /* LATENCYPUBLISHER_H_ */
