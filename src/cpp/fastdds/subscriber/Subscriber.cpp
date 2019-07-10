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

/*
 * Subscriber.cpp
 *
 */

#include <fastdds/subscriber/Subscriber.hpp>
#include <fastdds/domain/Participant.hpp>
#include "SubscriberImpl.hpp"

#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace eprosima::fastdds;

const SubscriberQos& Subscriber::get_qos() const
{
    return impl_->get_qos();
}

bool Subscriber::set_qos(
        const SubscriberQos& qos)
{
    return impl_->set_qos(qos);
}

const SubscriberListener* Subscriber::get_listener() const
{
    return impl_->get_listener();
}

bool Subscriber::set_listener(
        SubscriberListener* listener)
{
    return impl_->set_listener(listener);
}

DataReader* Subscriber::create_datareader(
        const fastrtps::TopicAttributes& topic_attr,
        const fastrtps::ReaderQos& reader_qos,
        DataReaderListener* listener)
{
    return impl_->create_datareader(topic_attr, reader_qos, listener);
}

bool Subscriber::delete_datareader(
        DataReader* reader)
{
    return impl_->delete_datareader(reader);
}

DataReader* Subscriber::lookup_datareader(
        const std::string& topic_name) const
{
    return impl_->lookup_datareader(topic_name);
}

bool Subscriber::get_datareaders(
        std::vector<DataReader*>& readers) const
{
    return impl_->get_datareaders(readers);
}

bool Subscriber::begin_access()
{
    return impl_->begin_access();
}

bool Subscriber::end_access()
{
    return impl_->end_access();
}

bool Subscriber::notify_datareaders() const
{
    return impl_->notify_datareaders();
}

bool Subscriber::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}

bool Subscriber::set_default_datareader_qos(
        const fastrtps::ReaderQos& qos)
{
    return impl_->set_default_datareader_qos(qos);
}

const fastrtps::ReaderQos& Subscriber::get_default_datareader_qos() const
{
    return impl_->get_default_datareader_qos();
}

bool Subscriber::copy_from_topic_qos(
        fastrtps::ReaderQos& reader_qos,
        const fastrtps::TopicAttributes& topic_qos) const
{
    return impl_->copy_from_topic_qos(reader_qos, topic_qos);
}

bool Subscriber::set_attributes(
        const fastrtps::SubscriberAttributes& att)
{
    return impl_->set_attributes(att);
}

const fastrtps::SubscriberAttributes& Subscriber::get_attributes() const
{
    return impl_->get_attributes();
}

const Participant* Subscriber::get_participant() const
{
    return impl_->get_participant();
}

const fastrtps::rtps::RTPSParticipant* Subscriber::rtps_participant() const
{
    return impl_->rtps_participant();
}

fastrtps::rtps::RTPSParticipant* Subscriber::rtps_participant()
{
    return impl_->rtps_participant();
}

const Subscriber* Subscriber::get_subscriber() const
{
    return impl_->get_subscriber();
}

const fastrtps::rtps::InstanceHandle_t& Subscriber::get_instance_handle() const
{
    return impl_->get_instance_handle();
}
