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

//#include "eprosimartps/dds/DomainParticipant.h"
//#include "eprosimartps/Participant.h"
//
//#include "eprosimartps/qos/ParameterList.h"
//#include "eprosimartps/utils/RTPSLog.h"




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
	//cout << "SERIALIZES: "<<sizeof(TestType)<<endl;
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

boost::interprocess::interprocess_semaphore sema(0);


class MyPubListener:public PublisherListener
{
	void onPublicationMatched()
	{
		cout << "PUBLICATION MATCHED"<<endl;
		sema.post();
	}
};

class MySubListener:public SubscriberListener
{
	void onSubscriptionMatched()
	{
		cout << "SUBSCRIPTION MATCHED "<<endl;
		sema.post();
	}
	void onNewDataMessage()
		{
			cout <<"New Message"<<endl;
		}
};


int main(int argc, char** argv)
{
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
		else
			type = 3;
	}
	else
		type = 1; //publisher

	TestTypeDataType TestTypeData;
	DomainParticipant::registerType((DDSTopicDataType*)&TestTypeData);


	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;



	switch(type)
	{
	case 1:
	{
		PParam.name = "participant1";
		//In this side we only have a Publisher so we don't need all discovery endpoints
		PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
		PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = false;
		Participant* p = DomainParticipant::createParticipant(PParam);
		PublisherAttributes Wparam;
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "TestType";
		Wparam.topic.topicName = "Test_Topic";
		Wparam.historyMaxSize = 14;
		Wparam.times.heartbeatPeriod.seconds = 2;
		Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		MyPubListener mylisten;
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam,(PublisherListener*)&mylisten);
		if(pub == NULL)
			return 0;
		cout << "Waiting for discovery"<<endl;
		sema.wait();
		p->stopParticipantAnnouncement();
		TestType tp;
		COPYSTR(tp.name,"Obje1");
		tp.value = 0;
		tp.price = 1.3;
		int n;
		cout << "Enter number to start: ";
		cin >> n;
		for(uint8_t i = 1;i<=10;i++)
		{
			tp.value++;
			tp.price *= (i);
			if(i == 3 || i==5 ||i ==6)
			{
				//THIS METHOD SOULD BE USED WITH GREAT CARE. IT DOES NOT CHECK WHO IS SENDING THE NEXT PACKET
				//DEPENDING IN THE TIMER PERIODS IT CAN PREVENT HB or ACKNACK packets from being sent
				p->loose_next_change();
			}
			pub->write((void*)&tp);
			cout << "Going to sleep "<< (int)i <<endl;
			eClock::my_sleep(1000);
			cout << "Wakes "<<endl;
		}
		pub->dispose((void*)&tp);
		eClock::my_sleep(1000);
		cout << "Wakes "<<endl;
		pub->unregister((void*)&tp);
		eClock::my_sleep(1000);
		cout << "Wakes "<<endl;
		break;
	}
	case 2:
	{
		PParam.name = "participant2";
		//In this side we only have a subscriber so we dont need all discovery endpoints
		PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = false;
		PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
		Participant* p = DomainParticipant::createParticipant(PParam);
		SubscriberAttributes Rparam;
		Rparam.historyMaxSize = 30;
		Rparam.topic.topicDataType = "TestType";
		Rparam.topic.topicName = "Test_Topic";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.times.heartbeatResponseDelay.fraction = 200*1000*1000;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		MySubListener mylisten;
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam,(SubscriberListener*)&mylisten);
		cout << "Waiting for discovery"<<endl;
		sema.wait();
		p->stopParticipantAnnouncement(); //Only for tests to see more clearly the communication
		int i = 0;
		while(i<20)
		{
			cout << "Waiting for new message "<<endl;
			sub->waitForUnreadMessage();
			TestType tp;
			SampleInfo_t info;
			if(sub->readNextData((void*)&tp,&info))
				tp.print();
			if(sub->getHistoryElementsNumber() >= 0.5*Rparam.historyMaxSize)
			{
				cout << "Taking all" <<endl;
				while(sub->takeNextData((void*)&tp,&info))
					tp.print();
			}
			i++;
		}
		break;
	}

	}

	cout << "Enter numer to stop "<< endl;
	int n;
	cin >> n;
	DomainParticipant::stopAll();


	return 0;

}



