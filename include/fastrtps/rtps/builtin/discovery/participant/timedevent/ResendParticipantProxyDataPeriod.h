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


#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/common/CDRMessage_t.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class PDPSimple;

/**
 * Class ResendRTPSParticipantProxyDataPeriod, TimedEvent used to periodically send the RTPSParticipantDiscovery Data.
 * @ingroup DISCOVERYMODULE
 */
class ResendParticipantProxyDataPeriod: public TimedEvent {
public:
	ResendParticipantProxyDataPeriod(PDPSimple* p_SPDP,
			double interval);
	virtual ~ResendParticipantProxyDataPeriod();
	//!Temporal event that resends the RTPSParticipantProxyData to all remote RTPSParticipants.
	void event(EventCode code, const char* msg= nullptr);
	//!Auxiliar data message.
	CDRMessage_t m_data_msg;
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESENDDATAPERIOD_H_ */
