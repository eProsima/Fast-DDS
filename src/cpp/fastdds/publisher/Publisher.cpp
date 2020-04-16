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

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>

#include <fastdds/dds/log/Log.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;

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

const PublisherQos& Publisher::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Publisher::get_qos(
        PublisherQos& qos) const
{
    qos = impl_->get_qos();
    return ReturnCode_t::RETCODE_OK;
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
    return impl_->set_listener(listener);
}

DataWriter* Publisher::create_datawriter(
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listener,
        const StatusMask& mask)
{
    return impl_->create_datawriter(topic, qos, listener, mask);
}

ReturnCode_t Publisher::delete_datawriter(
        DataWriter* writer)
{
    return impl_->delete_datawriter(writer);
}

DataWriter* Publisher::lookup_datawriter(
        const std::string& topic_name) const
{
    return impl_->lookup_datawriter(topic_name);
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

/* TODO
   bool Publisher::suspend_publications()
   {
    return impl_->suspend_publications();
   }
 */

/* TODO
   bool Publisher::resume_publications()
   {
    return impl_->resume_publications();
   }
 */

/* TODO
   bool Publisher::begin_coherent_changes()
   {
    return impl_->begin_coherent_changes();
   }
 */

/* TODO
   bool Publisher::end_coherent_changes()
   {
    return impl_->end_coherent_changes();
   }
 */

ReturnCode_t Publisher::wait_for_acknowledgments(
        const fastrtps::Duration_t& max_wait)
{
    return impl_->wait_for_acknowledgments(max_wait);
}

const DomainParticipant* Publisher::get_participant() const
{
    return impl_->get_participant();
}

/* TODO
   bool Publisher::delete_contained_entities()
   {
    return impl_->delete_contained_entities();
   }
 */

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
    return ReturnCode_t::RETCODE_OK;
}

/* TODO
   bool Publisher::copy_from_topic_qos(
        fastrtps::WriterQos& writer_qos,
        const fastrtps::TopicAttributes& topic_qos) const
   {
    return impl_->copy_from_topic_qos(writer_qos, topic_qos);
   }
 */

const fastrtps::rtps::InstanceHandle_t& Publisher::get_instance_handle() const
{
    return impl_->get_instance_handle();
}
