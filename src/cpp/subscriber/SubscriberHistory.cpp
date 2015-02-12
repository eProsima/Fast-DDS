/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberHistory.cpp
 *
 */

#include "fastrtps/subscriber/SubscriberHistory.h"
#include "fastrtps/subscriber/SubscriberImpl.h"

#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/reader/WriterProxy.h"

#include "fastrtps/TopicDataType.h"
#include "fastrtps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "SubscriberHistory";

inline bool sort_ReaderHistoryCache(CacheChange_t*c1,CacheChange_t*c2)
{
	return c1->sequenceNumber < c2->sequenceNumber;
}

SubscriberHistory::SubscriberHistory(SubscriberImpl* simpl,uint32_t payloadMaxSize,
		HistoryQosPolicy& history,
		ResourceLimitsQosPolicy& resource):
								ReaderHistory(HistoryAttributes(payloadMaxSize,resource.allocated_samples,resource.max_samples)),
								m_unreadCacheCount(0),
								m_historyQos(history),
								m_resourceLimitsQos(resource),
								mp_subImpl(simpl),
								mp_getKeyObject(nullptr)
{

	mp_getKeyObject = mp_subImpl->getType()->createData();

}

SubscriberHistory::~SubscriberHistory() {
	mp_subImpl->getType()->deleteData(mp_getKeyObject);

}

