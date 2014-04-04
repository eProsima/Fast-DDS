/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackSupressionDuration.cpp
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/NackSupressionDuration.h"
#include "eprosimartps/writer/StatefulWriter.h"
namespace eprosima {
namespace rtps {



NackSupressionDuration::~NackSupressionDuration()
{
	timer->cancel();
}

NackSupressionDuration::NackSupressionDuration(ReaderProxy* p_RP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_RP->mp_SFW->mp_event_thr->io_service,interval),
		mp_RP(p_RP)
{

}

void NackSupressionDuration::event(const boost::system::error_code& ec)
{
	if(ec == boost::system::errc::success)
	{
		boost::lock_guard<ReaderProxy> guard(*mp_RP);
		pDebugInfo("NackSupression: changing underway to unacked"<<endl);
		for(std::vector<ChangeForReader_t>::iterator cit=mp_RP->m_changesForReader.begin();
				cit!=mp_RP->m_changesForReader.end();++cit)
		{
			if(cit->status == UNDERWAY)
				cit->status = UNACKNOWLEDGED;
		}
		m_isWaiting = false;
	}
	else if(ec==boost::asio::error::operation_aborted)
		{
			pInfo("Nack Supression aborted"<<endl);
		}
		else
		{
			pInfo("Nack SUpression boost message: " <<ec.message()<<endl);
		}
}

} /* namespace dds */
} /* namespace eprosima */
