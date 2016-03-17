/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file StatelessReader.cpp
 *             	
 */

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include "../participant/RTPSParticipantImpl.h"


#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread.hpp>

#include <cassert>

#define IDSTRING "(ID:"<< boost::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps::rtps;

static const char* const CLASS_NAME = "StatelessReader";

StatelessReader::~StatelessReader()
{
	const char* const METHOD_NAME = "~StatelessReader";
	logInfo(RTPS_READER,"Removing reader "<<this->getGuid());
}

StatelessReader::StatelessReader(RTPSParticipantImpl* pimpl,GUID_t& guid,
		ReaderAttributes& att,ReaderHistory* hist,ReaderListener* listen):
						RTPSReader(pimpl,guid,att,hist, listen)
{

}



bool StatelessReader::matched_writer_add(RemoteWriterAttributes& wdata)
{
	const char* const METHOD_NAME = "matched_writer_add";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(auto it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it).guid == wdata.guid)
			return false;
	}
	logInfo(RTPS_READER,"Writer " << wdata.guid << " added to "<<m_guid.entityId);
	m_matched_writers.push_back(wdata);
	m_acceptMessagesFromUnkownWriters = false;
	return true;
}
bool StatelessReader::matched_writer_remove(RemoteWriterAttributes& wdata)
{
	const char* const METHOD_NAME = "matched_writer_remove";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(auto it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it).guid == wdata.guid)
		{
			logInfo(RTPS_READER,"Writer " <<wdata.guid<< " removed from "<<m_guid.entityId);
			m_matched_writers.erase(it);
			return true;
		}
	}
	return false;
}

bool StatelessReader::matched_writer_is_matched(RemoteWriterAttributes& wdata)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(auto it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
	{
		if((*it).guid == wdata.guid)
		{
			return true;
		}
	}
	return false;
}

bool StatelessReader::change_received(CacheChange_t* change)
{
	if(mp_history->received_change(change))
	{
        boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);
		
		// If change is update, we know it is a fragmented cache change.
		auto it = change->getDataFragments()->begin();

		for (; it != change->getDataFragments()->end(); ++it)
		{
			if (*it == ChangeFragmentStatus_t::NOT_PRESENT)
				break;
		}

		if (it == change->getDataFragments()->end())
		{
			if (getListener() != nullptr)
			{
				lock.unlock();
				getListener()->onNewCacheChangeAdded((RTPSReader*)this, change);
				lock.lock();
			}
		}

		mp_history->postSemaphore();
		return true;
	}
	return false;
}

bool StatelessReader::change_updated(CacheChange_t* change)
{
	boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

	// If change is update, we know it is a fragmented cache change.
	auto it = change->getDataFragments()->begin();

	for (; it != change->getDataFragments()->end(); ++it)
	{
		if (*it == ChangeFragmentStatus_t::NOT_PRESENT)
			break;
	}

	if (it == change->getDataFragments()->end())
	{
		if (getListener() != nullptr)
		{
			lock.unlock();
			getListener()->onNewCacheChangeAdded((RTPSReader*)this, change);
			lock.lock();
		}
	}

	mp_history->postSemaphore();
	return true;
}

bool StatelessReader::nextUntakenCache(CacheChange_t** change,WriterProxy** /*wpout*/)
{
	//const char* const METHOD_NAME = "nextUntakenCache";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	return mp_history->get_min_change(change);
}


bool StatelessReader::nextUnreadCache(CacheChange_t** change,WriterProxy** /*wpout*/)
{
	const char* const METHOD_NAME = "nextUnreadCache";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	//m_reader_cache.sortCacheChangesBySeqNum();
	bool found = false;
	std::vector<CacheChange_t*>::iterator it;
	//TODO PROTEGER ACCESO A HISTORIA AQUI??? YO CREO QUE NO, YA ESTA EL READER PROTEGIDO
	for(it = mp_history->changesBegin();
			it!=mp_history->changesEnd();++it)
	{
		if(!(*it)->isRead)
		{
			found = true;
			break;
		}
	}
	if(found)
	{
		*change = *it;
		return true;
	}
	logInfo(RTPS_READER,"No Unread elements left");
	return false;
}


bool StatelessReader::change_removed_by_history(CacheChange_t* /*ch*/, WriterProxy* /*prox*/)
{
	return true;
}

bool StatelessReader::processDataMsg(CacheChange_t *change)
{
    const char* const METHOD_NAME = "processDataMsg";

    assert(change);

	boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(change->writerGUID))
    {
        logInfo(RTPS_MSG_IN,IDSTRING"Trying to add change " << change->sequenceNumber <<" TO reader: "<< getGuid().entityId,C_BLUE);

        CacheChange_t* change_to_add;

        if(reserveCache(&change_to_add)) //Reserve a new cache from the corresponding cache pool
        { 
            if (!change_to_add->copy(change))
            {
                logWarning(RTPS_MSG_IN,IDSTRING"Problem copying CacheChange, received data is: " << change->serializedPayload.length
                        << " bytes and max size in reader " << getGuid().entityId << " is " << change_to_add->serializedPayload.max_size, C_BLUE);
                releaseCache(change_to_add);
                return false;
            }
        }
        else
        {
            logError(RTPS_MSG_IN,IDSTRING"Problem reserving CacheChange in reader: " << getGuid().entityId, C_BLUE);
            return false;
        }

        lock.unlock(); // Next function has its own lock.
        if(!change_received(change_to_add))
        {
            logInfo(RTPS_MSG_IN,IDSTRING"MessageReceiver not add change "
                    <<change_to_add->sequenceNumber, C_BLUE);
            releaseCache(change_to_add);

            if(getGuid().entityId == c_EntityId_SPDPReader)
            {
                mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(change->writerGUID.guidPrefix);
            }
        }
    }

    return true;
}

