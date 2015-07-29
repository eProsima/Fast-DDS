/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherHistory.cpp
 *
 */

#include <fastrtps/publisher/PublisherHistory.h>

#include "PublisherImpl.h"

#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

static const char* const CLASS_NAME = "PublisherHistory";

namespace eprosima {
namespace fastrtps {

PublisherHistory::PublisherHistory(PublisherImpl* pimpl,uint32_t payloadMaxSize,HistoryQosPolicy& history,
		ResourceLimitsQosPolicy& resource):
										WriterHistory(HistoryAttributes(payloadMaxSize,resource.allocated_samples,resource.max_samples)),
										m_historyQos(history),
										m_resourceLimitsQos(resource),
										mp_pubImpl(pimpl)
{
	// TODO Auto-generated constructor stub

}

PublisherHistory::~PublisherHistory() {
	// TODO Auto-generated destructor stub
}


bool PublisherHistory::add_pub_change(CacheChange_t* change)
{
	const char* const METHOD_NAME = "add_pub_change";
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_mutex);
	if(m_isHistoryFull && m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
	{
		logWarning(RTPS_HISTORY,"Attempting to add Data to Full WriterCache: "<<this->mp_pubImpl->getGuid().entityId
				<< " with KEEP ALL History ");
		return false;
	}
	//NO KEY HISTORY
	if(mp_pubImpl->getAttributes().topic.getTopicKind() == NO_KEY)
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
				if(this->remove_change_pub(mp_minSeqCacheChange))
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
	else if(mp_pubImpl->getAttributes().topic.getTopicKind() == WITH_KEY)
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
					if(remove_change_pub(vit->second.front(),&vit))
					{
						add = true;
					}
				}
			}
			if(add)
			{
				if(this->add_change(change))
				{

					logInfo(RTPS_HISTORY,this->mp_pubImpl->getGuid().entityId <<" Change "
							<< change->sequenceNumber.to64long()<< " added with key: "<<change->instanceHandle
							<< " and "<<change->serializedPayload.length<< " bytes");
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

bool PublisherHistory::find_Key(CacheChange_t* a_change,t_v_Inst_Caches::iterator* vit_out)
{
	const char* const METHOD_NAME = "find_Key";
	t_v_Inst_Caches::iterator vit;
	bool found = false;
	for(vit= m_keyedChanges.begin();vit!=m_keyedChanges.end();++vit)
	{
		if(a_change->instanceHandle == vit->first)
		{
			*vit_out = vit;
			return true;
		}
	}
	if(!found)
	{
		if((int)m_keyedChanges.size() < m_resourceLimitsQos.max_instances)
		{
			t_p_I_Change newpair;
			newpair.first = a_change->instanceHandle;
			m_keyedChanges.push_back(newpair);
			*vit_out = m_keyedChanges.end()-1;
			return true;
		}
		else
		{
			for (vit = m_keyedChanges.begin(); vit != m_keyedChanges.end(); ++vit)
			{
				if (vit->second.size() == 0)
				{
					m_keyedChanges.erase(vit);
					t_p_I_Change newpair;
					newpair.first = a_change->instanceHandle;
					m_keyedChanges.push_back(newpair);
					*vit_out = m_keyedChanges.end() - 1;
					return true;
				}
			}
			logWarning(SUBSCRIBER, "History has reached the maximum number of instances" << endl;)
		}
	}
	return false;
}


bool PublisherHistory::removeAllChange(size_t* removed)
{

	size_t rem = 0;
	//while(remove_min_change())
	while(m_changes.size()>0)
	{
		if(remove_change_pub(m_changes.front()))
			++rem;
		else
			break;
	}
	if(removed!=nullptr)
		*removed = rem;
	if (rem>0)
		return true;
	return false;
}


bool PublisherHistory::removeMinChange()
{
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_mutex);
	if(m_changes.size()>0)
		return remove_change_pub(m_changes.front());
	return false;
}

bool PublisherHistory::remove_change_pub(CacheChange_t* change,t_v_Inst_Caches::iterator* vit_in)
{
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_mutex);
	const char* const METHOD_NAME = "remove_change_pub";
	if(mp_pubImpl->getAttributes().topic.getTopicKind() == NO_KEY)
	{
		return this->remove_change(change);
	}
	else
	{
		t_v_Inst_Caches::iterator vit;
		if(vit_in!=nullptr)
			vit = *vit_in;
		else if(this->find_Key(change,&vit))
		{

		}
		else
			return false;
		for(auto chit = vit->second.begin();
				chit!= vit->second.end();++chit)
		{
			if((*chit)->sequenceNumber == change->sequenceNumber
					&& (*chit)->writerGUID == change->writerGUID)
			{
				if(remove_change(change))
				{
					vit->second.erase(chit);
					return true;
				}
			}
		}
		logError(PUBLISHER,"Change not found, something is wrong");
	}
	return false;
}


} /* namespace pubsub */
} /* namespace eprosima */
