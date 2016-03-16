/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessWriter.h
 */


#ifndef STATELESSWRITER_H_
#define STATELESSWRITER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../common/Time_t.h"
#include "RTPSWriter.h"
#include "ReaderLocator.h"

#include <list>

namespace eprosima {
namespace fastrtps{
namespace rtps {


/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITER_MODULE
 */
class StatelessWriter : public RTPSWriter
{
	friend class RTPSParticipantImpl;

	StatelessWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);
public:
	virtual ~StatelessWriter();
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
	 * @param att New attributes
	 */
	void updateAttributes(WriterAttributes& /*att*/){
		//FOR NOW THERE IS NOTHING TO UPDATE.
	};

	/**
	* Add a remote locator.
	*
	* @param rdata RemoteReaderAttributes necessary to create a new locator.
	* @param loc Locator to add.
	* @return True on success.
	*/
	bool add_locator(RemoteReaderAttributes& rdata,Locator_t& loc);

	/**
	* Remove a remote locator from the writer.
	*
	* @param loc Locator to remove.
	* @return True on success.
	*/
	bool remove_locator(Locator_t& loc);

	//!Reset the unsent changes.
	void unsent_changes_reset();

	/**
	* Get the number of matched readers
	* @param Number of matched readers
	*/
	inline size_t getMatchedReadersSize() const {return m_matched_readers.size();};

private:
	//Duration_t resendDataPeriod; //FIXME: Not used yet.
	std::vector<ReaderLocator> reader_locator;
	std::vector<RemoteReaderAttributes> m_matched_readers;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* STATELESSWRITER_H_ */
