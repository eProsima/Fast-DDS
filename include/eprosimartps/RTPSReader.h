/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSReader.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "rtps_all.h"
#include "HistoryCache.h"
#include "Endpoint.h"
#include "CDRMessageCreator.h"
#include "Participant.h"

#include "Subscriber.h"

#ifndef RTPSREADER_H_
#define RTPSREADER_H_


using namespace eprosima::dds;
namespace eprosima {

namespace rtps {

/**
 * Class RTPSReader, manages the reception of data from the writers.
  * @ingroup RTPSMODULE
 */
class RTPSReader : public Endpoint{
public:
	RTPSReader();
	virtual ~RTPSReader();
	StateKind_t stateType;
	HistoryCache reader_cache;
	bool expectsInlineQos;
	Duration_t heartbeatResponseDelay;
	Duration_t heartbeatSupressionDuration;

	CDRMessageCreator MC;
	Participant* participant;
	Subscriber* Sub;
	boost::interprocess::interprocess_semaphore* newMessageSemaphore;
	void (*newMessageCallback)();

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
