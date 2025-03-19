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

#include <rtps/builtin/liveliness/WLP.hpp>

#include <limits>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/builtin/liveliness/WLPListener.h>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/resources/TimedEvent.h>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/LivelinessManager.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <utils/TimeConversion.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using BaseReader = fastdds::rtps::BaseReader;

static void set_builtin_reader_history_attributes(
        HistoryAttributes& hatt,
        const ResourceLimitedContainerConfig& allocation,
        bool is_secure)
{
    constexpr uint32_t c_upper_limit = (std::numeric_limits<uint32_t>::max)() / 2u;

    hatt.payloadMaxSize = is_secure ? 128 : 28;

    if ((allocation.maximum < c_upper_limit) &&
            (allocation.initial < c_upper_limit))
    {
        hatt.initialReservedCaches = static_cast<uint32_t>(allocation.initial) * 2;
        hatt.maximumReservedCaches = static_cast<uint32_t>(allocation.maximum) * 2;
    }
    else
    {
        hatt.initialReservedCaches = static_cast<uint32_t>(allocation.initial) * 2;
        hatt.maximumReservedCaches = 0;
    }
}

static void set_builtin_writer_history_attributes(
        HistoryAttributes& hatt,
        bool is_secure)
{
    hatt.payloadMaxSize = is_secure ? 128 : 28;
    hatt.initialReservedCaches = 2;
    hatt.maximumReservedCaches = 2;
}

WLP::WLP(
        BuiltinProtocols* p)
    : min_automatic_ms_(std::numeric_limits<double>::max())
    , min_manual_by_participant_ms_(std::numeric_limits<double>::max())
    , mp_participant(nullptr)
    , mp_builtinProtocols(p)
    , mp_builtinWriter(nullptr)
    , mp_builtinReader(nullptr)
    , mp_builtinWriterHistory(nullptr)
    , mp_builtinReaderHistory(nullptr)
    , mp_listener(nullptr)
    , automatic_liveliness_assertion_(nullptr)
    , manual_liveliness_assertion_(nullptr)
    , automatic_writers_()
    , manual_by_participant_writers_()
    , manual_by_topic_writers_()
    , readers_()
    , automatic_readers_(false)
    , pub_liveliness_manager_(nullptr)
    , sub_liveliness_manager_(nullptr)
#if HAVE_SECURITY
    , mp_builtinWriterSecure(nullptr)
    , mp_builtinReaderSecure(nullptr)
    , mp_builtinWriterSecureHistory(nullptr)
    , mp_builtinReaderSecureHistory(nullptr)
#endif // if HAVE_SECURITY
    , temp_reader_proxy_data_(
        p->mp_participantImpl->get_attributes().allocation.locators.max_unicast_locators,
        p->mp_participantImpl->get_attributes().allocation.locators.max_multicast_locators,
        p->mp_participantImpl->get_attributes().allocation.data_limits,
        p->mp_participantImpl->get_attributes().allocation.content_filter)
    , temp_writer_proxy_data_(
        p->mp_participantImpl->get_attributes().allocation.locators.max_unicast_locators,
        p->mp_participantImpl->get_attributes().allocation.locators.max_multicast_locators,
        p->mp_participantImpl->get_attributes().allocation.data_limits)
{
    GUID_t tmp_guid = p->mp_participantImpl->getGuid();
    tmp_guid.entityId = 0;
    automatic_instance_handle_ = tmp_guid;
    manual_by_participant_instance_handle_ = automatic_instance_handle_;

    automatic_instance_handle_.value[15] = dds::AUTOMATIC_LIVELINESS_QOS + 0x01;
    manual_by_participant_instance_handle_.value[15] = dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS + 0x01;
}

