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



#include "ZMQThroughputPublisher.h"
#include "fastrtps/utils/TimeConversion.h"

#include <map>

uint32_t g_dataspub[] = {8,24,56,120,248,504,1016,2040,4088,8184};
//uint32_t dataspub[] = {504,1016,2040,4088,8184};
std::vector<uint32_t> g_data_size_pub (g_dataspub, g_dataspub + sizeof(g_dataspub) / sizeof(uint32_t) );

uint32_t g_demandspub[] = {500,750,850,1000,1250,1400,1500,1600,1750,2000};
vector<uint32_t> g_demand_pub (g_demandspub, g_demandspub + sizeof(g_demandspub) / sizeof(uint32_t) );



ZMQThroughputPublisher::~ZMQThroughputPublisher()
{

}

ZMQThroughputPublisher::ZMQThroughputPublisher():
			sema(0),
			mp_context(nullptr),
			mp_datapub(nullptr),mp_commandpub(nullptr),
			mp_commandsub(nullptr),
			ready(true)
{
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;


}

bool ZMQThroughputPublisher::init(std::string subIP,uint32_t basePORT)
{
	mp_context = new zmq::context_t(1);

	mp_commandsub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	stringstream ss2;
	ss2 << "tcp://"<<subIP<<":"<<(basePORT+3);
	mp_commandsub->connect(ss2.str().c_str());
	mp_commandsub->setsockopt(ZMQ_SUBSCRIBE,0,0);
	eClock::my_sleep(300);

	mp_datapub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	stringstream ss3;
	ss3 << "tcp://*:"<<(basePORT+1);
	mp_datapub->bind(ss3.str().c_str());
	mp_commandpub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	stringstream ss4;
	ss4 << "tcp://*:"<<(basePORT+2);
	mp_commandpub->bind(ss4.str().c_str());

	eClock::my_sleep(500);

	if(mp_datapub == nullptr || mp_commandsub == nullptr || mp_commandpub == nullptr)
	{
		return false;
	}
	eClock::my_sleep(5000);
	ready = true;
	return true;
}

void ZMQThroughputPublisher::run(uint32_t test_time,int demand,int msg_size)
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

	ThroughputCommandType command;
	printResultTitle();
	zmq::message_t command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double));
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

			m_commandDataType.serialize(&command,&command_msg);
			mp_commandpub->send(command_msg);
			command.m_command = DEFAULT;
			mp_commandsub->recv(&command_msg);
			m_commandDataType.deserialize(&command_msg,&command);

			if(command.m_command == BEGIN)
			{
				if(!test(test_time,*dit,sit->first))
				{
					command.m_command = ALL_STOPS;
					//	cout << "SEND COMMAND "<< command.m_command << endl;
					m_commandDataType.serialize(&command,&command_msg);
					mp_commandpub->send(command_msg);
					return;
				}
			}
		}
	}
	command.m_command = ALL_STOPS;
	//	cout << "SEND COMMAND "<< command.m_command << endl;
	m_commandDataType.serialize(&command,&command_msg);
	mp_commandpub->send(command_msg);
}

bool ZMQThroughputPublisher::test(uint32_t test_time,uint32_t demand,uint32_t size)
{
	zmq::message_t command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double));
	zmq::message_t latency_msg(sizeof(uint32_t)+size);
	LatencyType latency(size);
	m_Clock.setTimeNow(&m_t2);
	uint64_t timewait_us=0;
	uint32_t samples=0;
	ThroughputCommandType command;
	command.m_command = TEST_STARTS;
	//cout << "SEND COMMAND "<< command.m_command << endl;
	m_commandDataType.serialize(&command,&command_msg);
	mp_commandpub->send(command_msg);
	eClock::my_sleep(5);
	m_Clock.setTimeNow(&m_t1);
	while(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)<test_time*1000000)
	{
		for(uint32_t sample=0;sample<demand;sample++)
		{
			latency.seqnum++;
			m_latencyDataType.serialize(&latency,&latency_msg);
			mp_datapub->send(latency_msg);
			//cout << sample << "*"<<std::flush;
		}
		m_Clock.setTimeNow(&m_t2);
		samples+=demand;
		//cout << "samples sent: "<<samples<< endl;
		eClock::my_sleep(10);
		timewait_us+=(uint64_t)m_overhead+10;
		//cout << "Removing all..."<<endl;
		//cout << (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))<<endl;
	}
	command.m_command = TEST_ENDS;
	m_commandDataType.serialize(&command,&command_msg);
	//cout << "SEND COMMAND "<< command.m_command << endl;
	mp_commandpub->send(command_msg);
	mp_commandsub->recv(&command_msg);
	m_commandDataType.deserialize(&command_msg,&command);
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
	return false;

}


bool ZMQThroughputPublisher::loadDemandsPayload()
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
		cout << "Payload: "<< sit->first+8 << ": ";
		for(auto dit=sit->second.begin();dit!=sit->second.end();++dit)
		{
			cout << *dit << ", ";
		}
		cout << endl;
	}

	return true;
}



