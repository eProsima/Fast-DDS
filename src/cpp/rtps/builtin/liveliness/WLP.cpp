/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLP.cpp
 *
 */
#include <limits>

#include "fastrtps/builtin/liveliness/WLP.h"
#include "fastrtps/common/types/Guid.h"
#include "fastrtps/RTPSParticipant.h"
#include "fastrtps/writer/StatefulWriter.h"
#include "fastrtps/reader/StatefulReader.h"
#include "fastrtps/reader/WriterProxy.h"

#include "fastrtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h"
#include "fastrtps/builtin/BuiltinProtocols.h"
#include "fastrtps/RTPSParticipantProxyData.h"

#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/TimeConversion.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "WLP";

WLP::WLP(BuiltinProtocols* p):
												m_minAutomatic_MilliSec(std::numeric_limits<int64_t>::max()),
												m_minManRTPSParticipant_MilliSec(std::numeric_limits<int64_t>::max()),
												mp_participantImpl(nullptr),
												mp_builtinProtocols(p),
												mp_builtinRTPSParticipantMessageWriter(nullptr),
												mp_builtinRTPSParticipantMessageReader(nullptr),
#pragma warning(disable: 4355)
												m_listener(this),
												mp_livelinessAutomatic(nullptr),
												mp_livelinessManRTPSParticipant(nullptr)
{


}

WLP::~WLP()
{
	// TODO Auto-generated destructor stub
}

bool WLP::initWL(RTPSParticipantImpl* p)
{
	const char* const METHOD_NAME = "initWL";
	logInfo(RTPS_LIVELINESS,"Beginning Liveliness Protocol",EPRO_MAGENTA);
	mp_participantImpl = p;
	return createEndpoints();
}

bool WLP::createEndpoints()
{
	const char* const METHOD_NAME = "createEndpoints";
	//CREATE WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	//	Wparam.historyMaxSize = 2;
	Wparam.payloadMaxSize = 50;
	Wparam.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
	Wparam.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
	Wparam.topic.topicName = "DCPSRTPSParticipantMessage";
	Wparam.topic.topicDataType = "RTPSParticipantMessageData";
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
	if(mp_RTPSParticipant->createWriter(&wout,Wparam,Wparam.payloadMaxSize,true,STATEFUL,NULL,NULL,c_EntityId_WriterLiveliness))
	{
		mp_builtinRTPSParticipantMessageWriter = dynamic_cast<StatefulWriter*>(wout);
		logInfo(RTPS_LIVELINESS,"Builtin Liveliness Writer created",EPRO_MAGENTA);
	}
	else
	{
		logError(RTPS_LIVELINESS,"Liveliness Writer Creation failed ",EPRO_MAGENTA);
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
	Rparam.topic.topicName = "DCPSRTPSParticipantMessage";
	Rparam.topic.topicDataType = "RTPSParticipantMessageData";
	Rparam.topic.topicKind = WITH_KEY;
	RTPSReader* rout;
	if(mp_RTPSParticipant->createReader(&rout,Rparam,Rparam.payloadMaxSize,true,STATEFUL,
			(TopicDataType*)&this->m_wlpTopicDataType,(SubscriberListener*)&m_listener,c_EntityId_ReaderLiveliness))
	{
		mp_builtinRTPSParticipantMessageReader = dynamic_cast<StatefulReader*>(rout);
		logInfo(RTPS_LIVELINESS,"Builtin Liveliness Reader created",EPRO_MAGENTA);
	}
	else
	{
		logError(RTPS_LIVELINESS,"Liveliness Reader Creation failed ",EPRO_MAGENTA);
		return false;
	}

	return true;
}

bool WLP::assignRemoteEndpoints(RTPSParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	boost::lock_guard<WLP> guard(*this);
	logInfo(RTPS_LIVELINESS,"For remote RTPSParticipant "<<pdata->m_guid,EPRO_MAGENTA);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t partdet = endp;
	uint32_t auxendp = endp;
	partdet &= DISC_BUILTIN_ENDPOINT_RTPSParticipant_DETECTOR;
	auxendp &= BUILTIN_ENDPOINT_RTPSParticipant_MESSAGE_DATA_WRITER;
	//auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE
	if((auxendp!=0 || partdet!=0) && this->mp_builtinRTPSParticipantMessageReader!=NULL)
	{
		logInfo(RTPS_LIVELINESS,"Adding remote writer to my local Builtin Reader",EPRO_MAGENTA);
		WriterProxyData* wp = new WriterProxyData();
		wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		wp->m_guid.entityId = c_EntityId_WriterLiveliness;
		wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		wp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		pdata->m_builtinWriters.push_back(wp);
		mp_builtinRTPSParticipantMessageReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &=BUILTIN_ENDPOINT_RTPSParticipant_MESSAGE_DATA_READER;
	//auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE
	if((auxendp!=0 || partdet!=0) && this->mp_builtinRTPSParticipantMessageWriter!=NULL)
	{
		logInfo(RTPS_LIVELINESS,"Adding remote reader to my local Builtin Writer",EPRO_MAGENTA);
		ReaderProxyData* rp = new ReaderProxyData();
		rp->m_expectsInlineQos = false;
		rp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		rp->m_guid.entityId = c_EntityId_ReaderLiveliness;
		rp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		rp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		rp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		pdata->m_builtinReaders.push_back(rp);
		mp_builtinRTPSParticipantMessageWriter->matched_reader_add(rp);
	}
	return true;
}

void WLP::removeRemoteEndpoints(RTPSParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "removeRemoteEndpoints";
	logInfo(RTPS_LIVELINESS,"for RTPSParticipant: "<<pdata->m_guid,EPRO_MAGENTA);
	for(std::vector<ReaderProxyData*>::iterator it = pdata->m_builtinReaders.begin();
			it!=pdata->m_builtinReaders.end();++it)
	{
		if((*it)->m_guid.entityId == c_EntityId_ReaderLiveliness && this->mp_builtinRTPSParticipantMessageWriter !=NULL)
		{
			mp_builtinRTPSParticipantMessageWriter->matched_reader_remove(*it);
			break;
		}
	}
	for(std::vector<WriterProxyData*>::iterator it = pdata->m_builtinWriters.begin();
			it!=pdata->m_builtinWriters.end();++it)
	{
		if((*it)->m_guid.entityId == c_EntityId_WriterLiveliness && this->mp_builtinRTPSParticipantMessageReader !=NULL)
		{
			mp_builtinRTPSParticipantMessageReader->matched_writer_remove(*it);
			break;
		}
	}
}



bool WLP::addLocalWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "addLocalWriter";
	boost::lock_guard<WLP> guard(*this);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId << " in topic "<<W->getTopic().topicName
			<<" to Liveliness Protocol",EPRO_MAGENTA)
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
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_RTPSParticipant_LIVELINESS_QOS)
	{
		if(mp_livelinessManRTPSParticipant == NULL)
		{
			mp_livelinessManRTPSParticipant = new WLivelinessPeriodicAssertion(this,MANUAL_BY_RTPSParticipant_LIVELINESS_QOS);
			mp_livelinessManRTPSParticipant->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessManRTPSParticipant->restart_timer();
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minManRTPSParticipant_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessManRTPSParticipant->m_isWaiting && mp_livelinessManRTPSParticipant->getRemainingTimeMilliSec() > m_minManRTPSParticipant_MilliSec)
			{
				mp_livelinessManRTPSParticipant->stop_timer();
			}
			mp_livelinessManRTPSParticipant->restart_timer();
		}
		m_livManRTPSParticipantWriters.push_back(W);
	}
	return true;
}

