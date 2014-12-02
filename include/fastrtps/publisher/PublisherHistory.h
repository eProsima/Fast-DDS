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

/**
 * This class implements a WriterHistory with support for keyed topics and HistoryQOS.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 */
class PublisherHistory:public WriterHistory
{
public:
	typedef std::pair<InstanceHandle_t,std::vector<CacheChange_t*>> t_p_I_Change;
	typedef std::vector<t_p_I_Change> t_v_Inst_Caches;
	/**
	* Constructor of the PublisherHistory.
	* @param pimpl Pointer to the PublisherImpl.
	* @param payloadMax Maximum payload size.
	* @param history QOS of the associated History.
	* @param resource ResourceLimits for the History.
	*/
	PublisherHistory(PublisherImpl* pimpl,uint32_t payloadMax,
			HistoryQosPolicy& history,ResourceLimitsQosPolicy& resource);
			
	virtual ~PublisherHistory();
	
	/**
	* Add a change comming from the Publisher.
	* @param change Pointer to the change
	* @return True if added.
	*/
	bool add_pub_change(CacheChange_t* change);

	/**
	* Remove all change from the associated history.
	* @param removed Number of elements removed.
	* @return True if all elements were removed.
	*/
	bool removeAllChange(size_t* removed);

	/**
	* Remove the change with the minimum sequence Number.
	* @return True if removed.
	*/
	bool removeMinChange();

	/**
	* Remove a change by the publisher History.
	* @param change Pointer to the CacheChange_t.
	* @param vit Pointer to the iterator of the Keyed history vector.
	* @return True if removed.
	*/
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
