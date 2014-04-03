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
	unsigned char data[500];
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

void LatencyGetKey(void* data,InstanceHandle_t* handle )
{
	handle->value[0]  = 0;
	handle->value[1]  = 0;
	handle->value[2]  = 0;
	handle->value[3]  = 5; //length of string in CDR BE
	handle->value[4]  = 1;
	handle->value[5]  = 2;
	handle->value[6]  = 3;
	handle->value[7]  = 4;
	handle->value[8]  = 5;
	for(uint8_t i=9;i<16;i++)
		handle->value[i]  = 0;
}


boost::posix_time::ptime t1,t2,t3;

void newMsgCallback()
{
	t2 = boost::posix_time::microsec_clock::local_time();
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


	boost::posix_time::time_duration overhead;
	//CLOCK OVERHEAD
	t1 = boost::posix_time::microsec_clock::local_time();
	for(int i=0;i<400;i++)
		t2= boost::posix_time::microsec_clock::local_time();

	overhead = (t2-t1);
	long overhead_value = ceil(overhead.total_microseconds()/400);
	cout << "Overhead " << overhead_value << endl;


	LatencyType Latency;
	//***********  PARTICIPANT  ******************//
	ParticipantParams_t PParam;
	PParam.defaultSendPort = 10042;
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	DomainParticipant::registerType(std::string("LatencyType"),&LatencySer,&LatencyDeSer,&LatencyGetKey,sizeof(LatencyType));
	//************* PUBLISHER  **********************//
	if(type == 1)
	{
		WriterParams_t Wparam;
		Wparam.pushMode = true;
		Wparam.stateKind = STATEFUL;
		Wparam.topicKind = WITH_KEY;
		Wparam.topicDataType = std::string("LatencyType");
		Wparam.topicName = std::string("This is a test topic");
		Wparam.historySize = 10;
		Wparam.reliablility.heartbeatPeriod.seconds = 4;
		Wparam.reliablility.kind = RELIABLE;
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
		if(pub == NULL)
			return 0;
		//Reader Proxy
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10043;
		loc.set_IP4_address(192,168,1,23);
		GUID_t readerGUID;
		readerGUID.entityId = ENTITYID_UNKNOWN;
		pub->addReaderProxy(loc,readerGUID,true);

		for(uint8_t i = 0;i<10;i++)
		{
			if(i == 2)
				p->loose_next_change();
			pub->write((void*)&Latency);
			cout << "Going to sleep "<< (int)i <<endl;
			sleep(3);
			cout << "Wakes "<<endl;
		}
	}
	else if (type ==2) //**********  SUBSCRIBER  **************//
	{
		ReaderParams_t Rparam;
		Rparam.historySize = 15;
		Rparam.stateKind = STATEFUL;
		Rparam.topicDataType = std::string("LatencyType");
		Rparam.topicName = std::string("This is a test topic");
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
		while(1)
		{
			cout << "Waiting for new message "<<endl;
			sub->blockUntilNewMessage();
			sub->readLastAdded((void*)&Latency);
		}

	}

	return 0;

}



