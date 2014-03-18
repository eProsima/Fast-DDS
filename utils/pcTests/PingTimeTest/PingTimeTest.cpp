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
#include "eprosimartps/DomainParticipant.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/Publisher.h"
#include "eprosimartps/Subscriber.h"
#include "eprosimartps/common/colors.h"
#include "eprosimartps/ParameterListCreator.h"
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


//Funciones de serializacion y deserializacion para el ejemplo
void TestTypeSer(SerializedPayload_t* payload, void* data )
{
	payload->length = sizeof(TestType);
	payload->encapsulation = CDR_LE;
	if(payload->data !=NULL)
		free(payload->data);
	payload->data = (octet*)malloc(payload->length);
	memcpy(payload->data,data,payload->length);
}

void TestTypeDeser(SerializedPayload_t* payload, void* data )
{
	//cout << "Deserializando length: " << payload->length << endl;
	memcpy(data,payload->data,payload->length);
}

void TestTypeGetKey(void* data,InstanceHandle_t* handle )
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
}


void newMsgCallback()
{
	cout << MAGENTA"New Message Callback" <<DEF<< endl;
}



int main(int argc, char** argv){
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_DEBUGINFO_VERBOSITY_LEVEL);
	int type;
	if(argc > 1)
	{
		RTPSLog::Info << "Parsing arguments: " << argv[1] << endl;
		RTPSLog::printInfo();
		if(strcmp(argv[1],"writer")==0)
			type = 1;
		if(strcmp(argv[1],"reader")==0)
			type = 2;
	}
	else
		type = WR;

	//my_sleep(1);
	ParticipantParams_t PParam;
	PParam.defaultSendPort = 14456;
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	DomainParticipant::registerType(std::string("TestType"),&TestTypeSer,&TestTypeDeser,&TestTypeGetKey,sizeof(TestType));
	Locator_t loc;
	loc.kind = 1;
	loc.port = 10469;
	loc.set_IP4_address(192,168,1,16);

	//Create both publisher and subscriber.
	WriterParams_t Wparam;
	Wparam.historySize = 10;
	Wparam.pushMode = true;
	Wparam.stateKind = STATELESS;
	Wparam.topicKind = WITH_KEY;
	Wparam.topicDataType = std::string("TestType");
	Wparam.topicName = std::string("This is a test topic");
	Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
	//One of this two as locators.
	//loc.set_IP4_address(192,168,1,18);
	//pub->addReaderLocator(loc,true);
	loc.set_IP4_address(192,168,1,21);
	pub->addReaderLocator(loc,true);

	ReaderParams_t Rparam;
	Rparam.historySize = 5;
	Rparam.stateKind = STATELESS;
	Rparam.topicDataType = std::string("TestType");
	Rparam.topicName = std::string("This is a test topic");
	Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
	Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
	TestType tp;
	COPYSTR(tp.name,"Obje1");
	tp.value = 0;
	tp.price = 1.3;
	boost::posix_time::ptime t1(boost::posix_time::microsec_clock::local_time());
	pub->write((void*)&tp);
	sub->blockUntilNewMessage();
	sub->readMinSeqUnreadCache((void*)&tp);
	boost::posix_time::ptime t2(boost::posix_time::microsec_clock::local_time());
	pub->write((void*)&tp);
	sub->blockUntilNewMessage();
	sub->readMinSeqUnreadCache((void*)&tp);
	boost::posix_time::ptime t3(boost::posix_time::microsec_clock::local_time());
	cout<< "TIMES: " << endl;
	cout << (t2-t1).total_microseconds();
	cout << (t3-t1).total_microseconds();

	cout << "Enter numer "<< endl;
	int n;
	cin >> n;

	//my_sleep(3);
//
//
//
//	Participant p;
//	//CHECK PARTICIPANT
//	checkParticipant(&p);
//	//cout << "After participant creation, threadListensize: " << p.threadListenList.size() << endl;
//	WriterParams_t Wparam;
//	Wparam.historySize = 2;
//	Wparam.pushMode = true;
//	Duration_t dur;
//	dur.seconds = 1;
//	dur.fraction = 0;
//	Wparam.resendDataPeriod = dur;
//	Wparam.heartbeatPeriod = dur;
//	Wparam.nackResponseDelay = dur;
//	Wparam.nackSupressionDuration = dur;
//
//	Locator_t loc;
//	loc.kind = LOCATOR_KIND_UDPv4;
//	loc.port = 14244;
//	loc.set_IP4_address(127,0,0,1);
//	Wparam.unicastLocatorList.push_back(loc);
//	StatelessWriter* SW2 = new StatelessWriter();
//	p.createStatelessWriter(SW2,Wparam);
////	StatelessWriter* SW3 = new StatelessWriter();
////	p.createStatelessWriter(SW3,Wparam);
//
//	//Add a StatelessReader
//	ReaderParams_t Rparam;
//	Rparam.historySize = 2;
//
//	StatelessReader* SR1 = new StatelessReader();
//	p.createStatelessReader(SR1,Rparam);
//	cout << "Stateless Reader created correctly" << endl;
//
//
//	sleep(1);
//	//Add a ReaderLocator
//	ReaderLocator RL2;
//	RL2.expectsInlineQos = false;
//	RL2.locator.kind = 1;
//	RL2.locator.port = 14244;
//	RL2.locator.set_IP4_address(127,0,0,1);
//	cout << "IP ADDRESS: " <<
//	SW2->reader_locator_add(RL2);
//	ReaderLocator RL3;
//	RL3.expectsInlineQos = false;
//	RL3.locator.kind = 1;
//	RL3.locator.port = 14244;
//	RL3.locator.set_IP4_address(192,168,1,18);
//	SW2->reader_locator_add(RL3);
//	cout << "Reader LocatorS added " << endl;
//	int numbers[4] = {1,2,3,4};
//	SerializedPayload_t data;
//	data.length = 4*sizeof(int);
//	if(data.data !=NULL)
//		free(data.data);
//	data.data = (octet*)malloc(4*sizeof(int));
//	memcpy(data.data,(octet*)numbers,4*sizeof(int));
//	cout << "Before creating change" << endl;
//	//Create changes
//	CacheChange_t C21;
//	SW2->new_change(ALIVE,&data,(void*)numbers,&C21);
//	SW2->writer_cache.add_change(C21);
//
//
//	sleep(5);

//
//
//	//Testing serialize
//		cout << "Testing serial/deserial " << endl;
//		SerializedPayload_t p;
//		TipoPrueba_str tp2;
//		strcpy(tp2.name,"TestStr");
//		tp2.valor = 1;
//		TipoPruebaSerialize(&p,(void*)&tp2);
//		cout << "data in main: ";
//		for(int i=0;i<p.length;i++)
//		{
//			cout <<(int)((octet*)&tp2)[i]<< ".";
//		}
//		cout << endl;
//		cout << "payload in main: ";
//		for(int i=0;i<p.length;i++)
//		{
//			cout <<(int)p.data[i]<< ".";
//		}
//		cout << endl;
//		TipoPrueba_str tp3;
//		TipoPruebaDeserialize(&p,(void*)&tp3);
//		cout << "data in main: ";
//		for(int i=0;i<p.length;i++)
//		{
//			cout <<(int)((octet*)&tp3)[i]<< ".";
//		}
//		cout << endl;
//		cout << "data print: ";
//		tp3.print();
//
//




	return 0;

}



