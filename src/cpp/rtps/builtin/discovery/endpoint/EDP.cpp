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
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/WriterListener.h>
#include <fastdds/rtps/reader/ReaderListener.h>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/attributes/TopicAttributes.h>

#include <fastrtps/utils/StringMatching.h>
#include <fastdds/dds/log/Log.hpp>

#include <fastrtps/types/TypeObjectFactory.h>

#include <rtps/builtin/data/ProxyHashTables.hpp>

#include <mutex>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;
using eprosima::fastdds::dds::PublicationMatchedStatus;
using eprosima::fastdds::dds::SubscriptionMatchedStatus;
using eprosima::fastdds::dds::QosPolicyCount;

namespace eprosima {
namespace fastrtps {
namespace rtps {


EDP::EDP(
        PDP* p,
        RTPSParticipantImpl* part)
    : mp_PDP(p)
    , mp_RTPSParticipant(part)
    , temp_reader_proxy_data_(
        part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators,
        part->getRTPSParticipantAttributes().allocation.data_limits)
    , temp_writer_proxy_data_(
        part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators,
        part->getRTPSParticipantAttributes().allocation.data_limits)
{
}

EDP::~EDP()
{
    // TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(
        RTPSReader* reader,
        const TopicAttributes& att,
        const ReaderQos& rqos)
{
    logInfo(RTPS_EDP, "Adding " << reader->getGuid().entityId << " in topic " << att.topicName);

    auto init_fun = [this, reader, &att, &rqos](
        ReaderProxyData* rpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    logError(RTPS_EDP, "Adding already existent reader " << reader->getGuid().entityId << " in topic "
                                                                         << att.topicName);
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                rpd->isAlive(true);
                rpd->m_expectsInlineQos = reader->expectsInlineQos();
                rpd->guid(reader->getGuid());
                rpd->key() = rpd->guid();
                if (reader->getAttributes().multicastLocatorList.empty() &&
                        reader->getAttributes().unicastLocatorList.empty())
                {
                    rpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    rpd->set_multicast_locators(reader->getAttributes().multicastLocatorList, network);
                    rpd->set_announced_unicast_locators(reader->getAttributes().unicastLocatorList);
                }
                rpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
                rpd->topicName(att.getTopicName());
                rpd->typeName(att.getTopicDataType());
                rpd->topicKind(att.getTopicKind());
                rpd->type_id(att.type_id);
                rpd->type(att.type);
                rpd->type_information(att.type_information);
                rpd->m_qos = rqos;
                rpd->userDefinedId(reader->getAttributes().getUserDefinedID());
#if HAVE_SECURITY
                if (mp_RTPSParticipant->is_secure())
                {
                    rpd->security_attributes_ = reader->getAttributes().security_attributes().mask();
                    rpd->plugin_security_attributes_ =
                            reader->getAttributes().security_attributes().plugin_endpoint_attributes;
                }
                else
                {
                    rpd->security_attributes_ = 0UL;
                    rpd->plugin_security_attributes_ = 0UL;
                }
#endif
                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!rpd->type_information().assigned())
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

                    if (rpd->type_id().m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            rpd->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            rpd->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (rpd->type().m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            rpd->typeName().c_str(), rpd->type_id().m_type_identifier._d() == types::EK_COMPLETE);
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

    //PAIRING
    pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
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
    logInfo(RTPS_EDP, "Adding " << writer->getGuid().entityId << " in topic " << att.topicName);

    auto init_fun = [this, writer, &att, &wqos](
        WriterProxyData* wpd,
        bool updating,
        const ParticipantProxyData& participant_data)
            {
                if (updating)
                {
                    logError(RTPS_EDP, "Adding already existent writer " << writer->getGuid().entityId << " in topic "
                                                                         << att.topicName);
                    return false;
                }

                const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                wpd->guid(writer->getGuid());
                wpd->key() = wpd->guid();
                if (writer->getAttributes().multicastLocatorList.empty() &&
                        writer->getAttributes().unicastLocatorList.empty())
                {
                    wpd->set_locators(participant_data.default_locators);
                }
                else
                {
                    wpd->set_multicast_locators(writer->getAttributes().multicastLocatorList, network);
                    wpd->set_announced_unicast_locators(writer->getAttributes().unicastLocatorList);
                }
                wpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
                wpd->topicName(att.getTopicName());
                wpd->typeName(att.getTopicDataType());
                wpd->topicKind(att.getTopicKind());
                wpd->type_id(att.type_id);
                wpd->type(att.type);
                wpd->type_information(att.type_information);
                wpd->typeMaxSerialized(writer->getTypeMaxSerialized());
                wpd->m_qos = wqos;
                wpd->userDefinedId(writer->getAttributes().getUserDefinedID());
                wpd->persistence_guid(writer->getAttributes().persistence_guid);
#if HAVE_SECURITY
                if (mp_RTPSParticipant->is_secure())
                {
                    wpd->security_attributes_ = writer->getAttributes().security_attributes().mask();
                    wpd->plugin_security_attributes_ =
                            writer->getAttributes().security_attributes().plugin_endpoint_attributes;
                }
                else
                {
                    wpd->security_attributes_ = 0UL;
                    wpd->plugin_security_attributes_ = 0UL;
                }
#endif

                if (att.auto_fill_type_information)
                {
                    // TypeInformation, TypeObject and TypeIdentifier
                    if (!wpd->type_information().assigned())
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

                    if (wpd->type_id().m_type_identifier._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeIdentifier* type_id =
                                types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
                            wpd->typeName().c_str());
                        if (type_id != nullptr)
                        {
                            wpd->type_id().m_type_identifier = *type_id;
                        }
                    }

                    if (wpd->type().m_type_object._d() == static_cast<uint8_t>(0x00))
                    {
                        const types::TypeObject* type_obj =
                                types::TypeObjectFactory::get_instance()->get_type_object(
                            wpd->typeName().c_str(), wpd->type_id().m_type_identifier._d() == types::EK_COMPLETE);
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

    //PAIRING
    pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
    pairingWriter(writer, participant_guid, *writer_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalWriterProxyData(writer, writer_data);
    return true;
}

bool EDP::updatedLocalReader(
        RTPSReader* reader,
        const TopicAttributes& att,
        const ReaderQos& rqos)
{
    auto init_fun = [this, reader, &rqos, &att](
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
        pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
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
        pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
        pairingWriter(writer, participant_guid, *writer_data);
        return true;
    }
    return false;
}

bool EDP::unpairWriterProxy(
        const GUID_t& participant_guid,
        const GUID_t& writer_guid)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, writer_guid);

    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit != mp_RTPSParticipant->userReadersListEnd(); ++rit)
    {
        if ((*rit)->matched_writer_remove(writer_guid))
        {
#if HAVE_SECURITY
            mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(),
                    participant_guid, writer_guid);
#endif

            //MATCHED AND ADDED CORRECTLY:
            if ((*rit)->getListener() != nullptr)
            {
                MatchingInfo info;
                info.status = REMOVED_MATCHING;
                info.remoteEndpointGuid = writer_guid;
                (*rit)->getListener()->onReaderMatched((*rit), info);

                const SubscriptionMatchedStatus& sub_info =
                        update_subscription_matched_status(*(*rit), writer_guid, -1);
                (*rit)->getListener()->onReaderMatched((*rit), sub_info);
            }
        }
    }
    return true;
}

bool EDP::unpairReaderProxy(
        const GUID_t& participant_guid,
        const GUID_t& reader_guid)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, reader_guid);

    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit != mp_RTPSParticipant->userWritersListEnd(); ++wit)
    {
        if ((*wit)->matched_reader_remove(reader_guid))
        {
#if HAVE_SECURITY
            mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(),
                    participant_guid, reader_guid);
#endif
            //MATCHED AND ADDED CORRECTLY:
            if ((*wit)->getListener() != nullptr)
            {
                MatchingInfo info;
                info.status = REMOVED_MATCHING;
                info.remoteEndpointGuid = reader_guid;
                (*wit)->getListener()->onWriterMatched((*wit), info);

                const PublicationMatchedStatus& pub_info =
                        update_publication_matched_status(reader_guid, *(*wit), -1);
                (*wit)->getListener()->onWriterMatched((*wit), pub_info);
            }
        }
    }
    return true;
}

