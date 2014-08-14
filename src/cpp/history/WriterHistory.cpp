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

#include "eprosimartps/history/WriterHistory.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/common/CacheChange.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/writer/RTPSWriter.h"

namespace eprosima {
namespace rtps {


typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;



WriterHistory::WriterHistory(Endpoint* endp,
		uint32_t payload_max_size):
			History(endp,endp->getTopic().historyQos,endp->getTopic().resourceLimitsQos,payload_max_size),
			m_lastCacheChangeSeqNum(0,0),
			mp_writer((RTPSWriter*)endp)
{

}

WriterHistory::~WriterHistory()
{
	// TODO Auto-generated destructor stub
}

bool WriterHistory::add_change(CacheChange_t* a_change,WriterProxy* wp)
{
	if(m_isHistoryFull && m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
	{
		pWarning("Attempting to add Data to Full WriterCache: "<<this->mp_Endpoint->getGuid().entityId<< "/"<<this->mp_Endpoint->getTopic().getTopicName()<< " with KEEP ALL History "<<endl;)
		return false;
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
				if(mp_writer->change_removed_by_history(mp_minSeqCacheChange))
				{
					add =true;
				}
			}
		}
		if(add)
		{
			++m_lastCacheChangeSeqNum;
			a_change->sequenceNumber = m_lastCacheChangeSeqNum;
			m_changes.push_back(a_change);
			pDebugInfo("WriterHistory: Change "<< a_change->sequenceNumber.to64long() << " added."<< endl);
			updateMaxMinSeqNum();
			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
			{
				if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
					m_isHistoryFull = true;
			}
			else
			{
				if((int32_t)m_changes.size()==m_historyQos.depth)
					m_isHistoryFull = true;
			}
			return true;
		}
		else
			return false;
	}
	//HISTORY WITH KEY
	else if(mp_Endpoint->getTopic().topicKind == WITH_KEY)
	{
		t_vectorPairKeyChanges::iterator vit;
		if(find_Key(a_change,&vit))
		{
			pDebugInfo("WriterHistory: found key: "<< vit->first<<endl);
			bool add = false;
			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
			{
				if((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
				{
					add = true;
				}
				else
				{
					pWarning("WriterHistory: Change not added due to maximum number of samples per instance"<<endl;);
					return false;
				}
			}
			else if (m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
			{
				if(vit->second.size()< (size_t)m_historyQos.depth)
				{
					add = true;
				}
				else
				{
					if(mp_writer->change_removed_by_history(vit->second.front()))
					{
						add =true;
					}
				}
			}
			if(add)
			{
				++m_lastCacheChangeSeqNum;
				a_change->sequenceNumber = m_lastCacheChangeSeqNum;
				pDebugInfo("WriterHistory: " << this->mp_Endpoint->getGuid().entityId <<" Change "<< a_change->sequenceNumber.to64long() << " added with key: "<<a_change->instanceHandle<< endl);
				m_changes.push_back(a_change);
				updateMaxMinSeqNum();
				vit->second.push_back(a_change);
				if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
				{
					if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
						m_isHistoryFull = true;
				}
				else
				{
					if((int32_t)m_changes.size()==m_historyQos.depth*m_resourceLimitsQos.max_instances)
						m_isHistoryFull = true;
				}
				return true;
			}
			else
				return false;
		}
	}
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
	if(remove_change(mp_minSeqCacheChange))
	{
		updateMaxMinSeqNum();
		return true;
	}
	else
		return false;
}


} /* namespace rtps */
} /* namespace eprosima */
