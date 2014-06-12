/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterLiveliness.cpp
 *
 */


#include <limits>

#include "eprosimartps/liveliness/WriterLiveliness.h"

#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/liveliness/LivelinessPeriodicAssertion.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

WriterLiveliness::WriterLiveliness(ParticipantImpl* p):
		mp_builtinParticipantMessageWriter(NULL),
		mp_builtinParticipantMessageReader(NULL),
		m_minAutomaticLivelinessLeaseDuration_MilliSec(std::numeric_limits<double>::max()),
		m_minManualByParticipantLivelinessLeaseDuration_MilliSec(std::numeric_limits<double>::max()),
		mp_participant(p),
		m_listener(this),
		mp_AutomaticLivelinessAssertion(NULL),
		mp_ManualByParticipantLivelinessAssertion(NULL)
{
	// TODO Auto-generated constructor stub

}

WriterLiveliness::~WriterLiveliness()
{
	// TODO Auto-generated destructor stub
}

bool WriterLiveliness::createEndpoints()
{
	//CREATE WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	Wparam.historyMaxSize = 2;
	Wparam.payloadMaxSize = 20;
	Wparam.unicastLocatorList = mp_participant->m_defaultUnicastLocatorList;
	Wparam.multicastLocatorList = mp_participant->m_defaultMulticastLocatorList;
	Wparam.topic.topicName = "DCPSParticipantMessage";
	Wparam.topic.topicDataType = "ParticipantMessageData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.userDefinedId = -1;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSWriter* wout;
	if(mp_participant->createWriter(&wout,Wparam,Wparam.payloadMaxSize,true,STATEFUL,NULL,NULL,c_EntityId_WriterLiveliness))
	{
		mp_builtinParticipantMessageWriter = dynamic_cast<StatefulWriter*>(wout);
		pInfo(MAGENTA<<"Writer Liveliness created"<<DEF<<endl);
	}
	else
	{
		pError("Liveliness Writer Creation failed "<<endl;)
				return false;
	}
	SubscriberAttributes Rparam;
	Rparam.expectsInlineQos = true;
	Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	Rparam.userDefinedId = -1;
	Rparam.historyMaxSize = 10;
	Rparam.topic.topicName = "DCPSParticipantMessage";
	Rparam.topic.topicDataType = "ParticipantMessageData";
	Rparam.topic.topicKind = WITH_KEY;
	RTPSReader* rout;
	if(mp_participant->createReader(&rout,Rparam,Rparam.payloadMaxSize,true,STATEFUL,NULL,NULL,c_EntityId_ReaderLiveliness))
	{
		mp_builtinParticipantMessageReader = dynamic_cast<StatefulReader*>(rout);
		pInfo(MAGENTA<<"Reader Liveliness created"<<DEF<<endl);
	}
	else
	{
		pError("Liveliness Reader Creation failed "<<endl;)
		return false;
	}
	return true;
}