WLP::~WLP()
{
    if (automatic_liveliness_assertion_ != nullptr)
    {
        delete automatic_liveliness_assertion_;
        automatic_liveliness_assertion_ = nullptr;
    }
    if (manual_liveliness_assertion_ != nullptr)
    {
        delete this->manual_liveliness_assertion_;
        manual_liveliness_assertion_ = nullptr;
    }

#if HAVE_SECURITY
    if (mp_participant->is_secure())
    {
        mp_participant->deleteUserEndpoint(mp_builtinReaderSecure->getGuid());
        mp_participant->deleteUserEndpoint(mp_builtinWriterSecure->getGuid());

        if (mp_builtinReaderSecureHistory)
        {
            PoolConfig sreader_pool_cfg = PoolConfig::from_history_attributes(mp_builtinReaderSecureHistory->m_att);
            delete mp_builtinReaderSecureHistory;
            secure_payload_pool_->release_history(sreader_pool_cfg, true);
        }

        if (mp_builtinWriterSecureHistory)
        {
            PoolConfig swriter_pool_cfg = PoolConfig::from_history_attributes(mp_builtinWriterSecureHistory->m_att);
            delete mp_builtinWriterSecureHistory;
            secure_payload_pool_->release_history(swriter_pool_cfg, false);
        }
    }
#endif // if HAVE_SECURITY

    mp_participant->deleteUserEndpoint(mp_builtinReader->getGuid());
    mp_participant->deleteUserEndpoint(mp_builtinWriter->getGuid());

    if (mp_builtinReaderHistory)
    {
        PoolConfig reader_pool_cfg = PoolConfig::from_history_attributes(mp_builtinReaderHistory->m_att);
        delete mp_builtinReaderHistory;
        payload_pool_->release_history(reader_pool_cfg, true);
    }

    if (mp_builtinWriterHistory)
    {
        PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(mp_builtinWriterHistory->m_att);
        delete mp_builtinWriterHistory;
        payload_pool_->release_history(writer_pool_cfg, false);
    }

    delete mp_listener;

    delete pub_liveliness_manager_;
    delete sub_liveliness_manager_;
}

bool WLP::initWL(
        RTPSParticipantImpl* p)
{
    EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Initializing Liveliness Protocol");

    mp_participant = p;

    pub_liveliness_manager_ = new LivelinessManager(
        [&](const GUID_t& guid,
        const dds::LivelinessQosPolicyKind& kind,
        const dds::Duration_t& lease_duration,
        int alive_count,
        int not_alive_count) -> void
        {
            pub_liveliness_changed(
                guid,
                kind,
                lease_duration,
                alive_count,
                not_alive_count);
        },
        mp_participant->getEventResource(),
        false);

    sub_liveliness_manager_ = new LivelinessManager(
        [&](const GUID_t& guid,
        const dds::LivelinessQosPolicyKind& kind,
        const dds::Duration_t& lease_duration,
        int alive_count,
        int not_alive_count) -> void
        {
            sub_liveliness_changed(
                guid,
                kind,
                lease_duration,
                alive_count,
                not_alive_count);
        },
        mp_participant->getEventResource());

    bool retVal = createEndpoints();
#if HAVE_SECURITY
    if (retVal && p->is_secure())
    {
        retVal = createSecureEndpoints();
    }
#endif // if HAVE_SECURITY
    return retVal;
}

