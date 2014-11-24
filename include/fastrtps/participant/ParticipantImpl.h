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
#include "fastrtps/rtps/participant/RTPSParticipantListener.h"
#include "fastrtps/attributes/ParticipantAttributes.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{
class RTPSParticipant;
}

using namespace rtps;

class Participant;
class ParticipantListener;

class TopicDataType;
class Publisher;
class PublisherImpl;
class PublisherAttributes;
class PublisherListener;
class Subscriber;
class SubscriberImpl;
class SubscriberAttributes;
class SubscriberListener;



class ParticipantImpl {
	friend class Domain;
	typedef std::pair<Publisher*,PublisherImpl*> t_p_PublisherPair;
	typedef std::pair<Subscriber*,SubscriberImpl*> t_p_SubscriberPair;
	typedef std::vector<t_p_PublisherPair> t_v_PublisherPairs;
	typedef std::vector<t_p_SubscriberPair> t_v_SubscriberPairs;
private:
	ParticipantImpl(ParticipantAttributes& patt,Participant* pspart,ParticipantListener* listen = nullptr);
	virtual ~ParticipantImpl();

public:

	bool registerType(TopicDataType* type);

	Publisher* createPublisher(PublisherAttributes& att, PublisherListener* listen=nullptr);

	Subscriber* createSubscriber(SubscriberAttributes& att, SubscriberListener* listen=nullptr);

	const GUID_t& getGuid() const;

private:
	//!Participant Attributes
	ParticipantAttributes m_att;
	//!RTPSParticipant
	RTPSParticipant* mp_rtpsParticipant;
	//!Participant*
	Participant* mp_participant;
	//!Participant Listener
	ParticipantListener* mp_listener;
	//!Publisher Vector
	t_v_PublisherPairs m_publishers;
	//!Subscriber Vector
	t_v_SubscriberPairs m_subscribers;
	//!TOpicDatType vector
	std::vector<TopicDataType*> m_types;

	bool getRegisteredType(const char* typeName, TopicDataType** type);

	class MyRTPSParticipantListener : public RTPSParticipantListener
	{
		MyRTPSParticipantListener(ParticipantImpl* impl): mp_participantimpl(impl){};
		virtual ~MyRTPSParticipantListener(){};
		void onRTPSParticipantDiscovery(RTPSParticipant* part, RTPSParticipantDiscoveryInfo info);
		ParticipantImpl* mp_participantimpl;
	}m_rtps_listener;

};

} /* namespace  */
} /* namespace eprosima */

#endif /* PARTICIPANTIMPL_H_ */
