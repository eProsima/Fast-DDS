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

typedef enum ChangeForReaderStatus_t{
	UNSENT,
	UNACKNOWLEDGED,
	REQUESTED,
	ACKNOWLEDGED,
	UNDERWAY
}ChangeForReaderStatus_t;

typedef enum ChangeFromWriterStatus_t{
	UNKNOWN,
	MISSING,
	REQUESTED_WITH_NACK,
	RECEIVED,
	LOST
}ChangeFromWriterStatus_t;


//typedef void* InstanceHandle_t;

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
	CacheChange_t():
		kind(ALIVE)
	{

	}
	CacheChange_t(uint32_t payload_size):
		kind(ALIVE),
		serializedPayload(payload_size)
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
		if(serializedPayload.copy(&ch_ptr->serializedPayload))
			return true;
		else
			return false;
	}
	~CacheChange_t(){

	}
}CacheChange_t;

typedef struct ChangeForReader_t{
	CacheChange_t* change;
	ChangeForReaderStatus_t status;
	bool is_relevant;
}ChangeForReader_t;


typedef struct ChangeFromWriter_t{
	CacheChange_t* change;
	ChangeFromWriterStatus_t status;
	bool is_relevant;
}ChangeFromWriter_t;





}
}


#endif /* CACHECHANGE_H_ */
