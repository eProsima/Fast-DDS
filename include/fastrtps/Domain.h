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


class Domain
{
	typedef std::pair<Participant*,ParticipantImpl*> t_p_Participant;
public:
	Domain();
	virtual ~Domain();
	static Participant* createParticipant(ParticipantAttributes& att,ParticipantListener* listen = nullptr);

	static Publisher* createPublisher(Participant* part,PublisherAttributes& att, PublisherListener* listen = nullptr);

	static Subscriber* createSubscriber(Participant* part,SubscriberAttributes& att, SubscriberListener* listen = nullptr);

	static std::vector<t_p_Participant> m_participants;


};

} /* namespace  */
} /* namespace eprosima */

#endif /* DOMAIN_H_ */
