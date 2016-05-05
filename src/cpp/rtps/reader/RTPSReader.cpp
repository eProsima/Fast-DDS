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

#include <fastrtps/rtps/reader/ReaderListener.h>

#include <typeinfo>

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
		m_expectsInlineQos(att.expectsInlineQos)

{
	mp_history->mp_reader = this;
    mp_history->mp_mutex = mp_mutex;
	const char* const METHOD_NAME = "RTPSReader";
	logInfo(RTPS_READER,"RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
	const char* const METHOD_NAME = "~RTPSReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid().entityId;);
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

bool RTPSReader::setListener(ReaderListener *target){
	//Check if the Reader has an infectable ReaderListener
	//Add target as slave in case it has
	//Free the old one and replace it with target if not
	InfectableReaderListener* readerlistener_cast= nullptr;
	readerlistener_cast= dynamic_cast<InfectableReaderListener*>(target);
	//Host is not Infectable, replace and move on
	if(readerlistener_cast == nullptr){
		delete mp_listener;
		mp_listener = target;	
		return true;
	}else{
		//If we arrive here it means mp_listener is Infectable
		readerlistener_cast->attachListener(target);
	}
	return false;
}


}
} /* namespace rtps */
} /* namespace eprosima */


