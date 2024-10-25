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
 * @file RTPSParticipant.cpp
 *
 */

#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSParticipant::RTPSParticipant(
        RTPSParticipantImpl* pimpl)
    : mp_impl(pimpl)
{
}

RTPSParticipant::~RTPSParticipant()
{
    mp_impl = nullptr;
}

const GUID_t& RTPSParticipant::getGuid() const
{
    return mp_impl->getGuid();
}

void RTPSParticipant::announceRTPSParticipantState()
{
    return mp_impl->announceRTPSParticipantState();
}

void RTPSParticipant::stopRTPSParticipantAnnouncement()
{
    return mp_impl->stopRTPSParticipantAnnouncement();
}

void RTPSParticipant::resetRTPSParticipantAnnouncement()
{
    return mp_impl->resetRTPSParticipantAnnouncement();
}

bool RTPSParticipant::newRemoteWriterDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId)
{
    return mp_impl->newRemoteEndpointDiscovered(pguid, userDefinedId, WRITER);
}

bool RTPSParticipant::newRemoteReaderDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId)
{
    return mp_impl->newRemoteEndpointDiscovered(pguid, userDefinedId, READER);
}

uint32_t RTPSParticipant::getRTPSParticipantID() const
{
    return mp_impl->getRTPSParticipantID();
}

bool RTPSParticipant::register_writer(
        RTPSWriter* writer,
        const TopicDescription& topic,
        const fastdds::dds::WriterQos& qos)
{
    return mp_impl->register_writer(writer, topic, qos);
}

bool RTPSParticipant::register_reader(
        RTPSReader* reader,
        const TopicDescription& topic,
        const fastdds::dds::ReaderQos& qos,
        const ContentFilterProperty* content_filter)
{
    return mp_impl->register_reader(reader, topic, qos, content_filter);
}

void RTPSParticipant::update_attributes(
        const RTPSParticipantAttributes& patt)
{
    mp_impl->update_attributes(patt);
}

bool RTPSParticipant::update_writer(
        RTPSWriter* writer,
        const fastdds::dds::WriterQos& wqos)
{
    return mp_impl->update_writer(writer, wqos);
}

bool RTPSParticipant::update_reader(
        RTPSReader* reader,
        const fastdds::dds::ReaderQos& rqos,
        const ContentFilterProperty* content_filter)
{
    return mp_impl->update_reader(reader, rqos, content_filter);
}

std::vector<std::string> RTPSParticipant::getParticipantNames() const
{
    return mp_impl->getParticipantNames();
}

const RTPSParticipantAttributes& RTPSParticipant::get_attributes() const
{
    return mp_impl->get_attributes();
}

uint32_t RTPSParticipant::getMaxMessageSize() const
{
    return mp_impl->getMaxMessageSize();
}

uint32_t RTPSParticipant::getMaxDataSize() const
{
    return mp_impl->getMaxDataSize();
}

ResourceEvent& RTPSParticipant::get_resource_event() const
{
    return mp_impl->getEventResource();
}

WLP* RTPSParticipant::wlp() const
{
    return mp_impl->wlp();
}

bool RTPSParticipant::get_new_entity_id(
        EntityId_t& entityId)
{
    return mp_impl->get_new_entity_id(entityId);
}

void RTPSParticipant::set_check_type_function(
        std::function<bool(const std::string&)>&& check_type)
{
    mp_impl->set_check_type_function(std::move(check_type));
}

void RTPSParticipant::set_listener(
        RTPSParticipantListener* listener)
{
    mp_impl->set_listener(listener);
}

uint32_t RTPSParticipant::get_domain_id() const
{
    return mp_impl->get_domain_id();
}

void RTPSParticipant::enable()
{
    mp_impl->enable();
}

bool RTPSParticipant::ignore_participant(
        const GuidPrefix_t& participant_guid)
{
    return mp_impl->ignore_participant(participant_guid);
}

bool RTPSParticipant::ignore_writer(
        const GUID_t& /*writer_guid*/)
{
    return false;
}

bool RTPSParticipant::ignore_reader(
        const GUID_t& /*reader_guid*/)
{
    return false;
}

std::vector<TransportNetmaskFilterInfo> RTPSParticipant::get_netmask_filter_info() const
{
    return mp_impl->get_netmask_filter_info();
}

bool RTPSParticipant::get_publication_info(
        PublicationBuiltinTopicData& data,
        const GUID_t& writer_guid) const
{
    return mp_impl->get_publication_info(data, writer_guid);
}

bool RTPSParticipant::get_subscription_info(
        SubscriptionBuiltinTopicData& data,
        const GUID_t& reader_guid) const
{
    return mp_impl->get_subscription_info(data, reader_guid);
}

#if HAVE_SECURITY

bool RTPSParticipant::is_security_enabled_for_writer(
        const WriterAttributes& writer_attributes)
{
    return mp_impl->is_security_enabled_for_writer(writer_attributes);
}

bool RTPSParticipant::is_security_enabled_for_reader(
        const ReaderAttributes& reader_attributes)
{
    return mp_impl->is_security_enabled_for_reader(reader_attributes);
}

#endif // if HAVE_SECURITY

#ifdef FASTDDS_STATISTICS

bool RTPSParticipant::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    return mp_impl->add_statistics_listener(listener, kind);
}

bool RTPSParticipant::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    return mp_impl->remove_statistics_listener(listener, kind);
}

void RTPSParticipant::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    mp_impl->set_enabled_statistics_writers_mask(enabled_writers);
}

const fastdds::statistics::rtps::IStatusObserver* RTPSParticipant::create_monitor_service(
        fastdds::statistics::rtps::IStatusQueryable& sq)
{
    return mp_impl->create_monitor_service(sq);
}

bool RTPSParticipant::create_monitor_service()
{
    return mp_impl->create_monitor_service();
}

bool RTPSParticipant::is_monitor_service_created() const
{
    return mp_impl->is_monitor_service_created();
}

bool RTPSParticipant::enable_monitor_service() const
{
    return mp_impl->enable_monitor_service();
}

bool RTPSParticipant::disable_monitor_service() const
{
    return mp_impl->disable_monitor_service();
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        ParticipantBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        PublicationBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        SubscriptionBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

#endif // FASTDDS_STATISTICS

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

