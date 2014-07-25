/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLP.cpp
 *
 */
#include <limits>

#include "eprosimartps/builtin/liveliness/WLP.h"
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/reader/WriterProxy.h"

#include "eprosimartps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h"
#include "eprosimartps/builtin/BuiltinProtocols.h"
#include "eprosimartps/ParticipantProxyData.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/TimeConversion.h"

namespace eprosima {
namespace rtps {

WLP::WLP(ParticipantImpl* p):
						m_minAutomatic_MilliSec(std::numeric_limits<int64_t>::max()),
						m_minManParticipant_MilliSec(std::numeric_limits<int64_t>::max()),
						mp_participant(p),
						mp_builtinProtocols(NULL),
						mp_builtinParticipantMessageWriter(NULL),
						mp_builtinParticipantMessageReader(NULL),
						m_listener(this),
						mp_livelinessAutomatic(NULL),
						mp_livelinessManParticipant(NULL)
{

	pInfo(RTPS_B_MAGENTA<<"Beginning Liveliness Protocol initialization"<<RTPS_DEF<<endl;);
}

WLP::~WLP()
{
	// TODO Auto-generated destructor stub
}

bool WLP::initWL(BuiltinProtocols* prot)
{
	mp_builtinProtocols = prot;
	return createEndpoints();
}

bool WLP::createEndpoints()
{
	//CREATE WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	//	Wparam.historyMaxSize = 2;
	Wparam.payloadMaxSize = 50;
	Wparam.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
	Wparam.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
	Wparam.topic.topicName = "DCPSParticipantMessage";
	Wparam.topic.topicDataType = "ParticipantMessageData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.userDefinedId = -1;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_instances = 50;
	Wparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.resourceLimitsQos.allocated_samples = 50;
	RTPSWriter* wout;
	if(mp_participant->createWriter(&wout,Wparam,Wparam.payloadMaxSize,true,STATEFUL,NULL,NULL,c_EntityId_WriterLiveliness))
	{
		mp_builtinParticipantMessageWriter = dynamic_cast<StatefulWriter*>(wout);
		pInfo(RTPS_MAGENTA<<"Builtin Liveliness Writer created"<<RTPS_DEF<<endl);
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
	Rparam.unicastLocatorList =  mp_builtinProtocols->m_metatrafficUnicastLocatorList;
	Rparam.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
	Rparam.userDefinedId = -1;
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 1;
	Rparam.topic.resourceLimitsQos.max_instances = 2000;
	Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Rparam.topic.resourceLimitsQos.max_samples = 2000;
	Rparam.topic.resourceLimitsQos.allocated_samples = 200;
	Rparam.topic.topicName = "DCPSParticipantMessage";
	Rparam.topic.topicDataType = "ParticipantMessageData";
	Rparam.topic.topicKind = WITH_KEY;
	RTPSReader* rout;
	if(mp_participant->createReader(&rout,Rparam,Rparam.payloadMaxSize,true,STATEFUL,NULL,(SubscriberListener*)&m_listener,c_EntityId_ReaderLiveliness))
	{
		mp_builtinParticipantMessageReader = dynamic_cast<StatefulReader*>(rout);
		pInfo(RTPS_MAGENTA<<"Builtin Liveliness Reader created"<<RTPS_DEF<<endl);
	}
	else
	{
		pError("Liveliness Reader Creation failed "<<endl;)
						return false;
	}

	return true;
}

bool WLP::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	boost::lock_guard<WLP> guard(*this);
	pInfo(RTPS_MAGENTA<<"WriterLiveliness:assign remote Endpoints"<<RTPS_DEF<<endl;);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	//auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
	auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE
	if(auxendp!=0 && this->mp_builtinParticipantMessageReader!=NULL)
	{
		pDebugInfo(RTPS_MAGENTA<<"Adding remote writer to my local Builtin Reader"<<RTPS_DEF<<endl;);
		WriterProxyData* wp = new WriterProxyData();
		wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		wp->m_guid.entityId = c_EntityId_WriterLiveliness;
		wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		wp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		pdata->m_builtinWriters.push_back(wp);
		mp_builtinParticipantMessageReader->matched_writer_add(wp);
	}
	auxendp = endp;
	//auxendp &=BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
	auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE
	if(auxendp!=0 && this->mp_builtinParticipantMessageWriter!=NULL)
	{
		pDebugInfo(RTPS_MAGENTA<<"Adding remote reader to my local Builtin Writer"<<RTPS_DEF<<endl;);
		ReaderProxyData* rp = new ReaderProxyData();
		rp->m_expectsInlineQos = false;
		rp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		rp->m_guid.entityId = c_EntityId_SEDPPubReader;
		rp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		rp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		rp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		pdata->m_builtinReaders.push_back(rp);
		mp_builtinParticipantMessageWriter->matched_reader_add(rp);
	}
	return true;
}

bool WLP::addLocalWriter(RTPSWriter* W)
{
	boost::lock_guard<WLP> guard(*this);
	pDebugInfo(RTPS_MAGENTA<<"Adding local Writer to Liveliness Protocol"<<RTPS_DEF << endl;)
	int64_t wAnnouncementPeriodMilliSec(TimeConv::Time_t2MilliSecondsInt64(W->getQos().m_liveliness.announcement_period));
	if(W->getQos().m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS )
	{
		if(mp_livelinessAutomatic == NULL)
		{
			mp_livelinessAutomatic = new WLivelinessPeriodicAssertion(this,AUTOMATIC_LIVELINESS_QOS);
			mp_livelinessAutomatic->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessAutomatic->restart_timer();
			m_minAutomatic_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minAutomatic_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minAutomatic_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessAutomatic->update_interval_millisec(wAnnouncementPeriodMilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessAutomatic->m_isWaiting && mp_livelinessAutomatic->getRemainingTimeMilliSec() > m_minAutomatic_MilliSec)
			{
				mp_livelinessAutomatic->stop_timer();
			}
			mp_livelinessAutomatic->restart_timer();
		}
		m_livAutomaticWriters.push_back(W);
	}
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		if(mp_livelinessManParticipant == NULL)
		{
			mp_livelinessManParticipant = new WLivelinessPeriodicAssertion(this,MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
			mp_livelinessManParticipant->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessManParticipant->restart_timer();
			m_minManParticipant_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minManParticipant_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minManParticipant_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessManParticipant->update_interval_millisec(m_minManParticipant_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessManParticipant->m_isWaiting && mp_livelinessManParticipant->getRemainingTimeMilliSec() > m_minManParticipant_MilliSec)
			{
				mp_livelinessManParticipant->stop_timer();
			}
			mp_livelinessManParticipant->restart_timer();
		}
		m_livManParticipantWriters.push_back(W);
	}
	return true;
}

typedef std::vector<RTPSWriter*>::iterator t_WIT;

bool WLP::removeLocalWriter(RTPSWriter* W)
{
	boost::lock_guard<WLP> guard(*this);
	pInfo(RTPS_MAGENTA<<"Removing Local Writer from Liveliness Protocol"<<RTPS_DEF<<endl;);
	t_WIT wToEraseIt;
	bool found = false;
	if(W->getQos().m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
	{
		m_minAutomatic_MilliSec = std::numeric_limits<int64_t>::max();
		for(t_WIT it= m_livAutomaticWriters.begin();it!=m_livAutomaticWriters.end();++it)
		{
			int64_t mintimeWIT(TimeConv::Time_t2MilliSecondsInt64((*it)->getQos().m_liveliness.announcement_period));
			if(W->getGuid().entityId == (*it)->getGuid().entityId)
			{
				found = true;
				wToEraseIt = it;
				continue;
			}
			if(m_minAutomatic_MilliSec > mintimeWIT)
			{
				m_minAutomatic_MilliSec = mintimeWIT;
			}
		}
		if(found)
		{
			m_livAutomaticWriters.erase(wToEraseIt);
			if(mp_livelinessAutomatic!=NULL)
			{
				if(m_livAutomaticWriters.size()>0)
					mp_livelinessAutomatic->update_interval_millisec(m_minAutomatic_MilliSec);
				else
				{
					mp_livelinessAutomatic->stop_timer();
					delete(mp_livelinessAutomatic);
					mp_livelinessAutomatic = NULL;

				}
			}
		}
	}
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		m_minManParticipant_MilliSec = std::numeric_limits<int64_t>::max();
		for(t_WIT it= m_livManParticipantWriters.begin();it!=m_livManParticipantWriters.end();++it)
		{
			int64_t mintimeWIT(TimeConv::Time_t2MilliSecondsInt64((*it)->getQos().m_liveliness.announcement_period));
			if(W->getGuid().entityId == (*it)->getGuid().entityId)
			{
				found = true;
				wToEraseIt = it;
				continue;
			}
			if(m_minManParticipant_MilliSec > mintimeWIT)
			{
				m_minManParticipant_MilliSec = mintimeWIT;
			}
		}
		if(found)
		{
			m_livManParticipantWriters.erase(wToEraseIt);
			if(mp_livelinessManParticipant!=NULL)
			{
				if(m_livManParticipantWriters.size()>0)
					mp_livelinessManParticipant->update_interval_millisec(m_minManParticipant_MilliSec);
				else
				{

					mp_livelinessManParticipant->stop_timer();
					delete(mp_livelinessManParticipant);
					mp_livelinessManParticipant = NULL;
				}
			}
		}
	}
	else // OTHER VALUE OF LIVELINESS (BY TOPIC)
	return true;
	if(found)
		return true;
	else
		return false;
}



} /* namespace rtps */
} /* namespace eprosima */


