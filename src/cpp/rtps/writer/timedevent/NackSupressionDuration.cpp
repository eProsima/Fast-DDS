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

#include "eprosimartps/rtps/writer/timedevent/NackSupressionDuration.h"
#include "eprosimartps/rtps/writer/StatefulWriter.h"
#include "eprosimartps/rtps/writer/ReaderProxy.h"
#include "eprosimartps/rtps/ParticipantImpl.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "NackSupressionDuration";

NackSupressionDuration::~NackSupressionDuration()
{

}

NackSupressionDuration::NackSupressionDuration(ReaderProxy* p_RP,double millisec):
				TimedEvent(p_RP->mp_SFW->getParticipant()->getIOService(),millisec),
				mp_RP(p_RP)
{

}

void NackSupressionDuration::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";
	if(code == EVENT_SUCCESS)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*mp_RP->mp_mutex);
		logInfo(RTPS_WRITER,"NackSupression: changing underway to unacked for Reader: "<<mp_RP->m_att.guid);
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

} /* namespace dds */
} /* namespace eprosima */