bool EDP::validMatching(
        const WriterProxyData* wdata,
        const ReaderProxyData* rdata)
{
    if (wdata->topicName() != rdata->topicName())
    {
        return false;
    }

    // Type Consistency Enforcement QosPolicy
    if (!checkTypeValidation(wdata, rdata))
    {
        // TODO Trigger INCONSISTENT_TOPIC status change
        return false;
    }
    if (wdata->topicKind() != rdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Reader " << rdata->guid() << " is publishing in topic "
                                                               << rdata->topicName() << "(keyed:" << rdata->topicKind() <<
                "), local writer publishes as keyed: " << wdata->topicKind())
        return false;
    }
    if (!rdata->isAlive()) //Matching
    {
        logWarning(RTPS_EDP, "ReaderProxyData object is NOT alive");
        return false;
    }
    if ( wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS
            && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    //Means our writer is BE but the reader wants RE
    {
        failing_policy_ = wdata->m_qos.m_reliability.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote Reader "
                                                         << rdata->guid() << " is Reliable and local writer is BE ");
        return false;
    }
    if (wdata->m_qos.m_durability.kind < rdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        failing_policy_ = wdata->m_qos.m_durability.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):RemoteReader "
                                                         << rdata->guid() <<
                " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE");
        return false;
    }
    if (wdata->m_qos.m_ownership.kind != rdata->m_qos.m_ownership.kind)
    {
        failing_policy_ = wdata->m_qos.m_ownership.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote reader "
                                                         << rdata->guid() << " has different Ownership Kind");
        return false;
    }
    if (wdata->m_qos.m_deadline.period > rdata->m_qos.m_deadline.period)
    {
        failing_policy_ = wdata->m_qos.m_deadline.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "):Remote reader "
                                                         << rdata->guid() << " has smaller DEADLINE period");
        return false;
    }
    if (!wdata->m_qos.m_disablePositiveACKs.enabled && rdata->m_qos.m_disablePositiveACKs.enabled)
    {
        logWarning(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        return false;
    }
    if (wdata->m_qos.m_liveliness.lease_duration > rdata->m_qos.m_liveliness.lease_duration)
    {
        failing_policy_ = wdata->m_qos.m_liveliness.policy_id_;
        logWarning(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                << wdata->m_qos.m_liveliness.lease_duration << " must be <= requested lease duration "
                << rdata->m_qos.m_liveliness.lease_duration);
        return false;
    }
    if (wdata->m_qos.m_liveliness.kind < rdata->m_qos.m_liveliness.kind)
    {
        failing_policy_ = wdata->m_qos.m_liveliness.policy_id_;
        logWarning(RTPS_EDP, "Incompatible liveliness kinds: offered kind is < requested kind");
        return false;
    }

#if HAVE_SECURITY
    // TODO: Check EndpointSecurityInfo
#endif

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
            if (rnameit->size() == 0)
            {
                matched = true;
                break;
            }
        }
    }
    else if (wdata->m_qos.m_partition.size() > 0 && rdata->m_qos.m_partition.empty() )
    {
        for (auto wnameit = wdata->m_qos.m_partition.begin();
                wnameit !=  wdata->m_qos.m_partition.end(); ++wnameit)
        {
            if (wnameit->size() == 0)
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
        failing_policy_ = wdata->m_qos.m_partition.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << rdata->topicName() << "): Different Partitions");
    }
    return matched;
}

