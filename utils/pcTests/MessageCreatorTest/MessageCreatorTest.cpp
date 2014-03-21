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
#include <iomanip>
#include <bitset>
//
#include "eprosimartps/CDRMessageCreator.h"
#include "eprosimartps/MessageReceiver.h"
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




/**
 * Tests de MessageCreator class.
 * @return True if the test works, false if it doesn't.
 */

static inline void result_print(const char* p,bool result){
	cout << left << setw(TESTSTRLENGTH) << p; result ? (cout << BLUE <<"YES" << DEF) : (cout << RED << "NO" << DEF);cout << endl;
}
static inline void result_print_part(const char* p,bool result){
	cout << B_YELLOW << left << setw(TESTSTRLENGTH)  <<p << DEF; result ? (cout << B_BLUE <<"PASS" << DEF) : (cout << B_RED << "FAILED" << DEF);cout << endl;
}





bool testHeader(GuidPrefix_t* guidprefix,CDRMessage_t* msg){
	bool result,hres;
	hres = result = msg->buffer[msg->pos] == 'R';msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 'T';msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 'P';msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 'S';msg->pos++;
	result_print("Protocol String: ",result);
	hres &= result = msg->buffer[msg->pos] == 2;msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 1;msg->pos++;
	result_print("Protocol Version: ",result);
	hres &= result = msg->buffer[msg->pos] == 0;msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 0;msg->pos++;
	result_print("Vendor ID: ",result);
	hres &= result = msg->buffer[msg->pos] == guidprefix->value[0];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[1];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[2];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[3];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[4];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[5];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[6];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[7];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[8];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[9];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[10];msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == guidprefix->value[11];msg->pos++;
	result_print("Guid Prefix: ",result);
	result_print_part("HEADER: ",hres);

	return hres;
}


bool testSubMsgHeader(SubmsgData_t* DataSubM,CDRMessage_t* msg)
{
	MessageReceiver MR;
	bool hres, result;
	hres = result = msg->buffer[msg->pos] == DATA;msg->pos++;
	result_print("SubMessageId ",result);
	hres &= result = msg->buffer[msg->pos] == DataSubM->SubmessageHeader.flags;msg->pos++;
	result_print("Flags: ",result);
	//FIND OUT IS IT IS BIG OR LITTLE ENDIAN
	Endianness_t endianness;
	octet end = (DataSubM->SubmessageHeader.flags & BIT(0));
	if(end == 0x00)
		endianness = LITTLEEND; //LITTLEEND
	else if(end == 0x01)
		endianness = BIGEND;  //BIGEND
	msg->msg_endian = endianness;
	int16_t size;
	CDRMessage::readInt16(msg,&size);
	hres &= result = size == DataSubM->SubmessageHeader.submessageLength;
	result_print("Size: ",result);
	result_print_part("SUBMESSAGEHEADER: ",hres);

	return hres;
}

inline bool compareParameterLists(ParameterList_t* l1,ParameterList_t* l2){
	ParameterList_t::iterator it1,it2;
	bool result = false;
	for(it1=l1->begin();it1!=l1->end();it1++)
	{
		//	cout << "Looking for param: " << it1->parameterId << endl;
		result = false; //We suppose there is no match
		for(it2=l2->begin();it2!=l2->end();it2++) //Look in the other list, same order not guaranteed
		{
			if((it1->parameterId == it2->parameterId) && (it1->length == it2->length)) //Same Parameter
			{
				result = true; //We assume the data is correct.
				//cout << "Found parameter with same length: " << it1->length << endl;
				if(strcmp((const char*)it1->value,(const char*)it2->value)!=0){
					result = false;//Some data was not correct
					break;//dont check the rest of param.
				}
			}
		}
		result_print("Param: ",result);
		if(result == false) //No match for one parameter, the rest is irrelevant
			break;
	}
	return result;
}


bool testSubMsg(SubmsgData_t* DataSubM,CDRMessage_t* msg)
{
	bool hres, result;
	hres = result = msg->buffer[msg->pos] == 0x0;msg->pos++;
	hres &= result &= msg->buffer[msg->pos] == 0x0;msg->pos++;
	result_print("Extra Flags: ",result);
	int16_t size;
	CDRMessage::readInt16(msg,&size);
	hres &= result = size == RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG;
	result_print("Octets to InlineQos: ",result);
	result_print_part("FLAGS AND INLINE: ",hres);
	EntityId_t ID;
	CDRMessage::readEntityId(msg,&ID);
	hres = result = ID == DataSubM->readerId;
	result_print("Reader ID: ",result);
	CDRMessage::readEntityId(msg,&ID);
	hres &= result = ID == DataSubM->writerId;
	result_print("Writer ID: ",result);
	//SequenceNumber
	SequenceNumber_t sn;
	CDRMessage::readSequenceNumber(msg,&sn);
	hres &= result = sn == DataSubM->writerSN;
	result_print("Sequence Number: ",result);
	result_print_part("ENTITY AND SEQNUM  ",hres);
	octet inlineflag = DataSubM->SubmessageHeader.flags & BIT(1);
	octet serializedpayloadflag = DataSubM->SubmessageHeader.flags & BIT(2);
	if(inlineflag)
	{
		vector<Parameter_t> plist;
		CDRMessage::readParameterList(msg,&plist,&size);
		result = compareParameterLists(&DataSubM->inlineQos,&plist);
		hres = result;
		result_print_part("Param List  ",hres);
	}
	if(serializedpayloadflag)
	{
		octet* data = (octet*)malloc(sizeof(uint16_t)*10);
		CDRMessage::readData(msg,data,sizeof(uint16_t)*10);
		uint16_t* dd = (uint16_t*)data;
		hres = result = true;
		for(uint i = 0;i<10;i++){
			//cout << "Number, data: " << (uint16_t)i << " " << dd[i]<< endl;
			hres &= result = (uint16_t)i == dd[i];
			result_print("payload: ",result);
		}
		result_print_part("PAYLOAD  ",hres);
	}

	return hres;
}



