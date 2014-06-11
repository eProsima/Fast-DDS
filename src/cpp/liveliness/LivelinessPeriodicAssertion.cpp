/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LivelinessPeriodicAssertion.cpp
 */

#include "eprosimartps/liveliness/LivelinessPeriodicAssertion.h"
#include "eprosimartps/liveliness/WriterLiveliness.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/writer/StatefulWriter.h"

namespace eprosima {
namespace rtps {

LivelinessPeriodicAssertion::LivelinessPeriodicAssertion(WriterLiveliness* wLiveliness,LivelinessQosPolicyKind kind):
		TimedEvent(&wLiveliness->mp_participant->getEventResource()->io_service, boost::posix_time::milliseconds(0)),
		m_livelinessKind(kind),
		mp_writerLiveliness(wLiveliness)
{


}

LivelinessPeriodicAssertion::~LivelinessPeriodicAssertion()
{

}

void LivelinessPeriodicAssertion::event(const boost::system::error_code& ec)
{
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		if(m_livelinessKind == AUTOMATIC_LIVELINESS_QOS)
			AutomaticLivelinessAssertion();
		else if(m_livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
			ManualByParticipantLivelinessAssertion();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pWarning("Liveliness Periodic Assertion aborted"<<endl);
	}
	else
	{
		pInfo("Liveliness Periodic Assertion boost message: " <<ec.message()<<endl);
	}
}

bool LivelinessPeriodicAssertion::AutomaticLivelinessAssertion()
{
	if(this->mp_writerLiveliness->m_AutomaticLivelinessWriters.size()>0)
	{
		CacheChange_t* change=NULL;
		if(this->mp_writerLiveliness->mp_builtinParticipantMessageWriter->new_change(ALIVE,NULL,&change))
		{
			InstanceHandle_t iH;
			GuidPrefix_t guidP(this->mp_writerLiveliness->mp_participant->getGuid().guidPrefix);
			for(uint8_t i =0;i<12;++i)
			{
				iH.value[i] = guidP.value[i];
			}
			iH.value[15] = m_livelinessKind;
		}
	}
	return true;
}

bool LivelinessPeriodicAssertion::ManualByParticipantLivelinessAssertion()
{
return false;
}


} /* namespace rtps */
} /* namespace eprosima */


