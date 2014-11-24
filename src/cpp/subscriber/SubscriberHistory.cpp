/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberHistory.cpp
 *
 */

#include "fastrtps/subscriber/SubscriberHistory.h"
#include "fastrtps/TopicDataType.h"
#include "fastrtps/utils/RTPSLog.h"

namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "SubscriberHistory";

SubscriberHistory::SubscriberHistory(SubscriberImpl* simpl,uint32_t payloadMaxSize,
		HistoryQosPolicy& history,
		ResourceLimitsQosPolicy& resource):
				m_unreadCacheCount(0),
				m_historyQos(history),
				m_resourceLimitsQos(resource),
				mp_subImpl(simpl),
				mp_getKeyCache(nullptr)
{
	// TODO Auto-generated constructor stub
	this->reserve_Cache(&mp_getKeyCache);
}

SubscriberHistory::~SubscriberHistory() {
	// TODO Auto-generated destructor stub
}

bool SubscriberHistory::received_change(CacheChange_t* a_change, WriterProxy* WP)
{
	const char* const METHOD_NAME = "add_change";
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
				if(mp_reader->change_removed_by_history(mp_minSeqCacheChange,WP))
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
			if(this->add_change(a_change, WP))
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
		if(!a_change->instanceHandle.isDefined() && mp_subImpl->mp_type !=nullptr)
		{
//			if(mp_subImpl->getAttributes().getUserDefinedId() >= 0)
//			{
				logInfo(RTPS_HISTORY,"Getting Key of change with no Key transmitted")
						mp_subImpl->mp_type->deserialize(&a_change->serializedPayload,(void*)mp_getKeyCache->serializedPayload.data);
				if(!mp_subImpl->mp_type->getKey((void*)mp_getKeyCache->serializedPayload.data,&a_change->instanceHandle))
					return false;
			//}
//			else //FOR BUILTIN ENDPOINTS WE DIRECTLY SUPPLY THE SERIALIZEDPAYLOAD
//			{
//				if(!mp_subImpl->mp_type->getKey((void*)&a_change->serializedPayload,&a_change->instanceHandle))
//					return false;
//			}
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
					if(mp_reader->change_removed_by_history(vit->second.front(),WP))
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
				if(this->add_change(a_change, WP))
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
					logInfo(SUBSCRIBER,this->mp_Endpoint->getGuid().entityId
							<<": Change "<< a_change->sequenceNumber.to64long()<< " added from: "
							<< a_change->writerGUID<< "with KEY: "<< a_change->instanceHandle;);
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
	CacheChange_t* change;
	WriterProxy * wp;
	if(this->mp_reader->nextUnreadCache(&change,&wp))
	{
		change->isRead = true;
		this->decreaseUnreadCount();
		logInfo(SUBSCRIBER,this->getGuid().entityId<<": reading "<< change->sequenceNumber.to64long());
		if(change->kind == ALIVE)
			this->mp_subImpl->mp_type->deserialize(&change->serializedPayload,data);
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
				this->mp_subImpl->mp_type->getKey(data,&change->instanceHandle);
			}
			info->iHandle = change->instanceHandle;
		}
		return true;
	}
	return false;
}


bool SubscriberHistory::takeNextData(void* data, SampleInfo_t* info)
{
	CacheChange_t* change;
	WriterProxy * wp;
	if(this->mp_reader->nextUntakenCache(&change,&wp))
	{
		change->isRead = true;
		this->decreaseUnreadCount();
		logInfo(SUBSCRIBER,this->getGuid().entityId<<": reading "<< change->sequenceNumber.to64long());
		if(change->kind == ALIVE)
			this->mp_subImpl->mp_type->deserialize(&change->serializedPayload,data);
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
				this->mp_subImpl->mp_type->getKey(data,&change->instanceHandle);
			}
			info->iHandle = change->instanceHandle;
		}
		this->remove_change(change);
		return true;
	}
	return false;
}

} /* namespace fastrtps */
} /* namespace eprosima */
