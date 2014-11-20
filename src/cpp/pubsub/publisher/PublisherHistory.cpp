/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherHistory.cpp
 *
 */

#include "eprosimartps/pubsub/publisher/PublisherHistory.h"

#include "eprosimartps/utils/RTPSLog.h"

static const char* const CLASS_NAME = "PublisherHistory";

namespace eprosima {
namespace pubsub {

PublisherHistory::PublisherHistory() {
	// TODO Auto-generated constructor stub

}

PublisherHistory::~PublisherHistory() {
	// TODO Auto-generated destructor stub
}


bool PublisherHistory::add_pub_change(CacheChange_t* change)
{
	const char* const METHOD_NAME = "add_pub_change";
	if(m_isHistoryFull && m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
	{
		logWarning(RTPS_HISTORY,"Attempting to add Data to Full WriterCache: "<<this->mp_writer->getGuid().entityId
				<< " with KEEP ALL History ";)
						return false;
	}
	//NO KEY HISTORY
	if(mp_pub->getTopic().topicKind == NO_KEY)
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
			if(this->add_change(change))
			{
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
		}
	}
	//HISTORY WITH KEY
	else if(mp_pub->getTopic().topicKind == WITH_KEY)
	{
		t_v_Inst_Caches::iterator vit;
		if(find_Key(change,&vit))
		{
			logInfo(RTPS_HISTORY,"Found key: "<< vit->first);
			bool add = false;
			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
			{
				if((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
				{
					add = true;
				}
				else
				{
					logWarning(RTPS_HISTORY,"Change not added due to maximum number of samples per instance"<<endl;);
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
				if(this->add_change(change))
				{

					logInfo(RTPS_HISTORY,this->mp_Endpoint->getGuid().entityId <<" Change "
							<< a_change->sequenceNumber.to64long()<< " added with key: "<<a_change->instanceHandle
							<< " and "<<a_change->serializedPayload.length<< " bytes");
					vit->second.push_back(change);
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
			}
		}
	}
	return false;
}



} /* namespace pubsub */
} /* namespace eprosima */
