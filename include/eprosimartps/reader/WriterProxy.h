/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef WRITERPROXY_H_
#define WRITERPROXY_H_

#include "eprosimartps/rtps_all.h"

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "eprosimartps/timedevent/HeartbeatResponseDelay.h"
namespace eprosima {
namespace rtps {

class StatefulReader;

typedef struct WriterProxy_t{
	GUID_t remoteWriterGuid;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	WriterProxy_t(){
		GUID_UNKNOWN(remoteWriterGuid);
	}
}WriterProxy_t;



class WriterProxy: public boost::basic_lockable_adapter<boost::recursive_mutex> {
public:
	virtual ~WriterProxy();
	WriterProxy(WriterProxy_t*RPparam,StatefulReader* SR);


	bool available_changes_max(SequenceNumber_t* seqNum);

	bool missing_changes_update(SequenceNumber_t* seqNum);

	bool lost_changes_update(SequenceNumber_t* seqNum);

	bool received_change_set(CacheChange_t* change);

	bool irrelevant_change_set(SequenceNumber_t* seqNum);

	bool missing_changes(std::vector<ChangeFromWriter_t*>* missing);
	StatefulReader* mp_SFR;
	WriterProxy_t param;
	std::vector<ChangeFromWriter_t> m_changesFromW;
	uint32_t m_acknackCount;
	uint32_t m_lastHeartbeatCount;
	bool m_isMissingChangesEmpty;
	HeartbeatResponseDelay m_heartbeatResponse;
	bool m_heartbeatFinalFlag;


private:
	SequenceNumber_t max_seq_num();

	/**
	 * Add changesFromWriter up to the sequenceNumber passed, but not including.
	 * Ex: If you hace seqNums 1,2,3 and you receive seqNum 6, you need 4 and 5 as unknown.
	 * You then marked them as Missing or lost or whathever.
	 * @param seqNum SequenceNumber to use.
	 * @return True if correct
	 */
	bool add_unknown_changes(SequenceNumber_t& seqNum);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXY_H_ */
