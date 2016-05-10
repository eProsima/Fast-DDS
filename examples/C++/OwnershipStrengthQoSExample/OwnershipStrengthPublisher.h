/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file OwnershipStrengthPublisher.h
 *
 */

#ifndef OWNERSHIPSTRENGTHPUBLISHER_H_
#define OWNERSHIPSTRENGTHPUBLISHER_H_

#include "OwnershipStrengthPubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
using namespace eprosima::fastrtps;

#include "OwnershipStrength.h"

class OwnershipStrengthPublisher {
public:
	OwnershipStrengthPublisher(int strength);
	virtual ~OwnershipStrengthPublisher();
	//!Initialize
	bool init();
	//!Publish a sample
	bool publish();
	//!Run for number samples
	void run(uint32_t number);
private:
	OwnershipStrength m_Hello;
	Participant* mp_participant;
	Publisher* mp_publisher;
	class PubListener:public PublisherListener
	{
	public:
		PubListener():n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		int n_matched;
	}m_listener;
	OwnershipStrengthType m_type;
   int m_strength;
};



#endif /* OWNERSHIPSTRENGTHPUBLISHER_H_ */
