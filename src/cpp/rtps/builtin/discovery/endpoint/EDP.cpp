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

#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>

#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/WriterListener.h>

#include <fastrtps/attributes/TopicAttributes.h>

#include <fastrtps/utils/StringMatching.h>

#include <fastrtps/types/TypeObjectFactory.h>

#include <fastdds/core/policy/ParameterList.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <rtps/builtin/data/ProxyHashTables.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#ifdef FASTDDS_STATISTICS
#include <statistics/rtps/monitor-service/interfaces/IProxyObserver.hpp>
#endif //FASTDDS_STATISTICS

#include <utils/collections/node_size_helpers.hpp>

#include <mutex>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;
using eprosima::fastdds::dds::PublicationMatchedStatus;
using eprosima::fastdds::dds::SubscriptionMatchedStatus;
using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

using reader_map_helper = utilities::collections::map_size_helper<GUID_t, SubscriptionMatchedStatus>;
using writer_map_helper = utilities::collections::map_size_helper<GUID_t, PublicationMatchedStatus>;

static bool is_partition_empty(
        const fastdds::dds::Partition_t& partition)
{
    return partition.size() <= 1 && 0 == strlen(partition.name());
}

EDP::EDP(
        PDP* p,
        RTPSParticipantImpl* part)
    : mp_PDP(p)
    , mp_RTPSParticipant(part)
    , reader_status_allocator_(
        reader_map_helper::node_size,
        reader_map_helper::min_pool_size<pool_allocator_t>(
            part->getRTPSParticipantAttributes().allocation.total_readers().initial))
    , writer_status_allocator_(
        writer_map_helper::node_size,
        writer_map_helper::min_pool_size<pool_allocator_t>(
            part->getRTPSParticipantAttributes().allocation.total_writers().initial))
    , reader_status_(reader_status_allocator_)
    , writer_status_(writer_status_allocator_)
{
}

EDP::~EDP()
{
    // TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(
        RTPSReader* reader,
        const TopicAttributes& att,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Adding " << reader->getGuid().entityId << " in topic " << att.topicName);

    auto init_fun = [this, reader, &att, &rqos, content_filter](
        ReaderProxyData* rpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP,
                            "Adding already existent reader " << reader->getGuid().entityId << " in topic "
                                                              << att.topicName);
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                const auto& ratt = reader->getAttributes();

                rpd->isAlive(true);
                rpd->m_expectsInlineQos = reader->expectsInlineQos();
                rpd->guid(reader->getGuid());
                rpd->key() = rpd->guid();
                if (ratt.multicastLocatorList.empty() && ratt.unicastLocatorList.empty())
                {
                    rpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    rpd->set_multicast_locators(ratt.multicastLocatorList, network);
                    rpd->set_announced_unicast_locators(ratt.unicastLocatorList);
                    fastdds::rtps::network::external_locators::add_external_locators(*rpd,
                            ratt.external_unicast_locators);
                }
                rpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
                rpd->topicName(att.getTopicName());
                rpd->typeName(att.getTopicDataType());
                rpd->topicKind(att.getTopicKind());
                if (att.type_id.m_type_identifier._d() != static_cast<uint8_t>(0x00))
                {
                    rpd->type_id(att.type_id);
                }
                if (att.type.m_type_object._d() != static_cast<uint8_t>(0x00))
                {
                    rpd->type(att.type);
                }
                if (att.type_information.assigned())
                {
                    rpd->type_information(att.type_information);
                }
                rpd->m_qos.setQos(rqos, true);
                rpd->userDefinedId(ratt.getUserDefinedID());
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

                    rpd->content_filter(*content_filter);
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
                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!att.type_information.assigned())
                    {
                        const types::TypeInformation* type_info =
                                types::TypeObjectFactory::get_instance()->get_type_information(rpd->typeName().c_str());
                        if (type_info != nullptr)
                        {
                            rpd->type_information() = *type_info;
                        }
                    }
                }

                if (att.auto_fill_type_object)
                {
                    bool has_type_id = true;
                    if (att.type_id.m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        has_type_id = false;
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            rpd->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            has_type_id = true;
                            rpd->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (att.type.m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        bool type_is_complete = has_type_id &&
                                rpd->type_id().m_type_identifier._d() == types::EK_COMPLETE;
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            rpd->typeName().c_str(), type_is_complete);
                        if (type_obj != nullptr)
                        {
                            rpd->type().m_type_object = *type_obj;
                        }
                    }
                }

                return true;
            };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(reader->getGuid(), participant_guid, init_fun);
    if (reader_data == nullptr)
    {
        return false;
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service about the new local entity proxy
    if (nullptr != this->mp_PDP->get_proxy_observer())
    {
        this->mp_PDP->get_proxy_observer()->on_local_entity_change(reader_data->guid(), true);
    }
#endif //FASTDDS_STATISTICS

    //PAIRING
    if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
    }
    pairingReader(reader, participant_guid, *reader_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalReaderProxyData(reader, reader_data);
    return true;
}

