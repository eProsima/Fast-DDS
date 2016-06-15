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





ZMQThroughputSubscriber::~ZMQThroughputSubscriber()
{
	mp_commandsub->close();
	mp_commandpub->close();
	delete(mp_commandsub);
	delete(mp_commandpub);
	delete(mp_context);
}

ZMQThroughputSubscriber::ZMQThroughputSubscriber():	sema(0),
		mp_context(nullptr),
		mp_dataContext(nullptr),
		mp_commandpub(nullptr),
		mp_datasub(nullptr),
		mp_commandsub(nullptr),
		ready(true),m_datasize(0),m_demand(0),
		lastseqnum(0),saved_lastseqnum(0),
		lostsamples(0),saved_lostsamples(0),
		latencyin(nullptr),
		mp_latencyThread(nullptr),
		basePORT(0)
{
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;


}

bool ZMQThroughputSubscriber::init(std::string pubIP,uint32_t baseport)
{
	publisherIP = pubIP;
	basePORT = baseport;
	mp_context = new zmq::context_t(1);

	mp_commandsub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	stringstream ss1;
	ss1 << "tcp://"<<pubIP<<":"<<(basePORT+2);
	mp_commandsub->connect(ss1.str().c_str());
	mp_commandsub->setsockopt(ZMQ_SUBSCRIBE,0,0);



	mp_commandpub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	stringstream ss4;
	ss4 << "tcp://*:"<<(basePORT+3);
	mp_commandpub->bind(ss4.str().c_str());

	eClock::my_sleep(500);

	if(mp_commandsub == nullptr || mp_commandpub == nullptr)
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
	//	cout << "Waiting for command"<<endl;
		zmq::message_t command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double));
		mp_commandsub->recv(&command_msg);
		m_commandDataType.deserialize(&command_msg,&m_commandin);
		int res = commandReceived();
		if(res == -1)
		{
			return;
		}
		else if(res == 1) //TEST STARTS
		{

		}
		else if(res == 2) //TEST ENDS
		{
			//cout << "Removing context"<<endl;
			delete(mp_dataContext);
			mp_latencyThread->join();
			//cout << "Joining thread"<<endl;
			delete(mp_datasub);
			delete(mp_latencyThread);
			mp_latencyThread = nullptr;
			eClock::my_sleep(50);
		}
	}
	return;
}

void ZMQThroughputSubscriber::processMessages()
{
	while(1)
	{
		try{
			zmq::message_t latencymsg(latencyin->data.size()+8);
			mp_datasub->recv(&latencymsg);
			m_latencyDataType.deserialize(&latencymsg,latencyin);
			if((lastseqnum+1)<latencyin->seqnum)
			{
				lostsamples+=latencyin->seqnum-lastseqnum-1;
				//cout<< "***** lostsamples: "<< lastseqnum << "|"<< lostsamples<< "*****"<<std::flush;
			}
			lastseqnum = latencyin->seqnum;
		}
		catch(const zmq::error_t& ex)
		{
		//	cout << "Exception: "<<ex.what()<<endl;
			mp_datasub->close();
			return;
		}
	}
}

int ZMQThroughputSubscriber::commandReceived()
{
	//	cout << "RECEIVED COMMAND: "<< m_commandin.m_command << endl;
	switch(m_commandin.m_command)
	{
	case (DEFAULT):	{
		break;
	}
	case (BEGIN):{
		break;
	}
	case (READY_TO_START):{
		//	printf("READY TO START\n");
		m_datasize = m_commandin.m_size;
		m_demand = m_commandin.m_demand;
		cout << "Ready to start data size: " << m_datasize << " and demand; "<<m_demand << endl;
		latencyin = new LatencyType(m_datasize);

		eClock::my_sleep(50);
		this->resetResults();
		//	cout << "SEND COMMAND: "<< m_commandout.m_command << endl;
	    //cout << "writecall "<< ++writecalls << endl;
		m_commandout.m_command = BEGIN;
		zmq::message_t command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double));
		m_commandDataType.serialize(&m_commandout,&command_msg);


		stringstream ss2;
		ss2 << "tcp://" << publisherIP << ":" << (basePORT + 1);
		mp_dataContext = new zmq::context_t(1);
		mp_datasub = new zmq::socket_t(*mp_dataContext, ZMQ_SUB);
		mp_datasub->connect(ss2.str().c_str());
		mp_datasub->setsockopt(ZMQ_SUBSCRIBE, 0, 0);
		mp_latencyThread = new boost::thread(&ZMQThroughputSubscriber::processMessages, this);

		mp_commandpub->send(command_msg);
		return 0;
		break;
	}
	case (TEST_STARTS):	{
		//	cout << "TEST STARTS"<<endl;
		m_Clock.setTimeNow(&m_t1);
		return 1;
		break;
	}
	case (TEST_ENDS):{
		//	printf("TEST ENDS\n");
		m_Clock.setTimeNow(&m_t2);
		saveNumbers();
		cout << "TEST ends, sending results"<<endl;
		m_commandout.m_command = TEST_RESULTS;
		m_commandout.m_demand = m_demand;
		m_commandout.m_size = m_datasize+4+4;
		m_commandout.m_lastrecsample = saved_lastseqnum;
		m_commandout.m_lostsamples = saved_lostsamples;
		m_commandout.m_totaltime = (uint64_t)(TimeConv::Time_t2MicroSecondsDouble(m_t2)-
				TimeConv::Time_t2MicroSecondsDouble(m_t1));
		//	cout << "SEND COMMAND: "<< m_commandout.m_command << endl;
		zmq::message_t command_msg(4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double));
		m_commandDataType.serialize(&m_commandout,&command_msg);
		mp_commandpub->send(command_msg);

		return 2;
		break;
	}
	case (ALL_STOPS):{
		printf("ALL STOPS\n");
		return -1;
		break;
	}
	default:
		break;
	}
	return 0;
}


void ZMQThroughputSubscriber::resetResults()
{
	lastseqnum = 0;
	lostsamples=0;
}

void ZMQThroughputSubscriber::saveNumbers()
{
	saved_lastseqnum = lastseqnum;
	saved_lostsamples = lostsamples;
}









