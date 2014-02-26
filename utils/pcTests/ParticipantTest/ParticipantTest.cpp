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
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
//
#include "eprosimartps/Participant.h"
#include "eprosimartps/StatelessWriter.h"
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
	StatelessWriter* SW = new StatelessWriter();
	WriterParams_t Wparam;
	Wparam.HistorySize = 2;
	Wparam.pushMode = true;
	Duration_t dur;
	dur.seconds = 1;
	dur.fraction = 0;
	Wparam.resendDataPeriod = dur;
	Wparam.heartbeatPeriod = dur;
	Wparam.nackResponseDelay = dur;
	Wparam.nackSupressionDuration = dur;
	cout << "Param writer created" << endl;
	p.createStatelessWriter(SW,Wparam);
	//Create another one, this time with a different locator list
	Locator_t loc;
	loc.kind = LOCATOR_KIND_UDPv4;
	loc.port = 2222;
	loc.set_IP4_address(192,168,1,74);
	Wparam.unicastLocatorList.push_back(loc);
	StatelessWriter* SW2 = new StatelessWriter();
	p.createStatelessWriter(SW2,Wparam);
	StatelessWriter* SW3 = new StatelessWriter();
	p.createStatelessWriter(SW3,Wparam);


	sleep(3);
	return 0;

}



