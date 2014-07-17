/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLivelinessPeriodicAssertion.h
 *
 */

#ifndef WLIVELINESSPERIODICASSERTION_H_
#define WLIVELINESSPERIODICASSERTION_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/utils/TimedEvent.h"
#include "eprosimartps/qos/ParameterList.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class WLP;

class WLivelinessPeriodicAssertion: public TimedEvent {
public:
	WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind);
	virtual ~WLivelinessPeriodicAssertion();
	void event(const boost::system::error_code& ec);
	LivelinessQosPolicyKind m_livelinessKind;
	WLP* mp_WLP;

	bool AutomaticLivelinessAssertion();
	bool ManualByParticipantLivelinessAssertion();
	CDRMessage_t m_msg;
	InstanceHandle_t m_iHandle;
	GuidPrefix_t m_guidP;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLIVELINESSPERIODICASSERTION_H_ */
