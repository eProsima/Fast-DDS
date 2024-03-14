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

#include <fastdds/rtps/Endpoint.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <rtps/participant/RTPSParticipantImpl.h>

namespace eprosima {
namespace fastrtps {
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

bool RTPSParticipant::registerWriter(
        RTPSWriter* Writer,
        const TopicAttributes& topicAtt,
        const WriterQos& wqos)
{
    return mp_impl->registerWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipant::registerReader(
        RTPSReader* Reader,
        const TopicAttributes& topicAtt,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    return mp_impl->registerReader(Reader, topicAtt, rqos, content_filter);
}

void RTPSParticipant::update_attributes(
        const RTPSParticipantAttributes& patt)
{
    mp_impl->update_attributes(patt);
}

bool RTPSParticipant::updateWriter(
        RTPSWriter* Writer,
        const TopicAttributes& topicAtt,
        const WriterQos& wqos)
{
    return mp_impl->updateLocalWriter(Writer, topicAtt, wqos);
}

bool RTPSParticipant::updateReader(
        RTPSReader* Reader,
        const TopicAttributes& topicAtt,
        const ReaderQos& rqos,
        const fastdds::rtps::ContentFilterProperty* content_filter)
{
    return mp_impl->updateLocalReader(Reader, topicAtt, rqos, content_filter);
}

std::vector<std::string> RTPSParticipant::getParticipantNames() const
{
    return mp_impl->getParticipantNames();
}

const RTPSParticipantAttributes& RTPSParticipant::getRTPSParticipantAttributes() const
{
    return mp_impl->getRTPSParticipantAttributes();
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

fastdds::dds::builtin::TypeLookupManager* RTPSParticipant::typelookup_manager() const
{
    return mp_impl->typelookup_manager();
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

std::vector<fastdds::rtps::TransportNetmaskFilterInfo> RTPSParticipant::get_netmask_filter_info() const
{
    return mp_impl->get_netmask_filter_info();
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
        fastrtps::rtps::ParticipantProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        fastrtps::rtps::WriterProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

bool RTPSParticipant::fill_discovery_data_from_cdr_message(
        fastrtps::rtps::ReaderProxyData& data,
        fastdds::statistics::MonitorServiceStatusData& msg)
{
    return mp_impl->fill_discovery_data_from_cdr_message(data, msg);
}

#endif // FASTDDS_STATISTICS

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

