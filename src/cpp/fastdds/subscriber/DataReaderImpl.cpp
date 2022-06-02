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
 * @file DataReaderImpl.cpp
 */

#include <fastrtps/config.h>

#include <fastdds/subscriber/DataReaderImpl.hpp>

#include <fastdds/dds/core/StackAllocatedSequence.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <fastdds/core/condition/StatusConditionImpl.hpp>
#include <fastdds/core/policy/QosPolicyUtils.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/subscriber/DataReaderImpl/ReadTakeCommand.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>

#include <fastdds/topic/ContentFilteredTopicImpl.hpp>

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace dds {

static bool collections_have_same_properties(
        const LoanableCollection& data_values,
        const SampleInfoSeq& sample_infos)
{
    return ((data_values.has_ownership() == sample_infos.has_ownership()) &&
           (data_values.maximum() == sample_infos.maximum()) &&
           (data_values.length() == sample_infos.length()));
}

static bool qos_has_unique_network_request(
        const DataReaderQos& qos)
{
    return nullptr != PropertyPolicyHelper::find_property(qos.properties(), "fastdds.unique_network_flows");
}

static bool qos_has_specific_locators(
        const DataReaderQos& qos)
{
    const RTPSEndpointQos& endpoint = qos.endpoint();
    return !endpoint.unicast_locator_list.empty() ||
           !endpoint.multicast_locator_list.empty() ||
           !endpoint.remote_locator_list.empty();
}

DataReaderImpl::DataReaderImpl(
        SubscriberImpl* s,
        const TypeSupport& type,
        TopicDescription* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener)
    : subscriber_(s)
    , type_(type)
    , topic_(topic)
    , qos_(&qos == &DATAREADER_QOS_DEFAULT ? subscriber_->get_default_datareader_qos() : qos)
#pragma warning (disable : 4355 )
    , history_(type, *topic, qos_)
    , listener_(listener)
    , reader_listener_(this)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
    , sample_info_pool_(qos)
    , loan_manager_(qos)
{
    EndpointAttributes endpoint_attributes;
    endpoint_attributes.endpointKind = READER;
    endpoint_attributes.topicKind = type_->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    endpoint_attributes.setEntityID(qos_.endpoint().entity_id);
    endpoint_attributes.setUserDefinedID(qos_.endpoint().user_defined_id);
    fastrtps::rtps::RTPSParticipantImpl::preprocess_endpoint_attributes<READER, 0x04, 0x07>(
        EntityId_t::unknown(), subscriber_->get_participant_impl()->id_counter(), endpoint_attributes, guid_.entityId);
    guid_.guidPrefix = subscriber_->get_participant_impl()->guid().guidPrefix;
}

ReturnCode_t DataReaderImpl::enable()
{
    assert(reader_ == nullptr);

    fastrtps::rtps::ReaderAttributes att;

    att.endpoint.durabilityKind = qos_.durability().durabilityKind();
    att.endpoint.endpointKind = READER;
    att.endpoint.multicastLocatorList = qos_.endpoint().multicast_locator_list;
    att.endpoint.reliabilityKind = qos_.reliability().kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    att.endpoint.topicKind = type_->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    att.endpoint.unicastLocatorList = qos_.endpoint().unicast_locator_list;
    att.endpoint.remoteLocatorList = qos_.endpoint().remote_locator_list;
    att.endpoint.properties = qos_.properties();
    att.endpoint.setEntityID(qos_.endpoint().entity_id);
    att.endpoint.setUserDefinedID(qos_.endpoint().user_defined_id);
    att.times = qos_.reliable_reader_qos().times;
    att.liveliness_lease_duration = qos_.liveliness().lease_duration;
    att.liveliness_kind_ = qos_.liveliness().kind;
    att.matched_writers_allocation = qos_.reader_resource_limits().matched_publisher_allocation;
    att.expectsInlineQos = qos_.expects_inline_qos();
    att.disable_positive_acks = qos_.reliable_reader_qos().disable_positive_ACKs.enabled;


    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_->get_impl()->get_rtps_topic_name().c_str());
    att.endpoint.properties.properties().push_back(std::move(property));

    std::string* endpoint_partitions = PropertyPolicyHelper::find_property(qos_.properties(), "partitions");

    if (endpoint_partitions)
    {
        property.name("partitions");
        property.value(*endpoint_partitions);
        att.endpoint.properties.properties().push_back(std::move(property));
    }
    else if (subscriber_->get_qos().partition().names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        bool is_first_partition = true;
        for (auto partition : subscriber_->get_qos().partition().names())
        {
            partitions += (is_first_partition ? "" : ";") + partition;
            is_first_partition = false;
        }
        property.value(std::move(partitions));
        att.endpoint.properties.properties().push_back(std::move(property));
    }

    bool is_datasharing_compatible = false;
    ReturnCode_t ret_code = check_datasharing_compatible(att, is_datasharing_compatible);
    if (ret_code != ReturnCode_t::RETCODE_OK)
    {
        return ret_code;
    }
    if (is_datasharing_compatible)
    {
        DataSharingQosPolicy datasharing(qos_.data_sharing());
        if (datasharing.domain_ids().empty())
        {
            datasharing.add_domain_id(utils::default_domain_id());
        }
        att.endpoint.set_data_sharing_configuration(datasharing);
    }
    else
    {
        DataSharingQosPolicy datasharing;
        datasharing.off();
        att.endpoint.set_data_sharing_configuration(datasharing);
    }

    std::shared_ptr<IPayloadPool> pool = get_payload_pool();
    RTPSReader* reader = RTPSDomain::createRTPSReader(
        subscriber_->rtps_participant(),
        guid_.entityId,
        att, pool,
        static_cast<ReaderHistory*>(&history_),
        static_cast<ReaderListener*>(&reader_listener_));

    if (reader == nullptr)
    {
        release_payload_pool();
        logError(DATA_READER, "Problem creating associated Reader");
        return ReturnCode_t::RETCODE_ERROR;
    }

    auto content_topic = dynamic_cast<ContentFilteredTopicImpl*>(topic_->get_impl());
    if (nullptr != content_topic)
    {
        reader->set_content_filter(content_topic);
        content_topic->add_reader(this);
    }

    reader_ = reader;

    deadline_timer_ = new TimedEvent(subscriber_->get_participant()->get_resource_event(),
                    [&]() -> bool
                    {
                        return deadline_missed();
                    },
                    qos_.deadline().period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(subscriber_->get_participant()->get_resource_event(),
                    [&]() -> bool
                    {
                        return lifespan_expired();
                    },
                    qos_.lifespan().duration.to_ns() * 1e-6);

    // Register the reader
    ReaderQos rqos = qos_.get_readerqos(subscriber_->get_qos());
    if (!is_datasharing_compatible)
    {
        rqos.data_sharing.off();
    }
    if (endpoint_partitions)
    {
        std::istringstream partition_string(*endpoint_partitions);
        std::string partition_name;
        rqos.m_partition.clear();

        while (std::getline(partition_string, partition_name, ';'))
        {
            rqos.m_partition.push_back(partition_name.c_str());
        }
    }

    eprosima::fastdds::rtps::ContentFilterProperty* filter_property = nullptr;
    if (nullptr != content_topic && !content_topic->filter_property.filter_expression.empty())
    {
        filter_property = &content_topic->filter_property;
    }
    if (!subscriber_->rtps_participant()->registerReader(reader_, topic_attributes(), rqos, filter_property))
    {
        logError(DATA_READER, "Could not register reader on discovery protocols");

        reader_->setListener(nullptr);
        stop();

        return ReturnCode_t::RETCODE_ERROR;
    }

    return ReturnCode_t::RETCODE_OK;
}

void DataReaderImpl::disable()
{
    set_listener(nullptr);
    if (reader_ != nullptr)
    {
        reader_->setListener(nullptr);
    }
}

void DataReaderImpl::stop()
{
    delete lifespan_timer_;
    delete deadline_timer_;

    auto content_topic = dynamic_cast<ContentFilteredTopicImpl*>(topic_->get_impl());
    if (nullptr != content_topic)
    {
        content_topic->remove_reader(this);
    }

    if (reader_ != nullptr)
    {
        logInfo(DATA_READER, "Removing " << guid().entityId << " in topic: " << topic_->get_name());
        RTPSDomain::removeRTPSReader(reader_);
        reader_ = nullptr;
        release_payload_pool();
    }
}

DataReaderImpl::~DataReaderImpl()
{
    // Disable the datareader to prevent receiving data in the middle of deleting it
    disable();

    stop();

    delete user_datareader_;
}

bool DataReaderImpl::can_be_deleted() const
{
    if (reader_ != nullptr)
    {
        std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());
        return !loan_manager_.has_outstanding_loans();
    }

    return true;
}

