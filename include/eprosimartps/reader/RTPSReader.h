/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSReader.h
 *	RTPSReader class.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



#ifndef RTPSREADER_H_
#define RTPSREADER_H_

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>



#include "eprosimartps/Endpoint.h"
#include "eprosimartps/HistoryCache.h"
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

/**
 * Class RTPSReader, manages the reception of data from the writers.
  * @ingroup READERMODULE
 */
class RTPSReader : public Endpoint{

public:
	RTPSReader(GuidPrefix_t guid,EntityId_t entId,TopicAttributes topic,
			StateKind_t state = STATELESS,
			int16_t userDefinedId=-1,uint16_t historysize = 50,uint32_t payload_size = 500);
	virtual ~RTPSReader();




	virtual bool readNextCacheChange(void*data,SampleInfo_t* info)=0;
	virtual bool takeNextCacheChange(void*data,SampleInfo_t* info)=0;

	virtual bool isUnreadCacheChange()=0;


	CacheChange_t* reserve_Cache()
	{
		return m_reader_cache.reserve_Cache();
	}

	void release_Cache(CacheChange_t* ch)
	{
		return m_reader_cache.release_Cache(ch);
	}

	size_t getHistoryCacheSize(){return m_reader_cache.m_changes.size();};

	/**
	 * Add a change to the HistoryCache.
	 * @param[in] a_change Pointer to the change to add.
	 * @return True if correct.
	 */
	bool add_change(CacheChange_t* a_change){return m_reader_cache.add_change(a_change);};


	bool acceptMsgDirectedTo(EntityId_t& entityId);



	Semaphore m_semaphore;



	bool isHistoryFull()
	{
		return m_reader_cache.isFull();
	}
	SubscriberListener* getListener()
	{
		return mp_listener;
	}
	 void setListener(SubscriberListener* plistener)
		{
			mp_listener = plistener;
		}

	void setQos(ReaderQos& qos,bool first)
		{
			return m_qos.setQos(qos,first);
		}

protected:
	//!Pointer to the associated subscriber
	//	Subscriber* mp_Sub;
	//!Pointer to the object used by the user to implement the behaviour when messages are received.
	SubscriberListener* mp_listener;
	friend bool SubscriberImpl::assignListener(SubscriberListener* plistener);

	//!History Cache of the Reader.
	HistoryCache m_reader_cache;
	//!Whether the Reader expects Inline QOS.
	bool expectsInlineQos;



	ReaderQos m_qos;
	bool m_acceptMessagesToUnknownReaders;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
