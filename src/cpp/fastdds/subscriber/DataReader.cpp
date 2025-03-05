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
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>



namespace eprosima {

using namespace fastdds::rtps;

namespace fastdds {
namespace dds {

DataReader::DataReader(
        DataReaderImpl* impl,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(impl)
{
}

DataReader::DataReader(
        Subscriber* s,
        TopicDescription* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(s->create_datareader(topic, qos, listener, mask)->impl_)
{
}

DataReader::~DataReader()
{
}

ReturnCode_t DataReader::enable()
{
    if (enable_)
    {
        return RETCODE_OK;
    }

    if (false == impl_->get_subscriber()->is_enabled())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    ReturnCode_t ret_code = impl_->enable();
    enable_ = RETCODE_OK == ret_code;
    return ret_code;
}

bool DataReader::wait_for_unread_message(
        const fastdds::dds::Duration_t& timeout)
{
    return impl_->wait_for_unread_message(timeout);
}

ReturnCode_t DataReader::read(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->read(data_values, sample_infos, max_samples, sample_states, view_states, instance_states);
}

ReturnCode_t DataReader::read_w_condition(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        ReadCondition* a_condition)
{
    if ( nullptr == a_condition )
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return read(
        data_values,
        sample_infos,
        max_samples,
        a_condition->get_sample_state_mask(),
        a_condition->get_view_state_mask(),
        a_condition->get_instance_state_mask());
}

ReturnCode_t DataReader::read_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& a_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->read_instance(data_values, sample_infos, max_samples, a_handle, sample_states, view_states,
                   instance_states);
}

ReturnCode_t DataReader::read_next_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->read_next_instance(data_values, sample_infos, max_samples, previous_handle, sample_states,
                   view_states, instance_states);
}

ReturnCode_t DataReader::read_next_instance_w_condition(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        ReadCondition* a_condition)
{
    if ( nullptr == a_condition )
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return read_next_instance(
        data_values,
        sample_infos,
        max_samples,
        previous_handle,
        a_condition->get_sample_state_mask(),
        a_condition->get_view_state_mask(),
        a_condition->get_instance_state_mask());
}

ReturnCode_t DataReader::take(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->take(data_values, sample_infos, max_samples, sample_states, view_states, instance_states);
}

ReturnCode_t DataReader::take_w_condition(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        ReadCondition* a_condition)
{
    if ( nullptr == a_condition )
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return take(
        data_values,
        sample_infos,
        max_samples,
        a_condition->get_sample_state_mask(),
        a_condition->get_view_state_mask(),
        a_condition->get_instance_state_mask());
}

ReturnCode_t DataReader::take_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& a_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->take_instance(data_values, sample_infos, max_samples, a_handle, sample_states, view_states,
                   instance_states);
}

ReturnCode_t DataReader::take_next_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->take_next_instance(data_values, sample_infos, max_samples, previous_handle, sample_states,
                   view_states, instance_states);
}

ReturnCode_t DataReader::take_next_instance_w_condition(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        ReadCondition* a_condition)
{
    if ( nullptr == a_condition )
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return take_next_instance(
        data_values,
        sample_infos,
        max_samples,
        previous_handle,
        a_condition->get_sample_state_mask(),
        a_condition->get_view_state_mask(),
        a_condition->get_instance_state_mask());
}

ReturnCode_t DataReader::return_loan(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos)
{
    return impl_->return_loan(data_values, sample_infos);
}

ReturnCode_t DataReader::get_key_value(
        void* key_holder,
        const InstanceHandle_t& handle)
{
    static_cast<void> (key_holder);
    static_cast<void> (handle);
    return RETCODE_UNSUPPORTED;
}

InstanceHandle_t DataReader::lookup_instance(
        const void* instance) const
{
    return impl_->lookup_instance(instance);
}

ReturnCode_t DataReader::read_next_sample(
        void* data,
        SampleInfo* info)
{
    return impl_->read_next_sample(data, info);
}

ReturnCode_t DataReader::take_next_sample(
        void* data,
        SampleInfo* info)
{
    return impl_->take_next_sample(data, info);
}

ReturnCode_t DataReader::get_first_untaken_info(
        SampleInfo* info)
{
    return impl_->get_first_untaken_info(info);
}

uint64_t DataReader::get_unread_count() const
{
    return impl_->get_unread_count(false);
}

uint64_t DataReader::get_unread_count(
        bool mark_as_read) const
{
    return impl_->get_unread_count(mark_as_read);
}

const GUID_t& DataReader::guid()
{
    return impl_->guid();
}

const GUID_t& DataReader::guid() const
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

const DataReaderQos& DataReader::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t DataReader::get_qos(
        DataReaderQos& qos) const
{
    qos = impl_->get_qos();
    return RETCODE_OK;
}

ReturnCode_t DataReader::get_requested_deadline_missed_status(
        RequestedDeadlineMissedStatus& status)
{
    return impl_->get_requested_deadline_missed_status(status);
}

ReturnCode_t DataReader::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status)
{
    return impl_->get_requested_incompatible_qos_status(status);
}

