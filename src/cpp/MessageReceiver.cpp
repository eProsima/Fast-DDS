/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * MessageReceiver.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/MessageReceiver.h"
#include "eprosimartps/ThreadListen.h"
#include "eprosimartps/RTPSReader.h"
#include "eprosimartps/Subscriber.h"
#include "eprosimartps/ParameterListCreator.h"


namespace eprosima {
namespace rtps {




MessageReceiver::MessageReceiver()
{
	// TODO Auto-generated constructor stub
	PROTOCOLVERSION(destVersion);
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(destGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);

	/*unicastReplyLocatorList.clear();
	multicastReplyLocatorList.clear();*/
}

MessageReceiver::~MessageReceiver() {
	// TODO Auto-generated destructor stub
}

void MessageReceiver::reset(){
	PROTOCOLVERSION(destVersion);
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(destGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);
	if(!unicastReplyLocatorList.empty())
		unicastReplyLocatorList.clear();
	if(!multicastReplyLocatorList.empty())
		multicastReplyLocatorList.clear();
	Locator_t defUniL;
	defUniL.kind = LOCATOR_KIND_UDPv4;
	LOCATOR_ADDRESS_INVALID(defUniL.address);
	defUniL.port = LOCATOR_PORT_INVALID;
	unicastReplyLocatorList.push_back(defUniL);
	multicastReplyLocatorList.push_back(defUniL);
}

void MessageReceiver::processCDRMsg(GuidPrefix_t participantguidprefix,Locator_t loc,CDRMessage_t*msg)
{
	if(msg->length < RTPSMESSAGE_HEADER_SIZE)
		throw ERR_MESSAGE_TOO_SHORT;
	reset();
	destGuidPrefix = participantguidprefix;
	for(uint8_t i = 0;i<16;i++)
	{
		unicastReplyLocatorList[0].address[i] = loc.address[i];
	}

	msg->pos = 0; //Start reading at 0

	//Once everything is set, the reading begins:
	Header_t H;
	if(!readHeader(msg, &H))
		throw ERR_MESSAGE_INCORRECT_HEADER;
	processHeader(&H);
	// Loop until there are no more submessages

	bool last_submsg = false;
	bool valid;
	int count = 0;
	SubmessageHeader_t submsgh; //Current submessage header
	//Pointers to different types of messages:

	while(msg->pos < msg->length)// end of the message
	{
		//First 4 bytes must contain: ID | flags | octets to next header
		readSubmessageHeader(msg,&submsgh);
		if(msg->pos + submsgh.submessageLength > msg->length)
			throw ERR_SUBMSG_LENGTH_INVALID;
		valid = true;
		count++;
		switch(submsgh.submessageId)
		{
		case DATA:
		{
			valid = proc_Submsg_Data(msg,&submsgh,&last_submsg);
			if(valid)
			{
				pDebugInfo("Sub Message DATA processed"<<endl)
			}

//			SubmsgData_t* SubmsgData = new SubmsgData_t();
//			valid = readSubmessageData(msg,&submsgh,&last_submsg,SubmsgData);
//			pDebugInfo( "Message Read")
//			if(valid)
//			{
//				SubmsgData->print();
//				processSubmessageData(SubmsgData);
//				pDebugInfo("Sub Message DATA processed")
//			}
			break;
		}
		case GAP:
			break;
		case ACKNACK:
			break;
		case HEARTBEAT:
			break;
		case PAD:
			break;
		case INFO_DST:
			break;
		case INFO_SRC:
			break;
		case INFO_TS:
			msg->pos+=8;
			break;
		case INFO_REPLY:
			break;
		case INFO_REPLY_IP4:
			break;
		default:
			msg->pos += submsgh.submessageLength; //ID NOT KNOWN. IGNORE AND CONTINUE
			break;
		}
		if(!valid || last_submsg)
			break;
	}

}

bool MessageReceiver::readSubmessageHeader(CDRMessage_t* msg,	SubmessageHeader_t* smh)
{
	if(msg->length - msg->pos < 4)
		throw ERR_SUBMSGHEADER_TOO_SHORT;
	smh->submessageId = msg->buffer[msg->pos];msg->pos++;
	smh->flags = msg->buffer[msg->pos];msg->pos++;
	//Set endianness of message
	msg->msg_endian = smh->flags & BIT(0) ? LITTLEEND : BIGEND;
	CDRMessage::readUInt16(msg,&smh->submessageLength);
	return true;
}

bool MessageReceiver::readHeader(CDRMessage_t* msg, Header_t* H) {
	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
			msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
		return false;
	msg->pos+=4;
	//CHECK AND SET protocol version
	if(msg->buffer[msg->pos] <= destVersion.major)
	{
		H->version.major = msg->buffer[msg->pos];msg->pos++;
		H->version.minor = msg->buffer[msg->pos];msg->pos++;
	}
	else
		throw ERR_MESSAGE_VERSION_UNSUPPORTED;
	//Set source vendor id
	H->vendorId[0] = msg->buffer[msg->pos];msg->pos++;
	H->vendorId[1] = msg->buffer[msg->pos];msg->pos++;
	//set source guid prefix
	memcpy(H->guidPrefix.value,&msg->buffer[msg->pos],12);
	msg->pos+=12;
	return true;
}

void MessageReceiver::processHeader(Header_t* H)
{
	sourceGuidPrefix = H->guidPrefix;
	sourceVersion = H->version;
	sourceVendorId[0] = H->vendorId[0];
	sourceVendorId[1] = H->vendorId[1];
	haveTimestamp = false;
	return;
}


bool MessageReceiver::readSubmessageData(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last,SubmsgData_t* SubmsgData) {

	//VALIDITY
	SubmsgData_t DSM;
	DSM.SubmessageHeader = *smh; //Copy SubmessageHeader
	//Fill flags bool values
	bool endiannessFlag = DSM.SubmessageHeader.flags & BIT(0) ? true : false;
	bool inlineQosFlag = DSM.SubmessageHeader.flags & BIT(1) ? true : false;
	bool dataFlag = DSM.SubmessageHeader.flags & BIT(2) ? true : false;
	bool keyFlag = DSM.SubmessageHeader.flags & BIT(3) ? true : false;
	if(keyFlag && dataFlag)
	{
		pWarning( "Message received with Data and Key Flag set."<<endl)
				return false;
	}

	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	//Extra flags don't matter now. Avoid those bytes
	msg->pos+=2;
	int16_t octetsToInlineQos;
	CDRMessage::readInt16(msg,&octetsToInlineQos); //it should be 16 in this implementation

	//reader and writer ID
	CDRMessage::readEntityId(msg,&DSM.readerId);
	CDRMessage::readEntityId(msg,&DSM.writerId);

	//Get sequence number
	CDRMessage::readSequenceNumber(msg,&DSM.writerSN);
	if(DSM.writerSN.to64long()<=0 || (DSM.writerSN.high == -1 && DSM.writerSN.low == 0)) //message invalid
		return false;

	//Jump ahead if more paraemters are before inlineQos (not in this version, maybe if further minor versions.)
	if(octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
		msg->pos += (octetsToInlineQos-RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);

	uint32_t inlineQosSize = 0;
	if(inlineQosFlag)
	{
		if(!ParameterListCreator::readParamListfromCDRmessage(&DSM.inlineQos,msg,&inlineQosSize))
			return false;
	}
	if(dataFlag || keyFlag)
	{
		int16_t payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE+octetsToInlineQos+inlineQosSize);
		msg->pos+=1;
		octet encapsulation;
		CDRMessage::readOctet(msg,&encapsulation);
		DSM.serializedPayload.encapsulation = (uint16_t)encapsulation;
		msg->pos+=2; //CDR Options, not used in this version
		if(dataFlag)
		{
			if(DSM.serializedPayload.data !=NULL)
				free(DSM.serializedPayload.data);
			DSM.serializedPayload.data = (octet*)malloc(payload_size-2-2);
			DSM.serializedPayload.length = payload_size-2-2;
			CDRMessage::readData(msg,DSM.serializedPayload.data,DSM.serializedPayload.length);
			DSM.changeKind = ALIVE;
		}
		else if(keyFlag)
		{
			Endianness_t previous_endian = msg->msg_endian;
			if(DSM.serializedPayload.encapsulation == PL_CDR_BE)
				msg->msg_endian = BIGEND;
			else if(DSM.serializedPayload.encapsulation == PL_CDR_LE)
				msg->msg_endian = LITTLEEND;
			else
			{
				pError( "MEssage received with bat encapsulation for KeyHash and status parameter list"<< endl);

			}
			ParameterList_t p;
			uint32_t param_size;
			if(!ParameterListCreator::readParamListfromCDRmessage(&p,msg,&param_size))
				return false;
			octet status;
			ParameterListCreator::getKeyStatus(&p,&DSM.instanceHandle,&status);
			if(status == 1)
				DSM.changeKind = NOT_ALIVE_DISPOSED;
			else if (status == 2)
				DSM.changeKind = NOT_ALIVE_UNREGISTERED;
			msg->msg_endian = previous_endian;
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;
	*SubmsgData = DSM;
	return true;
}

void MessageReceiver::processSubmessageData(SubmsgData_t* SMD)
{
	//Create CacheChange_t with data:
	CacheChange_t* ch = new CacheChange_t();
	ch->writerGUID.guidPrefix = sourceGuidPrefix;
	ch->writerGUID.entityId = SMD->writerId;
	ch->sequenceNumber = SMD->writerSN;
	ch->kind = SMD->changeKind;
	if(!SMD->inlineQos.params.empty())
	{
		octet status;
		ParameterListCreator::getKeyStatus(&SMD->inlineQos,&ch->instanceHandle,&status);
	}
	if(SMD->changeKind == ALIVE)
	{
		ch->serializedPayload.copy(&SMD->serializedPayload);

	}
	//Look for the correct reader to add the change
	std::vector<RTPSReader*>::iterator it;
	for(it=threadListen_ptr->assoc_readers.begin();it!=threadListen_ptr->assoc_readers.end();it++)
	{
		if(SMD->readerId == ENTITYID_UNKNOWN || (*it)->guid.entityId == SMD->readerId) //add to all
		{

			if((*it)->reader_cache.add_change(ch))
			{
				if((*it)->newMessageCallback !=NULL)
					(*it)->newMessageCallback();
				//else ///FIXME: removed for testing, put back.
				(*it)->newMessageSemaphore->post();
				if((*it)->stateType == STATEFUL)
				{
					//FIXME: Stateful implementation
				}
			}
		}
	}
}


bool MessageReceiver::readSubmessageHeartbeat(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageGap(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageAcknak(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessagePad(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageInfoDestination(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageInfoSource(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageInfoTimestamp(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::readSubmessageInfoReply(CDRMessage_t* msg,
		SubmessageHeader_t* smh,bool*last) {
	return true;
}

bool MessageReceiver::proc_Submsg_Data(CDRMessage_t* msg,
		SubmessageHeader_t* smh, bool* last)
{
	//READ and PROCESS
	CacheChange_t* ch = new CacheChange_t();
	//Fill flags bool values
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool inlineQosFlag = smh->flags & BIT(1) ? true : false;
	bool dataFlag = smh->flags & BIT(2) ? true : false;
	bool keyFlag = smh->flags & BIT(3) ? true : false;
	if(keyFlag && dataFlag)
	{
		pWarning( "Message received with Data and Key Flag set."<<endl)
		return false;
	}

	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	//Extra flags don't matter now. Avoid those bytes
	msg->pos+=2;
	int16_t octetsToInlineQos;
	CDRMessage::readInt16(msg,&octetsToInlineQos); //it should be 16 in this implementation
	//reader and writer ID
	EntityId_t reader;
	CDRMessage::readEntityId(msg,&reader);
	ch->writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&ch->writerGUID.entityId);

	//Get sequence number
	CDRMessage::readSequenceNumber(msg,&ch->sequenceNumber);
	if(ch->sequenceNumber.to64long()<=0 || (ch->sequenceNumber.high == -1 && ch->sequenceNumber.low == 0)) //message invalid
	{
		pWarning("Invalid message received, bad sequence Number"<<endl)
		return false;
	}

	//Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
	if(octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
		msg->pos += (octetsToInlineQos-RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
	uint32_t inlineQosSize = 0;
	ParameterList_t ParamList;
	if(inlineQosFlag)
	{
		if(!ParameterListCreator::readParamListfromCDRmessage(&ParamList,msg,&inlineQosSize))
			return false;
		octet status;
		if(!ParamList.inlineqos_params.empty())
		{
			ParameterListCreator::getKeyStatus(&ParamList,&ch->instanceHandle,&status);
			if(status == 1)
				ch->kind = NOT_ALIVE_DISPOSED;
			else if (status == 2)
				ch->kind = NOT_ALIVE_UNREGISTERED;
		}
	}
	if(dataFlag || keyFlag)
	{
		int16_t payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE+octetsToInlineQos+inlineQosSize);
		msg->pos+=1;
		octet encapsulation;
		CDRMessage::readOctet(msg,&encapsulation);
		ch->serializedPayload.encapsulation = (uint16_t)encapsulation;
		msg->pos+=2; //CDR Options, not used in this version
		if(dataFlag)
		{
			if(ch->serializedPayload.data !=NULL)
				free(ch->serializedPayload.data);
			ch->serializedPayload.data = (octet*)malloc(payload_size-2-2);
			ch->serializedPayload.length = payload_size-2-2;
			CDRMessage::readData(msg,ch->serializedPayload.data,ch->serializedPayload.length);
			ch->kind = ALIVE;
		}
		else if(keyFlag)
		{
			Endianness_t previous_endian = msg->msg_endian;
			if(ch->serializedPayload.encapsulation == PL_CDR_BE)
				msg->msg_endian = BIGEND;
			else if(ch->serializedPayload.encapsulation == PL_CDR_LE)
				msg->msg_endian = LITTLEEND;
			else
			{
				pError( "MEssage received with bat encapsulation for KeyHash and status parameter list"<< endl);
			}
			uint32_t param_size;
			if(!ParameterListCreator::readParamListfromCDRmessage(&ParamList,msg,&param_size))
				return false;
			octet status;
			ParameterListCreator::getKeyStatus(&ParamList,&ch->instanceHandle,&status);
			if(status == 1)
				ch->kind = NOT_ALIVE_DISPOSED;
			else if (status == 2)
				ch->kind = NOT_ALIVE_UNREGISTERED;
			msg->msg_endian = previous_endian;
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;



	//Create CacheChange_t with data:

	//Look for the correct reader to add the change
	std::vector<RTPSReader*>::iterator it;
	for(it=threadListen_ptr->assoc_readers.begin();it!=threadListen_ptr->assoc_readers.end();it++)
	{
		if(reader == ENTITYID_UNKNOWN || (*it)->guid.entityId == reader) //add
		{
			if((*it)->reader_cache.add_change(ch))
			{
				if((*it)->newMessageCallback !=NULL)
					(*it)->newMessageCallback();
				//else ///FIXME: removed for testing, put back.
				(*it)->newMessageSemaphore->post();
				if((*it)->stateType == STATEFUL)
				{
					//FIXME: Stateful implementation
				}
			}
		}
	}





	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
