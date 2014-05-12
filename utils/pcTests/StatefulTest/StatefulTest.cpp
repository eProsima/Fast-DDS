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


#include "eprosimartps/dds/DomainParticipant.h"

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

#if defined(_WIN32)
#pragma warning(disable: 4430)
void my_sleep(int seconds)
{
	return Sleep(seconds*(long)1000);
}};
#else
void my_sleep(int seconds)
{
	sleep(seconds);
};
#endif

typedef struct TestType{
	char name[6]; //KEY
	int32_t value;
	double price;
	TestType()
	{
		value = -1;
		price = 0;
		COPYSTR(name,"UNDEF");
	}
	void print()
	{
		cout << "Name: ";
		printf("%s",name);
		cout << " |Value: "<< value;
		cout << " |Price: "<< price;
		cout << endl;
	}
}TestType;

class TestTypeDataType:public DDSTopicDataType
{
public:
	TestTypeDataType()
{
		m_topicDataTypeName = "TestType";
		m_typeSize = 6+4+sizeof(double);
		m_isGetKeyDefined = true;
};
	~TestTypeDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
};

//Funciones de serializacion y deserializacion para el ejemplo
bool TestTypeDataType::serialize(void*data,SerializedPayload_t* payload)
{
	payload->length = sizeof(TestType);
	payload->encapsulation = CDR_LE;
	if(payload->data !=NULL)
		free(payload->data);
	payload->data = (octet*)malloc(payload->length);
	memcpy(payload->data,data,payload->length);
	return true;
}

bool TestTypeDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	//cout << "Deserializando length: " << payload->length << endl;
	memcpy(data,payload->data,payload->length);
	return true;
}

bool TestTypeDataType::getKey(void*data,InstanceHandle_t* handle)
{
	TestType* tp = (TestType*)data;
	handle->value[0]  = 0;
	handle->value[1]  = 0;
	handle->value[2]  = 0;
	handle->value[3]  = 5; //length of string in CDR BE
	handle->value[4]  = tp->name[0];
	handle->value[5]  = tp->name[1];
	handle->value[6]  = tp->name[2];
	handle->value[7]  = tp->name[3];
	handle->value[8]  = tp->name[4];
	for(uint8_t i=9;i<16;i++)
		handle->value[i]  = 0;
	return true;
}

class TestTypeListener: public SubscriberListener{
public:
	TestTypeListener(){};
	~TestTypeListener(){};
	void onNewDataMessage()
	{
		cout <<"New Message"<<endl;
	}
};



int main(int argc, char** argv)
{
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_DEBUGINFO_VERBOSITY_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type;
	if(argc > 1)
	{
		RTPSLog::Info << "Parsing arguments: " << argv[1] << endl;
		RTPSLog::printInfo();
		if(strcmp(argv[1],"1")==0)
			type = 1;
		else if(strcmp(argv[1],"2")==0)
			type = 2;
		else if(strcmp(argv[1],"3")==0)
			type = 3;
	}
	else
		type = WR;

	TestTypeDataType TestTypeData;
	DomainParticipant::registerType((DDSTopicDataType*)&TestTypeData);


	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = false;
	Participant* p = DomainParticipant::createParticipant(PParam);


	switch(type)
	{
	case 1:
	{
		PublisherAttributes Wparam;
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = std::string("TestType");
		Wparam.topic.topicName = std::string("Test_topic");
		Wparam.historyMaxSize = 14;
		Wparam.reliability.heartbeatPeriod.seconds = 2;
		Wparam.reliability.nackResponseDelay.seconds = 5;
		Wparam.reliability.reliabilityKind = RELIABLE;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10046;
		Wparam.unicastLocatorList.push_back(loc);
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
		if(pub == NULL)
			return 0;
		//Reader Proxy
		loc.set_IP4_address(192,168,1,IPTEST2);
		GUID_t readerGUID;
		readerGUID.entityId = ENTITYID_UNKNOWN;
		pub->addReaderProxy(loc,readerGUID,true);
		TestType tp;
		COPYSTR(tp.name,"Obje1");
		tp.value = 0;
		tp.price = 1.3;
		int n;
		cout << "Enter number to start: ";
		cin >> n;
		for(uint8_t i = 0;i<10;i++)
		{
			tp.value++;
			tp.price *= (i+1);
			if(i == 2 || i==4||i==5)
				p->loose_next_change();
			pub->write((void*)&tp);
			cout << "Going to sleep "<< (int)i <<endl;
			my_sleep(1);
			cout << "Wakes "<<endl;
		}
		pub->dispose((void*)&tp);
		my_sleep(1);
		cout << "Wakes "<<endl;
		pub->unregister((void*)&tp);
		my_sleep(1);
		cout << "Wakes "<<endl;
		break;
	}
	case 2:
	case 3:
	{
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 15;
		Rparam.topic.topicDataType = std::string("TestType");
		Rparam.topic.topicName = std::string("Test_Topic");
		Rparam.reliability.reliabilityKind = RELIABLE;
		Locator_t loc;
		if(type == 2)
			loc.port = 10046;
		else if(type ==3)
			loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
		TestTypeListener listener;
		sub->assignListener((SubscriberListener*)&listener);
		while(1)
		{
			cout << "Waiting for new message "<<endl;
			sub->waitForUnreadMessage();
			TestType tp;
			SampleInfo_t info;
			if(sub->takeNextData((void*)&tp,&info))
				tp.print();
			if(sub->getHistory_n() >= 0.5*Rparam.historyMaxSize)
			{
				cout << "Taking all" <<endl;
				while(sub->takeNextData((void*)&tp,&info))
					tp.print();
			}
		}
		break;
	}

	}

	cout << "Enter numer "<< endl;
		int n;
		cin >> n;
		DomainParticipant::stopAll();


	return 0;

}



