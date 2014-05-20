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

#include "eprosimartps/common/attributes/TopicAttributes.h"
#include "eprosimartps/common/attributes/ReliabilityAttributes.h"
#include "eprosimartps/common/attributes/PublisherAttributes.h"
#include "eprosimartps/common/attributes/SubscriberAttributes.h"
#include "eprosimartps/common/attributes/ParticipantAttributes.h"

#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/RTPSMessageCreator.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/RTPSMessageGroup.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/qos/QosList.h"


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
	friend class HistoryCache;
public:
	RTPSWriter(uint16_t historysize,uint32_t payload_size);
	virtual ~RTPSWriter();

	/**
	 * Create a new change based on the provided data and instance handle.
	 * It assigns the correct values to each field and copies the data from data to change. The SequenceNumber is NOT assigned here but actually during
	 * the call to add_change in the HistoryCache, to prevent incorrect increments.
	 * @param changeKind The type of change.
	 * @param data Pointer to the serialized data that must be included in the change.
	 * @param change_out Pointer to pointer to return the change.
	 * @return True if correct.
	 */
	bool new_change(ChangeKind_t changeKind,void* data,CacheChange_t** change_out);

	/**
	 * Initialize the header message that is used in all RTPS Messages.
	 */
	void init_header();



	/**
	 * Increment the heartbeatCound.
	 */
	void heartbeatCount_increment() {
		++m_heartbeatCount;
	}
	/**
	 * Get the heartbeatCount.
	 * @return HeartbeatCount.
	 */
	Count_t getHeartbeatCount() const {
		return m_heartbeatCount;
	}



	StateKind_t getStateType() const {
		return m_stateType;
	}

	size_t getHistoryCacheSize()
	{
		return this->m_writer_cache.getHistorySize();
	}

	virtual void unsent_change_add(CacheChange_t* change)=0;
	virtual bool removeMinSeqCacheChange()=0;
	virtual bool removeAllCacheChange(int32_t* n_removed)=0;

	bool add_new_change(ChangeKind_t kind,void*Data);

	/**
	 * Get the minimum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_min(SequenceNumber_t* seqNum,GUID_t* writerGuid)
	{
		return m_writer_cache.get_seq_num_min(seqNum,writerGuid);
	}
	/**
	 * Get the maximum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_max(SequenceNumber_t* seqNum,GUID_t* writerGuid)
	{
		return m_writer_cache.get_seq_num_max(seqNum,writerGuid);
	}

	bool add_change(CacheChange_t*change)
	{
		return m_writer_cache.add_change(change);
	}

	bool get_last_added_cache(CacheChange_t**change)
	{
		return m_writer_cache.get_last_added_cache(change);
	}


protected:
	//!State type fo the writer
	StateKind_t m_stateType;
	//!Changes associated with this writer.
	HistoryCache m_writer_cache;
	//!Is the data sent directly or announced by HB and THEN send to the ones who ask for it?.
	bool m_pushMode;
	//!Type of the writer, either STATELESS or STATEFUL

	//SequenceNumber_t m_lastChangeSequenceNumber;
	Count_t m_heartbeatCount;



	RTPSMessageGroup_t m_cdrmessages;
public:


	Publisher* m_Pub;
	PublisherListener* mp_listener;
	QosList_t m_qosList;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
