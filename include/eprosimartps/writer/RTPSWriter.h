/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWriter.h
 */


#ifndef RTPSWRITER_H_
#define RTPSWRITER_H_

#include "eprosimartps/Endpoint.h"

#include "eprosimartps/history/WriterHistory.h"


#include "eprosimartps/writer/RTPSMessageGroup.h"

#include "eprosimartps/qos/WriterQos.h"
#include "eprosimartps/dds/Publisher.h"

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/dds/attributes/PublisherAttributes.h"

using namespace eprosima::dds;

namespace eprosima {

namespace dds{

class PublisherListener;
}

namespace rtps {

class ReaderProxyData;

/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a DDS Writer (not in this version) and a HistoryCache.
  * @ingroup WRITERMODULE
 */
class RTPSWriter: public Endpoint
{
	friend class LivelinessPeriodicAssertion;
public:
	RTPSWriter(GuidPrefix_t guid,EntityId_t entId,const PublisherAttributes& param,DDSTopicDataType* ptype,
			StateKind_t state = STATELESS,
			int16_t userDefinedId=-1,uint32_t payload_size = 500);
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
	//!Get the number of changes in the History.
	size_t getHistoryCacheSize()
	{
		return this->m_writer_cache.getHistorySize();
	}
	/**
	 * Add a change to the unsent list.
	 * @param change Pointer to the change to add.
	 */
	virtual void unsent_change_add(CacheChange_t* change)=0;
	/**
	 * Remove the change with the minimum SequenceNumber
	 * @return True if removed.
	 */
	virtual bool removeMinSeqCacheChange()=0;
	/**
	 * Remove all changes from history
	 * @param n_removed Pointer to return the number of elements removed.
	 * @return True if correct.
	 */
	virtual bool removeAllCacheChange(size_t* n_removed)=0;
	//!Get the number of matched subscribers.
	virtual size_t getMatchedSubscribers()=0;
	//!Add a new change to the history.
	bool add_new_change(ChangeKind_t kind,void*Data);

	/**
	 * Get the minimum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_min(SequenceNumber_t* seqNum,GUID_t* writerGuid)
	{
		CacheChange_t* change;
		if(m_writer_cache.get_min_change(&change))
		{

		*seqNum = change->sequenceNumber;
		if(writerGuid!=NULL)
			*writerGuid = change->writerGUID;
		return true;
		}
		else
		{
			*seqNum = SequenceNumber_t(0,0);
			return false;
		}
	}
	/**
	 * Get the maximum sequence number in the HistoryCache.
	 * @param[out] seqNum Pointer to store the sequence number
	 * @param[out] writerGuid Pointer to store the writerGuid.
	 * @return True if correct.
	 */
	bool get_seq_num_max(SequenceNumber_t* seqNum,GUID_t* writerGuid)
	{
		CacheChange_t* change;
		if(m_writer_cache.get_max_change(&change))
		{
			*seqNum = change->sequenceNumber;
			if(writerGuid!=NULL)
				*writerGuid = change->writerGUID;
			return true;
		}
		else
		{
			*seqNum = SequenceNumber_t(0,0);
			return false;
		}
	}

	bool add_change(CacheChange_t*change)
	{
		if(m_writer_cache.add_change(change))
		{
			m_livelinessAsserted = true;
			return true;
		}
		return false;
	}

	bool get_last_added_cache(CacheChange_t**change)
	{
		m_writer_cache.get_max_change(change);
		return true;
	}

	void setQos( WriterQos& qos,bool first)
	{
		return m_qos.setQos(qos,first);
	}
	const WriterQos& getQos(){return m_qos;}

	PublisherListener* getListener(){return mp_listener;}
	void setListener(PublisherListener* plisten){mp_listener = plisten;}

	ParameterList_t* getInlineQos(){return &m_inlineQos;}

	bool getLivelinessAsserted()
	{
		return m_livelinessAsserted;
	}

	void setLivelinessAsserted(bool live)
	{
		m_livelinessAsserted = live;
	}
	/**
	 * Inidicate the writer that a change has been removed by the history due to some HistoryQos requirement.
	 * @param a_change Pointer to the change that is going to be removed.
	 * @return True if removed correctly.
	 */
	virtual bool change_removed_by_history(CacheChange_t* a_change)=0;
	/**
	 * Add a matched reader.
	 * @param rdata Pointer to the ReaderProxyData object added.
	 * @return True if added.
	 */
	virtual bool matched_reader_add(ReaderProxyData* rdata)=0;
	/**
	 * Remove a matched reader.
	 * @param rdata Pointer to the object to remove.
	 * @return True if removed.
	 */
	virtual bool matched_reader_remove(ReaderProxyData* rdata)=0;

protected:

	//!Changes associated with this writer.
	WriterHistory m_writer_cache;
	//!Is the data sent directly or announced by HB and THEN send to the ones who ask for it?.
	bool m_pushMode;

	//Count_t m_heartbeatCount;

	RTPSMessageGroup_t m_cdrmessages;

	/**
	 * Initialize the header message that is used in all RTPS Messages.
	 */
	void init_header();


	WriterQos m_qos;
	//Publisher* m_Pub;
	PublisherListener* mp_listener;

	ParameterList_t m_inlineQos;

	bool m_livelinessAsserted;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
