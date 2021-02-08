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
 * @file SubscriberImpl.cpp
 *
 */
#include <fastrtps/config.h>

#include <fastrtps_deprecated/subscriber/SubscriberImpl.h>
#include <fastrtps_deprecated/participant/ParticipantImpl.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastdds/dds/log/Log.hpp>

#include <rtps/history/TopicPayloadPoolRegistry.hpp>

using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

using eprosima::fastdds::dds::TopicDataType;

namespace eprosima {
namespace fastrtps {


SubscriberImpl::SubscriberImpl(
        ParticipantImpl* p,
        TopicDataType* ptype,
        const SubscriberAttributes& att,
        SubscriberListener* listen)
    : mp_participant(p)
    , mp_reader(nullptr)
    , mp_type(ptype)
    , m_att(att)
#pragma warning (disable : 4355 )
    , m_history(att.topic,
            ptype,
            att.qos,
            ptype->m_typeSize  + 3 /*Possible alignment*/,
            att.historyMemoryPolicy)
    , mp_listener(listen)
    , m_readerListener(this)
    , mp_userSubscriber(nullptr)
    , mp_rtpsParticipant(nullptr)
    , deadline_duration_us_(m_att.qos.m_deadline.period.to_ns() * 1e-3)
    , deadline_missed_status_()
    , lifespan_duration_us_(m_att.qos.m_lifespan.duration.to_ns() * 1e-3)
{
    std::string topic_name = m_att.topic.getTopicName().to_string();
    PoolConfig pool_cfg = PoolConfig::from_history_attributes(m_history.m_att);
    payload_pool_ = TopicPayloadPoolRegistry::get(topic_name, pool_cfg);
    payload_pool_->reserve_history(pool_cfg, true);

    deadline_timer_ = new TimedEvent(mp_participant->get_resource_event(),
                    [&]() -> bool
                    {
                        return deadline_missed();
                    },
                    att.qos.m_deadline.period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(mp_participant->get_resource_event(),
                    [&]() -> bool
                    {
                        return lifespan_expired();
                    },
                    att.qos.m_lifespan.duration.to_ns() * 1e-6);
}

SubscriberImpl::~SubscriberImpl()
{
    delete(lifespan_timer_);
    delete(deadline_timer_);

    if (mp_reader != nullptr)
    {
        logInfo(SUBSCRIBER, this->getGuid().entityId << " in topic: " << this->m_att.topic.topicName);
    }

    RTPSDomain::removeRTPSReader(mp_reader);
    delete(this->mp_userSubscriber);

    std::string topic_name = m_att.topic.getTopicName().to_string();
    PoolConfig pool_cfg = PoolConfig::from_history_attributes(m_history.m_att);
    payload_pool_->release_history(pool_cfg, true);
}

bool SubscriberImpl::wait_for_unread_samples(
        const Duration_t& timeout)
{
    return mp_reader->wait_for_unread_cache(timeout);
}

bool SubscriberImpl::readNextData(
        void* data,
        SampleInfo_t* info)
{
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(m_att.qos.m_reliability.max_blocking_time));
#else
            std::chrono::hours(24);
#endif // if HAVE_STRICT_REALTIME
    return this->m_history.readNextData(data, info, max_blocking_time);
}

bool SubscriberImpl::takeNextData(
        void* data,
        SampleInfo_t* info)
{
    auto max_blocking_time = std::chrono::steady_clock::now() +
#if HAVE_STRICT_REALTIME
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(m_att.qos.m_reliability.max_blocking_time));
#else
            std::chrono::hours(24);
#endif // if HAVE_STRICT_REALTIME
    return this->m_history.takeNextData(data, info, max_blocking_time);
}

bool SubscriberImpl::get_first_untaken_info(
        SampleInfo_t* info)
{
    return m_history.get_first_untaken_info(info);
}

const GUID_t& SubscriberImpl::getGuid()
{
    return mp_reader->getGuid();
}

bool SubscriberImpl::updateAttributes(
        const SubscriberAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if (att.unicastLocatorList.size() != this->m_att.unicastLocatorList.size() ||
            att.multicastLocatorList.size() != this->m_att.multicastLocatorList.size())
    {
        logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
        updated &= false;
    }
    else
    {
        for (LocatorListConstIterator lit1 = this->m_att.unicastLocatorList.begin();
                lit1 != this->m_att.unicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.unicastLocatorList.begin();
                    lit2 != att.unicastLocatorList.end(); ++lit2)
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
        for (LocatorListConstIterator lit1 = this->m_att.multicastLocatorList.begin();
                lit1 != this->m_att.multicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.multicastLocatorList.begin();
                    lit2 != att.multicastLocatorList.end(); ++lit2)
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

    //TOPIC ATTRIBUTES
    if (this->m_att.topic != att.topic)
    {
        logWarning(RTPS_READER, "Topic Attributes cannot be updated");
        updated &= false;
    }
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if (!this->m_att.qos.canQosBeUpdated(att.qos))
    {
        updated &= false;
    }
    if (updated)
    {
        this->m_att.expectsInlineQos = att.expectsInlineQos;
        if (this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulReader* sfr = (StatefulReader*)mp_reader;
            sfr->updateTimes(att.times);
        }
        this->m_att.qos.setQos(att.qos, false);
        //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
        mp_rtpsParticipant->updateReader(this->mp_reader, m_att.topic, m_att.qos);

        // Deadline

        if (m_att.qos.m_deadline.period != c_TimeInfinite)
        {
            deadline_duration_us_ =
                    duration<double, std::ratio<1, 1000000>>(m_att.qos.m_deadline.period.to_ns() * 1e-3);
            deadline_timer_->update_interval_millisec(m_att.qos.m_deadline.period.to_ns() * 1e-6);
        }
        else
        {
            deadline_timer_->cancel_timer();
        }

        // Lifespan

        if (m_att.qos.m_lifespan.duration != c_TimeInfinite)
        {
            lifespan_duration_us_ =
                    std::chrono::duration<double,
                            std::ratio<1, 1000000>>(m_att.qos.m_lifespan.duration.to_ns() * 1e-3);
            lifespan_timer_->update_interval_millisec(m_att.qos.m_lifespan.duration.to_ns() * 1e-6);
        }
        else
        {
            lifespan_timer_->cancel_timer();
        }
    }

    return updated;
}

void SubscriberImpl::SubscriberReaderListener::onNewCacheChangeAdded(
        RTPSReader* /*reader*/,
        const CacheChange_t* const change_in)
{
    if (mp_subscriberImpl->onNewCacheChangeAdded(change_in))
    {
        if (mp_subscriberImpl->mp_listener != nullptr)
        {
            //cout << "FIRST BYTE: "<< (int)change->serializedPayload.data[0] << endl;
            mp_subscriberImpl->mp_listener->onNewDataMessage(mp_subscriberImpl->mp_userSubscriber);
        }
    }
}

void SubscriberImpl::SubscriberReaderListener::onReaderMatched(
        RTPSReader* /*reader*/,
        MatchingInfo& info)
{
    if (this->mp_subscriberImpl->mp_listener != nullptr)
    {
        mp_subscriberImpl->mp_listener->onSubscriptionMatched(mp_subscriberImpl->mp_userSubscriber, info);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_liveliness_changed(
        RTPSReader* reader,
        const LivelinessChangedStatus& status)
{
    (void)reader;

    if (mp_subscriberImpl->mp_listener != nullptr)
    {
        mp_subscriberImpl->mp_listener->on_liveliness_changed(
            mp_subscriberImpl->mp_userSubscriber,
            status);
    }
}

bool SubscriberImpl::onNewCacheChangeAdded(
        const CacheChange_t* const change_in)
{
    if (m_att.qos.m_deadline.period != c_TimeInfinite)
    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

        if (!m_history.set_next_deadline(
                    change_in->instanceHandle,
                    steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
        {
            logError(SUBSCRIBER, "Could not set next deadline in the history");
        }
        else if (timer_owner_ == change_in->instanceHandle || timer_owner_ == InstanceHandle_t())
        {
            if (deadline_timer_reschedule())
            {
                deadline_timer_->cancel_timer();
                deadline_timer_->restart_timer();
            }
        }
    }

    CacheChange_t* change = (CacheChange_t*)change_in;

    if (m_att.qos.m_lifespan.duration == c_TimeInfinite)
    {
        return true;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // The new change could have expired if it arrived too late
    // If so, remove it from the history and return false to avoid notifying the listener
    if (now - source_timestamp >= lifespan_duration_us_)
    {
        m_history.remove_change_sub(change);
        return false;
    }

    CacheChange_t* earliest_change;
    if (m_history.get_earliest_change(&earliest_change))
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

/*!
 * @brief Returns there is a clean state with all Publishers.
 * It occurs when the Subscriber received all samples sent by Publishers. In other words,
 * its WriterProxies are up to date.
 * @return There is a clean state with all Publishers.
 */
bool SubscriberImpl::isInCleanState() const
{
    return mp_reader->isInCleanState();
}

uint64_t SubscriberImpl::get_unread_count() const
{
    return mp_reader->get_unread_count();
}

bool SubscriberImpl::deadline_timer_reschedule()
{
    assert(m_att.qos.m_deadline.period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!m_history.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(SUBSCRIBER, "Could not get the next deadline from the history");
        return false;
    }
    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());
    deadline_timer_->update_interval_millisec((double)interval_ms.count());
    return true;
}

bool SubscriberImpl::deadline_missed()
{
    assert(m_att.qos.m_deadline.period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    mp_listener->on_requested_deadline_missed(mp_userSubscriber, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

    if (!m_history.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(SUBSCRIBER, "Could not set next deadline in the history");
        return false;
    }
    return deadline_timer_reschedule();
}

void SubscriberImpl::get_requested_deadline_missed_status(
        RequestedDeadlineMissedStatus& status)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
}

bool SubscriberImpl::lifespan_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

    CacheChange_t* earliest_change;
    while (m_history.get_earliest_change(&earliest_change))
    {
        auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
        auto now = system_clock::now();

        // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
        if (now - source_timestamp < lifespan_duration_us_)
        {
            auto interval = source_timestamp - now + lifespan_duration_us_;
            lifespan_timer_->update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
            return true;
        }

        // The earliest change has expired
        m_history.remove_change_sub(earliest_change);

        // Set the timer for the next change if there is one
        if (!m_history.get_earliest_change(&earliest_change))
        {
            return false;
        }

        // Calculate when the next change is due to expire and restart
        source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
        now = system_clock::now();
        auto interval = source_timestamp - now + lifespan_duration_us_;

        if (interval.count() > 0)
        {
            lifespan_timer_->update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
            return true;
        }
    }

    return false;
}

void SubscriberImpl::get_liveliness_changed_status(
        LivelinessChangedStatus& status)
{
    std::unique_lock<RecursiveTimedMutex> lock(mp_reader->getMutex());

    status = mp_reader->liveliness_changed_status_;

    mp_reader->liveliness_changed_status_.alive_count_change = 0u;
    mp_reader->liveliness_changed_status_.not_alive_count_change = 0u;
}

std::shared_ptr<rtps::IPayloadPool> SubscriberImpl::payload_pool()
{
    return payload_pool_;
}

} /* namespace fastrtps */
} /* namespace eprosima */
