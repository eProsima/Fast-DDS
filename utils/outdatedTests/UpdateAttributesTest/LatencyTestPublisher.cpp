// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.h"

uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );


LatencyTestPublisher::LatencyTestPublisher():
										mp_RTPSParticipant(NULL),
										mp_datapub(NULL),
										mp_commandpub(NULL),
										mp_datasub(NULL),
										mp_commandsub(NULL),
										mp_latency_in(NULL),
										mp_latency_out(NULL),
										m_overhead(0.0),
										n_subscribers(0),
										n_samples(0),
										m_disc_sema(0),
										m_comm_sema(0),
										m_data_sema(0),
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

}


bool LatencyTestPublisher::init(int n_sub,int n_sam)
{
	n_samples = n_sam;
	n_subscribers = n_sub;
	RTPSParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "RTPSParticipant_pub";
	mp_RTPSParticipant = DomainRTPSParticipant::createRTPSParticipant(PParam);
	if(mp_RTPSParticipant == NULL)
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
	PubDataparam.topic.historyQos.depth = n_samples+100;
	PubDataparam.topic.resourceLimitsQos.max_samples = n_samples+100;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples+100;
	PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	PubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "LatencySUB2PUB";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = n_samples+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples+100;
	mp_datasub = DomainRTPSParticipant::createSubscriber(mp_RTPSParticipant,SubDataparam,&this->m_datasublistener);
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
	mp_commandpub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,PubCommandParam,&this->m_commandpublistener);
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
	mp_commandsub = DomainRTPSParticipant::createSubscriber(mp_RTPSParticipant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == NULL)
		return false;
	return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Data Pub Matched "<<RTPS_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Data Sub Matched "<<RTPS_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Command Pub Matched "<<RTPS_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Command Sub Matched "<<RTPS_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			pError("More matched subscribers than expected"<<endl);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
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
			mp_up->m_comm_sema.post();
	}
}
#if defined(_WIN32)
void LatencyTestPublisher::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency_in,&mp_up->m_sampleinfo);

	if(mp_up->mp_latency_in->seqnum != mp_up->mp_latency_out->seqnum)
	{
		cout << "ERROR IN TEST"<<endl;
		mp_up->m_data_sema.post();
		TestCommandType command;
		command.m_command = STOP_ERROR;
		mp_up->mp_commandpub->write(&command);
		mp_up->m_status = -1;
		mp_up->m_data_sema.post();
	}
	else if(mp_up->mp_latency_in->seqnum == NSAMPLES)
	{

		mp_up->m_clock.setTimeNow(&mp_up->m_t2);
		mp_up->m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t2)-TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t1)-mp_up->m_overhead);
		mp_up->m_data_sema.post();

	}
	else
	{

		mp_up->mp_latency_out->seqnum++;
		mp_up->mp_datapub->write(mp_up->mp_latency_out);
		mp_up->mp_latency_in->seqnum = 0;
		mp_up->n_received = 0;

	}
}
#else
void LatencyTestPublisher::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency_in,&mp_up->m_sampleinfo);
	//eClock::my_sleep(50);
	//cout << "R: "<< mp_up->mp_latency_in->seqnum <<std::flush;
	//cout <<"T|"<<std::flush;
	mp_up->m_clock.setTimeNow(&mp_up->m_t2);
	mp_up->m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t2)-TimeConv::Time_t2MicroSecondsDouble(mp_up->m_t1)-mp_up->m_overhead);
	if(mp_up->mp_latency_in->seqnum != mp_up->mp_latency_out->seqnum)
	{
		cout << "ERROR IN TEST"<<endl;
		TestCommandType command;
		command.m_command = STOP_ERROR;
		mp_up->mp_commandpub->write(&command);
		mp_up->m_status = -1;
		mp_up->m_data_sema.post();
	}
	else if(mp_up->mp_latency_in->seqnum == (uint32_t)mp_up->n_samples) //TEST FINISHED
	{
		mp_up->m_data_sema.post();
	}
	else
	{
		mp_up->mp_latency_out->seqnum++;
		mp_up->m_clock.setTimeNow(&mp_up->m_t1);
		mp_up->mp_datapub->write(mp_up->mp_latency_out);
		mp_up->n_received = 0;
		mp_up->mp_latency_in->seqnum = 0;
	}
}
#endif


void LatencyTestPublisher::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:
	//EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
	for(uint8_t i = 0;i<n_subscribers*4;++i)
	{
		m_disc_sema.wait();
		if(m_status == -1)
			return;
	}
	cout << RTPS_B_MAGENTA << "DISCOVERY COMPLETE "<<RTPS_DEF<<endl;
	printf("Printing round-trip times in us, statistics for %d samples\n",n_samples);
	printf("   Bytes,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
	printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
	//int aux;
	for(std::vector<uint32_t>::iterator ndata = data_size_pub.begin();ndata!=data_size_pub.end();++ndata)
	{
		if(!this->test(*ndata))
			break;
		eClock::my_sleep(100);
		//		cout << "Enter number to start next text: ";
		//		std::cin >> aux;
	}
}

bool LatencyTestPublisher::test(uint32_t datasize)
{
	//cout << "Beginning test of size: "<<datasize+4 <<endl;
	m_status = 0;
	n_received = 0;
	mp_latency_in = new LatencyType(datasize);
	mp_latency_out = new LatencyType(datasize);
	m_times.clear();
	TestCommandType command;
	command.m_command = READY;
	mp_commandpub->write(&command);
	//	cout << "WATIGIN FOR COMMAND RESPONSES ";
	for(uint8_t i = 0;i<n_subscribers;++i)
	{
		m_comm_sema.wait();
		//cout << (int)i << " ";
	}
	//cout << endl;
	//BEGIN THE TEST:
	m_clock.setTimeNow(&m_t1);
	mp_datapub->write((void*)mp_latency_out);
	m_data_sema.wait();
	if(m_status !=0)
	{
		cout << "Error in test "<<endl;
		return false;
	}
	//TEST FINISHED:
	size_t removed=0;
	mp_datapub->removeAllChange(&removed);
	//	cout << "REMOVED: "<< removed<<endl;
	analyzeTimes(datasize);
	printStat(m_stats.back());
	//	delete(mp_latency_in);
	//	delete(mp_latency_out);
	return true;
}

#if defined(_WIN32)
void LatencyTestPublisher::analyzeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.mean = (double)( *m_times.begin()/(NSAMPLES+1));
	m_stats.push_back(TS);
}
void LatencyTestPublisher::printStat(TimeStats& TS)
{
	//cout << "MEAN PRINTING: " << TS.mean << endl;
	printf("%8llu,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.max);
}
#else
void LatencyTestPublisher::analyzeTimes(uint32_t datasize)
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


void LatencyTestPublisher::printStat(TimeStats& TS)
{
	//cout << "MEAN PRINTING: " << TS.mean << endl;
	printf("%8lu,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.max);
}
#endif
