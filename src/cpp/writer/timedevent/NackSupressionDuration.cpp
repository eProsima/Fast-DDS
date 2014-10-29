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
 */

#include "eprosimartps/writer/timedevent/NackSupressionDuration.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/writer/ReaderProxyData.h"

//#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "NackSupressionDuration";

NackSupressionDuration::~NackSupressionDuration()
{
	stop_timer();
	delete(timer);
}

NackSupressionDuration::NackSupressionDuration(ReaderProxy* p_RP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_RP->mp_SFW->mp_event_thr->io_service,interval),
		mp_RP(p_RP)
{

}

void NackSupressionDuration::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		boost::lock_guard<ReaderProxy> guard(*mp_RP);
		logInfo(RTPS_WRITER,"NackSupression: changing underway to unacked for Reader: "<<mp_RP->m_data->m_guid);
		for(std::vector<ChangeForReader_t>::iterator cit=mp_RP->m_changesForReader.begin();
				cit!=mp_RP->m_changesForReader.end();++cit)
		{
			if(cit->status == UNDERWAY)
			{
				if(mp_RP->m_data->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
					cit->status = UNACKNOWLEDGED;
				else
					cit->status = ACKNOWLEDGED;
			}
		}

	}
	else if(ec==boost::asio::error::operation_aborted)
		{
			logInfo(RTPS_WRITER,"Nack Supression aborted");
			this->mp_stopSemaphore->post();
		}
		else
		{
			logInfo(RTPS_WRITER,"Nack SUpression boost message: " <<ec.message());
		}
}

} /* namespace dds */
} /* namespace eprosima */
