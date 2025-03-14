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
 * @file EDP.cpp
 *
 */

#include <rtps/builtin/discovery/endpoint/EDP.h>

#include <cassert>
#include <mutex>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <fastdds/utils/TypePropagation.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ProxyHashTables.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <utils/BuiltinTopicKeyConversions.hpp>
#if HAVE_SECURITY
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY
#include <rtps/writer/BaseWriter.hpp>
#include <utils/collections/node_size_helpers.hpp>
#include <utils/StringMatching.hpp>
#ifdef FASTDDS_STATISTICS
#include <statistics/rtps/monitor-service/interfaces/IProxyObserver.hpp>
#endif //FASTDDS_STATISTICS

using eprosima::fastdds::dds::PublicationMatchedStatus;
using eprosima::fastdds::dds::SubscriptionMatchedStatus;
using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

using reader_map_helper = utilities::collections::map_size_helper<GUID_t, SubscriptionMatchedStatus>;
using writer_map_helper = utilities::collections::map_size_helper<GUID_t, PublicationMatchedStatus>;

static bool is_partition_empty(
        const fastdds::dds::Partition_t& partition)
{
    return partition.size() <= 1 && 0 == strlen(partition.name());
}

static bool is_same_type(
        const dds::xtypes::TypeInformation& t1,
        const dds::xtypes::TypeInformation& t2)
{
    return (dds::xtypes::TK_NONE != t1.complete().typeid_with_size().type_id()._d()
           && t1.complete().typeid_with_size() == t2.complete().typeid_with_size())
           || (dds::xtypes::TK_NONE != t1.minimal().typeid_with_size().type_id()._d()
           && t1.minimal().typeid_with_size() == t2.minimal().typeid_with_size());
}

EDP::EDP(
        PDP* p,
        RTPSParticipantImpl* part)
    : mp_PDP(p)
    , mp_RTPSParticipant(part)
{
}

EDP::~EDP()
{
    // TODO Auto-generated destructor stub
}

bool EDP::new_reader_proxy_data(
        RTPSReader* rtps_reader,
        const TopicDescription& topic,
        const fastdds::dds::ReaderQos& qos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    EPROSIMA_LOG_INFO(RTPS_EDP,
            "Adding " << rtps_reader->getGuid().entityId << " in topic " <<
            topic.topic_name.to_string());

    auto init_fun = [this, rtps_reader, &topic, &qos, content_filter](
        ReaderProxyData* rpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP,
                            "Adding already existent reader " << rtps_reader->getGuid().entityId << " in topic "
                                                              << topic.topic_name.to_string());
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                const auto& ratt = rtps_reader->getAttributes();

                rpd->is_alive(true);
                rpd->expects_inline_qos = rtps_reader->expects_inline_qos();
                rpd->guid = rtps_reader->getGuid();
                rpd->participant_guid = participant_data.guid;
                rpd->key() = rpd->guid;
                if (ratt.multicastLocatorList.empty() && ratt.unicastLocatorList.empty())
                {
                    rpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    rpd->set_multicast_locators(ratt.multicastLocatorList, network,
                            participant_data.is_from_this_host());
                    rpd->set_announced_unicast_locators(ratt.unicastLocatorList);
                    fastdds::rtps::network::external_locators::add_external_locators(*rpd,
                            ratt.external_unicast_locators);
                }
                rpd->rtps_participant_key() = mp_RTPSParticipant->getGuid();
                rpd->topic_name = topic.topic_name;
                rpd->type_name = topic.type_name;
                rpd->topic_kind = (rpd->guid.entityId.value[3] & 0x0F) == 0x07 ? WITH_KEY : NO_KEY;

                using dds::utils::TypePropagation;
                using dds::xtypes::TypeInformationParameter;

                auto type_propagation = mp_RTPSParticipant->type_propagation();
                assert(TypePropagation::TYPEPROPAGATION_UNKNOWN != type_propagation);

                const auto& type_info = topic.type_information;
                if (type_info.assigned())
                {
                    switch (type_propagation)
                    {
                        case TypePropagation::TYPEPROPAGATION_ENABLED:
                        {
                            rpd->type_information = type_info;
                            break;
                        }
                        case TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH:
                        {
                            TypeInformationParameter minimal;
                            minimal.type_information.minimal(type_info.type_information.minimal());
                            minimal.assigned(true);
                            rpd->type_information = minimal;
                            break;
                        }
                        case TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY:
                        default:
                        {
                            if (rpd->has_type_information())
                            {
                                rpd->type_information.assigned(false);
                            }
                            break;
                        }
                    }
                }

                rpd->set_qos(qos, true);
                rpd->user_defined_id(ratt.getUserDefinedID());
                if (nullptr != content_filter)
                {
                    // Check content of ContentFilterProperty.
                    if (!(0 < content_filter->content_filtered_topic_name.size() &&
                            0 < content_filter->related_topic_name.size() &&
                            0 < content_filter->filter_class_name.size() &&
                            0 < content_filter->filter_expression.size()
                            ))
                    {
                        return false;
                    }

                    rpd->content_filter = *content_filter;
                }

#if HAVE_SECURITY
                if (mp_RTPSParticipant->is_secure())
                {
                    rpd->security_attributes_ = ratt.security_attributes().mask();
                    rpd->plugin_security_attributes_ = ratt.security_attributes().plugin_endpoint_attributes;
                }
                else
                {
                    rpd->security_attributes_ = 0UL;
                    rpd->plugin_security_attributes_ = 0UL;
                }
#endif // if HAVE_SECURITY

                return true;
            };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(
        rtps_reader->getGuid(), participant_guid, init_fun);
    if (reader_data == nullptr)
    {
        return false;
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service about the new local entity proxy
    if (nullptr != this->mp_PDP->get_proxy_observer())
    {
        this->mp_PDP->get_proxy_observer()->on_local_entity_change(reader_data->guid, true);
    }
#endif //FASTDDS_STATISTICS

    //PAIRING
    if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
    }
    pairingReader(rtps_reader, participant_guid, *reader_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    process_reader_proxy_data(rtps_reader, reader_data);
    return true;
}

