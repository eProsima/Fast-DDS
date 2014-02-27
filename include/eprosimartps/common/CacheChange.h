/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CacheChange.h
 *
 *  Created on: Feb 25, 2014
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


typedef void* InstanceHandle_t;

/**
 * Structure CacheChange_t, contains information on a specific CacheChange.
 */
typedef struct CacheChange_t{
	ChangeKind_t kind;
	GUID_t writerGUID;
	InstanceHandle_t instanceHandle;
	SequenceNumber_t sequenceNumber;
	SerializedPayload_t serializedPayload;
	CacheChange_t(){

	}
	bool copy(CacheChange_t* ach){
		kind = ach->kind;
		writerGUID = ach->writerGUID;
		instanceHandle = ach->instanceHandle;
		sequenceNumber = ach->sequenceNumber;
		if(serializedPayload.copy(&ach->serializedPayload))
			return true;
		else
			return false;
	}
	~CacheChange_t(){

	}
}CacheChange_t;

}
}


#endif /* CACHECHANGE_H_ */
