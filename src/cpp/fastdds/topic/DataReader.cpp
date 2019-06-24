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

/**
 * @file DataReader.cpp
 *
 */

#include <fastdds/topic/DataReader.hpp>
#include "../../fastrtps/subscriber/SubscriberImpl.h"
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps//participant/Participant.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {


DataReader::DataReader(
        SubscriberImpl* s,
        TopicDataType* ptype,
        const TopicAttributes& topic_att,
        const rtps::ReaderAttributes& att,
        const ReaderQos& qos,
        SubscriberHistory&& history,
        DataReaderListener* listener)
    : subscriber_(s)
    , reader_(nullptr)
    , type_(ptype)
    , topic_att_(topic_att)
    , att_(att)
    , qos_(qos)
#pragma warning (disable : 4355 )
    , history_(std::move(history))
    , listener_(listener)
    , reader_listener_(this)
    , deadline_timer_(std::bind(&DataReader::deadline_missed, this),
                      qos.m_deadline.period.to_ns() * 1e-6,
                      subscriber_->get_participant()->get_resource_event().getIOService(),
                      subscriber_->get_participant()->get_resource_event().getThread())
    , deadline_duration_us_(qos.m_deadline.period.to_ns() * 1e-3)
    , deadline_missed_status_()
    , lifespan_timer_(std::bind(&DataReader::lifespan_expired, this),
                      qos.m_lifespan.duration.to_ns() * 1e-6,
                      subscriber_->get_participant()->get_resource_event().getIOService(),
                      subscriber_->get_participant()->get_resource_event().getThread())
    , lifespan_duration_us_(qos.m_lifespan.duration.to_ns() * 1e-3)
{
    RTPSReader* reader = RTPSDomain::createRTPSReader(
        subscriber_->rtps_participant(),
        att_,
        static_cast<ReaderHistory*>(&history_),
        static_cast<ReaderListener*>(&reader_listener_));

    if(reader == nullptr)
    {
        logError(DATA_READER, "Problem creating associated Reader");
    }

    reader_ = reader;
}

DataReader::~DataReader()
{
    if(reader_ != nullptr)
    {
        logInfo(DATA_READER, guid().entityId << " in topic: " << topic_att_.topicName);
    }

    RTPSDomain::removeRTPSReader(reader_);
}

void DataReader::wait_for_unread_message()
{
    if(history_.getUnreadCount()==0)
    {
        do
        {
            history_.waitSemaphore();
        }
        while(history_.getUnreadCount() == 0);
    }
}

bool DataReader::read_next_sample(
        void *data,
        SampleInfo_t *info)
{
    return history_.readNextData(data,info);
}

bool DataReader::take_next_sample(
        void *data,
        SampleInfo_t *info)
{
    return history_.takeNextData(data,info);
}

const GUID_t& DataReader::guid()
{
    return reader_->getGuid();
}

bool DataReader::set_qos(
        const ReaderQos& qos)
{
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if(!qos_.canQosBeUpdated(qos))
    {
        return false;
    }

    qos_.setQos(qos,false);
    //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
    //subscriber_->update_reader(this, topic_att_, qos_);
    subscriber_->rtps_participant()->updateReader(reader_, topic_att_, qos_);

    // Deadline
    if (qos_.m_deadline.period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000>>(qos_.m_deadline.period.to_ns() * 1e-3);
        deadline_timer_.update_interval_millisec(qos_.m_deadline.period.to_ns() * 1e-6);
    }
    else
    {
        deadline_timer_.cancel_timer();
    }

    // Lifespan
    if (qos_.m_lifespan.duration != c_TimeInfinite)
    {
        lifespan_duration_us_ =
                std::chrono::duration<double, std::ratio<1, 1000000>>(qos_.m_lifespan.duration.to_ns() * 1e-3);
        lifespan_timer_.update_interval_millisec(qos_.m_lifespan.duration.to_ns() * 1e-6);
    }
    else
    {
        lifespan_timer_.cancel_timer();
    }

    return true;
}

