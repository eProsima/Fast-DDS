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

/*
 * RTPSReader.cpp
 *
*/

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/log/Log.h>
#include "FragmentedChangePitStop.h"

#include <fastrtps/rtps/reader/ReaderListener.h>
#include "CompoundReaderListener.h"

#include <typeinfo>

namespace eprosima {
namespace fastrtps{
namespace rtps {

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
	logInfo(RTPS_READER,"RTPSReader created correctly");
}

RTPSReader::~RTPSReader()
{
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

bool RTPSReader::reserveCache(CacheChange_t** change, uint32_t dataCdrSerializedSize)
{
	return mp_history->reserve_Cache(change, dataCdrSerializedSize);
}

void RTPSReader::releaseCache(CacheChange_t* change)
{
		return mp_history->release_Cache(change);
}

ReaderListener* RTPSReader::getListener(){
	return mp_listener;
}

bool RTPSReader::setListener(ReaderListener *target){
	CompoundReaderListener* readerlistener_cast = dynamic_cast<CompoundReaderListener*>(mp_listener);
	//Host is not of compound type, replace and move on
	if(readerlistener_cast == nullptr){
		//Not a valid cast, replace base listener
		mp_listener = target;
		return true;
	}else{
		//If we arrive here it means mp_listener is Infectable
		readerlistener_cast->attachListener(target);
		return true;
	}
}

CacheChange_t* RTPSReader::findCacheInFragmentedCachePitStop(const SequenceNumber_t& sequence_number,
        const GUID_t& writer_guid)
{
    return fragmentedChangePitStop_->find(sequence_number, writer_guid);
}

}
} /* namespace rtps */
} /* namespace eprosima */


