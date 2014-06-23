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

#include "eprosimartps/discovery/timedevent/ParticipantLeaseDuration.h"
#include "eprosimartps/resources/ResourceEvent.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

ParticipantLeaseDuration::ParticipantLeaseDuration(ParticipantDiscoveryProtocol* pPDP,
		const GuidPrefix_t& pguid,
		ResourceEvent* pEvent,
		boost::posix_time::milliseconds interval):
				TimedEvent(&pEvent->io_service,interval),
				mp_PDP(pPDP),
				m_remoteParticipantGuid(pguid)
{

}

ParticipantLeaseDuration::~ParticipantLeaseDuration()
{
	timer->cancel();
	delete(timer);
}

void ParticipantLeaseDuration::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("ParticipantLeaseDuration Period" << endl);
		//FIXME: Change for liveliness protocol
		std::vector<DiscoveredParticipantData*>::iterator pit;
		bool found = false;
		for( pit = mp_PDP->m_discoveredParticipants.begin();
				pit!= mp_PDP->m_discoveredParticipants.end();++pit)
		{
			if((*pit)->m_guidPrefix == m_remoteParticipantGuid)
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
	}
	else
	{
		pInfo("Response Data Period boost message: " <<ec.message()<<endl);
	}
}


} /* namespace rtps */
} /* namespace eprosima */
