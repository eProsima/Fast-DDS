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
#include "eprosimartps/threadtype/ThreadListen.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/dds/ParameterList.h"
#include "eprosimartps/writer/ReaderProxy.h"

#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/reader/WriterProxy.h"



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
	{
		pWarning("Too short message")
		throw ERR_MESSAGE_TOO_SHORT;
	}
	reset();
	destGuidPrefix = participantguidprefix;
	for(uint8_t i = 0;i<16;i++)
	{
		unicastReplyLocatorList[0].address[i] = loc.address[i];
	}

	msg->pos = 0; //Start reading at 0

	//Once everything is set, the reading begins:
	if(!checkRTPSHeader(msg))
		return;
	// Loop until there are no more submessages

	bool last_submsg = false;
	bool valid;
	int count = 0;
	SubmessageHeader_t submsgh; //Current submessage header
	//Pointers to different types of messages:

	while(msg->pos < msg->length)// end of the message
	{
		//First 4 bytes must contain: ID | flags | octets to next header
		if(!readSubmessageHeader(msg,&submsgh))
			return;
		if(msg->pos + submsgh.submessageLength > msg->length)
		{
			pWarning("SubMsg of invalid length"<<endl);
			throw ERR_SUBMSG_LENGTH_INVALID;
		}
		valid = true;
		count++;
		switch(submsgh.submessageId)
		{
		case DATA:
		{
			valid = proc_Submsg_Data(msg,&submsgh,&last_submsg);
			break;
		}
		case GAP:
		{
			valid = proc_Submsg_Gap(msg,&submsgh,&last_submsg);
			break;
		}
		case ACKNACK:
		{
			valid = proc_Submsg_Acknack(msg,&submsgh,&last_submsg);
			break;
		}
		case HEARTBEAT:
		{
			valid = proc_Submsg_Heartbeat(msg,&submsgh,&last_submsg);
			break;
		}
		case PAD:
			break;
		case INFO_DST:
			break;
		case INFO_SRC:
			break;
		case INFO_TS:
		{
			valid = proc_Submsg_InfoTS(msg,&submsgh,&last_submsg);
			break;
		}
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

bool MessageReceiver::checkRTPSHeader(CDRMessage_t*msg)
{
	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
			msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
	{
		pWarning("Message NOT RTPS"<<endl);
		return false;
	}
	msg->pos+=4;
	//CHECK AND SET protocol version
	if(msg->buffer[msg->pos] <= destVersion.major)
	{
		sourceVersion.major = msg->buffer[msg->pos];msg->pos++;
		sourceVersion.minor = msg->buffer[msg->pos];msg->pos++;
	}
	else
	{
		pWarning("Major RTPS Version not supported"<<endl);
		throw ERR_MESSAGE_VERSION_UNSUPPORTED;
	}
	//Set source vendor id
	sourceVendorId[0] = msg->buffer[msg->pos];msg->pos++;
	sourceVendorId[1] = msg->buffer[msg->pos];msg->pos++;
	//set source guid prefix
	memcpy(sourceGuidPrefix.value,&msg->buffer[msg->pos],12);
	msg->pos+=12;
	haveTimestamp = false;
	return true;
}


bool MessageReceiver::readSubmessageHeader(CDRMessage_t* msg,	SubmessageHeader_t* smh)
{
	if(msg->length - msg->pos < 4)
	{
		pWarning("SubmessageHeader too short");
		return false;
	}
	smh->submessageId = msg->buffer[msg->pos];msg->pos++;
	smh->flags = msg->buffer[msg->pos];msg->pos++;
	//Set endianness of message
	msg->msg_endian = smh->flags & BIT(0) ? LITTLEEND : BIGEND;
	CDRMessage::readUInt16(msg,&smh->submessageLength);
	return true;
}




bool MessageReceiver::proc_Submsg_Data(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
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
		if(!ParameterList::readParameterList(msg,&ParamList,&inlineQosSize,&ch->kind,&ch->instanceHandle))
		{
			pDebugInfo("SubMessage Data ERROR"<<endl);
			return false;
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
			if(!ParameterList::readParameterList(msg,&ParamList,&param_size,&ch->kind,&ch->instanceHandle))
			{
				pDebugInfo("SubMessage Data ERROR"<<endl);
				return false;
			}
			msg->msg_endian = previous_endian;
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;


	//FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.

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
	pDebugInfo("Sub Message DATA processed"<<endl);
	return true;
}

bool MessageReceiver::proc_Submsg_Heartbeat(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool finalFlag = smh->flags & BIT(1) ? true : false;
	bool livelinessFlag = smh->flags & BIT(2) ? true : false;
	//Assign message endianness
		if(endiannessFlag)
			msg->msg_endian = LITTLEEND;
		else
			msg->msg_endian = BIGEND;

	GUID_t readerGUID,writerGUID;
	readerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg,&readerGUID.entityId);
	writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&writerGUID.entityId);
	SequenceNumber_t firstSN, lastSN;
	CDRMessage::readSequenceNumber(msg,&firstSN);
	CDRMessage::readSequenceNumber(msg,&lastSN);
	uint32_t HBCount;
	CDRMessage::readUInt32(msg,&HBCount);

	//Look for the correct reader and writers:

	std::vector<RTPSReader*>::iterator it;
	for(it=threadListen_ptr->assoc_readers.begin();it!=threadListen_ptr->assoc_readers.end();it++)
	{
		if((*it)->guid == readerGUID || readerGUID.entityId == ENTITYID_UNKNOWN)
		{
			if((*it)->stateType == STATEFUL)
			{
				StatefulReader* SR = (StatefulReader*)(*it);
				//Look for the associated writer
				WriterProxy* WP;
				if(SR->matched_writer_lookup(writerGUID,&WP))
				{
					WP->missing_changes_update(&lastSN);
					WP->lost_changes_update(&firstSN);
					//Analyze wheter a acknack message is needed:
					if(!finalFlag)
					{
						WP->heartbeatResponse.timer->async_wait(boost::bind(&HeartbeatResponseDelay::event,&WP->heartbeatResponse,
																	boost::asio::placeholders::error,WP));
					}
					else if(finalFlag && !livelinessFlag)
					{
						if(!WP->isMissingChangesEmpty)
							WP->heartbeatResponse.timer->async_wait(boost::bind(&HeartbeatResponseDelay::event,&WP->heartbeatResponse,
											boost::asio::placeholders::error,WP));
					}
				}
				else
					pWarning("HB received from NOT associated writer");
			}
		}
	}

	return true;
}


bool MessageReceiver::proc_Submsg_Acknack(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	GUID_t readerGUID,writerGUID;
	readerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&readerGUID.entityId);
	writerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg,&writerGUID.entityId);


	SequenceNumberSet_t SNSet;
	CDRMessage::readSequenceNumberSet(msg,&SNSet);
	uint32_t count;
	CDRMessage::readUInt32(msg,&count);
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;

	//Look for the correct writer to use the acknack

	std::vector<RTPSWriter*>::iterator it;
	for(it=threadListen_ptr->assoc_writers.begin();it!=threadListen_ptr->assoc_writers.end();it++)
	{
		if((*it)->guid == writerGUID)
		{
			if((*it)->stateType == STATEFUL)
			{
				StatefulWriter* SF = (StatefulWriter*)(*it);
				//Look for the readerProxy the acknack is from
				std::vector<ReaderProxy*>::iterator rit;
				for(rit = SF->matched_readers.begin();rit!=SF->matched_readers.end();rit++)
				{
					if((*rit)->param.remoteReaderGuid == readerGUID)
					{
						(*rit)->acked_changes_set(&SNSet.base);
						(*rit)->requested_changes_set(&SNSet.set);
						if(!(*rit)->isRequestedChangesEmpty)
							(*rit)->nackResponse.timer->async_wait(boost::bind(&NackResponseDelay::event,(*rit)->nackResponse,
											boost::asio::placeholders::error,(*rit)));
						break;
					}
				}
			}
			break;
		}
	}
	return true;
}



bool MessageReceiver::proc_Submsg_Gap(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	return true;
}

bool MessageReceiver::proc_Submsg_InfoTS(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	msg->pos+=8;
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
