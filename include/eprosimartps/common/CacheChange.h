/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CacheChange.h
 *	CacheChange definition.
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#ifndef CACHECHANGE_H_
#define CACHECHANGE_H_

namespace eprosima{
namespace rtps{

/**
 * Enum ChangeKind_t, different types of changes.
 */
typedef enum ChangeKind_t{
	ALIVE,                //!< ALIVE
	NOT_ALIVE_DISPOSED,   //!< NOT_ALIVE_DISPOSED
	NOT_ALIVE_UNREGISTERED//!< NOT_ALIVE_UNREGISTERED
}ChangeKind_t;

/**
 * Enum ChangeForReaderStatus_t, possible states for a CacheChange_t in a ReaderProxy.
 */
typedef enum ChangeForReaderStatus_t{
	UNSENT,        //!< UNSENT
	UNACKNOWLEDGED,//!< UNACKNOWLEDGED
	REQUESTED,     //!< REQUESTED
	ACKNOWLEDGED,  //!< ACKNOWLEDGED
	UNDERWAY       //!< UNDERWAY
}ChangeForReaderStatus_t;
/**
 * Enum ChangeFromWriterStatus_t, possible states for a CacheChange_t in a WriterProxy.
 */
typedef enum ChangeFromWriterStatus_t{
	UNKNOWN,
	MISSING,
	REQUESTED_WITH_NACK,
	RECEIVED,
	LOST
}ChangeFromWriterStatus_t;


/**
 * Structure CacheChange_t, contains information on a specific CacheChange.
 */
typedef struct CacheChange_t{
	//!Kind of change
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
	CacheChange_t():
		kind(ALIVE),
		isRead(false)
	{

	}
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
		if(serializedPayload.copy(&ch_ptr->serializedPayload))
			return true;
		else
			return false;
	}
	~CacheChange_t(){

	}
}CacheChange_t;

/**
 * Struct ChangeForReader_t used to indicate the state of a specific change with respect to a specific reader, as well as its relevance.
 */
typedef struct ChangeForReader_t{
	CacheChange_t* change;
	ChangeForReaderStatus_t status;
	bool is_relevant;
}ChangeForReader_t;

/**
 * Struct ChangeFromWriter_t used to indicate the state of a specific change with respect to a specific writer, as well as its relevance.
 */
typedef struct ChangeFromWriter_t{
	CacheChange_t* change;
	ChangeFromWriterStatus_t status;
	bool is_relevant;
}ChangeFromWriter_t;





}
}


#endif /* CACHECHANGE_H_ */