bool EDP::newLocalWriterProxyData(
        RTPSWriter* writer,
        const TopicAttributes& att,
        const WriterQos& wqos)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Adding " << writer->getGuid().entityId << " in topic " << att.topicName);

    auto init_fun = [this, writer, &att, &wqos](
        WriterProxyData* wpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP,
                            "Adding already existent writer " << writer->getGuid().entityId << " in topic "
                                                              << att.topicName);
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                const auto& watt = writer->getAttributes();

                wpd->guid(writer->getGuid());
                wpd->key() = wpd->guid();
                if (watt.multicastLocatorList.empty() && watt.unicastLocatorList.empty())
                {
                    wpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    wpd->set_multicast_locators(watt.multicastLocatorList, network);
                    wpd->set_announced_unicast_locators(watt.unicastLocatorList);
                    fastdds::rtps::network::external_locators::add_external_locators(*wpd,
                            watt.external_unicast_locators);
                }
                wpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
                wpd->topicName(att.getTopicName());
                wpd->typeName(att.getTopicDataType());
                wpd->topicKind(att.getTopicKind());
                if (att.type_id.m_type_identifier._d() != static_cast<uint8_t>(0x00))
                {
                    wpd->type_id(att.type_id);
                }
                if (att.type.m_type_object._d() != static_cast<uint8_t>(0x00))
                {
                    wpd->type(att.type);
                }
                if (att.type_information.assigned())
                {
                    wpd->type_information(att.type_information);
                }
                wpd->typeMaxSerialized(writer->getTypeMaxSerialized());
                wpd->m_qos.setQos(wqos, true);
                wpd->userDefinedId(watt.getUserDefinedID());
                wpd->persistence_guid(watt.persistence_guid);
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

                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!att.type_information.assigned())
                    {
                        const types::TypeInformation* type_info =
                                types::TypeObjectFactory::get_instance()->get_type_information(wpd->typeName().c_str());
                        if (type_info != nullptr)
                        {
                            wpd->type_information() = *type_info;
                        }
                    }
                }

                if (att.auto_fill_type_object)
                {
                    bool has_type_id = true;
                    if (att.type_id.m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        has_type_id = false;
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            wpd->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            has_type_id = true;
                            wpd->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (att.type.m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        bool type_is_complete = has_type_id &&
                                wpd->type_id().m_type_identifier._d() == types::EK_COMPLETE;
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            wpd->typeName().c_str(), type_is_complete);
                        if (type_obj != nullptr)
                        {
                            wpd->type().m_type_object = *type_obj;
                        }
                    }
                }

                return true;
            };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer->getGuid(), participant_guid, init_fun);
    if (writer_data == nullptr)
    {
        return false;
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service about the new local entity proxy
    if (nullptr != this->mp_PDP->get_proxy_observer())
    {
        this->mp_PDP->get_proxy_observer()->on_local_entity_change(writer_data->guid(), true);
    }
#endif //FASTDDS_STATISTICS

    //PAIRING
    if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
    {
        pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
    }
    pairingWriter(writer, participant_guid, *writer_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalWriterProxyData(writer, writer_data);
    return true;
}

bool EDP::updatedLocalReader(
        RTPSReader* reader,
        const TopicAttributes& att,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    auto init_fun = [this, reader, &rqos, &att, content_filter](
        ReaderProxyData* rdata,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                // Should only be called for existent data
                (void)updating;
                assert(updating);

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                if (reader->getAttributes().multicastLocatorList.empty() &&
                        reader->getAttributes().unicastLocatorList.empty())
                {
                    rdata->set_locators(participant_data.default_locators);
                }
                else
                {
                    rdata->set_multicast_locators(reader->getAttributes().multicastLocatorList, network);
                    rdata->set_announced_unicast_locators(reader->getAttributes().unicastLocatorList);
                }
                rdata->m_qos.setQos(rqos, false);
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

                    rdata->content_filter(*content_filter);
                }
                else
                {
                    rdata->content_filter().filter_class_name = "";
                    rdata->content_filter().filter_expression = "";
                }
                rdata->isAlive(true);
                rdata->m_expectsInlineQos = reader->expectsInlineQos();

                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!rdata->type_information().assigned())
                    {
                        const types::TypeInformation* type_info =
                                types::TypeObjectFactory::get_instance()->get_type_information(rdata->typeName().c_str());
                        if (type_info != nullptr)
                        {
                            rdata->type_information() = *type_info;
                        }
                    }
                }

                if (att.auto_fill_type_object)
                {

                    if (rdata->type_id().m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            rdata->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            rdata->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (rdata->type().m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            rdata->typeName().c_str(), rdata->type_id().m_type_identifier._d() == types::EK_COMPLETE);
                        if (type_obj != nullptr)
                        {
                            rdata->type().m_type_object = *type_obj;
                        }
                    }
                }

                return true;
            };

    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(reader->getGuid(), participant_guid, init_fun);
    if (reader_data != nullptr)
    {
        processLocalReaderProxyData(reader, reader_data);
#ifdef FASTDDS_STATISTICS
        // notify monitor service about the new local entity proxy update
        if (nullptr != this->mp_PDP->get_proxy_observer())
        {
            this->mp_PDP->get_proxy_observer()->on_local_entity_change(reader_data->guid(), true);
        }
#endif //FASTDDS_STATISTICS
        if (this->mp_PDP->getRTPSParticipant()->should_match_local_endpoints())
        {
            pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
        }
        pairingReader(reader, participant_guid, *reader_data);
        return true;
    }
    return false;
}

