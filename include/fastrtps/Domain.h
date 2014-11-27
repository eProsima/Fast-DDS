/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Domain.h
 *
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include "fastrtps/attributes/ParticipantAttributes.h"

namespace eprosima{
namespace fastrtps{


class ParticipantListener;
class Participant;
class ParticipantImpl;
class Publisher;
class PublisherAttributes;
class PublisherListener;
class Subscriber;
class SubscriberAttributes;
class SubscriberListener;
class TopicDataType;


class Domain
{
	typedef std::pair<Participant*,ParticipantImpl*> t_p_Participant;

	Domain();
	virtual ~Domain();
public:
	RTPS_DllAPI static Participant* createParticipant(ParticipantAttributes& att, ParticipantListener* listen = nullptr);

	RTPS_DllAPI static Publisher* createPublisher(Participant* part, PublisherAttributes& att, PublisherListener* listen = nullptr);

	RTPS_DllAPI static Subscriber* createSubscriber(Participant* part, SubscriberAttributes& att, SubscriberListener* listen = nullptr);

	RTPS_DllAPI static bool removeParticipant(Participant* part);

	RTPS_DllAPI static bool removePublisher(Publisher* pub);

	RTPS_DllAPI static bool removeSubscriber(Subscriber* sub);

	RTPS_DllAPI static bool registerType(Participant* part, TopicDataType * type);

	RTPS_DllAPI static void stopAll();
private:
	static std::vector<t_p_Participant> m_participants;


};

} /* namespace  */
} /* namespace eprosima */

#endif /* DOMAIN_H_ */
