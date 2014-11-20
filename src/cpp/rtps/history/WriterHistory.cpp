/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterHistory.cpp
 *
 */

#include "eprosimartps/rtps/history/WriterHistory.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/rtps/writer/RTPSWriter.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
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
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(mp_writer == nullptr)
	{
		logError(RTPS_HISTORY,"You need to create a Writer with this History before adding any changes");
		return false;
	}
	if(a_change->writerGUID != mp_writer->getGuid())
	{
		logError(RTPS_HISTORY,"The GUID_t of the change doesn't correspond with the GUID_t of the writer");
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
	logInfo(RTPS_HISTORY,"Change "<< a_change->sequenceNumber.to64long() << " added with "<<a_change->serializedPayload.length<< " bytes");
	updateMaxMinSeqNum();
	mp_writer->unsent_change_add(a_change);
	return true;
}

bool WriterHistory::remove_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "remove_change";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(a_change == nullptr)
	{
		logError(RTPS_HISTORY,"Pointer is not valid")
		return false;
	}
	if(a_change->writerGUID != mp_writer->getGuid())
	{
		logError(RTPS_HISTORY,"The GUID_t of the change doesn't correspond with the GUID_t of the writer");
		return false;
	}
	for(std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
			chit!=m_changes.end();++chit)
	{
		if((*chit)->sequenceNumber == a_change->sequenceNumber)
		{
			m_changePool.release_Cache(a_change);
			m_changes.erase(chit);
			updateMaxMinSeqNum();
			mp_writer->change_removed_by_history(a_change);
			return true;
		}
	}
	logWarning(RTPS_HISTORY,"SequenceNumber "<<a_change->sequenceNumber.to64long()<< " not found");
	return false;
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
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(remove_change(mp_minSeqCacheChange))
	{
		updateMaxMinSeqNum();
		return true;
	}
	else
		return false;
}

//	if(m_isHistoryFull && m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
//	{
//		logWarning(RTPS_HISTORY,"Attempting to add Data to Full WriterCache: "<<this->mp_Endpoint->getGuid().entityId<< "/"<<this->mp_Endpoint->getTopic().getTopicName()<< " with KEEP ALL History ";)
//		return false;
//	}
//	//NO KEY HISTORY
//	if(mp_Endpoint->getTopic().topicKind == NO_KEY)
//	{
//		bool add = false;
//		if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
//		{
//			add = true;
//		}
//		else if(m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
//		{
//			if(m_changes.size()<(size_t)m_historyQos.depth)
//			{
//				add = true;
//			}
//			else
//			{
//				if(mp_writer->change_removed_by_history(mp_minSeqCacheChange))
//				{
//					add =true;
//				}
//			}
//		}
//		if(add)
//		{
//			++m_lastCacheChangeSeqNum;
//			a_change->sequenceNumber = m_lastCacheChangeSeqNum;
//			m_changes.push_back(a_change);
//			logInfo(RTPS_HISTORY,"Change "<< a_change->sequenceNumber.to64long() << " added with "<<a_change->serializedPayload.length<< " bytes");
//			updateMaxMinSeqNum();
//			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
//			{
//				if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
//					m_isHistoryFull = true;
//			}
//			else
//			{
//				if((int32_t)m_changes.size()==m_historyQos.depth)
//					m_isHistoryFull = true;
//			}
//			return true;
//		}
//		else
//			return false;
//	}
//	//HISTORY WITH KEY
//	else if(mp_Endpoint->getTopic().topicKind == WITH_KEY)
//	{
//		t_vectorPairKeyChanges::iterator vit;
//		if(find_Key(a_change,&vit))
//		{
//			logInfo(RTPS_HISTORY,"Found key: "<< vit->first);
//			bool add = false;
//			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
//			{
//				if((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
//				{
//					add = true;
//				}
//				else
//				{
//					logWarning(RTPS_HISTORY,"Change not added due to maximum number of samples per instance"<<endl;);
//					return false;
//				}
//			}
//			else if (m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
//			{
//				if(vit->second.size()< (size_t)m_historyQos.depth)
//				{
//					add = true;
//				}
//				else
//				{
//					if(mp_writer->change_removed_by_history(vit->second.front()))
//					{
//						add =true;
//					}
//				}
//			}
//			if(add)
//			{
//				++m_lastCacheChangeSeqNum;
//				a_change->sequenceNumber = m_lastCacheChangeSeqNum;
//				logInfo(RTPS_HISTORY,this->mp_Endpoint->getGuid().entityId <<" Change "
//						<< a_change->sequenceNumber.to64long()<< " added with key: "<<a_change->instanceHandle
//						<< " and "<<a_change->serializedPayload.length<< " bytes");
//				m_changes.push_back(a_change);
//				updateMaxMinSeqNum();
//				vit->second.push_back(a_change);
//				if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
//				{
//					if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
//						m_isHistoryFull = true;
//				}
//				else
//				{
//					if((int32_t)m_changes.size()==m_historyQos.depth*m_resourceLimitsQos.max_instances)
//						m_isHistoryFull = true;
//				}
//				return true;
//			}
//			else
//				return false;
//		}
//	}
//	return false;
//}








} /* namespace rtps */
} /* namespace eprosima */
