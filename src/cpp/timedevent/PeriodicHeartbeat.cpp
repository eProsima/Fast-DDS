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

PeriodicHeartbeat::PeriodicHeartbeat() {
	// TODO Auto-generated constructor stub

}

PeriodicHeartbeat::~PeriodicHeartbeat() {
	// TODO Auto-generated destructor stub
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SW_ptr->eventTh.io_service,interval),
		SW(SW_ptr)
{

}

void PeriodicHeartbeat::event(const boost::system::error_code& ec,ReaderProxy* RP)
{
	if(!ec)
	{
		boost::lock_guard<ReaderProxy> guard(*RP);
		std::vector<ChangeForReader_t*> unack;
		RP->unacked_changes(&unack);
		if(!unack.empty())
		{
			CDRMessage_t msg;
			SequenceNumber_t first,last;
			SW->writer_cache.get_seq_num_min(&first,NULL);
			SW->writer_cache.get_seq_num_max(&last,NULL);
			SW->heartbeatCount++;
			RTPSMessageCreator::createMessageHeartbeat(&msg,SW->participant->guid.guidPrefix,ENTITYID_UNKNOWN,SW->guid.entityId,
					first,last,SW->heartbeatCount,false,false);
			std::vector<Locator_t>::iterator lit;
			for(lit = RP->param.unicastLocatorList.begin();lit!=RP->param.unicastLocatorList.end();lit++)
				SW->participant->threadSend.sendSync(&msg,*lit);
			for(lit = RP->param.multicastLocatorList.begin();lit!=RP->param.multicastLocatorList.end();lit++)
				SW->participant->threadSend.sendSync(&msg,*lit);
		}
	}
	//Reset TIMER, the cancellation is managed in the receiving thread,
	//when an acknack msg acknowledges all cache changes.
	timer->async_wait(boost::bind(&PeriodicHeartbeat::event,&SW->periodicHB,
			boost::asio::placeholders::error,RP));

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
