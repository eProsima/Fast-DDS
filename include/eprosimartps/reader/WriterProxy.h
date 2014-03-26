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
	WriterProxy();
	virtual ~WriterProxy();
	WriterProxy(WriterProxy_t*RPparam,StatefulReader* SR);
	WriterProxy_t param;

	std::vector<ChangeFromWriter_t> changes;

	bool available_changes_max(SequenceNumber_t* seqNum);

	bool missing_changes_update(SequenceNumber_t* seqNum);

	bool lost_changes_update(SequenceNumber_t* seqNum);

	bool received_change_set(CacheChange_t* change);

	bool irrelevant_change_set(SequenceNumber_t* seqNum);

	bool missing_changes(std::vector<ChangeFromWriter_t*>* missing);

	uint32_t acknackCount;

	uint32_t lastHeartbeatCount;
	bool isMissingChangesEmpty;

	HeartbeatResponseDelay heartbeatResponse;
	StatefulReader* SFR;
private:
	bool max_seq_num(SequenceNumber_t* sn);

	bool add_unknown_changes(SequenceNumber_t* sn);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXY_H_ */
