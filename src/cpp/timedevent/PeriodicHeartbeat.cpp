/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/PeriodicHeartbeat.h"
#include "eprosimartps/writer/StatefulWriter.h"


namespace eprosima {
namespace rtps{



PeriodicHeartbeat::~PeriodicHeartbeat() {
	// TODO Auto-generated destructor stub
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SW_ptr->participant->m_event_thr.io_service,interval),
		SW(SW_ptr)
{

}

void PeriodicHeartbeat::event(const boost::system::error_code& ec,ReaderProxy* RP)
{
	if(!ec)
	{

		std::vector<ChangeForReader_t*> unack;
		{
			RP->unacked_changes(&unack);
		}
		if(!unack.empty())
		{
			SequenceNumber_t first,last;
			SW->m_writer_cache.get_seq_num_min(&first,NULL);
			SW->m_writer_cache.get_seq_num_max(&last,NULL);
			SW->heartbeatCount_increment();
			CDRMessage::initCDRMsg(&m_periodic_hb_msg);
			RTPSMessageCreator::addMessageHeartbeat(&m_periodic_hb_msg,SW->participant->m_guid.guidPrefix,ENTITYID_UNKNOWN,SW->guid.entityId,
					first,last,SW->getHeartbeatCount(),false,false);
			std::vector<Locator_t>::iterator lit;
			for(lit = RP->param.unicastLocatorList.begin();lit!=RP->param.unicastLocatorList.end();++lit)
				SW->participant->m_send_thr.sendSync(&m_periodic_hb_msg,&(*lit));
			for(lit = RP->param.multicastLocatorList.begin();lit!=RP->param.multicastLocatorList.end();++lit)
				SW->participant->m_send_thr.sendSync(&m_periodic_hb_msg,&(*lit));
			//Reset TIMER
			if(SW->reliability.heartbeatPeriod.to64time() > 0)
				timer->async_wait(boost::bind(&PeriodicHeartbeat::event,&RP->periodicHB,
						boost::asio::placeholders::error,RP));
		}
	}


	if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("Periodic Heartbeat aborted");
	}
	else
	{
		pError(ec.message()<<endl);
	}
}


}
} /* namespace eprosima */
