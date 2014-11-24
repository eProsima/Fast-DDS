/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantImpl.h
 *
 */

#ifndef PARTICIPANTIMPL_H_
#define PARTICIPANTIMPL_H_

#include "fastrtps/rtps/common/Guid.h"

#include "fastrtps/attributes/ParticipantAttributes.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{
class RTPSParticipant;
}
}
}

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

class TopicDataType;
class Publisher;
class PublisherImpl;
class PublisherAttributes;
class PublisherListener;
class Participant;

class ParticipantImpl {
	friend class Domain;
	typedef std::pair<Publisher*,PublisherImpl*> t_p_PublisherPair;
	typedef std::vector<t_p_PublisherPair> t_v_PublisherPairs;
private:
	ParticipantImpl(ParticipantAttributes& patt,Participant* pspart,RTPSParticipant* part);
	virtual ~ParticipantImpl();

public:

	bool registerType(TopicDataType* type);

	Publisher* createPublisher(PublisherAttributes& att, PublisherListener* listen=nullptr);

	const GUID_t& getGuid() const;

private:
	//!Participant Attributes
	ParticipantAttributes m_att;
	//!RTPSParticipant
	RTPSParticipant* mp_rtpsParticipant;
	//!Participant*
	Participant* mp_Participant;
	//!Publisher Vector
	t_v_PublisherPairs m_publishers;
	//!TOpicDatType vector
	std::vector<TopicDataType*> m_types;

	bool getRegisteredType(const char* typeName, TopicDataType** type);

};

} /* namespace  */
} /* namespace eprosima */

#endif /* PARTICIPANTIMPL_H_ */
