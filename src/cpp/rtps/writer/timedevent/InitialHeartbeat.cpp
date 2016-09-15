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
 * @file InitialHeartbeat.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/InitialHeartbeat.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps{


InitialHeartbeat::~InitialHeartbeat()
{
	logInfo(RTPS_WRITER,"Destroying InitialHB");
    destroy();
}

InitialHeartbeat::InitialHeartbeat(ReaderProxy* rp, double interval) :
    TimedEvent(rp->mp_SFW->getRTPSParticipant()->getEventResource().getIOService(),
        rp->mp_SFW->getRTPSParticipant()->getEventResource().getThread(), interval),
    rp_(rp)
{
}

void InitialHeartbeat::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
		SequenceNumber_t firstSeq, lastSeq;
		Count_t heartbeatCount = 0;

		{//BEGIN PROTECTION
            boost::lock_guard<boost::recursive_mutex> guard_writer(*rp_->mp_SFW->getMutex());

			firstSeq = rp_->mp_SFW->get_seq_num_min();
			lastSeq = rp_->mp_SFW->get_seq_num_max();

            if(firstSeq == c_SequenceNumber_Unknown || lastSeq == c_SequenceNumber_Unknown)
            {
				firstSeq = rp_->mp_SFW->next_sequence_number();
                lastSeq = SequenceNumber_t(0, 0);
            }
            else
            {
                (void)firstSeq;
                assert(firstSeq <= lastSeq);
            }

            rp_->mp_SFW->incrementHBCount();
			heartbeatCount = rp_->mp_SFW->getHeartbeatCount();
		}

        CDRMessage::initCDRMsg(&initial_hb_msg_);
        // FinalFlag is always false because this is a StatefulWriter in Reliable.
        RTPSMessageCreator::addMessageHeartbeat(&initial_hb_msg_, rp_->mp_SFW->getGuid().guidPrefix, rp_->m_att.guid.guidPrefix,
                rp_->m_att.guid.entityId, rp_->mp_SFW->getGuid().entityId,
                firstSeq, lastSeq, heartbeatCount, false, false);
        logInfo(RTPS_WRITER, rp_->mp_SFW->getGuid().entityId << " Sending Heartbeat (" << firstSeq << " - " << lastSeq << ")");
        for (auto lit = rp_->m_att.endpoint.multicastLocatorList.begin(); lit != rp_->m_att.endpoint.multicastLocatorList.end(); ++lit)
            rp_->mp_SFW->getRTPSParticipant()->sendSync(&initial_hb_msg_, (Endpoint *)rp_->mp_SFW, (*lit));
        for (auto lit = rp_->m_att.endpoint.unicastLocatorList.begin(); lit != rp_->m_att.endpoint.unicastLocatorList.end(); ++lit)
            rp_->mp_SFW->getRTPSParticipant()->sendSync(&initial_hb_msg_, (Endpoint *)rp_->mp_SFW, (*lit));
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Aborted");
	}
	else
	{
		logInfo(RTPS_WRITER,"Boost message: " <<msg);
	}
}

}
}
} /* namespace eprosima */
