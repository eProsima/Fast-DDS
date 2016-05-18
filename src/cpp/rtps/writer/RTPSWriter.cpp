/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file RTPSWriter.cpp
 *
 */

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/utils/RTPSLog.h>
#include "../participant/RTPSParticipantImpl.h"

using namespace eprosima::fastrtps::rtps;

static const char* const CLASS_NAME = "RTPSWriter";

RTPSWriter::RTPSWriter(RTPSParticipantImpl* impl,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    Endpoint(impl,guid,att.endpoint),
    m_pushMode(true),
    //TODO 65536 put in constant or macro. It is max size of udp packet.
    m_cdrmessages(impl->getAttributes().sendSocketBufferSize > 65504 ? 65504 : impl->getAttributes().sendSocketBufferSize),
    m_livelinessAsserted(false),
    mp_unsetChangesNotEmpty(nullptr),
    mp_history(hist),
    mp_listener(listen),
    is_async_(att.mode == SYNCHRONOUS_WRITER ? false : true)
{
    const char* const METHOD_NAME = "RTPSWriter";
    mp_history->mp_writer = this;
    mp_history->mp_mutex = mp_mutex;
    this->init_header();
    logInfo(RTPS_WRITER,"RTPSWriter created");
}

void RTPSWriter::init_header()
{
    CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
    RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,m_guid.guidPrefix);
}


RTPSWriter::~RTPSWriter()
{
    const char* const METHOD_NAME = "~RTPSWriter";
    logInfo(RTPS_WRITER,"RTPSWriter destructor");

    // Deletion of the events has to be made in child destructor.

    mp_history->mp_writer = nullptr;
    mp_history->mp_mutex = nullptr;
}

CacheChange_t* RTPSWriter::new_change(ChangeKind_t changeKind,InstanceHandle_t handle)
{
    const char* const METHOD_NAME = "new_change";
    logInfo(RTPS_WRITER,"Creating new change");
    CacheChange_t* ch = nullptr;

    if(!mp_history->reserve_Cache(&ch))
    {
        logWarning(RTPS_WRITER,"Problem reserving Cache from the History");
        return nullptr;
    }

    ch->kind = changeKind;
    if(m_att.topicKind == WITH_KEY && !handle.isDefined())
    {
        logWarning(RTPS_WRITER,"Changes in KEYED Writers need a valid instanceHandle");
        //		if(mp_type->m_isGetKeyDefined)
        //		{
        //			mp_type->getKey(data,&ch->instanceHandle);
        //		}
        //		else
        //		{
        //			logWarning(RTPS_WRITER,"Get key function not defined";);
        //		}
    }
    ch->instanceHandle = handle;
    ch->writerGUID = m_guid;
    return ch;
}

SequenceNumber_t RTPSWriter::get_seq_num_min()
{
    CacheChange_t* change;
    if(mp_history->get_min_change(&change) && change!= nullptr)
        return change->sequenceNumber;
    else
        return c_SequenceNumber_Unknown;
}

SequenceNumber_t RTPSWriter::get_seq_num_max()
{
    CacheChange_t* change;
    if(mp_history->get_max_change(&change) && change!=nullptr)
        return change->sequenceNumber;
    else
        return c_SequenceNumber_Unknown;
}

uint32_t RTPSWriter::getTypeMaxSerialized()
{
    return mp_history->getTypeMaxSerialized();
}
