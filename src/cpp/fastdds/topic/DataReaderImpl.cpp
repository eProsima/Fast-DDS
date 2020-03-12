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

#include <fastdds/topic/DataReaderImpl.hpp>
#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

using DeprecatedSampleInfo = eprosima::fastrtps::SampleInfo_t;

namespace eprosima {
namespace fastdds {
namespace dds {

DataReaderImpl::DataReaderImpl(
        SubscriberImpl* s,
        TypeSupport type,
        Topic* topic,
        const fastrtps::rtps::ReaderAttributes& att,
        const DataReaderQos& qos,
        const MemoryManagementPolicy_t memory_policy,
        DataReaderListener* listener)
    : subscriber_(s)
    , reader_(nullptr)
    , type_(type)
    , topic_(topic)
    , att_(att)
    , qos_(&qos == &DDS_DATAREADER_QOS_DEFAULT ? subscriber_->get_default_datareader_qos() : qos)
    , rqos_(qos_.changeToReaderQos())
#pragma warning (disable : 4355 )
    , history_(topic->get_topic_attributes(),
            type_.get(),
            rqos_,
            type_.get()->m_typeSize + 3,    /* Possible alignment */
            memory_policy)
    , listener_(listener)
    , reader_listener_(this)
    , deadline_duration_us_(qos_.deadline.period.to_ns() * 1e-3)
    , deadline_missed_status_()
    , lifespan_duration_us_(topic_->get_qos().lifespan.duration.to_ns() * 1e-3)
    , user_datareader_(nullptr)
{
    deadline_timer_ = new TimedEvent(subscriber_->get_participant().get_resource_event(),
                    [&]() -> bool
                {
                    return deadline_missed();
                },
                    qos_.deadline.period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(subscriber_->get_participant().get_resource_event(),
                    [&]() -> bool
                {
                    return lifespan_expired();
                },
                    topic_->get_qos().lifespan.duration.to_ns() * 1e-6);

    RTPSReader* reader = RTPSDomain::createRTPSReader(
        subscriber_->rtps_participant(),
        att_,
        static_cast<ReaderHistory*>(&history_),
        static_cast<ReaderListener*>(&reader_listener_));

    if (reader == nullptr)
    {
        logError(DATA_READER, "Problem creating associated Reader");
    }

    reader_ = reader;
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
    }

    RTPSDomain::removeRTPSReader(reader_);
    delete user_datareader_;
}

bool DataReaderImpl::wait_for_unread_message(
        const fastrtps::Duration_t& timeout)
{
    return reader_->wait_for_unread_cache(timeout);
}

ReturnCode_t DataReaderImpl::read_next_sample(
        void* data,
        SampleInfo_t* info)
{
    if (!user_datareader_->is_enabled())
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability.max_blocking_time));
#else
            std::chrono::hours(24);
#endif

    DeprecatedSampleInfo dep_info;
    if (history_.readNextData(data, &dep_info, max_blocking_time))
    {
        // Transform SampleInfo
        info->valid_data = dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::ALIVE;
        info->view_state = ::dds::sub::status::ViewState::new_view();
        if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::ALIVE)
        {
            info->instance_state = ::dds::sub::status::InstanceState::alive();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_disposed();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_no_writers();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_mask();
        }
        info->sample_state = ::dds::sub::status::SampleState::read();
        info->sample_rank = static_cast<int32_t>(dep_info.sample_identity.sequence_number().to64long()); // ??
        info->generation_rank = 0;
        info->source_timestamp = dep_info.sourceTimestamp.to_duration_t();
        info->publication_handle = dep_info.sample_identity.writer_guid();
        info->absolute_generation_rank = 0;
        info->disposed_generation_count = 0;
        info->no_writers_generation_count = 0;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataReaderImpl::take_next_sample(
        void* data,
        SampleInfo_t* info)
{
    if (!user_datareader_->is_enabled())
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability.max_blocking_time));
#else
            std::chrono::hours(24);
#endif

