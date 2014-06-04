/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessTest.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

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
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}
	else
	{
		cout << "NEEDS publisher OR subscriber ARGUMENT"<<endl;
		return 0;
	}

	LatencyDataType latency_t;
	DomainParticipant::registerType((DDSTopicDataType*)&latency_t);

	ThroughputCommandDataType throuputcommand_t;
	DomainParticipant::registerType((DDSTopicDataType*)&throuputcommand_t);



	//uint32_t n_samples = 10000;

//	uint32_t datas[] = {16,32,64,128,256,512,1024,2048,4096};
//	vector<uint32_t> datasize (datas, datas + sizeof(datas) / sizeof(uint32_t) );
	switch (type)
	{
	case 1:
	{
		ThroughputPublisher tpub;
		tpub.run();
//		int aux;
//		LatencyPublisher latpub;
//		cout << "Waiting for discovery"<<endl;
//		latpub.sema.wait();
//		latpub.sema.wait();
//		latpub.m_part->stopParticipantAnnouncement();
//		eClock::my_sleep(5000);
//		cout << B_WHITE << "READY TO START" <<DEF << endl;
//		printf("Printing times in us\n");
//		printf(" Bytes,  Mean, stdev,   min,   max,   50%%,   90%%,   99%%, 99.99%%\n");
//		printf("------,------,------,------,------,------,------,------,------,\n");
//		for(std::vector<uint32_t>::iterator ndata = datasize.begin();ndata!=datasize.end();++ndata)
//		{
//			if(!latpub.test(*ndata,n_samples))
//				break;
//			eClock::my_sleep(500);
////			cout << "Finish Test, input to continue: ";
////			cin >> aux;
////			cout << endl;
//
//		}
		break;
	}
	case 2:
	{

		ThroughputSubscriber tsub;
		tsub.run();
//		LatencySubscriber latsub;
//		cout << "Waiting for discovery"<<endl;
//		latsub.sema.wait();
//		latsub.sema.wait();
//		latsub.m_part->stopParticipantAnnouncement();
//		eClock::my_sleep(1000);
//		cout << B_WHITE << "READY TO START" <<DEF << endl;
//		for(std::vector<uint32_t>::iterator ndata = datasize.begin();ndata!=datasize.end();++ndata)
//		{
//			if(!latsub.test(*ndata,n_samples))
//				break;
//		}

		break;
	}
	}

	DomainParticipant::stopAll();


	return 0;
}



