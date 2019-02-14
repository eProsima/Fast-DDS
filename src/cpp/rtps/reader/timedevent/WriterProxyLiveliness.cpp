// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file WriterProxyLiveliness.cpp
 *
 */

#include <fastrtps/rtps/reader/timedevent/WriterProxyLiveliness.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/common/MatchingInfo.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>



namespace eprosima {
namespace fastrtps{
namespace rtps {


WriterProxyLiveliness::WriterProxyLiveliness(WriterProxy* p_WP,double interval):
TimedEvent(p_WP->mp_SFR->getRTPSParticipant()->getEventResource().getIOService(),
p_WP->mp_SFR->getRTPSParticipant()->getEventResource().getThread(), interval, TimedEvent::ON_SUCCESS),
mp_WP(p_WP)
{

}

WriterProxyLiveliness::~WriterProxyLiveliness()
{
    destroy();
}

void WriterProxyLiveliness::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
	
		logInfo(RTPS_LIVELINESS,"Deleting Writer: "<<mp_WP->m_att.guid);
//		if(!mp_WP->is_alive())
//		{
			//logWarning(RTPS_LIVELINESS,"Liveliness failed, leaseDuration was "<< this->getIntervalMilliSec()<< " ms");
			if(mp_WP->mp_SFR->matched_writer_remove(mp_WP->m_att,false))
			{
				if(mp_WP->mp_SFR->getListener()!=nullptr)
				{
					MatchingInfo info(REMOVED_MATCHING,mp_WP->m_att.guid);
					mp_WP->mp_SFR->getListener()->onReaderMatched((RTPSReader*)mp_WP->mp_SFR,info);
				}
			}

			mp_WP->mp_writerProxyLiveliness = nullptr;
			delete(mp_WP);
//		}
//		mp_WP->set_not_alive();
//		this->restart_timer();
	}
	else if(code == EVENT_ABORT)
	{
        logInfo(RTPS_LIVELINESS, "WriterProxyLiveliness aborted");
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"message: " <<msg);
	}
}


}
} /* namespace rtps */
} /* namespace eprosima */
