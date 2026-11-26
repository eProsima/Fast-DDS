// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of RTPSParticipant: replaces src/cpp/rtps/participant/RTPSParticipant.cpp and forwards
// every method to the RTPSParticipantMock companion of the instance.

#include <fastdds/rtps/participant/RTPSParticipant.hpp>

#include <stdexcept>
#include <utility>

#include <mock/fastdds/rtps/participant/RTPSParticipantMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<RTPSParticipant>;

RTPSParticipant::RTPSParticipant(
        RTPSParticipantImpl* pimpl)
    : mp_impl(pimpl)
{
    Companion::attach(this);
}

RTPSParticipant::~RTPSParticipant()
{
    Companion::detach(this);
}

const GUID_t& RTPSParticipant::getGuid() const
{
    return Companion::get(this).getGuid();
}

void RTPSParticipant::announceRTPSParticipantState()
{
    Companion::get(this).announceRTPSParticipantState();
}

void RTPSParticipant::stopRTPSParticipantAnnouncement()
{
    Companion::get(this).stopRTPSParticipantAnnouncement();
}

void RTPSParticipant::resetRTPSParticipantAnnouncement()
{
    Companion::get(this).resetRTPSParticipantAnnouncement();
}

bool RTPSParticipant::newRemoteWriterDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId)
{
    return Companion::get(this).newRemoteWriterDiscovered(pguid, userDefinedId);
}

bool RTPSParticipant::newRemoteReaderDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId)
{
    return Companion::get(this).newRemoteReaderDiscovered(pguid, userDefinedId);
}

uint32_t RTPSParticipant::getRTPSParticipantID() const
{
    return Companion::get(this).getRTPSParticipantID();
}

bool RTPSParticipant::register_writer(
        RTPSWriter* rtps_writer,
        const TopicDescription& topic,
        const fastdds::dds::WriterQos& qos)
{
    return Companion::get(this).register_writer(rtps_writer, topic, qos);
}

dds::ReturnCode_t RTPSParticipant::register_writer(
        RTPSWriter* rtps_writer,
        const TopicDescription& topic,
        const PublicationBuiltinTopicData& pub_builtin_topic_data)
{
    return Companion::get(this).register_writer(rtps_writer, topic, pub_builtin_topic_data);
}

bool RTPSParticipant::register_reader(
        RTPSReader* rtps_reader,
        const TopicDescription& topic,
        const fastdds::dds::ReaderQos& qos,
        const ContentFilterProperty* content_filter)
{
    return Companion::get(this).register_reader(rtps_reader, topic, qos, content_filter);
}

dds::ReturnCode_t RTPSParticipant::register_reader(
        RTPSReader* rtps_reader,
        const TopicDescription& topic,
        const SubscriptionBuiltinTopicData& sub_builtin_topic_data,
        const ContentFilterProperty* content_filter)
{
    return Companion::get(this).register_reader(rtps_reader, topic, sub_builtin_topic_data, content_filter);
}

void RTPSParticipant::update_attributes(
        const RTPSParticipantAttributes& patt)
{
    Companion::get(this).update_attributes(patt);
}

bool RTPSParticipant::update_writer(
        RTPSWriter* rtps_writer,
        const fastdds::dds::WriterQos& wqos)
{
    return Companion::get(this).update_writer(rtps_writer, wqos);
}

bool RTPSParticipant::update_reader(
        RTPSReader* rtps_reader,
        const fastdds::dds::ReaderQos& rqos,
        const ContentFilterProperty* content_filter)
{
    return Companion::get(this).update_reader(rtps_reader, rqos, content_filter);
}

std::vector<std::string> RTPSParticipant::getParticipantNames() const
{
    return Companion::get(this).getParticipantNames();
}

const RTPSParticipantAttributes& RTPSParticipant::get_attributes() const
{
    return Companion::get(this).get_attributes();
}

const RTPSParticipantConstantAttributes& RTPSParticipant::get_const_attributes() const
{
    return Companion::get(this).get_const_attributes();
}

const RTPSParticipantMutableAttributes RTPSParticipant::get_mutable_attributes() const
{
    return Companion::get(this).get_mutable_attributes();
}