bool DataReaderImpl::wait_for_unread_message(
        const fastrtps::Duration_t& timeout)
{
    return reader_ ? reader_->wait_for_unread_cache(timeout) : false;
}

void DataReaderImpl::set_read_communication_status(
        bool trigger_value)
{
    StatusMask notify_status = StatusMask::data_on_readers();
    subscriber_->user_subscriber_->get_statuscondition().get_impl()->set_status(notify_status, trigger_value);

    notify_status = StatusMask::data_available();
    user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, trigger_value);
}

ReturnCode_t DataReaderImpl::check_collection_preconditions_and_calc_max_samples(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t& max_samples)
{
    // Properties should be the same on both collections
    if (!collections_have_same_properties(data_values, sample_infos))
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    // Check if a loan is required
    if (0 < data_values.maximum())
    {
        // Loan not required, input collections should not be already loaned
        if (false == data_values.has_ownership())
        {
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }

        int32_t collection_max = data_values.maximum();

        // We consider all negative value to be LENGTH_UNLIMITED
        if (0 > max_samples)
        {
            // When max_samples is LENGTH_UNLIMITED, the collection imposes the maximum number of samples
            max_samples = collection_max;
        }
        else
        {
            if (max_samples > collection_max)
            {
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
        }
    }

    // All preconditions have been checked. Now apply resource limits on max_samples.
    if ((0 > max_samples) || (max_samples > qos_.reader_resource_limits().max_samples_per_read))
    {
        max_samples = qos_.reader_resource_limits().max_samples_per_read;
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::prepare_loan(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t& max_samples)
{
    if (0 < data_values.maximum())
    {
        // A loan was not requested
        return ReturnCode_t::RETCODE_OK;
    }

    if (max_samples > 0)
    {
        // Check if there are enough sample_infos
        size_t num_infos = sample_info_pool_.num_allocated();
        if (num_infos == qos_.reader_resource_limits().sample_infos_allocation.maximum)
        {
            return ReturnCode_t::RETCODE_OUT_OF_RESOURCES;
        }

        // Limit max_samples to available sample_infos
        num_infos += max_samples;
        if (num_infos > qos_.reader_resource_limits().sample_infos_allocation.maximum)
        {
            size_t exceed = num_infos - qos_.reader_resource_limits().sample_infos_allocation.maximum;
            max_samples -= static_cast<uint32_t>(exceed);
        }
    }

    if (max_samples > 0)
    {
        // Check if there are enough samples
        int32_t num_samples = sample_pool_->num_allocated();
        int32_t max_resource_samples = qos_.resource_limits().max_samples;
        if (max_resource_samples <= 0)
        {
            max_resource_samples = std::numeric_limits<int32_t>::max();
        }
        if (num_samples == max_resource_samples)
        {
            return ReturnCode_t::RETCODE_OUT_OF_RESOURCES;
        }

        // Limit max_samples to available samples
        num_samples += max_samples;
        if (num_samples > max_resource_samples)
        {
            int32_t exceed = num_samples - max_resource_samples;
            max_samples -= exceed;
        }
    }

    // Check if there are enough loans
    ReturnCode_t code = loan_manager_.get_loan(data_values, sample_infos);
    if (!code)
    {
        return code;
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::read_or_take(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states,
        bool exact_instance,
        bool single_instance,
        bool should_take)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    ReturnCode_t code = check_collection_preconditions_and_calc_max_samples(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));
#else
            std::chrono::hours(24);
#endif // if HAVE_STRICT_REALTIME

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex(), std::defer_lock);

    if (!lock.try_lock_until(max_blocking_time))
    {
        return ReturnCode_t::RETCODE_TIMEOUT;
    }

    set_read_communication_status(false);

    auto it = history_.lookup_instance(handle, exact_instance);
    if (!it.first)
    {
        return exact_instance ? ReturnCode_t::RETCODE_BAD_PARAMETER : ReturnCode_t::RETCODE_NO_DATA;
    }

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, it.second, single_instance);
    while (!cmd.is_finished())
    {
        cmd.add_instance(should_take);
    }
    return cmd.return_value();
}

ReturnCode_t DataReaderImpl::read(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, HANDLE_NIL,
                   sample_states, view_states, instance_states, false, false, false);
}

