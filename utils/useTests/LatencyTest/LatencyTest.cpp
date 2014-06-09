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

#define NSAMPLES 5000


#include "eprosimartps/rtps_all.h"
#include "LatencyType.h"
#if defined(_WIN32)
#include "LatencyPublisher_win.h"
#else
#include "LatencyPublisher.h
#endif
#include "LatencySubscriber.h"

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


	uint32_t datas[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
	vector<uint32_t> datasize (datas, datas + sizeof(datas) / sizeof(uint32_t) );
	switch (type)
	{
	case 1:
	{
		LatencyPublisher latpub;
		cout << "Waiting for discovery"<<endl;
		latpub.sema.wait();
		latpub.sema.wait();
		cout << "Discovery completed"<<endl;
		latpub.m_part->stopParticipantAnnouncement();
		eClock::my_sleep(5000);
		cout << B_WHITE << "READY TO START" <<DEF << endl;
		printf("Printing times in us\n");
		printf(" Bytes, stdev,  mean,   min,   50%%,   90%%,   99%%, 99.99%%,   max\n");
		printf("------,------,------,------,------,------,------,------,------,\n");
		for(std::vector<uint32_t>::iterator ndata = datasize.begin();ndata!=datasize.end();++ndata)
		{
			if(!latpub.test(*ndata,NSAMPLES))
				break;
			eClock::my_sleep(500);
//			cout << "Finish Test, input to continue: ";
//			cin >> aux;
//			cout << endl;

		}
		cin >> type;
		break;
	}
	case 2:
	{
		LatencySubscriber latsub;
		cout << "Waiting for discovery"<<endl;
		latsub.sema.wait();
		latsub.sema.wait();
		cout << "Discovery Completed"<<endl;
		latsub.m_part->stopParticipantAnnouncement();
		eClock::my_sleep(1000);
		cout << B_WHITE << "READY TO START" <<DEF << endl;
		for(std::vector<uint32_t>::iterator ndata = datasize.begin();ndata!=datasize.end();++ndata)
		{
			if(!latsub.test(*ndata,NSAMPLES))
				break;
		}

		break;
	}
	}

	DomainParticipant::stopAll();


	return 0;
}



