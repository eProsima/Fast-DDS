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
 * @file EDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include "EDPSimpleListeners.h"
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>


#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {

// Default configuration values for EDP entities.
const Duration_t edp_heartbeat_period{1, 0}; // 1 second
const Duration_t edp_nack_response_delay{0, 400*1000*1000}; // ~93 milliseconds
const Duration_t edp_nack_supression_duration{0, 50*1000*1000}; // ~11 milliseconds
const Duration_t edp_heartbeat_response_delay{0, 50*1000*1000}; // ~11 milliseconds

const int32_t edp_initial_reserved_caches = 20;


EDPSimple::EDPSimple(
        PDPSimple* p,
        RTPSParticipantImpl* part)
    : EDP(p,part)
    , publications_listener_(nullptr)
    , subscriptions_listener_(nullptr)
{
}

EDPSimple::~EDPSimple()
{
#if HAVE_SECURITY
    if(this->publications_secure_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_secure_writer_.first);
        delete(publications_secure_writer_.second);
    }

    if(this->publications_secure_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_secure_reader_.first);
        delete(publications_secure_reader_.second);
    }

    if(this->subscriptions_secure_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_secure_writer_.first);
        delete(subscriptions_secure_writer_.second);
    }

    if(this->subscriptions_secure_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_secure_reader_.first);
        delete(subscriptions_secure_reader_.second);
    }
#endif

    if(this->publications_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_reader_.first);
        delete(publications_reader_.second);
    }
    if(this->subscriptions_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_reader_.first);
        delete(subscriptions_reader_.second);
    }
    if(this->publications_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_writer_.first);
        delete(publications_writer_.second);
    }
    if(this->subscriptions_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_writer_.first);
        delete(subscriptions_writer_.second);
    }

    if(nullptr != publications_listener_)
    {
        delete(publications_listener_);
    }

    if(nullptr != subscriptions_listener_)
    {
        delete(subscriptions_listener_);
    }
}


bool EDPSimple::initEDP(BuiltinAttributes& attributes)
{
    logInfo(RTPS_EDP,"Beginning Simple Endpoint Discovery Protocol");
    m_discovery = attributes;

    if(!createSEDPEndpoints())
    {
        logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
        return false;
    }

#if HAVE_SECURITY
    if(!create_sedp_secure_endpoints())
    {
        logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
        return false;
    }
#endif

    return true;
}


bool EDPSimple::createSEDPEndpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes hatt;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;

    publications_listener_ = new EDPSimplePUBListener(this);
    subscriptions_listener_ = new EDPSimpleSUBListener(this);

    if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        publications_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.heartbeatPeriod = edp_heartbeat_period;
        watt.times.nackResponseDelay = edp_nack_response_delay;
        watt.times.nackSupressionDuration = edp_nack_supression_duration;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;

        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, publications_writer_.second,
                publications_listener_, c_EntityId_SEDPPubWriter, true);

        if(created)
        {
            publications_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Publication Writer created");
        }
        else
        {
            delete(publications_writer_.second);
            publications_writer_.second = nullptr;
        }

        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        subscriptions_reader_.second = new ReaderHistory(hatt);
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay = edp_heartbeat_response_delay;

        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_reader_.second,
                subscriptions_listener_, c_EntityId_SEDPSubReader, true);

        if(created)
        {
            subscriptions_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Subscription Reader created");
        }
        else
        {
            delete(subscriptions_reader_.second);
            subscriptions_reader_.second = nullptr;
        }
    }
    if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        publications_reader_.second = new ReaderHistory(hatt);
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay = edp_heartbeat_response_delay;

        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, publications_reader_.second,
                publications_listener_, c_EntityId_SEDPPubReader, true);

        if(created)
        {
            publications_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Publication Reader created");

        }
        else
        {
            delete(publications_reader_.second);
            publications_reader_.second = nullptr;
        }

        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        subscriptions_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.heartbeatPeriod= edp_heartbeat_period;
        watt.times.nackResponseDelay = edp_nack_response_delay;
        watt.times.nackSupressionDuration = edp_nack_supression_duration;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;

        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_writer_.second,
                subscriptions_listener_, c_EntityId_SEDPSubWriter, true);

        if(created)
        {
            subscriptions_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Subscription Writer created");

        }
        else
        {
            delete(subscriptions_writer_.second);
            subscriptions_writer_.second = nullptr;
        }
    }
    logInfo(RTPS_EDP,"Creation finished");
    return created;
}

