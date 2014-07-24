/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendDataPeriod.cpp
 *
 */

#include "eprosimartps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/utils/RTPSLog.h"

//#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

//#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {

ResendParticipantProxyDataPeriod::ResendParticipantProxyDataPeriod(PDPSimple* pPDP,ResourceEvent* pEvent,boost::posix_time::milliseconds interval):
		TimedEvent(&pEvent->io_service,interval),
		mp_PDP(pPDP)
{


}

ResendParticipantProxyDataPeriod::~ResendParticipantProxyDataPeriod()
{
	stop_timer();
	delete(timer);
}

void ResendParticipantProxyDataPeriod::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("ResendDiscoveryData Period" << endl);
		//FIXME: Change for liveliness protocol
		mp_PDP->m_participantProxies.front()->m_manualLivelinessCount++;
		mp_PDP->announceParticipantState(true);

		this->restart_timer();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("Response Data Period aborted"<<endl);
		this->mp_stopSemaphore->post();
	}
	else
	{
		pInfo("Response Data Period boost message: " <<ec.message()<<endl);
	}
}


} /* namespace rtps */
} /* namespace eprosima */