bool EDP::updatedLocalWriter(
        RTPSWriter* writer,
        const TopicAttributes& att,
        const WriterQos& wqos)
{
    auto init_fun = [this, writer, &wqos, &att](
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
                    wdata->set_multicast_locators(writer->getAttributes().multicastLocatorList, network);
                    wdata->set_announced_unicast_locators(writer->getAttributes().unicastLocatorList);
                }
                wdata->m_qos.setQos(wqos, false);

                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!wdata->type_information().assigned())
                    {
                        const types::TypeInformation* type_info =
                                types::TypeObjectFactory::get_instance()->get_type_information(wdata->typeName().c_str());
                        if (type_info != nullptr)
                        {
                            wdata->type_information() = *type_info;
                        }
                    }
                }

                if (att.auto_fill_type_object)
                {

                    if (wdata->type_id().m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            wdata->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            wdata->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (wdata->type().m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            wdata->typeName().c_str(), wdata->type_id().m_type_identifier._d() == types::EK_COMPLETE);
                        if (type_obj != nullptr)
                        {
                            wdata->type().m_type_object = *type_obj;
                        }
                    }
                }

                return true;
            };

    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer->getGuid(), participant_guid, init_fun);
    if (writer_data != nullptr)
    {
        processLocalWriterProxyData(writer, writer_data);

#ifdef FASTDDS_STATISTICS
        // notify monitor service about the new local entity proxy update
        if (nullptr != this->mp_PDP->get_proxy_observer())
        {
            this->mp_PDP->get_proxy_observer()->on_local_entity_change(writer_data->guid(), true);
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

    mp_RTPSParticipant->forEachUserReader([&, removed_by_lease](RTPSReader& r) -> bool
            {
                if (r.matched_writer_remove(writer_guid, removed_by_lease))
                {
                    const GUID_t& reader_guid = r.getGuid();
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(reader_guid,
                    participant_guid, writer_guid);
#endif // if HAVE_SECURITY

                    //MATCHED AND ADDED CORRECTLY:
                    ReaderListener* listener = nullptr;
                    if (nullptr != (listener = r.getListener()))
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        listener->onReaderMatched(&r, info);

                        const SubscriptionMatchedStatus& sub_info =
                        update_subscription_matched_status(reader_guid, writer_guid, -1);
                        listener->onReaderMatched(&r, sub_info);
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

    mp_RTPSParticipant->forEachUserWriter([&](RTPSWriter& w) -> bool
            {
                if (w.matched_reader_remove(reader_guid))
                {
                    const GUID_t& writer_guid = w.getGuid();
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(writer_guid,
                    participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                    //MATCHED AND ADDED CORRECTLY:
                    WriterListener* listener = nullptr;
                    if (nullptr != (listener = w.getListener()))
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        listener->onWriterMatched(&w, info);

                        const PublicationMatchedStatus& pub_info =
                        update_publication_matched_status(reader_guid, writer_guid, -1);
                        listener->onWriterMatched(&w, pub_info);
                    }
                }

                // traverse all
                return true;
            });

    return true;
}

bool EDP::validMatching(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata)
{
    MatchingFailureMask reason;
    fastdds::dds::PolicyMask incompatible_qos;
    return valid_matching(wdata, rdata, reason, incompatible_qos);
}

bool EDP::valid_matching(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata,
        MatchingFailureMask& reason,
        fastdds::dds::PolicyMask& incompatible_qos)
{
    reason.reset();
    incompatible_qos.reset();

    if (wdata->topicName() != rdata->topicName())
    {
        reason.set(MatchingFailureMask::different_topic);
        return false;
    }

    // Type Consistency Enforcement QosPolicy
    if (!checkTypeValidation(wdata, rdata))
    {
        // TODO Trigger INCONSISTENT_TOPIC status change
        reason.set(MatchingFailureMask::inconsistent_topic);
        return false;
    }

    if (wdata->topicKind() != rdata->topicKind())
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS:Remote Reader " << rdata->guid() << " is publishing in topic "
                                                                         << rdata->topicName() << "(keyed:" << rdata->topicKind() <<
                "), local writer publishes as keyed: " << wdata->topicKind());

        reason.set(MatchingFailureMask::inconsistent_topic);
        return false;
    }

    if (!rdata->isAlive()) //Matching
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "ReaderProxyData object is NOT alive");

        return false;
    }

    if ( wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS
            && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    //Means our writer is BE but the reader wants RE
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote Reader "
                                                                   << rdata->guid() <<
                " is Reliable and local writer is BE ");
        incompatible_qos.set(fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    }

    if (wdata->m_qos.m_durability.kind < rdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):RemoteReader "
                                                                   << rdata->guid() <<
                " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE");
        incompatible_qos.set(fastdds::dds::DURABILITY_QOS_POLICY_ID);
    }

    if (wdata->m_qos.m_ownership.kind != rdata->m_qos.m_ownership.kind)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote reader "
                                                                   << rdata->guid() << " has different Ownership Kind");
        incompatible_qos.set(fastdds::dds::OWNERSHIP_QOS_POLICY_ID);
    }

    if (wdata->m_qos.m_deadline.period > rdata->m_qos.m_deadline.period)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote reader "
                                                                   << rdata->guid() << " has smaller DEADLINE period");
        incompatible_qos.set(fastdds::dds::DEADLINE_QOS_POLICY_ID);
    }

    if (!wdata->m_qos.m_disablePositiveACKs.enabled && rdata->m_qos.m_disablePositiveACKs.enabled)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        incompatible_qos.set(fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID);
    }

    if (wdata->m_qos.m_liveliness.lease_duration > rdata->m_qos.m_liveliness.lease_duration)
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                << wdata->m_qos.m_liveliness.lease_duration << " must be <= requested lease duration "
                << rdata->m_qos.m_liveliness.lease_duration);
        incompatible_qos.set(fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    }

    if (wdata->m_qos.m_liveliness.kind < rdata->m_qos.m_liveliness.kind)
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
    if (wdata->m_qos.m_partition.empty() && rdata->m_qos.m_partition.empty())
    {
        matched = true;
    }
    else if (wdata->m_qos.m_partition.empty() && rdata->m_qos.m_partition.size() > 0)
    {
        for (auto rnameit = rdata->m_qos.m_partition.begin();
                rnameit != rdata->m_qos.m_partition.end(); ++rnameit)
        {
            if (is_partition_empty(*rnameit))
            {
                matched = true;
                break;
            }
        }
    }
    else if (wdata->m_qos.m_partition.size() > 0 && rdata->m_qos.m_partition.empty())
    {
        for (auto wnameit = wdata->m_qos.m_partition.begin();
                wnameit !=  wdata->m_qos.m_partition.end(); ++wnameit)
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
        for (auto wnameit = wdata->m_qos.m_partition.begin();
                wnameit !=  wdata->m_qos.m_partition.end(); ++wnameit)
        {
            for (auto rnameit = rdata->m_qos.m_partition.begin();
                    rnameit != rdata->m_qos.m_partition.end(); ++rnameit)
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
        EPROSIMA_LOG_WARNING(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "): Different Partitions");
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
    const std::vector<fastdds::dds::DataRepresentationId_t>& rr = rdata->m_qos.representation.m_value;

    if (wdata->m_qos.representation.m_value.empty())
    {
        compatible |=  rr.empty() ||
                std::find(rr.begin(), rr.end(), fastdds::dds::XCDR_DATA_REPRESENTATION) != rr.end();
    }
    else
    {
        DataRepresentationId writerRepresentation {wdata->m_qos.representation.m_value.at(0)};

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

bool EDP::checkTypeValidation(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    // Step 1: Both specify a TypeObject
    if (hasTypeObject(wdata, rdata))
    {
        return checkTypeObject(wdata, rdata);
    }
    // Not explicitely said in the standard, but is not done, what's the intention of TypeIdV1?
    if (hasTypeIdentifier(wdata, rdata))
    {
        return checkTypeIdentifier(wdata, rdata);
    }

    // Step 2: Writer or reader doesn't specify a TypeObject
    return !rdata->m_qos.type_consistency.m_force_type_validation && (wdata->typeName() == rdata->typeName());
}

bool EDP::validMatching(
        const ReaderProxyData* rdata,
        const WriterProxyData* wdata)
{
    MatchingFailureMask reason;
    fastdds::dds::PolicyMask incompatible_qos;
    return valid_matching(rdata, wdata, reason, incompatible_qos);
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
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, rdata.guid() << " in topic: \"" << rdata.topicName() << "\"");
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
            const GUID_t& reader_guid = R->getGuid();
            const GUID_t& writer_guid = wdatait->guid();

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_writer(R->m_guid, (*pit)->m_guid,
                        *wdatait, R->getAttributes().security_attributes()))
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for reader " << reader_guid);
                }
#else
                if (R->matched_writer_add(*wdatait))
                {
                    EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "WP:" << wdatait->guid() << " match R:" << R->getGuid() << ". RLoc:" <<
                            wdatait->remote_locators());
                    //MATCHED AND ADDED CORRECTLY:
                    if (R->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        R->getListener()->onReaderMatched(R, info);

                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(reader_guid, writer_guid, 1);
                        R->getListener()->onReaderMatched(R, sub_info);
                    }
                }
