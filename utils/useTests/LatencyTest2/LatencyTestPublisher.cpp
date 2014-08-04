/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.h"

uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );


LatencyTestPublisher::LatencyTestPublisher():
		mp_participant(NULL),
		mp_datapub(NULL),
		mp_commandpub(NULL),
		mp_datasub(NULL),
		mp_commandsub(NULL),
		m_overhead(0.0),
		n_subscribers(0),
		sema(0),
		m_status(0),
		n_received(0),
		m_datapublistener(this),
		m_datasublistener(this),
		m_commandpublistener(this),
		m_commandsublistener(this)
{

}

LatencyTestPublisher::~LatencyTestPublisher()
{
	if(mp_participant!=NULL)
		DomainParticipant::removeParticipant(mp_participant);
}


bool LatencyTestPublisher::init(int n_sub)
{



	n_subscribers = n_sub;
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant_pub";
	mp_participant = DomainParticipant::createParticipant(PParam);
	if(mp_participant == NULL)
		return false;

	m_clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "LatencyType";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "LatencyPUB2SUB";
	PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = NSAMPLES+100;
	PubDataparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = DomainParticipant::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "LatencySUB2PUB";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	mp_datasub = DomainParticipant::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
	if(mp_datasub == NULL)
		return false;
	//COMMAND PUBLISHER
	PublisherAttributes PubCommandParam;
	PubCommandParam.topic.topicDataType = "TestCommandType";
	PubCommandParam.topic.topicKind = NO_KEY;
	PubCommandParam.topic.topicName = "CommandPUB2SUB";
	PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubCommandParam.topic.historyQos.depth = 100;
	PubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	PubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	mp_commandpub = DomainParticipant::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
	if(mp_commandpub == NULL)
		return false;
	SubscriberAttributes SubCommandParam;
	SubCommandParam.topic.topicDataType = "TestCommandType";
	SubCommandParam.topic.topicKind = NO_KEY;
	SubCommandParam.topic.topicName = "CommandSUB2PUB";
	SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubCommandParam.topic.historyQos.depth = 100;
	SubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	SubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_commandsub = DomainParticipant::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == NULL)
		return false;
	return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->sema.post();
		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->sema.post();
		}
	}
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->sema.post();
		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->sema.post();
		}
	}
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->sema.post();
		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->sema.post();
		}
	}
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->sema.post();
		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->sema.post();
		}
	}
}

void LatencyTestPublisher::CommandSubListener::onNewDataMessage()
{
	TestCommandType command;
	SampleInfo_t info;
	mp_up->mp_commandsub->takeNextData((void*)&command,&info);
	if(info.sampleKind == ALIVE)
	{
		if(command.m_command == BEGIN)
			mp_up->sema.post();
	}
}
#if defined(_WIN32)
void LatencyTestPublisher::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)m_latency_in,&m_sampleinfo);
	mp_up->n_received++;
	if(mp_up->m_latency_in.seqnum != mp_up->m_latency_out.seqnum)
	{
		cout << "ERROR IN TEST"<<endl;
		mp_up->sema.post();
		TestCommandType command;
		command.m_command = STOP_ERROR;
		mp_up->mp_commandpub->write(&command);
		mp_up->m_status = -1;
		sema.post();
	}
	else if(mp_up->m_latency_in == NSAMPLES)
	{
		if(mp_up->n_received == mp_up->n_subscribers)
		{
			mp_up->m_clock.setTimeNow(&m_t2);
			mp_up->m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-m_overhead);
			mp_up->sema.post();
		}
	}
	else
	{
		if(mp_up->n_received == mp_up->n_subscribers)
		{
			mp_up->m_latency_out.seqnum++;
			mp_up->mp_datapub->write(&m_latency_out);
			mp_up->m_latency_in.seqnum = 0;
			mp_up->n_received = 0;
		}
	}
}
#else
void LatencyTestPublisher::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency_in,&mp_up->m_sampleinfo);
	mp_up->n_received++;
	if(mp_up->n_received == mp_up->n_subscribers)
	{
		mp_up->m_clock.setTimeNow(&mp_up->m_t2);
		mp_up->m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t2)-TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t1)-mp_up->m_overhead);
	}
	if(mp_up->mp_latency_in->seqnum != mp_up->mp_latency_out->seqnum)
	{
		cout << "ERROR IN TEST"<<endl;
		TestCommandType command;
		command.m_command = STOP_ERROR;
		mp_up->mp_commandpub->write(&command);
		mp_up->m_status = -1;
		mp_up->sema.post();
	}
	else if(mp_up->mp_latency_in->seqnum == NSAMPLES)
	{
		if(mp_up->n_received == mp_up->n_subscribers)
		{
			mp_up->sema.post();
		}
	}
	else
	{
		if(mp_up->n_received == mp_up->n_subscribers)
		{
			mp_up->mp_latency_out->seqnum++;
			mp_up->m_clock.setTimeNow(&mp_up->m_t1);
			mp_up->mp_datapub->write(mp_up->mp_latency_out);
			mp_up->n_received = 0;
			mp_up->mp_latency_in->seqnum = 0;
		}
	}
}
#endif


void LatencyTestPublisher::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:
	//EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
	for(uint8_t i = 0;i<n_subscribers*4;++i)
	{
		sema.wait();
	}
	printf("Printing times in us\n");
	printf("   Bytes,   stdev,    mean,     min,    50%%,    90%%,    99%%, 99.99%%,     max\n");
	printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
	for(std::vector<uint32_t>::iterator ndata = data_size_pub.begin();ndata!=data_size_pub.end();++ndata)
	{
		if(!this->test(*ndata))
			break;
	}
}

bool LatencyTestPublisher::test(uint32_t datasize)
{
	cout << "Beginning test of size: "<<datasize+4 <<endl;
	m_status = 0;
	n_received = 0;
	mp_latency_in = new LatencyType(datasize);
	mp_latency_out = new LatencyType(datasize);
	m_times.clear();
	TestCommandType command;
	command.m_command = READY;
	mp_commandpub->write(&command);
	for(uint8_t i = 0;i<n_subscribers;++i)
	{
		sema.wait();
	}
	//BEGIN THE TEST:
	m_clock.setTimeNow(&m_t1);
	mp_datapub->write((void*)mp_latency_out);
	sema.wait();
	if(m_status !=0)
	{
		cout << "Error in test "<<endl;
		return false;
	}
	//TEST FINISHED:
	size_t removed=0;
	mp_datapub->removeAllChange(&removed);
	cout << "REMOVED: "<< removed<<endl;
	analizeTimes(datasize);
	printStat(m_stats.back());
//	delete(mp_latency_in);
//	delete(mp_latency_out);
	return true;
}

#if defined(_WIN32)
void LatencyTestPublisher::analizeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.mean = (TimeConv::Time_t2MicroSecondsDouble(m_t2) - TimeConv::Time_t2MicroSecondsDouble(m_t1))/NSAMPLES;
	m_stats.push_back(TS);
}
#else
void LatencyTestPublisher::analizeTimes(uint32_t datasize)
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


	//printStat(TS);
	m_stats.push_back(TS);
}
#endif

void LatencyTestPublisher::printStat(TimeStats& TS)
{
	printf("%6lu,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.max);
}
