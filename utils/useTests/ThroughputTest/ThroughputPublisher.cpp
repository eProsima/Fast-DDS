/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputPublisher.cxx
 *
 */



#include "ThroughputPublisher.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/attributes/SubscriberAttributes.h"

#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include "fastrtps/Domain.h"

uint32_t g_dataspub[] = {8,24,56,120,248,504,1016,2040,4088,8184};
//uint32_t dataspub[] = {504,1016,2040,4088,8184};
std::vector<uint32_t> g_data_size_pub (g_dataspub, g_dataspub + sizeof(g_dataspub) / sizeof(uint32_t) );

uint32_t g_demandspub[] = {500,750,850,1000,1250,1400,1500,1600,1750,2000};
vector<uint32_t> g_demand_pub (g_demandspub, g_demandspub + sizeof(g_demandspub) / sizeof(uint32_t) );


ThroughputPublisher::DataPubListener::DataPubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::DataPubListener::~DataPubListener(){};

void ThroughputPublisher::DataPubListener::onPublicationMatched(Publisher* pub,MatchingInfo info)
{

	if(info.status == MATCHED_MATCHING)
	{
		cout << C_RED << "DATA Pub Matched"<<C_DEF<<endl;
		m_up.sema.post();
	}
	else
	{
		cout << C_RED << "DATA PUBLISHER MATCHING REMOVAL" << C_DEF<<endl;
	}
}

ThroughputPublisher::CommandSubListener::CommandSubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::CommandSubListener::~CommandSubListener(){};
void ThroughputPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_RED << "COMMAND Sub Matched"<<C_DEF<<endl;
		m_up.sema.post();
	}
	else
	{
		cout << C_RED << "COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF<<endl;
	}
}

ThroughputPublisher::CommandPubListener::CommandPubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::CommandPubListener::~CommandPubListener(){};
void ThroughputPublisher::CommandPubListener::onPublicationMatched(Publisher* pub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_RED << "COMMAND Pub Matched"<<C_DEF<<endl;
		m_up.sema.post();
	}
	else
	{
		cout << C_RED << "COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF<<endl;
	}
}


ThroughputPublisher::~ThroughputPublisher(){Domain::stopAll();}

ThroughputPublisher::ThroughputPublisher():
																				sema(0),
																				m_DataPubListener(*this),m_CommandSubListener(*this),m_CommandPubListener(*this),
																				ready(true)
{
	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10042;
	PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
	PParam.rtps.sendSocketBufferSize = 65536;
	PParam.rtps.listenSocketBufferSize = 2*65536;
	PParam.rtps.setName("Participant_publisher");
	mp_par = Domain::createParticipant(PParam);
	if(mp_par == nullptr)
	{
		cout << "ERROR creating participant"<<endl;
		ready = false;
		return;
	}
	//REGISTER THE TYPES
	Domain::registerType(mp_par,(TopicDataType*)&latency_t);
	Domain::registerType(mp_par,(TopicDataType*)&throuputcommand_t);
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
	//PUBLISHER
	PublisherAttributes Wparam;
	//Wparam.historyMaxSize = 10000;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_samples = 10000;
	Wparam.topic.resourceLimitsQos.allocated_samples = 10000;
	Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = Domain::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_DataPubListener);
	//COMMAND
	SubscriberAttributes Rparam;
	Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	Rparam.topic.resourceLimitsQos.max_samples = 20;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommandS2P";
	mp_commandsub = Domain::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);
	PublisherAttributes Wparam2;
	Wparam2.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam2.topic.historyQos.depth = 50;
	Wparam2.topic.resourceLimitsQos.max_samples = 50;
	Wparam2.topic.resourceLimitsQos.allocated_samples = 50;
	Wparam2.topic.topicDataType = "ThroughputCommand";
	Wparam2.topic.topicKind = NO_KEY;
	Wparam2.topic.topicName = "ThroughputCommandP2S";
	Wparam2.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_commandpub = Domain::createPublisher(mp_par,Wparam2,(PublisherListener*)&this->m_CommandPubListener);

	if(mp_datapub == nullptr || mp_commandsub == nullptr || mp_commandpub == nullptr)
		ready = false;

	//mp_par->stopParticipantAnnouncement();
	eClock::my_sleep(5000);
}