bool testMessage(SubmsgData_t* DataSubM,CDRMessage_t* msg,GuidPrefix_t* guidprefix){
	//Reset position
	msg->pos = 0;
	if(!testHeader(guidprefix,msg))
		return false;
	if(!testSubMsgHeader(DataSubM,msg))
		return false;
	if(!testSubMsg(DataSubM,msg))
		return false;

	return true;
}



bool createMessage(SubmsgData_t* DataSubM,CDRMessage_t* msg,GuidPrefix_t* guidprefix){
	RTPSMessageCreator MC;
	for(uint8_t i = 0;i<12;i++)
		guidprefix->value[i] = (octet)i;
	//GUIDPREFIX_UNKNOWN(guidprefix);
	DataSubM->SubmessageHeader.submessageId = DATA;
	DataSubM->endiannessFlag = true;
	DataSubM->inlineQosFlag = true;
	DataSubM->dataFlag = true;
	DataSubM->keyFlag = false;
	octet flags = 0x0;
	if(DataSubM->endiannessFlag) flags = flags | BIT(0);
	if(DataSubM->inlineQosFlag) flags = flags | BIT(1);
	if(DataSubM->dataFlag) flags = flags | BIT(2);
	if(DataSubM->keyFlag) flags = flags | BIT(3);
	DataSubM->readerId = EntityId_t(ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
	DataSubM->writerId = EntityId_t(ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
	DataSubM->writerSN.low = 205210;
	DataSubM->writerSN.high = 2568;
	//PARAMETERS
	Parameter_t p1,p2,p3;
	p1.create(PID_TOPIC_NAME,std::string("Hola, este es un mensaje largo de mas longitud todavia"));
	DataSubM->inlineQos.push_back(p1);
	p2.create(PID_TYPE_NAME,std::string("Tipo de top"));
	DataSubM->inlineQos.push_back(p2);
	p3.create(PID_TYPE_NAME,std::string("Tipo de top otro dis"));
	DataSubM->inlineQos.push_back(p3);
	//DATA
	uint16_t numbers[] = {0,1,2,3,4,5,6,7,8,9};
	octet* oc_numbers = (octet*)numbers;
	DataSubM->serializedPayload.data = (octet*)malloc(sizeof(uint16_t)*10);
	DataSubM->serializedPayload.length = sizeof(uint16_t)*10;
	memcpy(DataSubM->serializedPayload.data,oc_numbers,sizeof(uint16_t)*10);
	//CREATE MESSAGE
	cout << "Creating message" << endl;
	MC.createMessageData(msg,*guidprefix,DataSubM);

	return true;
}




bool MessageDataTest2(){
	CDRMessage_t msg;
	SubmsgData_t sub;
	GuidPrefix_t guidprefix;
	createMessage(&sub,&msg,&guidprefix);


	testMessage(&sub,&msg,&guidprefix);

	return true;
}






int main(){


	MessageDataTest2();

	//LongTest();

	return 0;
}



//
//
//bool LongTest()
//{
//	octet* buff = (octet*)malloc(1000);
//	int32_t l1 = 244056;
//	uint32_t l2 = 230445;
//
//	Endianness_t end = BIGEND;
//	cout << "WRITE" << endl;
//	if(end == DEFAULT_ENDIAN){
//		int32_t* aux = (int32_t*)buff;
//		*aux = l1;
//		uint32_t * aux2 = (uint32_t*)(buff+4);
//		*aux2 = l2;
//	}
//	else
//	{
//		octet* o1;
//		octet* o2;
//		o1 = (octet*)&l1;
//		o2 = (octet*)&l2;
//		for(uint8_t i = 0;i<4;i++){
//			*(buff+i) = *(o1+4-1+i);
//			*(buff+i+4) = *(o2+4-1+i);
//		}
//	}
//	cout << "READ" << endl;
//	long l1r;
//	uint32_t l2r;
//	if(end == DEFAULT_ENDIAN){
//		int32_t* aux = (int32_t*)buff;
//		l1r =  *aux;
//		uint32_t * aux2 = (uint32_t*)(buff+4);
//		l2r = *aux2;
//	}
//	else
//	{
//		octet* o1;
//		octet* o2;
//		o1 = (octet*)&l1r;
//		o2 = (octet*)&l2r;
//		for(uint8_t i = 0;i<4;i++){
//			*(o1+i) = *(buff+4-1+i);
//			*(o2+i) = *(buff+4+4-1+i);
//		}
//	}
//	cout << l1r << " " << l2r << endl;
//	if(l1r==l1)
//		cout << "long ok" <<endl;
//	if(l2r==l2)
//		cout << "ulong ok" << endl;
//
//
//	return true;
//}