bool EDP::new_writer_proxy_data(
        RTPSWriter* rtps_writer,
        const TopicDescription& topic,
        const fastdds::dds::WriterQos& qos)
{
    EPROSIMA_LOG_INFO(RTPS_EDP,
            "Adding " << rtps_writer->getGuid().entityId << " in topic " <<
            topic.topic_name.to_string());

    auto init_fun = [this, rtps_writer, &topic, &qos](
        WriterProxyData* wpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP,
                            "Adding already existent writer " << rtps_writer->getGuid().entityId << " in topic "
                                                              << topic.topic_name.to_string());
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                const auto& watt = rtps_writer->getAttributes();

                wpd->guid = rtps_writer->getGuid();
                wpd->participant_guid = participant_data.guid;
                wpd->key() = wpd->guid;
                if (watt.multicastLocatorList.empty() && watt.unicastLocatorList.empty())
                {
                    wpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    wpd->set_multicast_locators(watt.multicastLocatorList, network,
                            participant_data.is_from_this_host());
                    wpd->set_announced_unicast_locators(watt.unicastLocatorList);
                    fastdds::rtps::network::external_locators::add_external_locators(*wpd,
                            watt.external_unicast_locators);
                }
                wpd->rtps_participant_key() = mp_RTPSParticipant->getGuid();
                from_guid_prefix_to_topic_key(mp_RTPSParticipant->getGuid().guidPrefix, wpd->participant_key.value);
                from_entity_id_to_topic_key(wpd->guid.entityId, wpd->PublicationBuiltinTopicData::key.value);
                wpd->topic_name = topic.topic_name;
                wpd->type_name = topic.type_name;
                wpd->topic_kind = (wpd->guid.entityId.value[3] & 0x0F) == 0x02 ? WITH_KEY : NO_KEY;

                using dds::utils::TypePropagation;
                using dds::xtypes::TypeInformationParameter;

                auto type_propagation = mp_RTPSParticipant->type_propagation();
                assert(TypePropagation::TYPEPROPAGATION_UNKNOWN != type_propagation);

                const auto& type_info = topic.type_information;
                if (type_info.assigned())
                {
                    switch (type_propagation)
                    {
                        case TypePropagation::TYPEPROPAGATION_ENABLED:
                        {
                            wpd->type_information = type_info;
                            break;
                        }
                        case TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH:
                        {
                            TypeInformationParameter minimal;
                            minimal.type_information.minimal(type_info.type_information.minimal());
                            minimal.assigned(true);
                            wpd->type_information = minimal;
                            break;
                        }
                        case TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY:
                        default:
                        {
                            if (wpd->has_type_information())
                            {
                                wpd->type_information.assigned(false);
                            }
                            break;
                        }
                    }
                }

                BaseWriter* base_writer = BaseWriter::downcast(rtps_writer);
                assert(base_writer->get_history() != nullptr);
                wpd->type_max_serialized(base_writer->get_history()->getTypeMaxSerialized());
                wpd->set_qos(qos, true);
                wpd->user_defined_id(watt.getUserDefinedID());
                wpd->persistence_guid = watt.persistence_guid;
#if HAVE_SECURITY
                if (mp_RTPSParticipant->is_secure())
                {
                    wpd->security_attributes_ = watt.security_attributes().mask();
                    wpd->plugin_security_attributes_ = watt.security_attributes().plugin_endpoint_attributes;
                }
                else
                {
                    wpd->security_attributes_ = 0UL;
                    wpd->plugin_security_attributes_ = 0UL;
                }
#endif // if HAVE_SECURITY

                return true;
            };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(rtps_writer->getGuid(), participant_guid, init_fun);
    if (writer_data == nullptr)
    {
        return false;
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service about the new local entity proxy
    if (nullptr != this->mp_PDP->get_proxy_observer())
    {
        this->mp_PDP->get_proxy_observer()->on_local_entity_change(writer_data->guid, true);
    }
#endif //FASTDDS_STATISTICS

    //PAIRING
    if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
    }
    pairingWriter(rtps_writer, participant_guid, *writer_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    process_writer_proxy_data(rtps_writer, writer_data);
    return true;
}

