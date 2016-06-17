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


#include <iostream>
#include <sstream>

#include "ZeroMQPublisher.h"
#include "ZeroMQSubscriber.h"

using namespace std;

int main (int argc, char** argv)
{
	int type = 0;
	std::vector<std::string> sub_ips;
	std::string pub_ip;
	int nsamples;
	int n_subscribers;
	int sub_number;
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
		if(type ==1)//PUBLISHER
		{
			std::istringstream n_subs( argv[2] );
			if(!(n_subs>>n_subscribers) || n_subscribers <= 0)
				cout << "Number of subscribers > 0"<<endl;
			for(auto i = 0;i<n_subscribers;++i)
			{
				std::istringstream ip(argv[3+i]);
				sub_ips.push_back(ip.str());
			}
			std::istringstream nsampl( argv[3+n_subscribers]);
			if(!(nsampl>>nsamples))
				cout << "Error reading data samples number"<<endl;
		}
		else
		{
			std::istringstream n_subs( argv[2] );
			if(!(n_subs>>sub_number) || sub_number <= 0)
				cout << "Subscriber Number > 0"<<endl;
			std::istringstream ip(argv[3]);
			pub_ip = ip.str();
			std::istringstream nsampl( argv[4]);
			if(!(nsampl>>nsamples))
				cout << "Error reading data samples number"<<endl;
		}
	}
	else
	{
		cout << "Application needs the following arguments:"<<endl;
		cout << "ZMQTest publisher N_SUBSCRIBERS SUB1_IP SUB2_IP... N_SAMPLES"<<endl;
		cout << "ZMQTest subscriber SUB_NUMBER PUB_IP N_SAMPLES"<<endl;
		cout << "Example: "<<endl;
		cout << "ZMQTest publisher 2 192.168.1.11 192.168.1.12 10000"<<endl;
		cout << "ZMQTest subscriber 1 192.168.1.10 10000" <<endl;
		return 0;
	}


	if(type == 1)
	{
		ZeroMQPublisher zmqpub;
		zmqpub.init(sub_ips,nsamples);
		zmqpub.run();
	}
	else if(type ==2)
	{
		ZeroMQSubscriber zmqsub;
		zmqsub.init(sub_number,pub_ip,nsamples);
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

