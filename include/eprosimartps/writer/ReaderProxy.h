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

/**
 * ReaderProxy_t structure that contains the information of a specific ReaderProxy.
 * @ingroup WRITERMODULE
 */
typedef struct ReaderProxy_t{
	GUID_t remoteReaderGuid;
	bool expectsInlineQos;
	std::vector<Locator_t> unicastLocatorList;
	std::vector<Locator_t> multicastLocatorList;
	ReliabilityKind_t m_reliablility;
	ReaderProxy_t(){
		GUID_UNKNOWN(remoteReaderGuid);
		expectsInlineQos = false;
		m_reliablility = RELIABLE;
	}
}ReaderProxy_t;

/**
 * ReaderProxy class that helps to keep the state of a specific Reader with respect to the RTPSWRITER.
 * @ingroup WRITERMODULE
 */
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

	/**
	 * Mark all changes up to the one indicated by the seqNum as Acknowledged.
	 * If seqNum == 30, changes 1-29 are marked as ack.
	 * @param seqNum Pointer to the seqNum
	 * @return True if correct.
	 */
	bool acked_changes_set(SequenceNumber_t* seqNum);

	/**
	 * Mark all changes in the vector as requested.
	 * @param seqNumSet Vector of sequenceNumbers
	 * @return True if correct.
	 */
	bool requested_changes_set(std::vector<SequenceNumber_t>& seqNumSet);

	/**
	 * Get the next requested change.
	 * @param[out] changeForReader Pointer to a ChangeForReader to modify with the information.
	 * @return True if correct.
	 */
	bool next_requested_change(ChangeForReader_t* changeForReader);

	/**
	 * Get the next unsent change.
	 * @param[out] changeForReader Pointer to a ChangeForReader to modify with the information.
	 * @return True if correct.
	 */
	bool next_unsent_change(ChangeForReader_t* changeForReader);

	/**
	 * Get a vector of all requested changes by this Reader.
	 * @param reqChanges Pointer to a vector of pointers.
	 * @return True if correct.
	 */
	bool requested_changes(std::vector<ChangeForReader_t*>* reqChanges);

	/**
	 * Get a vector of all unsent changes to this Reader.
	 * @param reqChanges Pointer to a vector of pointers.
	 * @return True if correct.
	 */
	bool unsent_changes(std::vector<ChangeForReader_t*>* reqChanges);

	/**
	 * Get a vector of all unacked changes by this Reader.
	 * @param reqChanges Pointer to a vector of pointers.
	 * @return True if correct.
	 */
	bool unacked_changes(std::vector<ChangeForReader_t*>* reqChanges);
	//!Pointer to the associated StatefulWriter.
	StatefulWriter* mp_SFW;
	//!Parameters of the ReaderProxy
	ReaderProxy_t m_param;
	//!Vector of the changes and its state.
	std::vector<ChangeForReader_t> m_changesForReader;
	bool m_isRequestedChangesEmpty;

	//!Timed Event to manage the periodic HB to the Reader.
	PeriodicHeartbeat m_periodicHB;
	//!Timed Event to manage the Acknack response delay.
	NackResponseDelay m_nackResponse;
	//!Timed Event to manage the delay to mark a change as UNACKED after sending it.
	NackSupressionDuration m_nackSupression;

	uint32_t m_lastAcknackCount;



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
