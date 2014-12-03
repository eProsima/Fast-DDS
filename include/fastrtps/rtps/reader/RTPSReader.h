/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSReader.h
 */



#ifndef RTPSREADER_H_
#define RTPSREADER_H_


#include "fastrtps/rtps/Endpoint.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

class ReaderListener;
class ReaderHistory;
struct CacheChange_t;
class WriterProxy;

/**
 * Class RTPSReader, manages the reception of data from the writers.
 * @ingroup READERMODULE
 */
class RTPSReader : public Endpoint
{
	friend class ReaderHistory;
	friend class RTPSParticipantImpl;
	friend class MessageReceiver;
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
	RTPS_DllAPI virtual bool matched_writer_is_matched(RemoteWriterAttributes&) = 0;

	/**
	 * Check if the reader accepts messages from a writer with a specific GUID_t.
	 *
	 * @param entityGUID GUID to check
	 * @param wp Writer to check
	 * @return true if the reader accepts messages from the writer with GUID_t entityGUID.
	 */
	RTPS_DllAPI virtual bool acceptMsgFrom(GUID_t& entityGUID, WriterProxy** wp = nullptr) = 0;
	
	/**
	* @param a_change
	* @param prox
	* @return
	*/
	RTPS_DllAPI virtual bool change_received(CacheChange_t* a_change, WriterProxy* prox = nullptr) = 0;
	
	/**
	* Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	* @param 
	* @param prox
	* @return
	*/
	RTPS_DllAPI virtual bool change_removed_by_history(CacheChange_t*, WriterProxy* prox = nullptr) = 0;

	/**
	* Get the associated listener
	* @return Associated reader listener
	*/
	RTPS_DllAPI ReaderListener* getListener(){ return mp_listener; }

	/**
	* @param entityId
	* @return
	*/
	RTPS_DllAPI bool acceptMsgDirectedTo(EntityId_t& entityId);
	
	/**
	* @param change
	* @return
	*/
	RTPS_DllAPI bool reserveCache(CacheChange_t** change);
	
	/**
	* @param change
	* @return
	*/
	RTPS_DllAPI void releaseCache(CacheChange_t* change);

	/**
	 * Read the next unread CacheChange_t from the history
	 * @param change POinter to pointer of CacheChange_t
	 * @return True if read.
	 */
	RTPS_DllAPI virtual bool nextUnreadCache(CacheChange_t** change, WriterProxy** wp) = 0;
	
	/**
	 * Get the next CacheChange_t from the history to take.
	 * @param change Pointer to pointer of CacheChange_t
	 * @return True if read.
	 */
	RTPS_DllAPI virtual bool nextUntakenCache(CacheChange_t** change, WriterProxy** wp) = 0;
	
	/**
	* @return
	*/
	RTPS_DllAPI inline bool expectsInlineQos(){ return m_expectsInlineQos; };
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
	bool m_acceptMessagesToUnknownReaders;
	bool m_acceptMessagesFromUnkownWriters;
	EntityId_t m_trustedWriterEntityId;
	bool m_expectsInlineQos;


};


///*
// *
// * void setListener(SubscriberListener* plistener){mp_listener = plistener;}
//	void setQos( ReaderQos& qos,bool first)	{return m_qos.setQos(qos,first);}
//	bool canQosBeUpdated(ReaderQos& qos){return m_qos.canQosBeUpdated(qos);}
// *
// *
//
// *std::vector<CacheChange_t*>::iterator readerHistoryCacheBegin(){return m_reader_cache.changesBegin();}
//	std::vector<CacheChange_t*>::iterator readerHistoryCacheEnd(){return m_reader_cache.changesEnd();}
// *
//	/**
// * Remove the CacheChange_t's that match the InstanceHandle_t passed.
// * @param key The instance handle to remove.
// * @return True if correct.
//
//	bool removeCacheChangesByKey(InstanceHandle_t& key);
// *
// *
// *	/**
//	 * Get the number of matched publishers.
//	 * @return True if correct.
//
//	virtual size_t getMatchedPublishers()=0;
//	//!Returns true if there are unread cacheChanges.
//	virtual bool isUnreadCacheChange()=0;
// *
// *	//

//	//
//	/**
//	 * Read the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
//	 * pointed by the StatusInfo_t structure.
//	 * @param data Pointer to memory that can hold a sample.
//	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
//	 * @return True if correct.
//	 */
//	virtual bool readNextCacheChange(CacheChange_t** change)=0;
////	/**
////	 * Take the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
////	 * pointed by the StatusInfo_t structure.
////	 * @param data Pointer to memory that can hold a sample.
////	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
////	 * @return True if correct.
////	 */
////	virtual bool takeNextCacheChange(CacheChange_t** change)=0;
// *
// *
// */
//

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
