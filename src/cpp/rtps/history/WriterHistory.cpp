/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterHistory.cpp
 *
 */

#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {


typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;

static const char* const CLASS_NAME = "WriterHistory";

WriterHistory::WriterHistory(const HistoryAttributes& att):
				History(att),
				mp_writer(nullptr)
{

}

WriterHistory::~WriterHistory()
{
	// TODO Auto-generated destructor stub
}

bool WriterHistory::add_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "add_change";

	if(mp_writer == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Writer with this History before adding any changes");
		return false;
	}

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(a_change->writerGUID != mp_writer->getGuid())
	{
		logError(RTPS_HISTORY,"Change writerGUID "<< a_change->writerGUID << " different than Writer GUID "<< mp_writer->getGuid());
		return false;
	}
	if(a_change->serializedPayload.length > m_att.payloadMaxSize)
	{
		logError(RTPS_HISTORY,"The Payload length is larger than the maximum payload size");
		return false;
	}
	++m_lastCacheChangeSeqNum;
	a_change->sequenceNumber = m_lastCacheChangeSeqNum;
	m_changes.push_back(a_change);
	logInfo(RTPS_HISTORY,"Change "<< a_change->sequenceNumber << " added with "<<a_change->serializedPayload.length<< " bytes");
	updateMaxMinSeqNum();

    mp_writer->unsent_change_added_to_history(a_change);

	return true;
}

bool WriterHistory::remove_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "remove_change";

	if(mp_writer == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Writer with this History before removing any changes");
		return false;
	}

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(a_change == nullptr)
	{
		logError(RTPS_HISTORY,"Pointer is not valid")
		return false;
	}
	if(a_change->writerGUID != mp_writer->getGuid())
	{
//		cout << "a change " << a_change->sequenceNumber<< endl;
//		cout << "a change "<< a_change->writerGUID << endl;
//		cout << "writer: "<< mp_writer->getGuid()<<endl;
		logError(RTPS_HISTORY,"Change writerGUID "<< a_change->writerGUID << " different than Writer GUID "<< mp_writer->getGuid());
		return false;
	}

	for(std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
            chit!=m_changes.end();++chit)
	{
		if((*chit)->sequenceNumber == a_change->sequenceNumber)
		{
			mp_writer->change_removed_by_history(a_change);
			m_changePool.release_Cache(a_change);
			m_changes.erase(chit);
			updateMaxMinSeqNum();
			return true;
		}
	}
	logWarning(RTPS_HISTORY,"SequenceNumber "<<a_change->sequenceNumber << " not found");
	return false;
}

bool WriterHistory::remove_change_g(CacheChange_t* a_change)
{
    return remove_change(a_change);
}

void WriterHistory::updateMaxMinSeqNum()
{
	if(m_changes.size()==0)
	{
		mp_minSeqCacheChange = mp_invalidCache;
		mp_maxSeqCacheChange = mp_invalidCache;
	}
	else
	{
		mp_minSeqCacheChange = m_changes.front();
		mp_maxSeqCacheChange = m_changes.back();
	}
}


bool WriterHistory::remove_min_change()
{
    const char* const METHOD_NAME = "remove_min_change";

	if(mp_writer == nullptr || mp_mutex == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Writer with this History before removing any changes");
		return false;
	}

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(m_changes.size() > 0 && remove_change(mp_minSeqCacheChange))
	{
		updateMaxMinSeqNum();
		return true;
	}
	else
		return false;
}

//TODO Hacer metodos de remove_all_changes. y hacer los metodos correspondientes en los writers y publishers.





}
} /* namespace rtps */
} /* namespace eprosima */
