/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.h"
#include "fastrtps/utils/RTPSLog.h"



using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );

static const char * const CLASS_NAME = "LatencyTestPublisher";

LatencyTestPublisher::LatencyTestPublisher():
										mp_participant(nullptr),
										mp_datapub(nullptr),
										mp_commandpub(nullptr),
										mp_datasub(nullptr),
										mp_commandsub(nullptr),
										mp_latency_in(nullptr),
										mp_latency_out(nullptr),
										m_overhead(0.0),
										n_subscribers(0),
										n_samples(0),
										m_disc_sema(0),
										m_comm_sema(0),
										m_data_sema(0),
										m_status(0),
										n_received(0),
										m_datapublistener(nullptr),
										m_datasublistener(nullptr),
										m_commandpublistener(nullptr),
										m_commandsublistener(nullptr)
{
	m_datapublistener.mp_up = this;
	m_datasublistener.mp_up = this;
	m_commandpublistener.mp_up = this;
	m_commandsublistener.mp_up = this;
}

LatencyTestPublisher::~LatencyTestPublisher()
{

}


bool LatencyTestPublisher::init(int n_sub,int n_sam)
{
	n_samples = n_sam;
	n_subscribers = n_sub;
	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10042;
	PParam.rtps.builtin.domainId = 80;
	PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;

	PParam.rtps.sendSocketBufferSize = 8712;
	PParam.rtps.listenSocketBufferSize = 17424;
	PParam.rtps.setName("Participant_pub");
	mp_participant = Domain::createParticipant(PParam);
	if(mp_participant == nullptr)
		return false;
	Domain::registerType(mp_participant,(TopicDataType*)&latency_t);
		Domain::registerType(mp_participant,(TopicDataType*)&command_t);
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
	mp_datapub = Domain::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == nullptr)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	SubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "LatencySUB2PUB";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = n_samples+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples+100;
	mp_datasub = Domain::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
	if(mp_datasub == nullptr)
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
	mp_commandpub = Domain::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
	if(mp_commandpub == nullptr)
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
	mp_commandsub = Domain::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == nullptr)
		return false;
	return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(Publisher* pub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Data Pub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			logUser(C_RED<<"More matched subscribers than expected"<<C_DEF);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Data Sub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			logUser(C_RED<<"More matched subscribers than expected"<<C_DEF);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(Publisher* pub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Command Pub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			logUser(C_RED<<"More matched subscribers than expected"<<C_DEF);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Command Sub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			logUser(C_RED<<"More matched subscribers than expected"<<C_DEF);
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandSubListener::onNewDataMessage(Subscriber* sub)
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
void LatencyTestPublisher::DataSubListener::onNewDataMessage(Subscriber* sub)
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
	else if(mp_up->mp_latency_in->seqnum == mp_up->n_samples)
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
void LatencyTestPublisher::DataSubListener::onNewDataMessage(Subscriber* sub)
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
	//	cout << "TEST with samples: "<<mp_up->n_samples<< " finished "<<endl;
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
	cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;
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
	cout << "REMOVING PUBLISHER"<<endl;
	Domain::removePublisher(this->mp_commandpub);
	cout << "REMOVING SUBSCRIBER"<<endl;
	Domain::removeSubscriber(mp_commandsub);
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
	//cout << "WAITING FOR COMMAND RESPONSES ";
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
	//cout << "   REMOVED: "<< removed<<endl;
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
	TS.mean = (double)( *m_times.begin()/(n_samples+1));
	m_stats.push_back(TS);
}
void LatencyTestPublisher::printStat(TimeStats& TS)
{
	//cout << "MEAN PRINTING: " << TS.mean << endl;
	printf("%8llu,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.m_min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.m_max);
}
#else
void LatencyTestPublisher::analizeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.m_min = *std::min_element(m_times.begin(),m_times.end());
	TS.m_max = *std::max_element(m_times.begin(),m_times.end());
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
			TS.m_min,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.m_max);
}
#endif
