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

#include "eprosimartps/writer/timedevent/PeriodicHeartbeat.h"
#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/RTPSMessageCreator.h"


namespace eprosima {
namespace rtps{

static const char* const CLASS_NAME = "PeriodicHeartbeat";

PeriodicHeartbeat::~PeriodicHeartbeat()
{
	stop_timer();
	delete(timer);
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* p_SFW,boost::posix_time::milliseconds interval):
		TimedEvent(&p_SFW->mp_event_thr->io_service,interval),
		mp_SFW(p_SFW)
{

}

void PeriodicHeartbeat::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	logInfo(RTPS_WRITER,"TimedEvent: PeriodicHeartbeat";);
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		logInfo(RTPS_WRITER,"Sending Heartbeat");
		std::vector<ChangeForReader_t*> unack;
		bool unacked_changes = false;
		for(std::vector<ReaderProxy*>::iterator it = mp_SFW->matchedReadersBegin();
				it!=mp_SFW->matchedReadersEnd();++it)
		{
			unack.clear();
			(*it)->unacked_changes(&unack);
			if(!unack.empty())
			{
				unacked_changes= true;
				break;
			}
		}
		if(unacked_changes)
		{
			SequenceNumber_t first,last;
			mp_SFW->get_seq_num_min(&first,NULL);
			mp_SFW->get_seq_num_max(&last,NULL);
			if(first.to64long()>0 && last.to64long() >= first.to64long())
			{
			mp_SFW->incrementHBCount();
			CDRMessage::initCDRMsg(&m_periodic_hb_msg);
			RTPSMessageCreator::addMessageHeartbeat(&m_periodic_hb_msg,mp_SFW->getGuid().guidPrefix,
													mp_SFW->getHBReaderEntityId(),mp_SFW->getGuid().entityId,
													first,last,mp_SFW->getHeartbeatCount(),false,false);
			std::vector<Locator_t>::iterator lit;
			for(std::vector<ReaderProxy*>::iterator rit = mp_SFW->matchedReadersBegin();
					rit!=mp_SFW->matchedReadersEnd();++rit)
			{
				for(lit = (*rit)->m_data->m_unicastLocatorList.begin();lit!=(*rit)->m_data->m_unicastLocatorList.end();++lit)
					mp_SFW->mp_send_thr->sendSync(&m_periodic_hb_msg,(*lit));
				for(lit = (*rit)->m_data->m_multicastLocatorList.begin();lit!=(*rit)->m_data->m_multicastLocatorList.end();++lit)
					mp_SFW->mp_send_thr->sendSync(&m_periodic_hb_msg,(*lit));
			}
			}
			//Reset TIMER
			this->restart_timer();
		}

	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logWarning(RTPS_WRITER,"Periodic Heartbeat aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_WRITER,"Periodic Heartbeat boost message: " <<ec.message());
	}
}


}
} /* namespace eprosima */