/**
 * @brief EDP::checkDataRepresentationQos
 * Table 7.57 XTypes document 1.2
 * Writer   Reader  Compatible
 * XCDR     XCDR    true
 * XCDR     XCDR2   true
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
        compatible |= std::find(rr.begin(), rr.end(), fastdds::dds::XCDR2_DATA_REPRESENTATION) != rr.end();
        compatible |= std::find(rr.begin(), rr.end(), fastdds::dds::XCDR_DATA_REPRESENTATION) != rr.end() || rr.empty();
    }
    else
    {
        for (DataRepresentationId writerRepresentation : wdata->m_qos.representation.m_value)
        {
            if (writerRepresentation == fastdds::dds::XCDR2_DATA_REPRESENTATION)
            {
                compatible |= std::find(rr.begin(), rr.end(), fastdds::dds::XCDR2_DATA_REPRESENTATION) != rr.end();
            }
            else if (writerRepresentation == fastdds::dds::XCDR_DATA_REPRESENTATION)
            {
                compatible |= std::find(rr.begin(), rr.end(), fastdds::dds::XCDR2_DATA_REPRESENTATION) != rr.end();
                compatible |=
                        std::find(rr.begin(), rr.end(),
                                fastdds::dds::XCDR_DATA_REPRESENTATION) != rr.end() || rr.empty();
            }
            else // XML_DATA_REPRESENTATION
            {
                logInfo(EDP, "DataRepresentationQosPolicy XML_DATA_REPRESENTATION isn't supported.");
            }
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
    if (rdata->topicName() != wdata->topicName())
    {
        return false;
    }

    // Type Consistency Enforcement QosPolicy
    if (!checkTypeValidation(wdata, rdata))
    {
        // TODO Trigger INCONSISTENT_TOPIC status change
        return false;
    }

    if (rdata->topicKind() != wdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Writer " << wdata->guid() <<
                " is publishing in topic " << wdata->topicName() << "(keyed:" << wdata->topicKind() <<
                "), local reader subscribes as keyed: " << rdata->topicKind())
        return false;
    }
    if (rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS
            && wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
    //Means our reader is reliable but hte writer is not
    {
        failing_policy_ = wdata->m_qos.m_reliability.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "): Remote Writer "
                                                         << wdata->guid() << " is Best Effort and local reader is RELIABLE " << endl;
                );
        return false;
    }
    if (rdata->m_qos.m_durability.kind > wdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        failing_policy_ = wdata->m_qos.m_durability.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):RemoteWriter "
                                                         << wdata->guid() << " has VOLATILE DURABILITY and we want TRANSIENT_LOCAL" << endl;
                );
        return false;
    }
    if (rdata->m_qos.m_ownership.kind != wdata->m_qos.m_ownership.kind)
    {
        failing_policy_ = wdata->m_qos.m_ownership.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):Remote Writer "
                                                         << wdata->guid() << " has different Ownership Kind" << endl; );
        return false;
    }
    if (rdata->m_qos.m_deadline.period < wdata->m_qos.m_deadline.period)
    {
        failing_policy_ = wdata->m_qos.m_deadline.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: "
                << wdata->topicName() << "):RemoteWriter "
                << wdata->guid() << "has smaller DEADLINE period");
        return false;
    }
    if (rdata->m_qos.m_disablePositiveACKs.enabled && !wdata->m_qos.m_disablePositiveACKs.enabled)
    {
        logWarning(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        return false;
    }
    if (wdata->m_qos.m_liveliness.lease_duration > rdata->m_qos.m_liveliness.lease_duration)
    {
        failing_policy_ = wdata->m_qos.m_liveliness.policy_id_;
        logWarning(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                << wdata->m_qos.m_liveliness.lease_duration << " must be <= requested lease duration "
                << rdata->m_qos.m_liveliness.lease_duration);
        return false;
    }
    if (wdata->m_qos.m_liveliness.kind < rdata->m_qos.m_liveliness.kind)
    {
        failing_policy_ = wdata->m_qos.m_liveliness.policy_id_;
        logWarning(RTPS_EDP, "Incompatible liveliness kinds: offered kind is < than requested kind");
        return false;
    }
#if HAVE_SECURITY
    // TODO: Check EndpointSecurityInfo
#endif

    //Partition check:
    bool matched = false;
    if (rdata->m_qos.m_partition.empty() && wdata->m_qos.m_partition.empty())
    {
        matched = true;
    }
    else if (rdata->m_qos.m_partition.empty() && wdata->m_qos.m_partition.size() > 0)
    {
        for (auto rnameit = wdata->m_qos.m_partition.begin();
                rnameit != wdata->m_qos.m_partition.end(); ++rnameit)
        {
            if (rnameit->size() == 0)
            {
                matched = true;
                break;
            }
        }
    }
    else if (rdata->m_qos.m_partition.size() > 0 && wdata->m_qos.m_partition.empty() )
    {
        for (auto wnameit = rdata->m_qos.m_partition.begin();
                wnameit !=  rdata->m_qos.m_partition.end(); ++wnameit)
        {
            if (wnameit->size() == 0)
            {
                matched = true;
                break;
            }
        }
    }
    else
    {
        for (auto wnameit = rdata->m_qos.m_partition.begin();
                wnameit !=  rdata->m_qos.m_partition.end(); ++wnameit)
        {
            for (auto rnameit = wdata->m_qos.m_partition.begin();
                    rnameit != wdata->m_qos.m_partition.end(); ++rnameit)
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
        failing_policy_ = wdata->m_qos.m_partition.policy_id_;
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " <<  wdata->topicName() <<
                "): Different Partitions");
    }

    return matched;

}

//TODO Estas cuatro funciones comparten codigo comun (2 a 2) y se podrían seguramente combinar.

bool EDP::pairingReader(
        RTPSReader* R,
        const GUID_t& participant_guid,
        const ReaderProxyData& rdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, rdata.guid() << " in topic: \"" << rdata.topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for (ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit != mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for (auto& pair : *(*pit)->m_writers)
        {
            WriterProxyData* wdatait = pair.second;
            bool valid = validMatching(&rdata, wdatait);
            const GUID_t& writer_guid = wdatait->guid();

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_writer(R->m_guid, (*pit)->m_guid,
                        *wdatait, R->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << R->getGuid());
                }
#else
                if (R->matched_writer_add(*wdatait))
                {
                    logInfo(RTPS_EDP, "Valid Matching to writerProxy: " << writer_guid);
                    //MATCHED AND ADDED CORRECTLY:
                    if (R->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        R->getListener()->onReaderMatched(R, info);

                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(*R, writer_guid, 1);
                        R->getListener()->onReaderMatched(R, sub_info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if (R->matched_writer_is_matched(wdatait->guid())
                        && R->matched_writer_remove(wdatait->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(R->getGuid(), participant_guid,
                            wdatait->guid());
#endif

                    //MATCHED AND ADDED CORRECTLY:
                    if (R->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        R->getListener()->onReaderMatched(R, info);

                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(*R, writer_guid, -1);
                        R->getListener()->onReaderMatched(R, sub_info);
                    }
                }
                if (update_requested_incompatible_qos_status(*R))
                {
                    R->getListener()->on_requested_incompatible_qos(R, R->requested_incompatible_qos_status_);
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

    logInfo(RTPS_EDP, W->getGuid() << " in topic: \"" << wdata.topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for (ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit != mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for (auto& pair : *(*pit)->m_readers)
        {
            ReaderProxyData* rdatait = pair.second;
            const GUID_t& reader_guid = rdatait->guid();
            if (reader_guid == c_Guid_Unknown)
            {
                continue;
            }

            bool valid = validMatching(&wdata, rdatait);

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_reader(W->getGuid(), (*pit)->m_guid,
                        *rdatait, W->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << W->getGuid());
                }
#else
                if (W->matched_reader_add(*rdatait))
                {
                    logInfo(RTPS_EDP, "Valid Matching to readerProxy: " << reader_guid);
                    //MATCHED AND ADDED CORRECTLY:
                    if (W->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W, info);

                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, *W, 1);
                        W->getListener()->onWriterMatched(W, pub_info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if (W->matched_reader_is_matched(reader_guid) && W->matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(W->getGuid(), participant_guid, reader_guid);
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if (W->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W, info);

                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, *W, -1);
                        W->getListener()->onWriterMatched(W, pub_info);

                    }
                }
                if (update_offered_incompatible_qos_status(*W))
                {
                    W->getListener()->on_offered_incompatible_qos(W, W->offered_incompatible_qos_status_);
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

    logInfo(RTPS_EDP, rdata->guid() << " in topic: \"" << rdata->topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit != mp_RTPSParticipant->userWritersListEnd(); ++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();
        if (mp_PDP->lookupWriterProxyData(writerGUID, temp_writer_proxy_data_))
        {
            bool valid = validMatching(&temp_writer_proxy_data_, rdata);
            const GUID_t& reader_guid = rdata->guid();

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID, participant_guid,
                        *rdata, (*wit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                }
#else
                if ((*wit)->matched_reader_add(*rdata))
                {
                    logInfo(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);
                    //MATCHED AND ADDED CORRECTLY:
                    if ((*wit)->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        (*wit)->getListener()->onWriterMatched((*wit), info);

                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, *(*wit), 1);
                        (*wit)->getListener()->onWriterMatched((*wit), pub_info);
                    }
                }
#endif
            }
            else
            {
                if ((*wit)->matched_reader_is_matched(reader_guid)
                        && (*wit)->matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(
                        (*wit)->getGuid(), participant_guid, reader_guid);
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if ((*wit)->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        (*wit)->getListener()->onWriterMatched((*wit), info);

                        const PublicationMatchedStatus& pub_info =
                                update_publication_matched_status(reader_guid, *(*wit), -1);
                        (*wit)->getListener()->onWriterMatched((*wit), pub_info);
                    }
                }
                if (update_offered_incompatible_qos_status(*(*wit)))
                {
                    (*wit)->getListener()->on_offered_incompatible_qos((*wit),
                            (*wit)->offered_incompatible_qos_status_);
                }
            }
        }
    }

    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_reader_proxy_with_local_writer(
        const GUID_t& local_writer,
        const GUID_t& remote_participant_guid,
        ReaderProxyData& rdata)
{
    logInfo(RTPS_EDP, rdata.guid() << " in topic: \"" << rdata.topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit != mp_RTPSParticipant->userWritersListEnd(); ++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();
        const GUID_t& reader_guid = rdata.guid();

        if (local_writer == writerGUID)
        {
            if (mp_PDP->lookupWriterProxyData(writerGUID, temp_writer_proxy_data_))
            {
                bool valid = validMatching(&temp_writer_proxy_data_, &rdata);

                if (valid)
                {
                    if (!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID,
                            remote_participant_guid, rdata, (*wit)->getAttributes().security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                    }
                }
                else
                {
                    if ((*wit)->matched_reader_is_matched(reader_guid)
                            && (*wit)->matched_reader_remove(reader_guid))
                    {
                        mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(),
                                remote_participant_guid, reader_guid);
                        //MATCHED AND ADDED CORRECTLY:
                        if ((*wit)->getListener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = REMOVED_MATCHING;
                            info.remoteEndpointGuid = reader_guid;
                            (*wit)->getListener()->onWriterMatched((*wit), info);

                            const PublicationMatchedStatus& pub_info =
                                    update_publication_matched_status(reader_guid, *(*wit), -1);
                            (*wit)->getListener()->onWriterMatched((*wit), pub_info);
                        }
                    }
                    if (update_offered_incompatible_qos_status(*(*wit)))
                    {
                        (*wit)->getListener()->on_offered_incompatible_qos((*wit),
                                (*wit)->offered_incompatible_qos_status_);
                    }
                }
            }
        }
    }

    return true;
}

bool EDP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit != mp_RTPSParticipant->userWritersListEnd(); ++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();
        const GUID_t& reader_guid = remote_reader_data.guid();

        if (local_writer == writerGUID)
        {
            if ((*wit)->matched_reader_add(remote_reader_data))
            {
                logInfo(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);
                //MATCHED AND ADDED CORRECTLY:
                if ((*wit)->getListener() != nullptr)
                {
                    MatchingInfo info;
                    info.status = MATCHED_MATCHING;
                    info.remoteEndpointGuid = reader_guid;
                    (*wit)->getListener()->onWriterMatched((*wit), info);


                    const PublicationMatchedStatus& pub_info =
                            update_publication_matched_status(reader_guid, *(*wit), 1);
                    (*wit)->getListener()->onWriterMatched((*wit), pub_info);
                }

                return true;
            }

            return false;
        }
    }

    return pairing_remote_reader_with_local_builtin_writer_after_security(local_writer, remote_reader_data);
}

#endif

bool EDP::pairing_writer_proxy_with_any_local_reader(
        const GUID_t& participant_guid,
        WriterProxyData* wdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, wdata->guid() << " in topic: \"" << wdata->topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit != mp_RTPSParticipant->userReadersListEnd(); ++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();
        if (mp_PDP->lookupReaderProxyData(readerGUID, temp_reader_proxy_data_))
        {
            bool valid = validMatching(&temp_reader_proxy_data_, wdata);
            const GUID_t& writer_guid = wdata->guid();

            if (valid)
            {
#if HAVE_SECURITY
                if (!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID, participant_guid,
                        *wdata, (*rit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                }
#else
                if ((*rit)->matched_writer_add(*wdata))
                {
                    logInfo(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);
                    //MATCHED AND ADDED CORRECTLY:
                    if ((*rit)->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        (*rit)->getListener()->onReaderMatched((*rit), info);


                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(*(*rit), writer_guid, 1);
                        (*rit)->getListener()->onReaderMatched((*rit), sub_info);
                    }
                }
#endif
            }
            else
            {
                if ((*rit)->matched_writer_is_matched(writer_guid)
                        && (*rit)->matched_writer_remove(writer_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(readerGUID, participant_guid, writer_guid);
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if ((*rit)->getListener() != nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = writer_guid;
                        (*rit)->getListener()->onReaderMatched((*rit), info);

                        const SubscriptionMatchedStatus& sub_info =
                                update_subscription_matched_status(*(*rit), writer_guid, -1);
                        (*rit)->getListener()->onReaderMatched((*rit), sub_info);
                    }
                }
                if (update_requested_incompatible_qos_status(*(*rit)))
                {
                    (*rit)->getListener()->on_requested_incompatible_qos((*rit),
                            (*rit)->requested_incompatible_qos_status_);
                }
            }
        }
    }
    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_writer_proxy_with_local_reader(
        const GUID_t& local_reader,
        const GUID_t& remote_participant_guid,
        WriterProxyData& wdata)
{
    logInfo(RTPS_EDP, wdata.guid() << " in topic: \"" << wdata.topicName() << "\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit != mp_RTPSParticipant->userReadersListEnd(); ++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();

        if (local_reader == readerGUID)
        {
            if (mp_PDP->lookupReaderProxyData(readerGUID, temp_reader_proxy_data_))
            {
                bool valid = validMatching(&temp_reader_proxy_data_, &wdata);
                const GUID_t& writer_guid = wdata.guid();

                if (valid)
                {
                    if (!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID,
                            remote_participant_guid, wdata, (*rit)->getAttributes().security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                    }
                }
                else
                {
                    if ((*rit)->matched_writer_is_matched(writer_guid)
                            && (*rit)->matched_writer_remove(writer_guid))
                    {
                        mp_RTPSParticipant->security_manager().remove_writer(readerGUID,
                                remote_participant_guid, writer_guid);
                        //MATCHED AND ADDED CORRECTLY:
                        if ((*rit)->getListener() != nullptr)
                        {
                            MatchingInfo info;
                            info.status = REMOVED_MATCHING;
                            info.remoteEndpointGuid = writer_guid;
                            (*rit)->getListener()->onReaderMatched((*rit), info);

                            const SubscriptionMatchedStatus& sub_info =
                                    update_subscription_matched_status(*(*rit), writer_guid, -1);
                            (*rit)->getListener()->onReaderMatched((*rit), sub_info);
                        }
                    }
                    if (update_requested_incompatible_qos_status(*(*rit)))
                    {
                        (*rit)->getListener()->on_requested_incompatible_qos((*rit),
                                (*rit)->requested_incompatible_qos_status_);
                    }
                }
            }
        }
    }
    return true;
}

bool EDP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for (std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit != mp_RTPSParticipant->userReadersListEnd(); ++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();
        const GUID_t& writer_guid = remote_writer_data.guid();

        if (local_reader == readerGUID)
        {
            // TODO(richiware) Implement and use move with attributes
            if ((*rit)->matched_writer_add(remote_writer_data))
            {
                logInfo(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);
                //MATCHED AND ADDED CORRECTLY:
                if ((*rit)->getListener() != nullptr)
                {
                    MatchingInfo info;
                    info.status = MATCHED_MATCHING;
                    info.remoteEndpointGuid = writer_guid;
                    (*rit)->getListener()->onReaderMatched((*rit), info);

                    const SubscriptionMatchedStatus& sub_info =
                            update_subscription_matched_status(*(*rit), writer_guid, 1);
                    (*rit)->getListener()->onReaderMatched((*rit), sub_info);

                }

                return true;
            }

            return false;
        }
    }

    return pairing_remote_writer_with_local_builtin_reader_after_security(local_reader, remote_writer_data);
}

#endif

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
        RTPSReader& reader,
        const GUID_t& writer_guid,
        int change)
{
    reader.subscription_matched_status_.current_count = change;
    reader.subscription_matched_status_.current_count_change = change;
    reader.subscription_matched_status_.total_count += change;
    reader.subscription_matched_status_.total_count_change += change;
    reader.subscription_matched_status_.last_publication_handle = writer_guid;

    return reader.subscription_matched_status_;
}

const fastdds::dds::PublicationMatchedStatus& EDP::update_publication_matched_status(
        const GUID_t& reader_guid,
        RTPSWriter& writer,
        int change)
{
    writer.publication_matched_status_.current_count = change;
    writer.publication_matched_status_.current_count_change = change;
    writer.publication_matched_status_.total_count += change;
    writer.publication_matched_status_.total_count_change += change;
    writer.publication_matched_status_.last_subscription_handle = reader_guid;


    return writer.publication_matched_status_;
}

bool EDP::update_offered_incompatible_qos_status(
        RTPSWriter& W)
{
    if (failing_policy_ != 0)
    {
        W.offered_incompatible_qos_status_.last_policy_id = failing_policy_;
        W.offered_incompatible_qos_status_.total_count++;
        W.offered_incompatible_qos_status_.total_count_change++;
        auto it = std::find_if(W.offered_incompatible_qos_status_.policies.begin(),
                        W.offered_incompatible_qos_status_.policies.end(),
                        [this] (const QosPolicyCount& policy) -> bool
                    {
                        return policy.policy_id == failing_policy_;
                    });
        if (it != W.offered_incompatible_qos_status_.policies.end())
        {
            it->count++;
        }
        else
        {
            QosPolicyCount policy(failing_policy_, 1);
            W.offered_incompatible_qos_status_.policies.push_back(policy);
        }
        failing_policy_ = 0;
        return true;
    }
    return false;
}

bool EDP::update_requested_incompatible_qos_status(
        RTPSReader& R)
{
    if (failing_policy_ != 0)
    {
        R.requested_incompatible_qos_status_.last_policy_id = failing_policy_;
        R.requested_incompatible_qos_status_.total_count++;
        R.requested_incompatible_qos_status_.total_count_change++;
        auto it = std::find_if(R.requested_incompatible_qos_status_.policies.begin(),
                        R.requested_incompatible_qos_status_.policies.end(),
                        [this] (const QosPolicyCount& policy) -> bool
                    {
                        return policy.policy_id == failing_policy_;
                    });
        if (it != R.requested_incompatible_qos_status_.policies.end())
        {
            it->count++;
        }
        else
        {
            QosPolicyCount policy(failing_policy_, 1);
            R.requested_incompatible_qos_status_.policies.push_back(policy);
        }
        failing_policy_ = 0;
        return true;
    }
    return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