bool EDP::update_reader(
        RTPSReader* rtps_reader,
        const fastdds::dds::ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    auto init_fun = [this, rtps_reader, &rqos, content_filter](
        ReaderProxyData* rdata,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                // Should only be called for existent data
                (void)updating;
                assert(updating);

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                if (rtps_reader->getAttributes().multicastLocatorList.empty() &&
                        rtps_reader->getAttributes().unicastLocatorList.empty())
                {
                    rdata->set_locators(participant_data.default_locators);
                }
                else
                {
                    rdata->set_multicast_locators(
                        rtps_reader->getAttributes().multicastLocatorList, network,
                        participant_data.is_from_this_host());
                    rdata->set_announced_unicast_locators(rtps_reader->getAttributes().unicastLocatorList);
                }
                rdata->set_qos(rqos, false);
                if (nullptr != content_filter)
                {
                    // Check content of ContentFilterProperty.
                    if (!(0 < content_filter->content_filtered_topic_name.size() &&
                            0 < content_filter->related_topic_name.size() &&
                            0 < content_filter->filter_class_name.size() &&
                            0 < content_filter->filter_expression.size()
                            ))
                    {
                        return false;
                    }

                    rdata->content_filter = *content_filter;
                }
                else
                {
                    rdata->content_filter.filter_class_name = "";
                    rdata->content_filter.filter_expression = "";
                }
                rdata->is_alive(true);
                rdata->expects_inline_qos = rtps_reader->expects_inline_qos();

                return true;
            };

    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(rtps_reader->getGuid(), participant_guid, init_fun);
    if (reader_data != nullptr)
    {
        process_reader_proxy_data(rtps_reader, reader_data);
#ifdef FASTDDS_STATISTICS
        // notify monitor service about the new local entity proxy update
        if (nullptr != this->mp_PDP->get_proxy_observer())
        {
            this->mp_PDP->get_proxy_observer()->on_local_entity_change(reader_data->guid, true);
        }
#endif //FASTDDS_STATISTICS
        if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
        {
            pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
        }
        pairingReader(rtps_reader, participant_guid, *reader_data);
        return true;
    }
    return false;
}

bool EDP::update_writer(
        RTPSWriter* writer,
        const fastdds::dds::WriterQos& wqos)
{
    auto init_fun = [this, writer, &wqos](
        WriterProxyData* wdata,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                // Should only be called for existent data
                (void)updating;
                assert(updating);

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                if (writer->getAttributes().multicastLocatorList.empty() &&
                        writer->getAttributes().unicastLocatorList.empty())
                {
                    wdata->set_locators(participant_data.default_locators);
                }
                else
                {
                    wdata->set_multicast_locators(
                        writer->getAttributes().multicastLocatorList, network,
                        participant_data.is_from_this_host());
                    wdata->set_announced_unicast_locators(writer->getAttributes().unicastLocatorList);
                }
                wdata->set_qos(wqos, false);

                return true;
            };

    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer->getGuid(), participant_guid, init_fun);
    if (writer_data != nullptr)
    {
        process_writer_proxy_data(writer, writer_data);

#ifdef FASTDDS_STATISTICS
        // notify monitor service about the new local entity proxy update
        if (nullptr != this->mp_PDP->get_proxy_observer())
        {
            this->mp_PDP->get_proxy_observer()->on_local_entity_change(writer_data->guid, true);
        }
#endif //FASTDDS_STATISTICS

        if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
        {
            pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
        }
        pairingWriter(writer, participant_guid, *writer_data);
        return true;
    }
    return false;
}

bool EDP::unpairWriterProxy(
        const GUID_t& participant_guid,
        const GUID_t& writer_guid,
        bool removed_by_lease)
{
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, writer_guid);

    mp_RTPSParticipant->forEachUserReader([&, removed_by_lease](BaseReader& r) -> bool
            {
                if (r.matched_writer_remove(writer_guid, removed_by_lease))
                {
#if HAVE_SECURITY
                    const GUID_t& reader_guid = r.getGuid();
                    mp_RTPSParticipant->security_manager().remove_writer(reader_guid,
                    participant_guid, writer_guid);
#endif // if HAVE_SECURITY

                    //MATCHED AND ADDED CORRECTLY:
                    ReaderListener* listener = nullptr;
                    if (nullptr != (listener = r.get_listener()))
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        listener->on_reader_matched(&r, info);
                    }
                }

                // traverse all
                return true;
            });

    return true;
}

