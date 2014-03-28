/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.h
 *
 *  Created on: Mar 17, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */



#ifndef READERPROXY_H_
#define READERPROXY_H_

#include "eprosimartps/rtps_all.h"

#include <algorithm>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "eprosimartps/timedevent/PeriodicHeartbeat.h"
#include "eprosimartps/timedevent/NackResponseDelay.h"
#include "eprosimartps/timedevent/NackSupressionDuration.h"

namespace eprosima {
namespace rtps {

class StatefulWriter;

typedef struct ReaderProxy_t{
	GUID_t remoteReaderGuid;
	bool expectsInlineQos;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	ReaderProxy_t(){
		GUID_UNKNOWN(remoteReaderGuid);
		expectsInlineQos = false;
	}
}ReaderProxy_t;


class ReaderProxy: public boost::basic_lockable_adapter<boost::recursive_mutex> {
public:
	virtual ~ReaderProxy();
	ReaderProxy(ReaderProxy_t*RPparam,StatefulWriter* SW);



	/**
	 * Get the ChangeForReader struct associated with a determined change
	 * @param[in] change Pointer to the change.
	 * @param[out] changeForReader Pointer to a changeforreader structure.
	 * @return True if found.
	 */
	bool getChangeForReader(CacheChange_t* change,ChangeForReader_t* changeForReader);


	bool acked_changes_set(SequenceNumber_t* seqNum);

	bool requested_changes_set(std::vector<SequenceNumber_t>* seqNumSet);

	bool next_requested_change(ChangeForReader_t* changeForReader);

	bool next_unsent_change(ChangeForReader_t* changeForReader);

	bool requested_changes(std::vector<ChangeForReader_t*>* reqChanges);

	bool unsent_changes(std::vector<ChangeForReader_t*>* reqChanges);

	bool unacked_changes(std::vector<ChangeForReader_t*>* reqChanges);

	ReaderProxy_t m_param;
	std::vector<ChangeForReader_t> m_changesForReader;
	bool m_isRequestedChangesEmpty;

	PeriodicHeartbeat m_periodicHB;
	NackResponseDelay m_nackResponse;
	NackSupressionDuration m_nackSupression;

	uint32_t m_lastAcknackCount;
	StatefulWriter* m_SFW;


private:

	bool changesList(std::vector<ChangeForReader_t*>* Changes,ChangeForReaderStatus_t status);

	bool minChange(std::vector<ChangeForReader_t*>* Changes,ChangeForReader_t* changeForReader);

public:
	//TODOG DDSFILTER
		bool dds_is_relevant(CacheChange_t* change);



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERPROXY_H_ */
