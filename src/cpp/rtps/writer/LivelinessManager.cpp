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

#include <rtps/writer/LivelinessManager.hpp>

#include <algorithm>

#include <fastdds/dds/log/Log.hpp>

using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace rtps {

using LivelinessDataIterator = ResourceLimitedVector<LivelinessData>::iterator;

LivelinessManager::LivelinessManager(
        const LivelinessCallback& callback,
        ResourceEvent& service,
        bool manage_automatic)
    : callback_(callback)
    , manage_automatic_(manage_automatic)
    , writers_()
    , mutex_()
    , col_mutex_()
    , timer_owner_(nullptr)
    , timer_(
        service,
        [this]() -> bool
        {
            return timer_expired();
        },
        0)
{
}

LivelinessManager::~LivelinessManager()
{
    std::lock_guard<std::mutex> _(mutex_);
    timer_owner_ = nullptr;
    timer_.cancel_timer();
}

bool LivelinessManager::add_writer(
        GUID_t guid,
        fastdds::dds::LivelinessQosPolicyKind kind,
        dds::Duration_t lease_duration)
{
    if (!manage_automatic_ && kind == fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Liveliness manager not managing automatic writers, writer not added");
        return false;
    }

    {
        // collection guard
        std::lock_guard<shared_mutex> _(col_mutex_);
        // writers_ elements guard
        std::lock_guard<std::mutex> __(mutex_);

        for (LivelinessData& writer : writers_)
        {
            if (writer.guid == guid &&
                    writer.kind == kind &&
                    writer.lease_duration == lease_duration)
            {
                writer.count++;
                return true;
            }
        }
        writers_.emplace_back(guid, kind, lease_duration);
    }

    if (!calculate_next())
    {
        // TimedEvent is thread safe
        timer_.cancel_timer();
        return true;
    }

    std::lock_guard<std::mutex> _(mutex_);

    if (timer_owner_ != nullptr)
    {
        // Some times the interval could be negative if a writer expired during the call to this function
        // Once in this situation there is not much we can do but let asio timers expire immediately
        auto interval = timer_owner_->time - steady_clock::now();
        timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        timer_.restart_timer();
    }

    return true;
}

bool LivelinessManager::remove_writer(
        GUID_t guid,
        fastdds::dds::LivelinessQosPolicyKind kind,
        dds::Duration_t lease_duration,
        LivelinessData::WriterStatus& writer_status)
{
    bool removed = false;

    {
        // collection guard
        std::lock_guard<shared_mutex> _(col_mutex_);
        // writers_ elements guard
        std::lock_guard<std::mutex> __(mutex_);

        removed = writers_.remove_if([guid, kind, lease_duration, &writer_status](LivelinessData& writer)
                        {
                            writer_status = writer.status;
                            return writer.guid == guid &&
                            writer.kind == kind &&
                            writer.lease_duration == lease_duration &&
                            --writer.count == 0;
                        });
    }

    if (!removed)
    {
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);

    if (timer_owner_ != nullptr)
    {
        lock.unlock();

        if (!calculate_next())
        {
            timer_.cancel_timer();
            return true;
        }

        lock.lock();

        if (timer_owner_ != nullptr)
        {
            // Some times the interval could be negative if a writer expired during the call to this function
            // Once in this situation there is not much we can do but let asio timers expire inmediately
            auto interval = timer_owner_->time - steady_clock::now();
            timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
            timer_.restart_timer();
        }
    }

    return true;
}

bool LivelinessManager::assert_liveliness(
        GUID_t guid,
        fastdds::dds::LivelinessQosPolicyKind kind,
        dds::Duration_t lease_duration)
{
    bool found = false;

    {
        // collection guard
        shared_lock<shared_mutex> _(col_mutex_);

        for (LivelinessData& writer : writers_)
        {
            // writers_ elements guard
            std::unique_lock<std::mutex> lock(mutex_);

            if (writer.guid == guid &&
                    writer.kind == kind &&
                    writer.lease_duration == lease_duration)
            {
                lock.unlock();

                found = true;

                // Execute the callbacks
                if (writer.kind == fastdds::dds::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS ||
                        writer.kind == fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
                {
                    for (LivelinessData& w: writers_)
                    {
                        if (w.kind == writer.kind &&
                                w.guid.guidPrefix == guid.guidPrefix)
                        {
                            assert_writer_liveliness(w);
                        }
                    }
                }
                else if (writer.kind == fastdds::dds::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
                {
                    assert_writer_liveliness(writer);
                }

                break;
            }
        }
    }

    if (!found)
    {
        return false;
    }

    timer_.cancel_timer();

    // Updates the timer owner
    if (!calculate_next())
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error when restarting liveliness timer");
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (timer_owner_ != nullptr)
    {
        // Some times the interval could be negative if a writer expired during the call to this function
        // Once in this situation there is not much we can do but let asio timers expire inmediately
        auto interval = timer_owner_->time - steady_clock::now();
        timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        timer_.restart_timer();
    }

    return true;
}

bool LivelinessManager::assert_liveliness(
        fastdds::dds::LivelinessQosPolicyKind kind,
        GuidPrefix_t guid_prefix)
{

    if (!manage_automatic_ && kind == fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Liveliness manager not managing automatic writers, writer not added");
        return false;
    }

    {
        // collection guard
        shared_lock<shared_mutex> _(col_mutex_);

        if (writers_.empty())
        {
            return true;
        }


        for (LivelinessData& writer: writers_)
        {
            if (writer.kind == kind &&
                    guid_prefix == writer.guid.guidPrefix)
            {
                assert_writer_liveliness(writer);
            }
        }
    }

    timer_.cancel_timer();

    // Updates the timer owner
    if (!calculate_next())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER,
                "Error when restarting liveliness timer: " << writers_.size() << " writers, liveliness " <<
                kind);
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (timer_owner_ != nullptr)
    {
        // Some times the interval could be negative if a writer expired during the call to this function
        // Once in this situation there is not much we can do but let asio timers expire inmediately
        auto interval = timer_owner_->time - steady_clock::now();
        timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        timer_.restart_timer();
    }

    return true;
}

bool LivelinessManager::calculate_next()
{
    // Keep this lock order to prevent ABBA deadlocks
    shared_lock<shared_mutex> _(col_mutex_);
    std::lock_guard<std::mutex> __(mutex_);

    bool any_alive = false;
    steady_clock::time_point min_time = steady_clock::now() + nanoseconds(dds::c_TimeInfinite.to_ns());

    timer_owner_ = nullptr;

    // collection guard
    for (LivelinessData& writer : writers_)
    {
        if (writer.status == LivelinessData::WriterStatus::ALIVE)
        {
            if (writer.time < min_time)
            {
                min_time = writer.time;
                timer_owner_ = &writer;
            }
            any_alive = true;
        }
    }

    return any_alive;
}

bool LivelinessManager::timer_expired()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (timer_owner_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Liveliness timer expired but there is no writer");
        return false;
    }
    else
    {
        timer_owner_->status = LivelinessData::WriterStatus::NOT_ALIVE;
    }

    auto guid = timer_owner_->guid;
    auto kind = timer_owner_->kind;
    auto lease_duration = timer_owner_->lease_duration;

    lock.unlock();

    if (callback_ != nullptr)
    {
        callback_(guid, kind, lease_duration, -1, 1);
    }

    if (calculate_next())
    {
        lock.lock();

        if ( timer_owner_ != nullptr)
        {
            // Some times the interval could be negative if a writer expired during the call to this function
            // Once in this situation there is not much we can do but let asio timers expire inmediately
            auto interval = timer_owner_->time - steady_clock::now();
            timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());

            return true;
        }
    }

    return false;
}

