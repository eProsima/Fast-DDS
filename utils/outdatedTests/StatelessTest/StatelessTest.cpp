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

#include "fastrtps/rtps_all.h"

#include "fastrtps/dds/DomainRTPSParticipant.h"

#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/dds/DDSTopicDataType.h"
#include "fastrtps/dds/SubscriberListener.h"
#include "fastrtps/dds/PublisherListener.h"



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

#pragma warning(disable: 4430)

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
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	int type;
	if(argc > 1)
	{
//		RTPSLog::Info << "Parsing arguments: " << argv[1] << endl;
//		RTPSLog::printInfo();
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
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&TestTypeData);

	RTPSParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
	RTPSParticipant* p = DomainRTPSParticipant::createRTPSParticipant(PParam);

	switch(type)
	{
	case 1:
	{
		PublisherAttributes PParam;
		PParam.historyMaxSize = 20;
		PParam.topic.topicKind = WITH_KEY;
		PParam.topic.topicDataType = "TestType";
		PParam.topic.topicName = "Test_topic";
		Publisher* pub1 = DomainRTPSParticipant::createPublisher(p,PParam);
		Publisher* pub2 = DomainRTPSParticipant::createPublisher(p,PParam);
		SubscriberAttributes Sparam;
		Sparam.historyMaxSize = 50;
		Sparam.topic.topicDataType = std::string("TestType");
		Sparam.topic.topicName = std::string("Test_topic");
		Sparam.topic.topicKind = NO_KEY;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10467;
		Sparam.unicastLocatorList.push_back(loc); //Listen in the 10469 port
		Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Sparam);

		loc.set_IP4_address(127,0,0,1);
		loc.port = 10466;
		pub1->addReaderLocator(loc,true);
		pub2->addReaderLocator(loc,true);
		TestType tp1,tp2,tp_in;
		SampleInfo_t info;
		COPYSTR(tp1.name,"Obje1");
		COPYSTR(tp2.name,"Obje2");
		tp1.value = 0;
		tp1.price = 1.3;
		tp2.value = 0;
		tp2.price = -1.3;
		int n;
		cout << "Enter number to start: ";
		cin >> n;
		for(uint i = 0;i<10;i++)
		{
			tp1.value++;
			tp1.price *= (i+1);
			tp2.value++;
			tp2.price *= (i+1);
			pub1->write((void*)&tp1);
			pub2->write((void*)&tp2);
			if(pub1->getHistoryElementsNumber() >= 0.8*Sparam.historyMaxSize)
				pub1->removeMinSeqChange();
			if(pub2->getHistoryElementsNumber() >= 0.8*Sparam.historyMaxSize)
				pub2->removeMinSeqChange();
			if(i==8)
			{
				pub1->dispose((void*)&tp1);
				pub1->unregister((void*)&tp1);
				pub2->dispose((void*)&tp2);
				pub2->unregister((void*)&tp2);
				COPYSTR(tp1.name,"Obje3");
				tp1.value = 0;
				tp1.price = 1.5;
				COPYSTR(tp2.name,"Obje4");
				tp2.value = 0;
				tp2.price = 1.5;
			}
			if(sub->getHistoryElementsNumber() >= 1)
			{
				cout << "Taking from subscriber" <<endl;
				if(sub->readNextData((void*)&tp_in,&info))
					tp_in.print();
				cout << "Subscriber History has now: " << sub->getHistoryElementsNumber() << " elements "<<endl;
			}
		}
		cout << "Sleeping 3 seconds"<<endl;
		my_sleep(3);
		cout << "Slept for 3 seconds"<< endl;
		while(sub->takeNextData((void*)&tp_in,&info))
		{
			if(info.sampleKind == ALIVE)
				tp_in.print();
			else
				cout << "NOT ALIVE SAMPLE"<< endl;
		}
		break;
	}
	case 2:
	{
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 50;
		Rparam.topic.topicDataType = std::string("TestType");
		Rparam.topic.topicName = std::string("Test_topic");
		Rparam.topic.topicKind = WITH_KEY;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10469;
		Rparam.unicastLocatorList.push_back(loc); //Listen in port 10469
		Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam);
		TestTypeListener listener;
		sub->assignListener((SubscriberListener*)&listener);
		PublisherAttributes WParam;
		WParam.historyMaxSize = 50;
		WParam.topic.topicKind = NO_KEY;
		WParam.topic.topicDataType = "TestType";
		WParam.topic.topicName = "Test_topic";
		Publisher* pub1 = DomainRTPSParticipant::createPublisher(p,WParam);
		loc.set_IP4_address(192,168,1,IPTEST0);
		pub1->addReaderLocator(loc,false);
		while(1)
		{
			cout << "Blocking until new message arrives " << endl;
			sub->waitForUnreadMessage();
			//cout << "After new message block " << sub->getHistory_n() << endl;
			TestType tp;
			SampleInfo_t info;
			while(sub->readNextData((void*)&tp,&info))
			{
				tp.print();
				pub1->write((void*)&tp);
				cout << "Write OK"<< endl;
				tp.value = -1;
				tp.price = 0;
				COPYSTR(tp.name,"UNDEF");
			}
			if(sub->getHistoryElementsNumber() >= 0.5*Rparam.historyMaxSize)
			{
				cout << "Taking all" <<endl;
				while(sub->takeNextData((void*)&tp,&info))
					tp.print();
			}
		}
		break;
	}
	default:
		break;
	}


	cout << "Enter numer "<< endl;
	int n;
	cin >> n;
	DomainRTPSParticipant::stopAll();



	return 0;

}



