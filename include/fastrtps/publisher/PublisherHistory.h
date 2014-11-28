/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherHistory.h
 *
 */

#ifndef PUBLISHERHISTORY_H_
#define PUBLISHERHISTORY_H_

#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/qos/QosPolicies.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

class PublisherImpl;

class PublisherHistory:public WriterHistory
{
public:
	typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_p_I_Change;
	typedef std::vector<t_p_I_Change> t_v_Inst_Caches;
	PublisherHistory(PublisherImpl* pimpl,uint32_t payloadMax,
			HistoryQosPolicy& history,ResourceLimitsQosPolicy& resource);
	virtual ~PublisherHistory();
	bool add_pub_change(CacheChange_t* change);

	bool removeAllChange(size_t* removed);

	bool removeMinChange();

	bool remove_change_pub(CacheChange_t* change,t_v_Inst_Caches::iterator* vit=nullptr);

private:
	//!Vector of pointer to the CacheChange_t divided by key.
	t_v_Inst_Caches m_keyedChanges;
	//!HistoryQosPolicy values.
	HistoryQosPolicy m_historyQos;
	//!ResourceLimitsQosPolicy values.
	ResourceLimitsQosPolicy m_resourceLimitsQos;
	//!Publisher Pointer
	PublisherImpl* mp_pubImpl;

	bool find_Key(CacheChange_t* a_change,t_v_Inst_Caches::iterator* vecPairIterrator);
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PUBLISHERHISTORY_H_ */
