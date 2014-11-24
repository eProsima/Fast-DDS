/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSReader.cpp
 *
*/

#include "fastrtps/rtps/reader/RTPSReader.h"

#include "fastrtps/rtps/history/ReaderHistory.h"

#include "fastrtps/utils/RTPSLog.h"

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
		m_acceptMessagesFromUnkownWriters(true)

{
	const char* const METHOD_NAME = "RTPSReader";
	logInfo(RTPS_READER,"RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
	const char* const METHOD_NAME = "~RTPSReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid().entityId;);
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


}
} /* namespace rtps */
} /* namespace eprosima */


