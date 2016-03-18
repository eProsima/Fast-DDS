/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MessageReceiver.cpp
 *
 */

#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastrtps/rtps/resources/ListenResource.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>

#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>

#include <fastrtps/rtps/reader/ReaderListener.h>

#include "../participant/RTPSParticipantImpl.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <limits>
#include <cassert>


#include <fastrtps/utils/RTPSLog.h>

#define IDSTRING "(ID:"<<this->mp_threadListen->m_ID<<") "<<

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "MessageReceiver";


MessageReceiver::MessageReceiver(uint32_t rec_buffer_size):
												m_rec_msg(rec_buffer_size),
												mp_change(nullptr)
{
	const char* const METHOD_NAME = "MessageReceiver";
	destVersion = c_ProtocolVersion;
	sourceVersion = c_ProtocolVersion;
	set_VendorId_Unknown(sourceVendorId);
	sourceGuidPrefix = c_GuidPrefix_Unknown;
	destGuidPrefix = c_GuidPrefix_Unknown;
	haveTimestamp = false;
	timestamp = c_TimeInvalid;

	defUniLoc.kind = LOCATOR_KIND_UDPv4;
	LOCATOR_ADDRESS_INVALID(defUniLoc.address);
	defUniLoc.port = LOCATOR_PORT_INVALID;
	mp_threadListen = nullptr;
	logInfo(RTPS_MSG_IN,"Created with CDRMessage of size: "<<m_rec_msg.max_size,C_BLUE);
	uint16_t max_payload = ((uint32_t)std::numeric_limits<uint16_t>::max() < rec_buffer_size) ? std::numeric_limits<uint16_t>::max() : (uint16_t)rec_buffer_size;
	mp_change = new CacheChange_t(max_payload, true);
	//cout << "MESSAGE RECEIVER CREATED WITH MAX SIZE: " << mp_change->serializedPayload.max_size << endl;
}

MessageReceiver::~MessageReceiver()
{
	const char* const METHOD_NAME = "~MessageReceiver";
	this->m_ParamList.deleteParams();
	delete(mp_change);
	logInfo(RTPS_MSG_IN,"",C_BLUE);
}

void MessageReceiver::reset(){
	destVersion = c_ProtocolVersion;
	sourceVersion = c_ProtocolVersion;
	set_VendorId_Unknown(sourceVendorId);
	sourceGuidPrefix = c_GuidPrefix_Unknown;
	destGuidPrefix = c_GuidPrefix_Unknown;
	haveTimestamp = false;
	timestamp = c_TimeInvalid;

	unicastReplyLocatorList.clear();
	unicastReplyLocatorList.reserve(1);
	multicastReplyLocatorList.clear();
	multicastReplyLocatorList.reserve(1);
	Locator_t  loc;
	unicastReplyLocatorList.push_back(loc);
	multicastReplyLocatorList.push_back(defUniLoc);
	mp_change->kind = ALIVE;
	mp_change->sequenceNumber.high = 0;
	mp_change->sequenceNumber.low = 0;
	mp_change->writerGUID = c_Guid_Unknown;
	mp_change->serializedPayload.length = 0;
	mp_change->serializedPayload.pos = 0;
	for (uint8_t i = 0; i<16; ++i)
		mp_change->instanceHandle.value[i] = 0;
	mp_change->isRead = 0;
	mp_change->sourceTimestamp.seconds = 0;
	mp_change->sourceTimestamp.fraction = 0;
	mp_change->setFragmentSize(0);
	//cout << "MESSAGE RECEIVER RESEST WITH MAX SIZE: " << mp_change->serializedPayload.max_size << endl;
}

