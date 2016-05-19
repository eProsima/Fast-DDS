/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSReader.cpp
 *
*/

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/utils/RTPSLog.h>
#include "FragmentedChangePitStop.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {
static const char* const CLASS_NAME = "RTPSReader";

RTPSReader::RTPSReader(RTPSParticipantImpl*pimpl,GUID_t& guid,
		ReaderAttributes& att,ReaderHistory* hist,ReaderListener* rlisten):
		Endpoint(pimpl,guid,att.endpoint),
		mp_history(hist),
		mp_listener(rlisten),
		m_acceptMessagesToUnknownReaders(true),
		m_acceptMessagesFromUnkownWriters(true),
		m_expectsInlineQos(att.expectsInlineQos),
        fragmentedChangePitStop_(nullptr)

{
	mp_history->mp_reader = this;
    mp_history->mp_mutex = mp_mutex;
    fragmentedChangePitStop_ = new FragmentedChangePitStop(this);
	const char* const METHOD_NAME = "RTPSReader";
	logInfo(RTPS_READER,"RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
	const char* const METHOD_NAME = "~RTPSReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid().entityId;);
    delete fragmentedChangePitStop_;
    mp_history->mp_reader = nullptr;
    mp_history->mp_mutex = nullptr;
}

bool RTPSReader::acceptMsgDirectedTo(EntityId_t& entityId)
{
	if(entityId == m_guid.entityId)
		return true;
	if(m_acceptMessagesToUnknownReaders && entityId == c_EntityId_Unknown)
		return true;
	else
		return false;
}

bool RTPSReader::reserveCache(CacheChange_t** change)
{
	return mp_history->reserve_Cache(change);
}

void RTPSReader::releaseCache(CacheChange_t* change)
{
		return mp_history->release_Cache(change);
}

CacheChange_t* RTPSReader::findCacheInFragmentedCachePitStop(const SequenceNumber_t& sequence_number,
        const GUID_t& writer_guid)
{
    return fragmentedChangePitStop_->find(sequence_number, writer_guid);
}

}
} /* namespace rtps */
} /* namespace eprosima */