bool EDP::unpairReaderProxy(
        const GUID_t& participant_guid,
        const GUID_t& reader_guid)
{
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, reader_guid);

    mp_RTPSParticipant->forEachUserWriter([&](BaseWriter& w) -> bool
            {
                if (w.matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    const GUID_t& writer_guid = w.getGuid();
                    mp_RTPSParticipant->security_manager().remove_reader(writer_guid,
                    participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                    //MATCHED AND ADDED CORRECTLY:
                    WriterListener* listener = nullptr;
                    if (nullptr != (listener = w.get_listener()))
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        listener->on_writer_matched(&w, info);
                    }
                }

                // traverse all
                return true;
            });

    return true;
}

bool EDP::valid_matching(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata,
        MatchingFailureMask& reason,
        fastdds::dds::PolicyMask& incompatible_qos)
{
    reason.reset();
    incompatible_qos.reset();

    if (wdata->topic_name != rdata->topic_name)
    {
        reason.set(MatchingFailureMask::different_topic);
        return false;
    }

    if ((wdata->has_type_information() && wdata->type_information.assigned()) &&
            (rdata->has_type_information() && rdata->type_information.assigned()))
    {
        if (!is_same_type(wdata->type_information.type_information, rdata->type_information.type_information))
        {
            reason.set(MatchingFailureMask::different_typeinfo);
            return false;
        }
    }
    else
    {
        if (wdata->type_name != rdata->type_name)
        {
            reason.set(MatchingFailureMask::inconsistent_topic);
            return false;
        }
    }

    if (wdata->topic_kind != rdata->topic_kind)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS:Remote Reader " << rdata->guid << " is publishing in topic "
                                                                         << rdata->topic_name << "(keyed:" << rdata->topic_kind <<
                "), local writer publishes as keyed: " << wdata->topic_kind);
        reason.set(MatchingFailureMask::inconsistent_topic);
        return false;
    }

    if (!rdata->is_alive()) //Matching
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "ReaderProxyData object is NOT alive");

        return false;
    }

    if ( wdata->reliability.kind == dds::BEST_EFFORT_RELIABILITY_QOS
            && rdata->reliability.kind == dds::RELIABLE_RELIABILITY_QOS)
    //Means our writer is BE but the reader wants RE
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topic_name << "):Remote Reader "
                                                                   << rdata->guid <<
                " is Reliable and local writer is BE ");
        incompatible_qos.set(fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    }

    if (wdata->durability.kind < rdata->durability.kind)
    {
        // TODO (MCC) Change log message
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topic_name << "):RemoteReader "
                                                                   << rdata->guid <<
                " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE");
        incompatible_qos.set(fastdds::dds::DURABILITY_QOS_POLICY_ID);
    }

    if (wdata->ownership.kind != rdata->ownership.kind)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topic_name << "):Remote reader "
                                                                   << rdata->guid << " has different Ownership Kind");
        incompatible_qos.set(fastdds::dds::OWNERSHIP_QOS_POLICY_ID);
    }

    if (wdata->deadline.period > rdata->deadline.period)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topic_name << "):Remote reader "
                                                                   << rdata->guid << " has smaller DEADLINE period");
        incompatible_qos.set(fastdds::dds::DEADLINE_QOS_POLICY_ID);
    }

    if (!wdata->disable_positive_acks.enabled && rdata->disable_positive_acks.enabled)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        incompatible_qos.set(fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID);
    }

    if (wdata->liveliness.lease_duration > rdata->liveliness.lease_duration)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                << wdata->liveliness.lease_duration << " must be <= requested lease duration "
                << rdata->liveliness.lease_duration);
        incompatible_qos.set(fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    }

    if (wdata->liveliness.kind < rdata->liveliness.kind)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible liveliness kinds: offered kind is < requested kind");
        incompatible_qos.set(fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    }

    // DataRepresentationQosPolicy
    if (!checkDataRepresentationQos(wdata, rdata))
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible Data Representation QoS");
        incompatible_qos.set(fastdds::dds::DATAREPRESENTATION_QOS_POLICY_ID);
    }

#if HAVE_SECURITY
    // TODO: Check EndpointSecurityInfo