void MessageReceiver::processCDRMsg(const GuidPrefix_t& RTPSParticipantguidprefix,
		Locator_t* loc, CDRMessage_t*msg)
{

	const char* const METHOD_NAME = "processCDRMsg";

	if(msg->length < RTPSMESSAGE_HEADER_SIZE)
	{
		logWarning(RTPS_MSG_IN,IDSTRING"Received message too short, ignoring",C_BLUE);
		return;
	}

	this->reset();

	destGuidPrefix = RTPSParticipantguidprefix;
	unicastReplyLocatorList.begin()->kind = loc->kind;

	uint8_t n_start = 0;
	if(loc->kind == 1)
		n_start = 12;
	else if(loc->kind == 2)
		n_start = 0;
	else
	{
		logWarning(RTPS_MSG_IN,IDSTRING"Locator kind invalid",C_BLUE);
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
			logWarning(RTPS_MSG_IN,IDSTRING"SubMsg of invalid length ("<<submsgh.submessageLength
					<< ") with current msg position/length ("<<msg->pos << "/"<<msg->length << ")",C_BLUE);
			return;
		}
		if(submsgh.submessageLength == 0) //THIS IS THE LAST SUBMESSAGE
		{
			submsgh.submsgLengthLarger = msg->length - msg->pos;
		}
		valid = true;
		count++;
		switch(submsgh.submessageId)
		{
		case DATA:
		{
			if(this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,IDSTRING"Data Submsg ignored, DST is another RTPSParticipant",C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,IDSTRING"Data Submsg received, processing.",C_BLUE);
				valid = proc_Submsg_Data(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case DATA_FRAG:
			if (this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN, IDSTRING"DataFrag Submsg ignored, DST is another RTPSParticipant", C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN, IDSTRING"DataFrag Submsg received, processing.", C_BLUE);
				valid = proc_Submsg_DataFrag(msg, &submsgh, &last_submsg);
			}
			break;
		case GAP:
		{
			if(this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,IDSTRING"Gap Submsg ignored, DST is another RTPSParticipant...",C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,IDSTRING"Gap Submsg received, processing...",C_BLUE);
				valid = proc_Submsg_Gap(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case ACKNACK:
		{
			if(this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,IDSTRING"Acknack Submsg ignored, DST is another RTPSParticipant...",C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,IDSTRING"Acknack Submsg received, processing...",C_BLUE);
				valid = proc_Submsg_Acknack(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case NACK_FRAG:
		{
			if (this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN, IDSTRING"NackFrag Submsg ignored, DST is another RTPSParticipant...", C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN, IDSTRING"NackFrag Submsg received, processing...", C_BLUE);
				valid = proc_Submsg_NackFrag(msg, &submsgh, &last_submsg);
			}
			break;
		}
		case HEARTBEAT:
		{
			if(this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN,IDSTRING"HB Submsg ignored, DST is another RTPSParticipant...",C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN,IDSTRING"Heartbeat Submsg received, processing...",C_BLUE);
				valid = proc_Submsg_Heartbeat(msg,&submsgh,&last_submsg);
			}
			break;
		}
		case HEARTBEAT_FRAG:
		{
			if (this->destGuidPrefix != RTPSParticipantguidprefix)
			{
				msg->pos += submsgh.submessageLength;
				logInfo(RTPS_MSG_IN, IDSTRING"HBFrag Submsg ignored, DST is another RTPSParticipant...", C_BLUE);
			}
			else
			{
				logInfo(RTPS_MSG_IN, IDSTRING"HeartbeatFrag Submsg received, processing...", C_BLUE);
				valid = proc_Submsg_HeartbeatFrag(msg, &submsgh, &last_submsg);
			}
			break;
		}
		case PAD:
			logWarning(RTPS_MSG_IN,IDSTRING"PAD messages not yet implemented, ignoring",C_BLUE);
			msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_DST:
			logInfo(RTPS_MSG_IN,IDSTRING"InfoDST message received, processing...",C_BLUE);
			valid = proc_Submsg_InfoDST(msg,&submsgh,&last_submsg);
			//				pWarning("Info DST messages not yet implemented"<<endl);
			//				msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_SRC:
			logInfo(RTPS_MSG_IN,IDSTRING"InfoSRC message received, processing...",C_BLUE);
			valid = proc_Submsg_InfoSRC(msg,&submsgh,&last_submsg);
			//			pWarning("Info SRC messages not yet implemented"<<endl);
			//			msg->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
			break;
		case INFO_TS:
		{
			logInfo(RTPS_MSG_IN,IDSTRING"InfoTS Submsg received, processing...",C_BLUE);
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

bool MessageReceiver::checkRTPSHeader(CDRMessage_t*msg) //check and proccess the RTPS Header
{
	const char* const METHOD_NAME = "checkRTPSHeader";

	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
			msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
	{
		logInfo(RTPS_MSG_IN,IDSTRING"Msg received with no RTPS in header, ignoring...",C_BLUE);
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
		logWarning(RTPS_MSG_IN,IDSTRING"Major RTPS Version not supported",C_BLUE);
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
		logWarning(RTPS_MSG_IN,IDSTRING"SubmessageHeader too short");
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
    boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());

    // Reset param list
    m_ParamList.deleteParams();

	//READ and PROCESS
	if(smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
	{
		logInfo(RTPS_MSG_IN,IDSTRING"Too short submessage received, ignoring",C_BLUE);
		return false;
	}
	//Fill flags bool values
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool inlineQosFlag = smh->flags & BIT(1) ? true : false;
	bool dataFlag = smh->flags & BIT(2) ? true : false;
	bool keyFlag = smh->flags & BIT(3) ? true : false;
	if(keyFlag && dataFlag)
	{
		logWarning(RTPS_MSG_IN,IDSTRING"Message received with Data and Key Flag set, ignoring");
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

	RTPSReader* firstReader = nullptr;
	if(mp_threadListen->m_assocReaders.empty())
	{
		logWarning(RTPS_MSG_IN,IDSTRING"Data received in locator: "<<mp_threadListen->getListenLocators()<< ", when NO readers are listening",C_BLUE);
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
	if(firstReader == nullptr) //Reader not found
	{
		logWarning(RTPS_MSG_IN,IDSTRING"No Reader in this Locator ("<<mp_threadListen->getListenLocators()<< ")"
				" accepts this message (directed to: " <<readerID << ")",C_BLUE);
		return false;
	}
	//FOUND THE READER.
	//We ask the reader for a cachechange to store the information.
	CacheChange_t* ch = mp_change;
	ch->writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&ch->writerGUID.entityId);

	//Get sequence number
	CDRMessage::readSequenceNumber(msg,&ch->sequenceNumber);

	if(ch->sequenceNumber <= SequenceNumber_t(0, 0) || (ch->sequenceNumber.high == -1 && ch->sequenceNumber.low == 0)) //message invalid //TODO make faster
	{
		logWarning(RTPS_MSG_IN,IDSTRING"Invalid message received, bad sequence Number",C_BLUE);
		return false;
	}

	//Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
	if(octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
		msg->pos += (octetsToInlineQos-RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);

	int32_t inlineQosSize = 0;

	if(inlineQosFlag)
	{
		inlineQosSize = ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch);

		if(inlineQosSize <= 0)
		{
			logInfo(RTPS_MSG_IN,IDSTRING"SubMessage Data ERROR, Inline Qos ParameterList error");
			//firstReader->releaseCache(ch);
			return false;
		}

	}

	if(dataFlag || keyFlag)
	{
		uint32_t payload_size;
		if(smh->submessageLength>0)
			payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE+octetsToInlineQos+inlineQosSize);
		else
			payload_size = smh->submsgLengthLarger;

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
                logWarning(RTPS_MSG_IN,IDSTRING"Serialized Payload larger than maximum allowed size "
                        "("<<payload_size-2-2<<"/"<<ch->serializedPayload.max_size<<")",C_BLUE);
                //firstReader->releaseCache(ch);
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
				logError(RTPS_MSG_IN,IDSTRING"Bad encapsulation for KeyHash and status parameter list");
				return false;
			}
			//uint32_t param_size;
			if(ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch) <= 0)
			{
				logInfo(RTPS_MSG_IN,IDSTRING"SubMessage Data ERROR, keyFlag ParameterList",C_BLUE);
				return false;
			}
			msg->msg_endian = previous_endian;
		}
	}
	//Is the final message?
	if(smh->submessageLength == 0)
		*last = true;

    // Set sourcetimestamp
    if(haveTimestamp)
        ch->sourceTimestamp = this->timestamp;


	//FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.
	logInfo(RTPS_MSG_IN,IDSTRING"from Writer " << ch->writerGUID << "; possible RTPSReaders: "<<mp_threadListen->m_assocReaders.size(),C_BLUE);
	//Look for the correct reader to add the change
	for(std::vector<RTPSReader*>::iterator it = mp_threadListen->m_assocReaders.begin();
			it != mp_threadListen->m_assocReaders.end(); ++it)
	{
		if((*it)->acceptMsgDirectedTo(readerID))
		{
            (*it)->processDataMsg(ch);
		}
	}

	logInfo(RTPS_MSG_IN,IDSTRING"Sub Message DATA processed",C_BLUE);
	return true;
}

bool MessageReceiver::proc_Submsg_DataFrag(CDRMessage_t* msg, SubmessageHeader_t* smh, bool* last)
{
	const char* const METHOD_NAME = "proc_Submsg_DataFrag";
	boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());

	// Reset param list
	m_ParamList.deleteParams();

	//READ and PROCESS
	if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
	{
		logInfo(RTPS_MSG_IN, IDSTRING"Too short submessage received, ignoring", C_BLUE);
		return false;
	}
	
	//Fill flags bool values
	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	bool inlineQosFlag = smh->flags & BIT(1) ? true : false;
	bool keyFlag = smh->flags & BIT(2) ? true : false;

	//Assign message endianness
	if (endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	//Extra flags don't matter now. Avoid those bytes
	msg->pos += 2;

	int16_t octetsToInlineQos;
	CDRMessage::readInt16(msg, &octetsToInlineQos); //it should be 16 in this implementation

	//reader and writer ID
	EntityId_t readerID;
	CDRMessage::readEntityId(msg, &readerID);

	//WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:
	if (mp_threadListen->m_assocReaders.empty())
	{
		logWarning(RTPS_MSG_IN, IDSTRING"Data received in locator: " << mp_threadListen->getListenLocators() << ", when NO readers are listening", C_BLUE);
		return false;
	}

	RTPSReader* firstReader = nullptr;
	for (std::vector<RTPSReader*>::iterator it = mp_threadListen->m_assocReaders.begin();
		it != mp_threadListen->m_assocReaders.end(); ++it)
	{
		if ((*it)->acceptMsgDirectedTo(readerID)) //add
		{
			firstReader = *it;
			break;
		}
	}

	if (firstReader == nullptr) //Reader not found
	{
		logWarning(RTPS_MSG_IN, IDSTRING"No Reader in this Locator (" << mp_threadListen->getListenLocators() << ")"
			" accepts this message (directed to: " << readerID << ")", C_BLUE);
		return false;
	}

	//FOUND THE READER.
	//We ask the reader for a cachechange to store the information.
	CacheChange_t* ch = mp_change;
	ch->writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg, &ch->writerGUID.entityId);
	
	//Get sequence number
	CDRMessage::readSequenceNumber(msg, &ch->sequenceNumber);

	if (ch->sequenceNumber.to64long() <= 0 || (ch->sequenceNumber.high == -1 && ch->sequenceNumber.low == 0)) //message invalid //TODO make faster
	{
		logWarning(RTPS_MSG_IN, IDSTRING"Invalid message received, bad sequence Number", C_BLUE);
		return false;
	}

	// READ FRAGMENT NUMBER
	uint32_t fragmentStartingNum;
	CDRMessage::readUInt32(msg, &fragmentStartingNum);

	// READ FRAGMENTSINSUBMESSAGE
	uint16_t fragmentsInSubmessage;
	CDRMessage::readUInt16(msg, &fragmentsInSubmessage);

	// READ FRAGMENTSIZE
	uint16_t fragmentSize;
	CDRMessage::readUInt16(msg, &fragmentSize);

	// READ SAMPLESIZE
	uint32_t sampleSize;
	CDRMessage::readUInt32(msg, &sampleSize);

	//Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
	if (octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG)
		msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG);

	int32_t inlineQosSize = 0;

	if (inlineQosFlag)
	{
		inlineQosSize = ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch);

		if (inlineQosSize <= 0)
		{
			logInfo(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, Inline Qos ParameterList error");
			//firstReader->releaseCache(ch);
			return false;
		}
	}

	uint32_t payload_size;
	if (smh->submessageLength>0)
		payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE + octetsToInlineQos + inlineQosSize);
	else
		payload_size = smh->submsgLengthLarger;

	// Validations??? XXX TODO

    // Only read encapsulation when is the first fragment.
    // Rest of fragments don't have the encapsulation.
    if(fragmentStartingNum == 1)
    {
        msg->pos += 1;
        octet encapsulation = 0;
        CDRMessage::readOctet(msg, &encapsulation);

        ch->serializedPayload.encapsulation = (uint16_t)encapsulation;
        msg->pos += 2; //CDR Options, not used in this version
    }

	if (!keyFlag)
	{
		if (ch->serializedPayload.max_size >= payload_size)
		{
			ch->serializedPayload.length = payload_size;

			// TODO Mejorar el reubicar el vector de fragmentos.
			ch->setFragmentSize(fragmentSize);
			ch->getDataFragments()->clear();
			ch->getDataFragments()->resize(fragmentsInSubmessage, ChangeFragmentStatus_t::PRESENT);
			
			CDRMessage::readData(msg,
				ch->serializedPayload.data, payload_size);

			ch->kind = ALIVE;
		}
		else
		{
			logWarning(RTPS_MSG_IN, IDSTRING"Serialized Payload larger than maximum allowed size "
				"(" << payload_size << "/" << ch->serializedPayload.max_size << ")", C_BLUE);
			//firstReader->releaseCache(ch);
			return false;
		}
	}
	else if (keyFlag)
	{
		/* XXX TODO
		Endianness_t previous_endian = msg->msg_endian;
		if (ch->serializedPayload.encapsulation == PL_CDR_BE)
			msg->msg_endian = BIGEND;
		else if (ch->serializedPayload.encapsulation == PL_CDR_LE)
			msg->msg_endian = LITTLEEND;
		else
		{
			logError(RTPS_MSG_IN, IDSTRING"Bad encapsulation for KeyHash and status parameter list");
			return false;
		}
		//uint32_t param_size;
		if (ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch) <= 0)
		{
			logInfo(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, keyFlag ParameterList", C_BLUE);
			return false;
		}
		msg->msg_endian = previous_endian;
		*/
	}

	//Is the final message?
	if (smh->submessageLength == 0)
		*last = true;

	// Set sourcetimestamp
	if (haveTimestamp)
		ch->sourceTimestamp = this->timestamp;

	//FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.
	logInfo(RTPS_MSG_IN, IDSTRING"from Writer " << ch->writerGUID << "; possible RTPSReaders: " << mp_threadListen->m_assocReaders.size(), C_BLUE);
	//Look for the correct reader to add the change
	for (std::vector<RTPSReader*>::iterator it = mp_threadListen->m_assocReaders.begin();
		it != mp_threadListen->m_assocReaders.end(); ++it)
	{
		if ((*it)->acceptMsgDirectedTo(readerID))
		{
			(*it)->processDataFragMsg(ch, sampleSize, fragmentStartingNum - 1);
		}
	}

	logInfo(RTPS_MSG_IN, IDSTRING"Sub Message DATA processed", C_BLUE);

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

	GUID_t readerGUID, writerGUID;
	readerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg,&readerGUID.entityId);
	writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg,&writerGUID.entityId);
	SequenceNumber_t firstSN, lastSN;
	CDRMessage::readSequenceNumber(msg,&firstSN);
	CDRMessage::readSequenceNumber(msg,&lastSN);
	if(lastSN<firstSN)
	{
		logInfo(RTPS_MSG_IN,IDSTRING"HB Received with lastSN < firstSN, ignoring",C_BLUE);
		return false;
	}
	uint32_t HBCount;
	CDRMessage::readUInt32(msg,&HBCount);

    boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());
	//Look for the correct reader and writers:
	for(std::vector<RTPSReader*>::iterator it = mp_threadListen->m_assocReaders.begin();
			it != mp_threadListen->m_assocReaders.end(); ++it)
	{
		if((*it)->acceptMsgDirectedTo(readerGUID.entityId))
		{
            (*it)->processHeartbeatMsg(writerGUID, HBCount, firstSN, lastSN, finalFlag, livelinessFlag);
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

    boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());
	//Look for the correct writer to use the acknack
	for(std::vector<RTPSWriter*>::iterator it=mp_threadListen->m_assocWriters.begin();
			it!=mp_threadListen->m_assocWriters.end();++it)
	{
        //Look for the readerProxy the acknack is from
        boost::lock_guard<boost::recursive_mutex> guardW(*(*it)->getMutex());

		if((*it)->getGuid() == writerGUID)
		{
			if((*it)->getAttributes()->reliabilityKind == RELIABLE)
			{
				StatefulWriter* SF = (StatefulWriter*)(*it);
				
				for(auto rit = SF->matchedReadersBegin();rit!=SF->matchedReadersEnd();++rit)
				{
                    boost::lock_guard<boost::recursive_mutex> guardReaderProxy(*(*rit)->mp_mutex);

					if((*rit)->m_att.guid == readerGUID )
					{
						if((*rit)->m_lastAcknackCount < Ackcount)
						{
							(*rit)->m_lastAcknackCount = Ackcount;
							(*rit)->acked_changes_set(SNSet.base);
							std::vector<SequenceNumber_t> set_vec = SNSet.get_set();
							(*rit)->requested_changes_set(set_vec);
							if(!(*rit)->m_isRequestedChangesEmpty || !finalFlag)
							{
								(*rit)->mp_nackResponse->restart_timer();
							}

                            if(SF->getAttributes()->durabilityKind == VOLATILE)
                            {
                                // Clean history.
                                // TODO Change mechanism
                                SF->clean_history();
                            }

						}
						break;
					}
				}
				return true;
			}
			else
			{
				logInfo(RTPS_MSG_IN,IDSTRING"Acknack msg to NOT stateful writer ",C_BLUE);
				return false;
			}
		}
	}
	logInfo(RTPS_MSG_IN,IDSTRING"Acknack msg to UNKNOWN writer (I loooked through "
			<< mp_threadListen->m_assocWriters.size() << " writers in this ListenResource)",C_BLUE);
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
	if(gapStart <= SequenceNumber_t(0, 0))
		return false;

    boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());
	for(std::vector<RTPSReader*>::iterator it=mp_threadListen->m_assocReaders.begin();
			it!=mp_threadListen->m_assocReaders.end();++it)
	{
		if((*it)->acceptMsgDirectedTo(readerGUID.entityId))
		{
            (*it)->processGapMsg(writerGUID, gapStart, gapList);
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
		logInfo(RTPS_MSG_IN,IDSTRING"DST RTPSParticipant is now: "<< this->destGuidPrefix,C_BLUE);
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
		logInfo(RTPS_MSG_IN,IDSTRING"SRC RTPSParticipant is now: "<<this->sourceGuidPrefix,C_BLUE);
		return true;
	}
	return false;
}

