/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantLeaseDuration.cpp
 *
 */

#include "eprosimartps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h"
#include "eprosimartps/resources/ResourceEvent.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"
#include "eprosimartps/ParticipantProxyData.h"
#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

RemoteParticipantLeaseDuration::RemoteParticipantLeaseDuration(PDPSimple* pPDP,
		ParticipantProxyData* pdata,
		ResourceEvent* pEvent,
		boost::posix_time::milliseconds interval):
				TimedEvent(&pEvent->io_service,interval),
				mp_PDP(pPDP),
				mp_participantProxyData(pdata)
{

}

RemoteParticipantLeaseDuration::~RemoteParticipantLeaseDuration()
{
	stop_timer();
	delete(timer);
}

void RemoteParticipantLeaseDuration::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("ParticipantLeaseDuration Period" << endl);
		if(mp_participantProxyData->isAlive)
			mp_participantProxyData->isAlive = false;
		else
		{
			pWarning("Removing remote participant "<< mp_participantProxyData->m_guid << endl);
			mp_PDP->removeRemoteParticipant(mp_participantProxyData);
			return;
		}
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