ReturnCode_t DataReader::set_listener(
        DataReaderListener* listener)
{
    return set_listener(listener, StatusMask::all());
}

ReturnCode_t DataReader::set_listener(
        DataReaderListener* listener,
        const StatusMask& mask)
{
    ReturnCode_t ret_val = impl_->set_listener(listener);
    if (ret_val == RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

const DataReaderListener* DataReader::get_listener() const
{
    return impl_->get_listener();
}

/* TODO
   bool DataReader::get_key_value(
        void* data,
        const InstanceHandle_t& handle)
   {
    return impl->get_key_value(...);
   }
 */

ReturnCode_t DataReader::get_liveliness_changed_status(
        LivelinessChangedStatus& status) const
{
    return impl_->get_liveliness_changed_status(status);
}

ReturnCode_t DataReader::get_sample_lost_status(
        SampleLostStatus& status) const
{
    return impl_->get_sample_lost_status(status);
}

ReturnCode_t DataReader::get_sample_rejected_status(
        SampleRejectedStatus& status) const
{
    return impl_->get_sample_rejected_status(status);
}

ReturnCode_t DataReader::get_subscription_matched_status(
        SubscriptionMatchedStatus& status) const
{
    return impl_->get_subscription_matched_status(status);
}

ReturnCode_t DataReader::get_matched_publication_data(
        PublicationBuiltinTopicData& publication_data,
        const fastdds::rtps::InstanceHandle_t& publication_handle) const
{
    return impl_->get_matched_publication_data(publication_data, publication_handle);
}

ReturnCode_t DataReader::get_matched_publications(
        std::vector<InstanceHandle_t>& publication_handles) const
{
    return impl_->get_matched_publications(publication_handles);
}

ReadCondition* DataReader::create_readcondition(
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return impl_->create_readcondition(sample_states, view_states, instance_states);
}

QueryCondition* DataReader::create_querycondition(
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states,
        const std::string& query_expression,
        const std::vector<std::string>& query_parameters)
{
    EPROSIMA_LOG_WARNING(DATA_READER, "create_querycondition method not implemented");
    static_cast<void> (sample_states);
    static_cast<void> (view_states);
    static_cast<void> (instance_states);
    static_cast<void> (query_expression);
    static_cast<void> (query_parameters);
    return nullptr;
    /*
       return impl_->create_querycondition(sample_states, view_states, instance_states, query_expression, query_parameters);
     */
}

ReturnCode_t DataReader::delete_readcondition(
        ReadCondition* a_condition)
{
    return impl_->delete_readcondition(a_condition);
}

ReturnCode_t DataReader::delete_contained_entities()
{
    return impl_->delete_contained_entities();
}

const Subscriber* DataReader::get_subscriber() const
{
    return impl_->get_subscriber();
}

ReturnCode_t DataReader::wait_for_historical_data(
        const dds::Duration_t& max_wait) const
{
    static_cast<void> (max_wait);
    return RETCODE_UNSUPPORTED;
    /*
       return impl_->wait_for_historical_data(a_condition);
     */
}

TypeSupport DataReader::type() const
{
    return impl_->type();
}

const TopicDescription* DataReader::get_topicdescription() const
{
    return impl_->get_topicdescription();
}

bool DataReader::is_sample_valid(
        const void* data,
        const SampleInfo* info) const
{
    return impl_->is_sample_valid(data, info);
}

ReturnCode_t DataReader::get_listening_locators(
        rtps::LocatorList& locators) const
{
    return impl_->get_listening_locators(locators);
}

ReturnCode_t DataReader::get_subscription_builtin_topic_data(
        SubscriptionBuiltinTopicData& subscription_data) const
{
    return impl_->get_subscription_builtin_topic_data(subscription_data);
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
