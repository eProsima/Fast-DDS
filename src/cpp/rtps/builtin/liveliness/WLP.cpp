/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLP.cpp
 *
 */
#include <limits>

#include <fastrtps/rtps/builtin/liveliness/WLP.h>
#include <fastrtps/rtps/builtin/liveliness/WLPListener.h>
#include <fastrtps/rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.h>
#include "../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/utils/TimeConversion.h>


#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "WLP";

WLP::WLP(BuiltinProtocols* p):	m_minAutomatic_MilliSec(std::numeric_limits<double>::max()),
		m_minManRTPSParticipant_MilliSec(std::numeric_limits<double>::max()),
		mp_participant(nullptr),
		mp_builtinProtocols(p),
		mp_builtinWriter(nullptr),
		mp_builtinReader(nullptr),
		mp_builtinWriterHistory(nullptr),
		mp_builtinReaderHistory(nullptr),
		mp_listener(nullptr),
		mp_livelinessAutomatic(nullptr),
		mp_livelinessManRTPSParticipant(nullptr),
		mp_mutex(new boost::recursive_mutex)
{


}

WLP::~WLP()
{
	// TODO Auto-generated destructor stub
	delete(this->mp_builtinReader);
	delete(this->mp_builtinWriter);
	delete(this->mp_builtinReaderHistory);
	delete(this->mp_builtinWriterHistory);
	delete(this->mp_listener);
	if(this->mp_livelinessAutomatic!=nullptr)
		delete(mp_livelinessAutomatic);
	if(this->mp_livelinessManRTPSParticipant!=nullptr)
		delete(this->mp_livelinessManRTPSParticipant);
	delete(mp_mutex);
}

bool WLP::initWL(RTPSParticipantImpl* p)
{
	const char* const METHOD_NAME = "initWL";
	logInfo(RTPS_LIVELINESS,"Beginning Liveliness Protocol",C_MAGENTA);
	mp_participant = p;
	return createEndpoints();
}

bool WLP::createEndpoints()
{
	const char* const METHOD_NAME = "createEndpoints";
	//CREATE WRITER
	HistoryAttributes hatt;
	hatt.initialReservedCaches = 20;
	hatt.maximumReservedCaches = 1000;
	hatt.payloadMaxSize = BUILTIN_PARTICIPANT_DATA_MAX_SIZE;
	mp_builtinWriterHistory = new WriterHistory(hatt);
	WriterAttributes watt;
	watt.endpoint.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
	watt.endpoint.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
	//	Wparam.topic.topicName = "DCPSRTPSParticipantMessage";
	//	Wparam.topic.topicDataType = "RTPSParticipantMessageData";
	watt.endpoint.topicKind = WITH_KEY;
	watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
	watt.endpoint.reliabilityKind = RELIABLE;
	RTPSWriter* wout;
	if(mp_participant->createWriter(&wout,watt,mp_builtinWriterHistory,nullptr,c_EntityId_WriterLiveliness,true))
	{
		mp_builtinWriter = dynamic_cast<StatefulWriter*>(wout);
		logInfo(RTPS_LIVELINESS,"Builtin Liveliness Writer created",C_MAGENTA);
	}
	else
	{
		logError(RTPS_LIVELINESS,"Liveliness Writer Creation failed ",C_MAGENTA);
		delete(mp_builtinWriterHistory);
		mp_builtinWriterHistory = nullptr;
		return false;
	}
	hatt.initialReservedCaches = 100;
	hatt.maximumReservedCaches = 2000;
	hatt.payloadMaxSize = BUILTIN_PARTICIPANT_DATA_MAX_SIZE;
	mp_builtinReaderHistory = new ReaderHistory(hatt);
	ReaderAttributes ratt;
	ratt.endpoint.topicKind = WITH_KEY;
	ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
	ratt.endpoint.reliabilityKind = RELIABLE;
	ratt.expectsInlineQos = true;
	ratt.endpoint.unicastLocatorList =  mp_builtinProtocols->m_metatrafficUnicastLocatorList;
	ratt.endpoint.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
	//Rparam.topic.topicName = "DCPSRTPSParticipantMessage";
	//Rparam.topic.topicDataType = "RTPSParticipantMessageData";
	ratt.endpoint.topicKind = WITH_KEY;
	//LISTENER CREATION
	mp_listener = new WLPListener(this);
	RTPSReader* rout;
	if(mp_participant->createReader(&rout,ratt,mp_builtinReaderHistory,(ReaderListener*)mp_listener,c_EntityId_ReaderLiveliness,true))
	{
		mp_builtinReader = dynamic_cast<StatefulReader*>(rout);
		logInfo(RTPS_LIVELINESS,"Builtin Liveliness Reader created",C_MAGENTA);
	}
	else
	{
		logError(RTPS_LIVELINESS,"Liveliness Reader Creation failed.",C_MAGENTA);
		delete(mp_builtinReaderHistory);
		mp_builtinReaderHistory = nullptr;
		delete(mp_listener);
		mp_listener = nullptr;
		return false;
	}

	return true;
}

