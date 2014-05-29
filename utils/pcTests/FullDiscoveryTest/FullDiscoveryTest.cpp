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
//#include "eprosimartps/dds/DomainParticipant.h"
//#include "eprosimartps/Participant.h"
//#include "eprosimartps/dds/Publisher.h"
//#include "eprosimartps/dds/Subscriber.h"
//#include "eprosimartps/dds/attributes/all_attributes.h"
//#include "eprosimartps/qos/ParameterList.h"
//#include "eprosimartps/qos/DDSQosPolicies.h"
//#include "eprosimartps/utils/RTPSLog.h"
//#include "eprosimartps/dds/DDSTopicDataType.h"
//
//#include "eprosimartps/dds/PublisherListener.h"
//#include "eprosimartps/dds/SubscriberListener.h"
//
//#include "eprosimartps/dds/SampleInfo.h"

#include "eprosimartps/rtps_all.h"

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

class MyPubListener: public PublisherListener
{
	void onPublicationMatched()
	{
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
		cout << B_RED << "PUBLICATION MATCHED"<<DEF<<endl;
	}
};

class MySubListener: public SubscriberListener
{
	void onSubscriptionMatched()
	{
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
		cout << B_RED << "SUBSCRIPTION MATCHED"<<DEF<<endl;
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
		if(strcmp(argv[1],"1")==0)
			type = 1;
		if(strcmp(argv[1],"2")==0)
			type = 2;
	}
	else
		type = WR;

	pDebugInfo("Stargin"<<endl);

	TestTypeDataType TestTypeData;
	DomainParticipant::registerType((DDSTopicDataType*)&TestTypeData);
//	LocatorList_t myIP;
//	DomainParticipant::getIPAddress(&myIP);
//	cout << "My IP: " << myIP.size() << ": " << myIP.begin()->printIP4Port()<< endl;
	//***********  PARTICIPANT  ******************//
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.discovery.resendDiscoveryParticipantDataPeriod.seconds = 30;

	PParam.discovery.domainId = 0;
	cout << "a"<<endl;
	switch(type)
	{
	case 1:
	{
		PParam.discovery.m_simpleEDP.use_Publication_Writer = true;
			PParam.discovery.m_simpleEDP.use_Subscription_Reader = true;
			PParam.discovery.m_simpleEDP.use_Publication_Reader = false;
			PParam.discovery.m_simpleEDP.use_Subscription_Writer = false;
		PParam.name = "participant1";
		Participant* p = DomainParticipant::createParticipant(PParam);


		PublisherAttributes Wparam;
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = std::string("TestType");
		Wparam.topic.topicName = std::string("Test_topic1");
		Wparam.historyMaxSize = 14;
		Wparam.times.heartbeatPeriod.seconds = 2;
		Wparam.times.nackResponseDelay.seconds = 5;
		Wparam.qos.m_reliability.kind = eprosima::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

		cout << "Reliable: "<< eprosima::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS << endl;
		cout << "BE: "<< eprosima::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS << endl;
		Wparam.userDefinedId = 1;
		cout << "MY DURABILITY: "<< Wparam.qos.m_durability.kind << endl;
		cout << "MY DURABILITY CHANGES: "<< eprosima::dds::DurabilityQosPolicyKind_t::TRANSIENT_LOCAL_DURABILITY_QOS << endl;
		Wparam.qos.m_durability.kind = eprosima::dds::DurabilityQosPolicyKind_t::PERSISTENT_DURABILITY_QOS;
		Wparam.qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
		cout << "MY DURABILITY: "<< Wparam.qos.m_durability.kind << endl;
		MyPubListener mylisten;
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam,&mylisten);


//		SubscriberAttributes Rparam;
//		Rparam.historyMaxSize = 50;
//		Rparam.topic.topicDataType = std::string("TestType");
//		Rparam.topic.topicName = std::string("Test_topic2");
//		Rparam.topic.topicKind = NO_KEY;
//		Locator_t loc;
//		loc = *myIP.begin();
//		loc.kind = 1;
//		loc.port = 10046;
//		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10469 port
//		Rparam.userDefinedId = 2;
//		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);

	//	p->announceParticipantState();
		cout << "Sleeping 4 seconds"<<endl;
		my_sleep(4);
		TestType tp1,tp_in;
		SampleInfo_t info_in;
		COPYSTR(tp1.name,"Obje1");
		tp1.value = 0;
		tp1.price = 1.3;
		int n;
		cout << "Enter number to start: "<<endl;
		cin >> n;
		for(uint i = 0;i<10;i++)
		{
			tp1.value++;
			tp1.price *= (i+1);

			pub->write((void*)&tp1);
//			sub->waitForUnreadMessage();
//			sub->readNextData((void*)&tp_in,&info_in);
//			if(tp_in.value == tp1.value &&
//					tp_in.price ==tp1.price)
//				cout << "Message RECEIVED = AS SEND: "<< i << endl;
		}
		break;
	}
	case 2:
	{
		PParam.discovery.m_simpleEDP.use_Publication_Writer = false;
		PParam.discovery.m_simpleEDP.use_Subscription_Reader = false;
		PParam.discovery.m_simpleEDP.use_Publication_Reader = true;
		PParam.discovery.m_simpleEDP.use_Subscription_Writer = true;
		//***********  PARTICIPANT  ******************//
		PParam.name = "participant2";
		Participant* p = DomainParticipant::createParticipant(PParam);
		SubscriberAttributes Rparam;
		Rparam.userDefinedId = 17;
		Rparam.historyMaxSize = 50;
		Rparam.topic.topicDataType = std::string("TestType");
		Rparam.topic.topicName = std::string("Test_topic1");
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10046 port
		loc.port = 10047;
		Rparam.unicastLocatorList.push_back(loc);
		MySubListener mylisten;
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam,&mylisten);

//		PublisherAttributes Wparam;
//		Wparam.userDefinedId = 18;
//		Wparam.topic.topicKind = NO_KEY;
//		Wparam.reliability.reliabilityKind = BEST_EFFORT;
//		Wparam.topic.topicDataType = std::string("TestType");
//		Wparam.topic.topicName = std::string("Test_topic2");
//		Wparam.historyMaxSize = 14;
//		Wparam.reliability.heartbeatPeriod.seconds = 2;
//		Wparam.reliability.nackResponseDelay.seconds = 5;
//
//
//		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);

		p->announceParticipantState();

	//	my_sleep(4);
		TestType tp_in;
		SampleInfo_t info_in;
		for(uint i = 0;i<10;i++)
		{
			sub->waitForUnreadMessage();
			sub->readNextData((void*)&tp_in,&info_in);
			tp_in.print();
		//	pub->write((void*)&tp_in);
		}

		break;
	}
	}

	sleep(2);

	DomainParticipant::stopAll();


	return 0;

}



