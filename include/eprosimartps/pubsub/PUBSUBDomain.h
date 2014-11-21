/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBDomain.h
 *
 */

#ifndef PUBSUBDOMAIN_H_
#define PUBSUBDOMAIN_H_

#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

namespace eprosima
{
namespace rtps
{
class RTPSParticipantListener;
}
}

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

class PUBSUBParticipant;
class PUBSUBParticipantImpl;
class Publisher;
class PublisherAttributes;
class PublisherListener;

class PUBSUBDomain
{
	typedef std::pair<PUBSUBParticipant*,PUBSUBParticipantImpl*> t_p_PUBSUBParticipant;
public:
	PUBSUBDomain();
	virtual ~PUBSUBDomain();
	static PUBSUBParticipant* createParticipant(RTPSParticipantAttributes& att,RTPSParticipantListener* listen = nullptr);

	static Publisher* createPublisher(PUBSUBParticipant* part,PublisherAttributes& att, PublisherListener* listen = nullptr);

	static std::vector<t_p_PUBSUBParticipant> m_pubsubParticipants;


};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBSUBDOMAIN_H_ */
