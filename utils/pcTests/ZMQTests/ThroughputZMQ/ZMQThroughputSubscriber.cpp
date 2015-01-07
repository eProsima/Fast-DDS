/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputSubscriber.cxx
 *
 */

#include "ZMQThroughputSubscriber.h"
#include "fastrtps/utils/TimeConversion.h"


using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include <vector>

int writecalls= 0;
//uint32_t datassub[] = {8,24,56,120,248,504,1016,2040,4088,8184};
//std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );
//
//uint32_t demandssub[] = {1,10,20,30,50,60,70,80};
//std::vector<uint32_t> demand_sub (demandssub, demandssub + sizeof(demandssub) / sizeof(uint32_t) );





ZMQThroughputSubscriber::~ZMQThroughputSubscriber(){Domain::stopAll();}

ZMQThroughputSubscriber::ZMQThroughputSubscriber():	sema(0),
		mp_context(nullptr),
		mp_datasub(nullptr),mp_commandpub(nullptr),
		mp_commandsub(nullptr),
		ready(true),m_datasize(0),m_demand(0),
		command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double)),
		latencyin(nullptr)
{
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;


}

bool ZMQThroughputSubscriber::init(std::string pubIP,uint32_t basePORT)
{
	mp_context = new zmq::context_t(1);

	mp_commandsub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	stringstream ss2;
	ss2 << "tcp://"<<pubIP<<":"<<(basePORT+2);
	mp_commandsub->connect(ss2.str().c_str());
	mp_commandsub->setsockopt(ZMQ_SUBSCRIBE,0,0);

	mp_datasub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	stringstream ss2;
	ss2 << "tcp://"<<pubIP<<":"<<(basePORT+1);
	mp_datasub->connect(ss2.str().c_str());
	mp_datasub->setsockopt(ZMQ_SUBSCRIBE,0,0);

	mp_commandpub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	stringstream ss4;
	ss4 << "tcp://*:"<<(basePORT+3);
	mp_commandpub->bind(ss4.str().c_str());

	eClock::my_sleep(500);

	if(mp_datasub == nullptr || mp_commandsub == nullptr || mp_commandpub == nullptr)
	{
		return false;
	}
	eClock::my_sleep(4000);
	ready = true;
	return true;

}

void ZMQThroughputSubscriber::run()
{
	if(!ready)
		return;
	while(1)
	{
		mp_commandsub->recv(&command_msg);
		m_commandDataType.deserialize(&command_msg,&m_commandin);
		int res = commandReceived();
		if(res == -1)
			break;

	}
	return;

}

int ZMQThroughputSubscriber::commandReceived()
{
	//cout << "RECEIVED COMMAND: "<< m_commandin.m_command << endl;
	switch(m_commandin.m_command)
	{
	default:
	{
		break;
	}
	case (DEFAULT):
	{
		break;
	}
	case (BEGIN):
	{
		break;
	}
	case (READY_TO_START):
	{
		m_datasize = m_commandin.m_size;
		m_demand = m_commandin.m_demand;
		//cout << "Ready to start data size: " << m_datasize << " and demand; "<<m_demand << endl;
		latencyin = new LatencyType(m_datasize);

		eClock::my_sleep(50);
		this->resetResults();
		//cout << "SEND COMMAND: "<< command.m_command << endl;
		//cout << "writecall "<< ++writecalls << endl;
		m_commandout.m_command = BEGIN;
		m_commandDataType.serialize(&m_commandout,&command_msg);
		mp_commandpub->send(command_msg);
		return 0;
		break;
	}
	case (TEST_STARTS):
	{
		m_Clock.setTimeNow(&m_t1);
		return 1;
		break;
	}
	case (TEST_ENDS):
	{
		m_Clock.setTimeNow(&m_t2);
		saveNumbers();
		//cout << "TEST ends, sending results"<<endl;
		m_commandout.m_command = TEST_RESULTS;
		m_commandout.m_demand = m_demand;
		m_commandout.m_size = m_datasize+4+4;
		m_commandout.m_lastrecsample = saved_lastseqnum;
		m_commandout.m_lostsamples = saved_lostsamples;
		m_commandout.m_totaltime = (uint64_t)(TimeConv::Time_t2MicroSecondsDouble(m_t2)-
				TimeConv::Time_t2MicroSecondsDouble(m_t1));
		//cout << "SEND COMMAND: "<< comm.m_command << endl;
		//cout << "writecall "<< ++writecalls << endl;
		m_commandDataType.serialize(&m_commandout,&command_msg);
		mp_commandpub->send(command_msg);
		eClock::my_sleep(100);
		return 0;
		break;
	}
	case (ALL_STOPS):
	{
		return -1;
		break;
	}
	}
	return 0;
}




ThroughputSubscriber::DataSubListener::DataSubListener(ThroughputSubscriber& up):
																														m_up(up),lastseqnum(0),saved_lastseqnum(0),lostsamples(0),saved_lostsamples(0),first(true),latencyin(nullptr)
{

};
ThroughputSubscriber::DataSubListener::~DataSubListener()
{

};

void ThroughputSubscriber::DataSubListener::reset(){
	lastseqnum = 0;
	first = true;
	lostsamples=0;
}

void ThroughputSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_RED << "DATA Sub Matched"<<C_DEF<<endl;
		m_up.sema.post();
	}
	else
	{
		cout << C_RED << "DATA SUBSCRIBER MATCHING REMOVAL" << C_DEF<<endl;
	}
}
void ThroughputSubscriber::DataSubListener::onNewDataMessage(Subscriber* sub)
{
	//	cout << "NEW DATA MSG: "<< latencyin->seqnum << endl;
	m_up.mp_datasub->takeNextData((void*)latencyin,&info);
	//myfile << latencyin.seqnum << ",";
	if(info.sampleKind == ALIVE)
	{
		if((lastseqnum+1)<latencyin->seqnum)
		{
			lostsamples+=latencyin->seqnum-lastseqnum-1;
			//	myfile << "***** lostsamples: "<< lastseqnum << "|"<< lostsamples<< "*****";
		}
		lastseqnum = latencyin->seqnum;
	}
	else
	{
		cout << "NOT ALIVE DATA RECEIVED"<<endl;
	}
}

void ThroughputSubscriber::DataSubListener::saveNumbers()
{
	saved_lastseqnum = lastseqnum;
	saved_lostsamples = lostsamples;
}