ReturnCode_t DataReaderImpl::read_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& a_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, a_handle,
                   sample_states, view_states, instance_states, true, true, false);
}

ReturnCode_t DataReaderImpl::read_next_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, previous_handle,
                   sample_states, view_states, instance_states, false, true, false);
}

ReturnCode_t DataReaderImpl::take(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, HANDLE_NIL,
                   sample_states, view_states, instance_states, false, false, true);
}

ReturnCode_t DataReaderImpl::take_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& a_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, a_handle,
                   sample_states, view_states, instance_states, true, true, true);
}

ReturnCode_t DataReaderImpl::take_next_instance(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        const InstanceHandle_t& previous_handle,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
{
    return read_or_take(data_values, sample_infos, max_samples, previous_handle,
                   sample_states, view_states, instance_states, false, true, true);
}

ReturnCode_t DataReaderImpl::return_loan(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos)
{
    static_cast<void>(data_values);
    static_cast<void>(sample_infos);

    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    // Properties should be the same on both collections
    if (!collections_have_same_properties(data_values, sample_infos))
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    // They should have a loan
    if (data_values.has_ownership() == true)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    // Check if they were loaned by this reader
    ReturnCode_t code = loan_manager_.return_loan(data_values, sample_infos);
    if (!code)
    {
        return code;
    }

    // Return samples and infos
    LoanableCollection::size_type n = sample_infos.length();
    while (n > 0)
    {
        --n;
        if (sample_infos[n].valid_data)
        {
            sample_pool_->return_loan(data_values.buffer()[n]);
        }

        sample_info_pool_.return_item(&sample_infos[n]);
    }

    data_values.unloan();
    sample_infos.unloan();

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::read_or_take_next_sample(
        void* data,
        SampleInfo* info,
        bool should_take)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    if (history_.getHistorySize() == 0)
    {
        return ReturnCode_t::RETCODE_NO_DATA;
    }

    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));
#else
            std::chrono::hours(24);
#endif // if HAVE_STRICT_REALTIME

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex(), std::defer_lock);

    if (!lock.try_lock_until(max_blocking_time))
    {
        return ReturnCode_t::RETCODE_TIMEOUT;
    }

    set_read_communication_status(false);

    auto it = history_.lookup_instance(HANDLE_NIL, false);
    if (!it.first)
    {
        return ReturnCode_t::RETCODE_NO_DATA;
    }

    StackAllocatedSequence<void*, 1> data_values;
    const_cast<void**>(data_values.buffer())[0] = data;
    StackAllocatedSequence<SampleInfo, 1> sample_infos;

    detail::StateFilter states{ NOT_READ_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, 1, states, it.second, false);
    while (!cmd.is_finished())
    {
        cmd.add_instance(should_take);
    }

    ReturnCode_t code = cmd.return_value();
    if (ReturnCode_t::RETCODE_OK == code)
    {
        *info = sample_infos[0];
    }
    return code;
}

ReturnCode_t DataReaderImpl::read_next_sample(
        void* data,
        SampleInfo* info)
{
    return read_or_take_next_sample(data, info, false);
}

ReturnCode_t DataReaderImpl::take_next_sample(
        void* data,
        SampleInfo* info)
{
    return read_or_take_next_sample(data, info, true);
}

ReturnCode_t DataReaderImpl::get_first_untaken_info(
        SampleInfo* info)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    if (history_.get_first_untaken_info(*info))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_NO_DATA;
}

uint64_t DataReaderImpl::get_unread_count() const
{
    return reader_ ? reader_->get_unread_count() : 0;
}

