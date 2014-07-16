/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendParticipantProxyDataPeriod
 *
 */

#ifndef RESENDDATAPERIOD_H_
#define RESENDDATAPERIOD_H_


#include "eprosimartps/utils/TimedEvent.h"
#include "eprosimartps/common/types/CDRMessage_t.h"

namespace eprosima {
namespace rtps {

class PDPSimple;
class ResourceEvent;
/**
 * Class ResendDiscoveryDataPeriod, TimedEvent used to periodically send the ParticipantDiscovery Data.
 * @ingroup DISCOVERYMODULE
 */
class ResendParticipantProxyDataPeriod: public TimedEvent {
public:
	ResendParticipantProxyDataPeriod(ParticipantDiscoveryProtocol* p_SPDP,
			ResourceEvent* pEvent,
			boost::posix_time::milliseconds interval);
	virtual ~ResendParticipantProxyDataPeriod();

	void event(const boost::system::error_code& ec);

	CDRMessage_t m_data_msg;
	PDPSimple* mp_PDP;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESENDDATAPERIOD_H_ */