#endif // if HAVE_SECURITY

    //Partition mismatch does not trigger status change
    if (incompatible_qos.any())
    {
        reason.set(MatchingFailureMask::incompatible_qos);
        return false;
    }

    //Partition check:
    bool matched = false;
    if (wdata->partition.empty() && rdata->partition.empty())
    {
        matched = true;
    }
    else if (wdata->partition.empty() && rdata->partition.size() > 0)
    {
        for (auto rnameit = rdata->partition.begin();
                rnameit != rdata->partition.end(); ++rnameit)
        {
            if (is_partition_empty(*rnameit))
            {
                matched = true;
                break;
            }
        }
    }
    else if (wdata->partition.size() > 0 && rdata->partition.empty())
    {
        for (auto wnameit = wdata->partition.begin();
                wnameit !=  wdata->partition.end(); ++wnameit)
        {
            if (is_partition_empty(*wnameit))
            {
                matched = true;
                break;
            }
        }
    }
    else
    {
        for (auto wnameit = wdata->partition.begin();
                wnameit !=  wdata->partition.end(); ++wnameit)
        {
            for (auto rnameit = rdata->partition.begin();
                    rnameit != rdata->partition.end(); ++rnameit)
            {
                if (StringMatching::matchString(wnameit->name(), rnameit->name()))
                {
                    matched = true;
                    break;
                }
            }
            if (matched)
            {
                break;
            }
        }
    }
    if (!matched) //Different partitions
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topic_name << "): Different Partitions");
        reason.set(MatchingFailureMask::partitions);
    }

    return matched;
}

/**
 * @brief EDP::checkDataRepresentationQos
 * Table 7.57 XTypes document 1.2
 * Writer   Reader  Compatible
 * XCDR     XCDR    true
 * XCDR     XCDR2   false
 * XCDR2    XCDR    false
 * XCDR2    XCDR2   true
 * @param wdata
 * @param rdata
 * @return
 */
bool EDP::checkDataRepresentationQos(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    bool compatible = false;
    const std::vector<fastdds::dds::DataRepresentationId_t>& rr = rdata->representation.m_value;

    if (wdata->representation.m_value.empty())
    {
        compatible |=  rr.empty() ||
                std::find(rr.begin(), rr.end(), fastdds::dds::XCDR_DATA_REPRESENTATION) != rr.end();
    }
    else
    {
        dds::DataRepresentationId writerRepresentation {wdata->representation.m_value.at(0)};

        if (writerRepresentation == fastdds::dds::XCDR2_DATA_REPRESENTATION)
        {
            compatible |= std::find(rr.begin(), rr.end(), fastdds::dds::XCDR2_DATA_REPRESENTATION) != rr.end();
        }
        else if (writerRepresentation == fastdds::dds::XCDR_DATA_REPRESENTATION)
        {
            compatible |= rr.empty() ||
                    std::find(rr.begin(), rr.end(), fastdds::dds::XCDR_DATA_REPRESENTATION) != rr.end();
        }
        else // XML_DATA_REPRESENTATION
        {
            EPROSIMA_LOG_INFO(EDP, "DataRepresentationQosPolicy XML_DATA_REPRESENTATION isn't supported.");
        }
    }

    return compatible;
}

bool EDP::valid_matching(
        const ReaderProxyData* rdata,
        const WriterProxyData* wdata,
        MatchingFailureMask& reason,
        fastdds::dds::PolicyMask& incompatible_qos)
{
    return valid_matching(wdata, rdata, reason, incompatible_qos);
}

ProxyPool<ReaderProxyData>& EDP::get_temporary_reader_proxies_pool()
{
    assert(mp_PDP != nullptr);
    return mp_PDP->get_temporary_reader_proxies_pool();
}

ProxyPool<WriterProxyData>& EDP::get_temporary_writer_proxies_pool()
{
    assert(mp_PDP != nullptr);
    return mp_PDP->get_temporary_writer_proxies_pool();
}

//TODO This four functions share common code (2 to 2) and surely can be templatized.

bool EDP::pairingReader(
        RTPSReader* R,
        const GUID_t& participant_guid,
        const ReaderProxyData& rdata)
{
    static_cast<void>(participant_guid);

    BaseReader* reader = BaseReader::downcast(R);

    EPROSIMA_LOG_INFO(RTPS_EDP, rdata.guid << " in topic: \"" << rdata.topic_name << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
    if (!this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pit++;
    }

    for (; pit != mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for (auto& pair : *(*pit)->m_writers)
        {
            WriterProxyData* wdatait = pair.second;
            MatchingFailureMask no_match_reason;
            fastdds::dds::PolicyMask incompatible_qos;
            bool valid = valid_matching(&rdata, wdatait, no_match_reason, incompatible_qos);
            const GUID_t& reader_guid = reader->getGuid();
            const GUID_t& writer_guid = wdatait->guid;

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_writer(reader_guid, (*pit)->guid,
                        *wdatait, reader->getAttributes().security_attributes()))
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for reader " << reader_guid);
                }
#else
                if (reader->matched_writer_add_edp(*wdatait))
                {
                    static_cast<void>(reader_guid);  // Void cast to force usage if we don't have LOG_INFOs
                    EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "WP:" << wdatait->guid << " match R:" << reader_guid << ". RLoc:" <<
                            wdatait->remote_locators);
                    //MATCHED AND ADDED CORRECTLY:
                    if (reader->get_listener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        reader->get_listener()->on_reader_matched(reader, info);
                    }
                }
#endif // if HAVE_SECURITY
            }
            else
            {
                if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && reader->get_listener() != nullptr)
                {
                    reader->get_listener()->on_requested_incompatible_qos(reader, incompatible_qos);
                    mp_PDP->notify_incompatible_qos_matching(R->getGuid(), wdatait->guid, incompatible_qos);
                }

                //EPROSIMA_LOG_INFO(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->guid<<RTPS_DEF<<endl);
                if (reader->matched_writer_is_matched(wdatait->guid)
                        && reader->matched_writer_remove(wdatait->guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(reader_guid, participant_guid,
                            wdatait->guid);
#endif // if HAVE_SECURITY

                    //MATCHED AND ADDED CORRECTLY:
                    if (reader->get_listener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        reader->get_listener()->on_reader_matched(reader, info);
                    }
                }
            }
        }
    }
    return true;
}

// TODO Add a WriterProxyData argument with a nullptr default value
bool EDP::pairingWriter(
        RTPSWriter* W,
        const GUID_t& participant_guid,
        const WriterProxyData& wdata)
{
    static_cast<void>(participant_guid);

    BaseWriter* writer = BaseWriter::downcast(W);
    const GUID_t& writer_guid = writer->getGuid();
    static_cast<void>(writer_guid); // Void cast to force usage if LOG_INFO and SECURITY are disabled

    EPROSIMA_LOG_INFO(RTPS_EDP, writer_guid << " in topic: \"" << wdata.topic_name << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
    if (!this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pit++;
    }

    for (; pit != mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for (auto& pair : *(*pit)->m_readers)
        {
            ReaderProxyData* rdatait = pair.second;
            const GUID_t& reader_guid = rdatait->guid;
            if (reader_guid == c_Guid_Unknown)
            {
                continue;
            }

            MatchingFailureMask no_match_reason;
            fastdds::dds::PolicyMask incompatible_qos;
            bool valid = valid_matching(&wdata, rdatait, no_match_reason, incompatible_qos);

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_reader(writer_guid, (*pit)->guid,
                        *rdatait, writer->getAttributes().security_attributes()))
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for writer " << writer_guid);
                }
#else
                if (writer->matched_reader_add_edp(*rdatait))
                {
                    EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "RP:" << rdatait->guid << " match W:" << writer_guid << ". WLoc:" <<
                            rdatait->remote_locators);
                    //MATCHED AND ADDED CORRECTLY:
                    if (writer->get_listener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        writer->get_listener()->on_writer_matched(writer, info);
                    }
                }
#endif // if HAVE_SECURITY
            }
            else
            {
                if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && writer->get_listener() != nullptr)
                {
                    writer->get_listener()->on_offered_incompatible_qos(writer, incompatible_qos);
                    mp_PDP->notify_incompatible_qos_matching(W->getGuid(), rdatait->guid, incompatible_qos);
                }

                //EPROSIMA_LOG_INFO(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->guid<<RTPS_DEF<<endl);
                if (writer->matched_reader_is_matched(reader_guid) && writer->matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(writer_guid, participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                    //MATCHED AND ADDED CORRECTLY:
                    if (writer->get_listener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        writer->get_listener()->on_writer_matched(writer, info);
                    }
                }
            }
        }
    }
    return true;
}

