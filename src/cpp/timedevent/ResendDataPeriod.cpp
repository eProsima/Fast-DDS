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

#include "eprosimartps/timedevent/ResendDataPeriod.h"
#include "eprosimartps/writer/StatelessWriter.h"

namespace eprosima {
namespace rtps {

ResendDataPeriod::ResendDataPeriod(StatelessWriter* pSLW,boost::posix_time::milliseconds interval):
		TimedEvent(&pSLW->mp_event_thr->io_service,interval),
		mp_SLW(pSLW)
{


}

ResendDataPeriod::~ResendDataPeriod()
{
	timer->cancel();
}

void ResendDataPeriod::event(const boost::system::error_code& ec)
{
	if(ec == boost::system::errc::success)
	{

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
