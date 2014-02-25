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
 */

#include "eprosimartps/RTPSWriter.h"

namespace eprosima {
namespace rtps {

RTPSWriter::RTPSWriter() {
	// TODO Auto-generated constructor stub

}

RTPSWriter::~RTPSWriter() {
	// TODO Auto-generated destructor stub
}

CacheChange_t RTPSWriter::new_change(ChangeKind_t changeKind,
		SerializedPayload_t data, InstanceHandle_t handle) {

	lastChangeSequenceNumber++;
	CacheChange_t cache;
	cache.kind = changeKind;
	cache.sequenceNumber = lastChangeSequenceNumber;
	cache.writerGUID = guid;
	cache.instanceHandle = handle;

	return cache;
}

} /* namespace rtps */
} /* namespace eprosima */