bool StatelessReader::processDataFragMsg(CacheChange_t *incomingChange, uint32_t sampleSize, uint32_t fragmentStartingNum)
{
	const char* const METHOD_NAME = "processDataFragMsg";

	assert(incomingChange);

	boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

	if (acceptMsgFrom(incomingChange->writerGUID))
	{
		logInfo(RTPS_MSG_IN, IDSTRING"Trying to add change " << incomingChange->sequenceNumber.to64long() << " TO reader: " << getGuid().entityId, C_BLUE);
		
		// Look in History for a CacheChange with same writerSN y writerId
		CacheChange_t* existingChange = NULL;
		for (std::vector<CacheChange_t*>::iterator it = getHistory()->changesBegin(); it != getHistory()->changesEnd(); ++it) {
			CacheChange_t* current = *it;
			if (current->writerGUID.entityId == incomingChange->writerGUID.entityId &&
				current->sequenceNumber == incomingChange->sequenceNumber) {
				existingChange = current;
				break;
			}
		}

		if (existingChange != NULL) { // If found, merge with new CacheChange
			
			for (uint32_t count = fragmentStartingNum; count < fragmentStartingNum + incomingChange->getFragmentCount(); ++count)
			{
				if (count + 1 != existingChange->getFragmentCount())
				{
					memcpy(existingChange->serializedPayload.data + fragmentStartingNum * existingChange->getFragmentSize(),
						incomingChange->serializedPayload.data, incomingChange->getFragmentSize());
				}
				else
				{
					memcpy(existingChange->serializedPayload.data + fragmentStartingNum * existingChange->getFragmentSize(),
						incomingChange->serializedPayload.data, existingChange->serializedPayload.length - (count * existingChange->getFragmentSize()));
				}

				existingChange->getDataFragments()->at(count) = ChangeFragmentStatus_t::PRESENT;
			}

			lock.unlock(); // Next function has its own lock.
			if (!change_updated(existingChange))
			{
				logInfo(RTPS_MSG_IN, IDSTRING"MessageReceiver not add change " << existingChange->sequenceNumber.to64long(), C_BLUE);
				releaseCache(existingChange);
			}
		}
		else {
			// If not found, insert new CacheChange

			CacheChange_t* change_to_add;

			if (reserveCache(&change_to_add)) //Reserve a new cache from the corresponding cache pool
			{
				change_to_add->copy_not_memcpy(incomingChange);

				// The length of the serialized payload has to be sample size.
				change_to_add->serializedPayload.length = sampleSize;
				change_to_add->setFragmentSize(incomingChange->getFragmentSize());

				for (uint32_t count = fragmentStartingNum; count < fragmentStartingNum + incomingChange->getFragmentCount(); ++count)
				{
					if (count + 1 != change_to_add->getFragmentCount())
					{
						memcpy(change_to_add->serializedPayload.data + fragmentStartingNum * change_to_add->getFragmentSize(),
							incomingChange->serializedPayload.data, incomingChange->getFragmentSize());
					}
					else
					{
						memcpy(change_to_add->serializedPayload.data + fragmentStartingNum * change_to_add->getFragmentSize(),
							incomingChange->serializedPayload.data, change_to_add->serializedPayload.length - (count * change_to_add->getFragmentSize()));
					}

					change_to_add->getDataFragments()->at(count) = ChangeFragmentStatus_t::PRESENT;
				}


				lock.unlock(); // Next function has its own lock.
				if (!change_received(change_to_add))
				{
					logInfo(RTPS_MSG_IN, IDSTRING"MessageReceiver not add change " << change_to_add->sequenceNumber.to64long(), C_BLUE);
					releaseCache(change_to_add);
					if (getGuid().entityId == c_EntityId_SPDPReader)
					{
						mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(change_to_add->writerGUID.guidPrefix);
					}
				}
			}
			else
			{
				logError(RTPS_MSG_IN, IDSTRING"Problem reserving CacheChange in reader: " << getGuid().entityId, C_BLUE);
				return false;
			}
		}
	}

	return true;
}

bool StatelessReader::processHeartbeatMsg(GUID_t& /*writerGUID*/, uint32_t /*hbCount*/, SequenceNumber_t& /*firstSN*/,
        SequenceNumber_t& /*lastSN*/, bool /*finalFlag*/, bool /*livelinessFlag*/)
{
    return true;
}

bool StatelessReader::processGapMsg(GUID_t& /*writerGUID*/, SequenceNumber_t& /*gapStart*/, SequenceNumberSet_t& /*gapList*/)
{
    return true;
}

bool StatelessReader::acceptMsgFrom(GUID_t& writerId)
{
	if(this->m_acceptMessagesFromUnkownWriters)
	{
		return true;
	}
	else
	{
		if(writerId.entityId == this->m_trustedWriterEntityId)
			return true;

		for(auto it = m_matched_writers.begin();it!=m_matched_writers.end();++it)
		{
			if((*it).guid == writerId)
			{
				return true;
			}
		}
	}

	return false;
}
