/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of eProsimaRTPS is licensed to you under the terms described in the
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

#include "Publisher.h"


#ifndef RTPSWRITER_H_
#define RTPSWRITER_H_




using namespace eprosima::dds;

namespace eprosima {



namespace rtps {



/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a DDS Writer (not in this version) and a HistoryCache.
  * @ingroup RTPSMODULE
 */
class RTPSWriter: public Endpoint {
public:
	RTPSWriter();
	virtual ~RTPSWriter();
	//!Changes associated with this writer.
	HistoryCache writer_cache;
	//!Is the data sent directly or announced by HB and THEN send to the ones who ask for it?.
	bool pushMode;
	//!Type of the writer, either STATELESS or STATEFUL
	StateKind_t stateType;
	Duration_t heartbeatPeriod;
	Duration_t nackResponseDelay;
	Duration_t nackSupressionDuration;
	SequenceNumber_t lastChangeSequenceNumber;
	Count_t heartbeatCount;
	/**
	 * Create a new change based on the provided data and instance handle.
	 * It assigns the correct values to each field and copies the data from data to change. The SequenceNumber is NOT assigned here but actually during
	 * the call to add_change in the HistoryCache, to prevent incorrect increments.
	 * @param changekind The type of change.
	 * @param data Pointer to the serialized data that must be included in the change.
	 * @param handle Instance Handle of the data.
	 * @param change Pointer to store the change.
	 * @return True if correct.
	 */
	bool new_change(ChangeKind_t changekind,SerializedPayload_t* data,InstanceHandle_t handle,CacheChange_t*change);
	CDRMessageCreator MC;
	Publisher* Pub;




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
