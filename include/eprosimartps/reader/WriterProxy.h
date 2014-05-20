/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.h
 *	WriterProxy class
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
#include "eprosimartps/common/attributes/ReliabilityAttributes.h"
#include "eprosimartps/timedevent/HeartbeatResponseDelay.h"
namespace eprosima {
namespace rtps {

class StatefulReader;

/**
 * Structure WriterProxy_t that stores information for each WriterProxy.
 * @ingroup READERMODULE
 */
typedef struct WriterProxy_t{
	GUID_t remoteWriterGuid;
	LocatorList_t unicastLocatorList;
	LocatorList_t multicastLocatorList;
	WriterProxy_t(){
		GUID_UNKNOWN(remoteWriterGuid);
	}
}WriterProxy_t;


/**
 * Class WriterProxy that contains the state of each matched writer for a specific reader.
 * @ingroup READERMODULE
 */
class WriterProxy: public boost::basic_lockable_adapter<boost::recursive_mutex> {
public:
	virtual ~WriterProxy();
	WriterProxy(WriterProxy_t*RPparam,StatefulReader* SR);

	/**
	 * Get the maximum sequenceNumber received from this Writer.
	 * @param[out] seqNum Pointer to the sequenceNumber
	 * @return True if correct.
	 */
	bool available_changes_max(SequenceNumber_t* seqNum);
	/**
	 * Get the minimum sequenceNumber available from this Writer.
	 * @param[out] seqNum Pointer to the sequenceNumber
	 * @return True if correct.
	 */
	bool available_changes_min(SequenceNumber_t* seqNum);
	/**
	 * Update the missing changes up to the provided sequenceNumber.
	 * All changes with status UNKNOWN with seqNum <= input seqNum are marked MISSING.
	 * @param[in] seqNum Pointer to the SequenceNumber.
	 * @return True if correct.
	 */
	bool missing_changes_update(SequenceNumber_t* seqNum);
	/**
	 * Update the lost changes up to the provided sequenceNumber.
	 * All changes with status UNKNOWN or MISSING with seqNum < input seqNum are marked LOST.
	 * @param[in] seqNum Pointer to the SequenceNumber.
	 * @return True if correct.
	 */
	bool lost_changes_update(SequenceNumber_t* seqNum);
	/**
	 * The provided change is marked as RECEIVED.
	 * @param[in] change Pointer to the change
	 * @return True if correct.
	 */
	bool received_change_set(CacheChange_t* change);
	/**
	 * The change of the input seqNum is marked as RECEIVED and NOT RELEVANT.
	 * @param seqNum
	 * @return
	 */
	bool irrelevant_change_set(SequenceNumber_t* seqNum);

	/**
	 * THe method returns a vector containing all missing changes.
	 * @param missing Pointer to vector of pointers to ChangeFromWriter_t structure.
	 * @return True if correct.
	 */
	bool missing_changes(std::vector<ChangeFromWriter_t*>* missing);
	//! Pointer to associated StatefulReader.
	StatefulReader* mp_SFR;
	//! Parameters of the WriterProxy
	WriterProxy_t param;
	//!Vector containing the ChangeFromWriter_t objects.
	std::vector<ChangeFromWriter_t> m_changesFromW;
	//! Acknack Count
	uint32_t m_acknackCount;
	//! LAst HEartbeatcount.
	uint32_t m_lastHeartbeatCount;

	bool m_isMissingChangesEmpty;
	//!Timed event to postpone the heartbeatResponse.
	HeartbeatResponseDelay m_heartbeatResponse;
	bool m_heartbeatFinalFlag;


	SequenceNumber_t m_lastRemovedSeqNum;

	/**
	 * Remove a ChangeFromWriter based on the SequenceNumber of its associated change.
	 * @param seq SequenceNumber
	 * @return True if correct.
	 */
	bool removeChangeFromWriter(SequenceNumber_t& seq);


private:
	//!Get the maximum sequenceNumber in the list.
	SequenceNumber_t max_seq_num();

	/**
	 * Add changesFromWriter up to the sequenceNumber passed, but not including.
	 * Ex: If you hace seqNums 1,2,3 and you receive seqNum 6, you need 4 and 5 as unknown.
	 * You then marked them as Missing or lost or whathever.
	 * @param seqNum SequenceNumber to use.
	 * @return True if correct
	 */
	bool add_unknown_changes(SequenceNumber_t& seqNum);

	SequenceNumber_t m_max_available_seqNum;
	SequenceNumber_t m_min_available_seqNum;
	bool m_hasMaxAvailableSeqNumChanged;
	bool m_hasMinAvailableSeqNumChanged;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXY_H_ */