bool WLP::createEndpoints()
{
    const RTPSParticipantAttributes& pattr = mp_participant->get_attributes();
    const ResourceLimitedContainerConfig& participants_allocation = pattr.allocation.participants;

    // Built-in writer history
    HistoryAttributes hatt;
    set_builtin_writer_history_attributes(hatt, false);
    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    payload_pool_ = TopicPayloadPoolRegistry::get("DCPSParticipantMessage", writer_pool_cfg);
    payload_pool_->reserve_history(writer_pool_cfg, false);
    mp_builtinWriterHistory = new WriterHistory(hatt, payload_pool_);

    // Built-in writer
    WriterAttributes watt = mp_participant->pdp()->create_builtin_writer_attributes();
    watt.endpoint.remoteLocatorList = mp_builtinProtocols->m_initialPeersList;

    RTPSWriter* wout;
    if (mp_participant->createWriter(
                &wout,
                watt,
                mp_builtinWriterHistory,
                nullptr,
                c_EntityId_WriterLiveliness,
                true))
    {
        mp_builtinWriter = dynamic_cast<StatefulWriter*>(wout);
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Builtin Liveliness Writer created");
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Liveliness Writer Creation failed ");
        delete(mp_builtinWriterHistory);
        mp_builtinWriterHistory = nullptr;
        payload_pool_->release_history(writer_pool_cfg, false);
        return false;
    }

    // Built-in reader history
    set_builtin_reader_history_attributes(hatt, participants_allocation, false);
    mp_builtinReaderHistory = new ReaderHistory(hatt);

    PoolConfig reader_pool_cfg = PoolConfig::from_history_attributes(hatt);
    payload_pool_->reserve_history(reader_pool_cfg, true);

    // WLP listener

    mp_listener = new WLPListener(this);

    // Built-in reader

    ReaderAttributes ratt = mp_participant->pdp()->create_builtin_reader_attributes();
    ratt.endpoint.remoteLocatorList = mp_builtinProtocols->m_initialPeersList;
    ratt.expects_inline_qos = true;

    RTPSReader* rout;
    if (mp_participant->createReader(
                &rout,
                ratt,
                payload_pool_,
                mp_builtinReaderHistory,
                (ReaderListener*)mp_listener,
                c_EntityId_ReaderLiveliness,
                true))
    {
        mp_builtinReader = dynamic_cast<StatefulReader*>(rout);
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Builtin Liveliness Reader created");
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Liveliness Reader Creation failed.");
        delete(mp_builtinReaderHistory);
        mp_builtinReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        payload_pool_->release_history(reader_pool_cfg, true);
        return false;
    }

    return true;
}

#if HAVE_SECURITY

bool WLP::createSecureEndpoints()
{
    const RTPSParticipantAttributes& pattr = mp_participant->get_attributes();
    const ResourceLimitedContainerConfig& participants_allocation = pattr.allocation.participants;

    //CREATE WRITER
    HistoryAttributes hatt;
    set_builtin_writer_history_attributes(hatt, true);
    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    secure_payload_pool_ = TopicPayloadPoolRegistry::get("DCPSParticipantMessageSecure", writer_pool_cfg);
    secure_payload_pool_->reserve_history(writer_pool_cfg, false);
    mp_builtinWriterSecureHistory = new WriterHistory(hatt, secure_payload_pool_);

    WriterAttributes watt = mp_participant->pdp()->create_builtin_writer_attributes();
    //	Wparam.topic.topicName = "DCPSParticipantMessageSecure";
    //	Wparam.topic.topicDataType = "RTPSParticipantMessageData";

    const security::ParticipantSecurityAttributes& part_attrs = mp_participant->security_attributes();
    security::PluginParticipantSecurityAttributes plugin_attrs(part_attrs.plugin_participant_attributes);
    security::EndpointSecurityAttributes* sec_attrs = &watt.endpoint.security_attributes();
    sec_attrs->is_submessage_protected = part_attrs.is_liveliness_protected;
    if (part_attrs.is_liveliness_protected)
    {
        sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (plugin_attrs.is_liveliness_encrypted)
        {
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        }
        if (plugin_attrs.is_liveliness_origin_authenticated)
        {
            sec_attrs->plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }
    }

    RTPSWriter* wout;
    if (mp_participant->createWriter(&wout, watt, mp_builtinWriterSecureHistory, nullptr,
            c_EntityId_WriterLivelinessSecure, true))
    {
        mp_builtinWriterSecure = dynamic_cast<StatefulWriter*>(wout);
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Builtin Secure Liveliness Writer created");
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Secure Liveliness Writer Creation failed ");
        delete(mp_builtinWriterSecureHistory);
        mp_builtinWriterSecureHistory = nullptr;
        secure_payload_pool_->release_history(writer_pool_cfg, false);
        return false;
    }

    set_builtin_reader_history_attributes(hatt, participants_allocation, true);

    PoolConfig reader_pool_cfg = PoolConfig::from_history_attributes(hatt);
    secure_payload_pool_->reserve_history(reader_pool_cfg, true);

    mp_builtinReaderSecureHistory = new ReaderHistory(hatt);
    ReaderAttributes ratt = mp_participant->pdp()->create_builtin_reader_attributes();
    ratt.expects_inline_qos = true;
    //Rparam.topic.topicName = "DCPSParticipantMessageSecure";
    //Rparam.topic.topicDataType = "RTPSParticipantMessageData";

    sec_attrs = &ratt.endpoint.security_attributes();
    sec_attrs->is_submessage_protected = part_attrs.is_liveliness_protected;
    if (part_attrs.is_liveliness_protected)
    {
        sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;
        if (plugin_attrs.is_liveliness_encrypted)
        {
            sec_attrs->plugin_endpoint_attributes |= PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        }
        if (plugin_attrs.is_liveliness_origin_authenticated)
        {
            sec_attrs->plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }
    }
    RTPSReader* rout;
    if (mp_participant->createReader(
                &rout,
                ratt, secure_payload_pool_,
                mp_builtinReaderSecureHistory,
                (ReaderListener*)mp_listener,
                c_EntityId_ReaderLivelinessSecure,
                true))
    {
        mp_builtinReaderSecure = dynamic_cast<StatefulReader*>(rout);
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Builtin Liveliness Reader created");
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_LIVELINESS, "Liveliness Reader Creation failed.");
        delete(mp_builtinReaderSecureHistory);
        mp_builtinReaderSecureHistory = nullptr;
        secure_payload_pool_->release_history(reader_pool_cfg, true);
        return false;
    }

    return true;
}