bool DataReader::set_topic(
        const TopicAttributes& topic_att)
{
    //TOPIC ATTRIBUTES
    if(topic_att_ != topic_att)
    {
        logWarning(RTPS_READER,"Topic Attributes cannot be updated");
        return false;
    }
    //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
    //subscriber_->update_reader(this, topic_att_, qos_);
    subscriber_->rtps_participant()->updateReader(reader_, topic_att_, qos_);
    return true;
}

const TopicAttributes& DataReader::get_topic() const
{
    return topic_att_;
}

bool DataReader::set_attributes(
        const rtps::ReaderAttributes &att)
{
    bool updated = true;
    bool missing = false;
    if(att.endpoint.unicastLocatorList.size() != att_.endpoint.unicastLocatorList.size() ||
            att.endpoint.multicastLocatorList.size() != att_.endpoint.multicastLocatorList.size())
    {
        logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
        updated &= false;
    }
    else
    {
        for(LocatorListConstIterator lit1 = att_.endpoint.unicastLocatorList.begin();
                lit1!=att_.endpoint.unicastLocatorList.end();++lit1)
        {
            missing = true;
            for(LocatorListConstIterator lit2 = att.endpoint.unicastLocatorList.begin();
                    lit2!= att.endpoint.unicastLocatorList.end();++lit2)
            {
                if(*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if(missing)
            {
                logWarning(RTPS_READER,"Locator: "<< *lit1 << " not present in new list");
                logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
            }
        }
        for(LocatorListConstIterator lit1 = att_.endpoint.multicastLocatorList.begin();
                lit1!=att_.endpoint.multicastLocatorList.end();++lit1)
        {
            missing = true;
            for(LocatorListConstIterator lit2 = att.endpoint.multicastLocatorList.begin();
                    lit2!= att.endpoint.multicastLocatorList.end();++lit2)
            {
                if(*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if(missing)
            {
                logWarning(RTPS_READER,"Locator: "<< *lit1<< " not present in new list");
                logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
            }
        }
    }

    if(updated)
    {
        att_.expectsInlineQos = att.expectsInlineQos;
        if(qos_.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulReader* sfr = static_cast<StatefulReader*>(reader_);
            sfr->updateTimes(att.times);
        }

        // Deadline
        if (qos_.m_deadline.period != c_TimeInfinite)
        {
            deadline_duration_us_ =
                    duration<double, std::ratio<1, 1000000>>(qos_.m_deadline.period.to_ns() * 1e-3);
            deadline_timer_.update_interval_millisec(qos_.m_deadline.period.to_ns() * 1e-6);
        }
        else
        {
            deadline_timer_.cancel_timer();
        }

        // Lifespan
        if (qos_.m_lifespan.duration != c_TimeInfinite)
        {
            lifespan_duration_us_ =
                    std::chrono::duration<double, std::ratio<1, 1000000>>(qos_.m_lifespan.duration.to_ns() * 1e-3);
            lifespan_timer_.update_interval_millisec(qos_.m_lifespan.duration.to_ns() * 1e-6);
        }
        else
        {
            lifespan_timer_.cancel_timer();
        }
    }

    return updated;
}

const ReaderAttributes& DataReader::get_attributes() const
{
    return att_;
}

void DataReader::InnerDataReaderListener::on_new_cache_change_added(
        RTPSReader* /*reader*/,
        const CacheChange_t * const change_in)
{
    if (data_reader_->on_new_cache_change_added(change_in))
    {
        if(data_reader_->listener_ != nullptr)
        {
            //cout << "FIRST BYTE: "<< (int)change->serializedPayload.data[0] << endl;
            data_reader_->listener_->on_data_available(data_reader_);
        }
    }
}

void DataReader::InnerDataReaderListener::on_reader_matched(
        RTPSReader* /*reader*/,
        MatchingInfo& info)
{
    if (data_reader_->listener_ != nullptr)
    {
        data_reader_->listener_->on_subscription_matched(data_reader_, info);
    }
}

bool DataReader::on_new_cache_change_added(
        const CacheChange_t *const change)
{
    if (qos_.m_deadline.period != c_TimeInfinite)
    {
        std::unique_lock<std::recursive_timed_mutex> lock(reader_->getMutex());

        if (!history_.set_next_deadline(
                    change->instanceHandle,
                    steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
        {
            logError(SUBSCRIBER, "Could not set next deadline in the history");
        }
        else if (timer_owner_ == change->instanceHandle || timer_owner_ == InstanceHandle_t())
        {
            deadline_timer_reschedule();
        }
    }

    CacheChange_t* new_change = const_cast<CacheChange_t*>(change);

    if (qos_.m_lifespan.duration == c_TimeInfinite)
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
            lifespan_timer_.cancel_timer();
        }
    }
    else
    {
        logError(SUBSCRIBER, "A change was added to history that could not be retrieved");
    }

    auto interval = source_timestamp - now + duration_cast<nanoseconds>(lifespan_duration_us_);

    // Update and restart the timer
    // If the timer is already running this will not have any effect
    lifespan_timer_.update_interval_millisec(interval.count() * 1e-6);
    lifespan_timer_.restart_timer();
    return true;
}

/*!
 * @brief Returns there is a clean state with all Publishers.
 * It occurs when the Subscriber received all samples sent by Publishers. In other words,
 * its WriterProxies are up to date.
 * @return There is a clean state with all Publishers.
 */
bool DataReader::is_in_clean_state() const
{
    return reader_->isInCleanState();
}

uint64_t DataReader::get_unread_count() const
{
    return history_.getUnreadCount();
}

void DataReader::deadline_timer_reschedule()
{
    assert(qos_.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(reader_->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!history_.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(SUBSCRIBER, "Could not get the next deadline from the history");
        return;
    }
    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());

    deadline_timer_.cancel_timer();
    deadline_timer_.update_interval_millisec(static_cast<double>(interval_ms.count()));
    deadline_timer_.restart_timer();
}

void DataReader::deadline_missed()
{
    assert(qos_.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(reader_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    listener_->on_requested_deadline_missed(this, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

    if (!history_.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(SUBSCRIBER, "Could not set next deadline in the history");
        return;
    }
    deadline_timer_reschedule();
}


void DataReader::get_requested_deadline_missed_status(RequestedDeadlineMissedStatus& status)
{
    std::unique_lock<std::recursive_timed_mutex> lock(reader_->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
}

void DataReader::lifespan_expired()
{
    std::unique_lock<std::recursive_timed_mutex> lock(reader_->getMutex());

    CacheChange_t* earliest_change;
    if (!history_.get_earliest_change(&earliest_change))
    {
        return;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
    if (now - source_timestamp < lifespan_duration_us_)
    {
        auto interval = source_timestamp - now + lifespan_duration_us_;
        lifespan_timer_.update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
        lifespan_timer_.restart_timer();
        return;
    }

    // The earliest change has expired
    history_.remove_change_sub(earliest_change);

    // Set the timer for the next change if there is one
    if (!history_.get_earliest_change(&earliest_change))
    {
        return;
    }

    // Calculate when the next change is due to expire and restart
    source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    now = system_clock::now();
    auto interval = source_timestamp - now + lifespan_duration_us_;

    assert(interval.count() > 0);

    lifespan_timer_.update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
    lifespan_timer_.restart_timer();
}

bool DataReader::read(
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

bool DataReader::take(
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

bool DataReader::get_key_value(
        void* data,
        const rtps::InstanceHandle_t& handle)
{
    (void)data;
    (void)handle;
    // TODO Implement
    return false;
}

bool DataReader::get_liveliness_changed_status(
        LivelinessChangedStatus& status) const
{
    (void)status;
    // TODO Implement
    return false;
}

bool DataReader::get_requested_incompatible_qos_status(
        RequestedIncompatibleQosStatus& status) const
{
    (void)status;
    // TODO Implement
    return false;
}

bool DataReader::get_sample_lost_status(
        SampleLostStatus& status) const
{
    (void)status;
    // TODO Implement
    return false;
}

bool DataReader::get_sample_rejected_status(
        SampleRejectedStatus& status) const
{
    (void)status;
    // TODO Implement
    return false;
}

const Subscriber* DataReader::get_subscriber() const
{
    return subscriber_->get_subscriber();
}

bool DataReader::wait_for_historical_data(
        const Duration_t& max_wait) const
{
    (void)max_wait;
    // TODO Implement
    return false;
}

} /* namespace fastdds */
} /* namespace eprosima */
