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

#include "eprosimartps/dds/DDSTopicDataType.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ReaderHistory";

typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_pairKeyChanges;
typedef std::vector<t_pairKeyChanges> t_vectorPairKeyChanges;

bool sort_ReaderHistoryCache(CacheChange_t*c1,CacheChange_t*c2)
{
	return c1->sequenceNumber < c2->sequenceNumber;
}

ReaderHistory::ReaderHistory(Endpoint* endp,
		uint32_t payload_max_size):
						History(endp,endp->getTopic().historyQos,endp->getTopic().resourceLimitsQos,payload_max_size),
						mp_lastAddedCacheChange(mp_invalidCache),
						mp_reader((RTPSReader*) endp),
						m_unreadCacheCount(0),
						mp_getKeyCache(NULL)

{
	mp_getKeyCache = m_changePool.reserve_Cache();
	//TODO Auto-generated constructor stub
}

ReaderHistory::~ReaderHistory()
{
	// TODO Auto-generated destructor stub
}

bool ReaderHistory::add_change(CacheChange_t* a_change,WriterProxy* WP)
{
	const char* const METHOD_NAME = "add_change";
	if(m_isHistoryFull)
	{
		logWarning(RTPS_HISTORY,"Attempting to add Data to Full ReaderCache: "
				<<this->mp_Endpoint->getGuid().entityId;)
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
			m_changes.push_back(a_change);
			this->mp_lastAddedCacheChange = a_change;
			increaseUnreadCount();
			if(a_change->sequenceNumber < mp_maxSeqCacheChange->sequenceNumber)
				sortCacheChanges();
			updateMaxMinSeqNum();
			if((int32_t)m_changes.size()==m_resourceLimitsQos.max_samples)
				m_isHistoryFull = true;
			logInfo(RTPS_HISTORY,"ReaderHistory " <<this->mp_Endpoint->getGuid().entityId
					<<": Change "<< a_change->sequenceNumber.to64long()<< " added from: "
					<< a_change->writerGUID;);
			//print_changes_seqNum();
			return true;
		}
		else
			return false;
	}
	//HISTORY WITH KEY
	else if(mp_Endpoint->getTopic().topicKind == WITH_KEY)
	{
		if(!a_change->instanceHandle.isDefined() && mp_reader->mp_type !=NULL)
		{
			if(this->mp_Endpoint->getUserDefinedId() >= 0)
			{
				logInfo(RTPS_HISTORY,"ReaderHistory getting Key of change with no Key transmitted"<<endl;)
						mp_reader->mp_type->deserialize(&a_change->serializedPayload,(void*)mp_getKeyCache->serializedPayload.data);
				if(!mp_reader->mp_type->getKey((void*)mp_getKeyCache->serializedPayload.data,&a_change->instanceHandle))
					return false;
			}
			else //FOR BUILTIN ENDPOINTS WE DIRECTLY SUPPLY THE SERIALIZEDPAYLOAD
			{
				if(!mp_reader->mp_type->getKey((void*)&a_change->serializedPayload,&a_change->instanceHandle))
					return false;
			}
		}
		else if(!a_change->instanceHandle.isDefined())
		{
			logWarning(RTPS_HISTORY,"ReaderHistory: NO KEY in topic: "<< this->mp_Endpoint->getTopic().topicName
					<< " and no method to obtain it";);
			return false;
		}
		t_vectorPairKeyChanges::iterator vit;
		if(find_Key(a_change,&vit))
		{
			//pDebugInfo("Trying to add change with KEY: "<< vit->first << endl;);
			bool add = false;
			if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
			{
				if((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
				{
					add = true;
				}
				else
				{
					logWarning(RTPS_HISTORY,"WriterHistory: Change not added due to maximum number of samples per instance";);
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
				m_changes.push_back(a_change);
				this->mp_lastAddedCacheChange = a_change;
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
				logInfo(RTPS_HISTORY,"ReaderHistory " <<this->mp_Endpoint->getGuid().entityId
						<<": Change "<< a_change->sequenceNumber.to64long()<< " added from: "
						<< a_change->writerGUID<< "with KEY: "<< a_change->instanceHandle;);
			//	print_changes_seqNum();
				return true;
			}
			else
				return false;
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

bool ReaderHistory::get_last_added_cache(CacheChange_t** change)
{
	if(mp_lastAddedCacheChange->sequenceNumber != mp_invalidCache->sequenceNumber)
	{
		*change = mp_lastAddedCacheChange;
		return true;
	}
	return false;
}


bool ReaderHistory::removeCacheChangesByKey(InstanceHandle_t& key)
{
	const char* const METHOD_NAME = "removeCacheChangesByKey";
	logError(RTPS_HISTORY,"Not Implemented yet";);
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