const GUID_t& DataReaderImpl::guid() const
{
    return guid_;
}

InstanceHandle_t DataReaderImpl::get_instance_handle() const
{
    return guid();
}

void DataReaderImpl::subscriber_qos_updated()
{
    update_rtps_reader_qos();
}

void DataReaderImpl::update_rtps_reader_qos()
{
    if (reader_)
    {
        eprosima::fastdds::rtps::ContentFilterProperty* filter_property = nullptr;
        auto content_topic = dynamic_cast<ContentFilteredTopicImpl*>(topic_->get_impl());
        if (nullptr != content_topic && !content_topic->filter_property.filter_expression.empty())
        {
            filter_property = &content_topic->filter_property;
        }
        ReaderQos rqos = qos_.get_readerqos(get_subscriber()->get_qos());
        subscriber_->rtps_participant()->updateReader(reader_, topic_attributes(), rqos, filter_property);
    }
}

ReturnCode_t DataReaderImpl::set_qos(
        const DataReaderQos& qos)
{
    bool enabled = reader_ != nullptr;
    const DataReaderQos& qos_to_set = (&qos == &DATAREADER_QOS_DEFAULT) ?
            subscriber_->get_default_datareader_qos() : qos;

    // Default qos is always considered consistent
    if (&qos != &DATAREADER_QOS_DEFAULT)
    {
        if (subscriber_->get_participant()->get_qos().allocation().data_limits.max_user_data != 0 &&
                subscriber_->get_participant()->get_qos().allocation().data_limits.max_user_data <
                qos_to_set.user_data().getValue().size())
        {
            return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
        }

        ReturnCode_t check_result = check_qos(qos_to_set);
        if (!check_result)
        {
            return check_result;
        }
    }

    if (enabled && !can_qos_be_updated(qos_, qos_to_set))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }

    set_qos(qos_, qos_to_set, !enabled);

    if (enabled)
    {
        // NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
        update_rtps_reader_qos();

        // Deadline
        if (qos_.deadline().period != c_TimeInfinite)
        {
            deadline_duration_us_ = duration<double, std::ratio<1, 1000000>>(qos_.deadline().period.to_ns() * 1e-3);
            deadline_timer_->update_interval_millisec(qos_.deadline().period.to_ns() * 1e-6);
        }
        else
        {
            deadline_timer_->cancel_timer();
        }

        // Lifespan
        if (qos_.lifespan().duration != c_TimeInfinite)
        {
            lifespan_duration_us_ =
                    std::chrono::duration<double, std::ratio<1, 1000000>>(qos_.lifespan().duration.to_ns() * 1e-3);
            lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
        }
        else
        {
            lifespan_timer_->cancel_timer();
        }
    }

    return ReturnCode_t::RETCODE_OK;
}

const DataReaderQos& DataReaderImpl::get_qos() const
{
    return qos_;
}

void DataReaderImpl::InnerDataReaderListener::onNewCacheChangeAdded(
        RTPSReader* /*reader*/,
        const CacheChange_t* const change_in)
{
    if (data_reader_->on_new_cache_change_added(change_in))
    {
        auto user_reader = data_reader_->user_datareader_;

        //First check if we can handle with on_data_on_readers
        SubscriberListener* subscriber_listener =
                data_reader_->subscriber_->get_listener_for(StatusMask::data_on_readers());
        if (subscriber_listener != nullptr)
        {
            subscriber_listener->on_data_on_readers(data_reader_->subscriber_->user_subscriber_);
        }
        else
        {
            // If not, try with on_data_available
            DataReaderListener* listener = data_reader_->get_listener_for(StatusMask::data_available());
            if (listener != nullptr)
            {
                listener->on_data_available(user_reader);
            }
        }

        data_reader_->set_read_communication_status(true);
    }
}

void DataReaderImpl::InnerDataReaderListener::onReaderMatched(
        RTPSReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    data_reader_->update_subscription_matched_status(info);
}

void DataReaderImpl::InnerDataReaderListener::on_liveliness_changed(
        RTPSReader* /*reader*/,
        const fastrtps::LivelinessChangedStatus& status)
{
    data_reader_->update_liveliness_status(status);
    StatusMask notify_status = StatusMask::liveliness_changed();
    DataReaderListener* listener = data_reader_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        LivelinessChangedStatus callback_status;
        if (data_reader_->get_liveliness_changed_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_liveliness_changed(data_reader_->user_datareader_, callback_status);
        }
    }
    data_reader_->user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataReaderImpl::InnerDataReaderListener::on_requested_incompatible_qos(
        RTPSReader* /*reader*/,
        fastdds::dds::PolicyMask qos)
{
    data_reader_->update_requested_incompatible_qos(qos);
    StatusMask notify_status = StatusMask::requested_incompatible_qos();
    DataReaderListener* listener = data_reader_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        RequestedIncompatibleQosStatus callback_status;
        if (data_reader_->get_requested_incompatible_qos_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_requested_incompatible_qos(data_reader_->user_datareader_, callback_status);
        }
    }
    data_reader_->user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataReaderImpl::InnerDataReaderListener::on_sample_lost(
        RTPSReader* /*reader*/,
        int32_t sample_lost_since_last_update)
{
    data_reader_->update_sample_lost_status(sample_lost_since_last_update);
    StatusMask notify_status = StatusMask::sample_lost();
    DataReaderListener* listener = data_reader_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        SampleLostStatus callback_status;
        if (data_reader_->get_sample_lost_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_sample_lost(data_reader_->user_datareader_, callback_status);
        }
    }
    data_reader_->user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataReaderImpl::InnerDataReaderListener::on_sample_rejected(
        RTPSReader* /*reader*/,
        SampleRejectedStatusKind reason,
        const CacheChange_t* const change_in)
{
    if (data_reader_->update_sample_rejected_status(reason, change_in))
    {
        StatusMask notify_status = StatusMask::sample_rejected();
        DataReaderListener* listener = data_reader_->get_listener_for(notify_status);
        if (listener != nullptr)
        {
            SampleRejectedStatus callback_status;
            if (data_reader_->get_sample_rejected_status(callback_status) == ReturnCode_t::RETCODE_OK)
            {
                listener->on_sample_rejected(data_reader_->user_datareader_, callback_status);
            }
        }
        data_reader_->user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);
    }
}

