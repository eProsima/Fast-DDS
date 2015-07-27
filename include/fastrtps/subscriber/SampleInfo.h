/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SampleInfo.h
 */

#ifndef SAMPLEINFO_H_
#define SAMPLEINFO_H_

#include <cstdint>

#include "../fastrtps_dll.h"

#include "../rtps/common/Guid.h"
#include "../rtps/common/Time_t.h"
#include "../rtps/common/InstanceHandle.h"
#include "../rtps/common/CacheChange.h"

namespace eprosima {
namespace fastrtps {

/**
 * Class SampleInfo_t with information that is provided along a sample when reading data from a Subscriber.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI SampleInfo_t {
public:
	SampleInfo_t():sampleKind(ALIVE),writerGUID(c_Guid_Unknown),ownershipStrength(0){};
	virtual ~SampleInfo_t(){};
	//!Sample kind.
	ChangeKind_t sampleKind;
	//!GUID_t of the writer of the sample.
	GUID_t writerGUID;
	//!Ownership Strength of the writer of the sample (0 if the ownership kind is set to SHARED_OWNERSHIP_QOS).
	uint16_t ownershipStrength;
	//!Source timestamp of the sample.
	Time_t sourceTimestamp;
	//!InstanceHandle of the data
	InstanceHandle_t iHandle;
};

} /* namespace  */
} /* namespace eprosima */

#endif /* SAMPLEINFO_H_ */
