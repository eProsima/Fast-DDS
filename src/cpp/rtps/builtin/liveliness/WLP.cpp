// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>


#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {


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
    mp_livelinessManRTPSParticipant(nullptr)
#if HAVE_SECURITY
    ,mp_builtinWriterSecure(nullptr)
    ,mp_builtinReaderSecure(nullptr)
    ,mp_builtinWriterSecureHistory(nullptr)
    ,mp_builtinReaderSecureHistory(nullptr)
#endif
{
}

WLP::~WLP()
{
#if HAVE_SECURITY
    mp_participant->deleteUserEndpoint(mp_builtinReaderSecure);
    mp_participant->deleteUserEndpoint(mp_builtinWriterSecure);
    delete(this->mp_builtinReaderSecureHistory);
    delete(this->mp_builtinWriterSecureHistory);
#endif
    mp_participant->deleteUserEndpoint(mp_builtinReader);
    mp_participant->deleteUserEndpoint(mp_builtinWriter);
    delete(this->mp_builtinReaderHistory);
    delete(this->mp_builtinWriterHistory);
    delete(this->mp_listener);
    if(this->mp_livelinessAutomatic!=nullptr)
        delete(mp_livelinessAutomatic);
    if(this->mp_livelinessManRTPSParticipant!=nullptr)
        delete(this->mp_livelinessManRTPSParticipant);
}

bool WLP::initWL(RTPSParticipantImpl* p)
{
    logInfo(RTPS_LIVELINESS,"Beginning Liveliness Protocol");
    mp_participant = p;

    bool retVal = createEndpoints();
#if HAVE_SECURITY
    if(retVal) createSecureEndpoints();
#endif
    return retVal;
}

