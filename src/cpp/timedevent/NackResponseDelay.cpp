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
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/NackResponseDelay.h"

#include "eprosimartps/writer/StatefulWriter.h"


namespace eprosima {
namespace rtps {

NackResponseDelay::~NackResponseDelay() {
	// TODO Auto-generated destructor stub
}

NackResponseDelay::NackResponseDelay(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SW_ptr->participant->m_event_thr.io_service,interval),
		SW(SW_ptr)
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,SW->participant->m_guid.guidPrefix);
}

bool sort_chFR (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->change->sequenceNumber.to64long() < c2->change->sequenceNumber.to64long());
}

void NackResponseDelay::event(const boost::system::error_code& ec,ReaderProxy* rp)
{
	if(!ec)
	{
		std::vector<ChangeForReader_t*> ch_vec;
		if(rp->requested_changes(&ch_vec))
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
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)SW,
						&relevant_changes,
						&rp->m_param.unicastLocatorList,
						&rp->m_param.multicastLocatorList,
						rp->m_param.expectsInlineQos,
						rp->m_param.remoteReaderGuid.entityId);
			if(!not_relevant_changes.empty())
				RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)SW,
						&not_relevant_changes,
						rp->m_param.remoteReaderGuid.entityId,
						&rp->m_param.unicastLocatorList,
						&rp->m_param.multicastLocatorList);
		}
	}
	if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("Nack Response delay aborted");
	}
	else
	{
		pError(ec.message()<<endl);
	}
}


} /* namespace dds */
} /* namespace eprosima */
