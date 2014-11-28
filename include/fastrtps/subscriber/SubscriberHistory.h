/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberHistory.h
 *
 */

#ifndef SUBSCRIBERHISTORY_H_
#define SUBSCRIBERHISTORY_H_

#include "fastrtps/rtps/history/ReaderHistory.h"
#include "fastrtps/qos/QosPolicies.h"
#include "fastrtps/subscriber/SampleInfo.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

namespace rtps{
class WriterProxy;
}
using namespace rtps;

class SubscriberImpl;

class SubscriberHistory: public ReaderHistory {
public:
	typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_p_I_Change;
	typedef std::vector<t_p_I_Change> t_v_Inst_Caches;
	SubscriberHistory(SubscriberImpl* pimpl,uint32_t payloadMax,
			HistoryQosPolicy& history,ResourceLimitsQosPolicy& resource);
	virtual ~SubscriberHistory();

	bool received_change(CacheChange_t* change, WriterProxy*prox = nullptr);

	bool readNextData(void* data, SampleInfo_t* info);

	bool takeNextData(void* data, SampleInfo_t* info);

	bool isUnreadCache();

	bool remove_change_sub(CacheChange_t* change,t_v_Inst_Caches::iterator* vit=nullptr);


	inline void increaseUnreadCount()
	{
		++m_unreadCacheCount;
	}
	//!Decrease the unread count.
	inline void decreaseUnreadCount()
	{
		if(m_unreadCacheCount>0)
			--m_unreadCacheCount;
	}
	//!Get the unread count.
	inline uint64_t getUnreadCount()
	{
		return m_unreadCacheCount;
	}
private:
	uint64_t m_unreadCacheCount;


	//!Vector of pointer to the CacheChange_t divided by key.
	t_v_Inst_Caches m_keyedChanges;
	//!HistoryQosPolicy values.
	HistoryQosPolicy m_historyQos;
	//!ResourceLimitsQosPolicy values.
	ResourceLimitsQosPolicy m_resourceLimitsQos;
	//!Publisher Pointer
	SubscriberImpl* mp_subImpl;
	//!Change to obtain key when it is not provided
	CacheChange_t* mp_getKeyCache;


	bool find_Key(CacheChange_t* a_change,t_v_Inst_Caches::iterator* vecPairIterrator);

};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SUBSCRIBERHISTORY_H_ */
