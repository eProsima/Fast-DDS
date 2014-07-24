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

#include "eprosimartps/writer/timedevent/NackResponseDelay.h"
#include "eprosimartps/writer/ReaderProxyData.h"
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
	stop_timer();
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
	return(c1->seqNum < c2->seqNum);
}

void NackResponseDelay::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		pDebugInfo("NackResponse:event:"<<endl;);
		std::vector<ChangeForReader_t*> ch_vec;
		if(mp_RP->requested_changes(&ch_vec))
		{
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
					not_relevant_changes.push_back((*cit)->seqNum);
				}
			}
			if(!relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&relevant_changes,
						mp_RP->m_data->m_unicastLocatorList,
						mp_RP->m_data->m_multicastLocatorList,
						mp_RP->m_data->m_expectsInlineQos,
						mp_RP->m_data->m_guid.entityId);
			if(!not_relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)mp_RP->mp_SFW,
						&not_relevant_changes,
						mp_RP->m_data->m_guid.entityId,
						&mp_RP->m_data->m_unicastLocatorList,
						&mp_RP->m_data->m_multicastLocatorList);
			if(relevant_changes.empty() && not_relevant_changes.empty())
			{
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				SequenceNumber_t first,last;
				mp_RP->mp_SFW->get_seq_num_min(&first,NULL);
				mp_RP->mp_SFW->get_seq_num_max(&last,NULL);
				mp_RP->mp_SFW->incrementHBCount();
				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,mp_RP->mp_SFW->getGuid().guidPrefix,
						mp_RP->m_data->m_guid.entityId,mp_RP->mp_SFW->getGuid().entityId,
						first,last,mp_RP->mp_SFW->getHeartbeatCount(),true,false);
				std::vector<Locator_t>::iterator lit;
				for(lit = mp_RP->m_data->m_unicastLocatorList.begin();lit!=mp_RP->m_data->m_unicastLocatorList.end();++lit)
					mp_RP->mp_SFW->mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));

				//					for(lit = (*rit)->m_param.multicastLocatorList.begin();lit!=mp_RP->m_param.multicastLocatorList.end();++lit)
				//						mp_RP->mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
			}
		}

	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("Nack response aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		pInfo("Nack response boost message: " <<ec.message()<<endl);
	}
}


} /* namespace dds */
} /* namespace eprosima */
