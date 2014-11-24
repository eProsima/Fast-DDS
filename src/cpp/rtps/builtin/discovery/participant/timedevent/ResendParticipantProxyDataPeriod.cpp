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

#include "fastrtps/builtin/discovery/RTPSParticipant/timedevent/ResendRTPSParticipantProxyDataPeriod.h"
#include "fastrtps/builtin/discovery/RTPSParticipant/PDPSimple.h"

#include "fastrtps/utils/RTPSLog.h"

//#include "fastrtps/resources/ResourceSend.h"
#include "fastrtps/resources/ResourceEvent.h"

//#include "fastrtps/RTPSMessageCreator.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ResendRTPSParticipantProxyDataPeriod";

ResendRTPSParticipantProxyDataPeriod::ResendRTPSParticipantProxyDataPeriod(PDPSimple* pPDP,ResourceEvent* pEvent,boost::posix_time::milliseconds interval):
		TimedEvent(&pEvent->io_service,interval),
		mp_PDP(pPDP)
{


}

ResendRTPSParticipantProxyDataPeriod::~ResendRTPSParticipantProxyDataPeriod()
{
	stop_timer();
	delete(timer);
}

void ResendRTPSParticipantProxyDataPeriod::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		logInfo(RTPS_PDP,"ResendDiscoveryData Period",EPRO_CYAN);
		//FIXME: Change for liveliness protocol
		mp_PDP->m_RTPSParticipantProxies.front()->m_manualLivelinessCount++;
		mp_PDP->announceRTPSParticipantState(false);

		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_PDP,"Response Data Period aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_PDP,"boost message: " <<ec.message());
	}
}


} /* namespace rtps */
} /* namespace eprosima */
