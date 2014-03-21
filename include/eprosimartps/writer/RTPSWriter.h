/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWriter.h
 *  RTPS Writer class.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/RTPSMessageCreator.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/dds/Publisher.h"


#ifndef RTPSWRITER_H_
#define RTPSWRITER_H_




using namespace eprosima::dds;

namespace eprosima {



namespace rtps {



/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a DDS Writer (not in this version) and a HistoryCache.
  * @ingroup WRITERMODULE
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
	RTPSMessageCreator MC;
	Publisher* Pub;

	void sendChangesList(std::vector<CacheChange_t*> changes,
			std::vector<Locator_t>* unicast,std::vector<Locator_t>* multicast,
			bool expectsInlineQos,EntityId_t ReaderId);

	void DataSubM(CDRMessage_t* submsg,bool expectsInlineQos,CacheChange_t* change,EntityId_t ReaderId);



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