bool WLP::createEndpoints()
{
    //CREATE WRITER
    HistoryAttributes hatt;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 1000;
    hatt.payloadMaxSize = BUILTIN_PARTICIPANT_DATA_MAX_SIZE;
    mp_builtinWriterHistory = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
    watt.endpoint.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
    watt.endpoint.remoteLocatorList = mp_builtinProtocols->m_initialPeersList;
    //	Wparam.topic.topicName = "DCPSRTPSParticipantMessage";
    //	Wparam.topic.topicDataType = "RTPSParticipantMessageData";
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = RELIABLE;
    if(mp_participant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            mp_participant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;
    RTPSWriter* wout;
    if(mp_participant->createWriter(&wout,watt,mp_builtinWriterHistory,nullptr,c_EntityId_WriterLiveliness,true))
    {
        mp_builtinWriter = dynamic_cast<StatefulWriter*>(wout);
        logInfo(RTPS_LIVELINESS,"Builtin Liveliness Writer created");
    }
    else
    {
        logError(RTPS_LIVELINESS,"Liveliness Writer Creation failed ");
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
    ratt.endpoint.remoteLocatorList = mp_builtinProtocols->m_initialPeersList;
    //Rparam.topic.topicName = "DCPSRTPSParticipantMessage";
    //Rparam.topic.topicDataType = "RTPSParticipantMessageData";
    ratt.endpoint.topicKind = WITH_KEY;
    //LISTENER CREATION
    mp_listener = new WLPListener(this);
    RTPSReader* rout;
    if(mp_participant->createReader(&rout,ratt,mp_builtinReaderHistory,(ReaderListener*)mp_listener,c_EntityId_ReaderLiveliness,true))
    {
        mp_builtinReader = dynamic_cast<StatefulReader*>(rout);
        logInfo(RTPS_LIVELINESS,"Builtin Liveliness Reader created");
    }
    else
    {
        logError(RTPS_LIVELINESS,"Liveliness Reader Creation failed.");
        delete(mp_builtinReaderHistory);
        mp_builtinReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    return true;
}

#if HAVE_SECURITY

bool WLP::createSecureEndpoints()
{
    //CREATE WRITER
    HistoryAttributes hatt;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 1000;
    hatt.payloadMaxSize = BUILTIN_PARTICIPANT_DATA_MAX_SIZE;
    mp_builtinWriterSecureHistory = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
    watt.endpoint.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
    //	Wparam.topic.topicName = "DCPSParticipantMessageSecure";
    //	Wparam.topic.topicDataType = "RTPSParticipantMessageData";
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = RELIABLE;
    if (mp_participant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
        mp_participant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;

    const security::ParticipantSecurityAttributes& part_attrs = mp_participant->security_attributes();
    security::PluginParticipantSecurityAttributes plugin_attrs(part_attrs.plugin_participant_attributes);
    security::EndpointSecurityAttributes* sec_attrs = &watt.endpoint.security_attributes();
    sec_attrs->is_submessage_protected = part_attrs.is_liveliness_protected;
    if (part_attrs.is_liveliness_protected)
    {
        sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (plugin_attrs.is_liveliness_encrypted)
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        if (plugin_attrs.is_liveliness_origin_authenticated)
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
    }

    RTPSWriter* wout;
    if (mp_participant->createWriter(&wout, watt, mp_builtinWriterSecureHistory, nullptr, c_EntityId_WriterLivelinessSecure, true))
    {
        mp_builtinWriterSecure = dynamic_cast<StatefulWriter*>(wout);
        logInfo(RTPS_LIVELINESS, "Builtin Secure Liveliness Writer created");
    }
    else
    {
        logError(RTPS_LIVELINESS, "Secure Liveliness Writer Creation failed ");
        delete(mp_builtinWriterSecureHistory);
        mp_builtinWriterSecureHistory = nullptr;
        return false;
    }
    hatt.initialReservedCaches = 100;
    hatt.maximumReservedCaches = 2000;
    hatt.payloadMaxSize = BUILTIN_PARTICIPANT_DATA_MAX_SIZE;
    mp_builtinReaderSecureHistory = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.expectsInlineQos = true;
    ratt.endpoint.unicastLocatorList = mp_builtinProtocols->m_metatrafficUnicastLocatorList;
    ratt.endpoint.multicastLocatorList = mp_builtinProtocols->m_metatrafficMulticastLocatorList;
    //Rparam.topic.topicName = "DCPSParticipantMessageSecure";
    //Rparam.topic.topicDataType = "RTPSParticipantMessageData";
    ratt.endpoint.topicKind = WITH_KEY;
    sec_attrs = &ratt.endpoint.security_attributes();
    sec_attrs->is_submessage_protected = part_attrs.is_liveliness_protected;
    if (part_attrs.is_liveliness_protected)
    {
        sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (plugin_attrs.is_liveliness_encrypted)
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        if (plugin_attrs.is_liveliness_origin_authenticated)
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
    }
    RTPSReader* rout;
    if (mp_participant->createReader(&rout, ratt, mp_builtinReaderSecureHistory, (ReaderListener*)mp_listener, c_EntityId_ReaderLivelinessSecure, true))
    {
        mp_builtinReaderSecure = dynamic_cast<StatefulReader*>(rout);
        logInfo(RTPS_LIVELINESS, "Builtin Liveliness Reader created");
    }
    else
    {
        logError(RTPS_LIVELINESS, "Liveliness Reader Creation failed.");
        delete(mp_builtinReaderSecureHistory);
        mp_builtinReaderSecureHistory = nullptr;
        return false;
    }

    return true;
}

bool WLP::pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
    const ReaderProxyData& remote_reader_data)
{
    if (local_writer.entityId == c_EntityId_WriterLivelinessSecure)
    {
        RemoteReaderAttributes attrs = remote_reader_data.toRemoteReaderAttributes();
        mp_builtinWriterSecure->matched_reader_add(attrs);
        return true;
    }

    return false;
}

bool WLP::pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
    const WriterProxyData& remote_writer_data)
{
    if (local_reader.entityId == c_EntityId_ReaderLivelinessSecure)
    {
        RemoteWriterAttributes attrs = remote_writer_data.toRemoteWriterAttributes();
        mp_builtinReaderSecure->matched_writer_add(attrs);
        return true;
    }

    return false;
}

#endif

bool WLP::assignRemoteEndpoints(const ParticipantProxyData& pdata)
{
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

    if((auxendp!=0 || partdet!=0) && this->mp_builtinReader!=nullptr)
    {
        logInfo(RTPS_LIVELINESS,"Adding remote writer to my local Builtin Reader");
        RemoteWriterAttributes watt(pdata.m_VendorId);
        watt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_WriterLiveliness;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.endpoint.reliabilityKind = RELIABLE;
        mp_builtinReader->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    if((auxendp!=0 || partdet!=0) && this->mp_builtinWriter!=nullptr)
    {
        logInfo(RTPS_LIVELINESS,"Adding remote reader to my local Builtin Writer");
        RemoteReaderAttributes ratt(pdata.m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_ReaderLiveliness;
        ratt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        mp_builtinWriter->matched_reader_add(ratt);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinReaderSecure != nullptr)
    {
        logInfo(RTPS_LIVELINESS, "Adding remote writer to my local Builtin Secure Reader");
        WriterProxyData watt;
        watt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid().entityId = c_EntityId_WriterLivelinessSecure;
        watt.persistence_guid(watt.guid());
        watt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        watt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        watt.topicKind(WITH_KEY);
        watt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        watt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        if(!mp_participant->security_manager().discovered_builtin_writer(
            mp_builtinReaderSecure->getGuid(), pdata.m_guid, watt,
            mp_builtinReaderSecure->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for reader " <<
                mp_builtinReaderSecure->getGuid());
        }
    }
    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinWriterSecure != nullptr)
    {
        logInfo(RTPS_LIVELINESS, "Adding remote reader to my local Builtin Secure Writer");
        ReaderProxyData ratt;
        ratt.m_expectsInlineQos = false;
        ratt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid().entityId = c_EntityId_ReaderLivelinessSecure;
        ratt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        ratt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        ratt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        ratt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        ratt.topicKind(WITH_KEY);
        if (!mp_participant->security_manager().discovered_builtin_reader(
            mp_builtinWriterSecure->getGuid(), pdata.m_guid, ratt,
            mp_builtinWriterSecure->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                mp_builtinWriterSecure->getGuid());
        }
    }
#endif

    return true;
}

void WLP::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->m_guid.guidPrefix;

    logInfo(RTPS_LIVELINESS,"for RTPSParticipant: "<<pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

    if((auxendp!=0 || partdet!=0) && this->mp_builtinReader!=nullptr)
    {
        logInfo(RTPS_LIVELINESS,"Removing remote writer from my local Builtin Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_WriterLiveliness;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.endpoint.reliabilityKind = RELIABLE;
        mp_builtinReader->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &=BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    if((auxendp!=0 || partdet!=0) && this->mp_builtinWriter!=nullptr)
    {
        logInfo(RTPS_LIVELINESS,"Removing remote reader from my local Builtin Writer");
        tmp_guid.entityId = c_EntityId_ReaderLiveliness;
        mp_builtinWriter->matched_reader_remove(tmp_guid);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinReaderSecure != nullptr)
    {
        logInfo(RTPS_LIVELINESS, "Removing remote writer from my local Builtin Secure Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_WriterLivelinessSecure;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.security_attributes() = mp_builtinReaderSecure->getAttributes().security_attributes();
        if (mp_builtinReaderSecure->matched_writer_remove(watt))
        {
            mp_participant->security_manager().remove_writer(
                mp_builtinReaderSecure->getGuid(), pdata->m_guid, watt.guid);
        }
    }
    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinWriterSecure != nullptr)
    {
        logInfo(RTPS_LIVELINESS, "Removing remote reader from my local Builtin Secure Writer");
        tmp_guid.entityId = c_EntityId_ReaderLivelinessSecure;
        if (mp_builtinWriterSecure->matched_reader_remove(tmp_guid))
        {
            mp_participant->security_manager().remove_reader(
                mp_builtinWriterSecure->getGuid(), pdata->m_guid, tmp_guid);
        }
    }
#endif
}



bool WLP::addLocalWriter(RTPSWriter* W, const WriterQos& wqos)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());
    logInfo(RTPS_LIVELINESS,W->getGuid().entityId	<<" to Liveliness Protocol")
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
            {
                mp_livelinessAutomatic->cancel_timer();
            }
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
            {
                mp_livelinessManRTPSParticipant->cancel_timer();
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
    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());
    logInfo(RTPS_LIVELINESS,W->getGuid().entityId
            <<" from Liveliness Protocol");
    t_WIT wToEraseIt;
    WriterProxyData wdata;
    if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData(W->getGuid(), wdata))
    {
        bool found = false;
        if(wdata.m_qos.m_liveliness.kind == AUTOMATIC_LIVELINESS_QOS)
        {
            m_minAutomatic_MilliSec = std::numeric_limits<double>::max();
            for(t_WIT it= m_livAutomaticWriters.begin();it!=m_livAutomaticWriters.end();++it)
            {
                WriterProxyData wdata2;
                if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData((*it)->getGuid(), wdata2))
                {
                    double mintimeWIT(TimeConv::Time_t2MilliSecondsDouble(wdata2.m_qos.m_liveliness.announcement_period));
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
        else if(wdata.m_qos.m_liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
        {
            m_minManRTPSParticipant_MilliSec = std::numeric_limits<double>::max();
            for(t_WIT it= m_livManRTPSParticipantWriters.begin();it!=m_livManRTPSParticipantWriters.end();++it)
            {
                WriterProxyData wdata2;
                if(this->mp_builtinProtocols->mp_PDP->lookupWriterProxyData((*it)->getGuid(), wdata2))
                {
                    double mintimeWIT(TimeConv::Time_t2MilliSecondsDouble(wdata2.m_qos.m_liveliness.announcement_period));
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
    logWarning(RTPS_LIVELINESS,"Writer "<<W->getGuid().entityId << " not found.");
    return false;
}

bool WLP::updateLocalWriter(RTPSWriter* W, const WriterQos& wqos)
{

    // Unused in release mode.
    (void)W;

    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());
    logInfo(RTPS_LIVELINESS,W->getGuid().entityId);
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
            {
                mp_livelinessAutomatic->cancel_timer();
            }
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
            {
                mp_livelinessManRTPSParticipant->cancel_timer();
            }
            mp_livelinessManRTPSParticipant->restart_timer();
        }
    }
    return true;
}

StatefulWriter* WLP::getBuiltinWriter()
{
    StatefulWriter* ret_val = mp_builtinWriter;

#if HAVE_SECURITY
    if (mp_participant->security_attributes().is_liveliness_protected)
    {
        ret_val = mp_builtinWriterSecure;
    }
#endif

    return ret_val;
}

WriterHistory* WLP::getBuiltinWriterHistory()
{
    WriterHistory* ret_val = mp_builtinWriterHistory;

#if HAVE_SECURITY
    if (mp_participant->security_attributes().is_liveliness_protected)
    {
        ret_val = mp_builtinWriterSecureHistory;
    }
#endif

    return ret_val;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
