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

#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace dds {

DataReaderImpl::DataReaderImpl(
        SubscriberImpl* s,
        TypeSupport type,
        const TopicAttributes& topic_att,
        const fastrtps::rtps::ReaderAttributes& att,
        const DataReaderQos& qos,
        const MemoryManagementPolicy_t memory_policy,
        DataReaderListener* listener)
    : subscriber_(s)
    , reader_(nullptr)
    , type_(type)
    , topic_att_(topic_att)
    , att_(att)
    , qos_(&qos == &DATAREADER_QOS_DEFAULT ? subscriber_->get_default_datareader_qos() : qos)
#pragma warning (disable : 4355 )
    , history_(topic_att_,
            type_.get(),
            qos_.get_readerqos(subscriber_->get_qos()),
            type_->m_typeSize + 3,    /* Possible alignment */
            memory_policy)
    , listener_(listener)
    , reader_listener_(this)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , deadline_missed_status_()
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
    , user_datareader_(nullptr)
{
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
        logInfo(DATA_READER, guid().entityId << " in topic: " << topic_att_.topicName);
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
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));
#else
            std::chrono::hours(24);
#endif
    if (history_.readNextData(data, info, max_blocking_time))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataReaderImpl::take_next_sample(
        void* data,
        SampleInfo_t* info)
{
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));
#else
            std::chrono::hours(24);
#endif
    if (history_.takeNextData(data, info, max_blocking_time))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataReaderImpl::get_first_untaken_info(
        SampleInfo_t* info)
{
    return history_.get_first_untaken_info(info) ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_NO_DATA;
}

const GUID_t& DataReaderImpl::guid()
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
    ReturnCode_t check_result = check_qos(qos);
    if (!check_result)
    {
        return check_result;
    }
    else if (!can_qos_be_updated(qos_, qos))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }

    set_qos(qos_, qos, false);

    //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
    ReaderQos rqos = qos.get_readerqos(get_subscriber()->get_qos());
    subscriber_->rtps_participant()->updateReader(reader_, topic_att_, rqos);

    // Deadline
    if (qos_.deadline().period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000> >(qos_.deadline().period.to_ns() * 1e-3);
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
                std::chrono::duration<double, std::ratio<1, 1000000> >(qos_.lifespan().duration.to_ns() * 1e-3);
        lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
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
        const TopicAttributes& topic_att)
{
    //TOPIC ATTRIBUTES
    if (topic_att_ != topic_att)
    {
        logWarning(RTPS_READER, "Topic Attributes cannot be updated");
        return false;
    }
    //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
    //subscriber_->update_reader(this, topic_att_, qos_);
    //subscriber_->rtps_participant()->updateReader(reader_, topic_att_, qos_);
    return true;
}

const TopicAttributes& DataReaderImpl::get_topic() const
{
    return topic_att_;
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
        if (data_reader_->listener_ != nullptr)
        {
            data_reader_->listener_->on_data_available(data_reader_->user_datareader_);
        }

        data_reader_->subscriber_->subscriber_listener_.on_data_available(data_reader_->user_datareader_);
    }
}

void DataReaderImpl::InnerDataReaderListener::onReaderMatched(
        RTPSReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (data_reader_->listener_ != nullptr)
    {
        data_reader_->listener_->on_subscription_matched(data_reader_->user_datareader_, info);
    }

    data_reader_->subscriber_->subscriber_listener_.on_subscription_matched(data_reader_->user_datareader_, info);
}

void DataReaderImpl::InnerDataReaderListener::on_liveliness_changed(
        RTPSReader* /*reader*/,
        const fastrtps::LivelinessChangedStatus& status)
{
    if (data_reader_->listener_ != nullptr)
    {
        data_reader_->listener_->on_liveliness_changed(data_reader_->user_datareader_, status);
    }

    data_reader_->subscriber_->subscriber_listener_.on_liveliness_changed(data_reader_->user_datareader_, status);
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
        LivelinessChangedStatus& status) const
{
    std::unique_lock<RecursiveTimedMutex> lock(reader_->getMutex());

    status = reader_->liveliness_changed_status_;

    reader_->liveliness_changed_status_.alive_count_change = 0u;
    reader_->liveliness_changed_status_.not_alive_count_change = 0u;
    // TODO add callback call subscriber_->subscriber_listener_->on_liveliness_changed
    return ReturnCode_t::RETCODE_OK;
}

/* TODO
   bool DataReaderImpl::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status) const
   {
    (void)status;
    // TODO Implement
    // TODO add callback call subscriber_->subscriber_listener_->on_requested_incompatibe_qos
    return false;
   }
 */

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

TypeSupport DataReaderImpl::type()
{
    return type_;
}

ReturnCode_t DataReaderImpl::check_qos (const DataReaderQos& qos)
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
    if (to.topicData().getValue() != from.topicData().getValue())
    {
        to.topicData() = from.topicData();
        to.topicData().hasChanged = true;
    }
    if (first_time || !(to.durabilityService() == from.durabilityService()))
    {
        to.durabilityService() = from.durabilityService();
        to.durabilityService().hasChanged = true;
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

    if (to.expectsInlineQos() != from.expectsInlineQos())
    {
        to.expectsInlineQos(from.expectsInlineQos());
    }

    if (first_time && !(to.properties() == from.properties()))
    {
        to.properties() = from.properties();
    }

    if (first_time && !(to.enpoint() == from.enpoint()))
    {
        to.enpoint() = from.enpoint();
    }

    if (first_time && !(to.reader_resource_limits() == from.reader_resource_limits()))
    {
        to.reader_resource_limits() = from.reader_resource_limits();
    }
}


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
