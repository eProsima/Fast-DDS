/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.h
 *
 */

#ifndef PDPSIMPLELISTENER_H_
#define PDPSIMPLELISTENER_H_
#include "eprosimartps/dds/SubscriberListener.h"
#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/ParticipantProxyData.h"
using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class SimplePDP;
class DiscoveredParticipantData;


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
	//!Method to be called when a new data message is received.
	void onNewDataMessage();
	//!Process a new added cache with this method.
	bool newAddedCache();

	void assignUserId(std::string& type,uint16_t userId,EntityId_t& entityId,DiscoveredParticipantData* pdata);

	ParticipantProxyData m_participantProxyData;
};



} /* namespace rtps */
} /* namespace eprosima */

#endif /* PDPSIMPLELISTENER_H_ */