bool DataReaderImpl::on_new_cache_change_added(
        const CacheChange_t* const change)
{
    std::lock_guard<RecursiveTimedMutex> guard(reader_->getMutex());

    if (qos_.deadline().period != c_TimeInfinite)
    {
        if (!history_.set_next_deadline(
                    change->instanceHandle,
                    steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
        {
            logError(SUBSCRIBER, "Could not set next deadline in the history");
        }
        else if (timer_owner_ == change->instanceHandle || timer_owner_ == InstanceHandle_t())
        {
            if (deadline_timer_reschedule())
            {
                deadline_timer_->cancel_timer();
                deadline_timer_->restart_timer();
            }
        }
    }

    CacheChange_t* new_change = const_cast<CacheChange_t*>(change);
    history_.update_instance_nts(new_change);

    if (qos_.lifespan().duration == c_TimeInfinite)
    {
        return true;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // The new change could have expired if it arrived too late
    // If so, remove it from the history and return false to avoid notifying the listener
    if (now - source_timestamp >= lifespan_duration_us_)
    {
        history_.remove_change_sub(new_change);
        return false;
    }

    CacheChange_t* earliest_change;
    if (history_.get_earliest_change(&earliest_change))
    {
        if (earliest_change == change)
        {
            // The new change has been added at the beginning of the the history
            // As the history is sorted by timestamp, this means that the new change has the smallest timestamp
            // We have to stop the timer as this will be the next change to expire
            lifespan_timer_->cancel_timer();
        }
    }
    else
    {
        logError(SUBSCRIBER, "A change was added to history that could not be retrieved");
    }

    auto interval = source_timestamp - now + duration_cast<nanoseconds>(lifespan_duration_us_);

    // Update and restart the timer
    // If the timer is already running this will not have any effect
    lifespan_timer_->update_interval_millisec(interval.count() * 1e-6);
    lifespan_timer_->restart_timer();
    return true;
}

void DataReaderImpl::update_subscription_matched_status(
        const SubscriptionMatchedStatus& status)
{
    auto count_change = status.current_count_change;
    subscription_matched_status_.current_count += count_change;
    subscription_matched_status_.current_count_change += count_change;
    if (count_change > 0)
    {
        subscription_matched_status_.total_count += count_change;
        subscription_matched_status_.total_count_change += count_change;
    }
    subscription_matched_status_.last_publication_handle = status.last_publication_handle;

    if (count_change < 0)
    {
        history_.writer_not_alive(iHandle2GUID(status.last_publication_handle));
    }

    StatusMask notify_status = StatusMask::subscription_matched();
    DataReaderListener* listener = get_listener_for(notify_status);
    if (listener != nullptr)
    {
        listener->on_subscription_matched(user_datareader_, subscription_matched_status_);
        subscription_matched_status_.current_count_change = 0;
        subscription_matched_status_.total_count_change = 0;
    }
    user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

ReturnCode_t DataReaderImpl::get_subscription_matched_status(
        SubscriptionMatchedStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

        status = subscription_matched_status_;
        subscription_matched_status_.current_count_change = 0;
        subscription_matched_status_.total_count_change = 0;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::subscription_matched(), false);
    return ReturnCode_t::RETCODE_OK;
}

bool DataReaderImpl::deadline_timer_reschedule()
{
    assert(qos_.deadline().period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!history_.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(SUBSCRIBER, "Could not get the next deadline from the history");
        return false;
    }
    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());

    deadline_timer_->update_interval_millisec(static_cast<double>(interval_ms.count()));
    return true;
}

bool DataReaderImpl::deadline_missed()
{
    assert(qos_.deadline().period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    StatusMask notify_status = StatusMask::requested_deadline_missed();
    auto listener = get_listener_for(notify_status);
    if (nullptr != listener)
    {
        listener->on_requested_deadline_missed(user_datareader_, deadline_missed_status_);
        deadline_missed_status_.total_count_change = 0;
    }
    user_datareader_->get_statuscondition().get_impl()->set_status(notify_status, true);

    if (!history_.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(SUBSCRIBER, "Could not set next deadline in the history");
        return false;
    }
    return deadline_timer_reschedule();
}

ReturnCode_t DataReaderImpl::get_requested_deadline_missed_status(
        RequestedDeadlineMissedStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

        status = deadline_missed_status_;
        deadline_missed_status_.total_count_change = 0;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::requested_deadline_missed(), false);
    return ReturnCode_t::RETCODE_OK;
}

bool DataReaderImpl::lifespan_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    CacheChange_t* earliest_change;
    while (history_.get_earliest_change(&earliest_change))
    {
        auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
        auto now = system_clock::now();

        // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
        if (now - source_timestamp < lifespan_duration_us_)
        {
            auto interval = source_timestamp - now + lifespan_duration_us_;
            lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
            return true;
        }

        // The earliest change has expired
        history_.remove_change_sub(earliest_change);

        // Set the timer for the next change if there is one
        if (!history_.get_earliest_change(&earliest_change))
        {
            return false;
        }

        // Calculate when the next change is due to expire and restart
        source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
        now = system_clock::now();
        auto interval = source_timestamp - now + lifespan_duration_us_;

        if (interval.count() > 0)
        {
            lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
            return true;
        }
    }

    return false;
}

ReturnCode_t DataReaderImpl::set_listener(
        DataReaderListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

const DataReaderListener* DataReaderImpl::get_listener() const
{
    return listener_;
}

/* TODO
   bool DataReaderImpl::get_key_value(
        void* data,
        const rtps::InstanceHandle_t& handle)
   {
    (void)data;
    (void)handle;
    // TODO Implement
    return false;
   }
 */

ReturnCode_t DataReaderImpl::get_liveliness_changed_status(
        LivelinessChangedStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

        status = liveliness_changed_status_;
        liveliness_changed_status_.alive_count_change = 0u;
        liveliness_changed_status_.not_alive_count_change = 0u;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::liveliness_changed(), false);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

        status = requested_incompatible_qos_status_;
        requested_incompatible_qos_status_.total_count_change = 0u;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::requested_incompatible_qos(), false);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::get_sample_lost_status(
        SampleLostStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

        status = sample_lost_status_;
        sample_lost_status_.total_count_change = 0u;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::sample_lost(), false);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::get_sample_rejected_status(
        SampleRejectedStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    {
        std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

        status = sample_rejected_status_;
        sample_rejected_status_.total_count_change = 0u;
        sample_rejected_status_.last_reason = NOT_REJECTED;
        sample_rejected_status_.last_instance_handle = c_InstanceHandle_Unknown;
    }

    user_datareader_->get_statuscondition().get_impl()->set_status(StatusMask::sample_rejected(), false);
    return ReturnCode_t::RETCODE_OK;
}

const Subscriber* DataReaderImpl::get_subscriber() const
{
    return subscriber_->get_subscriber();
}

/* TODO
   bool DataReaderImpl::wait_for_historical_data(
        const Duration_t& max_wait) const
   {
    (void)max_wait;
    // TODO Implement
    return false;
   }
 */

const TopicDescription* DataReaderImpl::get_topicdescription() const
{
    return topic_;
}

TypeSupport DataReaderImpl::type()
{
    return type_;
}

RequestedIncompatibleQosStatus& DataReaderImpl::update_requested_incompatible_qos(
        PolicyMask incompatible_policies)
{
    ++requested_incompatible_qos_status_.total_count;
    ++requested_incompatible_qos_status_.total_count_change;
    for (fastrtps::rtps::octet id = 1; id < NEXT_QOS_POLICY_ID; ++id)
    {
        if (incompatible_policies.test(id))
        {
            ++requested_incompatible_qos_status_.policies[static_cast<QosPolicyId_t>(id)].count;
            requested_incompatible_qos_status_.last_policy_id = static_cast<QosPolicyId_t>(id);
        }
    }
    return requested_incompatible_qos_status_;
}

LivelinessChangedStatus& DataReaderImpl::update_liveliness_status(
        const fastrtps::LivelinessChangedStatus& status)
{
    if (0 < status.not_alive_count_change)
    {
        history_.writer_not_alive(iHandle2GUID(status.last_publication_handle));
    }

    liveliness_changed_status_.alive_count = status.alive_count;
    liveliness_changed_status_.not_alive_count = status.not_alive_count;
    liveliness_changed_status_.alive_count_change += status.alive_count_change;
    liveliness_changed_status_.not_alive_count_change += status.not_alive_count_change;
    liveliness_changed_status_.last_publication_handle = status.last_publication_handle;

    return liveliness_changed_status_;
}

SampleLostStatus& DataReaderImpl::update_sample_lost_status(
        int32_t sample_lost_since_last_update)
{
    sample_lost_status_.total_count += sample_lost_since_last_update;
    sample_lost_status_.total_count_change += sample_lost_since_last_update;

    return sample_lost_status_;
}

ReturnCode_t DataReaderImpl::check_qos (
        const DataReaderQos& qos)
{
    if (qos.durability().kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(DDS_QOS_CHECK, "PERSISTENT Durability not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (qos.destination_order().kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(DDS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (qos.reliability().kind == BEST_EFFORT_RELIABILITY_QOS && qos.ownership().kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(DDS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    if (qos.reader_resource_limits().max_samples_per_read <= 0)
    {
        logError(DDS_QOS_CHECK, "max_samples_per_read should be strictly possitive");
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    if (qos_has_unique_network_request(qos) && qos_has_specific_locators(qos))
    {
        logError(DDS_QOS_CHECK, "unique_network_request cannot be set along specific locators");
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    return ReturnCode_t::RETCODE_OK;
}

bool DataReaderImpl::can_qos_be_updated(
        const DataReaderQos& to,
        const DataReaderQos& from)
{
    bool updatable = true;
    if (!(to.resource_limits() == from.resource_limits()))
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "resource_limits cannot be changed after the creation of a DataReader.");
    }
    if (to.history().kind != from.history().kind ||
            to.history().depth != from.history().depth)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "History cannot be changed after the creation of a DataReader.");
    }

    if (to.durability().kind != from.durability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Durability kind cannot be changed after the creation of a DataReader.");
    }
    if (to.liveliness().kind != from.liveliness().kind ||
            to.liveliness().lease_duration != from.liveliness().lease_duration ||
            to.liveliness().announcement_period != from.liveliness().announcement_period)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness cannot be changed after the creation of a DataReader.");
    }
    if (to.reliability().kind != from.reliability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a DataReader.");
    }
    if (to.ownership().kind != from.ownership().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a DataReader.");
    }
    if (to.destination_order().kind != from.destination_order().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a DataReader.");
    }
    if (!(to.reader_resource_limits() == from.reader_resource_limits()))
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "reader_resource_limits cannot be changed after the creation of a DataReader.");
    }
    if (to.data_sharing().kind() != from.data_sharing().kind())
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Data sharing configuration cannot be changed after the creation of a DataReader.");
    }
    if (to.data_sharing().shm_directory() != from.data_sharing().shm_directory())
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Data sharing configuration cannot be changed after the creation of a DataReader.");
    }
    if (to.data_sharing().domain_ids() != from.data_sharing().domain_ids())
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Data sharing configuration cannot be changed after the creation of a DataReader.");
    }
    if (qos_has_unique_network_request(to) != qos_has_unique_network_request(from))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK,
                "Unique network flows request cannot be changed after the creation of a DataReader.");
    }
    return updatable;
}

