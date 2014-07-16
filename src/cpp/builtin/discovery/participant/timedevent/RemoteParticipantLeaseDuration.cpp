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
		const GUID_t& pguid,
		ResourceEvent* pEvent,
		boost::posix_time::milliseconds interval):
				TimedEvent(&pEvent->io_service,interval),
				mp_PDP(pPDP),
				m_remoteParticipantGuid(pguid)
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
		//FIXME: Change for liveliness protocol
		std::vector<ParticipantProxyData*>::iterator pit;
		bool found = false;
		for( pit = mp_PDP->m_participantProxies.begin();
				pit!= mp_PDP->m_participantProxies.end();++pit)
		{
			if((*pit)->m_guid == m_remoteParticipantGuid)
			{
				found = true;
				break;
			}
		}
		if(found)
		{
			if((*pit)->isAlive)
			{
				(*pit)->isAlive = false;
			}
			else
			{
				pWarning("Removing remote participant "<< m_remoteParticipantGuid << endl);
				mp_PDP->removeRemoteParticipant(m_remoteParticipantGuid);
				return;
			}
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
