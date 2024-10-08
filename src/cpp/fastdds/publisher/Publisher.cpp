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
 * @file Publisher.cpp
 *
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

Publisher::Publisher(
        PublisherImpl* p,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(p)
{
}

Publisher::Publisher(
        DomainParticipant* dp,
        const PublisherQos& qos,
        PublisherListener* listener,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(dp->create_publisher(qos, listener, mask)->impl_)
{
}

Publisher::~Publisher()
{
}

ReturnCode_t Publisher::enable()
{
    if (enable_)
    {
        return RETCODE_OK;
    }

    if (false == impl_->get_participant()->is_enabled())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    enable_ = true;
    ReturnCode_t ret_code = impl_->enable();
    enable_ = RETCODE_OK == ret_code;
    return ret_code;
}

const PublisherQos& Publisher::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Publisher::get_qos(
        PublisherQos& qos) const
{
    qos = impl_->get_qos();
    return RETCODE_OK;
}

ReturnCode_t Publisher::set_qos(
        const PublisherQos& qos)
{
    return impl_->set_qos(qos);
}

const PublisherListener* Publisher::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t Publisher::set_listener(
        PublisherListener* listener)
{
    return set_listener(listener, StatusMask::all());
}

ReturnCode_t Publisher::set_listener(
        PublisherListener* listener,
        const StatusMask& mask)
{
    ReturnCode_t ret_val = impl_->set_listener(listener);
    if (ret_val == RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

DataWriter* Publisher::create_datawriter(
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listener,
        const StatusMask& mask,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
{
    return impl_->create_datawriter(topic, qos, listener, mask, payload_pool);
}

DataWriter* Publisher::create_datawriter_with_profile(
        Topic* topic,
        const std::string& profile_name,
        DataWriterListener* listener,
        const StatusMask& mask,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
{
    return impl_->create_datawriter_with_profile(topic, profile_name, listener, mask, payload_pool);
}

ReturnCode_t Publisher::delete_datawriter(
        const DataWriter* writer)
{
    return impl_->delete_datawriter(writer);
}

DataWriter* Publisher::lookup_datawriter(
        const std::string& topic_name) const
{
    return impl_->lookup_datawriter(topic_name);
}

ReturnCode_t Publisher::suspend_publications()
{
    return RETCODE_UNSUPPORTED;
    /*
       return impl_->suspend_publications();
     */
}

ReturnCode_t Publisher::resume_publications()
{
    return RETCODE_UNSUPPORTED;
    /*
       return impl_->resume_publications();
     */
}

ReturnCode_t Publisher::begin_coherent_changes()
{
    return RETCODE_UNSUPPORTED;
    /*
       return impl_->begin_coherent_changes();
     */
}

ReturnCode_t Publisher::end_coherent_changes()
{
    return RETCODE_UNSUPPORTED;
    /*
       return impl_->end_coherent_changes();
     */
}

ReturnCode_t Publisher::wait_for_acknowledgments(
        const fastdds::dds::Duration_t& max_wait)
{
    return impl_->wait_for_acknowledgments(max_wait);
}

const DomainParticipant* Publisher::get_participant() const
{
    return impl_->get_participant();
}

ReturnCode_t Publisher::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}

ReturnCode_t Publisher::set_default_datawriter_qos(
        const DataWriterQos& qos)
{
    return impl_->set_default_datawriter_qos(qos);
}

const DataWriterQos& Publisher::get_default_datawriter_qos() const
{
    return impl_->get_default_datawriter_qos();
}

ReturnCode_t Publisher::get_default_datawriter_qos(
        DataWriterQos& qos) const
{
    qos = impl_->get_default_datawriter_qos();
    return RETCODE_OK;
}

ReturnCode_t Publisher::copy_from_topic_qos(
        fastdds::dds::DataWriterQos& writer_qos,
        const fastdds::dds::TopicQos& topic_qos)
{
    return PublisherImpl::copy_from_topic_qos(writer_qos, topic_qos);
}

const fastdds::rtps::InstanceHandle_t& Publisher::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

bool Publisher::get_datawriters(
        std::vector<DataWriter*>& writers) const
{
    return impl_->get_datawriters(writers);
}

bool Publisher::has_datawriters() const
{
    return impl_->has_datawriters();
}

ReturnCode_t Publisher::get_datawriter_qos_from_profile(
        const std::string& profile_name,
        DataWriterQos& qos) const
{
    return impl_->get_datawriter_qos_from_profile(profile_name, qos);
}

ReturnCode_t Publisher::get_datawriter_qos_from_profile(
        const std::string& profile_name,
        DataWriterQos& qos,
        std::string& topic_name) const
{
    return impl_->get_datawriter_qos_from_profile(profile_name, qos, topic_name);
}

ReturnCode_t Publisher::get_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos) const
{
    return impl_->get_datawriter_qos_from_xml(xml, qos);
}

ReturnCode_t Publisher::get_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos,
        std::string& topic_name) const
{
    return impl_->get_datawriter_qos_from_xml(xml, qos, topic_name);
}

ReturnCode_t Publisher::get_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos,
        const std::string& profile_name) const
{
    return impl_->get_datawriter_qos_from_xml(xml, qos, profile_name);
}

ReturnCode_t Publisher::get_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos,
        std::string& topic_name,
        const std::string& profile_name) const
{
    return impl_->get_datawriter_qos_from_xml(xml, qos, topic_name, profile_name);
}

ReturnCode_t Publisher::get_default_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos) const
{
    return impl_->get_default_datawriter_qos_from_xml(xml, qos);
}

ReturnCode_t Publisher::get_default_datawriter_qos_from_xml(
        const std::string& xml,
        DataWriterQos& qos,
        std::string& topic_name) const
{
    return impl_->get_default_datawriter_qos_from_xml(xml, qos, topic_name);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