void DataReaderImpl::set_qos(
        DataReaderQos& to,
        const DataReaderQos& from,
        bool first_time)
{
    if (first_time && to.durability().kind != from.durability().kind)
    {
        to.durability() = from.durability();
        to.durability().hasChanged = true;
    }
    if (to.deadline().period != from.deadline().period)
    {
        to.deadline() = from.deadline();
        to.deadline().hasChanged = true;
    }
    if (to.latency_budget().duration != from.latency_budget().duration)
    {
        to.latency_budget() = from.latency_budget();
        to.latency_budget().hasChanged = true;
    }
    if (first_time && !(to.liveliness() == from.liveliness()))
    {
        to.liveliness() = from.liveliness();
        to.liveliness().hasChanged = true;
    }
    if (first_time && !(to.reliability() == from.reliability()))
    {
        to.reliability() = from.reliability();
        to.reliability().hasChanged = true;
    }
    if (first_time && to.ownership().kind != from.ownership().kind)
    {
        to.ownership() = from.ownership();
        to.ownership().hasChanged = true;
    }
    if (first_time && to.destination_order().kind != from.destination_order().kind)
    {
        to.destination_order() = from.destination_order();
        to.destination_order().hasChanged = true;
    }
    if (to.user_data().data_vec() != from.user_data().data_vec())
    {
        to.user_data() = from.user_data();
        to.user_data().hasChanged = true;
    }
    if (to.time_based_filter().minimum_separation != from.time_based_filter().minimum_separation )
    {
        to.time_based_filter() = from.time_based_filter();
        to.time_based_filter().hasChanged = true;
    }
    if (first_time || !(to.durability_service() == from.durability_service()))
    {
        to.durability_service() = from.durability_service();
        to.durability_service().hasChanged = true;
    }
    if (to.lifespan().duration != from.lifespan().duration )
    {
        to.lifespan() = from.lifespan();
        to.lifespan().hasChanged = true;
    }
    if (first_time && !(to.reliable_reader_qos() == from.reliable_reader_qos()))
    {
        to.reliable_reader_qos() = from.reliable_reader_qos();
    }
    if (first_time || !(to.type_consistency() == from.type_consistency()))
    {
        to.type_consistency() = from.type_consistency();
        to.type_consistency().hasChanged = true;
    }
    if (first_time && (to.history().kind != from.history().kind ||
            to.history().depth != from.history().depth))
    {
        to.history() = from.history();
        to.history().hasChanged = true;
    }
    if (first_time && !(to.resource_limits() == from.resource_limits()))
    {
        to.resource_limits() = from.resource_limits();
        to.resource_limits().hasChanged = true;
    }
    if (!(to.reader_data_lifecycle() == from.reader_data_lifecycle()))
    {
        to.reader_data_lifecycle() = from.reader_data_lifecycle();
    }

    if (to.expects_inline_qos() != from.expects_inline_qos())
    {
        to.expects_inline_qos(from.expects_inline_qos());
    }

    if (first_time && !(to.properties() == from.properties()))
    {
        to.properties() = from.properties();
    }

    if (first_time && !(to.endpoint() == from.endpoint()))
    {
        to.endpoint() = from.endpoint();
    }

    if (first_time && !(to.reader_resource_limits() == from.reader_resource_limits()))
    {
        to.reader_resource_limits() = from.reader_resource_limits();
    }

    if (first_time && !(to.data_sharing() == from.data_sharing()))
    {
        to.data_sharing() = from.data_sharing();
    }
}

