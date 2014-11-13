/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MessageReceiver.cpp
 *
 */

#include "eprosimartps/MessageReceiver.h"
#include "eprosimartps/common/RTPS_messages.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/resources/ListenResource.h"

#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/reader/WriterProxyData.h"

#include "eprosimartps/pubsub/SubscriberListener.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "MessageReceiver";


MessageReceiver::MessageReceiver(uint32_t rec_buffer_size):
				m_rec_msg(rec_buffer_size)
{
	const char* const METHOD_NAME = "MessageReceiver";
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
	logInfo(RTPS_MSG_IN,"Created with CDRMessage of size: "<<m_rec_msg.max_size,EPRO_BLUE);

}

MessageReceiver::~MessageReceiver()
{
	const char* const METHOD_NAME = "~MessageReceiver";
	this->m_ParamList.deleteParams();
	logInfo(RTPS_MSG_IN,"",EPRO_BLUE);
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

void MessageReceiver::processCDRMsg(const GuidPrefix_t& participantguidprefix,
		Locator_t* loc,CDRMessage_t*msg)
{
	const char* const METHOD_NAME = "processCDRMsg";
	if(msg->length < RTPSMESSAGE_HEADER_SIZE)
	{
		logWarning(RTPS_MSG_IN,"Received message too short, ignoring",EPRO_BLUE)
		return;
	}
	reset();
	destGuidPrefix = participantguidprefix;
	unicastReplyLocatorList.begin()->kind = loc->kind;
	uint8_t n_start = 0;
	if(loc->kind == 1)
		n_start = 12;
	else if(loc->kind == 2)
		n_start = 0;
	else
	{
		logWarning(RTPS_MSG_IN,"Locator kind invalid",EPRO_BLUE);
		return;
	}
	for(uint8_t i = n_start;i<16;i++)
	{
		unicastReplyLocatorList.begin()->address[i] = loc->address[i];
	}
	unicastReplyLocatorList.begin()->port = loc->port;
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
		//cout << msg->pos << "||"<<submsgh.submessageLength << "||"<<msg->length<<endl;
		if(msg->pos + submsgh.submessageLength > msg->length)
		{
			logWarning(RTPS_MSG_IN,"SubMsg of invalid length ("<<submsgh.submessageLength
					<< ") with current msg position/length ("<<msg->pos << "/"<<msg->length << ")",EPRO_BLUE);
			return;
		}
		valid = true;
		count++;
		switch(submsgh.submessageId)
		{
		case DATA:
		{
			if(this->destGuidPrefix != participantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,"Data Submsg ignored, DST is another participant",EPRO_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,"Data Submsg received, processing.",EPRO_BLUE);
				valid = proc_Submsg_Data(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case GAP:
		{
			if(this->destGuidPrefix != participantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,"Gap Submsg ignored, DST is another participant...",EPRO_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,"Gap Submsg received, processing...",EPRO_BLUE);
				valid = proc_Submsg_Gap(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case ACKNACK:
		{
			if(this->destGuidPrefix != participantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,"Acknack Submsg ignored, DST is another participant...",EPRO_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,"Acknack Submsg received, processing...",EPRO_BLUE);
				valid = proc_Submsg_Acknack(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case HEARTBEAT:
		{
			if(this->destGuidPrefix != participantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,"HB Submsg ignored, DST is another participant...",EPRO_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,"Heartbeat Submsg received, processing...",EPRO_BLUE);
				valid = proc_Submsg_Heartbeat(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case PAD:
			logWarning(RTPS_MSG_IN,"PAD messages not yet implemented, ignoring",EPRO_BLUE);
			msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_DST:
			logInfo(RTPS_MSG_IN,"InfoDST message received, processing...",EPRO_BLUE);
			valid = proc_Submsg_InfoDST(msg,&submsgh,&last_submsg);
			//				pWarning("Info DST messages not yet implemented"<<endl);
			//				msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_SRC:
			logInfo(RTPS_MSG_IN,"InfoSRC message received, processing...",EPRO_BLUE);
			valid = proc_Submsg_InfoSRC(msg,&submsgh,&last_submsg);
			//			pWarning("Info SRC messages not yet implemented"<<endl);
			//			msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_TS:
		{
			logInfo(RTPS_MSG_IN,"InfoTS Submsg received, processing...",EPRO_BLUE);
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
	const char* const METHOD_NAME = "checkRTPSHeader";
	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
			msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
	{
		logInfo(RTPS_MSG_IN,"Msg received with no RTPS in header, ignoring...",EPRO_BLUE);
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
		logWarning(RTPS_MSG_IN,"Major RTPS Version not supported",EPRO_BLUE);
		return false;
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
	const char* const METHOD_NAME = "readSubmessageHeader";
	if(msg->length - msg->pos < 4)
	{
		logWarning(RTPS_MSG_IN,"SubmessageHeader too short");
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
	const char* const METHOD_NAME = "proc_Submsg_Data";
	//READ and PROCESS
	if(smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
	{
		logInfo(RTPS_MSG_IN,"Too short submessage received, ignoring",EPRO_BLUE);
		return false;
	}
	//Fill flags bool values
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool inlineQosFlag = smh->flags & BIT(1) ? true : false;
	bool dataFlag = smh->flags & BIT(2) ? true : false;
	bool keyFlag = smh->flags & BIT(3) ? true : false;
	if(keyFlag && dataFlag)
	{
		logWarning(RTPS_MSG_IN, "Message received with Data and Key Flag set, ignoring")
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
	EntityId_t readerID;
	CDRMessage::readEntityId(msg,&readerID);

	//WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:

	RTPSReader* firstReader = NULL;
	bool firstReaderNeedsToRelease = true;
	if(mp_threadListen->m_assocReaders.empty())
	{
		logWarning(RTPS_MSG_IN,"Data received in locator: "<<mp_threadListen->m_listenLoc<< ", when NO readers are listening",EPRO_BLUE);
		return false;
	}
	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assocReaders.begin();
			it!=mp_threadListen->m_assocReaders.end();++it)
	{
		if((*it)->acceptMsgDirectedTo(readerID)) //add
		{
			firstReader = *it;
			break;
		}
	}
	if(firstReader == NULL) //Reader not found
	{
		logWarning(RTPS_MSG_IN,"No Reader in this Locator ("<<mp_threadListen->m_listenLoc<< ")"
				" accepts this message (directed to: " <<readerID << ")",EPRO_BLUE);
		return false;
	}
	//FOUND THE READER.
	//We ask the reader for a cachechange to store the information.
	CacheChange_t* ch = firstReader->reserve_Cache();
	ch->writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&ch->writerGUID.entityId);

	//Get sequence number
	CDRMessage::readSequenceNumber(msg,&ch->sequenceNumber);

	if(ch->sequenceNumber.to64long()<=0 || (ch->sequenceNumber.high == -1 && ch->sequenceNumber.low == 0)) //message invalid
	{
		logWarning(RTPS_MSG_IN,"Invalid message received, bad sequence Number",EPRO_BLUE)
		return false;
	}

	//Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
	if(octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
		msg->pos += (octetsToInlineQos-RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);

	uint32_t inlineQosSize = 0;

	if(inlineQosFlag)
	{
		inlineQosSize = ParameterList::readParameterListfromCDRMsg(msg,&m_ParamList,&ch->instanceHandle,&ch->kind);
		if(inlineQosSize <= 0)
		{
			logInfo(RTPS_MSG_IN,"SubMessage Data ERROR, Inline Qos ParameterList error");
			firstReader->release_Cache(ch);
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
			if(ch->serializedPayload.max_size >= payload_size-2-2)
			{
				ch->serializedPayload.length = payload_size-2-2;
				CDRMessage::readData(msg,ch->serializedPayload.data,ch->serializedPayload.length);
				ch->kind = ALIVE;
			}
			else
			{
				logWarning(RTPS_MSG_IN,"Serlialized Payload larger than maximum allowed size "
						"("<<payload_size-2-2<<"/"<<ch->serializedPayload.max_size<<")",EPRO_BLUE);
				firstReader->release_Cache(ch);
				return false;
			}
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
				logError(RTPS_MSG_IN, "Bad encapsulation for KeyHash and status parameter list");
				return false;
			}
			//uint32_t param_size;
			if(ParameterList::readParameterListfromCDRMsg(msg,&m_ParamList,&ch->instanceHandle,&ch->kind) <= 0)
			{
				logInfo(RTPS_MSG_IN,"SubMessage Data ERROR, keyFlag ParameterList",EPRO_BLUE);
				return false;
			}
			msg->msg_endian = previous_endian;
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;


	//FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.
	logInfo(RTPS_MSG_IN,"Looking through all RTPSReaders associated with this ListenResource",EPRO_BLUE);
	//Look for the correct reader to add the change
	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assocReaders.begin();
			it!=mp_threadListen->m_assocReaders.end();++it)
	{
		boost::lock_guard<Endpoint> guard(*(Endpoint*)(*it));
		WriterProxy* pWP = NULL;
		if((*it)->acceptMsgDirectedTo(readerID) && (*it)->acceptMsgFrom(ch->writerGUID,&pWP)) //add
		{
			logInfo(RTPS_MSG_IN,"MessageReceiver: Trying to add change "
					<< ch->sequenceNumber.to64long() <<" TO reader: "<<(*it)->getGuid().entityId,EPRO_BLUE);
			CacheChange_t* change_to_add;
			if(firstReader->getGuid().entityId == (*it)->getGuid().entityId) //IS the same as the first one
			{
				change_to_add = ch;
				firstReaderNeedsToRelease = false;
			}
			else
			{
				change_to_add = (*it)->reserve_Cache(); //Reserve a new cache from the corresponding cache pool
				change_to_add->copy(ch);
			}
			if(haveTimestamp)
				change_to_add->sourceTimestamp = this->timestamp;
			if((*it)->getStateType() == STATEFUL)
			{
				if((*it)->add_change(change_to_add,pWP))
				{
					SequenceNumber_t maxSeqNumAvailable;
					pWP->available_changes_max(&maxSeqNumAvailable);
					if(change_to_add->sequenceNumber <= maxSeqNumAvailable)
					{
						if((*it)->getListener()!=NULL)
						{
							//cout << "CALLING NEWDATAMESSAGE "<<endl;
							(*it)->getListener()->onNewDataMessage();
							//cout << "FINISH CALLING " <<endl;
							if((*it)->isHistoryFull())
								(*it)->getListener()->onHistoryFull();
						}
						(*it)->m_semaphore.post();
					}
				}
				else
				{
					(*it)->release_Cache(change_to_add);
					logInfo(RTPS_MSG_IN,"MessageReceiver not add change "<<change_to_add->sequenceNumber.to64long(),EPRO_BLUE);
				}

			}
			else
			{
				if((*it)->add_change(change_to_add))
				{
					if((*it)->getListener()!=NULL)
					{
						(*it)->getListener()->onNewDataMessage();
						if((*it)->isHistoryFull())
							(*it)->getListener()->onHistoryFull();
					}
					(*it)->m_semaphore.post();
				}
				else
				{
					logInfo(RTPS_MSG_IN,"MessageReceiver not add change "
							<<change_to_add->sequenceNumber.to64long(),EPRO_BLUE);
					(*it)->release_Cache(change_to_add);
					if((*it)->getGuid().entityId == c_EntityId_SPDPReader)
					{
						this->mp_threadListen->getParticipantImpl()->getBuiltinProtocols()->mp_PDP->assertRemoteParticipantLiveliness(this->sourceGuidPrefix);
					}
				}
			}
		}
	}
	//cout << "CHECKED ALL READERS "<<endl;
	if(firstReaderNeedsToRelease)
		firstReader->release_Cache(ch);
	logInfo(RTPS_MSG_IN,"Sub Message DATA processed",EPRO_BLUE);
	return true;
}

bool MessageReceiver::proc_Submsg_Heartbeat(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_Heartbeat";
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
	if(lastSN<firstSN)
	{
		logInfo(RTPS_MSG_IN,"HB Received with lastSN < firstSN, ignoring",EPRO_BLUE);
		return false;
	}
	uint32_t HBCount;
	CDRMessage::readUInt32(msg,&HBCount);

	//Look for the correct reader and writers:


	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assocReaders.begin();
			it!=mp_threadListen->m_assocReaders.end();++it)
	{
		if((*it)->acceptMsgFrom(writerGUID) && (*it)->acceptMsgDirectedTo(readerGUID.entityId)) //(*it)->acceptMsgDirectedTo(readerGUID.entityId) &&
		{
			if((*it)->getStateType() == STATEFUL)
			{
				StatefulReader* SR = (StatefulReader*)(*it);
				//Look for the associated writer
				WriterProxy* WP;
				//FIXME: Ignoring too close received HB.
				if(SR->matched_writer_lookup(writerGUID,&WP))
				{
					if(WP->m_lastHeartbeatCount < HBCount)
					{
						WP->m_lastHeartbeatCount = HBCount;
						WP->lost_changes_update(firstSN);
						WP->missing_changes_update(lastSN);
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
						//FIXME: livelinessFlag
						if(livelinessFlag && WP->m_data->m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
							WP->m_data->m_isAlive = true;
					}
				}
				else
				{
					logInfo(RTPS_MSG_IN,"HB received from NOT associated writer",EPRO_BLUE);
				}
			}
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;
	return true;
}


bool MessageReceiver::proc_Submsg_Acknack(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_Acknack";
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool finalFlag = smh->flags & BIT(1) ? true: false;
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

	for(std::vector<RTPSWriter*>::iterator it=mp_threadListen->m_assocWriters.begin();
			it!=mp_threadListen->m_assocWriters.end();++it)
	{
		if((*it)->getGuid() == writerGUID)
		{
			if((*it)->getStateType() == STATEFUL)
			{
				StatefulWriter* SF = (StatefulWriter*)(*it);
				//Look for the readerProxy the acknack is from
				for(p_ReaderProxyIterator rit = SF->matchedReadersBegin();rit!=SF->matchedReadersEnd();++rit)
				{
					if((*rit)->m_data->m_guid == readerGUID )//|| (*rit)->m_param.remoteReaderGuid.entityId == ENTITYID_UNKNOWN) //FIXME: only for testing
					{

						if((*rit)->m_lastAcknackCount < Ackcount)
						{
							(*rit)->m_lastAcknackCount = Ackcount;
							(*rit)->acked_changes_set(SNSet.base);
							std::vector<SequenceNumber_t> set_vec = SNSet.get_set();
							(*rit)->requested_changes_set(set_vec);
							if(!(*rit)->m_isRequestedChangesEmpty || !finalFlag)
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
				logInfo(RTPS_MSG_IN,"Acknack msg to NOT stateful writer ",EPRO_BLUE);
				return false;
			}
		}
	}
	logInfo(RTPS_MSG_IN,"Acknack msg to UNKNOWN writer (I loooked through "
			<< mp_threadListen->m_assocWriters.size() << " writers in this ListenResource)",EPRO_BLUE);
	return false;
}



bool MessageReceiver::proc_Submsg_Gap(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_Gap";
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;

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


	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assocReaders.begin();
			it!=mp_threadListen->m_assocReaders.end();++it)
	{
		if((*it)->acceptMsgFrom(writerGUID) && (*it)->acceptMsgDirectedTo(readerGUID.entityId))
		{
			if((*it)->getStateType() == STATEFUL)
			{
				StatefulReader* SR = (StatefulReader*)(*it);
				//Look for the associated writer
				WriterProxy* WP;
				if(SR->matched_writer_lookup(writerGUID,&WP))
				{
					SequenceNumber_t auxSN;
					SequenceNumber_t finalSN = gapList.base -1;
					for(auxSN = gapStart;auxSN<=finalSN;auxSN++)
						WP->irrelevant_change_set(auxSN);

					for(std::vector<SequenceNumber_t>::iterator it=gapList.get_begin();it!=gapList.get_end();++it)
						WP->irrelevant_change_set((*it));
				}
				else
					logWarning(RTPS_MSG_IN,"GAP received from NOT associated writer",EPRO_BLUE);
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
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;
	if(!timeFlag)
	{
		haveTimestamp = true;
		CDRMessage::readTimestamp(msg,&timestamp);
	}
	else
		haveTimestamp = false;

	return true;
}

bool MessageReceiver::proc_Submsg_InfoDST(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_InfoDST";
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//bool timeFlag = smh->flags & BIT(1) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;
	GuidPrefix_t guidP;
	CDRMessage::readData(msg,guidP.value,12);
	if(guidP != c_GuidPrefix_Unknown)
	{
		this->destGuidPrefix = guidP;
		logInfo(RTPS_MSG_IN,"DST Participant is now: "<< this->destGuidPrefix,EPRO_BLUE);
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;
	return true;
}

bool MessageReceiver::proc_Submsg_InfoSRC(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_InfoSRC";
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//bool timeFlag = smh->flags & BIT(1) ? true : false;
	//Assign message endianness
	if(endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;
	if(smh->submessageLength == 20 || smh->submessageLength==0)
	{
		//AVOID FIRST 4 BYTES:
		msg->pos+=4;
		CDRMessage::readOctet(msg,&this->sourceVersion.m_major);
		CDRMessage::readOctet(msg,&this->sourceVersion.m_minor);
		CDRMessage::readData(msg,this->sourceVendorId,2);
		CDRMessage::readData(msg,this->sourceGuidPrefix.value,12);
		//Is the final message?
		if(smh->submessageLength == 0)
			*last = true;
		logInfo(RTPS_MSG_IN,"SRC Participant is now: "<<this->sourceGuidPrefix,EPRO_BLUE);
		return true;
	}
	return false;
}





} /* namespace rtps */
} /* namespace eprosima */
