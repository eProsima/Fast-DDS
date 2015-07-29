/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CacheChange.h
 */

#ifndef CACHECHANGE_H_
#define CACHECHANGE_H_

#include "Types.h"
#include "Guid.h"
#include "SequenceNumber.h"
#include "SerializedPayload.h"
#include "Time_t.h"
#include "InstanceHandle.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{


/**
 * @enum ChangeKind_t, different types of CacheChange_t.
 * @ingroup COMMON_MODULE
 */
#if defined(_WIN32)
	enum RTPS_DllAPI ChangeKind_t{
#else
	enum ChangeKind_t{
#endif
	ALIVE,                //!< ALIVE
	NOT_ALIVE_DISPOSED,   //!< NOT_ALIVE_DISPOSED
	NOT_ALIVE_UNREGISTERED,//!< NOT_ALIVE_UNREGISTERED
	NOT_ALIVE_DISPOSED_UNREGISTERED //!<NOT_ALIVE_DISPOSED_UNREGISTERED
};


/**
 * Structure CacheChange_t, contains information on a specific CacheChange.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI CacheChange_t{
	//!Kind of change, default value ALIVE.
	ChangeKind_t kind;
	//!GUID_t of the writer that generated this change.
	GUID_t writerGUID;
	//!Handle of the data associated wiht this change.
	InstanceHandle_t instanceHandle;
	//!SequenceNumber of the change
	SequenceNumber_t sequenceNumber;
	//!Serialized Payload associated with the change.
	SerializedPayload_t serializedPayload;
	//!Indicates if the cache has been read (only used in READERS)
	bool isRead;
	//!Source TimeStamp (only used in Readers)
	Time_t sourceTimestamp;
	
	//!Default constructor.
	CacheChange_t():
		kind(ALIVE),
		isRead(false)
	{

	}
	
	/**
	* Constructor with payload size
	* @param payload_size Serialized payload size
	*/
	CacheChange_t(uint32_t payload_size):
		kind(ALIVE),
		serializedPayload(payload_size),
		isRead(false)
	{

	}
	/*!
	 * Copy a different change into this one. All the elements are copied, included the data, allocating new memory.
	 * @param[in] ch_ptr Pointer to the change.
	 * @return True if correct.
	 */
	bool copy(CacheChange_t* ch_ptr)
	{
		kind = ch_ptr->kind;
		writerGUID = ch_ptr->writerGUID;
		instanceHandle = ch_ptr->instanceHandle;
		sequenceNumber = ch_ptr->sequenceNumber;
		sourceTimestamp = ch_ptr->sourceTimestamp;
		return serializedPayload.copy(&ch_ptr->serializedPayload);
	}
	~CacheChange_t(){

	}
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Enum ChangeForReaderStatus_t, possible states for a CacheChange_t in a ReaderProxy.
 *  @ingroup COMMON_MODULE
 */
enum ChangeForReaderStatus_t{
	UNSENT = 0,        //!< UNSENT
	UNACKNOWLEDGED = 1,//!< UNACKNOWLEDGED
	REQUESTED = 2,     //!< REQUESTED
	ACKNOWLEDGED = 3,  //!< ACKNOWLEDGED
	UNDERWAY = 4       //!< UNDERWAY
};
/**
 * Enum ChangeFromWriterStatus_t, possible states for a CacheChange_t in a WriterProxy.
 *  @ingroup COMMON_MODULE
 */
enum ChangeFromWriterStatus_t{
	UNKNOWN = 0,
	MISSING = 1,
	//REQUESTED_WITH_NACK,
	RECEIVED = 2,
	LOST = 3
};



/**
 * Struct ChangeForReader_t used to represent the state of a specific change with respect to a specific reader, as well as its relevance.
 *  @ingroup COMMON_MODULE
 */
 class ChangeForReader_t{
 public:
	 ChangeForReader_t():status(UNSENT),is_relevant(true),m_isValid(false),change(NULL){};
	 virtual ~ChangeForReader_t(){};
	 //!Status
	 ChangeForReaderStatus_t status;
	 //!Boolean specifying if this change is relevant
	bool is_relevant;
	//!Sequence number
	SequenceNumber_t seqNum;
	 /**
	 * Get the cache change
	 * @return Cache change
	 */
	CacheChange_t* getChange()
	{
		return change;
	}
	 /**
	 * Set the cache change
	 * @param a_change Cache change
	 */
	bool setChange(CacheChange_t* a_change)
	{
		m_isValid = true;
		seqNum = a_change->sequenceNumber;
		change = a_change;
		return true;
	}
	
	 //! Set change as not valid
	void notValid()
	{
		is_relevant = false;
		m_isValid = false;
		change = NULL;
	}
	
	//! Set change as valid
	bool isValid()
	{
		return m_isValid;
	}
 private:
	bool m_isValid;
	CacheChange_t* change;
};

/**
 * Struct ChangeFromWriter_t used to indicate the state of a specific change with respect to a specific writer, as well as its relevance.
 *  @ingroup COMMON_MODULE
 */
class ChangeFromWriter_t
{
 public:
	 ChangeFromWriter_t():status(UNKNOWN),is_relevant(true),m_isValid(false),change(NULL)
	 {

	 }
	 virtual ~ChangeFromWriter_t(){};
	 //!Status
	 ChangeFromWriterStatus_t status;
	 //!Boolean specifying if this change is relevant
	 bool is_relevant;
	 //!Sequence number
	 SequenceNumber_t seqNum;
	 /**
	 * Get the cache change
	 * @return Cache change
	 */
	 CacheChange_t* getChange()
	 {
		 return change;
	 }
	 /**
	 * Set the cache change
	 * @param a_change Cache change
	 */
	 bool setChange(CacheChange_t* a_change)
	 {
		 m_isValid = true;
		 seqNum = a_change->sequenceNumber;
		 change = a_change;
		 return true;
	 }
	 //! Set change as not valid
	 void notValid()
	 {
		 is_relevant = false;
		 m_isValid = false;
		 change = NULL;
	 }
	 //! Set change as valid
	 bool isValid()
	 {
		 return m_isValid;
	 }
 private:
	 	bool m_isValid;
	CacheChange_t* change;

};

#endif


}
}
}


#endif /* CACHECHANGE_H_ */
