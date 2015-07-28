/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackSupressionDuration.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include "../../participant/RTPSParticipantImpl.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "NackSupressionDuration";

NackSupressionDuration::~NackSupressionDuration()
{
	stop_timer();
}

NackSupressionDuration::NackSupressionDuration(ReaderProxy* p_RP,double millisec):
				TimedEvent(p_RP->mp_SFW->getRTPSParticipant()->getIOService(),millisec),
				mp_RP(p_RP)
{

}

void NackSupressionDuration::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";
	if(code == EVENT_SUCCESS)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*mp_RP->mp_mutex);
		logInfo(RTPS_WRITER,"Changing underway to unacked for Reader: "<<mp_RP->m_att.guid);
		for(std::vector<ChangeForReader_t>::iterator cit=mp_RP->m_changesForReader.begin();
				cit!=mp_RP->m_changesForReader.end();++cit)
		{
			if(cit->status == UNDERWAY)
			{
				if(mp_RP->m_att.endpoint.reliabilityKind == RELIABLE)
					cit->status = UNACKNOWLEDGED;
				else
					cit->status = ACKNOWLEDGED;
			}
		}

	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Aborted");
		this->stopSemaphorePost();
	}
	else
	{
		logInfo(RTPS_WRITER,"Boost message: " <<msg);
	}
}
}
} /* namespace dds */
} /* namespace eprosima */
