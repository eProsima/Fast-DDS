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
 */



#ifndef READERPROXY_H_
#define READERPROXY_H_



#include <algorithm>


#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/common/types/Locator.h"

namespace boost
{
class recursive_mutex;
}

namespace eprosima {
namespace rtps {

class StatefulWriter;
class NackResponseDelay;
class NackSupressionDuration;


/**
 * ReaderProxy class that helps to keep the state of a specific Reader with respect to the RTPSWRITER.
 * @ingroup WRITERMODULE
 */
class ReaderProxy
{
public:
	virtual ~ReaderProxy();
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




	bool max_acked_change(SequenceNumber_t* sn);


	//!Attributes of the Remote Reader
	RemoteReaderAttributes m_att;


	//!Pointer to the associated StatefulWriter.
	StatefulWriter* mp_SFW;

	//!Vector of the changes and its state.
	std::vector<ChangeForReader_t> m_changesForReader;
	bool m_isRequestedChangesEmpty;

	bool changesList(std::vector<ChangeForReader_t*>* Changes,ChangeForReaderStatus_t status);

	bool minChange(std::vector<ChangeForReader_t*>* Changes,ChangeForReader_t* changeForReader);


	//!Timed Event to manage the Acknack response delay.
	NackResponseDelay* mp_nackResponse;
	//!Timed Event to manage the delay to mark a change as UNACKED after sending it.
	NackSupressionDuration* mp_nackSupression;

	uint32_t m_lastAcknackCount;


	//TODOG FILTER
	bool pubsub_is_relevant(CacheChange_t* change);

	boost::recursive_mutex* mp_mutex;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERPROXY_H_ */
