/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessReader.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/reader/StatelessReader.h"

namespace eprosima {
namespace rtps {

StatelessReader::StatelessReader() {
	// TODO Auto-generated constructor stub

}

StatelessReader::~StatelessReader() {
	// TODO Auto-generated destructor stub
}

StatelessReader::StatelessReader(ReaderParams_t* param)
{
	//reader_cache.changes.reserve(param.historySize);
	stateType = STATELESS;
	reader_cache.rtpsreader = (RTPSReader*)this;
	reader_cache.historyKind = READER;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	heartbeatResponseDelay = param->heartbeatResponseDelay;
	heartbeatSupressionDuration = param->heartbeatSupressionDuration;
	expectsInlineQos = param->expectsInlineQos;
	topicKind = param->topicKind;
	reader_cache.historySize = param->historySize;
}


} /* namespace rtps */
} /* namespace eprosima */
