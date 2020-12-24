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
 *
 */

#include <fastrtps/config.h>

#include <fastdds/subscriber/DataReaderImpl.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/subscriber/DataReaderImpl/ReadTakeCommand.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <rtps/history/TopicPayloadPoolRegistry.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace dds {

static void sample_info_to_dds (
        const SampleInfo_t& rtps_info,
        SampleInfo* dds_info)
{
    dds_info->sample_state = NOT_READ_SAMPLE_STATE;
    dds_info->view_state = NOT_NEW_VIEW_STATE;
    dds_info->disposed_generation_count = 0;
    dds_info->no_writers_generation_count = 1;
    dds_info->sample_rank = 0;
    dds_info->generation_rank = 0;
    dds_info->absoulte_generation_rank = 0;
    dds_info->source_timestamp = rtps_info.sourceTimestamp;
    dds_info->instance_handle = rtps_info.iHandle;
    dds_info->publication_handle = fastrtps::rtps::InstanceHandle_t(rtps_info.sample_identity.writer_guid());
    dds_info->sample_identity = rtps_info.sample_identity;
    dds_info->related_sample_identity = rtps_info.related_sample_identity;
    dds_info->valid_data = rtps_info.sampleKind == eprosima::fastrtps::rtps::ALIVE ? true : false;

    switch (rtps_info.sampleKind)
    {
        case eprosima::fastrtps::rtps::ALIVE:
            dds_info->instance_state = ALIVE_INSTANCE_STATE;
            break;
        case eprosima::fastrtps::rtps::NOT_ALIVE_DISPOSED:
            dds_info->instance_state = NOT_ALIVE_DISPOSED_INSTANCE_STATE;
            break;
        default:
            //TODO [ILG] change this if the other kinds ever get implemented
            dds_info->instance_state = ALIVE_INSTANCE_STATE;
            break;
    }
}

DataReaderImpl::DataReaderImpl(
        SubscriberImpl* s,
        TypeSupport& type,
        TopicDescription* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener)
    : subscriber_(s)
    , type_(type)
    , topic_(topic)
    , qos_(&qos == &DATAREADER_QOS_DEFAULT ? subscriber_->get_default_datareader_qos() : qos)
