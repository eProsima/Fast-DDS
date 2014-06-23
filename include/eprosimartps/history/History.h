/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file History.h
 *
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

using namespace boost;

#include "eprosimartps/utils/CacheChangePool.h"
#include "eprosimartps/common/types/SequenceNumber.h"
#include "eprosimartps/common/types/Guid.h"

#include "eprosimartps/qos/DDSQosPolicies.h"
using namespace eprosima::dds;

namespace eprosima {
namespace rtps {


class RTPSWriter;
class RTPSReader;
class Endpoint;

/**
 * Class HistoryCache, container of the different CacheChanges and the methods to access them.
 * @ingroup COMMONMODULE
 */
class History: public boost::basic_lockable_adapter<boost::recursive_mutex>
{
public:
	History(Endpoint* endp,
			HistoryQosPolicy historypolicy=HistoryQosPolicy(),
			ResourceLimitsQosPolicy resourcelimits=ResourceLimitsQosPolicy(),
			uint32_t payload_max_size=5000);

	virtual ~History();

	CacheChange_t* reserve_Cache()
	{
		return m_changePool.reserve_Cache();
	}

	void release_Cache(CacheChange_t* ch)
	{
		return m_changePool.release_Cache(ch);
	}

	//!Returns true if the History is full.
	bool isFull()
	{
		return m_isHistoryFull;
	}

	size_t getHistorySize(){
		return m_changes.size();
	}

	void sortCacheChangesBySeqNum();

	bool remove_all_changes();

	virtual void updateMaxMinSeqNum()=0;

	virtual bool add_change(CacheChange_t* a_change)=0;

	bool remove_change(CacheChange_t* ch);

protected:
	//!Vector of pointers to the CacheChange_t.
	std::vector<CacheChange_t*> m_changes;

	std::vector<std::pair<InstanceHandle_t,std::vector<CacheChange_t*>>> m_keyedChanges;

	const Endpoint* mp_Endpoint;

	HistoryQosPolicy m_historyQos;
	ResourceLimitsQosPolicy m_resourceLimitsQos;

	//!Variable to know if the history is full without needing to block the History mutex.
	bool m_isHistoryFull;

	CacheChange_t* mp_invalidCache;

	CacheChangePool m_changePool;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORY_H_ */
