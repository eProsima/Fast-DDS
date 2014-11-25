/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessWriter.h
 */


#ifndef STATELESSWRITER_H_
#define STATELESSWRITER_H_

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/writer/ReaderLocator.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {


/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITERMODULE
 */
class StatelessWriter : public RTPSWriter
{
	friend class RTPSParticipantImpl;
public:
	//StatelessWriter();
	virtual ~StatelessWriter();
	StatelessWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);

	/**
	 * Add a specific change to all ReaderLocators.
	 * @param p Pointer to the change.
	 */
	void unsent_change_added_to_history(CacheChange_t* p);
	/**
	 * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
	 * @param a_change Pointer to the change that is going to be removed.
	 * @return True if removed correctly.
	 */
	bool change_removed_by_history(CacheChange_t* a_change);
	/**
	 * Add a matched reader.
	 * @param rdata Pointer to the ReaderProxyData object added.
	 * @return True if added.
	 */
	bool matched_reader_add(RemoteReaderAttributes& ratt);
	/**
	 * Remove a matched reader.
	 * @param rdata Pointer to the object to remove.
	 * @return True if removed.
	 */
	bool matched_reader_remove(RemoteReaderAttributes& ratt);
	/**
	 * Tells us if a specific Reader is matched against this writer
	 * @param rdata Pointer to the ReaderProxyData object
	 * @return True if it was matched.
	 */
	bool matched_reader_is_matched(RemoteReaderAttributes& ratt);
	/**
	 * Method to indicate that there are changes not sent in some of all ReaderProxy.
	 */
	void unsent_changes_not_empty();

	/**
	 * Update the Attributes of the Writer.
	 */
	void updateAttributes(WriterAttributes& att){/*TODOG TO FINISH METHOD*/		};

	bool add_locator(RemoteReaderAttributes& rdata,Locator_t& loc);

	bool remove_locator(Locator_t& loc);

	//!Reset the unsent changes.
	void unsent_changes_reset();
private:

	//	/**
	//	 * Add a ReaderLocator to the StatelessWriter.
	//	 * @param locator Locator to add
	//	 * @param expectsInlineQos Boolean variable indicating that the locator expects inline Qos.
	//	 * @return
	//	 */
	//	bool reader_locator_add(Locator_t& locator,bool expectsInlineQos);

	//	/**
	//	 * Add a specific change to all ReaderLocators.
	//	 * @param p Pointer to the change.
	//	 */
	//	void unsent_change_add(CacheChange_t* p);
	//	/**
	//	 * Method to indicate that there are changes not sent in some of all ReaderLocator.
	//	 */
	//	void unsent_changes_not_empty();
	//	/**
	//	 * Remove the change with the minimum SequenceNumber
	//	 * @return True if removed.
	//	 */
	//	bool removeMinSeqCacheChange();
	//	/**
	//	 * Remove all changes from history
	//	 * @param n_removed Pointer to return the number of elements removed.
	//	 * @return True if correct.
	//	 */
	//	bool removeAllCacheChange(size_t* n_removed);
	//	//!Get the number of matched subscribers.
	//	size_t getMatchedSubscribers(){return reader_locator.size();}
	//	bool change_removed_by_history(CacheChange_t* a_change);
	//
private:
	//Duration_t resendDataPeriod; //FIXME: Not used yet.
	std::vector<ReaderLocator> reader_locator;
	std::vector<RemoteReaderAttributes> m_matched_readers;

};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSWRITER_H_ */
