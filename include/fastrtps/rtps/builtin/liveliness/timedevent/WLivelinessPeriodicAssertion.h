/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLivelinessPeriodicAssertion.h
 *
 */

#ifndef WLIVELINESSPERIODICASSERTION_H_
#define WLIVELINESSPERIODICASSERTION_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/qos/QosPolicies.h"
#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/qos/ParameterList.h"

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

class WLP;

/**
 * Class WLivelinessPeriodicAssertion, used to assert the liveliness of the writers in a RTPSParticipant.
 * @ingroup LIVELINESS_MODULE
 */
class WLivelinessPeriodicAssertion: public TimedEvent {
public:
	/**
	* @param pwlp
	* @param kind
	*/
	WLivelinessPeriodicAssertion(WLP* pwlp,LivelinessQosPolicyKind kind);
	virtual ~WLivelinessPeriodicAssertion();
	/**
	* Method invoked when the event occurs
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);
	//!Liveliness Kind that is being asserted by this object.
	LivelinessQosPolicyKind m_livelinessKind;
	//!Pointer to the WLP object.
	WLP* mp_WLP;

	//!
	bool AutomaticLivelinessAssertion();
	//!
	bool ManualByRTPSParticipantLivelinessAssertion();
	//!
	CDRMessage_t m_msg;
	//!
	InstanceHandle_t m_iHandle;
	//!
	GuidPrefix_t m_guidP;
};

} /* namespace rtps */
} /* namespace eprosima */
}
#endif
#endif /* WLIVELINESSPERIODICASSERTION_H_ */
