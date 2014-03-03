/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/HistoryCache.h"

#include "eprosimartps/Publisher.h"

namespace eprosima {
namespace rtps {

RTPSWriter::RTPSWriter() {
	// TODO Auto-generated constructor stub

}

RTPSWriter::~RTPSWriter() {
	// TODO Auto-generated destructor stub
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,
		SerializedPayload_t* data, InstanceHandle_t handle,CacheChange_t* change) {

	if(writer_cache.changes.size()+1 >(size_t)writer_cache.historySize)
		return false;


	change->kind = changeKind;
	//change->sequenceNumber = lastChangeSequenceNumber;
	change->writerGUID = guid;
	change->instanceHandle = handle;
	change->serializedPayload.copy(data);


	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