    DeprecatedSampleInfo dep_info;
    if (history_.takeNextData(data, &dep_info, max_blocking_time))
    {
        // Transform SampleInfo
        info->valid_data = dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::ALIVE;
        info->view_state = ::dds::sub::status::ViewState::new_view();
        if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::ALIVE)
        {
            info->instance_state = ::dds::sub::status::InstanceState::alive();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_disposed();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_no_writers();
        }
        else if (dep_info.sampleKind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_mask();
        }
        info->sample_state = ::dds::sub::status::SampleState::read();
        info->sample_rank = static_cast<int32_t>(dep_info.sample_identity.sequence_number().to64long()); // ??
        info->generation_rank = 0;
        info->source_timestamp = dep_info.sourceTimestamp.to_duration_t();
        info->publication_handle = dep_info.sample_identity.writer_guid();
        info->absolute_generation_rank = 0;
        info->disposed_generation_count = 0;
        info->no_writers_generation_count = 0;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

const GUID_t& DataReaderImpl::guid() const
{
    return reader_->getGuid();
}

InstanceHandle_t DataReaderImpl::get_instance_handle() const
{
    InstanceHandle_t handle;
    handle = reader_->getGuid();
    return handle;
}

ReturnCode_t DataReaderImpl::set_qos(
        const DataReaderQos& qos)
{
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if (!qos.checkQos())
    {
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    else if (!qos_.canQosBeUpdated(qos))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }

    qos_.setQos(qos, false);
    //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
    ReaderQos rqos = qos_.changeToReaderQos();
    subscriber_->rtps_participant()->updateReader(reader_, topic_->get_topic_attributes(), rqos);

    // Deadline
    if (qos_.deadline.period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000> >(qos_.deadline.period.to_ns() * 1e-3);
        deadline_timer_->update_interval_millisec(qos_.deadline.period.to_ns() * 1e-6);
    }
    else
    {
        deadline_timer_->cancel_timer();
    }

    // Lifespan
    if (topic_->get_qos().lifespan.duration != c_TimeInfinite)
    {
        lifespan_duration_us_ =
                std::chrono::duration<double,
                        std::ratio<1, 1000000> >(topic_->get_qos().lifespan.duration.to_ns() * 1e-3);
        lifespan_timer_->update_interval_millisec(topic_->get_qos().lifespan.duration.to_ns() * 1e-6);
    }
    else
    {
        lifespan_timer_->cancel_timer();
    }

    return ReturnCode_t::RETCODE_OK;
}

const DataReaderQos& DataReaderImpl::get_qos() const
{
    return qos_;
}

bool DataReaderImpl::set_topic(
        Topic& topic)
{
    topic_ = &topic;
    return true;
}

Topic* DataReaderImpl::get_topic() const
{
    return topic_;
}

TopicDescription* DataReaderImpl::get_topicdescription()
{
    DomainParticipant dp = get_subscriber()->get_participant();
    return dp.lookup_topicdescription(topic_->get_name());
}

bool DataReaderImpl::set_attributes(
        const fastrtps::rtps::ReaderAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if (att.endpoint.unicastLocatorList.size() != att_.endpoint.unicastLocatorList.size() ||
            att.endpoint.multicastLocatorList.size() != att_.endpoint.multicastLocatorList.size())
    {
        logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
        updated &= false;
    }
    else
    {
        for (LocatorListConstIterator lit1 = att_.endpoint.unicastLocatorList.begin();
                lit1 != att_.endpoint.unicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.endpoint.unicastLocatorList.begin();
                    lit2 != att.endpoint.unicastLocatorList.end(); ++lit2)
            {
                if (*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if (missing)
            {
                logWarning(RTPS_READER, "Locator: " << *lit1 << " not present in new list");
                logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
            }
        }
        for (LocatorListConstIterator lit1 = att_.endpoint.multicastLocatorList.begin();
                lit1 != att_.endpoint.multicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.endpoint.multicastLocatorList.begin();
                    lit2 != att.endpoint.multicastLocatorList.end(); ++lit2)
            {
                if (*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if (missing)
            {
                logWarning(RTPS_READER, "Locator: " << *lit1 << " not present in new list");
                logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
            }
        }
    }

    if (updated)
    {
        att_.expectsInlineQos = att.expectsInlineQos;
    }

    return updated;
}

const ReaderAttributes& DataReaderImpl::get_attributes() const
{
    return att_;
}

void DataReaderImpl::InnerDataReaderListener::onNewCacheChangeAdded(
        RTPSReader* /*reader*/,
        const CacheChange_t* const change_in)
{
    if (data_reader_->on_new_cache_change_added(change_in))
    {
        if (data_reader_->get_subscriber()->get_status_mask().is_compatible(::dds::core::status::StatusMask::
                data_on_readers()) && data_reader_->subscriber_->get_subscriber()->is_enabled())
        {
            if (data_reader_->subscriber_->listener_ != nullptr)
            {
                data_reader_->subscriber_->listener_->on_data_on_readers(data_reader_->get_subscriber());
            }
            data_reader_->subscriber_->participant_->subscriber_listener_.on_data_on_readers(
                data_reader_->get_subscriber());

        }
        else if (data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(::dds::core::status::
                StatusMask::data_on_readers()) && data_reader_->subscriber_->get_participant().is_enabled())
        {
            if (data_reader_->subscriber_->get_participant().get_listener() != nullptr)
            {
                data_reader_->subscriber_->get_participant().get_listener()->on_data_on_readers(
                    data_reader_->get_subscriber());
            }
            data_reader_->subscriber_->participant_->subscriber_listener_.on_data_on_readers(
                data_reader_->get_subscriber());

        }
        else if (data_reader_->listener_ != nullptr &&
                data_reader_->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::
                data_available()) && data_reader_->user_datareader_->is_enabled())
        {
            data_reader_->listener_->on_data_available(data_reader_->user_datareader_);
        }
        else if (data_reader_->subscriber_->get_participant().get_listener() != nullptr &&
                data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(::dds::core::status::
                StatusMask::data_available()) && data_reader_->subscriber_->get_participant().is_enabled())
        {
            data_reader_->subscriber_->get_participant().get_listener()->on_data_available(
                data_reader_->user_datareader_);
        }
    }
}

void DataReaderImpl::InnerDataReaderListener::onReaderMatched(
        RTPSReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
	bool matched = false;
	
    //Update Matched Publications List
    if (info.current_count_change == 1) //MATCHED_MATCHING
    {
        data_reader_->matched_publications_.push_back(info.last_publication_handle);
    }
    else if (info.current_count_change == -1) //REMOVE_MATCHING
    {
        auto it = std::find(data_reader_->matched_publications_.begin(),
                        data_reader_->matched_publications_.end(), info.last_publication_handle);
        if (it != data_reader_->matched_publications_.end())
        {
            data_reader_->matched_publications_.erase(it);
        }
        BuiltinSubscriber::get_instance()->delete_publication_data(info.last_publication_handle);
    }

    //TODO: Check if the DataWriter should be ignored (DomainParticipant::ignore_publication)
    if (data_reader_->listener_ != nullptr && data_reader_->user_datareader_->is_enabled() &&
            data_reader_->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::
            subscription_matched()))
    {
        matched = true;
        data_reader_->listener_->on_subscription_matched(data_reader_->user_datareader_, info);
    }

    else if (data_reader_->subscriber_->get_participant().get_listener() != nullptr &&
            data_reader_->subscriber_->get_participant().is_enabled() &&
            data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(
                ::dds::core::status::StatusMask::subscription_matched()))
    {
        matched = true;
        data_reader_->subscriber_->get_participant().get_listener()->
        on_subscription_matched(data_reader_->user_datareader_, info);
    }

    //Update Matched Publications List
    if (info.current_count_change == 1 && matched) //MATCHED_MATCHING
    {
        data_reader_->matched_publications_.push_back(info.last_publication_handle);
    }
    else if (info.current_count_change == -1 && matched) //REMOVE_MATCHING
    {
        auto it = std::find(data_reader_->matched_publications_.begin(),
                        data_reader_->matched_publications_.end(), info.last_publication_handle);
        if (it != data_reader_->matched_publications_.end())
        {
            data_reader_->matched_publications_.erase(it);
        }
        BuiltinSubscriber::get_instance()->delete_publication_data(info.last_publication_handle);
    }
}

void DataReaderImpl::InnerDataReaderListener::on_liveliness_changed(
        RTPSReader* /*reader*/,
        const fastrtps::LivelinessChangedStatus& status)
{
    if (data_reader_->listener_ != nullptr && data_reader_->user_datareader_->is_enabled() &&
            data_reader_->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::
            liveliness_changed()))
    {
        data_reader_->listener_->on_liveliness_changed(data_reader_->user_datareader_, status);
    }

