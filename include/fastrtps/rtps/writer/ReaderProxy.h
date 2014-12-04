/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.h
 *
 */



#ifndef READERPROXY_H_
#define READERPROXY_H_

#include <algorithm>
#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/common/SequenceNumber.h"
#include "fastrtps/rtps/common/CacheChange.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
namespace boost
{
class recursive_mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class NackResponseDelay;
class NackSupressionDuration;


/**
 * ReaderProxy class that helps to keep the state of a specific Reader with respect to the RTPSWriter.
 * @ingroup WRITERMODULE
 */
class ReaderProxy
{
public:
	virtual ~ReaderProxy();
	
	/**
	* Constructor.
	* @param rdata RemoteWriterAttributes to use in the creation.
	* @param times WriterTimes to use in the ReaderProxy.
	* @param SW Pointer to the StatefulWriter.
	*/
	ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW);

	/**
	 * Get the ChangeForReader struct associated with a determined change
	 * @param[in] change Pointer to the change.
	 * @param[out] changeForReader Pointer to a changeforreader structure.
	 * @return True if found.
	 */
	bool getChangeForReader(CacheChange_t* change,ChangeForReader_t* changeForReader);
	/**
	 * Get the ChangeForReader struct associated with a determined sequenceNumber
	 * @param[in] seq SequenceNumber
	 * @param[out] changeForReader Pointer to a changeforreader structure.
	 * @return True if found.
	 */
	bool getChangeForReader( SequenceNumber_t& seq,ChangeForReader_t* changeForReader);

	/**
	 * Mark all changes up to the one indicated by the seqNum as Acknowledged.
	 * If seqNum == 30, changes 1-29 are marked as ack.
	 * @param seqNum Pointer to the seqNum
	 * @return True if correct.
	 */
	bool acked_changes_set( SequenceNumber_t& seqNum);

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

	/**
	 * Sets the passes sequnceNumber to the maximum SequenceNumber_t.
	 * @param sn Pointer to the sequenceNumber.
	 * @return True if correct.
	 */
	bool max_acked_change(SequenceNumber_t* sn);


	//!Attributes of the Remote Reader
	RemoteReaderAttributes m_att;

	//!Pointer to the associated StatefulWriter.
	StatefulWriter* mp_SFW;

	//!Vector of the changes and its state.
	std::vector<ChangeForReader_t> m_changesForReader;
	
	//!Tells whether the requested changes list is empty
	bool m_isRequestedChangesEmpty;

	/**
	 * Returns a list of CacheChange_t that have the passes status.
	 * @param Changes Pointer to a vector of CacheChange_t pointers.
	 * @param status Status to be used.
	 * @return True if correctly obtained.
	 */
	bool changesList(std::vector<ChangeForReader_t*>* Changes,ChangeForReaderStatus_t status);

	/**
	 * Return the minimum change in a vector of CacheChange_t.
	 * @param Changes Pointer to a vector of CacheChange_t.
	 * @param changeForReader Pointer to the CacheChange_t.
	 * @return True if correct.
	 */
	bool minChange(std::vector<ChangeForReader_t*>* Changes,ChangeForReader_t* changeForReader);

	//!Timed Event to manage the Acknack response delay.
	NackResponseDelay* mp_nackResponse;
	//!Timed Event to manage the delay to mark a change as UNACKED after sending it.
	NackSupressionDuration* mp_nackSupression;
	//!Last ack/nack count
	uint32_t m_lastAcknackCount;


	//TODOG FILTER
	/**
	 * Filter a CacheChange_t, in this version always returns true.
	 * @param change
	 * @return
	 */
	inline bool rtps_is_relevant(CacheChange_t* change){return true;};

	//!Mutex
	boost::recursive_mutex* mp_mutex;

};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERPROXY_H_ */
