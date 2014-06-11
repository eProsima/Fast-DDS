/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LivelinessPeriodicAssertion.h

 */

#ifndef LIVELINESSPERIODICASSERTION_H_
#define LIVELINESSPERIODICASSERTION_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/timedevent/TimedEvent.h"
#include "eprosimartps/qos/ParameterList.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class WriterLiveliness;

class LivelinessPeriodicAssertion : public TimedEvent {
public:
	LivelinessPeriodicAssertion(WriterLiveliness* wLiveliness,LivelinessQosPolicyKind kind);
	virtual ~LivelinessPeriodicAssertion();
	void event(const boost::system::error_code& ec);
	LivelinessQosPolicyKind m_livelinessKind;
	WriterLiveliness* mp_writerLiveliness;
	bool AutomaticLivelinessAssertion();
	bool ManualByParticipantLivelinessAssertion();
	CDRMessage_t m_msg;
	InstanceHandle_t m_iHandle;
	bool first;
	GuidPrefix_t m_guidP;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* LIVELINESSPERIODICASSERTION_H_ */
