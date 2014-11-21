/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBParticipantImpl.h
 *
 */

#ifndef PUBSUBPARTICIPANTIMPL_H_
#define PUBSUBPARTICIPANTIMPL_H_

#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

namespace eprosima
{
namespace rtps{
class RTPSParticipant;
}
}

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

class TopicDataType;
class Publisher;
class PublisherImpl;
class PublisherAttributes;
class PublisherListener;

class PUBSUBParticipantImpl {
	friend class PUBSUBDomain;
	typedef std::pair<Publisher*,PublisherImpl*> t_p_PublisherPair;
	typedef std::vector<t_p_PublisherPair> t_v_PublisherPairs;
private:
	PUBSUBParticipantImpl(RTPSParticipantAttributes& patt,RTPSParticipant* part);
	virtual ~PUBSUBParticipantImpl();

public:

	bool registerType(TopicDataType* type);

	Publisher* createPublisher(PublisherAttributes& att, PublisherListener* listen=nullptr);


private:
	//!Participant Attributes
	RTPSParticipantAttributes m_att;
	//!RTPSParticipant
	RTPSParticipant* mp_rtpsParticipant;
	//!Publisher Vector
	t_v_PublisherPairs m_publishers;
	//!TOpicDatType vector
	std::vector<TopicDataType*> m_types;

	bool getRegisteredType(const char* typeName, TopicDataType** type);

};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBSUBPARTICIPANTIMPL_H_ */
