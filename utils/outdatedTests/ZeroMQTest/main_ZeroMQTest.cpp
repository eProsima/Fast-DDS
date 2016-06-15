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
 * @file main_ClientServerTest.cpp
 *
 */


//#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//#include <assert.h>
#include <iostream>

#include "ZeroMQPublisher.h"
#include "ZeroMQSubscriber.h"

using namespace std;

int main (int argc, char** argv)
{
	int type = 0;
	std::string ipstr;
	int nsamples;
	if(argc > 3)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
		else
		{
			cout << "NEEDS publisher OR subscriber as first argument"<<endl;
			return 0;
		}
		std::istringstream ip(argv[2]);
		ipstr = ip.str();
		std::istringstream nsampl( argv[3] );
		if(!(nsampl>>nsamples))
		{
			cout << "Problem reading samples number"<<endl;
		}
	}
	else
	{
		cout << "Application needs at least 3 arguments: publisher/subscriber SUB_IP/PUB_IP N_SAMPLES"<<endl;
		return 0;
	}


	if(type == 1)
	{
		ZeroMQPublisher zmqpub;
		zmqpub.init(ipstr,nsamples);
		zmqpub.run();
	}
	else if(type ==2)
	{
	ZeroMQSubscriber zmqsub;
		zmqsub.init(ipstr,nsamples);
		zmqsub.run();
	}
	else
		return 0;
	//TEST WITHOUT LATENCY STUFF:

//		if(type == 1)
//		{
//			try{
//			zmq::context_t context (1);
//			zmq::socket_t publisher (context, ZMQ_PUB);
//			publisher.bind("tcp://*:5556");
//			cout << "HERE GOOD"<<endl;
//		//	publisher.bind("ipc://weather.ipc");
//			cout << "HERE GOOD2"<<endl;
//			eClock::my_sleep(300);
//			zmq::message_t latency_out(12+4);
//			memset(latency_out.data(),65,12+4);
//			std::istringstream iss2(static_cast<char*>(latency_out.data()));
//			cout << "SENDING DATA: "<< iss2.str()<< endl;
//			uint32_t i =2;
//			memcpy(latency_out.data(),(void*)&i,sizeof(uint32_t));
//			std::istringstream iss3(static_cast<char*>(latency_out.data()));
//			cout << "SENDING DATA: "<< iss3.str()<< endl;
//			cout << "UN BYTE: "<< *((char*)latency_out.data()+6) << endl;
//			for(int i =0;i<20;++i)
//			{
//				cout << "sends"<<endl;
//				publisher.send(latency_out);
//			}
//			}
//			catch(const zmq::error_t& ze)
//{
//   std::cout << "Exception: " << ze.what() << std::endl;
//   int aux;
//   std::cin >> aux;
//}
//	
//		}
//		else if(type ==2)
//		{
//			zmq::context_t context (1);
//			//  Socket to talk to server
//			std::cout << "Collecting updates from weather server…\n" << std::endl;
//			zmq::socket_t subscriber (context, ZMQ_SUB);
//			subscriber.connect("tcp://localhost:5556");
//			//  Subscribe to zipcode, default is NYC, 10001
//			subscriber.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
//			eClock::my_sleep(300);
//			zmq::message_t update;
//			while(1)
//			{
//				cout << "waiting"<<endl;
//				subscriber.recv(&update);
//				cout << "rec"<<endl;
//				std::istringstream iss(static_cast<char*>(update.data()));
//				cout << "RECEIVED: "<<iss.str() << endl;
//				cout << "UN BYTE: "<< *((char*)update.data()+6) << endl;
//			}
//		}
//		else
//			return 0;
//

}

