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

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;

Subscriber::Subscriber(
        SubscriberImpl* pimpl,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(pimpl)
{
}

Subscriber::Subscriber(
        DomainParticipant* dp,
        const SubscriberQos& qos,
        SubscriberListener* listener,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(dp->create_subscriber(qos, listener, mask)->impl_)
{
}

ReturnCode_t Subscriber::enable()
{
    if (enable_)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    if (false == impl_->get_participant()->is_enabled())
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    enable_ = true;
    ReturnCode_t ret_code = impl_->enable();
    enable_ = ReturnCode_t::RETCODE_OK == ret_code;
    return ret_code;
}

const SubscriberQos& Subscriber::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Subscriber::get_qos(
        SubscriberQos& qos) const
{
    qos = impl_->get_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t Subscriber::set_qos(
        const SubscriberQos& qos)
{
    return impl_->set_qos(qos);
}

const SubscriberListener* Subscriber::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t Subscriber::set_listener(
        SubscriberListener* listener)
{
    return set_listener(listener, StatusMask::all());
}

ReturnCode_t Subscriber::set_listener(
        SubscriberListener* listener,
        const StatusMask& mask)
{
    ReturnCode_t ret_val = impl_->set_listener(listener);
    if (ret_val == ReturnCode_t::RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

DataReader* Subscriber::create_datareader(
        TopicDescription* topic,
        const DataReaderQos& reader_qos,
        DataReaderListener* listener,
        const StatusMask& mask)
{
    return impl_->create_datareader(topic, reader_qos, listener, mask);
}

DataReader* Subscriber::create_datareader_with_profile(
        TopicDescription* topic,
        const std::string& profile_name,
        DataReaderListener* listener,
        const StatusMask& mask)
{
    return impl_->create_datareader_with_profile(topic, profile_name, listener, mask);
}

ReturnCode_t Subscriber::delete_datareader(
        DataReader* reader)
{
    return impl_->delete_datareader(reader);
}

DataReader* Subscriber::lookup_datareader(
        const std::string& topic_name) const
{
    return impl_->lookup_datareader(topic_name);
}

ReturnCode_t Subscriber::get_datareaders(
        std::vector<DataReader*>& readers) const
{
    return impl_->get_datareaders(readers);
}

bool Subscriber::has_datareaders() const
{
    return impl_->has_datareaders();
}

/* TODO
bool Subscriber::begin_access()
{
    return impl_->begin_access();
}
*/

/* TODO
bool Subscriber::end_access()
{
    return impl_->end_access();
}
*/

ReturnCode_t Subscriber::notify_datareaders() const
{
    return impl_->notify_datareaders();
}

/* TODO
bool Subscriber::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}
*/

ReturnCode_t Subscriber::set_default_datareader_qos(
        const DataReaderQos& qos)
{
    return impl_->set_default_datareader_qos(qos);
}

const DataReaderQos& Subscriber::get_default_datareader_qos() const
{
    return impl_->get_default_datareader_qos();
}

DataReaderQos& Subscriber::get_default_datareader_qos()
{
    return impl_->get_default_datareader_qos();
}

ReturnCode_t Subscriber::get_default_datareader_qos(
        DataReaderQos& qos) const
{
    qos = impl_->get_default_datareader_qos();
    return ReturnCode_t::RETCODE_OK;
}

/* TODO
bool Subscriber::copy_from_topic_qos(
        DataReaderQos& reader_qos,
        const fastrtps::TopicAttributes& topic_qos) const
{
    return impl_->copy_from_topic_qos(reader_qos, topic_qos);
}
*/

const DomainParticipant* Subscriber::get_participant() const
{
    return impl_->get_participant();
}

const fastrtps::rtps::InstanceHandle_t& Subscriber::get_instance_handle() const
{
    return impl_->get_instance_handle();
}
