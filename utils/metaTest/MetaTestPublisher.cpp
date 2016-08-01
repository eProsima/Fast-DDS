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
 * @file MetaTestPublisher.cpp
 *
 */

#include "MetaTestPublisher.h"
#include "fastrtps/fastrtps_all.h"

#include "types/MetaTestTypes.h"


#include <sstream>



//TESTS INCLUDE FILES

#include "../useTests/RTPSTest_as_socket/TestWriterSocket.h"
#include "../useTests/RTPSTest_registered/TestWriterRegistered.h"
#include "../useTests/ClientServerTest/EprosimaClient.h"
#include "../useTests/HelloWorldTest/HelloWorldPublisher.h"

namespace eprosima {

MetaTestPublisher::MetaTestPublisher():mp_par(nullptr),
		mp_pub(nullptr),
		mp_sub(nullptr)
{
	// TODO Auto-generated constructor stub

}

MetaTestPublisher::~MetaTestPublisher() {
	// TODO Auto-generated destructor stub
	Domain::removeParticipant(mp_par);
}

bool MetaTestPublisher::init()
{
	ParticipantAttributes Patt;
	Patt.rtps.builtin.domainId = 50;
	Patt.rtps.builtin.leaseDuration = c_TimeInfinite;
	Patt.rtps.setName("MetaPublisher");

	mp_par = Domain::createParticipant(Patt);
	if(mp_par == nullptr)
		return false;
	Domain::registerType(mp_par,&m_dataType);

	PublisherAttributes Wparam;
	Wparam.topic.topicName = "metaTest_P2S";
	Wparam.topic.topicDataType = "MetaTestType";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;

	mp_pub = Domain::createPublisher(mp_par,Wparam,&m_publisten);


	SubscriberAttributes Rparam;
	Rparam.topic.topicName = "metaTest_S2P";
	Rparam.topic.topicDataType = "MetaTestType";

	mp_sub = Domain::createSubscriber(mp_par,Rparam,&m_sublisten);

	if(mp_par == nullptr || mp_pub == nullptr || mp_sub == nullptr)
		return false;
	logInfo("MetaTest Publisher initialized");
	return true;
}

void MetaTestPublisher::run()
{
	while(m_publisten.n_matched == 0 || m_sublisten.n_matched == 0)
		eClock::my_sleep(300);
	std::stringstream ss;
	//RUN ALL TESTS

	ss << "T_RTPS_REGISTERED: " << t_rtps_registered() << endl;
	clean();
	ss << "T_RTPS_SOCKET    : " << t_rtps_socket() << endl;
	clean();
	ss << "T_CLIENT_SERVER  : " << t_client_server() << endl;
	clean();
	ss << "T_HELLO_WORLD    : " << t_hello_world() << endl;
		clean();

	MetaTestType testinfo;
	testinfo.kind(STOP_ALL_TESTS);
	mp_pub->write(&testinfo);


	logInfo("TEST SUMMARY"<<endl<<ss.str());
}

void MetaTestPublisher::clean()
{
	mp_pub->removeAllChange();
	eClock::my_sleep(500);
}

std::string MetaTestPublisher::t_hello_world()
{
	logInfo("Starting TEST HELLO WORLD");
	int samples = 10;
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	testinfo.kind(T_HELLO_WORLD);
	testinfo.samples(samples);
	mp_pub->write(&testinfo);
	HelloWorldPublisher hwpub;
	if(hwpub.init())
	{
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_SUB_READY)
			{
				printf("Running\n");
				hwpub.run(samples);
				printf("Finished");
				eClock::my_sleep(200);
				testinfo.status(T_PUB_FINISH);
				testinfo.samples(samples);
				mp_pub->write(&testinfo);
				mp_sub->waitForUnreadMessage();
				if(mp_sub->takeNextData(&testinfo,&sampleinfo))
				{
					if(testinfo.status() == T_SUB_OK)
						return "OK";
					else
					{
						std::stringstream ss;
						ss << "FAILED "<<testinfo.comment();
						return ss.str();
					}
				}
				return "META ERROR";
			}
			else if(testinfo.status() == T_SUB_FAILED)
			{
				std::stringstream ss;
				ss << "FAILED "<<testinfo.comment();
				return ss.str();
			}
		}
		return "META ERROR";
	}
	return " HelloWorld Publisher not initialized";
}


