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
	DomainParticipant::registerType((DDSTopicDataType*)&TestTypeData);

	switch(type)
	{
	case 1:
	{
		//***********  PARTICIPANT  ******************//
		ParticipantParams_t PParam;
		PParam.name = "participant1";
		PParam.defaultSendPort = 10042;
		PParam.m_useStaticEndpointDiscovery = true;
		PParam.resendSPDPDataPeriod_sec = 30;
		Participant* p = DomainParticipant::createParticipant(PParam);
		WriterParams_t Wparam;
		Wparam.stateKind = STATELESS;
		Wparam.topicKind = WITH_KEY;
		Wparam.topicDataType = std::string("TestType");
		Wparam.topicName = std::string("Test_topic");
		Wparam.historySize = 14;
		Wparam.reliablility.heartbeatPeriod.seconds = 2;
		Wparam.reliablility.nackResponseDelay.seconds = 5;
		Wparam.reliablility.kind = BEST_EFFORT;
		Wparam.userDefinedId = 1;
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
		ReaderParams_t Rparam;
		Rparam.historySize = 50;
		Rparam.topicDataType = std::string("TestType");
		Rparam.topicName = std::string("Test_topic");
		Rparam.topicKind = NO_KEY;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10469 port
		Rparam.userDefinedId = 2;
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
		break;
	}
	case 2:
	{
		//***********  PARTICIPANT  ******************//
		ParticipantParams_t PParam;
		PParam.name = "participant2";
		PParam.defaultSendPort = 10042;
		PParam.m_useStaticEndpointDiscovery = true;
		PParam.resendSPDPDataPeriod_sec = 30;
		Participant* p = DomainParticipant::createParticipant(PParam);
		ReaderParams_t Rparam;
		Rparam.userDefinedId = 17;
		Rparam.historySize = 50;
		Rparam.topicDataType = std::string("TestType");
		Rparam.topicName = std::string("Test_topic");
		Rparam.topicKind = WITH_KEY;
		Rparam.stateKind == STATELESS;
		Locator_t loc;
		loc.kind = 1;
		loc.port = 10046;
		Rparam.unicastLocatorList.push_back(loc); //Listen in the 10046 port

		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);

		WriterParams_t Wparam;
		Wparam.userDefinedId = 18;
		Wparam.stateKind = STATELESS;
		Wparam.topicKind = NO_KEY;
		Wparam.reliablility.kind = BEST_EFFORT;
		Wparam.topicDataType = std::string("TestType");
		Wparam.topicName = std::string("Test_topic");
		Wparam.historySize = 14;
		Wparam.reliablility.heartbeatPeriod.seconds = 2;
		Wparam.reliablility.nackResponseDelay.seconds = 5;


		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);

		break;
	}
	}


	int a;
	cin >> a;


	return 0;

}



