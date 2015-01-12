/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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

#include <map>



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
#pragma warning(disable:4355)
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
	//Overhead
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;

	//DATA PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1000;
	Wparam.topic.resourceLimitsQos.max_samples = 100000;
	Wparam.topic.resourceLimitsQos.allocated_samples = 100000;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	//Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	Wparam.times.heartbeatPeriod = TimeConv::MilliSeconds2Time_t(5);
	Wparam.times.nackSupressionDuration = TimeConv::MilliSeconds2Time_t(0);
	Wparam.times.nackResponseDelay = TimeConv::MilliSeconds2Time_t(0);
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
	{

		return;
	}
	if(demand == 0 || msg_size == 0)
	{
		if(!this->loadDemandsPayload())
			return;
	}
	else
	{
		m_demand_payload[msg_size-8].push_back(demand);
	}
	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
	sema.wait();
	cout << "Discovery complete"<<endl;

	ThroughputCommandType command;
	SampleInfo_t info;
	printResultTitle();
	for(auto sit=m_demand_payload.begin();sit!=m_demand_payload.end();++sit)
	{
		for(auto dit=sit->second.begin();dit!=sit->second.end();++dit)
		{
			eClock::my_sleep(100);
			//cout << "Starting test with demand: " << *dit << endl;
			command.m_command = READY_TO_START;
			command.m_size = sit->first;
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
				if(!test(test_time,*dit,sit->first))
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
		eClock::my_sleep(20);
		timewait_us+=(uint64_t)m_overhead+20;
		//cout << "Removing all..."<<endl;
		mp_datapub->removeAllChange(&aux);
		//cout << (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))<<endl;
	}
	command.m_command = TEST_ENDS;
	//cout << "SEND COMMAND "<< command.m_command << endl;
	eClock::my_sleep(100);
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
			result.publisher.totaltime_us = (uint64_t)(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-timewait_us);
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


bool ThroughputPublisher::loadDemandsPayload()
{
	std::ifstream fi("payloads_demands.csv");
	cout << "Reading File: payloads_demands.csv" << endl;
	std::string DELIM = ";";
	if(!fi.is_open())
	{
		std::cout << "Could not open file: "<<"payload_demands.csv" << " , closing." <<std::endl;
		return false;
	}

	std::string line;
	size_t start;
	size_t end;
	bool first = true;
	bool more = true;
	while(std::getline(fi,line))
	{
	//	cout << "READING LINE: "<< line<<endl;
		start = 0;
		end = line.find(DELIM);
		first = true;
		uint32_t payload;
		uint32_t demand;
		more = true;
		while(more)
		{
		//	cout << "SUBSTR: "<< line.substr(start,end-start) << endl;
			std::istringstream iss(line.substr(start,end-start));
			if(first)
			{
				iss >> payload;
				if(payload<8)
				{
					cout << "Minimum payload is 16 bytes"<<endl;
					return false;
				}
				payload -=8;
				first = false;
			}
			else
			{
				iss >> demand;
				m_demand_payload[payload].push_back(demand);
			}
			start = end+DELIM.length();
			end = line.find(DELIM,start);
			if(end == std::string::npos)
			{
				more = false;
				std::istringstream iss(line.substr(start,end-start));
				if(iss >> demand)
					m_demand_payload[payload].push_back(demand);
			}
		}
	}
	fi.close();
	cout << "Performing test with this payloads/demands:"<<endl;
	for(auto sit=m_demand_payload.begin();sit!=m_demand_payload.end();++sit)
	{
		printf("Payload: %6d; Demands: ",sit->first+8);
		for(auto dit=sit->second.begin();dit!=sit->second.end();++dit)
		{
			printf("%6d, ",*dit);
		}
		printf("\n");
	}

	return true;
}



