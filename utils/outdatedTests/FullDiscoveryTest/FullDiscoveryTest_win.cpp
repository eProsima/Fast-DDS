// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
#include "fastrtps/dds/DomainRTPSParticipant.h"
#include "fastrtps/RTPSParticipant.h"
#include "fastrtps/dds/Publisher.h"
#include "fastrtps/dds/Subscriber.h"
#include "fastrtps/common/colors.h"
#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/log/Log.h"


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

#if defined(_WIN32)
#pragma warning(disable: 4430)
void my_sleep(int seconds)
{
	return Sleep(seconds*(long)1000);
};
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



int main(int argc, char** argv){
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_LONGINFO_VERBOSITY_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type;
	if(argc > 1)
	{
		RTPSLog::Info << "Parsing arguments: " << argv[1] << endl;
		RTPSLog::printInfo();
		if(strcmp(argv[1],"1")==0)
			type = 1;
		if(strcmp(argv[1],"2")==0)
			type = 2;
	}
	else
		type = WR;


	TestTypeDataType TestTypeData;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&TestTypeData);
	LocatorList_t myIP;
	DomainRTPSParticipant::getIPAddress(&myIP);


	switch(type)
	{
	case 1:
	{
		//***********  RTPSParticipant  ******************//
		RTPSParticipantAttributes PParam;
		PParam.name = "RTPSParticipant1";
		PParam.defaultSendPort = 10042;
		PParam.discovery.use_STATIC_EndpointDiscoveryProtocol = true;
		PParam.discovery.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
		PParam.discovery.m_staticEndpointXMLFilename ="D:\\Trabajo\\workspace\\eRTPS\\utils\\pcTests\\StaticRTPSParticipantInfo.xml";
		PParam.discovery.resendSPDPDataPeriod_sec = 30;
		PParam.domainId = 50;
		RTPSParticipant* p = DomainRTPSParticipant::createRTPSParticipant(PParam);
		PublisherAttributes Wparam;
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = std::string("TestType");
		Wparam.topic.topicName = std::string("Test_topic1");
		Wparam.historyMaxSize = 14;
		Wparam.reliability.heartbeatPeriod.seconds = 2;
		Wparam.reliability.nackResponseDelay.seconds = 5;
		Wparam.reliability.reliabilityKind = BEST_EFFORT;
		Wparam.userDefinedId = 1;
		Publisher* pub = DomainRTPSParticipant::createPublisher(p,Wparam);
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 50;
		Rparam.topic.topicDataType = std::string("TestType");
		Rparam.topic.topicName = std::string("Test_topic2");
		Rparam.topic.topicKind = NO_KEY;
		Locator_t loc;
		loc = *myIP.begin();
		loc.kind = 1;
		loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10469 port
		Rparam.userDefinedId = 2;
		Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam);

		p->announceRTPSParticipantState();
		my_sleep(4);
		TestType tp1,tp_in;
		SampleInfo_t info_in;
		COPYSTR(tp1.name,"Obje1");
		tp1.value = 0;
		tp1.price = 1.3;
		int n;
		cout << "Enter number to start: ";
		cin >> n;
		for(uint i = 0;i<10;i++)
		{
			tp1.value++;
			tp1.price *= (i+1);

			pub->write((void*)&tp1);
			sub->waitForUnreadMessage();
			sub->readNextData((void*)&tp_in,&info_in);
			if(tp_in.value == tp1.value &&
					tp_in.price ==tp1.price)
				cout << "Message RECEIVED = AS SEND: "<< i << endl;
		}
		break;
	}
	case 2:
	{
		//***********  RTPSParticipant  ******************//
		RTPSParticipantAttributes PParam;
		PParam.name = "RTPSParticipant2";
		PParam.defaultSendPort = 10042;
		PParam.discovery.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
		PParam.discovery.use_STATIC_EndpointDiscoveryProtocol= true;
		PParam.discovery.m_staticEndpointXMLFilename = "D:\\Trabajo\\workspace\\eRTPS\\utils\\pcTests\\StaticRTPSParticipantInfo.xml";
		PParam.discovery.resendSPDPDataPeriod_sec = 30;
		PParam.domainId = 50;
		RTPSParticipant* p = DomainRTPSParticipant::createRTPSParticipant(PParam);
		SubscriberAttributes Rparam;
		Rparam.userDefinedId = 17;
		Rparam.historyMaxSize = 50;
		Rparam.topic.topicDataType = std::string("TestType");
		Rparam.topic.topicName = std::string("Test_topic1");
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.reliability.reliabilityKind = BEST_EFFORT;
		Locator_t loc;
		loc = *myIP.begin();
		loc.kind = 1;
		loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10046 port
		loc.port = 10047;
		Rparam.unicastLocatorList.push_back(loc);

		Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam);

		PublisherAttributes Wparam;
		Wparam.userDefinedId = 18;
		Wparam.topic.topicKind = NO_KEY;
		Wparam.reliability.reliabilityKind = BEST_EFFORT;
		Wparam.topic.topicDataType = std::string("TestType");
		Wparam.topic.topicName = std::string("Test_topic2");
		Wparam.historyMaxSize = 14;
		Wparam.reliability.heartbeatPeriod.seconds = 2;
		Wparam.reliability.nackResponseDelay.seconds = 5;


		Publisher* pub = DomainRTPSParticipant::createPublisher(p,Wparam);

		p->announceRTPSParticipantState();

		my_sleep(4);
		TestType tp_in;
		SampleInfo_t info_in;
		for(uint i = 0;i<10;i++)
		{
			sub->waitForUnreadMessage();
			sub->readNextData((void*)&tp_in,&info_in);
			tp_in.print();
			pub->write((void*)&tp_in);
		}

		break;
	}
	}

	my_sleep(3);

	DomainRTPSParticipant::stopAll();

	cout << "Finish" << endl;

	return 0;

}



