/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBParticipant.h
 *
 */

#ifndef PUBSUBPARTICIPANT_H_
#define PUBSUBPARTICIPANT_H_

#include "eprosimartps/rtps/common/Guid.h"

#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

class PUBSUBParticipantImpl;


class PUBSUBParticipant {
	friend class PUBSUBDomain;
	friend class PUBSUBParticipantImpl;
private:
	PUBSUBParticipant();
	virtual ~PUBSUBParticipant();


	PUBSUBParticipantImpl* mp_impl;

public:

	const GUID_t& getGuid()const ;

};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBSUBPARTICIPANT_H_ */