#endif // if HAVE_SECURITY
            }
            else
            {
                if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && R->getListener() != nullptr)
                {
                    R->getListener()->on_requested_incompatible_qos(R, incompatible_qos);
                }

                //EPROSIMA_LOG_INFO(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if (R->matched_writer_is_matched(wdatait->guid())
                        && R->matched_writer_remove(wdatait->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(reader_guid, participant_guid,
                            wdatait->guid());
#endif // if HAVE_SECURITY

                    //MATCHED AND ADDED CORRECTLY:
                    if (R->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        R->getListener()->onReaderMatched(R, info);

                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(reader_guid, writer_guid, -1);
                        R->getListener()->onReaderMatched(R, sub_info);
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
    (void)participant_guid;

    EPROSIMA_LOG_INFO(RTPS_EDP, W->getGuid() << " in topic: \"" << wdata.topicName() << "\"");
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
            const GUID_t& reader_guid = rdatait->guid();
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
                if (!mp_RTPSParticipant->security_manager().discovered_reader(W->getGuid(), (*pit)->m_guid,
                        *rdatait, W->getAttributes().security_attributes()))
                {
                    EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for writer " << W->getGuid());
                }
#else
                if (W->matched_reader_add(*rdatait))
                {
                    EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "RP:" << rdatait->guid() << " match W:" << W->getGuid() << ". WLoc:" <<
                            rdatait->remote_locators());
                    //MATCHED AND ADDED CORRECTLY:
                    if (W->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W, info);

                        const GUID_t& writer_guid = W->getGuid();
                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, writer_guid, 1);
                        W->getListener()->onWriterMatched(W, pub_info);
                    }
                }
