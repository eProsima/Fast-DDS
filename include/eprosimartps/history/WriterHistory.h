/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterHistory.h
 *
 */

#ifndef WRITERHISTORY_H_
#define WRITERHISTORY_H_

#include "eprosimartps/history/History.h"

namespace eprosima {
namespace rtps {

//typedef std::pair<InstanceHandle_t,uint32_t> t_KeyNumber;

class WriterHistory : public History
{
public:
	WriterHistory(Endpoint* endp,uint32_t payload_max_size=5000);
	virtual ~WriterHistory();

	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	void updateMaxMinSeqNum();
	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @return True if added.
	 */
	bool add_change(CacheChange_t* a_change,WriterProxy* wp=NULL);
	/**
	 * Remove the CacheChange_t with the minimum sequenceNumber.
	 * @return True if correctly removed.
	 */
	bool remove_min_change();

protected:
	SequenceNumber_t m_lastCacheChangeSeqNum;
	RTPSWriter* mp_writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERHISTORY_H_ */
