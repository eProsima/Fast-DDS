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
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/RTPSMessageCreator.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/reader/RTPSListener.h"

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

	StateKind_t m_stateType;
	HistoryCache m_reader_cache;
	bool expectsInlineQos;

	//!Structure used to create messages.
	//RTPSMessageCreator MC;
	//!Pointer to the associated subscriber

	Subscriber* mp_Sub;

	//!Semaphore used to stop threads based on the arrival of messages.
	boost::interprocess::interprocess_semaphore* newMessageSemaphore;
	//!Function to call when a new message is received.
	void (*newMessageCallback)();
	RTPSListener* m_listener;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
