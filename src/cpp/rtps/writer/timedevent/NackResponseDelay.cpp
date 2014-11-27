/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.cpp
 *
 */

#include "fastrtps/rtps/writer/timedevent/NackResponseDelay.h"

#include "fastrtps/rtps/writer/StatefulWriter.h"
#include "fastrtps/rtps/writer/ReaderProxy.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"

#include "fastrtps/utils/RTPSLog.h"

#include "fastrtps/rtps/messages/RTPSMessageCreator.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "NackResponseDelay";

NackResponseDelay::~NackResponseDelay()
{
	stop_timer();
}

NackResponseDelay::NackResponseDelay(ReaderProxy* p_RP,double millisec):
						TimedEvent(p_RP->mp_SFW->getRTPSParticipant()->getIOService(),millisec),
						mp_RP(p_RP)
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,mp_RP->mp_SFW->getGuid().guidPrefix);
}

bool sort_chFR (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->seqNum < c2->seqNum);
}

void NackResponseDelay::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";
	if(code == EVENT_SUCCESS)
	{
		logInfo(RTPS_WRITER,"Responding to Acknack msg";);
		boost::lock_guard<boost::recursive_mutex> guard(*mp_RP->mp_mutex);
		std::vector<ChangeForReader_t*> ch_vec;
		if(mp_RP->requested_changes(&ch_vec))
		{
			logInfo(RTPS_WRITER,"Requested "<<ch_vec.size() << " changes";);
			//cout << "REQUESTED CHANGES SIZE: "<< ch_vec.size() << endl;
			//	std::sort(ch_vec.begin(),ch_vec.end(),sort_chFR);
			//Get relevant data cache changes
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<SequenceNumber_t> not_relevant_changes;
			for(std::vector<ChangeForReader_t*>::iterator cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant && (*cit)->isValid())
				{
					relevant_changes.push_back((*cit)->getChange());
				}
				else
				{
					//	cout << "ADDED "<<(*cit)->seqNum.to64long()<< " TO NOT RELEVANT CHANGES"<<endl;
					not_relevant_changes.push_back((*cit)->seqNum);
				}
			}
			if(!relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&relevant_changes,
						mp_RP->m_att.endpoint.unicastLocatorList,
						mp_RP->m_att.endpoint.multicastLocatorList,
						mp_RP->m_att.expectsInlineQos,
						mp_RP->m_att.guid.entityId);
			if(!not_relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&not_relevant_changes,
						mp_RP->m_att.guid.entityId,
						&mp_RP->m_att.endpoint.unicastLocatorList,
						&mp_RP->m_att.endpoint.multicastLocatorList);
			if(relevant_changes.empty() && not_relevant_changes.empty())
			{
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				SequenceNumber_t first = mp_RP->mp_SFW->get_seq_num_min();
				SequenceNumber_t last = mp_RP->mp_SFW->get_seq_num_min();
				if(first.to64long() > 0 && last.to64long()>=first.to64long())
				{
					mp_RP->mp_SFW->incrementHBCount();
					RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,mp_RP->mp_SFW->getGuid().guidPrefix,
							mp_RP->m_att.guid.entityId,mp_RP->mp_SFW->getGuid().entityId,
							first,last,mp_RP->mp_SFW->getHeartbeatCount(),true,false);
					std::vector<Locator_t>::iterator lit;
					for(lit = mp_RP->m_att.endpoint.unicastLocatorList.begin();lit!=mp_RP->m_att.endpoint.unicastLocatorList.end();++lit)
						mp_RP->mp_SFW->getRTPSParticipant()->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
				}
				//					for(lit = (*rit)->m_param.multicastLocatorList.begin();lit!=mp_RP->m_param.multicastLocatorList.end();++lit)
				//						mp_RP->mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
			}
		}

	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"Nack response aborted");
		this->stopSemaphorePost();
	}
	else
	{
		logInfo(RTPS_WRITER,"Nack response boost message: " <<msg);
	}
}

}
} /* namespace  */
} /* namespace eprosima */
