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

NackResponseDelay::NackResponseDelay() {
	// TODO Auto-generated constructor stub

}

NackResponseDelay::~NackResponseDelay() {
	// TODO Auto-generated destructor stub
}

NackResponseDelay::NackResponseDelay(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SW_ptr->participant->eventThread.io_service,interval),
		SW(SW_ptr)
{

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
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();cit++)
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
				SW->sendChangesList(relevant_changes,&rp->param.unicastLocatorList,
						&rp->param.multicastLocatorList,
						rp->param.expectsInlineQos,
						rp->param.remoteReaderGuid.entityId);
			if(!not_relevant_changes.empty())
				SW->sendChangesListAsGap(&not_relevant_changes,
						rp->param.remoteReaderGuid.entityId,
						&rp->param.unicastLocatorList,
						&rp->param.multicastLocatorList);
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