bool WLP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    if (local_writer.entityId == c_EntityId_WriterLivelinessSecure)
    {
        mp_builtinWriterSecure->matched_reader_add_edp(remote_reader_data);
        return true;
    }

    return false;
}

bool WLP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    if (local_reader.entityId == c_EntityId_ReaderLivelinessSecure)
    {
        mp_builtinReaderSecure->matched_writer_add_edp(remote_writer_data);
        return true;
    }

    return false;
}

#endif // if HAVE_SECURITY

bool WLP::assignRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool assign_secure_endpoints)
{
    const NetworkFactory& network = mp_participant->network_factory();
    uint32_t endp = pdata.m_available_builtin_endpoints;
    uint32_t auxendp = endp;
    bool use_multicast_locators = !mp_participant->get_attributes().builtin.avoid_builtin_multicast ||
            pdata.metatraffic_locators.unicast.empty();

    std::lock_guard<std::mutex> data_guard(temp_data_lock_);

    temp_writer_proxy_data_.guid.guidPrefix = pdata.guid.guidPrefix;
    temp_writer_proxy_data_.persistence_guid = pdata.get_persistence_guid();
    temp_writer_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
            pdata.is_from_this_host());
    temp_writer_proxy_data_.topic_kind = WITH_KEY;
    temp_writer_proxy_data_.durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_writer_proxy_data_.reliability.kind = dds::RELIABLE_RELIABILITY_QOS;

    temp_reader_proxy_data_.clear();
    temp_reader_proxy_data_.expects_inline_qos = false;
    temp_reader_proxy_data_.guid.guidPrefix = pdata.guid.guidPrefix;
    temp_reader_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
            pdata.is_from_this_host());
    temp_reader_proxy_data_.topic_kind = WITH_KEY;
    temp_reader_proxy_data_.durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_proxy_data_.reliability.kind = dds::RELIABLE_RELIABILITY_QOS;

    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
    if (auxendp != 0 && this->mp_builtinReader != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Adding remote writer to my local Builtin Reader");
        temp_writer_proxy_data_.guid.entityId = c_EntityId_WriterLiveliness;
        temp_writer_proxy_data_.set_persistence_entity_id(c_EntityId_WriterLiveliness);
        mp_builtinReader->matched_writer_add_edp(temp_writer_proxy_data_);
    }
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    if (auxendp != 0 && this->mp_builtinWriter != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Adding remote reader to my local Builtin Writer");
        temp_reader_proxy_data_.guid.entityId = c_EntityId_ReaderLiveliness;
        mp_builtinWriter->matched_reader_add_edp(temp_reader_proxy_data_);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
    if (auxendp != 0 && this->mp_builtinReaderSecure != nullptr && assign_secure_endpoints)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Adding remote writer to my local Builtin Secure Reader");
        temp_writer_proxy_data_.guid.entityId = c_EntityId_WriterLivelinessSecure;
        temp_writer_proxy_data_.set_persistence_entity_id(c_EntityId_WriterLivelinessSecure);

        if (!mp_participant->security_manager().discovered_builtin_writer(
                    mp_builtinReaderSecure->getGuid(), pdata.guid, temp_writer_proxy_data_,
                    mp_builtinReaderSecure->getAttributes().security_attributes()))
        {
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for reader " <<
                    mp_builtinReaderSecure->getGuid());
        }
    }
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
    if (auxendp != 0 && this->mp_builtinWriterSecure != nullptr && assign_secure_endpoints)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Adding remote reader to my local Builtin Secure Writer");
        temp_reader_proxy_data_.guid.entityId = c_EntityId_ReaderLivelinessSecure;
        if (!mp_participant->security_manager().discovered_builtin_reader(
                    mp_builtinWriterSecure->getGuid(), pdata.guid, temp_reader_proxy_data_,
                    mp_builtinWriterSecure->getAttributes().security_attributes()))
        {
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for writer " <<
                    mp_builtinWriterSecure->getGuid());
        }
    }