#if HAVE_SECURITY
bool EDPSimple::create_sedp_secure_endpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes hatt;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;

    auto& part_attr = mp_RTPSParticipant->security_attributes();
    security::PluginParticipantSecurityAttributes plugin_part_attr(part_attr.plugin_participant_attributes);

    if(m_discovery.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        publications_secure_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.heartbeatPeriod = edp_heartbeat_period;
        watt.times.nackResponseDelay = edp_nack_response_delay;
        watt.times.nackSupressionDuration = edp_nack_supression_duration;
        watt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
        watt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (part_attr.is_discovery_protected)
        {
            if (plugin_part_attr.is_discovery_encrypted)
                watt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            if (plugin_part_attr.is_discovery_origin_authenticated)
                watt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }

        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;

        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, publications_secure_writer_.second,
                publications_listener_, sedp_builtin_publications_secure_writer, true);

        if(created)
        {
            publications_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Publication Writer created");
        }
        else
        {
            delete(publications_secure_writer_.second);
            publications_secure_writer_.second = nullptr;
        }
        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        subscriptions_secure_reader_.second = new ReaderHistory(hatt);
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay = edp_heartbeat_response_delay;
        ratt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
        ratt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (part_attr.is_discovery_protected)
        {
            if (plugin_part_attr.is_discovery_encrypted)
                ratt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            if (plugin_part_attr.is_discovery_origin_authenticated)
                ratt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }

        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_secure_reader_.second,
                subscriptions_listener_, sedp_builtin_subscriptions_secure_reader, true);

        if(created)
        {
            subscriptions_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Subscription Reader created");
        }
        else
        {
            delete(subscriptions_secure_reader_.second);
            subscriptions_secure_reader_.second = nullptr;
        }
    }

    if(m_discovery.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        publications_secure_reader_.second = new ReaderHistory(hatt);
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay = edp_heartbeat_response_delay;
        ratt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
        ratt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (part_attr.is_discovery_protected)
        {
            if (plugin_part_attr.is_discovery_encrypted)
                ratt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            if (plugin_part_attr.is_discovery_origin_authenticated)
                ratt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }

        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, publications_secure_reader_.second,
                publications_listener_, sedp_builtin_publications_secure_reader, true);

        if(created)
        {
            publications_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Publication Reader created");

        }
        else
        {
            delete(publications_secure_reader_.second);
            publications_secure_reader_.second = nullptr;
        }

        hatt.initialReservedCaches = edp_initial_reserved_caches;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        subscriptions_secure_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.heartbeatPeriod = edp_heartbeat_period;
        watt.times.nackResponseDelay = edp_nack_response_delay;
        watt.times.nackSupressionDuration = edp_nack_supression_duration;
        watt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
        watt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (part_attr.is_discovery_protected)
        {
            if (plugin_part_attr.is_discovery_encrypted)
                watt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            if (plugin_part_attr.is_discovery_origin_authenticated)
                watt.endpoint.security_attributes().plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;

        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_secure_writer_.second,
            subscriptions_listener_, sedp_builtin_subscriptions_secure_writer, true);

        if(created)
        {
            subscriptions_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Subscription Writer created");

        }
        else
        {
            delete(subscriptions_secure_writer_.second);
            subscriptions_secure_writer_.second = nullptr;
        }
    }
    logInfo(RTPS_EDP,"Creation finished");
    return created;
}
#endif

bool EDPSimple::processLocalReaderProxyData(RTPSReader* local_reader, ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP,rdata->guid().entityId);
    (void)local_reader;

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if(local_reader->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif

    if(writer->first != nullptr)
    {
        // TODO(Ricardo) Write a getCdrSerializedPayload for ReaderProxyData.
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;},
                ALIVE,rdata->key());

        if(change !=nullptr)
        {
            CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            rdata->writeToCDRMessage(&aux_msg, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            writer->second->add_change(change);

            return true;
        }

        return false;
    }

    return true;
}
bool EDPSimple::processLocalWriterProxyData(RTPSWriter* local_writer, WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, wdata->guid().entityId);
    (void)local_writer;

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if(local_writer->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif

    if(writer->first !=nullptr)
    {
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;},
                ALIVE, wdata->key());
        if(change != nullptr)
        {
            CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            wdata->writeToCDRMessage(&aux_msg, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            writer->second->add_change(change);

            return true;
        }
        return false;
    }
    return true;
}

bool EDPSimple::removeLocalWriter(RTPSWriter* W)
{
    logInfo(RTPS_EDP,W->getGuid().entityId);

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if(W->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif

    if(writer->first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = W->getGuid();
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;},
                NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }

            }

            writer->second->add_change(change);
        }
    }
    return mp_PDP->removeWriterProxyData(W->getGuid());
}

bool EDPSimple::removeLocalReader(RTPSReader* R)
{
    logInfo(RTPS_EDP,R->getGuid().entityId);

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if(R->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif

    if(writer->first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = (R->getGuid());
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;},
                NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            writer->second->add_change(change);
        }
    }
    return mp_PDP->removeReaderProxyData(R->getGuid());
}



