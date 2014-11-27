/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendDataPeriod.cpp
 *
 */

#include "fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h"
#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/builtin/data/ParticipantProxyData.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"

#include "fastrtps/utils/RTPSLog.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "ResendParticipantProxyDataPeriod";

ResendParticipantProxyDataPeriod::ResendParticipantProxyDataPeriod(PDPSimple* p_SPDP,
		double interval):
				TimedEvent(p_SPDP->getRTPSParticipant()->getIOService(),interval),
				mp_PDP(p_SPDP)
{


}

ResendParticipantProxyDataPeriod::~ResendParticipantProxyDataPeriod()
{
	stop_timer();
}

void ResendParticipantProxyDataPeriod::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";
	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_PDP,"ResendDiscoveryData Period",C_CYAN);
		//FIXME: Change for liveliness protocol
		mp_PDP->getLocalParticipantProxyData()->m_manualLivelinessCount++;
		mp_PDP->announceParticipantState(false);

		this->restart_timer();
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_PDP,"Response Data Period aborted");
		this->stopSemaphorePost();
	}
	else
	{
		logInfo(RTPS_PDP,"boost message: " <<msg);
	}
}

}
} /* namespace rtps */
} /* namespace eprosima */