#else
    static_cast<void>(assign_secure_endpoints);
#endif // if HAVE_SECURITY

    return true;
}

void WLP::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->guid.guidPrefix;

    EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "for RTPSParticipant: " << pdata->guid);
    uint32_t endp = pdata->m_available_builtin_endpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && this->mp_builtinReader != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Removing remote writer from my local Builtin Reader");
        tmp_guid.entityId = c_EntityId_WriterLiveliness;
        mp_builtinReader->matched_writer_remove(tmp_guid);
    }
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinWriter != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Removing remote reader from my local Builtin Writer");
        tmp_guid.entityId = c_EntityId_ReaderLiveliness;
        mp_builtinWriter->matched_reader_remove(tmp_guid);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinReaderSecure != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Removing remote writer from my local Builtin Secure Reader");
        tmp_guid.entityId = c_EntityId_WriterLivelinessSecure;
        if (mp_builtinReaderSecure->matched_writer_remove(tmp_guid))
        {
            mp_participant->security_manager().remove_writer(
                mp_builtinReaderSecure->getGuid(), pdata->guid, tmp_guid);
        }
    }
    auxendp = endp;
    auxendp &= fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
    if ((auxendp != 0 || partdet != 0) && this->mp_builtinWriterSecure != nullptr)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Removing remote reader from my local Builtin Secure Writer");
        tmp_guid.entityId = c_EntityId_ReaderLivelinessSecure;
        if (mp_builtinWriterSecure->matched_reader_remove(tmp_guid))
        {
            mp_participant->security_manager().remove_reader(
                mp_builtinWriterSecure->getGuid(), pdata->guid, tmp_guid);
        }
    }
#endif // if HAVE_SECURITY
}

