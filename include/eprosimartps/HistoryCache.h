/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HistoryCache.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"

#ifndef HISTORYCACHE_H_
#define HISTORYCACHE_H_

namespace eprosima {
namespace rtps {

class RTPSWriter;

class HistoryCache {
public:
	HistoryCache();
	virtual ~HistoryCache();
	bool get_change(SequenceNumber_t seqnum,CacheChange_t* change);
	bool add_change(CacheChange_t a_change);
	bool remove_change(CacheChange_t a_change);
	bool remove_change(SequenceNumber_t seqNum);
	SequenceNumber_t get_seq_num_min();
	SequenceNumber_t get_seq_num_max();
	std::vector<CacheChange_t> changes;
	RTPSWriter* rtpswriter;
private:

	SequenceNumber_t minSeqNum;
	SequenceNumber_t maxSeqNum;
	void updateMaxMinSeqNum();


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* HISTORYCACHE_H_ */
