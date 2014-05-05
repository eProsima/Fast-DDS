/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SampleInfo.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SAMPLEINFO_H_
#define SAMPLEINFO_H_

namespace eprosima {
namespace dds {

class SampleInfo_t {
public:
	SampleInfo_t():sampleKind(ALIVE){};
	virtual ~SampleInfo_t(){};
	ChangeKind_t sampleKind;
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* SAMPLEINFO_H_ */