bool WriterLiveliness::addWriter(RTPSWriter* W)
{
	double wLeaseDurationMilliSec(Time_t2MilliSec(W->getQos().m_liveliness.lease_duration));
	if(W->getQos().m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS )
	{
		if(mp_AutomaticLivelinessAssertion == NULL)
		{
			mp_AutomaticLivelinessAssertion = new LivelinessPeriodicAssertion(this,AUTOMATIC_LIVELINESS_QOS);
			mp_AutomaticLivelinessAssertion->update_interval_millisec(wLeaseDurationMilliSec);
			mp_AutomaticLivelinessAssertion->restart_timer();
		}
		else if(m_minAutomaticLivelinessLeaseDuration_MilliSec > wLeaseDurationMilliSec)
		{
			m_minAutomaticLivelinessLeaseDuration_MilliSec = wLeaseDurationMilliSec;
			mp_AutomaticLivelinessAssertion->update_interval_millisec(wLeaseDurationMilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_AutomaticLivelinessAssertion->m_isWaiting && mp_AutomaticLivelinessAssertion->getRemainingTimeMilliSec() > m_minAutomaticLivelinessLeaseDuration_MilliSec)
			{
				mp_AutomaticLivelinessAssertion->stop_timer();
			}
			mp_AutomaticLivelinessAssertion->restart_timer();
		}
		m_AutomaticLivelinessWriters.push_back(W);
	}
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		if(mp_ManualByParticipantLivelinessAssertion == NULL)
		{
			mp_ManualByParticipantLivelinessAssertion = new LivelinessPeriodicAssertion(this,MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
			mp_ManualByParticipantLivelinessAssertion->update_interval_millisec(wLeaseDurationMilliSec);
			mp_ManualByParticipantLivelinessAssertion->restart_timer();
		}
		else if(m_minManualByParticipantLivelinessLeaseDuration_MilliSec > wLeaseDurationMilliSec)
		{
			m_minManualByParticipantLivelinessLeaseDuration_MilliSec = wLeaseDurationMilliSec;
			mp_ManualByParticipantLivelinessAssertion->update_interval_millisec(m_minManualByParticipantLivelinessLeaseDuration_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_ManualByParticipantLivelinessAssertion->m_isWaiting && mp_ManualByParticipantLivelinessAssertion->getRemainingTimeMilliSec() > m_minManualByParticipantLivelinessLeaseDuration_MilliSec)
			{
				mp_ManualByParticipantLivelinessAssertion->stop_timer();
			}
			mp_ManualByParticipantLivelinessAssertion->restart_timer();
		}
		m_ManualByParticipantLivelinessWriters.push_back(W);
	}
	return true;
}

typedef std::vector<RTPSWriter*>::iterator t_WIT;

bool WriterLiveliness::removeWriter(RTPSWriter* W)
{
	t_WIT wToEraseIt;
	bool found = false;
	if(W->getQos().m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
	{
		for(t_WIT it= m_AutomaticLivelinessWriters.begin();it!=m_AutomaticLivelinessWriters.end();++it)
		{
			double mintimeWIT(Time_t2MilliSec((*it)->getQos().m_liveliness.lease_duration));
			if(W->getGuid().entityId == (*it)->getGuid().entityId)
			{
				found = true;
				if(m_minAutomaticLivelinessLeaseDuration_MilliSec < mintimeWIT)
				{
					m_AutomaticLivelinessWriters.erase(it);
					return true;
				}
				else
				{
					wToEraseIt = it;
				}
			}
			if(m_minAutomaticLivelinessLeaseDuration_MilliSec > mintimeWIT)
			{
				m_minAutomaticLivelinessLeaseDuration_MilliSec = mintimeWIT;
			}
		}
		if(found)
		{
			m_AutomaticLivelinessWriters.erase(wToEraseIt);
			mp_AutomaticLivelinessAssertion->update_interval_millisec(m_minAutomaticLivelinessLeaseDuration_MilliSec);
		}
	}
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		for(t_WIT it= m_ManualByParticipantLivelinessWriters.begin();it!=m_ManualByParticipantLivelinessWriters.end();++it)
		{
			double mintimeWIT(Time_t2MilliSec((*it)->getQos().m_liveliness.lease_duration));
			if(W->getGuid().entityId == (*it)->getGuid().entityId)
			{
				found = true;
				if(m_minManualByParticipantLivelinessLeaseDuration_MilliSec < mintimeWIT)
				{
					m_ManualByParticipantLivelinessWriters.erase(it);
					return true;
				}
				else
				{
					wToEraseIt = it;
				}
			}
			if(m_minManualByParticipantLivelinessLeaseDuration_MilliSec > mintimeWIT)
			{
				m_minManualByParticipantLivelinessLeaseDuration_MilliSec = mintimeWIT;
			}
		}
		if(found)
		{
			m_ManualByParticipantLivelinessWriters.erase(wToEraseIt);
			mp_ManualByParticipantLivelinessAssertion->update_interval_millisec(m_minManualByParticipantLivelinessLeaseDuration_MilliSec);
		}
	}
	else // OTHER VALUE OF LIVELINESS (BY TOPIC)
		return true;
	if(found)
		return true;
	else
		return false;
}

bool WriterLiveliness::updateWriter(RTPSWriter* W)
{
	pError("NOT IMPLEMENTED, WriterLiveliness CANNOT BE UPDATE YET"<<endl);
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