RTPSParticipantAttributes RTPSParticipant::copy_attributes() const
{
    return Companion::get(this).copy_attributes();
}

uint32_t RTPSParticipant::getMaxMessageSize() const
{
    return Companion::get(this).getMaxMessageSize();
}

uint32_t RTPSParticipant::getMaxDataSize() const
{
    return Companion::get(this).getMaxDataSize();
}

ResourceEvent& RTPSParticipant::get_resource_event() const
{
    ResourceEvent* event = Companion::get(this).get_resource_event();
    if (nullptr == event)
    {
        throw std::logic_error("No ResourceEvent set on the mock companion of this RTPSParticipant");
    }
    return *event;
}

WLP* RTPSParticipant::wlp() const
{
    return Companion::get(this).wlp();
}

bool RTPSParticipant::get_new_entity_id(
        EntityId_t& entityId)
{
    return Companion::get(this).get_new_entity_id(entityId);
}

void RTPSParticipant::set_check_type_function(
        std::function<bool(const std::string&)>&& check_type)
{
    Companion::get(this).set_check_type_function(std::move(check_type));
}

void RTPSParticipant::set_listener(
        RTPSParticipantListener* listener)
{
    Companion::get(this).set_listener(listener);
}

uint32_t RTPSParticipant::get_domain_id() const
{
    return Companion::get(this).get_domain_id();
}

void RTPSParticipant::enable()
{
    Companion::get(this).enable();
}

bool RTPSParticipant::ignore_participant(
        const GuidPrefix_t& participant_guid)
{
    return Companion::get(this).ignore_participant(participant_guid);
}

bool RTPSParticipant::ignore_writer(
        const GUID_t& writer_guid)
{
    return Companion::get(this).ignore_writer(writer_guid);
}

bool RTPSParticipant::ignore_reader(
        const GUID_t& reader_guid)
{
    return Companion::get(this).ignore_reader(reader_guid);
}

std::vector<TransportNetmaskFilterInfo> RTPSParticipant::get_netmask_filter_info() const
{
    return Companion::get(this).get_netmask_filter_info();
}

bool RTPSParticipant::get_publication_info(
        PublicationBuiltinTopicData& data,
        const GUID_t& writer_guid) const
{
    return Companion::get(this).get_publication_info(data, writer_guid);
}

bool RTPSParticipant::get_subscription_info(
        SubscriptionBuiltinTopicData& data,
        const GUID_t& reader_guid) const
{
    return Companion::get(this).get_subscription_info(data, reader_guid);
}

#if HAVE_SECURITY

bool RTPSParticipant::is_security_enabled_for_writer(
        const WriterAttributes& writer_attributes)
{
    return Companion::get(this).is_security_enabled_for_writer(writer_attributes);
}

bool RTPSParticipant::is_security_enabled_for_reader(
        const ReaderAttributes& reader_attributes)
{
    return Companion::get(this).is_security_enabled_for_reader(reader_attributes);
}

#endif // if HAVE_SECURITY

bool RTPSParticipant::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    return Companion::get(this).add_statistics_listener(listener, kind);
}

bool RTPSParticipant::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener,
        uint32_t kind)
{
    return Companion::get(this).remove_statistics_listener(listener, kind);
}

void RTPSParticipant::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    Companion::get(this).set_enabled_statistics_writers_mask(enabled_writers);
}

const fastdds::statistics::rtps::IStatusObserver* RTPSParticipant::create_monitor_service(
        fastdds::statistics::rtps::IStatusQueryable& status_queryable)
{
    return Companion::get(this).create_monitor_service(status_queryable);
}

bool RTPSParticipant::create_monitor_service()
{
    return Companion::get(this).create_monitor_service();
}

bool RTPSParticipant::is_monitor_service_created() const
{
    return Companion::get(this).is_monitor_service_created();
}

bool RTPSParticipant::enable_monitor_service() const
{
    return Companion::get(this).enable_monitor_service();
}

bool RTPSParticipant::disable_monitor_service() const
{
    return Companion::get(this).disable_monitor_service();
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        ParticipantBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return Companion::get(this).fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        PublicationBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return Companion::get(this).fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        SubscriptionBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    return Companion::get(this).fill_discovery_data_from_cdr_message(data, msg);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