typedef std::vector<RTPSWriter*>::iterator t_WIT;

bool WLP::removeLocalWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "removeLocalWriter";
	boost::lock_guard<WLP> guard(*this);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId << " in topic "<<W->getTopic().topicName
			<<" from Liveliness Protocol",EPRO_MAGENTA);
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
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_RTPSParticipant_LIVELINESS_QOS)
	{
		m_minManRTPSParticipant_MilliSec = std::numeric_limits<int64_t>::max();
		for(t_WIT it= m_livManRTPSParticipantWriters.begin();it!=m_livManRTPSParticipantWriters.end();++it)
		{
			int64_t mintimeWIT(TimeConv::Time_t2MilliSecondsInt64((*it)->getQos().m_liveliness.announcement_period));
			if(W->getGuid().entityId == (*it)->getGuid().entityId)
			{
				found = true;
				wToEraseIt = it;
				continue;
			}
			if(m_minManRTPSParticipant_MilliSec > mintimeWIT)
			{
				m_minManRTPSParticipant_MilliSec = mintimeWIT;
			}
		}
		if(found)
		{
			m_livManRTPSParticipantWriters.erase(wToEraseIt);
			if(mp_livelinessManRTPSParticipant!=NULL)
			{
				if(m_livManRTPSParticipantWriters.size()>0)
					mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
				else
				{

					mp_livelinessManRTPSParticipant->stop_timer();
					delete(mp_livelinessManRTPSParticipant);
					mp_livelinessManRTPSParticipant = NULL;
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

bool WLP::updateLocalWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "updateLocalWriter";
	boost::lock_guard<WLP> guard(*this);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId,EPRO_MAGENTA);
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
	}
	else if(W->getQos().m_liveliness.kind == MANUAL_BY_RTPSParticipant_LIVELINESS_QOS)
	{
		if(mp_livelinessManRTPSParticipant == NULL)
		{
			mp_livelinessManRTPSParticipant = new WLivelinessPeriodicAssertion(this,MANUAL_BY_RTPSParticipant_LIVELINESS_QOS);
			mp_livelinessManRTPSParticipant->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessManRTPSParticipant->restart_timer();
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minManRTPSParticipant_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessManRTPSParticipant->m_isWaiting && mp_livelinessManRTPSParticipant->getRemainingTimeMilliSec() > m_minManRTPSParticipant_MilliSec)
			{
				mp_livelinessManRTPSParticipant->stop_timer();
			}
			mp_livelinessManRTPSParticipant->restart_timer();
		}
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */


