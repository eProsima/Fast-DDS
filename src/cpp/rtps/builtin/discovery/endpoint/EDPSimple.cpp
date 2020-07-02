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
#include <fastrtps/rtps/builtin/discovery/participant/PDP.h>
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
#include <forward_list>
#include <algorithm>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Default configuration values for EDP entities.
static const Duration_t edp_heartbeat_period{1, 0}; // 1 second
static const Duration_t edp_nack_response_delay{0, 100 * 1000 }; // 100 milliseconds
static const Duration_t edp_nack_supression_duration{0, 10 * 1000}; // 10 milliseconds
static const Duration_t edp_heartbeat_response_delay{0, 10 * 1000}; // 10 milliseconds

static const int32_t edp_initial_reserved_caches = 20;


EDPSimple::EDPSimple(
        PDP* p,
        RTPSParticipantImpl* part)
    : EDP(p, part)
    , publications_listener_(nullptr)
    , subscriptions_listener_(nullptr)
    , temp_reader_proxy_data_(
        part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators)
    , temp_writer_proxy_data_(
        part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators)
{
}

EDPSimple::~EDPSimple()
{
#if HAVE_SECURITY
    if (this->publications_secure_writer_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_secure_writer_.first);
        delete(publications_secure_writer_.second);
    }

    if (this->publications_secure_reader_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_secure_reader_.first);
        delete(publications_secure_reader_.second);
    }

    if (this->subscriptions_secure_writer_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_secure_writer_.first);
        delete(subscriptions_secure_writer_.second);
    }

    if (this->subscriptions_secure_reader_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_secure_reader_.first);
        delete(subscriptions_secure_reader_.second);
    }
#endif // if HAVE_SECURITY

    if (this->publications_reader_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_reader_.first);
        delete(publications_reader_.second);
    }
    if (this->subscriptions_reader_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_reader_.first);
        delete(subscriptions_reader_.second);
    }
    if (this->publications_writer_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(publications_writer_.first);
        delete(publications_writer_.second);
    }
    if (this->subscriptions_writer_.first != nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(subscriptions_writer_.first);
        delete(subscriptions_writer_.second);
    }

    if (nullptr != publications_listener_)
    {
        delete(publications_listener_);
    }

    if (nullptr != subscriptions_listener_)
    {
        delete(subscriptions_listener_);
    }
}

bool EDPSimple::initEDP(
        BuiltinAttributes& attributes)
{
    logInfo(RTPS_EDP, "Beginning Simple Endpoint Discovery Protocol");
    m_discovery = attributes;

    if (!createSEDPEndpoints())
    {
        logError(RTPS_EDP, "Problem creation SimpleEDP endpoints");
        return false;
    }

#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure() && !create_sedp_secure_endpoints())
    {
        logError(RTPS_EDP, "Problem creation SimpleEDP endpoints");
        return false;
    }
#endif // if HAVE_SECURITY

    return true;
}

//! Process the info recorded in the persistence database
void EDPSimple::processPersistentData(
        t_p_StatefulReader& reader,
        t_p_StatefulWriter& writer,
        key_list& demises)
{
    std::lock_guard<RecursiveTimedMutex> guardR(reader.first->getMutex());
    std::lock_guard<RecursiveTimedMutex> guardW(writer.first->getMutex());
    std::lock_guard<std::recursive_mutex> guardP(*mp_PDP->getMutex());

    // own server instance
    InstanceHandle_t server_key = mp_PDP->getLocalParticipantProxyData()->m_key;

    // reference own references from writer history
    std::forward_list<CacheChange_t*> removal;

    // List known participants
    key_list known_participants;

    std::for_each(
        mp_PDP->ParticipantProxiesBegin(),
        mp_PDP->ParticipantProxiesEnd(),
        [&known_participants](const ParticipantProxyData* pD)
        {
            known_participants.insert(pD->m_key);
        });

    // We have not processed any PDP message yet
    assert(demises.empty());

    std::for_each(writer.second->changesBegin(),
            writer.second->changesEnd(),
            [&reader, &known_participants, &demises, &removal, &server_key](CacheChange_t* change)
            {
                // Get Participant InstanceHandle
                InstanceHandle_t handle;
                {
                    GUID_t guid = iHandle2GUID(change->instanceHandle);
                    guid.entityId = c_EntityId_RTPSParticipant;
                    handle = guid;
                }

                // mark for removal endpoints from unknown participants
                if ( known_participants.find(handle) == known_participants.end() )
                {
                    demises.insert(change->instanceHandle);
                    return;
                }

                // check if its own data: mark for removal and ignore
                if ( handle == server_key)
                {
                    removal.push_front(change);
                    return;
                }

                CacheChange_t* change_to_add = nullptr;

                if (!reader.first->reserveCache(&change_to_add, change->serializedPayload.length)) //Reserve a new cache from the corresponding cache pool
                {
                    logError(RTPS_EDP, "Problem reserving CacheChange in EDPServer reader");
                    return;
                }

                if (!change_to_add->copy(change))
                {
                    logWarning(RTPS_EDP, "Problem copying CacheChange, received data is: "
                        << change->serializedPayload.length << " bytes and max size in EDPServer reader"
                        << " is " << change_to_add->serializedPayload.max_size);

                    reader.first->releaseCache(change_to_add);
                    return;
                }

                if (!reader.first->change_received(change_to_add, nullptr))
                {
                    logInfo(RTPS_EDP, "EDPServer couldn't process database data not add change "
                        << change_to_add->sequenceNumber);
                    reader.first->releaseCache(change_to_add);
                }

                // change_to_add would be released within change_received
            });

    // remove our own old server samples
    for (auto pC : removal)
    {
        writer.second->remove_change(pC);
    }

    // We don't need to awake the server thread because we are in it
}

void EDPSimple::set_builtin_reader_history_attributes(
        HistoryAttributes& attributes)
{
    attributes.initialReservedCaches = edp_initial_reserved_caches;
    attributes.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
    attributes.memoryPolicy = mp_PDP->builtin_attributes().readerHistoryMemoryPolicy;
}

void EDPSimple::set_builtin_writer_history_attributes(
        HistoryAttributes& attributes)
{
    attributes.initialReservedCaches = edp_initial_reserved_caches;
    attributes.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
    attributes.memoryPolicy = mp_PDP->builtin_attributes().writerHistoryMemoryPolicy;
}

void EDPSimple::set_builtin_reader_attributes(
        ReaderAttributes& attributes)
{
    // Matched writers will depend on total number of participants
    attributes.matched_writers_allocation =
            mp_PDP->getRTPSParticipant()->getRTPSParticipantAttributes().allocation.participants;

    // As participants allocation policy includes the local participant, one has to be substracted
    if (attributes.matched_writers_allocation.initial > 1)
    {
        attributes.matched_writers_allocation.initial--;
    }
    if ((attributes.matched_writers_allocation.maximum > 1) &&
            (attributes.matched_writers_allocation.maximum < std::numeric_limits<size_t>::max()))
    {
        attributes.matched_writers_allocation.maximum--;
    }

    // Locators are copied from the local participant metatraffic locators
    attributes.endpoint.unicastLocatorList.clear();
    for (const Locator_t& loc : this->mp_PDP->getLocalParticipantProxyData()->metatraffic_locators.unicast)
    {
        attributes.endpoint.unicastLocatorList.push_back(loc);
    }
    attributes.endpoint.multicastLocatorList.clear();
    for (const Locator_t& loc : this->mp_PDP->getLocalParticipantProxyData()->metatraffic_locators.multicast)
    {
        attributes.endpoint.multicastLocatorList.push_back(loc);
    }

    // Timings are configured using EDP default values
    attributes.times.heartbeatResponseDelay = edp_heartbeat_response_delay;

    // EDP endpoints are always reliable, transsient local, keyed topics
    attributes.endpoint.reliabilityKind = RELIABLE;
    attributes.endpoint.durabilityKind = TRANSIENT_LOCAL;
    attributes.endpoint.topicKind = WITH_KEY;

    // Built-in EDP readers never expect inline qos
    attributes.expectsInlineQos = false;
}

void EDPSimple::set_builtin_writer_attributes(
        WriterAttributes& attributes)
{
    // Matched readers will depend on total number of participants
    attributes.matched_readers_allocation =
            mp_PDP->getRTPSParticipant()->getRTPSParticipantAttributes().allocation.participants;

    // As participants allocation policy includes the local participant, one has to be substracted
    if (attributes.matched_readers_allocation.initial > 1)
    {
        attributes.matched_readers_allocation.initial--;
    }
    if ((attributes.matched_readers_allocation.maximum > 1) &&
            (attributes.matched_readers_allocation.maximum < std::numeric_limits<size_t>::max()))
    {
        attributes.matched_readers_allocation.maximum--;
    }

    // Locators are copied from the local participant metatraffic locators
    attributes.endpoint.unicastLocatorList.clear();
    for (const Locator_t& loc : this->mp_PDP->getLocalParticipantProxyData()->metatraffic_locators.unicast)
    {
        attributes.endpoint.unicastLocatorList.push_back(loc);
    }
    attributes.endpoint.multicastLocatorList.clear();
    for (const Locator_t& loc : this->mp_PDP->getLocalParticipantProxyData()->metatraffic_locators.multicast)
    {
        attributes.endpoint.multicastLocatorList.push_back(loc);
    }

    // Timings are configured using EDP default values
    attributes.times.heartbeatPeriod = edp_heartbeat_period;
    attributes.times.nackResponseDelay = edp_nack_response_delay;
    attributes.times.nackSupressionDuration = edp_nack_supression_duration;

    // EDP endpoints are always reliable, transsient local, keyed topics
    attributes.endpoint.reliabilityKind = RELIABLE;
    attributes.endpoint.durabilityKind = TRANSIENT_LOCAL;
    attributes.endpoint.topicKind = WITH_KEY;

    // Set as asynchronous if there is a throughput controller installed
    if (mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
    {
        attributes.mode = ASYNCHRONOUS_WRITER;
    }
}

bool EDPSimple::createSEDPEndpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes reader_history_att;
    HistoryAttributes writer_history_att;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;

    set_builtin_reader_history_attributes(reader_history_att);
    set_builtin_writer_history_attributes(writer_history_att);
    set_builtin_reader_attributes(ratt);
    set_builtin_writer_attributes(watt);

    publications_listener_ = new EDPSimplePUBListener(this);
    subscriptions_listener_ = new EDPSimpleSUBListener(this);

    if (m_discovery.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        publications_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, publications_writer_.second,
                        publications_listener_, c_EntityId_SEDPPubWriter, true);

        if (created)
        {
            publications_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP, "SEDP Publication Writer created");
        }
        else
        {
            delete(publications_writer_.second);
            publications_writer_.second = nullptr;
        }

        subscriptions_reader_.second = new ReaderHistory(reader_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_reader_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubReader, true);

        if (created)
        {
            subscriptions_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Subscription Reader created");
        }
        else
        {
            delete(subscriptions_reader_.second);
            subscriptions_reader_.second = nullptr;
        }
    }
    if (m_discovery.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        publications_reader_.second = new ReaderHistory(reader_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, publications_reader_.second,
                        publications_listener_, c_EntityId_SEDPPubReader, true);

        if (created)
        {
            publications_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Publication Reader created");

        }
        else
        {
            delete(publications_reader_.second);
            publications_reader_.second = nullptr;
        }

        subscriptions_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_writer_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubWriter, true);

        if (created)
        {
            subscriptions_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP, "SEDP Subscription Writer created");

        }
        else
        {
            delete(subscriptions_writer_.second);
            subscriptions_writer_.second = nullptr;
        }
    }
    logInfo(RTPS_EDP, "Creation finished");
    return created;
}

