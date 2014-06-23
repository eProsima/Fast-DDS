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

typedef std::pair<InstanceHandle_t,uint32_t> t_KeyNumber;

class WriterHistory : public History
{
public:
	WriterHistory(Endpoint* endp,
			uint32_t payload_max_size=5000);
	virtual ~WriterHistory();


	void updateMaxMinSeqNum();

	bool add_change(CacheChange_t* a_change);

	void get_min_change(CacheChange_t** min_change)
	{
		*min_change = mp_minSeqCacheChange;
	}
	void get_max_change(CacheChange_t** max_change)
	{
		*max_change = mp_maxSeqCacheChange;
	}

	bool remove_min_change();

	std::vector<CacheChange_t*>::iterator changesBegin(){return m_changes.begin();}
	std::vector<CacheChange_t*>::iterator changesEnd(){return m_changes.end();}

	bool find_Key(CacheChange_t* a_change,
			std::vector<std::pair<InstanceHandle_t,std::vector<CacheChange_t*>>>::iterator* vecPairIterrator);

protected:
	SequenceNumber_t m_lastCacheChangeSeqNum;
	CacheChange_t* mp_minSeqCacheChange;
	CacheChange_t* mp_maxSeqCacheChange;


	RTPSWriter* mp_writer;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERHISTORY_H_ */