bool WLP::add_local_writer(
        RTPSWriter* writer,
        const fastdds::dds::LivelinessQosPolicy& qos)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());

    EPROSIMA_LOG_INFO(RTPS_LIVELINESS, writer->getGuid().entityId << " to Liveliness Protocol");

    BaseWriter* base_writer = BaseWriter::downcast(writer);

    double wAnnouncementPeriodMilliSec(TimeConv::Duration_t2MilliSecondsDouble(qos.announcement_period));

    if (qos.kind == dds::AUTOMATIC_LIVELINESS_QOS )
    {
        if (automatic_liveliness_assertion_ == nullptr)
        {
            automatic_liveliness_assertion_ = new TimedEvent(mp_participant->getEventResource(),
                            [&]() -> bool
                            {
                                automatic_liveliness_assertion();
                                return true;
                            },
                            wAnnouncementPeriodMilliSec);
            automatic_liveliness_assertion_->restart_timer();
            min_automatic_ms_ = wAnnouncementPeriodMilliSec;
        }
        else if (min_automatic_ms_ > wAnnouncementPeriodMilliSec)
        {
            min_automatic_ms_ = wAnnouncementPeriodMilliSec;
            automatic_liveliness_assertion_->update_interval_millisec(wAnnouncementPeriodMilliSec);
            //CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
            if (automatic_liveliness_assertion_->getRemainingTimeMilliSec() > min_automatic_ms_)
            {
                automatic_liveliness_assertion_->cancel_timer();
            }
            automatic_liveliness_assertion_->restart_timer();
        }
        automatic_writers_.push_back(base_writer);
    }
    else if (qos.kind == dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (manual_liveliness_assertion_ == nullptr)
        {
            manual_liveliness_assertion_ = new TimedEvent(mp_participant->getEventResource(),
                            [&]() -> bool
                            {
                                participant_liveliness_assertion();
                                return true;
                            },
                            wAnnouncementPeriodMilliSec);
            manual_liveliness_assertion_->restart_timer();
            min_manual_by_participant_ms_ = wAnnouncementPeriodMilliSec;
        }
        else if (min_manual_by_participant_ms_ > wAnnouncementPeriodMilliSec)
        {
            min_manual_by_participant_ms_ = wAnnouncementPeriodMilliSec;
            manual_liveliness_assertion_->update_interval_millisec(min_manual_by_participant_ms_);
            //CHECK IF THE TIMER IS GOING TO BE CALLED AFTER THIS NEW SET LEASE DURATION
            if (manual_liveliness_assertion_->getRemainingTimeMilliSec() > min_manual_by_participant_ms_)
            {
                manual_liveliness_assertion_->cancel_timer();
            }
            manual_liveliness_assertion_->restart_timer();
        }
        manual_by_participant_writers_.push_back(base_writer);

        if (!pub_liveliness_manager_->add_writer(
                    writer->getGuid(),
                    qos.kind,
                    qos.lease_duration))
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Could not add writer " << writer->getGuid() << " to liveliness manager");
        }
    }
    else if (qos.kind == dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        manual_by_topic_writers_.push_back(base_writer);

        if (!pub_liveliness_manager_->add_writer(
                    writer->getGuid(),
                    qos.kind,
                    qos.lease_duration))
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Could not add writer " << writer->getGuid() << " to liveliness manager");
        }
    }

    return true;
}

bool WLP::remove_local_writer(
        RTPSWriter* writer)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());

    EPROSIMA_LOG_INFO(RTPS_LIVELINESS, writer->getGuid().entityId << " from Liveliness Protocol");

    LivelinessData::WriterStatus writer_status;
    BaseWriter* base_writer = BaseWriter::downcast(writer);

    if (base_writer->get_liveliness_kind() == dds::AUTOMATIC_LIVELINESS_QOS)
    {
        auto it = std::find(
            automatic_writers_.begin(),
            automatic_writers_.end(),
            base_writer);

        if (it == automatic_writers_.end())
        {
            EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Writer " << base_writer->getGuid() << " not found.");
            return false;
        }

        automatic_writers_.erase(it);

        min_automatic_ms_ = std::numeric_limits<double>::max();
        if (automatic_writers_.size() == 0)
        {
            automatic_liveliness_assertion_->cancel_timer();
            return true;
        }

        // There are still some writers. Calculate the new minimum announcement period
        for (const auto& w : automatic_writers_)
        {
            auto announcement_period = fastdds::rtps::TimeConv::Duration_t2MilliSecondsDouble(
                w->get_liveliness_announcement_period());
            if (min_automatic_ms_ > announcement_period)
            {
                min_automatic_ms_ = announcement_period;
            }
        }
        automatic_liveliness_assertion_->update_interval_millisec(min_automatic_ms_);
        return true;
    }
    else if (base_writer->get_liveliness_kind() == dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        auto it = std::find(
            manual_by_participant_writers_.begin(),
            manual_by_participant_writers_.end(),
            base_writer);

        if (it == manual_by_participant_writers_.end())
        {
            EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Writer " << base_writer->getGuid() << " not found.");
            return false;
        }

        manual_by_participant_writers_.erase(it);

        if (!pub_liveliness_manager_->remove_writer(
                    base_writer->getGuid(),
                    base_writer->get_liveliness_kind(),
                    base_writer->get_liveliness_lease_duration(),
                    writer_status))
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Could not remove writer " << base_writer->getGuid() << " from liveliness manager");
        }

        min_manual_by_participant_ms_ = std::numeric_limits<double>::max();
        if (manual_by_participant_writers_.size() == 0)
        {
            manual_liveliness_assertion_->cancel_timer();
            return true;
        }

        // There are still some writers. Calculate the new minimum announcement period
        for (const auto& w : manual_by_participant_writers_)
        {
            auto announcement_period = fastdds::rtps::TimeConv::Duration_t2MilliSecondsDouble(
                w->get_liveliness_announcement_period());
            if (min_manual_by_participant_ms_ > announcement_period)
            {
                min_manual_by_participant_ms_ = announcement_period;
            }
        }
        manual_liveliness_assertion_->update_interval_millisec(min_manual_by_participant_ms_);
        return true;
    }
    else if (base_writer->get_liveliness_kind() == dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        auto it = std::find(
            manual_by_topic_writers_.begin(),
            manual_by_topic_writers_.end(),
            base_writer);

        if (it == manual_by_topic_writers_.end())
        {
            EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Writer " << base_writer->getGuid() << " not found.");
            return false;
        }

        manual_by_topic_writers_.erase(it);

        if (!pub_liveliness_manager_->remove_writer(
                    base_writer->getGuid(),
                    base_writer->get_liveliness_kind(),
                    base_writer->get_liveliness_lease_duration(),
                    writer_status))
        {
            EPROSIMA_LOG_ERROR(RTPS_LIVELINESS,
                    "Could not remove writer " << base_writer->getGuid() << " from liveliness manager");
        }
        return true;
    }

    EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Writer " << base_writer->getGuid() << " not found.");
    return false;
}

