/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSReader.h
 */



#ifndef RTPSREADER_H_
#define RTPSREADER_H_


#include "../Endpoint.h"
#include "../attributes/ReaderAttributes.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

class ReaderListener;
class ReaderHistory;
struct CacheChange_t;
class WriterProxy;
struct SequenceNumber_t;
class SequenceNumberSet_t;

/**
 * Class RTPSReader, manages the reception of data from the writers.
 * @ingroup READER_MODULE
 */
class RTPSReader : public Endpoint
{
	friend class ReaderHistory;
	friend class RTPSParticipantImpl;
	friend class MessageReceiver;
	friend class EDP;
protected:
	RTPSReader(RTPSParticipantImpl*,GUID_t& guid,
			ReaderAttributes& att,ReaderHistory* hist,ReaderListener* listen=nullptr);
	virtual ~RTPSReader();
public:
	/**
	 * Add a matched writer represented by a WriterProxyData object.
	 * @param wdata Pointer to the WPD object to add.
	 * @return True if correctly added.
	 */
	RTPS_DllAPI virtual bool matched_writer_add(RemoteWriterAttributes& wdata) = 0;
	
	/**
	 * Remove a WriterProxyData from the matached writers.
	 * @param wdata Pointer to the WPD object.
	 * @return True if correct.
	 */
	RTPS_DllAPI virtual bool matched_writer_remove(RemoteWriterAttributes& wdata) = 0;
	
	/**
	 * Tells us if a specific Writer is matched against this reader
	 * @param wdata Pointer to the WriterProxyData object
	 * @return True if it is matched.
	 */
	RTPS_DllAPI virtual bool matched_writer_is_matched(RemoteWriterAttributes& wdata) = 0;

	/**
	 * Returns true if the reader accepts a message directed to entityId.
	*/
	RTPS_DllAPI bool acceptMsgDirectedTo(EntityId_t& entityId);

	/**
	 * Processes a new DATA message. Previously the message must have been accepted by function acceptMsgDirectedTo.
	 *
     * @param change Pointer to the CacheChange_t.
	 * @return true if the reader accepts messages from the.
	 */
	RTPS_DllAPI virtual bool processDataMsg(CacheChange_t *change) = 0;

	/**
	* Processes a new DATA FRAG message. Previously the message must have been accepted by function acceptMsgDirectedTo.
	*
	* @param change Pointer to the CacheChange_t.
	* @return true if the reader accepts message.
	*/
	RTPS_DllAPI virtual bool processDataFragMsg(CacheChange_t *change, uint32_t sampleSize, uint32_t fragmentStartingNum) = 0;

	/**
	 * Processes a new HEARTBEAT message. Previously the message must have been accepted by function acceptMsgDirectedTo.
	 *
	 * @return true if the reader accepts messages from the.
	 */
    RTPS_DllAPI virtual bool processHeartbeatMsg(GUID_t &writerGUID, uint32_t hbCount, SequenceNumber_t &firstSN,
            SequenceNumber_t &lastSN, bool finalFlag, bool livelinessFlag) = 0;

    RTPS_DllAPI virtual bool processGapMsg(GUID_t &writerGUID, SequenceNumber_t &gapStart, SequenceNumberSet_t &gapList) = 0;
	
	/**
	* Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	* @param change Pointer to the CacheChange_t.
	* @param prox Pointer to the WriterProxy.
	* @return True if correctly removed.
	*/
	RTPS_DllAPI virtual bool change_removed_by_history(CacheChange_t* change, WriterProxy* prox = nullptr) = 0;

	/**
	* Get the associated listener.
	* @return Pointer to the associated reader listener.
	*/
	RTPS_DllAPI ReaderListener* getListener(){ return mp_listener; }
	
	/**
	 * Reserve a CacheChange_t.
	* @param change Pointer to pointer to the Cache.
	* @return True if correctly reserved.
	*/
	RTPS_DllAPI bool reserveCache(CacheChange_t** change);
	
	/**
	 * Release a cacheChange.
	*/
	RTPS_DllAPI void releaseCache(CacheChange_t* change);

	/**
	 * Read the next unread CacheChange_t from the history
	 * @param change POinter to pointer of CacheChange_t
	 * @param wp Pointer to pointer to the WriterProxy
	 * @return True if read.
	 */
	RTPS_DllAPI virtual bool nextUnreadCache(CacheChange_t** change, WriterProxy** wp) = 0;
	
	/**
	 * Get the next CacheChange_t from the history to take.
	 * @param change Pointer to pointer of CacheChange_t.
	 * @param wp Pointer to pointer to the WriterProxy.
	 * @return True if read.
	 */
	RTPS_DllAPI virtual bool nextUntakenCache(CacheChange_t** change, WriterProxy** wp) = 0;
	
	/**
	* @return True if the reader expects Inline QOS.
	*/
	RTPS_DllAPI inline bool expectsInlineQos(){ return m_expectsInlineQos; };
	//! Returns a pointer to the associated History.
	RTPS_DllAPI inline ReaderHistory* getHistory() {return mp_history;};

protected:
	void setTrustedWriter(EntityId_t writer)
	{
		m_acceptMessagesFromUnkownWriters=false;
		m_trustedWriterEntityId = writer;
	}
	//!ReaderHistory
	ReaderHistory* mp_history;
	//!Listener
	ReaderListener* mp_listener;
	//!Accept msg to unknwon readers (default=true)
	bool m_acceptMessagesToUnknownReaders;
	//!Accept msg from unknwon writers (BE-true,RE-false)
	bool m_acceptMessagesFromUnkownWriters;
	//!Trusted writer (for Builtin)
	EntityId_t m_trustedWriterEntityId;
	//!Expects Inline Qos.
	bool m_expectsInlineQos;

    std::vector<CacheChange_t*> cache_;

    private:

        RTPSReader& operator=(const RTPSReader&) NON_COPYABLE_CXX11;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