#pragma warning (disable : 4355 )
    , history_(topic_attributes(),
            type_.get(),
            qos_.get_readerqos(subscriber_->get_qos()),
            type_->m_typeSize + 3,    /* Possible alignment */
            qos_.endpoint().history_memory_policy)
    , listener_(listener)
    , reader_listener_(this)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
    , sample_info_pool_(qos)
    , loan_manager_(qos)
{
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

    if (qos_.endpoint().entity_id > 0)
    {
        att.endpoint.setEntityID(static_cast<uint8_t>(qos_.endpoint().entity_id));
    }

    if (qos_.endpoint().user_defined_id > 0)
    {
        att.endpoint.setUserDefinedID(static_cast<uint8_t>(qos_.endpoint().user_defined_id));
    }

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
    property.value(topic_->get_name().c_str());
    att.endpoint.properties.properties().push_back(std::move(property));
    if (subscriber_->get_qos().partition().names().size() > 0)
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

    std::shared_ptr<IPayloadPool> pool = get_payload_pool();
    RTPSReader* reader = RTPSDomain::createRTPSReader(
        subscriber_->rtps_participant(),
        att, pool,
        static_cast<ReaderHistory*>(&history_),
        static_cast<ReaderListener*>(&reader_listener_));

    if (reader == nullptr)
    {
        release_payload_pool();
        logError(DATA_READER, "Problem creating associated Reader");
        return ReturnCode_t::RETCODE_ERROR;
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
    subscriber_->rtps_participant()->registerReader(reader_, topic_attributes(), rqos);

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

DataReaderImpl::~DataReaderImpl()
{
    delete lifespan_timer_;
    delete deadline_timer_;

    if (reader_ != nullptr)
    {
        logInfo(DATA_READER, guid().entityId << " in topic: " << topic_->get_name());
        RTPSDomain::removeRTPSReader(reader_);
        release_payload_pool();
    }

    delete user_datareader_;
}

bool DataReaderImpl::wait_for_unread_message(
        const fastrtps::Duration_t& timeout)
{
    return reader_ ? reader_->wait_for_unread_cache(timeout) : false;
}

ReturnCode_t DataReaderImpl::check_collection_preconditions_and_calc_max_samples(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t& max_samples)
{
    // Properties should be the same on both collections
    if ((data_values.has_ownership() != sample_infos.has_ownership()) ||
            (data_values.maximum() != sample_infos.maximum()) ||
            (data_values.length() != sample_infos.length()))
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

        uint32_t collection_input_max = data_values.maximum();
        int32_t collection_max = (collection_input_max > std::numeric_limits<uint32_t>::max() / 2u) ?
                std::numeric_limits<int32_t>::max() : static_cast<int32_t>(collection_input_max);

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

    // Check if there are enough loans
    ReturnCode_t code = loan_manager_.get_loan(data_values, sample_infos);
    if (!code)
    {
        return code;
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::read(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
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

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    auto instance = history_.lookup_instance(HANDLE_NIL, false).second;
    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, instance);
    while (!cmd.is_finished())
    {
        cmd.add_instance(false);
    }
    return cmd.return_value();
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
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    ReturnCode_t code = check_collection_preconditions_and_calc_max_samples(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    // Check if the instance exists
    auto it = history_.lookup_instance(a_handle, true);
    if (!it.first)
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, it.second,
            true);
    while (!cmd.is_finished())
    {
        cmd.add_instance(false);
    }
    return cmd.return_value();
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
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    ReturnCode_t code = check_collection_preconditions_and_calc_max_samples(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    auto instance = history_.lookup_instance(previous_handle, false).second;
    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, instance);
    while (!cmd.is_finished())
    {
        if (cmd.add_instance(false))
        {
            break;
        }
    }
    return cmd.return_value();
}

ReturnCode_t DataReaderImpl::take(
        LoanableCollection& data_values,
        SampleInfoSeq& sample_infos,
        int32_t max_samples,
        SampleStateMask sample_states,
        ViewStateMask view_states,
        InstanceStateMask instance_states)
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

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    auto instance = history_.lookup_instance(HANDLE_NIL, false).second;
    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, instance);
    while (!cmd.is_finished())
    {
        cmd.add_instance(true);
    }
    return cmd.return_value();
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
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    ReturnCode_t code = check_collection_preconditions_and_calc_max_samples(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    // Check if the instance exists
    auto it = history_.lookup_instance(a_handle, true);
    if (!it.first)
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, it.second,
            true);
    while (!cmd.is_finished())
    {
        cmd.add_instance(true);
    }
    return cmd.return_value();
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
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    ReturnCode_t code = check_collection_preconditions_and_calc_max_samples(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    code = prepare_loan(data_values, sample_infos, max_samples);
    if (!code)
    {
        return code;
    }

    auto instance = history_.lookup_instance(previous_handle, false).second;
    detail::StateFilter states{ sample_states, view_states, instance_states };
    detail::ReadTakeCommand cmd(*this, data_values, sample_infos, max_samples, states, instance);
    while (!cmd.is_finished())
    {
        if (cmd.add_instance(true))
        {
            break;
        }
    }
    return cmd.return_value();
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
    if ((data_values.has_ownership() != sample_infos.has_ownership()) ||
            (data_values.maximum() != sample_infos.maximum()) ||
            (data_values.length() != sample_infos.length()))
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
            if (!type_->is_plain())
            {
                // Return loaned sample
                type_->deleteData(data_values.buffer()[n]);
            }
        }

        sample_info_pool_.return_item(&sample_infos[n]);
    }

    data_values.unloan();
    sample_infos.unloan();

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::read_next_sample(
        void* data,
        SampleInfo* info)
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
    SampleInfo_t rtps_info;
    if (history_.readNextData(data, &rtps_info, max_blocking_time))
    {
        sample_info_to_dds(rtps_info, info);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataReaderImpl::take_next_sample(
        void* data,
        SampleInfo* info)
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

    SampleInfo_t rtps_info;
    if (history_.takeNextData(data, &rtps_info, max_blocking_time))
    {
        sample_info_to_dds(rtps_info, info);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataReaderImpl::get_first_untaken_info(
        SampleInfo* info)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    SampleInfo_t rtps_info;
    if (history_.get_first_untaken_info(&rtps_info))
    {
        sample_info_to_dds(rtps_info, info);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_NO_DATA;
}

const GUID_t& DataReaderImpl::guid() const
{
    return reader_ ? reader_->getGuid() : c_Guid_Unknown;
}

InstanceHandle_t DataReaderImpl::get_instance_handle() const
{
    return guid();
}

void DataReaderImpl::subscriber_qos_updated()
{
    if (reader_)
    {
        //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
        ReaderQos rqos = qos_.get_readerqos(get_subscriber()->get_qos());
        subscriber_->rtps_participant()->updateReader(reader_, topic_attributes(), rqos);
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
        //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
        ReaderQos rqos = qos.get_readerqos(get_subscriber()->get_qos());
        subscriber_->rtps_participant()->updateReader(reader_, topic_attributes(), rqos);

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
                listener->on_data_available(data_reader_->user_datareader_);
            }
        }
    }
}

void DataReaderImpl::InnerDataReaderListener::onReaderMatched(
        RTPSReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    DataReaderListener* listener = data_reader_->get_listener_for(StatusMask::subscription_matched());
    if (listener != nullptr)
    {
        listener->on_subscription_matched(data_reader_->user_datareader_, info);
    }
}

void DataReaderImpl::InnerDataReaderListener::on_liveliness_changed(
        RTPSReader* /*reader*/,
        const fastrtps::LivelinessChangedStatus& status)
{
    data_reader_->update_liveliness_status(status);
    DataReaderListener* listener = data_reader_->get_listener_for(StatusMask::liveliness_changed());
    if (listener != nullptr)
    {
        LivelinessChangedStatus callback_status;
        if (data_reader_->get_liveliness_changed_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_liveliness_changed(data_reader_->user_datareader_, callback_status);
        }
    }
}

void DataReaderImpl::InnerDataReaderListener::on_requested_incompatible_qos(
        RTPSReader* /*reader*/,
        fastdds::dds::PolicyMask qos)
{
    data_reader_->update_requested_incompatible_qos(qos);
    DataReaderListener* listener = data_reader_->get_listener_for(StatusMask::requested_incompatible_qos());
    if (listener != nullptr)
    {
        RequestedIncompatibleQosStatus callback_status;
        if (data_reader_->get_requested_incompatible_qos_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_requested_incompatible_qos(data_reader_->user_datareader_, callback_status);
        }
    }
}

bool DataReaderImpl::on_new_cache_change_added(
        const CacheChange_t* const change)
{
    if (qos_.deadline().period != c_TimeInfinite)
    {
        std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

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
            // The new change has been added at the begining of the the history
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
    listener_->on_requested_deadline_missed(user_datareader_, deadline_missed_status_);
    subscriber_->subscriber_listener_.on_requested_deadline_missed(user_datareader_, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

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

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
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

/* TODO
   bool DataReaderImpl::read(
        std::vector<void *>& data_values,
        std::vector<SampleInfo_t>& sample_infos,
        uint32_t max_samples)
   {
    (void)data_values;
    (void)sample_infos;
    (void)max_samples;
    // TODO Implement
    return false;
   }

   bool DataReaderImpl::take(
        std::vector<void *>& data_values,
        std::vector<SampleInfo_t>& sample_infos,
        uint32_t max_samples)
   {
    (void)data_values;
    (void)sample_infos;
    (void)max_samples;
    // TODO Implement
    return false;
   }
 */

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

    std::lock_guard<RecursiveTimedMutex> lock(reader_->getMutex());

    status = liveliness_changed_status_;
    liveliness_changed_status_.alive_count_change = 0u;
    liveliness_changed_status_.not_alive_count_change = 0u;

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status)
{
    if (reader_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    status = requested_incompatible_qos_status_;
    requested_incompatible_qos_status_.total_count_change = 0u;
    return ReturnCode_t::RETCODE_OK;
}

/* TODO
   bool DataReaderImpl::get_sample_lost_status(
        SampleLostStatus& status) const
   {
    (void)status;
    // TODO Implement
    // TODO add callback call subscriber_->subscriber_listener_->on_sample_lost
    return false;
   }
 */

/* TODO
   bool DataReaderImpl::get_sample_rejected_status(
        SampleRejectedStatus& status) const
   {
    (void)status;
    // TODO Implement
    // TODO add callback call subscriber_->subscriber_listener_->on_sample_rejected
    return false;
   }
 */

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
    liveliness_changed_status_.alive_count = status.alive_count;
    liveliness_changed_status_.not_alive_count = status.not_alive_count;
    liveliness_changed_status_.alive_count_change += status.alive_count_change;
    liveliness_changed_status_.not_alive_count_change += status.not_alive_count_change;
    liveliness_changed_status_.last_publication_handle = status.last_publication_handle;

    return liveliness_changed_status_;
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
}

fastrtps::TopicAttributes DataReaderImpl::topic_attributes() const
{
    fastrtps::TopicAttributes topic_att;
    topic_att.topicKind = type_->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    topic_att.topicName = topic_->get_name();
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
    PoolConfig config = PoolConfig::from_history_attributes(history_.m_att );

    if (!payload_pool_)
    {
        payload_pool_ = TopicPayloadPoolRegistry::get(topic_->get_name(), config);
    }

    payload_pool_->reserve_history(config, true);
    return payload_pool_;
}

void DataReaderImpl::release_payload_pool()
{
    assert(payload_pool_);

    PoolConfig config = PoolConfig::from_history_attributes(history_.m_att);
    payload_pool_->release_history(config, true);

    TopicPayloadPoolRegistry::release(payload_pool_);
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