bool WLP::add_local_reader(
        RTPSReader* reader,
        const fastdds::dds::LivelinessQosPolicy& qos)
{
    auto base_reader = BaseReader::downcast(reader);

    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());

    if (qos.kind == dds::AUTOMATIC_LIVELINESS_QOS)
    {
        automatic_readers_ = true;
    }

    readers_.push_back(base_reader);

    return true;
}

bool WLP::remove_local_reader(
        RTPSReader* reader)
{
    auto base_reader = BaseReader::downcast(reader);
    std::lock_guard<std::recursive_mutex> guard(*mp_builtinProtocols->mp_PDP->getMutex());

    auto it = std::find(
        readers_.begin(),
        readers_.end(),
        base_reader);
    if (it != readers_.end())
    {
        readers_.erase(it);
        return true;
    }

    EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Reader not removed from WLP, unknown reader");
    return false;
}

bool WLP::automatic_liveliness_assertion()
{
    std::unique_lock<std::recursive_mutex> lock(*mp_builtinProtocols->mp_PDP->getMutex());

    if (0 < automatic_writers_.size())
    {
        lock.unlock();
        return send_liveliness_message(automatic_instance_handle_);
    }

    return true;
}

bool WLP::participant_liveliness_assertion()
{
    std::unique_lock<std::recursive_mutex> lock(*mp_builtinProtocols->mp_PDP->getMutex());

    if (0 < manual_by_participant_writers_.size())
    {
        if (pub_liveliness_manager_->is_any_alive(dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS))
        {
            lock.unlock();
            return send_liveliness_message(manual_by_participant_instance_handle_);
        }
    }

    return false;
}

bool WLP::send_liveliness_message(
        const InstanceHandle_t& instance)
{
    StatefulWriter* writer = builtin_writer();
    WriterHistory* history = builtin_writer_history();
    std::shared_ptr<IPayloadPool> pool = builtin_writer_pool();

    std::lock_guard<RecursiveTimedMutex> wguard(writer->getMutex());

    CacheChange_t* change = history->create_change(WLP::builtin_participant_data_max_size, ALIVE, instance);
    if (change != nullptr)
    {
        change->serializedPayload.encapsulation = (uint16_t)DEFAULT_ENCAPSULATION;
        change->serializedPayload.data[0] = 0;
        change->serializedPayload.data[1] = DEFAULT_ENCAPSULATION;
        change->serializedPayload.data[2] = 0;
        change->serializedPayload.data[3] = 0;

        memcpy(change->serializedPayload.data + 4, instance.value, 16);

        for (size_t i = 20; i < 28; ++i)
        {
            change->serializedPayload.data[i] = 0;
        }
        change->serializedPayload.length = 4 + 12 + 4 + 4 + 4;

        if (history->getHistorySize() > 0)
        {
            for (auto chit = history->changesBegin(); chit != history->changesEnd(); ++chit)
            {
                if ((*chit)->instanceHandle == change->instanceHandle)
                {
                    history->remove_change(*chit);
                    break;
                }
            }
        }
        history->add_change(change);
        return true;
    }
    return false;
}

