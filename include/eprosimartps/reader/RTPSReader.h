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

	virtual bool readNextCacheChange(void*data,SampleInfo_t* info)=0;
	virtual bool takeNextCacheChange(void*data,SampleInfo_t* info)=0;
	virtual bool matched_writer_add(WriterProxyData* wdata)=0;
	virtual bool matched_writer_remove(WriterProxyData* wdata)=0;
	virtual size_t getMatchedPublishers()=0;
	virtual bool isUnreadCacheChange()=0;
	virtual bool acceptMsgFrom(GUID_t& entityId)=0;
	virtual bool change_removed_by_history(CacheChange_t*)=0;



	CacheChange_t* reserve_Cache(){return m_reader_cache.reserve_Cache();}
	void release_Cache(CacheChange_t* ch){return m_reader_cache.release_Cache(ch);}
	size_t getHistoryCacheSize(){return m_reader_cache.getHistorySize();};
	bool add_change(CacheChange_t* a_change){return m_reader_cache.add_change(a_change);};
	bool isHistoryFull(){return m_reader_cache.isFull();}
	SubscriberListener* getListener(){return mp_listener;}
	void setListener(SubscriberListener* plistener){mp_listener = plistener;}
	void setQos( ReaderQos& qos,bool first)	{return m_qos.setQos(qos,first);}
	const ReaderQos& getQos(){return m_qos;}
	bool expectsInlineQos(){return m_expectsInlineQos;}
	std::vector<CacheChange_t*>::iterator readerHistoryCacheBegin(){return m_reader_cache.changesBegin();}
	std::vector<CacheChange_t*>::iterator readerHistoryCacheEnd(){return m_reader_cache.changesEnd();}
	bool acceptMsgDirectedTo(EntityId_t& entityId);
	bool get_last_added_cache(CacheChange_t** change){	return m_reader_cache.get_last_added_cache(change);}
	void setTrustedWriter(EntityId_t writer){m_acceptMessagesFromUnkownWriters=false;m_trustedWriterEntityId = writer;	}

	Semaphore m_semaphore;


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
