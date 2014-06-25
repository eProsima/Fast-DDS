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


	void updateMaxMinSeqNum();

	bool add_change(CacheChange_t* a_change);

	bool remove_min_change();

protected:
	SequenceNumber_t m_lastCacheChangeSeqNum;
	RTPSWriter* mp_writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERHISTORY_H_ */
