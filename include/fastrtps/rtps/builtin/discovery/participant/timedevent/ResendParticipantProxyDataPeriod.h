/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendRTPSParticipantProxyDataPeriod.h
 *
 */

#ifndef RESENDDATAPERIOD_H_
#define RESENDDATAPERIOD_H_


#include "fastrtps/utils/TimedEvent.h"
#include "fastrtps/common/types/CDRMessage_t.h"

namespace eprosima {
namespace rtps {

class PDPSimple;
class ResourceEvent;
/**
 * Class ResendRTPSParticipantProxyDataPeriod, TimedEvent used to periodically send the RTPSParticipantDiscovery Data.
 * @ingroup DISCOVERYMODULE
 */
class ResendRTPSParticipantProxyDataPeriod: public TimedEvent {
public:
	ResendRTPSParticipantProxyDataPeriod(PDPSimple* p_SPDP,
			ResourceEvent* pEvent,
			boost::posix_time::milliseconds interval);
	virtual ~ResendRTPSParticipantProxyDataPeriod();
	//!Temporal event that resends the RTPSParticipantProxyData to all remote RTPSParticipants.
	void event(const boost::system::error_code& ec);
	//!Auxiliar data message.
	CDRMessage_t m_data_msg;
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESENDDATAPERIOD_H_ */