bool WLP::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	boost::lock_guard<boost::recursive_mutex> guard2(*pdata->mp_mutex);
	logInfo(RTPS_LIVELINESS,"For remote RTPSParticipant "<<pdata->m_guid,C_MAGENTA);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t partdet = endp;
	uint32_t auxendp = endp;
	//TODO Hacer el check con RTI y solo añadirlo cuando el vendor ID sea RTI.
	partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
	auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
	//auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE

	if((auxendp!=0 || partdet!=0) && this->mp_builtinReader!=nullptr)
	{
		logInfo(RTPS_LIVELINESS,"Adding remote writer to my local Builtin Reader",C_MAGENTA);
		RemoteWriterAttributes watt;
		watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		watt.guid.entityId = c_EntityId_WriterLiveliness;
		watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		watt.endpoint.topicKind = WITH_KEY;
		watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		watt.endpoint.reliabilityKind = RELIABLE;
		pdata->m_builtinWriters.push_back(watt);
		mp_builtinReader->matched_writer_add(watt);
	}
	auxendp = endp;
	auxendp &=BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
	//auxendp = 1;
	//FIXME: WRITERLIVELINESS PUT THIS BACK TO THE ORIGINAL LINE
	if((auxendp!=0 || partdet!=0) && this->mp_builtinWriter!=nullptr)
	{
		logInfo(RTPS_LIVELINESS,"Adding remote reader to my local Builtin Writer",C_MAGENTA);
		RemoteReaderAttributes ratt;
		ratt.expectsInlineQos = false;
		ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
		ratt.guid.entityId = c_EntityId_ReaderLiveliness;
		ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		ratt.endpoint.topicKind = WITH_KEY;
		ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
		ratt.endpoint.reliabilityKind = RELIABLE;
		pdata->m_builtinReaders.push_back(ratt);
		mp_builtinWriter->matched_reader_add(ratt);
	}
	return true;
}

void WLP::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "removeRemoteEndpoints";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	boost::lock_guard<boost::recursive_mutex> guard2(*pdata->mp_mutex);
	logInfo(RTPS_LIVELINESS,"for RTPSParticipant: "<<pdata->m_guid,C_MAGENTA);
	for(auto it = pdata->m_builtinReaders.begin();
			it!=pdata->m_builtinReaders.end();++it)
	{
		if((*it).guid.entityId == c_EntityId_ReaderLiveliness && this->mp_builtinWriter !=nullptr)
		{
			mp_builtinWriter->matched_reader_remove(*it);
			break;
		}
	}
	for(auto it = pdata->m_builtinWriters.begin();
			it!=pdata->m_builtinWriters.end();++it)
	{
		if((*it).guid.entityId == c_EntityId_WriterLiveliness && this->mp_builtinReader !=nullptr)
		{
			mp_builtinReader->matched_writer_remove(*it);
			break;
		}
	}
}