std::string MetaTestPublisher::t_client_server()
{
	logInfo("Starting TEST CLIENT SERVER");
	int samples = 20;
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	testinfo.kind(T_CLIENT_SERVER);
	testinfo.samples(samples);
	mp_pub->write(&testinfo);
	EprosimaClient client;
	if(client.init())
	{
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_SUB_READY)
			{
				int32_t res;
				while(!client.isReady())
				{
					eClock::my_sleep(100);
				}
				cout << "Running client for "<< samples << " samples"<<endl;
				for(int i = 0;i<samples;++i)
				{
					if(client.calculate(Operation::ADDITION,10,20,&res) != Result::GOOD_RESULT)
						return "Bad Calculation performed";
				}
				testinfo.status(T_PUB_FINISH);
				mp_pub->write(&testinfo);
				mp_sub->waitForUnreadMessage();
				if(mp_sub->takeNextData(&testinfo,&sampleinfo))
				{
					if(testinfo.status() == T_SUB_OK)
						return "OK";
					else
						return "Subscriber Failed";
				}
				return "META ERROR";

			}
			else if(testinfo.status() == T_SUB_FAILED)
			{
				std::stringstream ss;
				ss << "FAILED "<<testinfo.comment();
				return ss.str();
			}
		}
		return "Meta Error";
	}
	return " Client not initialized";
}

std::string MetaTestPublisher::t_rtps_registered()
{
	logInfo("Starting TEST RTPS REGISTERED");
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	testinfo.kind(T_RTPS_REG);
	mp_pub->write(&testinfo);
	TestWriterRegistered twreg;
	if(twreg.init() && twreg.reg())
	{
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_SUB_READY)
			{
				twreg.run(10);
				eClock::my_sleep(150);
				testinfo.status(T_PUB_FINISH);
				testinfo.samples(10);
				mp_pub->write(&testinfo);
				mp_sub->waitForUnreadMessage();
				if(mp_sub->takeNextData(&testinfo,&sampleinfo))
				{
					if(testinfo.status() == T_SUB_OK)
						return "OK";
					else
						return "Subscriber Failed";
				}
				return "META ERROR";
			}
			else if(testinfo.status() == T_SUB_FAILED)
			{
				std::stringstream ss;
				ss << "FAILED "<<testinfo.comment();
				return ss.str();
			}
		}
		return "META ERROR";
	}
	return " Writer not initialized";
}



std::string MetaTestPublisher::t_rtps_socket()
{
	logInfo("Starting TEST RTPS SOCKET");
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	std::string ip_string = "239.255.1.4";
	uint32_t port = 22222;
	//TEST T_RTPS_SOCKET
	testinfo.kind(T_RTPS_SOCKET);
	testinfo.ip_string(ip_string);
	testinfo.ip_port(port);
	mp_pub->write(&testinfo);
	mp_sub->waitForUnreadMessage();
	if(mp_sub->takeNextData(&testinfo,&sampleinfo))
	{
		if(testinfo.status() == T_SUB_READY)
		{
			TestWriterSocket tsocket;
			if(tsocket.init("239.255.1.4",port))
			{
				tsocket.run(10);
				eClock::my_sleep(150);
				testinfo.status(T_PUB_FINISH);
				testinfo.samples(10);
				mp_pub->write(&testinfo);
				mp_sub->waitForUnreadMessage();
				if(mp_sub->takeNextData(&testinfo,&sampleinfo))
				{
					if(testinfo.status() == T_SUB_OK)
						return "OK";
					else
					{
						std::stringstream ss;
						ss << "FAILED "<<testinfo.comment();
						return ss.str();
					}
				}
			}
			else
			{
				return "Writer FAILED Initialization";
			}
		}
		else if(testinfo.status() == T_SUB_FAILED)
		{
			std::stringstream ss;
			ss << "FAILED "<<testinfo.comment();
			return ss.str();
		}
	}
	return "META ERROR";
}




} /* namespace eprosima */
