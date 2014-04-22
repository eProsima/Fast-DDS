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
	PROTOCOLVERSION(destVersion);
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(destGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);

	defUniLoc.kind = LOCATOR_KIND_UDPv4;
	LOCATOR_ADDRESS_INVALID(defUniLoc.address);
	defUniLoc.port = LOCATOR_PORT_INVALID;
	mp_threadListen = NULL;

}

MessageReceiver::~MessageReceiver()
{
	this->m_ParamList.deleteParams();
	pDebugInfo("MessageReceiver destructor"<<endl;);
}

void MessageReceiver::reset(){
	PROTOCOLVERSION(destVersion);
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(destGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);

	unicastReplyLocatorList.clear();

	multicastReplyLocatorList.clear();


	unicastReplyLocatorList.push_back(defUniLoc);
	multicastReplyLocatorList.push_back(defUniLoc);
}

void MessageReceiver::processCDRMsg(GuidPrefix_t& participantguidprefix,
		Locator_t* loc,CDRMessage_t*msg)
{
	if(msg->length < RTPSMESSAGE_HEADER_SIZE)
	{
		pWarning("Too short message")
		throw ERR_MESSAGE_TOO_SHORT;
	}
	reset();
	destGuidPrefix = participantguidprefix;
	unicastReplyLocatorList[0].kind = loc->kind;
	uint8_t n_start = 0;
	if(loc->kind == 1)
		n_start = 12;
	else if(loc->kind == 2)
		n_start = 0;
	else
	{
		pWarning("MesReceiver:processCDRMsg:Locator kind invalid"<<endl);
		return;
	}
	for(uint8_t i = n_start;i<16;i++)
	{
		unicastReplyLocatorList[0].address[i] = loc->address[i];
	}
	unicastReplyLocatorList[0].port = loc->port;
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
			pDebugInfo("Data Submsg received, processing..."<<endl);
			valid = proc_Submsg_Data(msg,&submsgh,&last_submsg);
			break;
		}
		case GAP:
		{
			pDebugInfo("Gap Submsg received, processing..."<<endl);
			valid = proc_Submsg_Gap(msg,&submsgh,&last_submsg);
			break;
		}
		case ACKNACK:
		{
			pDebugInfo("Acknack Submsg received, processing..."<<endl);
			valid = proc_Submsg_Acknack(msg,&submsgh,&last_submsg);
			break;
		}
		case HEARTBEAT:
		{
			pDebugInfo("Heartbeat Submsg received, processing..."<<endl);
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
			pDebugInfo("InfoTS Submsg received, processing..."<<endl);
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
	if(msg->buffer[msg->pos] <= destVersion.m_major)
	{
		sourceVersion.m_major = msg->buffer[msg->pos];msg->pos++;
		sourceVersion.m_minor = msg->buffer[msg->pos];msg->pos++;
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
	//WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:

	RTPSReader* firstReader = NULL;
	if(mp_threadListen->m_assoc_readers.empty())
	{
		pWarning("Data received when NO readers are created"<<endl);
		return false;
	}
	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assoc_readers.begin();
			it!=mp_threadListen->m_assoc_readers.end();++it)
	{
		if(reader == ENTITYID_UNKNOWN || (*it)->m_guid.entityId == reader) //add
		{
			firstReader = *it;
			break;
		}
	}
	if(firstReader == NULL) //Reader not found
	{
		pWarning("Data Message received for unknown reader");
		return false;
	}
	//FOUND THE READER.
	//We ask the reader for a cachechange
	CacheChange_t* ch = firstReader->m_reader_cache.reserve_Cache();
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

	if(inlineQosFlag)
	{
		if(ParameterList::readParameterListfromCDRMsg(msg,&m_ParamList,&ch->instanceHandle,&ch->kind) > 0)
		{
			pDebugInfo("SubMessage Data ERROR"<<endl);
			return false;
		}
	}

	if(dataFlag || keyFlag)
	{
		int16_t payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE+octetsToInlineQos+inlineQosSize);
		msg->pos+=1;
		octet encapsulation =0;
		CDRMessage::readOctet(msg,&encapsulation);
		ch->serializedPayload.encapsulation = (uint16_t)encapsulation;
		msg->pos+=2; //CDR Options, not used in this version
		if(dataFlag)
		{
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
			//uint32_t param_size;
			if(ParameterList::readParameterListfromCDRMsg(msg,&m_ParamList,&ch->instanceHandle,&ch->kind) > 0)
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
	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assoc_readers.begin();
			it!=mp_threadListen->m_assoc_readers.end();++it)
	{
		if(reader == ENTITYID_UNKNOWN || (*it)->m_guid.entityId == reader) //add
		{
			CacheChange_t* change_to_add;
			if(firstReader->m_guid.entityId == (*it)->m_guid.entityId) //IS the same as the first one
			{
				change_to_add = ch;
			}
			else
			{
				change_to_add = (*it)->m_reader_cache.reserve_Cache(); //Reserve a new cache from the corresponding cache pool
				change_to_add->copy(ch);
			}


			if((*it)->m_reader_cache.add_change(change_to_add))
			{
				if((*it)->m_stateType == STATEFUL)
				{
					StatefulReader* SFR = (StatefulReader*)(*it);
					WriterProxy* WP;
					if(SFR->matched_writer_lookup(change_to_add->writerGUID,&WP))
					{
						WP->received_change_set(change_to_add);
					}
					else
					{
						//FIXME: REMOVE THIS ONCE DISCOVERY WORKS.
						WriterProxy_t newWriterProxy;
						newWriterProxy.remoteWriterGuid.guidPrefix = sourceGuidPrefix;
						newWriterProxy.remoteWriterGuid.entityId = change_to_add->writerGUID.entityId;
						this->unicastReplyLocatorList[0].port = 10043; //default receiving port
						newWriterProxy.unicastLocatorList   = this->unicastReplyLocatorList;
						newWriterProxy.multicastLocatorList = this->multicastReplyLocatorList;
						SFR->matched_writer_add(&newWriterProxy);
						SFR->matched_writer_lookup(change_to_add->writerGUID,&WP);
						WP->received_change_set(change_to_add);
					}
				}
				(*it)->mp_listener->newMessageCallback();
//				if((*it)->newMessageCallback !=NULL)
//					(*it)->newMessageCallback();
				///FIXME: removed for testing, put back.
				(*it)->newMessageSemaphore->post();
			}
			else
			{
				pWarning("MessageReceiver not add change "<<change_to_add->sequenceNumber.to64long()<<endl);
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


	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assoc_readers.begin();
			it!=mp_threadListen->m_assoc_readers.end();++it)
	{
		if((*it)->m_guid == readerGUID || readerGUID.entityId == ENTITYID_UNKNOWN)
		{
			if((*it)->m_stateType == STATEFUL)
			{
				StatefulReader* SR = (StatefulReader*)(*it);
				//Look for the associated writer
				WriterProxy* WP;
				if(SR->matched_writer_lookup(writerGUID,&WP))
				{
					if(WP->m_lastHeartbeatCount < HBCount)
					{
						WP->m_lastHeartbeatCount = HBCount;
						WP->missing_changes_update(&lastSN);
						WP->lost_changes_update(&firstSN);
						WP->m_heartbeatFinalFlag = finalFlag;
						//Analyze wheter a acknack message is needed:

						if(!finalFlag)
						{
							WP->m_heartbeatResponse.restart_timer();
						}
						else if(finalFlag && !livelinessFlag)
						{
							if(!WP->m_isMissingChangesEmpty)
								WP->m_heartbeatResponse.restart_timer();
						}
						//TODOG: Livelinessflag behaviour
					}
				}
				else
					pWarning("HB received from NOT associated writer"<<endl);
			}
		}
	}

	return true;
}


bool MessageReceiver::proc_Submsg_Acknack(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{

	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;
	GUID_t readerGUID,writerGUID;
	readerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&readerGUID.entityId);
	writerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg,&writerGUID.entityId);


	SequenceNumberSet_t SNSet;
	CDRMessage::readSequenceNumberSet(msg,&SNSet);
	uint32_t Ackcount;
	CDRMessage::readUInt32(msg,&Ackcount);
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;

	//Look for the correct writer to use the acknack

	for(std::vector<RTPSWriter*>::iterator it=mp_threadListen->m_assoc_writers.begin();
			it!=mp_threadListen->m_assoc_writers.end();++it)
	{
		if((*it)->m_guid == writerGUID)
		{
			if((*it)->m_stateType == STATEFUL)
			{
				StatefulWriter* SF = (StatefulWriter*)(*it);
				//Look for the readerProxy the acknack is from
				for(std::vector<ReaderProxy*>::iterator rit = SF->matched_readers.begin();
						rit!=SF->matched_readers.end();++rit)
				{
					if((*rit)->m_param.remoteReaderGuid == readerGUID || (*rit)->m_param.remoteReaderGuid.entityId == ENTITYID_UNKNOWN) //FIXME: only for testing
					{

						if((*rit)->m_lastAcknackCount < Ackcount)
						{
							(*rit)->m_lastAcknackCount = Ackcount;
							(*rit)->acked_changes_set(&SNSet.base);
							std::vector<SequenceNumber_t> set_vec = SNSet.get_set();
							(*rit)->requested_changes_set(set_vec);
							if(!(*rit)->m_isRequestedChangesEmpty)
							{
								(*rit)->m_nackResponse.restart_timer();
							}
						}
						break;
					}
				}
				return true;
			}
			else
			{
				pDebugInfo("Acknack msg to NOT stateful writer "<<endl);
				return false;
			}
		}
	}
	pDebugInfo("Acknack msg to unknown writer");
	return false;
}



bool MessageReceiver::proc_Submsg_Gap(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;
	GUID_t writerGUID,readerGUID;
	readerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg,&readerGUID.entityId);
	writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&writerGUID.entityId);
	SequenceNumber_t gapStart;
	CDRMessage::readSequenceNumber(msg,&gapStart);
	SequenceNumberSet_t gapList;
	CDRMessage::readSequenceNumberSet(msg,&gapList);
	if(gapStart.to64long()<=0)
		return false;


	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assoc_readers.begin();
			it!=mp_threadListen->m_assoc_readers.end();++it)
	{
		if((*it)->m_guid == readerGUID || readerGUID.entityId == ENTITYID_UNKNOWN)
		{
			if((*it)->m_stateType == STATEFUL)
			{
				StatefulReader* SR = (StatefulReader*)(*it);
				//Look for the associated writer
				WriterProxy* WP;
				if(SR->matched_writer_lookup(writerGUID,&WP))
				{
					SequenceNumber_t auxSN;
					SequenceNumber_t finalSN = gapList.base -1;
					for(auxSN = gapStart;auxSN<=finalSN;auxSN++)
						WP->irrelevant_change_set(&auxSN);

					for(std::vector<SequenceNumber_t>::iterator it=gapList.get_begin();it!=gapList.get_end();++it)
						WP->irrelevant_change_set(&(*it));
				}
				else
					pWarning("GAP received from NOT associated writer"<<endl);
			}
		}
	}
	return true;
}

bool MessageReceiver::proc_Submsg_InfoTS(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool timeFlag = smh->flags & BIT(1) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	if(!timeFlag)
	{
		haveTimestamp = true;
		CDRMessage::readTimestamp(msg,&timestamp);
	}
	else
		haveTimestamp = false;

	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
