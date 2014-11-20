/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/



#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>

#include "eprosimartps/rtps_all.h"

#include "ThroughputTypes.h"

#include "ThroughputPublisher.h"
#include "ThroughputSubscriber.h"

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
	uint32_t test_time_sec = 30;
	int demand = 0;
	int msg_size = 0;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		if(strcmp(argv[1],"subscriber")==0)
			type = 2;
		if(argc > 2 && type == 1)
		{
			std::istringstream iss( argv[2] );
			if (!(iss >> test_time_sec))
			{
				cout << "Problem reading test time,using 30s as default value "<< endl;
				test_time_sec = 30;
			}
		}
		if (argc > 3 && type == 1)
		{
			std::istringstream iss_demand( argv[3] );
			if (!(iss_demand >> demand))
			{
				cout << "Problem reading demand,using default demand vector "<< endl;
				demand = 0;
			}
		}
		if (argc > 4 && type == 1)
		{
			std::istringstream iss_size( argv[4] );
			if (!(iss_size >> msg_size))
			{
				cout << "Problem reading msg size,using default size vector "<< endl;
				msg_size = 0;
			}
		}
	}
	else
	{
		cout << "NEEDS publisher OR subscriber ARGUMENT"<<endl;
		cout << "Usage: "<<endl;
		cout << "ThroughputTest \"publisher\" SECONDS DEMAND MSG_SIZE"<<endl;
		cout << "ThroughputTest \"subscriber\""<<endl;
		return 0;
	}

	LatencyDataType latency_t;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&latency_t);

	ThroughputCommandDataType throuputcommand_t;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&throuputcommand_t);



	switch (type)
	{
	case 1:
	{
		ThroughputPublisher tpub;
		tpub.run(test_time_sec,demand,msg_size);
		break;
	}
	case 2:
	{

		ThroughputSubscriber tsub;
		tsub.run();
		break;
	}
	}

	DomainRTPSParticipant::stopAll();


	return 0;
}



