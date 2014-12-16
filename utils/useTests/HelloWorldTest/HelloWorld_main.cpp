/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include "fastrtps/Domain.h"

#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/RTPSLog.h"

#include "fastrtps/rtps/rtps_all.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
	Log::setVerbosity(VERB_INFO);
	cout << "Starting "<< endl;
	int type = 1;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}
	else
	{
		cout << "publisher OR subscriber argument needed"<<endl;
		return 0;
	}


	switch(type)
	{
	case 1:
	{
		/*
		//RTPSParticipantAttributes patt;
		//patt.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
		patt.builtin.use_WriterLivelinessProtocol = false;
		RTPSParticipant* part = RTPSDomain::createParticipant(patt);
		WriterAttributes watt;
		watt.endpoint.reliabilityKind = BEST_EFFORT;
		HistoryAttributes hatt;
		WriterHistory* hist = new WriterHistory(hatt);
		RTPSWriter* writer=RTPSDomain::createRTPSWriter(part,watt,hist);
		RemoteReaderAttributes ratt;
		Locator_t loc;
		loc.set_IP4_address(192,168,1,27);
		loc.port = 27405;
		//ratt.endpoint.unicastLocatorList.push_back(loc);
		loc.set_IP4_address(169, 254, 229, 255);
		ratt.endpoint.unicastLocatorList.push_back(loc);
		writer->matched_reader_add(ratt);
		CacheChange_t* change = writer->new_change(ALIVE);
		change->serializedPayload.length = 2;
		hist->add_change(change);
		*/

		HelloWorldPublisher mypub;
		for(int i = 0;i<10;++i)
		{
			if(mypub.publish())
			{
				eClock::my_sleep(500);
			}
			else
			{
				//cout << "Sleeping till discovery"<<endl;
				eClock::my_sleep(200);
				--i;
			}
}
		break;
	}
	case 2:
	{
		HelloWorldSubscriber mysub;
		cout << "Waiting for messages, press enter to stop."<<endl;
		std::cin.ignore();
		break;
	}
	}

	Domain::stopAll();

	return 0;
}
