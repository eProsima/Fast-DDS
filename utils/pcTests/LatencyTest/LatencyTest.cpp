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
//
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/common/colors.h"
#include "eprosimartps/dds/ParameterList.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"



using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;


#define WR 1 //Writer 1, Reader 2

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

	
typedef struct LatencyType{
	int64_t seqnum;
	unsigned char data[5];
	LatencyType()
	{
		seqnum = 0;
	}
}LatencyType;


void LatencySer(SerializedPayload_t* payload,void*data)
{
	memcpy(payload->data,data,sizeof(LatencyType));
}

void LatencyDeSer(SerializedPayload_t* payload,void*data)
{
	memcpy(data,payload->data,sizeof(LatencyType));
}






void newMsgCallback()
{
	cout << MAGENTA"New Message Callback" <<DEF<< endl;
}



int main(int argc, char** argv){
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_DEBUGINFO_VERBOSITY_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type;
	if(argc > 1)
	{
		RTPSLog::Info << "Parsing arguments: " << argv[1] << endl;
		RTPSLog::printInfo();
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}
	else
		type = WR;

	boost::posix_time::ptime t1,t2,t3;
	boost::posix_time::time_duration overhead;
	//CLOCK OVERHEAD
	t1 = boost::posix_time::microsec_clock::local_time();
	for(int i=0;i<400;i++)
		t2= boost::posix_time::microsec_clock::local_time();

	overhead = (t2-t1);
	long overhead_value = ceil(overhead.total_microseconds()/400);
	cout << "Overhead " << overhead.total_microseconds()/400 << endl;



	//my_sleep(1);
	ParticipantParams_t PParam;
	PParam.defaultSendPort = 14456;
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	DomainParticipant::registerType(std::string("LatencyType"),&LatencySer,&LatencyDeSer,NULL,sizeof(LatencyType));
	Locator_t loc;
	loc.kind = 1;
	loc.port = 10469;
	loc.set_IP4_address(192,168,1,16);

	ReaderParams_t Rparam;
	WriterParams_t Wparam;
	if(type == 1)
	{
		Wparam.historySize = 10;
		Rparam.historySize = 15;
	}
	else if(type == 2)
	{
		Wparam.historySize = 1000;
		Rparam.historySize = 1000;
	}


	//Create both publisher and subscriber.


	Wparam.pushMode = true;
	Wparam.stateKind = STATELESS;
	Wparam.topicKind = NO_KEY;
	Wparam.topicDataType = std::string("LatencyType");
	Wparam.topicName = std::string("This is a test topic");
	Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
	//One of these two as locators.
//	loc.set_IP4_address(192,168,1,18);
	//	pub->addReaderLocator(loc,true);
	if(type == 1)
	{
		loc.set_IP4_address(192,168,1,21);
		pub->addReaderLocator(loc,false);
	}
	else if(type == 2)
	{
		loc.set_IP4_address(192,168,1,16);
		pub->addReaderLocator(loc,false);
	}


	Rparam.stateKind = STATELESS;
	Rparam.topicDataType = std::string("LatencyType");
	Rparam.topicName = std::string("This is a test topic");
	Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
	Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);

	LatencyType Latency;

	SequenceNumber_t seq;
	GUID_t guid;
	uint64_t total=0;
	uint16_t n_samples = 1000;
	uint64_t us[n_samples];
	int samples = 0;

	for(int i=1;i<n_samples;i++)
	{

		if(type == 1)
		{
			Latency.seqnum++;
			t1 = boost::posix_time::microsec_clock::local_time();
			pub->write((void*)&Latency);
			sub->blockUntilNewMessage();
		//	sub->readLastAdded((void*)&Latency);
			t2 = boost::posix_time::microsec_clock::local_time();
			cout<< "T: " <<(t2-t1).total_microseconds()-overhead_value<< " | ";
			total+=(t2-t1).total_microseconds()-overhead_value;
			us[i] = (t2-t1).total_microseconds()-overhead_value;
			samples++;
			if(samples%10==0)
				cout << endl;
			pub->removeMinSeqChange();
			sub->takeMinSeqCache((void*)&Latency);
		}
		else if (type == 2)
		{
			sub->blockUntilNewMessage();
			sub->readLastAdded((void*)&Latency);
			pub->write((void*)&Latency);
		}
	}
	cout << endl;
	if(type == 1)
	{
		cout << "Mean: " << total/samples << endl;
		uint64_t min_us= 150000;
		for(int i =0;i<n_samples;i++)
		{
			min_us = min(min_us,us[i]);
		}
		cout << "Min us: " << min_us << endl;

	}



	return 0;

}