bool LivelinessManager::is_any_alive(
        fastdds::dds::LivelinessQosPolicyKind kind)
{
    // Keep this lock order to prevent ABBA deadlocks
    shared_lock<shared_mutex> _(col_mutex_);
    std::lock_guard<std::mutex> __(mutex_);

    for (const auto& writer : writers_)
    {
        if (writer.kind == kind && writer.status == LivelinessData::WriterStatus::ALIVE)
        {
            return true;
        }
    }

    return false;
}

void LivelinessManager::assert_writer_liveliness(
        LivelinessData& writer)
{
    // The shared_mutex is taken, that is, the writer referenced will not be destroyed during this call
    std::unique_lock<std::mutex> lock(mutex_);

    auto status = writer.status;
    auto guid = writer.guid;
    auto kind = writer.kind;
    auto lease_duration = writer.lease_duration;

    writer.status = LivelinessData::WriterStatus::ALIVE;
    writer.time = steady_clock::now() + nanoseconds(writer.lease_duration.to_ns());

    lock.unlock();

    if (callback_ != nullptr)
    {
        if (status == LivelinessData::WriterStatus::NOT_ASSERTED)
        {
            callback_(guid, kind, lease_duration, 1, 0);
        }
        else if (status == LivelinessData::WriterStatus::NOT_ALIVE)
        {
            callback_(guid, kind, lease_duration, 1, -1);
        }
    }
}

const ResourceLimitedVector<LivelinessData>& LivelinessManager::get_liveliness_data() const
{
    return writers_;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