    else if (data_reader_->subscriber_->get_participant().get_listener() != nullptr &&
            data_reader_->subscriber_->get_participant().is_enabled() &&
            data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(
                ::dds::core::status::StatusMask::liveliness_changed()))
    {
        data_reader_->subscriber_->get_participant().get_listener()->
        on_liveliness_changed(data_reader_->user_datareader_, status);
    }

}

void DataReaderImpl::InnerDataReaderListener::on_requested_incompatible_qos(
        RTPSReader* /*reader*/,
        const RequestedIncompatibleQosStatus& status)
{
    if (data_reader_->listener_ != nullptr && data_reader_->user_datareader_->is_enabled() &&
            (data_reader_->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::
            requested_incompatible_qos())))
    {
        data_reader_->listener_->on_requested_incompatible_qos(data_reader_->user_datareader_, status);
    }

    else if (data_reader_->subscriber_->get_participant().get_listener() != nullptr &&
            data_reader_->subscriber_->get_participant().is_enabled() &&
            data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(
                ::dds::core::status::StatusMask::requested_incompatible_qos()))
    {
        data_reader_->subscriber_->get_participant().get_listener()->
        on_requested_incompatible_qos(data_reader_->user_datareader_,
                status);
    }
}

void DataReaderImpl::InnerDataReaderListener::on_sample_rejected(
        RTPSReader* /*reader*/,
        const SampleRejectedStatus& status)
{
    if (data_reader_->listener_ != nullptr && data_reader_->user_datareader_->is_enabled() &&
            (data_reader_->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::
            sample_rejected())))
    {
        data_reader_->listener_->on_sample_rejected(data_reader_->user_datareader_, status);
    }

    else if (data_reader_->subscriber_->get_participant().get_listener() != nullptr &&
            data_reader_->subscriber_->get_participant().is_enabled() &&
            data_reader_->subscriber_->get_participant().get_status_mask().is_compatible(
                ::dds::core::status::StatusMask::sample_rejected()))
    {
        data_reader_->subscriber_->get_participant().get_listener()->
        on_sample_rejected(data_reader_->user_datareader_,
                status);
    }
}

