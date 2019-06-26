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

#include <fastdds/publisher/Publisher.hpp>
#include <fastdds/domain/Participant.hpp>
#include "PublisherImpl.hpp"

#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace eprosima::fastdds;

Publisher::Publisher(
        PublisherImpl* p)
    : impl_(p)
{
}

Publisher::~Publisher()
{
}

const PublisherQos& Publisher::get_qos() const
{
    return impl_->get_qos();
}

bool Publisher::set_qos(
        const PublisherQos& qos)
{
    return impl_->set_qos(qos);
}

const PublisherListener* Publisher::get_listener() const
{
    return impl_->get_listener();
}

bool Publisher::set_listener(
        PublisherListener* listener)
{
    return impl_->set_listener(listener);
}

DataWriter* Publisher::create_datawriter(
        const fastrtps::TopicAttributes& topic_attr,
        const fastrtps::WriterQos& writer_qos,
        DataWriterListener* listener)
{
    return impl_->create_datawriter(topic_attr, writer_qos, listener);
}

bool Publisher::delete_datawriter(
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

bool Publisher::suspend_publications()
{
    return impl_->suspend_publications();
}

bool Publisher::resume_publications()
{
    return impl_->resume_publications();
}

bool Publisher::begin_coherent_changes()
{
    return impl_->begin_coherent_changes();
}

bool Publisher::end_coherent_changes()
{
    return impl_->end_coherent_changes();
}

bool Publisher::wait_for_acknowledments(
        const fastrtps::Duration_t& max_wait)
{
    return impl_->wait_for_acknowledments(max_wait);
}

const Participant* Publisher::get_participant() const
{
    return impl_->get_participant();
}

bool Publisher::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}

bool Publisher::set_default_datawriter_qos(
        const fastrtps::WriterQos& qos)
{
    return impl_->set_default_datawriter_qos(qos);
}

const fastrtps::WriterQos& Publisher::get_default_datawriter_qos() const
{
    return impl_->get_default_datawriter_qos();
}

bool Publisher::copy_from_topic_qos(
        fastrtps::WriterQos& writer_qos,
        const fastrtps::TopicAttributes& topic_qos) const
{
    return impl_->copy_from_topic_qos(writer_qos, topic_qos);
}

const Publisher* Publisher::get_publisher() const
{
    return impl_->get_publisher();
}

const fastrtps::PublisherAttributes& Publisher::get_attributes() const
{
    return impl_->get_attributes();
}

bool Publisher::set_attributes(const fastrtps::PublisherAttributes& att)
{
    return impl_->set_attributes(att);
}

fastrtps::rtps::RTPSParticipant* Publisher::rtps_participant()
{
    return impl_->rtps_participant();
}

const fastrtps::rtps::RTPSParticipant* Publisher::rtps_participant() const
{
    return impl_->rtps_participant();
}

const fastrtps::rtps::InstanceHandle_t& Publisher::get_instance_handle() const
{
    return impl_->get_instance_handle();
}