bool EDP::pairing_reader_proxy_with_any_local_writer(
        const GUID_t& participant_guid,
        ReaderProxyData* rdata)
{
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, rdata->guid << " in topic: \"" << rdata->topic_name << "\"");

    mp_RTPSParticipant->forEachUserWriter([&, rdata](BaseWriter& w) -> bool
            {
                auto temp_writer_proxy_data = get_temporary_writer_proxies_pool().get();
                GUID_t writerGUID = w.getGuid();

                if (mp_PDP->lookupWriterProxyData(writerGUID, *temp_writer_proxy_data))
                {
                    MatchingFailureMask no_match_reason;
                    fastdds::dds::PolicyMask incompatible_qos;
                    bool valid = valid_matching(temp_writer_proxy_data.get(), rdata, no_match_reason, incompatible_qos);
                    const GUID_t& reader_guid = rdata->guid;

                    temp_writer_proxy_data.reset();

                    if (valid)
                    {
#if HAVE_SECURITY
                        if (!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID, participant_guid,
                        *rdata, w.getAttributes().security_attributes()))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                        }
#else
                        if (w.matched_reader_add_edp(*rdata))
                        {
                            EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "RP:" << rdata->guid << " match W:" << w.getGuid() << ". RLoc:" <<
                                rdata->remote_locators);
                            //MATCHED AND ADDED CORRECTLY:
                            if (w.get_listener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = MATCHED_MATCHING;
                                info.remoteEndpointGuid = reader_guid;
                                w.get_listener()->on_writer_matched(&w, info);
                            }
                        }
#endif // if HAVE_SECURITY
                    }
                    else
                    {
                        if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && w.get_listener() != nullptr)
                        {
                            w.get_listener()->on_offered_incompatible_qos(&w, incompatible_qos);
                            mp_PDP->notify_incompatible_qos_matching(w.getGuid(), rdata->guid, incompatible_qos);
                        }

                        if (w.matched_reader_is_matched(reader_guid)
                        && w.matched_reader_remove(reader_guid))
                        {
#if HAVE_SECURITY
                            mp_RTPSParticipant->security_manager().remove_reader(
                                w.getGuid(), participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                            //MATCHED AND ADDED CORRECTLY:
                            if (w.get_listener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = REMOVED_MATCHING;
                                info.remoteEndpointGuid = reader_guid;
                                w.get_listener()->on_writer_matched(&w, info);
                            }
                        }
                    }
                }
                // next iteration
                return true;
            });

    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_reader_proxy_with_local_writer(
        const GUID_t& local_writer,
        const GUID_t& remote_participant_guid,
        ReaderProxyData& rdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, rdata.guid << " in topic: \"" << rdata.topic_name << "\"");

    mp_RTPSParticipant->forEachUserWriter([&](BaseWriter& w) -> bool
            {
                GUID_t writerGUID = w.getGuid();
                const GUID_t& reader_guid = rdata.guid;

                if (local_writer == writerGUID)
                {
                    auto temp_writer_proxy_data = get_temporary_writer_proxies_pool().get();

                    if (mp_PDP->lookupWriterProxyData(writerGUID, *temp_writer_proxy_data))
                    {
                        MatchingFailureMask no_match_reason;
                        fastdds::dds::PolicyMask incompatible_qos;
                        bool valid = valid_matching(temp_writer_proxy_data.get(), &rdata, no_match_reason,
                        incompatible_qos);

                        temp_writer_proxy_data.reset();

                        if (valid)
                        {
                            if (!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID,
                            remote_participant_guid, rdata, w.getAttributes().security_attributes()))
                            {
                                EPROSIMA_LOG_ERROR(RTPS_EDP,
                                "Security manager returns an error for writer " << writerGUID);
                            }
                        }
                        else
                        {
                            if (no_match_reason.test(MatchingFailureMask::incompatible_qos) &&
                            w.get_listener() != nullptr)
                            {
                                w.get_listener()->on_offered_incompatible_qos(&w, incompatible_qos);
                                mp_PDP->notify_incompatible_qos_matching(local_writer, rdata.guid, incompatible_qos);
                            }

                            if (w.matched_reader_is_matched(reader_guid)
                            && w.matched_reader_remove(reader_guid))
                            {
                                mp_RTPSParticipant->security_manager().remove_reader(w.getGuid(),
                                remote_participant_guid, reader_guid);
                                //MATCHED AND ADDED CORRECTLY:
                                if (w.get_listener() != nullptr)
                                {
                                    MatchingInfo info;
                                    info.status = REMOVED_MATCHING;
                                    info.remoteEndpointGuid = reader_guid;
                                    w.get_listener()->on_writer_matched(&w, info);
                                }
                            }
                        }
                    }
                }
                // next iteration
                return true;
            });

    return true;
}

bool EDP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    bool matched = false;
    bool found = false;

    mp_RTPSParticipant->forEachUserWriter([&](BaseWriter& w) -> bool
            {
                GUID_t writerGUID = w.getGuid();

                const GUID_t& reader_guid = remote_reader_data.guid;

                if (local_writer == writerGUID)
                {
                    found = true;

                    if (w.matched_reader_add_edp(remote_reader_data))
                    {
                        EPROSIMA_LOG_INFO(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);

                        matched = true;

                        //MATCHED AND ADDED CORRECTLY:
                        if (w.get_listener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = MATCHED_MATCHING;
                            info.remoteEndpointGuid = reader_guid;
                            w.get_listener()->on_writer_matched(&w, info);
                        }
                    }
                    // don't look anymore
                    return false;
                }
                // keep looking
                return true;
            });

    return found ? matched : pairing_remote_reader_with_local_builtin_writer_after_security(local_writer,
                   remote_reader_data);
}

#endif // if HAVE_SECURITY