fastrtps::TopicAttributes DataReaderImpl::topic_attributes() const
{
    fastrtps::TopicAttributes topic_att;
    topic_att.topicKind = type_->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    topic_att.topicName = topic_->get_impl()->get_rtps_topic_name();
    topic_att.topicDataType = topic_->get_type_name();
    topic_att.historyQos = qos_.history();
    topic_att.resourceLimitsQos = qos_.resource_limits();
    if (type_->type_object())
    {
        topic_att.type = *type_->type_object();
    }
    if (type_->type_identifier())
    {
        topic_att.type_id = *type_->type_identifier();
    }
    if (type_->type_information())
    {
        topic_att.type_information = *type_->type_information();
    }
    topic_att.auto_fill_type_object = type_->auto_fill_type_object();
    topic_att.auto_fill_type_information = type_->auto_fill_type_information();

    return topic_att;
}

DataReaderListener* DataReaderImpl::get_listener_for(
        const StatusMask& status)
{
    if (listener_ != nullptr &&
            user_datareader_->get_status_mask().is_active(status))
    {
        return listener_;
    }
    return subscriber_->get_listener_for(status);
}

std::shared_ptr<IPayloadPool> DataReaderImpl::get_payload_pool()
{
    // When the user requested PREALLOCATED_WITH_REALLOC, but we know the type cannot
    // grow, we translate the policy into bare PREALLOCATED
    if (PREALLOCATED_WITH_REALLOC_MEMORY_MODE == history_.m_att.memoryPolicy &&
            (type_->is_bounded() || type_->is_plain()))
    {
        history_.m_att.memoryPolicy = PREALLOCATED_MEMORY_MODE;
    }

    PoolConfig config = PoolConfig::from_history_attributes(history_.m_att);

    if (!payload_pool_)
    {
        payload_pool_ = TopicPayloadPoolRegistry::get(topic_->get_impl()->get_rtps_topic_name(), config);
        sample_pool_ = std::make_shared<detail::SampleLoanManager>(config, type_);
    }

    payload_pool_->reserve_history(config, true);
    return payload_pool_;
}

