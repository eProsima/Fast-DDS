// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * DataWriter.cpp
 *
 */

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

DataWriter::DataWriter(
        DataWriterImpl* impl,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(impl)
{
}

DataWriter::DataWriter(
        Publisher* pub,
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listener,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(pub->create_datawriter(topic, qos, listener, mask)->impl_)
{
}

DataWriter::~DataWriter()
{
}

bool DataWriter::write(
        void* data)
{
    return impl_->write(data);
}

bool DataWriter::write(
        void* data,
        fastrtps::rtps::WriteParams& params)
{
    return impl_->write(data, params);
}

ReturnCode_t DataWriter::write(
        void* data,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->write(data, handle);
}

fastrtps::rtps::InstanceHandle_t DataWriter::register_instance(
        void* key)
{
    return impl_->register_instance(key);
}

ReturnCode_t DataWriter::dispose(
        void* data,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    return impl_->dispose(data, handle);
}

bool DataWriter::dispose(
        void* data)
{
    return impl_->dispose(data);
}

const fastrtps::rtps::GUID_t& DataWriter::guid()
{
    return impl_->guid();
}

fastrtps::rtps::InstanceHandle_t DataWriter::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

ReturnCode_t DataWriter::set_qos(
        const DataWriterQos& qos)
{
    return impl_->set_qos(qos);
}

const DataWriterQos& DataWriter::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t DataWriter::get_qos(
        DataWriterQos& qos) const
{
    qos = impl_->get_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriter::set_listener(
        DataWriterListener* listener)
{
    return impl_->set_listener(listener);
}

const DataWriterListener* DataWriter::get_listener() const
{
    return impl_->get_listener();
}

Topic* DataWriter::get_topic() const
{
    return impl_->get_topic();
}

const Publisher* DataWriter::get_publisher() const
{
    return impl_->get_publisher();
}

ReturnCode_t DataWriter::wait_for_acknowledgments(
        const fastrtps::Duration_t& max_wait)
{
    return impl_->wait_for_acknowledgments(max_wait);
}

ReturnCode_t DataWriter::get_offered_deadline_missed_status(
        OfferedDeadlineMissedStatus& status)
{
    return impl_->get_offered_deadline_missed_status(status);
}

ReturnCode_t DataWriter::get_liveliness_lost_status(
        LivelinessLostStatus& status)
{
    return impl_->get_liveliness_lost_status(status);
}

ReturnCode_t DataWriter::assert_liveliness()
{
    return impl_->assert_liveliness();
}

ReturnCode_t DataWriter::clear_history(
        size_t* removed)
{
    return impl_->clear_history(removed);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
