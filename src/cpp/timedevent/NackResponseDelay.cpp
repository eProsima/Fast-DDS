/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackResponseDelay.cpp
 *
*/

#include "eprosimartps/timedevent/NackResponseDelay.h"

#include "eprosimartps/writer/StatefulWriter.h"

#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/writer/RTPSMessageGroup.h"
#include "eprosimartps/RTPSMessageCreator.h"

namespace eprosima {
namespace rtps {

NackResponseDelay::~NackResponseDelay() {
	timer->cancel();
	delete(timer);
}

NackResponseDelay::NackResponseDelay(ReaderProxy* p_RP,boost::posix_time::milliseconds interval):
		TimedEvent(&p_RP->mp_SFW->mp_event_thr->io_service,interval),
		mp_RP(p_RP)
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,mp_RP->mp_SFW->getGuid().guidPrefix);
}

bool sort_chFR (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->change->sequenceNumber.to64long() < c2->change->sequenceNumber.to64long());
}

void NackResponseDelay::event(const boost::system::error_code& ec)
{
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("NackResponse:event:"<<endl;);
		std::vector<ChangeForReader_t*> ch_vec;
		if(mp_RP->requested_changes(&ch_vec))
		{
			std::sort(ch_vec.begin(),ch_vec.end(),sort_chFR);
			//Get relevant data cache changes
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<CacheChange_t*> not_relevant_changes;
			for(std::vector<ChangeForReader_t*>::iterator cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant)
				{
					relevant_changes.push_back((*cit)->change);
				}
				else
				{
					not_relevant_changes.push_back((*cit)->change);
				}
			}
			if(!relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&relevant_changes,
						mp_RP->m_param.unicastLocatorList,
						mp_RP->m_param.multicastLocatorList,
						mp_RP->m_param.expectsInlineQos,
						mp_RP->m_param.remoteReaderGuid.entityId);
			if(!not_relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&not_relevant_changes,
						mp_RP->m_param.remoteReaderGuid.entityId,
						&mp_RP->m_param.unicastLocatorList,
						&mp_RP->m_param.multicastLocatorList);
		}
		m_isWaiting = false;
	}
	else if(ec==boost::asio::error::operation_aborted)
			{
				pInfo("Nack response aborted");
			}
			else
			{
				pInfo("Nack response boost message: " <<ec.message()<<endl);
			}
}


} /* namespace dds */
} /* namespace eprosima */