#if HAVE_SECURITY
bool EDPSimple::create_sedp_secure_endpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes reader_history_att;
    HistoryAttributes writer_history_att;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;

    set_builtin_reader_history_attributes(reader_history_att);
    set_builtin_writer_history_attributes(writer_history_att);
    set_builtin_reader_attributes(ratt);
    set_builtin_writer_attributes(watt);

    const security::ParticipantSecurityAttributes& part_attr = mp_RTPSParticipant->security_attributes();

    ratt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
    ratt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;

    watt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
    watt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;

    if (part_attr.is_discovery_protected)
    {
        security::PluginParticipantSecurityAttributes plugin_part_attr(part_attr.plugin_participant_attributes);

        if (plugin_part_attr.is_discovery_encrypted)
        {
            ratt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            watt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        }
        if (plugin_part_attr.is_discovery_origin_authenticated)
        {
            ratt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
            watt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }
    }

    if (m_discovery.discovery_config.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        publications_secure_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, publications_secure_writer_.second,
                        publications_listener_, sedp_builtin_publications_secure_writer, true);

        if (created)
        {
            publications_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP, "SEDP Publication Writer created");
        }
        else
        {
            delete(publications_secure_writer_.second);
            publications_secure_writer_.second = nullptr;
        }
        subscriptions_secure_reader_.second = new ReaderHistory(reader_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_secure_reader_.second,
                        subscriptions_listener_, sedp_builtin_subscriptions_secure_reader, true);

        if (created)
        {
            subscriptions_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Subscription Reader created");
        }
        else
        {
            delete(subscriptions_secure_reader_.second);
            subscriptions_secure_reader_.second = nullptr;
        }
    }

    if (m_discovery.discovery_config.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        publications_secure_reader_.second = new ReaderHistory(reader_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, publications_secure_reader_.second,
                        publications_listener_, sedp_builtin_publications_secure_reader, true);

        if (created)
        {
            publications_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Publication Reader created");

        }
        else
        {
            delete(publications_secure_reader_.second);
            publications_secure_reader_.second = nullptr;
        }

        subscriptions_secure_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_secure_writer_.second,
                        subscriptions_listener_, sedp_builtin_subscriptions_secure_writer, true);

        if (created)
        {
            subscriptions_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP, "SEDP Subscription Writer created");

        }
        else
        {
            delete(subscriptions_secure_writer_.second);
            subscriptions_secure_writer_.second = nullptr;
        }
    }
    logInfo(RTPS_EDP, "Creation finished");
    return created;
}

#endif // if HAVE_SECURITY