bool WLP::addLocalWriter(RTPSWriter* W,WriterQos& wqos)
{
	const char* const METHOD_NAME = "addLocalWriter";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId	<<" to Liveliness Protocol",C_MAGENTA)
	double wAnnouncementPeriodMilliSec(TimeConv::Time_t2MilliSecondsDouble(wqos.m_liveliness.announcement_period));
	if(wqos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS )
	{
		if(mp_livelinessAutomatic == nullptr)
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
			if(mp_livelinessAutomatic->getRemainingTimeMilliSec() > m_minAutomatic_MilliSec)
                mp_livelinessAutomatic->restart_timer();
		}
		m_livAutomaticWriters.push_back(W);
	}
	else if(wqos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		if(mp_livelinessManRTPSParticipant == nullptr)
		{
			mp_livelinessManRTPSParticipant = new WLivelinessPeriodicAssertion(this,MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
			mp_livelinessManRTPSParticipant->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessManRTPSParticipant->restart_timer();
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minManRTPSParticipant_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessManRTPSParticipant->getRemainingTimeMilliSec() > m_minManRTPSParticipant_MilliSec)
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
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId
			<<" from Liveliness Protocol",C_MAGENTA);
	t_WIT wToEraseIt;
	WriterProxyData* wdata = nullptr;
	if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData(W->getGuid(),&wdata))
	{
		bool found = false;
		if(wdata->m_qos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
		{
			m_minAutomatic_MilliSec = std::numeric_limits<double>::max();
			for(t_WIT it= m_livAutomaticWriters.begin();it!=m_livAutomaticWriters.end();++it)
			{
				WriterProxyData* wdata2;
				if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData((*it)->getGuid(),&wdata2))
				{
					double mintimeWIT(TimeConv::Time_t2MilliSecondsDouble(wdata2->m_qos.m_liveliness.announcement_period));
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
			}
			if(found)
			{
				m_livAutomaticWriters.erase(wToEraseIt);
				if(mp_livelinessAutomatic!=nullptr)
				{
					if(m_livAutomaticWriters.size()>0)
						mp_livelinessAutomatic->update_interval_millisec(m_minAutomatic_MilliSec);
					else
					{
						delete(mp_livelinessAutomatic);
						mp_livelinessAutomatic = nullptr;

					}
				}
			}
		}
		else if(wdata->m_qos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
		{
			m_minManRTPSParticipant_MilliSec = std::numeric_limits<double>::max();
			for(t_WIT it= m_livManRTPSParticipantWriters.begin();it!=m_livManRTPSParticipantWriters.end();++it)
			{
				WriterProxyData* wdata2;
				if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData((*it)->getGuid(),&wdata2))
				{
					double mintimeWIT(TimeConv::Time_t2MilliSecondsDouble(wdata2->m_qos.m_liveliness.announcement_period));
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
			}
			if(found)
			{
				m_livManRTPSParticipantWriters.erase(wToEraseIt);
				if(mp_livelinessManRTPSParticipant!=nullptr)
				{
					if(m_livManRTPSParticipantWriters.size()>0)
						mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
					else
					{
						delete(mp_livelinessManRTPSParticipant);
						mp_livelinessManRTPSParticipant = nullptr;
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
	logWarning(RTPS_LIVELINESS,"Writer "<<W->getGuid().entityId << " not found.",C_MAGENTA);
	return false;
}

bool WLP::updateLocalWriter(RTPSWriter* W, WriterQos& wqos)
{
	const char* const METHOD_NAME = "updateLocalWriter";

    // Unused in release mode.
    (void)W;

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	logInfo(RTPS_LIVELINESS,W->getGuid().entityId,C_MAGENTA);
	double wAnnouncementPeriodMilliSec(TimeConv::Time_t2MilliSecondsDouble(wqos.m_liveliness.announcement_period));
	if(wqos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS )
	{
		if(mp_livelinessAutomatic == nullptr)
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
			if(mp_livelinessAutomatic->getRemainingTimeMilliSec() > m_minAutomatic_MilliSec)
                mp_livelinessAutomatic->restart_timer();
		}
	}
	else if(wqos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
	{
		if(mp_livelinessManRTPSParticipant == nullptr)
		{
			mp_livelinessManRTPSParticipant = new WLivelinessPeriodicAssertion(this,MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
			mp_livelinessManRTPSParticipant->update_interval_millisec(wAnnouncementPeriodMilliSec);
			mp_livelinessManRTPSParticipant->restart_timer();
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
		}
		else if(m_minManRTPSParticipant_MilliSec > wAnnouncementPeriodMilliSec)
		{
			m_minManRTPSParticipant_MilliSec = wAnnouncementPeriodMilliSec;
			mp_livelinessManRTPSParticipant->update_interval_millisec(m_minManRTPSParticipant_MilliSec);
			//CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
			if(mp_livelinessManRTPSParticipant->getRemainingTimeMilliSec() > m_minManRTPSParticipant_MilliSec)
                mp_livelinessManRTPSParticipant->restart_timer();
		}
	}
	return true;
}

}
} /* namespace rtps */
} /* namespace eprosima */


