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

/**
 * @file DataReader.cpp
 *
 */

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>


namespace eprosima {

using namespace fastrtps;
using namespace fastrtps::rtps;

namespace fastdds {
namespace dds {

DataReader::DataReader(
        DataReaderImpl* impl)
    : impl_(impl)
{
}

DataReader::~DataReader()
{
}

bool DataReader::wait_for_unread_message(
        const fastrtps::Duration_t& timeout)
{
    return impl_->wait_for_unread_message(timeout);
}

ReturnCode_t DataReader::read_next_sample(
        void* data,
        SampleInfo_t* info)
{
    return impl_->read_next_sample(data, info);
}

ReturnCode_t DataReader::take_next_sample(
        void* data,
        SampleInfo_t* info)
{
    return impl_->take_next_sample(data, info);
}

ReturnCode_t DataReader::get_first_untaken_info(
        SampleInfo_t* info)
{
    return impl_->get_first_untaken_info(info);
}

const GUID_t& DataReader::guid()
{
    return impl_->guid();
}

InstanceHandle_t DataReader::get_instance_handle() const
{
    return impl_->get_instance_handle();
}

ReturnCode_t DataReader::set_qos(
        const DataReaderQos& qos)
{
    return impl_->set_qos(qos);
}

bool DataReader::set_topic(
        const TopicAttributes& topic_att)
{
    return impl_->set_topic(topic_att);
}

const DataReaderQos& DataReader::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t DataReader::get_qos(
        DataReaderQos& qos) const
{
    qos = impl_->get_qos();
    return ReturnCode_t::RETCODE_OK;
}

const TopicAttributes& DataReader::get_topic() const
{
    return impl_->get_topic();
}

bool DataReader::set_attributes(
        const fastrtps::rtps::ReaderAttributes& att)
{
    return impl_->set_attributes(att);
}

const ReaderAttributes& DataReader::get_attributes() const
{
    return impl_->get_attributes();
}

ReturnCode_t DataReader::get_requested_deadline_missed_status(
        RequestedDeadlineMissedStatus& status)
{
    return impl_->get_requested_deadline_missed_status(status);
}

/* TODO
   bool DataReader::read(
        std::vector<void *>& data_values,
        std::vector<SampleInfo_t>& sample_infos,
        uint32_t max_samples)
   {
    return impl_->read(...);
   }

   bool DataReader::take(
        std::vector<void *>& data_values,
        std::vector<SampleInfo_t>& sample_infos,
        uint32_t max_samples)
   {
    return impl_->take(...);
   }
 */

ReturnCode_t DataReader::set_listener(
        DataReaderListener* listener)
{
    return impl_->set_listener(listener);
}

const DataReaderListener* DataReader::get_listener() const
{
    return impl_->get_listener();
}

/* TODO
   bool DataReader::get_key_value(
        void* data,
        const rtps::InstanceHandle_t& handle)
   {
    return impl->get_key_value(...);
   }
 */

ReturnCode_t DataReader::get_liveliness_changed_status(
        LivelinessChangedStatus& status) const
{
    return impl_->get_liveliness_changed_status(status);
}

/* TODO
   bool DataReader::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status) const
   {
    return impl_->get...;
   }
 */

/* TODO
   bool DataReader::get_sample_lost_status(
        SampleLostStatus& status) const
   {
    return impl_->get...;
   }
 */

/* TODO
   bool DataReader::get_sample_rejected_status(
        SampleRejectedStatus& status) const
   {
    return impl_->get...;
   }
 */

const Subscriber* DataReader::get_subscriber() const
{
    return impl_->get_subscriber();
}

/* TODO
   bool DataReader::wait_for_historical_data(
        const Duration_t& max_wait) const
   {
    return impl_->wait_for_historical_data(max_wait);
   }
 */

TypeSupport DataReader::type()
{
    return impl_->type();
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
