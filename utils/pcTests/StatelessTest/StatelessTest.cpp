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

#include "eprosimartps/Participant.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/common/colors.h"
#include "eprosimartps/dds/ParameterList.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/reader/RTPSListener.h"



using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;

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


typedef struct TestType{
	char name[6]; //KEY
	int32_t value;
	double price;
	TestType()
	{
		value = -1;
		price = 0;
		strcpy(name,"UNDEF");
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

class TestTypeListener: public RTPSListener{
public:
	TestTypeListener(){};
	~TestTypeListener(){};
	void newMessageCallback()
	{
		cout <<"New Message"<<endl;
	}
};


int main(int argc, char** argv)
{
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_INFO_VERBOSITY_LEVEL);
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

	ParticipantParams_t PParam;
	PParam.defaultSendPort = 10042;
	PParam.m_useSimpleParticipantDiscovery = false;
	Participant* p = DomainParticipant::createParticipant(PParam);

	switch(type)
	{
	case 1:
	{
		WriterParams_t WParam;
		WParam.historySize = 20;
		WParam.topicKind = WITH_KEY;
		WParam.topicDataType = "TestType";
		WParam.topicName = "Test_topic";
		Publisher* pub1 = DomainParticipant::createPublisher(p,WParam);
		Publisher* pub2 = DomainParticipant::createPublisher(p,WParam);
		ReaderParams_t Rparam;
		Rparam.historySize = 50;
		Rparam.topicDataType = std::string("TestType");
		Rparam.topicName = std::string("Test_topic");
		Rparam.topicKind = NO_KEY;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10469;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10469 port
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);

		loc.set_IP4_address(192,168,1,IPTEST2);
		pub1->addReaderLocator(loc,true);
		pub2->addReaderLocator(loc,true);
		TestType tp1,tp2,tp_in;
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
			if(pub1->getHistory_n() >= 0.8*WParam.historySize)
				pub1->removeMinSeqChange();
			if(pub2->getHistory_n() >= 0.8*WParam.historySize)
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
			while(sub->readMinSeqUnreadCache((void*)&tp_in))
			{
				tp_in.print();
			}
			if(sub->getHistory_n() >= 0.8*Rparam.historySize)
			{
				cout << "Taking all from subscriber" <<endl;
				std::vector<void*> data_vec;
				sub->takeAllCache(&data_vec);
				for(unsigned int i=0;i<data_vec.size();i++)
					((TestType*)data_vec[i])->print();
				cout << "Subscriber History has now: " << sub->getHistory_n() << " elements "<<endl;
			}
		}
		break;
	}
	case 2:
	{
		ReaderParams_t Rparam;
		Rparam.historySize = 50;
		Rparam.topicDataType = std::string("TestType");
		Rparam.topicName = std::string("Test_topic");
		Rparam.topicKind = WITH_KEY;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10469;
		Rparam.unicastLocatorList.push_back(loc); //Listen in port 10469
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
		TestTypeListener listener;
		sub->assignListener((RTPSListener*)&listener);
		WriterParams_t WParam;
		WParam.historySize = 50;
		WParam.topicKind = NO_KEY;
		WParam.topicDataType = "TestType";
		WParam.topicName = "Test_topic";
		Publisher* pub1 = DomainParticipant::createPublisher(p,WParam);
		loc.set_IP4_address(192,168,1,IPTEST1);
		pub1->addReaderLocator(loc,false);
		while(1)
		{
			cout << "Blocking until new message arrives " << endl;
			sub->blockUntilNewMessage();
			cout << "After new message block " << sub->getHistory_n() << endl;
			TestType tp;
			while(sub->readMinSeqUnreadCache((void*)&tp))
			{
				tp.print();
				pub1->write((void*)&tp);
			}
			cout << "Read: " << sub->getReadElements_n() <<" from History: "<<sub->getHistory_n()<< endl;
			if(sub->getHistory_n() >= 0.5*Rparam.historySize)
			{
				cout << "Taking all" <<endl;
				std::vector<void*> data_vec;
				sub->takeAllCache(&data_vec);
				for(unsigned int i=0;i<data_vec.size();i++)
					((TestType*)data_vec[i])->print();
				cout << "History has now: " << sub->getHistory_n() << " elements ";
				cout << " and is FUll?: " << sub->isHistoryFull() << endl;
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
	DomainParticipant::stopAll();



	return 0;

}