#endif // if HAVE_SECURITY
            }
            else
            {
                if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && W->getListener() != nullptr)
                {
                    W->getListener()->on_offered_incompatible_qos(W, incompatible_qos);
                }

                //EPROSIMA_LOG_INFO(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if (W->matched_reader_is_matched(reader_guid) && W->matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(W->getGuid(), participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                    //MATCHED AND ADDED CORRECTLY:
                    if (W->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W, info);

                        const GUID_t& writer_guid = W->getGuid();
                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, writer_guid, -1);
                        W->getListener()->onWriterMatched(W, pub_info);

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

    EPROSIMA_LOG_INFO(RTPS_EDP, rdata->guid() << " in topic: \"" << rdata->topicName() << "\"");

    mp_RTPSParticipant->forEachUserWriter([&, rdata](RTPSWriter& w) -> bool
            {
                auto temp_writer_proxy_data = get_temporary_writer_proxies_pool().get();
                GUID_t writerGUID = w.getGuid();

                if (mp_PDP->lookupWriterProxyData(writerGUID, *temp_writer_proxy_data))
                {
                    MatchingFailureMask no_match_reason;
                    fastdds::dds::PolicyMask incompatible_qos;
                    bool valid = valid_matching(temp_writer_proxy_data.get(), rdata, no_match_reason, incompatible_qos);
                    const GUID_t& reader_guid = rdata->guid();

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
                        if (w.matched_reader_add(*rdata))
                        {
                            EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "RP:" << rdata->guid() << " match W:" << w.getGuid() << ". RLoc:" <<
                                rdata->remote_locators());
                            //MATCHED AND ADDED CORRECTLY:
                            if (w.getListener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = MATCHED_MATCHING;
                                info.remoteEndpointGuid = reader_guid;
                                w.getListener()->onWriterMatched(&w, info);

                                const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, writerGUID, 1);
                                w.getListener()->onWriterMatched(&w, pub_info);
                            }
                        }
#endif // if HAVE_SECURITY
                    }
                    else
                    {
                        if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && w.getListener() != nullptr)
                        {
                            w.getListener()->on_offered_incompatible_qos(&w, incompatible_qos);
                        }

                        if (w.matched_reader_is_matched(reader_guid)
                        && w.matched_reader_remove(reader_guid))
                        {
#if HAVE_SECURITY
                            mp_RTPSParticipant->security_manager().remove_reader(
                                w.getGuid(), participant_guid, reader_guid);
#endif // if HAVE_SECURITY
                            //MATCHED AND ADDED CORRECTLY:
                            if (w.getListener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = REMOVED_MATCHING;
                                info.remoteEndpointGuid = reader_guid;
                                w.getListener()->onWriterMatched(&w, info);

                                const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, writerGUID, -1);
                                w.getListener()->onWriterMatched(&w, pub_info);
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
    EPROSIMA_LOG_INFO(RTPS_EDP, rdata.guid() << " in topic: \"" << rdata.topicName() << "\"");

    mp_RTPSParticipant->forEachUserWriter([&](RTPSWriter& w) -> bool
            {
                GUID_t writerGUID = w.getGuid();
                const GUID_t& reader_guid = rdata.guid();

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
                            w.getListener() != nullptr)
                            {
                                w.getListener()->on_offered_incompatible_qos(&w, incompatible_qos);
                            }

                            if (w.matched_reader_is_matched(reader_guid)
                            && w.matched_reader_remove(reader_guid))
                            {
                                mp_RTPSParticipant->security_manager().remove_reader(w.getGuid(),
                                remote_participant_guid, reader_guid);
                                //MATCHED AND ADDED CORRECTLY:
                                if (w.getListener() != nullptr)
                                {
                                    MatchingInfo info;
                                    info.status = REMOVED_MATCHING;
                                    info.remoteEndpointGuid = reader_guid;
                                    w.getListener()->onWriterMatched(&w, info);

                                    const PublicationMatchedStatus& pub_info =
                                    update_publication_matched_status(reader_guid, writerGUID, -1);
                                    w.getListener()->onWriterMatched(&w, pub_info);
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

    mp_RTPSParticipant->forEachUserWriter([&](RTPSWriter& w) -> bool
            {
                GUID_t writerGUID = w.getGuid();

                const GUID_t& reader_guid = remote_reader_data.guid();

                if (local_writer == writerGUID)
                {
                    found = true;

                    if (w.matched_reader_add(remote_reader_data))
                    {
                        EPROSIMA_LOG_INFO(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);

                        matched = true;

                        //MATCHED AND ADDED CORRECTLY:
                        if (w.getListener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = MATCHED_MATCHING;
                            info.remoteEndpointGuid = reader_guid;
                            w.getListener()->onWriterMatched(&w, info);

                            const PublicationMatchedStatus& pub_info =
                            update_publication_matched_status(reader_guid, writerGUID, 1);
                            w.getListener()->onWriterMatched(&w, pub_info);
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

    EPROSIMA_LOG_INFO(RTPS_EDP, wdata->guid() << " in topic: \"" << wdata->topicName() << "\"");

    mp_RTPSParticipant->forEachUserReader([&, wdata](RTPSReader& r) -> bool
            {
                auto temp_reader_proxy_data = get_temporary_reader_proxies_pool().get();
                GUID_t readerGUID = r.getGuid();

                if (mp_PDP->lookupReaderProxyData(readerGUID, *temp_reader_proxy_data))
                {
                    MatchingFailureMask no_match_reason;
                    fastdds::dds::PolicyMask incompatible_qos;
                    bool valid = valid_matching(temp_reader_proxy_data.get(), wdata, no_match_reason, incompatible_qos);
                    const GUID_t& writer_guid = wdata->guid();

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
                        if (r.matched_writer_add(*wdata))
                        {
                            EPROSIMA_LOG_INFO(RTPS_EDP_MATCH,
                            "WP:" << wdata->guid() << " match R:" << r.getGuid() << ". WLoc:" <<
                                wdata->remote_locators());
                            //MATCHED AND ADDED CORRECTLY:
                            if (r.getListener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = MATCHED_MATCHING;
                                info.remoteEndpointGuid = writer_guid;
                                r.getListener()->onReaderMatched(&r, info);

                                const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(readerGUID, writer_guid, 1);
                                r.getListener()->onReaderMatched(&r, sub_info);
                            }
                        }
#endif // if HAVE_SECURITY
                    }
                    else
                    {
                        if (no_match_reason.test(MatchingFailureMask::incompatible_qos) && r.getListener() != nullptr)
                        {
                            r.getListener()->on_requested_incompatible_qos(&r, incompatible_qos);
                        }

                        if (r.matched_writer_is_matched(writer_guid)
                        && r.matched_writer_remove(writer_guid))
                        {
#if HAVE_SECURITY
                            mp_RTPSParticipant->security_manager().remove_writer(readerGUID, participant_guid,
                            writer_guid);
#endif // if HAVE_SECURITY
                            //MATCHED AND ADDED CORRECTLY:
                            if (r.getListener() != nullptr)
                            {
                                MatchingInfo info;
                                info.status = REMOVED_MATCHING;
                                info.remoteEndpointGuid = writer_guid;
                                r.getListener()->onReaderMatched(&r, info);

                                const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(readerGUID, writer_guid, -1);
                                r.getListener()->onReaderMatched(&r, sub_info);
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
    EPROSIMA_LOG_INFO(RTPS_EDP, wdata.guid() << " in topic: \"" << wdata.topicName() << "\"");

    mp_RTPSParticipant->forEachUserReader([&](RTPSReader& r) -> bool
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
                        const GUID_t& writer_guid = wdata.guid();

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
                            r.getListener() != nullptr)
                            {
                                r.getListener()->on_requested_incompatible_qos(&r, incompatible_qos);
                            }

                            if (r.matched_writer_is_matched(writer_guid)
                            && r.matched_writer_remove(writer_guid))
                            {
                                mp_RTPSParticipant->security_manager().remove_writer(readerGUID,
                                remote_participant_guid, writer_guid);
                                //MATCHED AND ADDED CORRECTLY:
                                if (r.getListener() != nullptr)
                                {
                                    MatchingInfo info;
                                    info.status = REMOVED_MATCHING;
                                    info.remoteEndpointGuid = writer_guid;
                                    r.getListener()->onReaderMatched(&r, info);

                                    const SubscriptionMatchedStatus& sub_info =
                                    update_subscription_matched_status(readerGUID, writer_guid, -1);
                                    r.getListener()->onReaderMatched(&r, sub_info);
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

    mp_RTPSParticipant->forEachUserReader([&](RTPSReader& r) -> bool
            {
                GUID_t readerGUID = r.getGuid();

                const GUID_t& writer_guid = remote_writer_data.guid();

                if (local_reader == readerGUID)
                {
                    found = true;

                    // TODO(richiware) Implement and use move with attributes
                    if (r.matched_writer_add(remote_writer_data))
                    {
                        EPROSIMA_LOG_INFO(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);

                        matched = true;

                        //MATCHED AND ADDED CORRECTLY:
                        if (r.getListener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = MATCHED_MATCHING;
                            info.remoteEndpointGuid = writer_guid;
                            r.getListener()->onReaderMatched(&r, info);

                            const SubscriptionMatchedStatus& sub_info =
                            update_subscription_matched_status(readerGUID, writer_guid, 1);
                            r.getListener()->onReaderMatched(&r, sub_info);

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

bool EDP::checkTypeIdentifier(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    // TODO - Remove once XCDR or XCDR2 is implemented.
    TypeConsistencyEnforcementQosPolicy coercion;
    coercion.m_kind = DISALLOW_TYPE_COERCION;
    coercion.m_ignore_member_names = false;
    coercion.m_ignore_string_bounds = false;
    coercion.m_force_type_validation = true;
    coercion.m_prevent_type_widening = true;
    coercion.m_ignore_sequence_bounds = false;
    return wdata->type_id().m_type_identifier._d() != static_cast<uint8_t>(0x00) &&
           wdata->type_id().m_type_identifier.consistent(
        //rdata->type_id().m_type_identifier, rdata->m_qos.type_consistency);
        rdata->type_id().m_type_identifier, coercion);
}

bool EDP::hasTypeIdentifier(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    return wdata->has_type_id() && wdata->type_id().m_type_identifier._d() != static_cast<uint8_t>(0x00) &&
           rdata->has_type_id() && rdata->type_id().m_type_identifier._d() != static_cast<uint8_t>(0x00);
}

bool EDP::checkTypeObject(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    if (wdata->has_type_information() && wdata->type_information().assigned() &&
            rdata->has_type_information() && rdata->type_information().assigned())
    {
        const types::TypeIdentifier* rtype = nullptr;
        const types::TypeIdentifier* wtype = nullptr;

        if (wdata->type_information().type_information.complete().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00) &&
                rdata->type_information().type_information.complete().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00))
        {
            rtype = &rdata->type_information().type_information.complete().typeid_with_size().type_id();
            wtype = &wdata->type_information().type_information.complete().typeid_with_size().type_id();
        }
        else if (wdata->type_information().type_information.minimal().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00) &&
                rdata->type_information().type_information.minimal().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00))
        {
            rtype = &rdata->type_information().type_information.minimal().typeid_with_size().type_id();
            wtype = &wdata->type_information().type_information.minimal().typeid_with_size().type_id();
        }

        if (wtype != nullptr)
        {
            // TODO - Remove once XCDR or XCDR2 is implemented.
            /*
             * Currently consistency checks are applied to type structure and compatibility,
             * but doesn't check about annotations behavior.
             * This may cause false matching cases with annotations @key or @non_serialize,
             * for example.
             * Once XCDR or XCDR2 is implemented, is it doesn't solve this cases, we must
             * think about this problem and how consistency could solve it.
             */
            TypeConsistencyEnforcementQosPolicy coercion;
            coercion.m_kind = DISALLOW_TYPE_COERCION;
            coercion.m_ignore_member_names = false;
            coercion.m_ignore_string_bounds = false;
            coercion.m_force_type_validation = true;
            coercion.m_prevent_type_widening = true;
            coercion.m_ignore_sequence_bounds = false;
            //return wtype->consistent(*rtype, rdata->m_qos.type_consistency);
            return wtype->consistent(*rtype, coercion);
        }

        return false;
    }

    if (wdata->has_type() && wdata->type().m_type_object._d() != static_cast<uint8_t>(0x00) &&
            rdata->has_type() && rdata->type().m_type_object._d() != static_cast<uint8_t>(0x00))
    {
        // TODO - Remove once XCDR or XCDR2 is implemented.
        /*
         * Currently consistency checks are applied to type structure and compatibility,
         * but doesn't check about annotations behavior.
         * This may cause false matching cases with annotations @key or @non_serialize,
         * for example.
         * Once XCDR or XCDR2 is implemented, is it doesn't solve this cases, we must
         * think about this problem and how consistency could solve it.
         */
        TypeConsistencyEnforcementQosPolicy coercion;
        coercion.m_kind = DISALLOW_TYPE_COERCION;
        coercion.m_ignore_member_names = false;
        coercion.m_ignore_string_bounds = false;
        coercion.m_force_type_validation = true;
        coercion.m_prevent_type_widening = true;
        coercion.m_ignore_sequence_bounds = false;
        //return wdata->type().m_type_object.consistent(rdata->type().m_type_object, rdata->m_qos.type_consistency);
        return wdata->type().m_type_object.consistent(rdata->type().m_type_object, coercion);
    }

    return false;
}

bool EDP::hasTypeObject(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata) const
{
    if (wdata->has_type_information() && wdata->type_information().assigned() &&
            rdata->has_type_information() && rdata->type_information().assigned())
    {
        if (wdata->type_information().type_information.complete().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00) &&
                rdata->type_information().type_information.complete().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00))
        {
            return true;
        }
        else if (wdata->type_information().type_information.minimal().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00) &&
                rdata->type_information().type_information.minimal().typeid_with_size().type_id()._d() !=
                static_cast<uint8_t>(0x00))
        {
            return true;
        }
        return false;
    }

    if (wdata->has_type() && wdata->type().m_type_object._d() != static_cast<uint8_t>(0x00) &&
            rdata->has_type() && rdata->type().m_type_object._d() != static_cast<uint8_t>(0x00))
    {
        return true;
    }

    return false;
}

const SubscriptionMatchedStatus& EDP::update_subscription_matched_status(
        const GUID_t& reader_guid,
        const GUID_t& writer_guid,
        int change)
{
    std::lock_guard<std::recursive_mutex> _(*mp_PDP->getMutex());

    SubscriptionMatchedStatus* status;
    auto it = reader_status_.find(reader_guid);
    if (it == reader_status_.end())
    {
        auto pair = reader_status_.emplace(reader_guid, SubscriptionMatchedStatus{});
        status = &pair.first->second;
    }
    else
    {
        status = &it->second;
    }

    status->current_count = change;
    status->current_count_change = change;
    status->total_count += change;
    status->total_count_change += change;
    status->last_publication_handle = writer_guid;

    return *status;
}

const fastdds::dds::PublicationMatchedStatus& EDP::update_publication_matched_status(
        const GUID_t& reader_guid,
        const GUID_t& writer_guid,
        int change)
{
    std::lock_guard<std::recursive_mutex> _(*mp_PDP->getMutex());

    PublicationMatchedStatus* status;
    auto it = writer_status_.find(writer_guid);
    if (it == writer_status_.end())
    {
        auto pair = writer_status_.emplace(writer_guid, PublicationMatchedStatus{});
        status = &pair.first->second;
    }
    else
    {
        status = &it->second;
    }

    status->current_count = change;
    status->current_count_change = change;
    status->total_count += change;
    status->total_count_change += change;
    status->last_subscription_handle = reader_guid;

    return *status;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
