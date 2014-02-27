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

#include "rtps_all.h"
#include "HistoryCache.h"
#include "Endpoint.h"
#include "CDRMessageCreator.h"
#include "Participant.h"

#ifndef RTPSREADER_H_
#define RTPSREADER_H_

namespace eprosima {
namespace rtps {

/**
 * Class RTPSReader, manages the reception of data from the writers.
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
	Participant * participant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSREADER_H_ */
