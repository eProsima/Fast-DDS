/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ParticipantTest.cpp
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

#define TESTSTRLENGTH 30

using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;

const bool print_detail_1 = true;
const bool print_detail_2 = true;

static inline void result_print(const char* p,bool result){
	if(print_detail_1)
	cout << left << setw(TESTSTRLENGTH) << p; result ? (cout << BLUE <<"YES" << DEF) : (cout << RED << "NO" << DEF);cout << endl;
}
static inline void result_print_part(const char* p,bool result){
	if(print_detail_2)
	cout << B_YELLOW << left << setw(TESTSTRLENGTH)  <<p << DEF; result ? (cout << B_BLUE <<"PASS" << DEF) : (cout << B_RED << "FAILED" << DEF);cout << endl;
}



#define WR 1 //Writer 1, Reader 2

#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

//void my_sleep(int seconds)
//{
//#if defined(_WIN32)
//	Sleep(seconds*(long)1000);
//#else
//	sleep(second);
//#endif
//}​;

#if defined(_WIN32)
	#define COPYSTR strcpy_s
#else
	#define COPYSTR strcpy
#endif

	
typedef struct TipoPrueba_str{
	int32_t valor;
	char name[10];
	void print()
	{
		cout << "Valor: "<< valor << " name: ";
		for(int i=0;i<10;i++)
		{
			cout << name[i] << ".";
		}
		cout << endl;
	}
}TipoPrueba_str;


//Funciones de serializacion y deserializacion para el ejemplo
void TipoPruebaSerialize(SerializedPayload_t* payload, void* data )
{
	payload->length = sizeof(TipoPrueba_str);
	payload->encapsulation = CDR_LE;
	if(payload->data !=NULL)
		free(payload->data);
	payload->data = (octet*)malloc(payload->length);
	memcpy(payload->data,data,payload->length);
}

void TipoPruebaDeserialize(SerializedPayload_t* payload, void* data )
{
	cout << "Deserializando length: " << payload->length << endl;
	memcpy(data,payload->data,payload->length);
}


void newMsgCallback()
{
	cout << MAGENTA"New Message Callback" <<DEF<< endl;
}



int main(int argc, char** argv){
	RTPSLog::setVerbosity(RTPSLog::EPROSIMA_INFO_VERBOSITY_LEVEL);
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
	Participant* p = DomainParticipant::createParticipant(PParam);
	//Registrar tipo de dato.
	DomainParticipant::registerType(std::string("TipoPrueba"),&TipoPruebaSerialize,&TipoPruebaDeserialize,sizeof(TipoPrueba_str));
	Locator_t loc;
	loc.kind = 1;
	loc.port = 10469;
	loc.set_IP4_address(192,168,1,16);
	if(type == 1) //writer
	{
		WriterParams_t Wparam;
		Wparam.historySize = 2;
		Wparam.pushMode = true;
		Wparam.stateKind = STATELESS;
		Wparam.topicDataType = std::string("TipoPrueba");
		Wparam.topicName = std::string("Topico de pruebaf");
		Publisher* pub = DomainParticipant::createPublisher(p,Wparam);
		//inline Qos parameters
		ParameterListCreator::addParameterLocator(&pub->ParamList,PID_UNICAST_LOCATOR,loc);
		ParameterListCreator::addParameterPort(&pub->ParamList,PID_METATRAFFIC_UNICAST_PORT,1203);
		ParameterListCreator::addParameterString(&pub->ParamList,PID_TOPIC_NAME,Wparam.topicName);
		ParameterListCreator::addParameterString(&pub->ParamList,PID_TYPE_NAME,Wparam.topicDataType);

	//	cout << "p"<< endl;
		//sleep(1);
		pub->addReaderLocator(loc,true);
		loc.set_IP4_address(192,168,1,18);
		pub->addReaderLocator(loc,true);
		loc.set_IP4_address(127,0,0,1);
		pub->addReaderLocator(loc,true);
		//Create new data and send it
		TipoPrueba_str tp;
		COPYSTR(tp.name,"TestStr");
		tp.valor = 1;
		pub->write((void*)&tp);
		//sleep(2);
		tp.valor++;
		pub->write((void*)&tp);
		tp.valor++;
		pub->write((void*)&tp);
	}
	else if(type == 2) //Reader
	{
		ReaderParams_t Rparam;
		Rparam.historySize = 2;
		Rparam.stateKind = STATELESS;
		Rparam.topicDataType = std::string("TipoPrueba");
		Rparam.topicName = std::string("Topico de prueba");
		Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
		Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);

		sub->assignNewMessageCallback(&newMsgCallback);

		while(1)
		{
			cout << "Blocking until new message arrives " << endl;
			sub->blockUntilNewMessage();
			cout << "After new message block " << endl;
			TipoPrueba_str tp;
			//if(sub->readOlderUnread((void*)&tp))
			//{
			//	tp.print();
			//}
			//cout << "Read: " << sub->getReadElements_n()<<" from History: "<<sub->getHistory_n()<< endl;
			//if(sub->isHistoryFull())
			//{
			//	cout << "Taking all read" <<endl;
			//	//sub->takeAllRead();

			//}
		}
	}
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



