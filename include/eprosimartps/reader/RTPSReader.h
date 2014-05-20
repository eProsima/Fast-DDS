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

#include "eprosimartps/rtps_all.h"

#include "eprosimartps/common/attributes/TopicAttributes.h"
#include "eprosimartps/common/attributes/ReliabilityAttributes.h"
#include "eprosimartps/common/attributes/PublisherAttributes.h"
#include "eprosimartps/common/attributes/SubscriberAttributes.h"
#include "eprosimartps/common/attributes/ParticipantAttributes.h"

#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/Endpoint.h"



#include "eprosimartps/RTPSMessageCreator.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/dds/SubscriberListener.h"
#include "eprosimartps/dds/SampleInfo.h"

#include "eprosimartps/utils/Semaphore.h"




using namespace eprosima::dds;

namespace eprosima {

namespace rtps {

/**
 * Class RTPSReader, manages the reception of data from the writers.
  * @ingroup READERMODULE
 */
class RTPSReader : public Endpoint{

public:
	RTPSReader(uint16_t historysize,uint32_t payload_size);
	virtual ~RTPSReader();
	//!Type of Reader, STATELESS or STATEFUL.
	StateKind_t m_stateType;
	//!History Cache of the Reader.
	HistoryCache m_reader_cache;
	//!Whether the Reader expects Inline QOS.
	bool expectsInlineQos;

	//!Structure used to create messages.
	//RTPSMessageCreator MC;

	//!Pointer to the associated subscriber
	Subscriber* mp_Sub;

	Semaphore m_semaphore;

	//!Pointer to the object used by the user to implement the behaviour when messages are received.
	SubscriberListener* mp_listener;



	virtual bool readNextCacheChange(void*data,SampleInfo_t* info)=0;
	virtual bool takeNextCacheChange(void*data,SampleInfo_t* info)=0;

	virtual bool isUnreadCacheChange()=0;




	StateKind_t getStateType() const {
		return m_stateType;
	}
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
