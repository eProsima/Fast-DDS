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
 */

#include "eprosimartps/timedevent/PeriodicHeartbeat.h"



#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/RTPSMessageCreator.h"


namespace eprosima {
namespace rtps{



PeriodicHeartbeat::~PeriodicHeartbeat()
{
	timer->cancel();
	delete(timer);
}

PeriodicHeartbeat::PeriodicHeartbeat(ReaderProxy* p_RP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_RP->mp_SFW->mp_event_thr->io_service,interval),
		mp_RP(p_RP)
{

}

void PeriodicHeartbeat::event(const boost::system::error_code& ec)
{
	pDebugInfo("TimedEvent: PeriodicHeartbeat"<<endl;);
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("Sending Heartbeat"<<endl);
		std::vector<ChangeForReader_t*> unack;
		mp_RP->unacked_changes(&unack);
		//cout << "Unacked changes: "<< unack.size()<<endl;
		if(!unack.empty())
		{
			SequenceNumber_t first,last;
			mp_RP->mp_SFW->get_seq_num_min(&first,NULL);
			mp_RP->mp_SFW->get_seq_num_max(&last,NULL);
			mp_RP->mp_SFW->incrementHBCount();
			CDRMessage::initCDRMsg(&m_periodic_hb_msg);
			RTPSMessageCreator::addMessageHeartbeat(&m_periodic_hb_msg,mp_RP->mp_SFW->getGuid().guidPrefix,
													c_EntityId_Unknown,mp_RP->mp_SFW->getGuid().entityId,
													first,last,mp_RP->mp_SFW->getHeartbeatCount(),false,false);
			std::vector<Locator_t>::iterator lit;
			for(lit = mp_RP->m_param.unicastLocatorList.begin();lit!=mp_RP->m_param.unicastLocatorList.end();++lit)
				mp_RP->mp_SFW->mp_send_thr->sendSync(&m_periodic_hb_msg,(*lit));
			for(lit = mp_RP->m_param.multicastLocatorList.begin();lit!=mp_RP->m_param.multicastLocatorList.end();++lit)
				mp_RP->mp_SFW->mp_send_thr->sendSync(&m_periodic_hb_msg,(*lit));



			//Reset TIMER
			this->restart_timer();
		}

	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pWarning("Periodic Heartbeat aborted"<<endl);
	}
	else
	{
		pInfo("Periodic Heartbeat boost message: " <<ec.message()<<endl);
	}
}


}
} /* namespace eprosima */
