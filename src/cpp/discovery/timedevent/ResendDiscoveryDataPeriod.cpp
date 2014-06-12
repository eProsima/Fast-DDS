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

#include "eprosimartps/discovery/timedevent/ResendDiscoveryDataPeriod.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {

ResendDiscoveryDataPeriod::ResendDiscoveryDataPeriod(ParticipantDiscoveryProtocol* pPDP,ResourceEvent* pEvent,boost::posix_time::milliseconds interval):
		TimedEvent(&pEvent->io_service,interval),
		mp_PDP(pPDP)
{


}

ResendDiscoveryDataPeriod::~ResendDiscoveryDataPeriod()
{
	timer->cancel();
	delete(timer);
}

void ResendDiscoveryDataPeriod::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("ResendDiscoveryData Period" << endl);
		//FIXME: Change for liveliness protocol
		mp_PDP->announceParticipantState(false);

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
