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
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/ResendDiscoveryDataPeriod.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/discovery/SimplePDP.h"

namespace eprosima {
namespace rtps {

ResendDiscoveryDataPeriod::ResendDiscoveryDataPeriod(SimplePDP* pSLW,boost::posix_time::milliseconds interval):
		TimedEvent(&pSLW->mp_SPDPWriter->mp_event_thr->io_service,interval),
		mp_SPDP(pSLW)
{


}

ResendDiscoveryDataPeriod::~ResendDiscoveryDataPeriod()
{
	timer->cancel();
	delete(timer);
}

void ResendDiscoveryDataPeriod::event(const boost::system::error_code& ec)
{
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("ResendDiscoveryData Period" << endl);
		//FIXME: Change for liveliness protocol
		mp_SPDP->announceParticipantState(false);
		m_isWaiting = false;
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
