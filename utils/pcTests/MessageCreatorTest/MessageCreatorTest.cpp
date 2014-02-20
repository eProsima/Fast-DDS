/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * MessageCreatorTest.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */


#include <stdio.h>
#include <string>
#include <iostream>


#include "eprosimartps/CDRMessageCreator.h"



using namespace eprosima::rtps;
using namespace std;



/**
 * Tests de MessageCreator class.
 * @return True if the test works, false if it doesn't.
 */
bool MessageDataTest(){

	CDRMessageCreator MC = CDRMessageCreator();

	CDRMessage_t msg;
	octet guidprefix[12] = GUIDPREFIX_UNKNOWN;
	SubmsgData_t DataSubM;
	DataSubM.SubmessageHeader.submessageId = DATA;
	DataSubM.endiannessFlag = true;
	DataSubM.inlineQosFlag = true;
	DataSubM.dataFlag = true;
	DataSubM.keyFlag = false;
	DataSubM.readerId = EntityId_t(ENTITYID_UNKNOWN);
	DataSubM.writerId = EntityId_t(ENTITYID_UNKNOWN);
	DataSubM.writerSN.low = 20;
	DataSubM.writerSN.high = 456;
	int numbers[] = {0,1,2,3,4,5,6,7,8,9};
	octet* oc_numbers = (octet*)numbers;
	DataSubM.serializedPayload.data = (octet*)malloc(sizeof(int)*10);
	DataSubM.serializedPayload.length = 10;
	memcpy(DataSubM.serializedPayload.data,oc_numbers,sizeof(int)*10);
	Parameter_t p1,p2;
	p1.parameterId = PID_TOPIC_NAME;
	std::string str("Hola Cocacola");
	p1.length =str.size()*sizeof(char);
	memcpy(p1.value,str.data(),p1.length);
	p2.parameterId = PID_TYPE_NAME;
	std::string str2("Tipo de dato");
	p2.length = str2.size()*sizeof(char);
	memcpy(p2.value,str2.data(),p2.length);
	DataSubM.inlineQos.push_back(p2);
	MC.createMessageData(&msg,guidprefix,&DataSubM);
	//CHECK THE MESSAGE
	bool result;
	short ioc = 0;
	result = msg.buffer[ioc] == 'R';
	result = msg.buffer[ioc++] == 'T';
	result = msg.buffer[ioc++] == 'P';
	result = msg.buffer[ioc++] == 'S';
	cout << "Protocol string: " << (result ? "OK" : "FAILED") << endl;


	return true;
}

int main(){


	bool res = MessageDataTest();


	return 0;
}