bool SubscriberHistory::received_change(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "add_change";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(m_isHistoryFull)
	{
		logWarning(SUBSCRIBER,"Attempting to add Data to Full ReaderHistory: "<<this->mp_subImpl->getGuid().entityId);
		return false;
	}
	//CHECK IF THE SAME CHANGE IS ALREADY IN THE HISTORY:
	if(a_change->sequenceNumber < mp_maxSeqCacheChange->sequenceNumber)
	{
		for(std::vector<CacheChange_t*>::reverse_iterator it=m_changes.rbegin();it!=m_changes.rend();++it)
		{
			if((*it)->sequenceNumber == a_change->sequenceNumber &&
					(*it)->writerGUID == a_change->writerGUID)
			{
				logInfo(RTPS_HISTORY,"Change (seqNum: "
						<< a_change->sequenceNumber.to64long()<< ") already in ReaderHistory";);
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
	if(mp_subImpl->getAttributes().topic.getTopicKind() == NO_KEY)
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
				bool read = false;
				if(mp_minSeqCacheChange->isRead)
					read = true;
				if(this->remove_change_sub(mp_minSeqCacheChange))
				{
					if(!read)
					{
						this->decreaseUnreadCount();
					}
					add = true;
				}
			}
		}
		if(add)
		{
			if(this->add_change(a_change))
			{
				increaseUnreadCount();
				if(a_change->sequenceNumber < mp_maxSeqCacheChange->sequenceNumber)
					sortCacheChanges();
				updateMaxMinSeqNum();
				if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
					m_isHistoryFull = true;
				logInfo(SUBSCRIBER,this->mp_subImpl->getGuid().entityId
						<<": Change "<< a_change->sequenceNumber.to64long()<< " added from: "
						<< a_change->writerGUID;);
				//print_changes_seqNum();
				return true;
			}
		}
		else
			return false;
	}
	//HISTORY WITH KEY
	else if(mp_subImpl->getAttributes().topic.getTopicKind() == WITH_KEY)
	{
		if(!a_change->instanceHandle.isDefined() && mp_subImpl->getType() !=nullptr)
		{
			logInfo(RTPS_HISTORY,"Getting Key of change with no Key transmitted")
							mp_subImpl->getType()->deserialize(&a_change->serializedPayload,mp_getKeyObject);
			if(!mp_subImpl->getType()->getKey(mp_getKeyObject,&a_change->instanceHandle))
				return false;

		}
		else if(!a_change->instanceHandle.isDefined())
		{
			logWarning(RTPS_HISTORY,"NO KEY in topic: "<< this->mp_subImpl->getAttributes().topic.topicName
					<< " and no method to obtain it";);
			return false;
		}
		t_v_Inst_Caches::iterator vit;
		if(find_Key(a_change,&vit))
		{
			//logInfo(RTPS_EDP,"Trying to add change with KEY: "<< vit->first << endl;);
			bool add = false;
			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
			{
				if((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
				{
					add = true;
				}
				else
				{
					logWarning(SUBSCRIBER,"Change not added due to maximum number of samples per instance";);
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
					bool read = false;
					if(vit->second.front()->isRead)
						read = true;
					if(this->remove_change_sub(vit->second.front(),&vit))
					{
						if(!read)
						{
							this->decreaseUnreadCount();
						}
						add = true;
					}
				}
			}
			if(add)
			{
				if(this->add_change(a_change))
				{
					increaseUnreadCount();
					if(a_change->sequenceNumber < mp_maxSeqCacheChange->sequenceNumber)
						sortCacheChanges();
					updateMaxMinSeqNum();
					if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
						m_isHistoryFull = true;
					//ADD TO KEY VECTOR
					if(vit->second.size() == 0)
					{
						vit->second.push_back(a_change);
					}
					else if(vit->second.back()->sequenceNumber < a_change->sequenceNumber)
					{
						vit->second.push_back(a_change);
					}
					else
					{
						vit->second.push_back(a_change);
						std::sort(vit->second.begin(),vit->second.end(),sort_ReaderHistoryCache);
					}
					logInfo(SUBSCRIBER,this->mp_reader->getGuid().entityId
							<<": Change "<< a_change->sequenceNumber.to64long()<< " added from: "
							<< a_change->writerGUID<< " with KEY: "<< a_change->instanceHandle;);
					//	print_changes_seqNum();
					return true;
				}
			}
			else
				return false;
		}
	}
	return false;
}

bool SubscriberHistory::readNextData(void* data, SampleInfo_t* info)
{
	const char* const METHOD_NAME = "readNextData";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	CacheChange_t* change;
	WriterProxy * wp;
	if(this->mp_reader->nextUnreadCache(&change,&wp))
	{
		change->isRead = true;
		this->decreaseUnreadCount();
		logInfo(SUBSCRIBER,this->mp_reader->getGuid().entityId<<": reading "<< change->sequenceNumber.to64long());
		if(change->kind == ALIVE)
			this->mp_subImpl->getType()->deserialize(&change->serializedPayload,data);
		if(info!=nullptr)
		{
			info->sampleKind = change->kind;
			info->writerGUID = change->writerGUID;
			info->sourceTimestamp = change->sourceTimestamp;
			if(this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
				info->ownershipStrength = wp->m_att.ownershipStrength;
			if(this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
					change->instanceHandle == c_InstanceHandle_Unknown &&
					change->kind == ALIVE)
			{
				this->mp_subImpl->getType()->getKey(data,&change->instanceHandle);
			}
			info->iHandle = change->instanceHandle;
		}
		return true;
	}
	return false;
}


bool SubscriberHistory::takeNextData(void* data, SampleInfo_t* info)
{
	const char* const METHOD_NAME = "takeNextData";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	CacheChange_t* change;
	WriterProxy * wp;
	if(this->mp_reader->nextUntakenCache(&change,&wp))
	{
		if(!change->isRead)
			this->decreaseUnreadCount();
		change->isRead = true;
		logInfo(SUBSCRIBER,this->mp_reader->getGuid().entityId<<": taking seqNum"<< change->sequenceNumber.to64long() <<
				" from writer: "<<change->writerGUID);
		if(change->kind == ALIVE)
			this->mp_subImpl->getType()->deserialize(&change->serializedPayload,data);
		if(info!=nullptr)
		{
			info->sampleKind = change->kind;
			info->writerGUID = change->writerGUID;
			info->sourceTimestamp = change->sourceTimestamp;
			if(this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
				info->ownershipStrength = wp->m_att.ownershipStrength;
			if(this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
					change->instanceHandle == c_InstanceHandle_Unknown &&
					change->kind == ALIVE)
			{
				this->mp_subImpl->getType()->getKey(data,&change->instanceHandle);
			}
			info->iHandle = change->instanceHandle;
		}
		this->remove_change_sub(change);
		return true;
	}
	//cout << "NEXT UNTAKEN CACHE BAD"<<endl;
	return false;
}

bool SubscriberHistory::find_Key(CacheChange_t* a_change, t_v_Inst_Caches::iterator* vit_out)
{
	const char* const METHOD_NAME = "find_Key";
	t_v_Inst_Caches::iterator vit;
	bool found = false;
	for (vit = m_keyedChanges.begin(); vit != m_keyedChanges.end(); ++vit)
	{
		if (a_change->instanceHandle == vit->first)
		{
			*vit_out = vit;
			return true;
		}
	}
	if (!found)
	{
		if ((int)m_keyedChanges.size() < m_resourceLimitsQos.max_instances)
		{
			t_p_I_Change newpair;
			newpair.first = a_change->instanceHandle;
			m_keyedChanges.push_back(newpair);
			*vit_out = m_keyedChanges.end() - 1;
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


bool SubscriberHistory::remove_change_sub(CacheChange_t* change,t_v_Inst_Caches::iterator* vit_in)
{
	const char* const METHOD_NAME = "remove_change_sub";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(mp_subImpl->getAttributes().topic.getTopicKind() == NO_KEY)
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
		logError(SUBSCRIBER,"Change not found, something is wrong");
	}
	return false;
}


} /* namespace fastrtps */
} /* namespace eprosima */