bool MessageReceiver::proc_Submsg_NackFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last) {
	
	const char* const METHOD_NAME = "proc_Submsg_NackFrag";

	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//Assign message endianness
	if (endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	GUID_t readerGUID, writerGUID;
	readerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg, &readerGUID.entityId);
	writerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg, &writerGUID.entityId);

	SequenceNumber_t writerSN;
	CDRMessage::readSequenceNumber(msg, &writerSN);

	FragmentNumberSet_t fnState;
	CDRMessage::readFragmentNumberSet(msg, &fnState);

	uint32_t Ackcount;
	CDRMessage::readUInt32(msg, &Ackcount);

	if (smh->submessageLength == 0)
		*last = true;

	// XXX TODO VALIDATE?

	boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());
	//Look for the correct writer to use the acknack
	for (std::vector<RTPSWriter*>::iterator it = mp_threadListen->m_assocWriters.begin();
		it != mp_threadListen->m_assocWriters.end(); ++it)
	{
		//Look for the readerProxy the acknack is from
		boost::lock_guard<boost::recursive_mutex> guardW(*(*it)->getMutex());
		/* XXX TODO PROCESS
		if ((*it)->getGuid() == writerGUID)
		{
			if ((*it)->getAttributes()->reliabilityKind == RELIABLE)
			{
				StatefulWriter* SF = (StatefulWriter*)(*it);

				for (auto rit = SF->matchedReadersBegin(); rit != SF->matchedReadersEnd(); ++rit)
				{
					boost::lock_guard<boost::recursive_mutex> guardReaderProxy(*(*rit)->mp_mutex);

					if ((*rit)->m_att.guid == readerGUID)
					{
						if ((*rit)->m_lastAcknackCount < Ackcount)
						{
							(*rit)->m_lastAcknackCount = Ackcount;
							(*rit)->acked_changes_set(SNSet.base);
							std::vector<SequenceNumber_t> set_vec = SNSet.get_set();
							(*rit)->requested_changes_set(set_vec);
							if (!(*rit)->m_isRequestedChangesEmpty || !finalFlag)
							{
								(*rit)->mp_nackResponse->restart_timer();
							}

							if (SF->getAttributes()->durabilityKind == VOLATILE)
							{
								// Clean history.
								// TODO Change mechanism
								SF->clean_history();
							}

						}
						break;
					}
				}
				return true;
			}
			else
			{
				logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to NOT stateful writer ", C_BLUE);
				return false;
			}
		}
		*/
	}
	logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to UNKNOWN writer (I looked through "
		<< mp_threadListen->m_assocWriters.size() << " writers in this ListenResource)", C_BLUE);
	return false;
}

