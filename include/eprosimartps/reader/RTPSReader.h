/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSReader.h
 */



#ifndef RTPSREADER_H_
#define RTPSREADER_H_

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>



#include "eprosimartps/Endpoint.h"
//#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/history/ReaderHistory.h"
#include "eprosimartps/writer/RTPSMessageGroup.h"
#include "eprosimartps/qos/ReaderQos.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/utils/Semaphore.h"



using namespace eprosima::dds;

namespace eprosima {

namespace dds{

class SubscriberListener;
class SampleInfo_t;
}

namespace rtps {

class WriterProxyData;


/**
 * Class RTPSReader, manages the reception of data from the writers.
 * @ingroup READERMODULE
 */
class RTPSReader : public Endpoint{

public:
	RTPSReader(GuidPrefix_t guid,EntityId_t entId,TopicAttributes topic,DDSTopicDataType* ptype,
			StateKind_t state = STATELESS,
			int16_t userDefinedId=-1,uint32_t payload_size = 500);
	virtual ~RTPSReader();
	/**
	 * Read the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
	 * pointed by the StatusInfo_t structure.
	 * @param data Pointer to memory that can hold a sample.
	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
	 * @return True if correct.
	 */
	virtual bool readNextCacheChange(void*data,SampleInfo_t* info)=0;
	/**
	 * Take the next CacheChange_t from the history, deserializing it into the memory pointer by data (if the status is ALIVE), and filling the information
	 * pointed by the StatusInfo_t structure.
	 * @param data Pointer to memory that can hold a sample.
	 * @param info Pointer to SampleInfo_t structure to gather information about the sample.
	 * @return True if correct.
	 */
	virtual bool takeNextCacheChange(void*data,SampleInfo_t* info)=0;
	/**
	 * Add a matched writer represented by a WriterProxyData object.
	 * @param wdata Pointer to the WPD object to add.
	 * @return True if correctly added.
	 */
	virtual bool matched_writer_add(WriterProxyData* wdata)=0;
	/**
	 * Remove a WriterProxyData from the matached writers.
	 * @param wdata Pointer to the WPD object.
	 * @return True if correct.
	 */
	virtual bool matched_writer_remove(WriterProxyData* wdata)=0;
	/**
	 * Tells us if a specific Writer is matched against this reader
	 * @param wdata Pointer to the WriterProxyData object
	 * @return True if it is matched.
	 */
	virtual bool matched_writer_is_matched(WriterProxyData* wdata)=0;
	/**
	 * Get the number of matched publishers.
	 * @return True if correct.
	 */
	virtual size_t getMatchedPublishers()=0;
	//!Returns true if there are unread cacheChanges.
	virtual bool isUnreadCacheChange()=0;
	//!Returns true if the reader accepts messages from the writer with GUID_t entityGUID.
	virtual bool acceptMsgFrom(GUID_t& entityGUID,WriterProxy** wp = NULL)=0;
	//!Method to indicate the reader that some change has been removed due to HistoryQos requirements.
	virtual bool change_removed_by_history(CacheChange_t*,WriterProxy* prox = NULL)=0;



	CacheChange_t* reserve_Cache(){return m_reader_cache.reserve_Cache();}
	void release_Cache(CacheChange_t* ch){return m_reader_cache.release_Cache(ch);}
	size_t getHistoryCacheSize(){return m_reader_cache.getHistorySize();};
	virtual bool add_change(CacheChange_t* a_change,WriterProxy* prox = NULL)=0;//{return m_reader_cache.add_change(a_change,prox);};
	bool isHistoryFull(){return m_reader_cache.isFull();}
	SubscriberListener* getListener(){return mp_listener;}
	void setListener(SubscriberListener* plistener){mp_listener = plistener;}
	void setQos( ReaderQos& qos,bool first)	{return m_qos.setQos(qos,first);}
	bool canQosBeUpdated(ReaderQos& qos){return m_qos.canQosBeUpdated(qos);}
	const ReaderQos& getQos(){return m_qos;}
	bool expectsInlineQos(){return m_expectsInlineQos;}
	void setExpectsInlineQos(bool exp){m_expectsInlineQos = exp;}
	std::vector<CacheChange_t*>::iterator readerHistoryCacheBegin(){return m_reader_cache.changesBegin();}
	std::vector<CacheChange_t*>::iterator readerHistoryCacheEnd(){return m_reader_cache.changesEnd();}
	bool acceptMsgDirectedTo(EntityId_t& entityId);
	bool get_last_added_cache(CacheChange_t** change){	return m_reader_cache.get_last_added_cache(change);}
	void setTrustedWriter(EntityId_t writer){m_acceptMessagesFromUnkownWriters=false;m_trustedWriterEntityId = writer;	}


	Semaphore m_semaphore;

	/**
	 * Remove the CacheChange_t's that match the InstanceHandle_t passed.
	 * @param key The instance handle to remove.
	 * @return True if correct.
	 */
	bool removeCacheChangesByKey(InstanceHandle_t& key);

protected:
	//!Pointer to the associated subscriber
	//	Subscriber* mp_Sub;
	//!Pointer to the object used by the user to implement the behaviour when messages are received.
	SubscriberListener* mp_listener;
	friend bool SubscriberImpl::assignListener(SubscriberListener* plistener);

	//!History Cache of the Reader.
	ReaderHistory m_reader_cache;
	//!Whether the Reader expects Inline QOS.
	bool m_expectsInlineQos;

	ReaderQos m_qos;
	bool m_acceptMessagesToUnknownReaders;
	bool m_acceptMessagesFromUnkownWriters;
	EntityId_t m_trustedWriterEntityId;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
