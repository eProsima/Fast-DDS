// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file MessageReceiver.cpp
 *
 */

#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>

#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>

#include <fastrtps/rtps/reader/ReaderListener.h>

#include "../participant/RTPSParticipantImpl.h"

#include <mutex>

#include <limits>
#include <cassert>


#include <fastrtps/log/Log.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


MessageReceiver::MessageReceiver(RTPSParticipantImpl* participant) : participant_(participant) {}

MessageReceiver::MessageReceiver(RTPSParticipantImpl* participant, uint32_t rec_buffer_size) :
    m_rec_msg(rec_buffer_size),
#if HAVE_SECURITY
    m_crypto_msg(rec_buffer_size),
#endif
    participant_(participant)
    {
    }

void MessageReceiver::init(uint32_t rec_buffer_size){
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
    logInfo(RTPS_MSG_IN,"Created with CDRMessage of size: "<<m_rec_msg.max_size);
    mMaxPayload_ = ((uint32_t)std::numeric_limits<uint16_t>::max() < rec_buffer_size) ? std::numeric_limits<uint16_t>::max() : (uint16_t)rec_buffer_size;
}

MessageReceiver::~MessageReceiver()
{
    logInfo(RTPS_MSG_IN,"");
    assert(AssociatedWriters.size() == 0);
    assert(AssociatedReaders.size() == 0);
}

