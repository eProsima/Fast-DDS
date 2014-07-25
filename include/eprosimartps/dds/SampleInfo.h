/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SampleInfo.h
 */

#ifndef SAMPLEINFO_H_
#define SAMPLEINFO_H_

namespace eprosima {
namespace dds {

/**
 * Information provided along with a sample when reading data from a Subscriber.
 * Currently only the type of sample is provided (ALIVE, NOT_ALIVE_DISPOSED or NOT_ALIVE_UNREGISTER),
 * but more information will be added in future releases.
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

} /* namespace dds */
} /* namespace eprosima */

#endif /* SAMPLEINFO_H_ */
