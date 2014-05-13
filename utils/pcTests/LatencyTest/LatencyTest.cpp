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

#define IPTEST0 16
#define IPTEST1 23
#define IPTEST2 25
#define IPTEST3 27
#define IPTESTWIN 11


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


class LatencyListener1 : public SubscriberListener
{
public:
	LatencyListener1():m_pub(NULL),m_sub(NULL){};
	~LatencyListener1(){};
	Publisher* m_pub;
	Subscriber* m_sub;
	LatencyType m_latency;
	SampleInfo_t m_info;
	void onNewDataMessage()
	{
		m_sub->readNextData((void*)&m_latency,&m_info);
 		gettimeofday(&m_t2,NULL);
	}
	timeval m_t1,m_t2;
};


class LatencyListener2 : public SubscriberListener
{
public:
	LatencyListener2():m_pub(NULL),m_sub(NULL){};
	~LatencyListener2(){};
	Publisher* m_pub;
	Subscriber* m_sub;
	LatencyDataType m_latency;
	SampleInfo_t m_info;
	void onNewDataMessage()
	{
		m_sub->readNextData((void*)&m_latency,&m_info);
		m_pub->write((void*)&m_latency);
	}
};



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

	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	LatencyDataType latency_t;
	DomainParticipant::registerType((DDSTopicDataType*)&latency_t);

	switch (type)
	{
	case 1:
	{
		LatencyListener1 listener;
		gettimeofday(&listener.m_t1,NULL);
		for(int i=0;i<400;i++)
			gettimeofday(&listener.m_t2,NULL);
		long overhead_value = listener.m_t1.tv_sec*pow(10,6)+listener.m_t1.tv_usec-listener.m_t2.tv_sec*pow(10,6)-listener.m_t1.tv_usec;
		cout << "Overhead " << overhead_value << endl;
		PublisherAttributes Wparam;
		Wparam.historyMaxSize = 20;
		Wparam.topic.topicDataType = "LatencyType";
		Wparam.topic.topicKind = NO_KEY;
		listener.m_pub = DomainParticipant::createPublisher(p,Wparam);
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10469;
		loc.set_IP4_address(192,168,1,IPTEST2);
		listener.m_pub->addReaderLocator(loc,false);
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 15;
		Rparam.topic.topicDataType = std::string("LatencyType");
		loc.set_IP4_address(0,0,0,0);
		Rparam.unicastLocatorList.push_back(loc);
		listener.m_sub = DomainParticipant::createSubscriber(p,Rparam);
		listener.m_sub->assignListener((SubscriberListener*)&listener);
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
			int64_t seqnum;
			listener.m_latency.seqnum++;
			seqnum = listener.m_latency.seqnum;
			gettimeofday(&listener.m_t1,NULL);
			listener.m_pub->write((void*)&listener.m_latency);
			listener.m_sub->waitForUnreadMessage();
			//t2 = boost::posix_time::microsec_clock::local_time();
			if(seqnum == listener.m_latency.seqnum)
			{
				cout << "ok ";
			}
			us=(listener.m_t2.tv_sec-listener.m_t1.tv_sec)*1000000+listener.m_t2.tv_usec-listener.m_t1.tv_usec;
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
			listener.m_pub->removeMinSeqChange();
			listener.m_sub->takeNextData((void*)&listener.m_latency,&info);
		}
		cout << "Mean: " << total/samples << endl;
				cout << "Min us: " << min_us << endl;
		break;
	}
	case 2:
	{
		LatencyListener2 listener;
		PublisherAttributes Wparam;
		Wparam.historyMaxSize = 1100;
		Wparam.topic.topicDataType = "LatencyType";
		Wparam.topic.topicKind = NO_KEY;
		listener.m_pub = DomainParticipant::createPublisher(p,Wparam);
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10469;
		loc.set_IP4_address(192,168,1,IPTEST1);
		listener.m_pub->addReaderLocator(loc,false);
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 1100;
		Rparam.topic.topicDataType = std::string("LatencyType");
		loc.set_IP4_address(0,0,0,0);
		Rparam.unicastLocatorList.push_back(loc);
		listener.m_sub = DomainParticipant::createSubscriber(p,Rparam);
		listener.m_sub->assignListener((SubscriberListener*)&listener);
		cout << "Waiting for completion "<<endl;
		int n;
		cin >> n;

		break;
	}
	}

	DomainParticipant::stopAll();


	return 0;
}



