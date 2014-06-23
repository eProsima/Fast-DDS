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

namespace eprosima {
namespace rtps {

WriterHistory::WriterHistory(Endpoint* endp,
							HistoryQosPolicy historypolicy,
							ResourceLimitsQosPolicy resourcelimits,
							uint32_t payload_max_size):
	History(endp,historypolicy,resourcelimits,payload_max_size),
	m_lastCacheChangeSeqNum(0),
	mp_minSeqCacheChange(mp_invalidCache),
	mp_maxSeqCacheChange(mp_invalidCache),
	mp_writer((RTPSWriter*)endp)
{

}

WriterHistory::~WriterHistory()
{
	// TODO Auto-generated destructor stub
}

bool WriterHistory::add_change(CacheChange_t* a_change)
{
	if(m_isHistoryFull)
	{
		pWarning("Attempting to add Data to Full WriterCache"<<endl;)
		return false;
	}
	if(mp_Endpoint->getTopic().topicKind == NO_KEY)
	{
		bool add = false;
		 if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
		 {
			 add = true;
		 }
		 else if(m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
		 {
			 if(m_changes.size()<m_historyQos.depth)
			 {
				 add = true;
			 }
			 else
			 {
				 mp_writer->change_removed_by_history(mp_minSeqCacheChange);
			 }
		 }
		 if(add)
		 {
			 m_lastCacheChangeSeqNum++;
			 a_change->sequenceNumber = m_lastCacheChangeSeqNum;
			 m_changes.push_back(a_change);
			 mp_maxSeqCacheChange = m_changes.back();
			 return true;
		 }





	}
	else if(mp_Endpoint->getTopic().topicKind == NO_KEY)
	{

	}
	//m_keyedChanges

	return true;
}


} /* namespace rtps */
} /* namespace eprosima */
