/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSReader.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/HistoryCache.h"

#include "eprosimartps/dds/Subscriber.h"

namespace eprosima {
namespace rtps {

RTPSReader::RTPSReader(uint16_t historysize,uint32_t payload_size):
		stateType(STATELESS),
		reader_cache(historysize,payload_size),
		expectsInlineQos(true),
		Sub(NULL),
		newMessageSemaphore(new boost::interprocess::interprocess_semaphore(0)),
		newMessageCallback(NULL)
{
	reader_cache.rtpsreader = this;
	reader_cache.historyKind = READER;
}

RTPSReader::~RTPSReader() {
	// TODO Auto-generated destructor stub
	pDebugInfo("RTPSReader destructor"<<endl;);
}


} /* namespace rtps */
} /* namespace eprosima */


