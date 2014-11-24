/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.h
 */

#ifndef WRITERPROXY_H_
#define WRITERPROXY_H_





#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/Locator.h"

#include "fastrtps/rtps/common/CacheChange.h"


namespace boost
{
class recursive_mutex;
}


namespace eprosima {
namespace fastrtps{
namespace rtps {

class RemoteWriterAttributes;
class StatefulReader;
class HeartbeatResponseDelay;
class WriterProxyLiveliness;

/**
 * Class WriterProxy that contains the state of each matched writer for a specific reader.
 * @ingroup READERMODULE
 */
class WriterProxy {
public:
	virtual ~WriterProxy();
	WriterProxy(RemoteWriterAttributes& watt,Duration_t heartbeatResponse,StatefulReader* SR);

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
	bool missing_changes_update(SequenceNumber_t& seqNum);
	/**
	 * Update the lost changes up to the provided sequenceNumber.
	 * All changes with status UNKNOWN or MISSING with seqNum < input seqNum are marked LOST.
	 * @param[in] seqNum Pointer to the SequenceNumber.
	 * @return True if correct.
	 */
	bool lost_changes_update(SequenceNumber_t& seqNum);
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
	bool irrelevant_change_set(SequenceNumber_t& seqNum);

	/**
	 * THe method returns a vector containing all missing changes.
	 * @param missing Pointer to vector of pointers to ChangeFromWriter_t structure.
	 * @return True if correct.
	 */
	bool missing_changes(std::vector<ChangeFromWriter_t*>* missing);
	//! Pointer to associated StatefulReader.
	StatefulReader* mp_SFR;
	//! Parameters of the WriterProxy
	RemoteWriterAttributes m_att;
	//!Vector containing the ChangeFromWriter_t objects.
	std::vector<ChangeFromWriter_t> m_changesFromW;
	//! Acknack Count
	uint32_t m_acknackCount;
	//! LAst HEartbeatcount.
	uint32_t m_lastHeartbeatCount;

	bool m_isMissingChangesEmpty;
	//!Timed event to postpone the heartbeatResponse.
	HeartbeatResponseDelay* mp_heartbeatResponse;
	//!TO check the liveliness Status periodically.
	WriterProxyLiveliness* mp_writerProxyLiveliness;


	bool m_heartbeatFinalFlag;


	SequenceNumber_t m_lastRemovedSeqNum;

	/**
	 * Remove a ChangeFromWriter based on the SequenceNumber of its associated change.
	 * @param seq SequenceNumber
	 * @return True if correct.
	 */
	bool removeChangesFromWriterUpTo(SequenceNumber_t& seq);

	bool get_change(SequenceNumber_t& seq,CacheChange_t** change);

	inline bool isAlive(){return m_isAlive;};
	inline void assertLiveliness(){m_isAlive=true;};

private:
	/**
	 * Add changesFromWriter up to the sequenceNumber passed, but not including.
	 * Ex: If you have seqNums 1,2,3 and you receive seqNum 6, you need to add 4,5 and 6
	 * as unknown to then later mark 6 as received.
	 * You then mark them as Missing or lost.
	 * The only exception is when you have received no messages, then you only add the seqNum you provide the method.
	 * @param seqNum SequenceNumber to use.
	 * @return True if correct
	 */
	bool add_changes_from_writer_up_to(SequenceNumber_t seqNum);


	SequenceNumber_t m_max_available_seqNum;
	SequenceNumber_t m_min_available_seqNum;
	bool m_hasMaxAvailableSeqNumChanged;
	bool m_hasMinAvailableSeqNumChanged;
	bool m_isAlive;

	void print_changes_fromWriter_test2();

	bool m_firstReceived;

	boost::recursive_mutex* mp_mutex;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXY_H_ */
