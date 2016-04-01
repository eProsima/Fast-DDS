/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulWriter.h
 *
 */

#ifndef STATEFULWRITER_H_
#define STATEFULWRITER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "RTPSWriter.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class PeriodicHeartbeat;
class ReaderProxy;

/**
 * Class StatefulWriter, specialization of RTPSWriter that maintains information of each matched Reader.
 * @ingroup WRITER_MODULE
 */
class StatefulWriter: public RTPSWriter
{
	friend class RTPSParticipantImpl;
public:
	//!Destructor
	virtual ~StatefulWriter();
private:
	//!Constructor
	StatefulWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);
	//!Count of the sent heartbeats.
	Count_t m_heartbeatCount;
	//!Timed Event to manage the periodic HB to the Reader.
	PeriodicHeartbeat* mp_periodicHB;
	//!WriterTimes
	WriterTimes m_times;
	//! Vector containin all the associated ReaderProxies.
	std::vector<ReaderProxy*> matched_readers;
	//!EntityId used to send the HB.(only for builtin types performance)
	EntityId_t m_HBReaderEntityId;
public:
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
	 * Method to indicate that there are changes not sent in some of all ReaderProxy.
	 */
	void unsent_changes_not_empty();
	//!Increment the HB count.
	inline void incrementHBCount(){ ++m_heartbeatCount; };
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
	 * Remove the change with the minimum SequenceNumber
	 * @return True if removed.
	 */
	bool is_acked_by_all(CacheChange_t* a_change);

    bool clean_history(unsigned int max = 0);

	/**
	 * Update the Attributes of the Writer.
	 * @param att New attributes
	 */
	void updateAttributes(WriterAttributes& att);

	/**
	 * Find a Reader Proxy in this writer.
	 * @param[in] readerGuid The GUID_t of the reader.
	 * @param[out] RP Pointer to pointer to return the ReaderProxy.
	 * @return True if correct.
	 */
	bool matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP);

	/** Get count of heartbeats
	* @return count of heartbeats
	*/
	inline Count_t getHeartbeatCount() const {return this->m_heartbeatCount;};
	
	/** Get heartbeat reader entity id
	* @return heartbeat reader entity id
	*/
	inline EntityId_t getHBReaderEntityId() {return this->m_HBReaderEntityId;};

	/**
	* Get the begin of the matched readers
	* @return A vector iterator pointing to de begin of the matched readers list
	*/
	inline std::vector<ReaderProxy*>::iterator matchedReadersBegin(){return this->matched_readers.begin();};

	/**
	* Get the end of the matched readers
	* @return A vector iterator pointing to de end of the matched readers list
	*/
	inline std::vector<ReaderProxy*>::iterator matchedReadersEnd(){return this->matched_readers.end();};

	/**
	* Get the RTPS participant
	* @return RTPS participant
	*/
	inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}

	/**
	* Get the number of matched readers
	* @return Number of the matched readers
	*/
	inline size_t getMatchedReadersSize() const {return matched_readers.size();};

	/**
	 * Update the WriterTimes attributes of all associated ReaderProxy.
	* @param times WriterTimes parameter.
	*/
	void updateTimes(WriterTimes& times);

private:

    StatefulWriter& operator=(const StatefulWriter&) NON_COPYABLE_CXX11;

};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* STATEFULWRITER_H_ */