StatefulWriter* WLP::builtin_writer()
{
    StatefulWriter* ret_val = mp_builtinWriter;

#if HAVE_SECURITY
    if (mp_participant->security_attributes().is_liveliness_protected)
    {
        ret_val = mp_builtinWriterSecure;
    }
#endif // if HAVE_SECURITY

    return ret_val;
}

WriterHistory* WLP::builtin_writer_history()
{
    WriterHistory* ret_val = mp_builtinWriterHistory;

#if HAVE_SECURITY
    if (mp_participant->security_attributes().is_liveliness_protected)
    {
        ret_val = mp_builtinWriterSecureHistory;
    }
#endif // if HAVE_SECURITY

    return ret_val;
}

std::shared_ptr<IPayloadPool> WLP::builtin_writer_pool()
{
#if HAVE_SECURITY
    if (mp_participant->security_attributes().is_liveliness_protected)
    {
        return secure_payload_pool_;
    }
#endif // if HAVE_SECURITY

    return payload_pool_;
}

bool WLP::assert_liveliness(
        GUID_t writer,
        dds::LivelinessQosPolicyKind kind,
        dds::Duration_t lease_duration)
{
    return pub_liveliness_manager_->assert_liveliness(
        writer,
        kind,
        lease_duration);
}

bool WLP::assert_liveliness_manual_by_participant()
{
    if (manual_by_participant_writers_.size() > 0)
    {
        return pub_liveliness_manager_->assert_liveliness(
            dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            mp_participant->getGuid().guidPrefix);
    }
    return false;
}

void WLP::pub_liveliness_changed(
        const GUID_t& writer,
        const dds::LivelinessQosPolicyKind& kind,
        const dds::Duration_t& lease_duration,
        int32_t alive_change,
        int32_t not_alive_change)
{
    (void)lease_duration;
    (void)alive_change;

    // On the publishing side we only have to notify if one of our writers loses liveliness
    if (not_alive_change != 1)
    {
        return;
    }

    if (kind == dds::AUTOMATIC_LIVELINESS_QOS)
    {
        for (BaseWriter* w: automatic_writers_)
        {
            if (w->getGuid() == writer)
            {
                w->liveliness_lost();
                return;
            }
        }
    }
    else if (kind == dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        for (BaseWriter* w: manual_by_participant_writers_)
        {
            if (w->getGuid() == writer)
            {
                w->liveliness_lost();
                return;
            }
        }
    }
    else if (kind == dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        for (BaseWriter* w: manual_by_topic_writers_)
        {
            if (w->getGuid() == writer)
            {
                w->liveliness_lost();
                return;
            }
        }
    }
}

void WLP::sub_liveliness_changed(
        const GUID_t& writer,
        const dds::LivelinessQosPolicyKind& kind,
        const dds::Duration_t& lease_duration,
        int32_t alive_change,
        int32_t not_alive_change)
{
    // Writer with given guid lost liveliness, check which readers were matched and inform them

    for (BaseReader* reader : readers_)
    {
        if (reader->liveliness_kind() == kind &&
                reader->liveliness_lease_duration() == lease_duration)
        {
            if (reader->matched_writer_is_matched(writer))
            {
                update_liveliness_changed_status(
                    writer,
                    reader,
                    alive_change,
                    not_alive_change);
            }
        }
    }
}

void WLP::update_liveliness_changed_status(
        GUID_t writer,
        BaseReader* reader,
        int32_t alive_change,
        int32_t not_alive_change)
{
    reader->update_liveliness_changed_status(writer, alive_change, not_alive_change);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
