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
//
#include "eprosimartps/Participant.h"
#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/StatelessReader.h"
#include "eprosimartps/common/colors.h"

#define TESTSTRLENGTH 30


using namespace eprosima;
using namespace rtps;
using namespace std;

#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

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






bool checkParticipant(Participant* p){
	bool result,hres;
	hres = result = p->protocolVersion.major == 2;
	hres &= result &= p->protocolVersion.minor == 1;
	result_print("Protocol Version",result);
	hres &= result = p->vendorId[0] == (0x2A);
	hres &= result &= p->vendorId[1] == (0x2A);
	result_print("VendorId",result);
	for(int i=0;i<12;i++)
		cout << (int)p->guid.guidPrefix.value[i] << ",";
	cout << endl;
	for(int i=0;i<4;i++)
		cout << hex << (int)p->guid.entityId.value[i] << ",";
	cout << dec << endl;

	return true;
}



int main(){

	Participant p;
	//CHECK PARTICIPANT
	checkParticipant(&p);
	//cout << "After participant creation, threadListensize: " << p.threadListenList.size() << endl;
	WriterParams_t Wparam;
	Wparam.historySize = 2;
	Wparam.pushMode = true;
	Duration_t dur;
	dur.seconds = 1;
	dur.fraction = 0;
	Wparam.resendDataPeriod = dur;
	Wparam.heartbeatPeriod = dur;
	Wparam.nackResponseDelay = dur;
	Wparam.nackSupressionDuration = dur;

	Locator_t loc;
	loc.kind = LOCATOR_KIND_UDPv4;
	loc.port = 14244;
	loc.set_IP4_address(127,0,0,1);
	Wparam.unicastLocatorList.push_back(loc);
	StatelessWriter* SW2 = new StatelessWriter();
	p.createStatelessWriter(SW2,Wparam);
//	StatelessWriter* SW3 = new StatelessWriter();
//	p.createStatelessWriter(SW3,Wparam);

	//Add a StatelessReader
	ReaderParams_t Rparam;
	Rparam.historySize = 2;

	StatelessReader* SR1 = new StatelessReader();
	p.createStatelessReader(SR1,Rparam);
	cout << "Stateless Reader created correctly" << endl;


	sleep(1);
	//Add a ReaderLocator
	ReaderLocator RL2;
	RL2.expectsInlineQos = false;
	RL2.locator.kind = 1;
	RL2.locator.port = 14244;
	RL2.locator.set_IP4_address(127,0,0,1);
	cout << "IP ADDRESS: " <<
	SW2->reader_locator_add(RL2);
	ReaderLocator RL3;
	RL3.expectsInlineQos = false;
	RL3.locator.kind = 1;
	RL3.locator.port = 14244;
	RL3.locator.set_IP4_address(192,168,1,18);
	SW2->reader_locator_add(RL3);
	cout << "Reader LocatorS added " << endl;
	int numbers[4] = {1,2,3,4};
	SerializedPayload_t data;
	data.length = 4*sizeof(int);
	if(data.data !=NULL)
		free(data.data);
	data.data = (octet*)malloc(4*sizeof(int));
	memcpy(data.data,(octet*)numbers,4*sizeof(int));
	cout << "Before creating change" << endl;
	//Create changes
	CacheChange_t C21;
	SW2->new_change(ALIVE,&data,(void*)numbers,&C21);
	SW2->writer_cache.add_change(C21);


	sleep(5);
	return 0;

}



