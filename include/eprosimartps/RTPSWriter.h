/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSWriter.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "rtps_all.h"
#include "HistoryCache.h"
#include "Endpoint.h"
#include "CDRMessageCreator.h"
#include "Participant.h"

#ifndef RTPSWRITER_H_
#define RTPSWRITER_H_

namespace eprosima {
namespace rtps {

/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a DDS Writer (not in this version) and a HistoryCache.
 */
class RTPSWriter: public Endpoint {
public:
	RTPSWriter();
	virtual ~RTPSWriter();
	HistoryCache writer_cache;
	bool pushMode;
	StateKind_t stateType;
	Duration_t heartbeatPeriod;
	Duration_t nackResponseDelay;
	Duration_t nackSupressionDuration;
	SequenceNumber_t lastChangeSequenceNumber;
	bool new_change(ChangeKind_t changekind,SerializedPayload_t* data,InstanceHandle_t handle,CacheChange_t*change);
	CDRMessageCreator MC;




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
