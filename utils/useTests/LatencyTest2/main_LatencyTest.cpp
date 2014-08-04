/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/



#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>



#include "eprosimartps/rtps_all.h"

#include "LatencyTestPublisher.h"
#include "LatencyTestSubscriber.h"

using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;


#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif


int main(int argc, char** argv){
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type;
	int sub_number = 1;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		if(strcmp(argv[1],"subscriber")==0)
			type = 2;
		if(argc > 2 && type == 1)
		{
			std::istringstream iss( argv[2] );
			if (iss >> sub_number)
			{

			}
			else
			{
				cout << "Problem reading subscriber number "<< endl;
			}
		}

	}
	else
	{
		cout << "NEEDS publisher OR subscriber ARGUMENT"<<endl;
		return 0;
	}

	LatencyDataType latency_t;
	DomainParticipant::registerType((DDSTopicDataType*)&latency_t);

	TestCommandDataType command_t;
	DomainParticipant::registerType((DDSTopicDataType*)&command_t);

	switch (type)
	{
	case 1:
	{
		cout << "Performing test with "<< sub_number << " subscribers"<<endl;
		LatencyTestPublisher latencyPub;
		latencyPub.init(sub_number);
		latencyPub.run();
		break;
	}
	case 2:
	{
		LatencyTestSubscriber latencySub;
		latencySub.init();
		latencySub.run();
		break;
	}
	}

	DomainParticipant::stopAll();


	return 0;
}



