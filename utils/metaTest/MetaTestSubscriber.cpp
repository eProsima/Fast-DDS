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
 * @file MetaTestSubscriber.cpp
 *
 */

#include "MetaTestSubscriber.h"
#include "fastrtps/fastrtps_all.h"

#include "types/MetaTestTypes.h"
//TESTS INCLUDE FILES

#include "../useTests/RTPSTest_as_socket/TestReaderSocket.h"
#include "../useTests/RTPSTest_registered/TestReaderRegistered.h"
#include "../useTests/ClientServerTest/EprosimaServer.h"
#include "../useTests/HelloWorldTest/HelloWorldSubscriber.h"

namespace eprosima {

MetaTestSubscriber::MetaTestSubscriber():mp_par(nullptr),
		mp_pub(nullptr),
		mp_sub(nullptr)
{
	// TODO Auto-generated constructor stub

}

MetaTestSubscriber::~MetaTestSubscriber() {
	// TODO Auto-generated destructor stub
	Domain::removeParticipant(mp_par);
}

bool MetaTestSubscriber::init()
{
	ParticipantAttributes Patt;
	Patt.rtps.builtin.domainId = 50;
	Patt.rtps.builtin.leaseDuration = c_TimeInfinite;
	Patt.rtps.setName("MetaSubscriber");

	mp_par = Domain::createParticipant(Patt);
	if(mp_par == nullptr)
		return false;
	Domain::registerType(mp_par,&m_dataType);

	PublisherAttributes Wparam;
	Wparam.topic.topicName = "metaTest_S2P";
	Wparam.topic.topicDataType = "MetaTestType";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	mp_pub = Domain::createPublisher(mp_par,Wparam,&m_publisten);


	SubscriberAttributes Rparam;
	Rparam.topic.topicName = "metaTest_P2S";
	Rparam.topic.topicDataType = "MetaTestType";

	mp_sub = Domain::createSubscriber(mp_par,Rparam,&m_sublisten);

	if(mp_par == nullptr ||mp_pub == nullptr || mp_sub == nullptr)
		return false;
	logInfo("MetaTest Subscriber initialized");
	return true;
}

void MetaTestSubscriber::run()
{
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	while(1)
	{
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			switch(testinfo.kind())
			{
			case T_RTPS_SOCKET:
				t_rtps_socket(testinfo);
				break;
			case T_RTPS_REG:
				t_rtps_registered(testinfo);
				break;
			case T_CLIENT_SERVER:
				t_client_server(testinfo);
				break;
			case T_HELLO_WORLD:
				t_hello_world(testinfo);
				break;
			case STOP_ALL_TESTS:
				return;
			}
		}
	}
}

void MetaTestSubscriber::t_hello_world(MetaTestType& testinfo)
{
	logInfo("Starting TEST HELLO WORLD");
	HelloWorldSubscriber hwsub;
	SampleInfo_t sampleinfo;
	if(hwsub.init())
	{
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logInfo("Publisher has finished");
				if(hwsub.m_listener.n_samples == (uint16_t)testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
				{
					std::stringstream ss;
					ss << "Received "<<hwsub.m_listener.n_samples << " samples of the expected ";
					ss << (uint16_t)testinfo.samples();
					testinfo.comment(ss.str());
				}
			}
			else
			{
				testinfo.comment("Received bad finish code");
			}
		}
		else
			testinfo.comment("META ERROR");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
}

void MetaTestSubscriber::t_client_server(MetaTestType& testinfo)
{
	logInfo("Starting TEST CLIENT SERVER");
	EprosimaServer server;
	SampleInfo_t sampleinfo;
	if(server.init())
	{
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logInfo("Publisher has finished");
				if(server.m_n_served == (uint16_t)testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
					testinfo.comment("Bad samples number received");
			}
			else
				testinfo.comment("Received bad finish code");
		}
		else
			testinfo.comment("META ERROR");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
}

void MetaTestSubscriber::t_rtps_registered(MetaTestType& testinfo)
{
	logInfo("Starting TEST RTPS REGISTERED");
	TestReaderRegistered trreg;
	SampleInfo_t sampleinfo;
	if(trreg.init() && trreg.reg())
	{
		while (trreg.m_listener.n_matched==0)
		{
			eClock::my_sleep(100);
		}
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logInfo("Publisher has finished (" << testinfo.samples() << " samples send), I received: "
						<<trreg.m_listener.n_received);
				if(trreg.m_listener.n_received == (uint16_t)testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
					testinfo.comment("Bad samples number received");
			}
			else
				testinfo.comment("Received bad finish code");
		}
		else
			testinfo.comment("Meta Error");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
	return;
}



void MetaTestSubscriber::t_rtps_socket(MetaTestType& testinfo)
{
	logInfo("Starting TEST RTPS SOCKET");
	TestReaderSocket tsocket;
	SampleInfo_t sampleinfo;
	if(tsocket.init(testinfo.ip_string(),(uint32_t)testinfo.ip_port()))
	{
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logInfo("Publisher has finished (" << testinfo.samples() << " samples send), I received: "
						<<tsocket.m_listener.m_received);
				if(tsocket.m_listener.m_received == (uint16_t)testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
					testinfo.comment("Bad samples number received");
			}
			else
				testinfo.comment("Received bad finish code");
		}
		else
			testinfo.comment("Meta Error");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
	return;
}



} /* namespace eprosima */
