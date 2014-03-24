/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HeartbeatResponseDelay.cpp
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/HeartbeatResponseDelay.h"

#include "eprosimartps/reader/StatefulReader.h"

namespace eprosima {
namespace rtps {

HeartbeatResponseDelay::HeartbeatResponseDelay() {
	// TODO Auto-generated constructor stub

}

HeartbeatResponseDelay::~HeartbeatResponseDelay() {
	// TODO Auto-generated destructor stub
}

HeartbeatResponseDelay::HeartbeatResponseDelay(StatefulReader* SR_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SR_ptr->participant->eventThread.io_service,interval),
		SR(SR_ptr)
{

}

void HeartbeatResponseDelay::event(const boost::system::error_code& ec,WriterProxy* wp)
{
	if(!ec)
	{

		std::vector<ChangeFromWriter_t*> ch_vec;
		{
		boost::lock_guard<WriterProxy> guard(*wp);
		wp->missing_changes(&ch_vec);
		}
		if(!ch_vec.empty())
		{
			SequenceNumberSet_t sns;
			wp->available_changes_max(&sns.base);
			sns.base++;
			std::vector<ChangeFromWriter_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();cit++)
			{
				sns.set.push_back((*cit)->change->sequenceNumber);
			}
			wp->acknackCount++;
			CDRMessage_t msg;
			RTPSMessageCreator::createMessageAcknack(&msg,SR->participant->guid.guidPrefix,
					SR->guid.entityId,wp->param.remoteWriterGuid.entityId,sns,wp->acknackCount,false);
			std::vector<Locator_t>::iterator lit;
			for(lit = wp->param.unicastLocatorList.begin();lit!=wp->param.unicastLocatorList.end();lit++)
				SR->participant->threadSend.sendSync(&msg,*lit);
			for(lit = wp->param.multicastLocatorList.begin();lit!=wp->param.multicastLocatorList.end();lit++)
				SR->participant->threadSend.sendSync(&msg,*lit);

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




} /* namespace rtps */
} /* namespace eprosima */