void MessageReceiver::associateEndpoint(Endpoint *to_add){
    bool found = false;
    std::lock_guard<std::mutex> guard(mtx);
    if(to_add->getAttributes()->endpointKind == WRITER)
    {
        for(auto it = AssociatedWriters.begin(); it != AssociatedWriters.end(); ++it)
        {
            if( (*it) == (RTPSWriter*)to_add )
            {
                found = true;
                break;
            }
        }
        if(!found) AssociatedWriters.push_back((RTPSWriter*)to_add);
    }
    else
    {
        for(auto it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
        {
            if( (*it) == (RTPSReader*)to_add )
            {
                found = true;
                break;
            }
        }
        if(!found) AssociatedReaders.push_back((RTPSReader*)to_add);
    }
    return;
}
void MessageReceiver::removeEndpoint(Endpoint *to_remove){

    std::lock_guard<std::mutex> guard(mtx);
    if(to_remove->getAttributes()->endpointKind == WRITER){
        RTPSWriter* var = (RTPSWriter *)to_remove;
        for(auto it=AssociatedWriters.begin(); it !=AssociatedWriters.end(); ++it){
            if ((*it) == var){
                AssociatedWriters.erase(it);
                break;
            }
        }
    }else{
        RTPSReader *var = (RTPSReader *)to_remove;
        for(auto it=AssociatedReaders.begin(); it !=AssociatedReaders.end(); ++it){
            if ((*it) == var){
                AssociatedReaders.erase(it);
                break;
            }
        }
    }
    return;
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
}

void MessageReceiver::processCDRMsg(const GuidPrefix_t& RTPSParticipantguidprefix,
        Locator_t* loc, CDRMessage_t*msg)
{
    if(msg->length < RTPSMESSAGE_HEADER_SIZE)
    {
        logWarning(RTPS_MSG_IN,IDSTRING"Received message too short, ignoring");
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
        logWarning(RTPS_MSG_IN,IDSTRING"Locator kind invalid");
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
    {
        return;
    }

#if HAVE_SECURITY
    CDRMessage_t* auxiliary_buffer = &m_crypto_msg;
    CDRMessage::initCDRMsg(auxiliary_buffer);

    int decode_ret = participant_->security_manager().decode_rtps_message(*msg, *auxiliary_buffer, sourceGuidPrefix);

    if(decode_ret < 0)
        return;
    else if(decode_ret == 0)
    {
        // Swap
        std::swap(msg, auxiliary_buffer);
    }
#endif

    // Loop until there are no more submessages
    bool last_submsg = false;
    bool valid;
    int count = 0;
    SubmessageHeader_t submsgh; //Current submessage header
    //Pointers to different types of messages:

    while(msg->pos < msg->length)// end of the message
    {
        CDRMessage_t* submessage = msg;

#if HAVE_SECURITY
        CDRMessage::initCDRMsg(auxiliary_buffer);
        decode_ret = participant_->security_manager().decode_rtps_submessage(*msg, *auxiliary_buffer, sourceGuidPrefix);

        if(decode_ret < 0)
        {
            return;
        }
        else if(decode_ret == 0)
        {
            submessage = auxiliary_buffer;
        }
#endif

        //First 4 bytes must contain: ID | flags | octets to next header
        if(!readSubmessageHeader(submessage, &submsgh))
            return;

        if(submessage->pos + submsgh.submessageLength > submessage->length)
        {
            logWarning(RTPS_MSG_IN,IDSTRING"SubMsg of invalid length ("<<submsgh.submessageLength
                    << ") with current msg position/length (" << submessage->pos << "/" << submessage->length << ")");
            return;
        }
        if(submsgh.submessageLength == 0) //THIS IS THE LAST SUBMESSAGE
        {
            submsgh.submsgLengthLarger = submessage->length - submessage->pos;
        }
        valid = true;
        count++;
        switch(submsgh.submessageId)
        {
            case DATA:
                {
                    if(this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN,IDSTRING"Data Submsg ignored, DST is another RTPSParticipant");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN,IDSTRING"Data Submsg received, processing.");
                        valid = proc_Submsg_Data(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case DATA_FRAG:
                if (this->destGuidPrefix != RTPSParticipantguidprefix)
                {
                    submessage->pos += submsgh.submessageLength;
                    logInfo(RTPS_MSG_IN, IDSTRING"DataFrag Submsg ignored, DST is another RTPSParticipant");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING"DataFrag Submsg received, processing.");
                    valid = proc_Submsg_DataFrag(submessage, &submsgh, &last_submsg);
                }
                break;
            case GAP:
                {
                    if(this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN,IDSTRING"Gap Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN,IDSTRING"Gap Submsg received, processing...");
                        valid = proc_Submsg_Gap(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case ACKNACK:
                {
                    if(this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN,IDSTRING"Acknack Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN,IDSTRING"Acknack Submsg received, processing...");
                        valid = proc_Submsg_Acknack(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case NACK_FRAG:
                {
                    if (this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN, IDSTRING"NackFrag Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN, IDSTRING"NackFrag Submsg received, processing...");
                        valid = proc_Submsg_NackFrag(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case HEARTBEAT:
                {
                    if(this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN,IDSTRING"HB Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN,IDSTRING"Heartbeat Submsg received, processing...");
                        valid = proc_Submsg_Heartbeat(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case HEARTBEAT_FRAG:
                {
                    if (this->destGuidPrefix != RTPSParticipantguidprefix)
                    {
                        submessage->pos += submsgh.submessageLength;
                        logInfo(RTPS_MSG_IN, IDSTRING"HBFrag Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN, IDSTRING"HeartbeatFrag Submsg received, processing...");
                        valid = proc_Submsg_HeartbeatFrag(submessage, &submsgh, &last_submsg);
                    }
                    break;
                }
            case PAD:
                logWarning(RTPS_MSG_IN,IDSTRING"PAD messages not yet implemented, ignoring");
                submessage->pos += submsgh.submessageLength; //IGNORE AND CONTINUE
                break;
            case INFO_DST:
                logInfo(RTPS_MSG_IN,IDSTRING"InfoDST message received, processing...");
                valid = proc_Submsg_InfoDST(submessage, &submsgh, &last_submsg);
                break;
            case INFO_SRC:
                logInfo(RTPS_MSG_IN,IDSTRING"InfoSRC message received, processing...");
                valid = proc_Submsg_InfoSRC(submessage, &submsgh, &last_submsg);
                break;
            case INFO_TS:
                {
                    logInfo(RTPS_MSG_IN,IDSTRING"InfoTS Submsg received, processing...");
                    valid = proc_Submsg_InfoTS(submessage, &submsgh, &last_submsg);
                    break;
                }
            case INFO_REPLY:
                break;
            case INFO_REPLY_IP4:
                break;
            default:
                submessage->pos += submsgh.submessageLength; //ID NOT KNOWN. IGNORE AND CONTINUE
                break;
        }

        if(!valid || last_submsg)
        {
            break;
        }
    }
}

bool MessageReceiver::checkRTPSHeader(CDRMessage_t*msg) //check and proccess the RTPS Header
{

    if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
            msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
    {
        logInfo(RTPS_MSG_IN,IDSTRING"Msg received with no RTPS in header, ignoring...");
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
        logWarning(RTPS_MSG_IN,IDSTRING"Major RTPS Version not supported");
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


bool MessageReceiver::readSubmessageHeader(CDRMessage_t* msg, SubmessageHeader_t* smh)
{
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
    std::lock_guard<std::mutex> guard(mtx);

    //READ and PROCESS
    if(smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        logInfo(RTPS_MSG_IN,IDSTRING"Too short submessage received, ignoring");
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

    bool valid = true;
    int16_t octetsToInlineQos;
    valid &= CDRMessage::readInt16(msg, &octetsToInlineQos); //it should be 16 in this implementation

    //reader and writer ID
    EntityId_t readerID;
    valid &= CDRMessage::readEntityId(msg,&readerID);

    //WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:

    RTPSReader* firstReader = nullptr;
    if(AssociatedReaders.empty())
    {
        logWarning(RTPS_MSG_IN,IDSTRING"Data received when NO readers are listening");
        return false;
    }

    for(std::vector<RTPSReader*>::iterator it=AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
    {
        if((*it)->acceptMsgDirectedTo(readerID)) //add
        {
            firstReader = *it;
            break;
        }
    }
    if(firstReader == nullptr) //Reader not found
    {
        logWarning(RTPS_MSG_IN, IDSTRING"No Reader accepts this message (directed to: " <<readerID << ")");
        return false;
    }
    //FOUND THE READER.
    //We ask the reader for a cachechange to store the information.
    CacheChange_t ch;
    ch.serializedPayload.max_size = mMaxPayload_;
    ch.writerGUID.guidPrefix = sourceGuidPrefix;
    valid &= CDRMessage::readEntityId(msg,&ch.writerGUID.entityId);

    //Get sequence number
    valid &= CDRMessage::readSequenceNumber(msg,&ch.sequenceNumber);

    if (!valid){
        return false;
    }

    if(ch.sequenceNumber <= SequenceNumber_t(0, 0) || (ch.sequenceNumber.high == -1 && ch.sequenceNumber.low == 0)) //message invalid //TODO make faster
    {
        logWarning(RTPS_MSG_IN,IDSTRING"Invalid message received, bad sequence Number");
        return false;
    }

    //Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
    if(octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
    {
        msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
        if (msg->pos > msg->length)
        {
            logWarning(RTPS_MSG_IN, IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    int32_t inlineQosSize = 0;

    if(inlineQosFlag)
    {
        ParameterList_t parameter_list;
        inlineQosSize = ParameterList::readParameterListfromCDRMsg(msg, &parameter_list, &ch, false);

        if(inlineQosSize <= 0)
        {
            logInfo(RTPS_MSG_IN,IDSTRING"SubMessage Data ERROR, Inline Qos ParameterList error");
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

        if(dataFlag)
        {
            if(ch.serializedPayload.max_size >= payload_size && payload_size > 0)
            {
                ch.serializedPayload.data = &msg->buffer[msg->pos];
                ch.serializedPayload.length = payload_size;
                msg->pos += payload_size;
                ch.kind = ALIVE;
            }
            else
            {
                logWarning(RTPS_MSG_IN,IDSTRING"Serialized Payload value invalid or larger than maximum allowed size"
                        "(" <<payload_size <<"/"<< ch.serializedPayload.max_size<<")");
                return false;
            }
        }
        else if(keyFlag)
        {
            Endianness_t previous_endian = msg->msg_endian;
            if(ch.serializedPayload.encapsulation == PL_CDR_BE)
                msg->msg_endian = BIGEND;
            else if(ch.serializedPayload.encapsulation == PL_CDR_LE)
                msg->msg_endian = LITTLEEND;
            else
            {
                logError(RTPS_MSG_IN,IDSTRING"Bad encapsulation for KeyHash and status parameter list");
                return false;
            }
            //uint32_t param_size;
            ParameterList_t parameter_list;
            if(ParameterList::readParameterListfromCDRMsg(msg, &parameter_list, &ch, false) <= 0)
            {
                logInfo(RTPS_MSG_IN,IDSTRING"SubMessage Data ERROR, keyFlag ParameterList");
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
    {
        ch.sourceTimestamp = this->timestamp;
    }


    //FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.
    logInfo(RTPS_MSG_IN,IDSTRING"from Writer " << ch.writerGUID << "; possible RTPSReaders: "<<AssociatedReaders.size());
    //Look for the correct reader to add the change
    for(std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
    {
        if((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processDataMsg(&ch);
        }
    }

    //TODO(Ricardo) If a exception is thrown (ex, by fastcdr), this line is not executed -> segmentation fault
    ch.serializedPayload.data = nullptr;

    logInfo(RTPS_MSG_IN,IDSTRING"Sub Message DATA processed");
    return true;
}

bool MessageReceiver::proc_Submsg_DataFrag(CDRMessage_t* msg, SubmessageHeader_t* smh, bool* last)
{
    std::lock_guard<std::mutex> guard(mtx);

    //READ and PROCESS
    if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        logInfo(RTPS_MSG_IN, IDSTRING"Too short submessage received, ignoring");
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

    bool valid = true;
    int16_t octetsToInlineQos;
    valid &= CDRMessage::readInt16(msg, &octetsToInlineQos); //it should be 16 in this implementation

    //reader and writer ID
    EntityId_t readerID;
    valid &= CDRMessage::readEntityId(msg, &readerID);

    //WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:
    if(AssociatedReaders.empty())
    {
        logWarning(RTPS_MSG_IN, IDSTRING"Data received when NO readers are listening");
        return false;
    }

    RTPSReader* firstReader = nullptr;
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID)) //add
        {
            firstReader = *it;
            break;
        }
    }

    if (firstReader == nullptr) //Reader not found
    {
        logWarning(RTPS_MSG_IN, IDSTRING"No Reader accepts this message (directed to: " << readerID << ")");
        return false;
    }

    //FOUND THE READER.
    //We ask the reader for a cachechange to store the information.
    CacheChange_t ch;
    ch.serializedPayload.max_size = mMaxPayload_;
    ch.writerGUID.guidPrefix = sourceGuidPrefix;
    valid &= CDRMessage::readEntityId(msg, &ch.writerGUID.entityId);

    //Get sequence number
    valid &= CDRMessage::readSequenceNumber(msg, &ch.sequenceNumber);

    if (ch.sequenceNumber <= SequenceNumber_t())
    {
        logWarning(RTPS_MSG_IN, IDSTRING"Invalid message received, bad sequence Number");
        return false;
    }

    // READ FRAGMENT NUMBER
    uint32_t fragmentStartingNum;
    valid &= CDRMessage::readUInt32(msg, &fragmentStartingNum);

    // READ FRAGMENTSINSUBMESSAGE
    uint16_t fragmentsInSubmessage;
    valid &= CDRMessage::readUInt16(msg, &fragmentsInSubmessage);

    // READ FRAGMENTSIZE
    uint16_t fragmentSize;
    valid &= CDRMessage::readUInt16(msg, &fragmentSize);

    // READ SAMPLESIZE
    uint32_t sampleSize;
    valid &= CDRMessage::readUInt32(msg, &sampleSize);

    if(!valid){
        return false;
    }

    //Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
    if (octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG)
    {
        msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG);
        if (msg->pos > msg->length)
        {
            logWarning(RTPS_MSG_IN, IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    int32_t inlineQosSize = 0;

    if (inlineQosFlag)
    {
        ParameterList_t parameter_list;
        inlineQosSize = ParameterList::readParameterListfromCDRMsg(msg, &parameter_list, &ch, false);

        if (inlineQosSize <= 0)
        {
            logInfo(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, Inline Qos ParameterList error");
            return false;
        }
    }

    uint32_t payload_size;
    if (smh->submessageLength>0)
        payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE + octetsToInlineQos + inlineQosSize);
    else
        payload_size = smh->submsgLengthLarger;

    // Validations??? XXX TODO

    if (!keyFlag)
    {
        if (ch.serializedPayload.max_size >= payload_size && payload_size > 0)
        {
            ch.serializedPayload.length = payload_size;

            // TODO Mejorar el reubicar el vector de fragmentos.
            ch.setFragmentSize(fragmentSize);
            ch.getDataFragments()->clear();
            ch.getDataFragments()->resize(fragmentsInSubmessage, ChangeFragmentStatus_t::PRESENT);

            ch.serializedPayload.data = &msg->buffer[msg->pos];
            ch.serializedPayload.length = payload_size;
            msg->pos += payload_size;

            ch.kind = ALIVE;
        }
        else
        {
            logWarning(RTPS_MSG_IN, IDSTRING"Serialized Payload value invalid or larger than maximum allowed size "
                    "(" << payload_size << "/" << ch.serializedPayload.max_size << ")");
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
        if (ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch, false) <= 0)
        {
        logInfo(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, keyFlag ParameterList");
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
        ch.sourceTimestamp = this->timestamp;

    //FIXME: DO SOMETHING WITH PARAMETERLIST CREATED.
    logInfo(RTPS_MSG_IN, IDSTRING"from Writer " << ch.writerGUID << "; possible RTPSReaders: " << AssociatedReaders.size());
    //Look for the correct reader to add the change
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processDataFragMsg(&ch, sampleSize, fragmentStartingNum);
        }
    }

    ch.serializedPayload.data = nullptr;

    logInfo(RTPS_MSG_IN, IDSTRING"Sub Message DATA_FRAG processed");

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

    GUID_t readerGUID, writerGUID;
    readerGUID.guidPrefix = destGuidPrefix;
    CDRMessage::readEntityId(msg,&readerGUID.entityId);
    writerGUID.guidPrefix = sourceGuidPrefix;
    CDRMessage::readEntityId(msg,&writerGUID.entityId);
    SequenceNumber_t firstSN, lastSN;
    CDRMessage::readSequenceNumber(msg,&firstSN);
    CDRMessage::readSequenceNumber(msg,&lastSN);
    if(lastSN < firstSN && lastSN != SequenceNumber_t(0, 0))
    {
        logWarning(RTPS_MSG_IN, IDSTRING"Invalid Heartbeat received (" << firstSN << ") - (" <<
                lastSN << "), ignoring");
        return false;
    }
    uint32_t HBCount;
    CDRMessage::readUInt32(msg,&HBCount);

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct reader and writers:
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
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

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct writer to use the acknack
    for (std::vector<RTPSWriter*>::iterator it = AssociatedWriters.begin();
            it != AssociatedWriters.end(); ++it)
    {
        if((*it)->getGuid() == writerGUID)
        {
            if((*it)->getAttributes()->reliabilityKind == RELIABLE)
            {
                StatefulWriter* SF = (StatefulWriter*)(*it);
                SF->process_acknack(readerGUID, Ackcount, SNSet, finalFlag);
                return true;
            }
            else
            {
                logInfo(RTPS_MSG_IN,IDSTRING"Acknack msg to NOT stateful writer ");
                return false;
            }
        }
    }
    logInfo(RTPS_MSG_IN,IDSTRING"Acknack msg to UNKNOWN writer (I loooked through "
            << AssociatedWriters.size() << " writers in this ListenResource)");
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

    std::lock_guard<std::mutex> guard(mtx);
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
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
        logInfo(RTPS_MSG_IN,IDSTRING"DST RTPSParticipant is now: "<< this->destGuidPrefix);
    }
    //Is the final message?
    if(smh->submessageLength == 0)
        *last = true;
    return true;
}

bool MessageReceiver::proc_Submsg_InfoSRC(CDRMessage_t* msg,SubmessageHeader_t* smh, bool* last)
{
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
        logInfo(RTPS_MSG_IN,IDSTRING"SRC RTPSParticipant is now: "<<this->sourceGuidPrefix);
        return true;
    }
    return false;
}

bool MessageReceiver::proc_Submsg_NackFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last) {


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

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct writer to use the acknack
    for (std::vector<RTPSWriter*>::iterator it = AssociatedWriters.begin();
            it != AssociatedWriters.end(); ++it)
    {
        //Look for the readerProxy the acknack is from
        std::lock_guard<std::recursive_mutex> guardW(*(*it)->getMutex());
        if ((*it)->getGuid() == writerGUID)
        {
            if ((*it)->getAttributes()->reliabilityKind == RELIABLE)
            {
                StatefulWriter* SF = (StatefulWriter*)(*it);

                for (auto rit = SF->matchedReadersBegin(); rit != SF->matchedReadersEnd(); ++rit)
                {
                    std::lock_guard<std::recursive_mutex> guardReaderProxy(*(*rit)->mp_mutex);

                    if ((*rit)->m_att.guid == readerGUID)
                    {
                        if ((*rit)->getLastNackfragCount() < Ackcount)
                        {
                            (*rit)->setLastNackfragCount(Ackcount);
                            // TODO Not doing Acknowledged.
                            if((*rit)->requested_fragment_set(writerSN, fnState))
                            {
                                (*rit)->mp_nackResponse->restart_timer();
                            }
                        }
                        break;
                    }
                }
                return true;
            }
            else
            {
                logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to NOT stateful writer ");
                return false;
            }
        }
    }
    logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to UNKNOWN writer (I looked through "
            << AssociatedWriters.size() << " writers in this ListenResource)");
    return false;
}

bool MessageReceiver::proc_Submsg_HeartbeatFrag(CDRMessage_t*msg, SubmessageHeader_t* smh, bool*last) {

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

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct reader and writers:
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin();
            it != AssociatedReaders.end(); ++it)
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