bool DataReaderImpl::on_new_cache_change_added(
        const CacheChange_t* const change)
{
    if (qos_.deadline.period != c_TimeInfinite)
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

    //    if (qos_.m_lifespan.duration == c_TimeInfinite)
    //    {
    //        return true;
    //    }

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
    assert(qos_.deadline.period != c_TimeInfinite);

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
    assert(qos_.deadline.period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;

    if (listener_ != nullptr && user_datareader_->is_enabled() &&
            user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::requested_deadline_missed()))
    {
        listener_->on_requested_deadline_missed(user_datareader_, deadline_missed_status_);
    }

    else if (subscriber_->get_participant().get_listener() != nullptr && subscriber_->get_participant().is_enabled() &&
            subscriber_->get_participant().get_status_mask().is_compatible(::dds::core::status::StatusMask::
            requested_deadline_missed()))
    {
        subscriber_->get_participant().get_listener()->
        on_requested_deadline_missed(user_datareader_, deadline_missed_status_);
    }

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
    if (!user_datareader_->is_enabled())
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
    if (!history_.get_earliest_change(&earliest_change))
    {
        return false;
    }

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

    assert(interval.count() > 0);

    lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
    return true;
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
        LivelinessChangedStatus& status) const
{
    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    status = reader_->liveliness_changed_status_;

    reader_->liveliness_changed_status_.alive_count_change = 0u;
    reader_->liveliness_changed_status_.not_alive_count_change = 0u;
    // TODO add callback call subscriber_->subscriber_listener_->on_liveliness_changed
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataReaderImpl::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status) const
{
    status = reader_->requested_incompatible_qos_status_;
    reader_->requested_incompatible_qos_status_.total_count_change = 0;
    return ReturnCode_t::RETCODE_OK;
}

/* TODO
   bool DataReaderImpl::get_sample_lost_status(
        SampleLostStatus& status) const
   {
    if (data_reader_->listener_ != nullptr && (data_reader_->mask_ == ::dds::core::status::StatusMask::all() ||
            data_reader_->mask_ == ::dds::core::status::StatusMask::sample_lost()))
    {
        data_reader_->listener_->on_sample_lost(data_reader_->user_datareader_, status);
    }

    if (data_reader_->subscriber_->mask_ == ::dds::core::status::StatusMask::all() ||
            data_reader_->subscriber_->mask_ == ::dds::core::status::StatusMask::sample_lost())
    {
        data_reader_->subscriber_->subscriber_listener_.on_sample_lost(data_reader_->user_datareader_,
                status);
    }
   }
 */

ReturnCode_t DataReaderImpl::get_sample_rejected_status(
        SampleRejectedStatus& status) const
{
    status = reader_->sample_rejected_status_;
    return ReturnCode_t::RETCODE_OK;
}

types::ReturnCode_t DataReaderImpl::get_subscription_matched_status(
        SubscriptionMatchedStatus& status) const
{
    status = reader_->subscription_matched_status_;
    return ReturnCode_t::RETCODE_OK;
}

Subscriber* DataReaderImpl::get_subscriber() const
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

TypeSupport DataReaderImpl::type()
{
    return type_;
}

ReturnCode_t DataReaderImpl::get_matched_publication_data(
        PublicationBuiltinTopicData& publication_data,
        const fastrtps::rtps::InstanceHandle_t& publication_handle) const
{
    PublicationBuiltinTopicData* data = BuiltinSubscriber::get_instance()->get_publication_data(publication_handle);
    if (data != nullptr)
    {
        publication_data = *data;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t DataReaderImpl::get_matched_publications(
        std::vector<InstanceHandle_t>& publication_handles) const
{
    publication_handles = matched_publications_;
    return ReturnCode_t::RETCODE_OK;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
