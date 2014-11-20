/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherHistory.h
 *
 */

#ifndef PUBLISHERHISTORY_H_
#define PUBLISHERHISTORY_H_

#include "eprosimartps/rtps/history/WriterHistory.h"
#include "eprosimartps/pubsub/qos/QosPolicies.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

class PublisherHistory:public WriterHistory
{
public:
	typedef std::vector<std::pair<InstanceHandle_t,std::vector<CacheChange_t*>>> t_v_Inst_Caches;
	PublisherHistory();
	virtual ~PublisherHistory();
	bool add_pub_change(CacheChange_t* change);
private:
	//!Vector of pointer to the CacheChange_t divided by key.
	t_v_Inst_Caches m_keyedChanges;
	//!HistoryQosPolicy values.
	HistoryQosPolicy m_historyQos;
	//!ResourceLimitsQosPolicy values.
	ResourceLimitsQosPolicy m_resourceLimitsQos;
	//!Publisher Pointer
	PublisherImpl* mp_pub;

	bool find_Key(CacheChange_t* a_change,t_v_Inst_Caches::iterator* vecPairIterrator);
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBLISHERHISTORY_H_ */