bool MessageReceiver::proc_Submsg_HeartbeatFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last) {
	//const char* const METHOD_NAME = "proc_Submsg_HeartbeatFrag";

	bool endiannessFlag = smh->flags & BIT(0) ? true : false;
	//Assign message endianness
	if (endiannessFlag)
		msg->msg_endian = LITTLEEND;
	else
		msg->msg_endian = BIGEND;

	GUID_t readerGUID, writerGUID;
	readerGUID.guidPrefix = destGuidPrefix;
	CDRMessage::readEntityId(msg, &readerGUID.entityId);
	writerGUID.guidPrefix = sourceGuidPrefix;
	CDRMessage::readEntityId(msg, &writerGUID.entityId);

	SequenceNumber_t writerSN;
	CDRMessage::readSequenceNumber(msg, &writerSN);

	FragmentNumber_t lastFN;
	CDRMessage::readUInt32(msg, (uint32_t*)&lastFN);

	uint32_t HBCount;
	CDRMessage::readUInt32(msg, &HBCount);

	// XXX TODO VALIDATE DATA?

	boost::lock_guard<boost::mutex> guard(*this->mp_threadListen->getMutex());
	//Look for the correct reader and writers:
	for (std::vector<RTPSReader*>::iterator it = mp_threadListen->m_assocReaders.begin();
		it != mp_threadListen->m_assocReaders.end(); ++it)
	{
		/* XXX TODO PROCESS
		if ((*it)->acceptMsgDirectedTo(readerGUID.entityId))
		{
			(*it)->processHeartbeatMsg(writerGUID, HBCount, firstSN, lastSN, finalFlag, livelinessFlag);
		}
		*/
	}

	//Is the final message?
	if (smh->submessageLength == 0)
		*last = true;
	return true;
}


}
} /* namespace rtps */
} /* namespace eprosima */