void EDPSimple::assignRemoteEndpoints(const ParticipantProxyData& pdata)
{
    logInfo(RTPS_EDP,"New DPD received, adding remote endpoints to our SimpleEDP endpoints");
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && publications_reader_.first!=nullptr) //Exist Pub Writer and i have pub reader
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Writer to my Pub Reader");
        RemoteWriterAttributes watt(pdata.m_VendorId);
        watt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPPubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        publications_reader_.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && publications_writer_.first!=nullptr) //Exist Pub Detector
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Reader to my Pub Writer");
        RemoteReaderAttributes ratt(pdata.m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPPubReader;
        ratt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        publications_writer_.first->matched_reader_add(ratt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && subscriptions_reader_.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt(pdata.m_VendorId);
        watt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPSubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        subscriptions_reader_.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && subscriptions_writer_.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        RemoteReaderAttributes ratt(pdata.m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPSubReader;
        ratt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        //ratt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        subscriptions_writer_.first->matched_reader_add(ratt);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && publications_secure_reader_.first != nullptr)
    {
        WriterProxyData watt;
        watt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid().entityId = sedp_builtin_publications_secure_writer;
        watt.persistence_guid(watt.guid());
        watt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        watt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        watt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        watt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    publications_secure_reader_.first->getGuid(), pdata.m_guid, watt,
                    publications_secure_reader_.first->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    publications_secure_reader_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && publications_secure_writer_.first!=nullptr)
    {
        ReaderProxyData ratt;
        ratt.m_expectsInlineQos = false;
        ratt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid().entityId = sedp_builtin_publications_secure_reader;
        ratt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        ratt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        ratt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        ratt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    publications_secure_writer_.first->getGuid(), pdata.m_guid, ratt,
                    publications_secure_writer_.first->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    publications_secure_writer_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && subscriptions_secure_reader_.first != nullptr)
    {
        WriterProxyData watt;
        watt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid().entityId = sedp_builtin_subscriptions_secure_writer;
        watt.persistence_guid(watt.guid());
        watt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        watt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        watt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        watt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    subscriptions_secure_reader_.first->getGuid(), pdata.m_guid, watt,
                    subscriptions_secure_reader_.first->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    subscriptions_secure_reader_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && subscriptions_secure_writer_.first!=nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        ReaderProxyData ratt;
        ratt.m_expectsInlineQos = false;
        ratt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid().entityId = sedp_builtin_subscriptions_secure_reader;
        ratt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        ratt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        ratt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        ratt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    subscriptions_secure_writer_.first->getGuid(), pdata.m_guid, ratt,
                    subscriptions_secure_writer_.first->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    subscriptions_secure_writer_.first->getGuid());
        }
    }
#endif
}


void EDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_EDP,"For RTPSParticipant: "<<pdata->m_guid);

    GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->m_guid.guidPrefix;

    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && publications_reader_.first!=nullptr) //Exist Pub Writer and i have pub reader
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPPubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        publications_reader_.first->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && publications_writer_.first!=nullptr) //Exist Pub Detector
    {
        tmp_guid.entityId = c_EntityId_SEDPPubReader;
        publications_writer_.first->matched_reader_remove(tmp_guid);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && subscriptions_reader_.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPSubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        subscriptions_reader_.first->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && subscriptions_writer_.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        tmp_guid.entityId = c_EntityId_SEDPSubReader;
        subscriptions_writer_.first->matched_reader_remove(tmp_guid);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && publications_secure_reader_.first != nullptr)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = sedp_builtin_publications_secure_writer;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        if(publications_secure_reader_.first->matched_writer_remove(watt))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                    publications_secure_reader_.first->getGuid(), pdata->m_guid, watt.guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && publications_secure_writer_.first != nullptr)
    {
        tmp_guid.entityId = sedp_builtin_publications_secure_reader;
        if(publications_secure_writer_.first->matched_reader_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                    publications_secure_writer_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && subscriptions_secure_reader_.first != nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = sedp_builtin_subscriptions_secure_writer;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        //watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        if(subscriptions_secure_reader_.first->matched_writer_remove(watt))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                    subscriptions_secure_reader_.first->getGuid(), pdata->m_guid, watt.guid);
        }
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && subscriptions_secure_writer_.first!=nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        tmp_guid.entityId = sedp_builtin_subscriptions_secure_reader;
        if(subscriptions_secure_writer_.first->matched_reader_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                    subscriptions_secure_writer_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }
#endif
}

#if HAVE_SECURITY
bool EDPSimple::pairing_remote_writer_with_local_builtin_reader_after_security(const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    bool returned_value = false;

    if(local_reader.entityId == sedp_builtin_publications_secure_reader)
    {
        RemoteWriterAttributes attrs = remote_writer_data.toRemoteWriterAttributes();
        publications_secure_reader_.first->matched_writer_add(attrs);
        returned_value = true;
    }
    else if(local_reader.entityId == sedp_builtin_subscriptions_secure_reader)
    {
        RemoteWriterAttributes attrs = remote_writer_data.toRemoteWriterAttributes();
        subscriptions_secure_reader_.first->matched_writer_add(attrs);
        returned_value = true;
    }

    return returned_value;
}

bool EDPSimple::pairing_remote_reader_with_local_builtin_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    bool returned_value = false;

    if(local_writer.entityId == sedp_builtin_publications_secure_writer)
    {
        RemoteReaderAttributes attrs = remote_reader_data.toRemoteReaderAttributes();
        publications_secure_writer_.first->matched_reader_add(attrs);
        returned_value = true;
    }
    else if(local_writer.entityId == sedp_builtin_subscriptions_secure_writer)
    {
        RemoteReaderAttributes attrs = remote_reader_data.toRemoteReaderAttributes();
        subscriptions_secure_writer_.first->matched_reader_add(attrs);
        returned_value = true;
    }

    return returned_value;
}
#endif

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