void ThroughputPublisher::run(uint32_t test_time,int demand,int msg_size)
{
	if(!ready)
		return;
	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
	sema.wait();
	cout << "Discovery complete"<<endl;
	std::vector<uint32_t> data_size_pub;
	std::vector<uint32_t> demand_pub;
	if(demand == 0)
	{
		demand_pub = g_demand_pub;
	}
	else
	{
		demand_pub.push_back(demand);
	}
	if(msg_size == 0)
	{
		data_size_pub = g_data_size_pub;
	}
	else
	{
		data_size_pub.push_back(msg_size);
	}

	ThroughputCommandType command;
	SampleInfo_t info;
	printResultTitle();
	for(std::vector<uint32_t>::iterator sit=data_size_pub.begin();sit!=data_size_pub.end();++sit)
	{
		for(std::vector<uint32_t>::iterator dit=demand_pub.begin();dit!=demand_pub.end();++dit)
		{
			eClock::my_sleep(100);
			//cout << "Starting test with demand: " << *dit << endl;
			command.m_command = READY_TO_START;
			command.m_size = *sit;
			command.m_demand = *dit;
			//cout << "SEND COMMAND "<< command.m_command << endl;
			mp_commandpub->write((void*)&command);
			command.m_command = DEFAULT;
			mp_commandsub->waitForUnreadMessage();
			mp_commandsub->takeNextData((void*)&command,&info);
			//cout << "RECI COMMAND "<< command.m_command << endl;
			//cout << "Received command of type: "<< command << endl;
			if(command.m_command == BEGIN)
			{
				if(!test(test_time,*dit,*sit))
				{
					command.m_command = ALL_STOPS;
					//	cout << "SEND COMMAND "<< command.m_command << endl;
					mp_commandpub->write((void*)&command);
					return;
				}
			}
		}
	}
	command.m_command = ALL_STOPS;
	//	cout << "SEND COMMAND "<< command.m_command << endl;
	mp_commandpub->write((void*)&command);
}

bool ThroughputPublisher::test(uint32_t test_time,uint32_t demand,uint32_t size)
{
	LatencyType latency(size);
	m_Clock.setTimeNow(&m_t2);
	uint64_t timewait_us=0;
	uint32_t samples=0;
	size_t aux;
	ThroughputCommandType command;
	SampleInfo_t info;
	command.m_command = TEST_STARTS;
	//cout << "SEND COMMAND "<< command.m_command << endl;
	mp_commandpub->write((void*)&command);
	m_Clock.setTimeNow(&m_t1);
	while(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)<test_time*1000000)
	{
		for(uint32_t sample=0;sample<demand;sample++)
		{
			latency.seqnum++;
			mp_datapub->write((void*)&latency);
			//cout << sample << "*"<<std::flush;
		}
		m_Clock.setTimeNow(&m_t2);
		samples+=demand;
		//cout << "samples sent: "<<samples<< endl;
		eClock::my_sleep(10);
		timewait_us+=m_overhead+10;
		//cout << "Removing all..."<<endl;
		mp_datapub->removeAllChange(&aux);
		//cout << (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))<<endl;
	}
	command.m_command = TEST_ENDS;
	//cout << "SEND COMMAND "<< command.m_command << endl;
	mp_commandpub->write((void*)&command);
	mp_commandpub->removeAllChange(&aux);
	mp_commandsub->waitForUnreadMessage();
	if(mp_commandsub->takeNextData((void*)&command,&info))
	{
		//cout << "RECI COMMAND "<< command.m_command << endl;
		if(command.m_command == TEST_RESULTS)
		{
			//cout << "Received results from subscriber"<<endl;
			TroughputResults result;
			result.demand = demand;
			result.payload_size = size+4+4;
			result.publisher.send_samples = samples;
			result.publisher.totaltime_us = TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-timewait_us;
			result.subscriber.recv_samples = command.m_lastrecsample-command.m_lostsamples;
			result.subscriber.totaltime_us = command.m_totaltime;
			result.subscriber.lost_samples = command.m_lostsamples;
			result.compute();
			m_timeStats.push_back(result);
			printResults(result);
			return true;
		}
		else
		{
			cout << "The test expected results, stopping"<<endl;
		}
	}
	else
		cout << "PROBLEM READING RESULTS;"<<endl;

	return false;

}


