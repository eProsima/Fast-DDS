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

#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/topic/DataReaderImpl.hpp>
#include <dds/sub/SampleInfo.hpp>
#include <dds/sub/detail/SampleInfo.hpp>
#include <dds/core/detail/Value.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>

namespace eprosima {

using namespace fastrtps;
using namespace fastrtps::rtps;

namespace fastdds {
namespace dds {

DataReader::DataReader(
        Subscriber* sub,
        Topic* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener,
        const ::dds::core::status::StatusMask& mask)
    : DomainEntity(mask)
    , impl_(sub->create_datareader(topic, qos, listener)->impl_)
{
    impl_->set_topic(*topic);
}

DataReader::DataReader(
        DataReaderImpl* impl,
        const ::dds::core::status::StatusMask& mask)
    : DomainEntity(mask)
    , impl_(impl)
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

ReturnCode_t DataReader::take_next_sample(
        void* data,
        ::dds::sub::SampleInfo& info)
{
    SampleInfo_t sinfo;
    ReturnCode_t result = impl_->take_next_sample(data, &sinfo);
    info = sinfo;
    return result;
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
        Topic& topic)
{
    return impl_->set_topic(topic);
}

TopicDescription* DataReader::get_topicdescription()
{
    return impl_->get_topicdescription();
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

Topic* DataReader::get_topic() const
{
    return impl_->get_topic();
}

bool DataReader::set_attributes(
        const rtps::ReaderAttributes& att)
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
        DataReaderListener* listener,
        const ::dds::core::status::StatusMask& mask)
{
    status_mask_ = mask;
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

ReturnCode_t DataReader::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status) const
{
    return impl_->get_requested_incompatible_qos_status(status);
}

/* TODO
   bool DataReader::get_sample_lost_status(
        SampleLostStatus& status) const
   {
    return impl_->get...;
   }
 */

ReturnCode_t DataReader::get_sample_rejected_status(
        SampleRejectedStatus& status) const
{
    return impl_->get_sample_rejected_status(status);
}

ReturnCode_t DataReader::get_subscription_matched_status(
        SubscriptionMatchedStatus& status)
{
    return impl_->get_subscription_matched_status(status);
}

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

ReturnCode_t DataReader::get_matched_publication_data(
        PublicationBuiltinTopicData& publication_data,
        const fastrtps::rtps::InstanceHandle_t& publication_handle) const
{
    return impl_->get_matched_publication_data(publication_data, publication_handle);
}

ReturnCode_t DataReader::get_matched_publications(
        std::vector<InstanceHandle_t>& publication_handles) const
{
    return impl_->get_matched_publications(publication_handles);
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