bool EDPSimple::processLocalReaderProxyData(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP, rdata->guid().entityId);
    (void)local_reader;

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (local_reader->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        // TODO(Ricardo) Write a getCdrSerializedPayload for ReaderProxyData.
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t
                        {
                            return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
                        },
                        ALIVE, rdata->key());

        if (change != nullptr)
        {
            CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif // if __BIG_ENDIAN__

            rdata->writeToCDRMessage(&aux_msg, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<RecursiveTimedMutex> lock(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
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

bool EDPSimple::processLocalWriterProxyData(
        RTPSWriter* local_writer,
        WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, wdata->guid().entityId);
    (void)local_writer;

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (local_writer->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t
                        {
                            return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
                        },
                        ALIVE, wdata->key());
        if (change != nullptr)
        {
            CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif // if __BIG_ENDIAN__

            wdata->writeToCDRMessage(&aux_msg, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<RecursiveTimedMutex> lock(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
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

bool EDPSimple::removeLocalWriter(
        RTPSWriter* W)
{
    logInfo(RTPS_EDP, W->getGuid().entityId);

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (W->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = W->getGuid();
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t
                        {
                            return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
                        },
                        NOT_ALIVE_DISPOSED_UNREGISTERED, iH);
        if (change != nullptr)
        {
            {
                std::lock_guard<RecursiveTimedMutex> guard(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
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

bool EDPSimple::removeLocalReader(
        RTPSReader* R)
{
    logInfo(RTPS_EDP, R->getGuid().entityId);

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (R->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = (R->getGuid());
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t
                        {
                            return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
                        },
                        NOT_ALIVE_DISPOSED_UNREGISTERED, iH);
        if (change != nullptr)
        {
            {
                std::lock_guard<RecursiveTimedMutex> guard(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
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

void EDPSimple::assignRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    logInfo(RTPS_EDP, "New DPD received, adding remote endpoints to our SimpleEDP endpoints");
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    bool use_multicast_locators = !mp_PDP->getRTPSParticipant()->getAttributes().builtin.avoid_builtin_multicast ||
            pdata.metatraffic_locators.unicast.empty();
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;

    std::lock_guard<std::mutex> data_guard(temp_data_lock_);

    temp_reader_proxy_data_.clear();
    temp_reader_proxy_data_.m_expectsInlineQos = false;
    temp_reader_proxy_data_.guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_reader_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators);
    temp_reader_proxy_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_proxy_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    temp_writer_proxy_data_.clear();
    temp_writer_proxy_data_.guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_writer_proxy_data_.persistence_guid(pdata.get_persistence_guid());
    temp_writer_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators);
    temp_writer_proxy_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_writer_proxy_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_reader_.first != nullptr) //Exist Pub Writer and i have pub reader
    {
        logInfo(RTPS_EDP, "Adding SEDP Pub Writer to my Pub Reader");
        temp_writer_proxy_data_.guid().entityId = c_EntityId_SEDPPubWriter;
        temp_writer_proxy_data_.set_persistence_entity_id(c_EntityId_SEDPPubWriter);
        publications_reader_.first->matched_writer_add(temp_writer_proxy_data_);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_writer_.first != nullptr) //Exist Pub Detector
    {
        logInfo(RTPS_EDP, "Adding SEDP Pub Reader to my Pub Writer");
        temp_reader_proxy_data_.guid().entityId = c_EntityId_SEDPPubReader;
        publications_writer_.first->matched_reader_add(temp_reader_proxy_data_);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_reader_.first != nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Writer to my Sub Reader");
        temp_writer_proxy_data_.guid().entityId = c_EntityId_SEDPSubWriter;
        temp_writer_proxy_data_.set_persistence_entity_id(c_EntityId_SEDPSubWriter);
        subscriptions_reader_.first->matched_writer_add(temp_writer_proxy_data_);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_writer_.first != nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Reader to my Sub Writer");
        temp_reader_proxy_data_.guid().entityId = c_EntityId_SEDPSubReader;
        subscriptions_writer_.first->matched_reader_add(temp_reader_proxy_data_);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_secure_reader_.first != nullptr)
    {
        temp_writer_proxy_data_.guid().entityId = sedp_builtin_publications_secure_writer;
        temp_writer_proxy_data_.set_persistence_entity_id(sedp_builtin_publications_secure_writer);

        if (!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    publications_secure_reader_.first->getGuid(), pdata.m_guid, temp_writer_proxy_data_,
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
    if (auxendp != 0 && publications_secure_writer_.first != nullptr)
    {
        temp_reader_proxy_data_.guid().entityId = sedp_builtin_publications_secure_reader;
        if (!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    publications_secure_writer_.first->getGuid(), pdata.m_guid, temp_reader_proxy_data_,
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
    if (auxendp != 0 && subscriptions_secure_reader_.first != nullptr)
    {
        temp_writer_proxy_data_.guid().entityId = sedp_builtin_subscriptions_secure_writer;
        temp_writer_proxy_data_.set_persistence_entity_id(sedp_builtin_subscriptions_secure_writer);

        if (!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    subscriptions_secure_reader_.first->getGuid(), pdata.m_guid, temp_writer_proxy_data_,
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
    if (auxendp != 0 && subscriptions_secure_writer_.first != nullptr)
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Reader to my Sub Writer");
        temp_reader_proxy_data_.guid().entityId = sedp_builtin_subscriptions_secure_reader;
        if (!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    subscriptions_secure_writer_.first->getGuid(), pdata.m_guid, temp_reader_proxy_data_,
                    subscriptions_secure_writer_.first->getAttributes().security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    subscriptions_secure_writer_.first->getGuid());
        }
    }
#endif // if HAVE_SECURITY
}

void EDPSimple::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    logInfo(RTPS_EDP, "For RTPSParticipant: " << pdata->m_guid);

    GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->m_guid.guidPrefix;

    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_reader_.first != nullptr) //Exist Pub Writer and i have pub reader
    {
        tmp_guid.entityId = c_EntityId_SEDPPubWriter;
        publications_reader_.first->matched_writer_remove(tmp_guid);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_writer_.first != nullptr) //Exist Pub Detector
    {
        tmp_guid.entityId = c_EntityId_SEDPPubReader;
        publications_writer_.first->matched_reader_remove(tmp_guid);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_reader_.first != nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Writer to my Sub Reader");
        tmp_guid.entityId = c_EntityId_SEDPSubWriter;
        subscriptions_reader_.first->matched_writer_remove(tmp_guid);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_writer_.first != nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Reader to my Sub Writer");
        tmp_guid.entityId = c_EntityId_SEDPSubReader;
        subscriptions_writer_.first->matched_reader_remove(tmp_guid);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_secure_reader_.first != nullptr)
    {
        tmp_guid.entityId = sedp_builtin_publications_secure_writer;
        if (publications_secure_reader_.first->matched_writer_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                publications_secure_reader_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && publications_secure_writer_.first != nullptr)
    {
        tmp_guid.entityId = sedp_builtin_publications_secure_reader;
        if (publications_secure_writer_.first->matched_reader_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                publications_secure_writer_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_secure_reader_.first != nullptr)
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Writer to my Sub Reader");
        tmp_guid.entityId = sedp_builtin_subscriptions_secure_writer;
        if (subscriptions_secure_reader_.first->matched_writer_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                subscriptions_secure_reader_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if (auxendp != 0 && subscriptions_secure_writer_.first != nullptr)
    {
        logInfo(RTPS_EDP, "Adding SEDP Sub Reader to my Sub Writer");
        tmp_guid.entityId = sedp_builtin_subscriptions_secure_reader;
        if (subscriptions_secure_writer_.first->matched_reader_remove(tmp_guid))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                subscriptions_secure_writer_.first->getGuid(), pdata->m_guid, tmp_guid);
        }
    }
#endif // if HAVE_SECURITY
}

bool EDPSimple::areRemoteEndpointsMatched(
        const ParticipantProxyData* pdata)
{
    uint32_t endp = pdata->m_availableBuiltinEndpoints;

    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    if (auxendp != 0 && publications_reader_.first != nullptr) //Exist Pub Writer and I have Pub Reader
    {
        GUID_t wguid;
        wguid.guidPrefix = pdata->m_guid.guidPrefix;
        wguid.entityId = c_EntityId_SEDPPubWriter;

        if (!publications_reader_.first->matched_writer_is_matched(wguid))
        {
            return false;
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    if (auxendp != 0 && publications_writer_.first != nullptr) //Exist Pub Detector
    {
        GUID_t rguid;
        rguid.guidPrefix = pdata->m_guid.guidPrefix;
        rguid.entityId = c_EntityId_SEDPPubReader;

        if (!publications_writer_.first->matched_reader_is_matched(rguid))
        {
            return false;
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    if (auxendp != 0 && subscriptions_reader_.first != nullptr) //Exist Pub Announcer
    {
        GUID_t wguid;
        wguid.guidPrefix = pdata->m_guid.guidPrefix;
        wguid.entityId = c_EntityId_SEDPSubWriter;

        if (!subscriptions_reader_.first->matched_writer_is_matched(wguid))
        {
            return false;
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    if (auxendp != 0 && subscriptions_writer_.first != nullptr) //Exist Pub Announcer
    {
        GUID_t rguid;
        rguid.guidPrefix = pdata->m_guid.guidPrefix;
        rguid.entityId = c_EntityId_SEDPSubReader;

        if (!subscriptions_writer_.first->matched_reader_is_matched(rguid))
        {
            return false;
        }
    }

    return true;
}

#if HAVE_SECURITY
bool EDPSimple::pairing_remote_writer_with_local_builtin_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    bool returned_value = false;

    if (local_reader.entityId == sedp_builtin_publications_secure_reader)
    {
        publications_secure_reader_.first->matched_writer_add(remote_writer_data);
        returned_value = true;
    }
    else if (local_reader.entityId == sedp_builtin_subscriptions_secure_reader)
    {
        subscriptions_secure_reader_.first->matched_writer_add(remote_writer_data);
        returned_value = true;
    }

    return returned_value;
}

bool EDPSimple::pairing_remote_reader_with_local_builtin_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    bool returned_value = false;

    if (local_writer.entityId == sedp_builtin_publications_secure_writer)
    {
        publications_secure_writer_.first->matched_reader_add(remote_reader_data);
        returned_value = true;
    }
    else if (local_writer.entityId == sedp_builtin_subscriptions_secure_writer)
    {
        subscriptions_secure_writer_.first->matched_reader_add(remote_reader_data);
        returned_value = true;
    }

    return returned_value;
}

#endif // if HAVE_SECURITY

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