void DataReaderImpl::release_payload_pool()
{
    assert(payload_pool_);

    PoolConfig config = PoolConfig::from_history_attributes(history_.m_att);
    payload_pool_->release_history(config, true);
    payload_pool_.reset();
}

ReturnCode_t DataReaderImpl::check_datasharing_compatible(
        const ReaderAttributes& reader_attributes,
        bool& is_datasharing_compatible) const
{
#if HAVE_SECURITY
    bool has_security_enabled = subscriber_->rtps_participant()->is_security_enabled_for_reader(reader_attributes);
#else
    (void) reader_attributes;
#endif // HAVE_SECURITY

    bool has_key = type_->m_isGetKeyDefined;

    is_datasharing_compatible = false;
    switch (qos_.data_sharing().kind())
    {
        case DataSharingKind::OFF:
            return ReturnCode_t::RETCODE_OK;
            break;
        case DataSharingKind::ON:
#if HAVE_SECURITY
            if (has_security_enabled)
            {
                logError(DATA_READER, "Data sharing cannot be used with security protection.");
                return ReturnCode_t::RETCODE_NOT_ALLOWED_BY_SECURITY;
            }
#endif // if HAVE_SECURITY
            if (!type_.is_bounded())
            {
                logInfo(DATA_READER, "Data sharing cannot be used with unbounded data types");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            if (has_key)
            {
                logError(DATA_READER, "Data sharing cannot be used with keyed data types");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            is_datasharing_compatible = true;
            return ReturnCode_t::RETCODE_OK;
            break;
        case DataSharingKind::AUTO:
#if HAVE_SECURITY
            if (has_security_enabled)
            {
                logInfo(DATA_READER, "Data sharing disabled due to security configuration.");
                return ReturnCode_t::RETCODE_OK;
            }
#endif // if HAVE_SECURITY

            if (!type_.is_bounded())
            {
                logInfo(DATA_READER, "Data sharing disabled because data type is not bounded");
                return ReturnCode_t::RETCODE_OK;
            }

            if (has_key)
            {
                logInfo(DATA_READER, "Data sharing disabled because data type is keyed");
                return ReturnCode_t::RETCODE_OK;
            }

            is_datasharing_compatible = true;
            return ReturnCode_t::RETCODE_OK;
            break;
        default:
            logError(DATA_WRITER, "Unknown data sharing kind.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

bool DataReaderImpl::is_sample_valid(
        const void* data,
        const SampleInfo* info) const
{
    return reader_->is_sample_valid(data, info->sample_identity.writer_guid(), info->sample_identity.sequence_number());
}

ReturnCode_t DataReaderImpl::get_listening_locators(
        rtps::LocatorList& locators) const
{
    if (nullptr == reader_)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    locators.assign(reader_->getAttributes().unicastLocatorList);
    locators.push_back(reader_->getAttributes().multicastLocatorList);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::delete_contained_entities()
{
    // Until Query Conditions are implemented, there are no contained entities to destroy, so return OK.
    return ReturnCode_t::RETCODE_OK;
}

void DataReaderImpl::filter_has_been_updated()
{
    update_rtps_reader_qos();
}

InstanceHandle_t DataReaderImpl::lookup_instance(
        const void* instance) const
{
    InstanceHandle_t handle = HANDLE_NIL;

    if (instance && type_->m_isGetKeyDefined)
    {
        if (type_->getKey(const_cast<void*>(instance), &handle, false))
        {
            auto it = history_.lookup_instance(handle, true);
            if (!it.first)
            {
                handle = HANDLE_NIL;
            }
        }
    }
    return handle;
}

bool DataReaderImpl::update_sample_rejected_status(
        SampleRejectedStatusKind reason,
        const CacheChange_t* const change_in)
{
    bool ret_val = false;

    if (c_InstanceHandle_Unknown != change_in->instanceHandle ||
            history_.compute_key_for_change_fn(const_cast<CacheChange_t*>(change_in)))
    {
        ++sample_rejected_status_.total_count;
        ++sample_rejected_status_.total_count_change;
        sample_rejected_status_.last_reason = reason;
        sample_rejected_status_.last_instance_handle = change_in->instanceHandle;
        ret_val = true;
    }

    return ret_val;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
