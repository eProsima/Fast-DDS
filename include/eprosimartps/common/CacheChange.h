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
 */

#ifndef CACHECHANGE_H_
#define CACHECHANGE_H_

namespace eprosima{
namespace rtps{

typedef enum ChangeKind_t{
	ALIVE,
	NOT_ALIVE_DISPOSED,
	NOT_ALIVE_UNREGISTERED
}ChangeKind_t;


typedef void* InstanceHandle_t;


typedef struct CacheChange_t{
	ChangeKind_t kind;
	GUID_t writerGUID;
	InstanceHandle_t instanceHandle;
	SequenceNumber_t sequenceNumber;

}CacheChange_t;

}
}


#endif /* CACHECHANGE_H_ */
