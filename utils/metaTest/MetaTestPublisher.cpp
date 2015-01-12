/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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

	mp_pub = Domain::createPublisher(mp_par,Wparam,&m_publisten);


	SubscriberAttributes Rparam;
	Rparam.topic.topicName = "metaTest_S2P";
	Rparam.topic.topicDataType = "MetaTestType";

	mp_sub = Domain::createSubscriber(mp_par,Rparam,&m_sublisten);

	if(mp_par == nullptr || mp_pub == nullptr || mp_sub == nullptr)
		return false;
	logUser("MetaTest Publisher initialized");
	return true;
}

void MetaTestPublisher::run()
{
	while(m_publisten.n_matched == 0 || m_sublisten.n_matched == 0)
		eClock::my_sleep(300);
	std::stringstream ss;
	//RUN ALL TESTS
	ss << "T_RTPS_REGISTERED: " << t_rtps_registered() << endl;
	ss << "T_RTPS_SOCKET    : " << t_rtps_socket() << endl;



	MetaTestType testinfo;
	testinfo.kind(STOP_ALL_TESTS);
	mp_pub->write(&testinfo);


	logUser("TEST SUMMARY"<<endl<<ss.str());
}

std::string MetaTestPublisher::t_rtps_registered()
{
	logUser("Starting TEST RTPS REGISTERED");
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
		}
	}
	return " Writer not initialized";
}



std::string MetaTestPublisher::t_rtps_socket()
{
	logUser("Starting TEST RTPS SOCKET");
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
