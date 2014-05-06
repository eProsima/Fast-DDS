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
#include "eprosimartps/qos/ParameterList.h"
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

const int num_length = 100;
const int len_laten = num_length+8;

typedef struct LatencyType{
	int64_t seqnum;
	unsigned char nums[num_length];
	LatencyType()
	{
		seqnum = 0;
		cout << num_length << " "<< len_laten << endl;
		for(int i=0;i<num_length;i++)
		{
			nums[i] = 1;
		}
	}
}LatencyType;

LatencyType g_latency;

void LatencySer(SerializedPayload_t* payload,void*data)
{
	memcpy(payload->data,data,len_laten);
	payload->length = len_laten;
}

void LatencyDeSer(SerializedPayload_t* payload,void*data)
{
	memcpy(data,payload->data,payload->length);
}



class LatencyDataType: public DDSTopicDataType
{
public:
	LatencyDataType()
{
		m_topicDataTypeName = "LatencyType";
		m_typeSize = len_laten;
		m_isGetKeyDefined = false;
};
	~LatencyDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};

//Funciones de serializacion y deserializacion para el ejemplo
bool LatencyDataType::serialize(void*data,SerializedPayload_t* payload)
{
	memcpy(payload->data,data,len_laten);
		payload->length = len_laten;
	return true;
}

bool LatencyDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	memcpy(data,payload->data,payload->length);
	return true;
}


boost::posix_time::ptime t1,t2,t3;


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



	//my_sleep(1);
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	LatencyDataType latency_t;
	DomainParticipant::registerType((DDSTopicDataType*)&latency_t);
	Locator_t loc;
	loc.kind = 1;
	loc.port = 10469;
	loc.set_IP4_address(192,168,1,16);

	SubscriberAttributes Rparam;
	PublisherAttributes Wparam;
	if(type == 1)
	{
		Wparam.historyMaxSize = 10;
		Rparam.historyMaxSize = 15;
	}
	else if(type == 2)
	{
		Wparam.historyMaxSize = 1000;
		Rparam.historyMaxSize = 1000;
	}

	//Create both publisher and subscriber.
	Wparam.pushMode = true;
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicDataType = std::string("LatencyType");
	Wparam.topic.topicName = std::string("This is a test topic");
	Publisher* pub = DomainParticipant::createPublisher(p,Wparam);

	if(type == 1)
	{
		loc.set_IP4_address(192,168,1,23);
		pub->addReaderLocator(loc,false);
	}
	else if(type == 2)
	{
		loc.set_IP4_address(192,168,1,27);
		pub->addReaderLocator(loc,false);
	}


	Rparam.topic.topicDataType = std::string("LatencyType");
	Rparam.topic.topicName = std::string("This is a test topic");
	Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
	Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);


	SequenceNumber_t seq;
	GUID_t guid;
	uint64_t total=0;
	uint16_t n_samples = 1000;
	uint64_t us;
	uint64_t min_us= 150000;
	int samples = 0;
	SampleInfo_t info;
	for(int i=1;i<n_samples;i++)
	{
		if(type == 1)
		{
			timeval t1,t2;
			int64_t seqnum;
			g_latency.seqnum++;
			seqnum = g_latency.seqnum;
			//t1 = boost::posix_time::microsec_clock::local_time();
			gettimeofday(&t1,NULL);
			pub->write((void*)&g_latency);
			sub->waitForUnreadMessage();
			sub->readNextData((void*)&g_latency,&info);
			gettimeofday(&t2,NULL);
			//t2 = boost::posix_time::microsec_clock::local_time();
			if(seqnum == g_latency.seqnum)
			{
				cout << "ok ";
			}
			us=(t2.tv_sec-t1.tv_sec)*1000000+t2.tv_usec-t1.tv_usec;
			//if((t2.tv_usec-t1.tv_usec)>0)
			//us = (t2-t1).total_microseconds()-overhead_value;
			//{	us = t2.tv_usec-t1.tv_usec;}
			//else
			//{us = pow(10.0,6)+t2.tv_usec-t1.tv_usec;}
			cout<< "T: " <<us<< " | "<<endl;
			total+=us;
			if(us<min_us)
			{
				min_us = us;
			}
			samples++;
			if(samples%10==0)
			{
				cout << endl;
			}
			pub->removeMinSeqChange();
			sub->takeNextData((void*)&g_latency,&info);
		}
		else if (type == 2)
		{
			sub->waitForUnreadMessage();
			sub->readNextData((void*)&g_latency,&info);
			pub->write((void*)&g_latency);
		}
	}
	cout << endl;
	if(type == 1)
	{
		cout << "Mean: " << total/samples << endl;
		cout << "Min us: " << min_us << endl;
	}

	return 0;
}