bool EDP::pairing_writer_proxy_with_any_local_reader(
        const GUID_t& participant_guid,
        WriterProxyData* wdata)
{
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, wdata->guid << " in topic: \"" << wdata->topic_name << "\"");

    mp_RTPSParticipant->forEachUserReader([&, wdata](BaseReader& r) -> bool
            {
                auto temp_reader_proxy_data = get_temporary_reader_proxies_pool().get();
                GUID_t readerGUID = r.getGuid();

                if (mp_PDP->lookupReaderProxyData(readerGUID, *temp_reader_proxy_data))
                {
                    MatchingFailureMask no_match_reason;
                    fastdds::dds::PolicyMask incompatible_qos;
                    bool valid = valid_matching(temp_reader_proxy_data.get(), wdata, no_match_reason, incompatible_qos);
                    const GUID_t& writer_guid = wdata->guid;

                    temp_reader_proxy_data.reset();

                    if (valid)
                    {
#if HAVE_SECURITY
                        if (!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID, participant_guid,
                        *wdata, r.getAttributes().security_attributes()))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                        }
#else
                        if (r.matched_writer_add_edp(*wdata))
                        {
                            EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "WP:" << wdata->guid << " match R:" << r.getGuid() << ". WLoc:" <<
                                wdata->remote_locators);
                            //MATCHED AND ADDED CORRECTLY:
                            if (r.get_listener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = MATCHED_MATCHING;
                                info.remoteEndpointGuid = writer_guid;
                                r.get_listener()->on_reader_matched(&r, info);
                            }
                        }
#endif // if HAVE_SECURITY
                    }
                    else
                    {
                        if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && r.get_listener() != nullptr)
                        {
                            r.get_listener()->on_requested_incompatible_qos(&r, incompatible_qos);
                            mp_PDP->notify_incompatible_qos_matching(r.getGuid(), wdata->guid, incompatible_qos);
                        }

                        if (r.matched_writer_is_matched(writer_guid)
                        && r.matched_writer_remove(writer_guid))
                        {
#if HAVE_SECURITY
                            mp_RTPSParticipant->security_manager().remove_writer(readerGUID, participant_guid,
                            writer_guid);
#endif // if HAVE_SECURITY
                            //MATCHED AND ADDED CORRECTLY:
                            if (r.get_listener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = REMOVED_MATCHING;
                                info.remoteEndpointGuid = writer_guid;
                                r.get_listener()->on_reader_matched(&r, info);
                            }
                        }
                    }
                }
                // keep looking
                return true;
            });

    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_writer_proxy_with_local_reader(
        const GUID_t& local_reader,
        const GUID_t& remote_participant_guid,
        WriterProxyData& wdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, wdata.guid << " in topic: \"" << wdata.topic_name << "\"");

    mp_RTPSParticipant->forEachUserReader([&](BaseReader& r) -> bool
            {
                GUID_t readerGUID = r.getGuid();

                if (local_reader == readerGUID)
                {
                    auto temp_reader_proxy_data = get_temporary_reader_proxies_pool().get();

                    if (mp_PDP->lookupReaderProxyData(readerGUID, *temp_reader_proxy_data))
                    {
                        MatchingFailureMask no_match_reason;
                        fastdds::dds::PolicyMask incompatible_qos;
                        bool valid = valid_matching(temp_reader_proxy_data.get(), &wdata, no_match_reason,
                        incompatible_qos);
                        const GUID_t& writer_guid = wdata.guid;

                        temp_reader_proxy_data.reset();

                        if (valid)
                        {
                            if (!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID,
                            remote_participant_guid, wdata, r.getAttributes().security_attributes()))
                            {
                                EPROSIMA_LOG_ERROR(RTPS_EDP,
                                "Security manager returns an error for reader " << readerGUID);
                            }
                        }
                        else
                        {
                            if (no_match_reason.test(MatchingFailureMask::incompatible_qos) &&
                            r.get_listener() != nullptr)
                            {
                                r.get_listener()->on_requested_incompatible_qos(&r, incompatible_qos);
                                mp_PDP->notify_incompatible_qos_matching(local_reader, wdata.guid, incompatible_qos);
                            }

                            if (r.matched_writer_is_matched(writer_guid)
                            && r.matched_writer_remove(writer_guid))
                            {
                                mp_RTPSParticipant->security_manager().remove_writer(readerGUID,
                                remote_participant_guid, writer_guid);
                                //MATCHED AND ADDED CORRECTLY:
                                if (r.get_listener() != nullptr)
                                {
                                    MatchingInfo info;
                                    info.status = REMOVED_MATCHING;
                                    info.remoteEndpointGuid = writer_guid;
                                    r.get_listener()->on_reader_matched(&r, info);
                                }
                            }
                        }
                    }
                    // don't keep searching
                    return false;
                }
                // keep searching
                return true;
            });

    return true;
}

bool EDP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    bool matched = false;
    bool found = false;

    mp_RTPSParticipant->forEachUserReader([&](BaseReader& r) -> bool
            {
                GUID_t readerGUID = r.getGuid();

                const GUID_t& writer_guid = remote_writer_data.guid;

                if (local_reader == readerGUID)
                {
                    found = true;

                    if (r.matched_writer_add_edp(remote_writer_data))
                    {
                        EPROSIMA_LOG_INFO(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);

                        matched = true;

                        //MATCHED AND ADDED CORRECTLY:
                        if (r.get_listener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = MATCHED_MATCHING;
                            info.remoteEndpointGuid = writer_guid;
                            r.get_listener()->on_reader_matched(&r, info);

                        }
                    }
                    // dont' look anymore
                    return false;
                }
                // keep looking
                return true;
            });

    return found ? matched : pairing_remote_writer_with_local_builtin_reader_after_security(local_reader,
                   remote_writer_data);
}

#endif // if HAVE_SECURITY

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
