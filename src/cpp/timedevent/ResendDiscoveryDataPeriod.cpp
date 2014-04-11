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

namespace eprosima {
namespace rtps {

ResendDiscoveryDataPeriod::ResendDiscoveryDataPeriod(SimpleParticipantDiscoveryProtocol* pSLW,boost::posix_time::milliseconds interval):
		TimedEvent(&pSLW->m_SPDPbPWriter->mp_event_thr->io_service,interval),
		mp_SPDP(pSLW)
{


}

ResendDiscoveryDataPeriod::~ResendDiscoveryDataPeriod()
{
	timer->cancel();
}

void ResendDiscoveryDataPeriod::event(const boost::system::error_code& ec)
{
	if(ec == boost::system::errc::success)
	{
		mp_SPDP->sendDPDMsg();
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
