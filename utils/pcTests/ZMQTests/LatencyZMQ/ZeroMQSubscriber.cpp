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
 * @file ZeroMQSubscriber.cpp
 *
 */

#include "fastrtps/utils/TimeConversion.h"
#include "ZeroMQSubscriber.h"
#include <sstream>
#include <vector>

uint32_t datassub[] = {12,28,60,124,252,508,1020,2044,4092,8188,16380};
std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );


ZeroMQSubscriber::ZeroMQSubscriber():
						mp_context(nullptr),
						mp_datapub(nullptr),
						mp_commandpub(nullptr),
						mp_datasub(nullptr),
						mp_commandsub(nullptr),
						n_samples(1000),
						n_sub(0)
{

}

ZeroMQSubscriber::~ZeroMQSubscriber()
{
	// TODO Auto-generated destructor stub
}

bool ZeroMQSubscriber::init(int nsub,string pubip,int samples)
{
	n_samples = samples;
	n_sub = nsub;
	mp_context = new zmq::context_t(1);
	mp_datasub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	cout << "PUB IP: "<< pubip << endl;
	stringstream ss;
	ss << "tcp://"<<pubip<<":17551";
	cout << "Connecting to Data Publisher: "<< ss.str()<<endl;
	mp_datasub->connect(ss.str().c_str());
	mp_datasub->setsockopt(ZMQ_SUBSCRIBE,0,0);
	mp_commandsub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	stringstream ss2;
	ss2 << "tcp://"<<pubip<<":17552";
	cout << "Connecting to Command Publisher: "<< ss2.str()<<endl;
	mp_commandsub->connect(ss2.str().c_str());
	mp_commandsub->setsockopt(ZMQ_SUBSCRIBE,0,0);
	eClock::my_sleep(300);


	mp_datapub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	int port = 17553 + n_sub-1;
	std::stringstream ss3;
	ss3 << "tcp://*:"<< port;
	cout << "Creating Data Publisher: "<< ss3.str()<<endl;
	mp_datapub->bind(ss3.str().c_str());
	//mp_datapub->bind("ipc://latency.ipc");
	mp_commandpub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	port = 17573 + n_sub-1;
	std::stringstream ss4;
	ss4 << "tcp://*:"<< port;
	cout << "Creating Command Publisher: "<< ss4.str()<<endl;
	mp_commandpub->bind(ss4.str().c_str());
	//mp_commandpub->bind("ipc://command2sub.ipc");
	eClock::my_sleep(200);



	return true;
}


void ZeroMQSubscriber::run()
{
	for(std::vector<uint32_t>::iterator ndata = data_size_sub.begin();ndata!=data_size_sub.end();++ndata)
	{
		if(!this->test(*ndata))
			break;
	}
}

bool ZeroMQSubscriber::test(uint32_t datasize)
{
	cout << "Preparing test with data size: " << datasize+4<<endl;
	zmq::message_t command(1);



	//cout << "WAITING FOR COMMAND"<<endl;
	mp_commandsub->recv(&command);
	//cout << "COMMAND RECEIVED"<<endl;
	if(*(char*)command.data()!='S')
	{
		return false;
	}
	*(char*)command.data()=2;
	mp_commandpub->send(command);
	for(uint32_t i = 0;i<(uint32_t)n_samples;++i)
	{
		//cout << "waiting for data "<<endl;
		zmq::message_t latency_in;
		zmq::message_t latency_out(datasize+4);
		mp_datasub->recv(&latency_in);
		if(n_sub == 1)
		{
			//cout << "received of size:"<<latency_in.size()<<endl;
			//		std::istringstream iss(static_cast<char*>(latency_in.data()));
			//		cout << "RECEIVED DATA: "<< iss.str()<< endl;
			//
			//		memset(latency_out.data(),'S',datasize+4);
			memcpy(latency_out.data(),latency_in.data(),latency_in.size());
			//	cout << "REC/SENT: "<< *(uint32_t*)latency_in.data() <<" / "<<*(uint32_t*)latency_out.data()<<endl;
			mp_datapub->send(latency_out);
		}
	}
	cout << "TEST OF SiZE: "<< datasize +4 << " ENDS"<<endl;
	eClock::my_sleep(100);


	return true;
}

