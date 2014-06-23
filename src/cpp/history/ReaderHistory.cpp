/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.cpp
 *
 */

#include "eprosimartps/history/ReaderHistory.h"

#include "eprosimartps/Endpoint.h"
#include "eprosimartps/common/CacheChange.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/reader/RTPSReader.h"

namespace eprosima {
namespace rtps {

bool sort_ReaderHistoryCache(CacheChange_t*c1,CacheChange_t*c2)
{
	return c1->sequenceNumber < c2->sequenceNumber;
}

ReaderHistory::ReaderHistory(Endpoint* endp,
		uint32_t payload_max_size):
		History(endp,endp->getTopic().historyQos,endp->getTopic().resourceLimitsQos,payload_max_size),
		mp_minSeqCacheChange(mp_invalidCache),
		mp_maxSeqCacheChange(mp_invalidCache),
		mp_reader((RTPSReader*) endp),
		m_unreadCacheCount(0)

{
	// TODO Auto-generated constructor stub
}

ReaderHistory::~ReaderHistory()
{
	// TODO Auto-generated destructor stub
}

bool ReaderHistory::add_change(CacheChange_t* a_change)
{
	if(m_isHistoryFull)
	{
		pWarning("Attempting to add Data to Full WriterCache"<<endl;)
		return false;
	}
	//CHECK IF THE SAME CHANGE IS ALREADY IN THE HISTORY:
	if(a_change->sequenceNumber <= mp_maxSeqCacheChange)
	{
		for(std::vector<CacheChange_t*>::reverse_iterator it=m_changes.rbegin();it!=m_changes.rend();++it)
		{
			if((*it)->sequenceNumber == a_change->sequenceNumber &&
					(*it)->writerGUID == a_change->writerGUID)
			{
				pDebugInfo("Change (seqNum: "<< change->sequenceNumber.to64long()<< ") already in History (kind:"<<mp_Endpoint->getEndpointKind()<<")" << endl);
				return false;
			}
			if((*it)->writerGUID == a_change->writerGUID &&
					(*it)->sequenceNumber < a_change->sequenceNumber)
			{
				//SINCE THE ELEMENTS ARE ORDERED WE CAN STOP SEARCHING NOW
				//ALL REMAINING ELEMENTS WOULD BE LOWER THAN THE ONE WE ARE LOOKING FOR.
				break;
			}
		}
	}
	//NO KEY HISTORY
	if(mp_Endpoint->getTopic().topicKind == NO_KEY)
	{
		bool add = false;
		if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
		{
			add = true;
		}
		else if(m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
		{
			if(m_changes.size()<(size_t)m_historyQos.depth)
			{
				add = true;
			}
			else
			{
				if(mp_reader->change_removed_by_history(mp_minSeqCacheChange))
				{
					add =true;
				}
			}
		}
		if(add)
		{
			m_changes.push_back(a_change);
			increaseUnreadCount();
			if(a_change->sequenceNumber < mp_maxSeqCacheChange->sequenceNumber)
				sortCacheChanges();
			updateMaxMinSeqNum();
			if(m_changes.size()==m_resourceLimitsQos.max_samples)
				m_isHistoryFull = true;
			return true;
		}
		else
			return false;
	}
	//HISTORY WITH KEY
	else if(mp_Endpoint->getTopic().topicKind == WITH_KEY)
	{
		if(!a_change->instanceHandle.isDefined())
		{
			void* data = malloc(mp_reader->mp_type->m_typeSize);
			mp_reader->mp_type->deserialize(&a_change->serializedPayload,data);
			mp_reader->mp_type->getKey(data,&a_change->instanceHandle);
		}
	}

	return false;
}

void ReaderHistory::sortCacheChanges()
{
	std::sort(m_changes.begin(),m_changes.end(),sort_ReaderHistoryCache);
}


void ReaderHistory::updateMaxMinSeqNum()
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

bool ReaderHistory::isUnreadCache()
{
	if(m_unreadCacheCount>0)
		return true;
	else
		return false;
}


} /* namespace rtps */
} /* namespace eprosima */
