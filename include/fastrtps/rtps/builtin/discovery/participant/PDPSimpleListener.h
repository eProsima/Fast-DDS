/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.h
 *
 */

#ifndef PDPSIMPLELISTENER_H_
#define PDPSIMPLELISTENER_H_
#include "fastrtps/pubsub/SubscriberListener.h"
#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/RTPSParticipantProxyData.h"
using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

class SimplePDP;
class DiscoveredRTPSParticipantData;


/**
 * Class PDPSimpleListener, specification of SubscriberListener used by the SPDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 * @ingroup DISCOVERYMODULE
 */
class PDPSimpleListener: public SubscriberListener {
public:
	PDPSimpleListener(PDPSimple* in_SPDP):mp_SPDP(in_SPDP){};
	virtual ~PDPSimpleListener(){};
	//!Pointer to the associated mp_SPDP;
	PDPSimple* mp_SPDP;
	//!Method to be called when a new data message is received.l
	void onNewDataMessage();
	//!Process a new added cache with this method.
	bool newAddedCache();
	//FIXME: TO IMPLEMENT WHEN A StaticEDP is created.
	void assignUserId(std::string& type,uint16_t userId,EntityId_t& entityId,DiscoveredRTPSParticipantData* pdata);

	//!Temporal RTPSParticipantProxyData object used to read the messages.
	RTPSParticipantProxyData m_RTPSParticipantProxyData;
};



} /* namespace rtps */
} /* namespace eprosima */

#endif /* PDPSIMPLELISTENER_H_ */
