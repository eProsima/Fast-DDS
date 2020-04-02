#include <fastdds/rtps/writer/LivelinessManager.h>
#include <fastdds/dds/log/Log.hpp>

#include <algorithm>

using namespace std::chrono;

namespace eprosima {
namespace fastrtps {
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
    std::unique_lock<std::mutex> lock(mutex_);
    timer_owner_ = nullptr;
    timer_.cancel_timer();
}

bool LivelinessManager::add_writer(
        GUID_t guid,
        LivelinessQosPolicyKind kind,
        Duration_t lease_duration)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!manage_automatic_ && kind == LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        logWarning(RTPS_WRITER, "Liveliness manager not managing automatic writers, writer not added");
        return false;
    }

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
    return true;
}

bool LivelinessManager::remove_writer(
        GUID_t guid,
        LivelinessQosPolicyKind kind,
        Duration_t lease_duration)
{
    std::unique_lock<std::mutex> lock(mutex_);

    for (LivelinessData& writer: writers_)
    {
        if (writer.guid == guid &&
                writer.kind == kind &&
                writer.lease_duration == lease_duration)
        {
            if (--writer.count == 0)
            {
                writers_.remove(writer);

                if (callback_ != nullptr)
                {
                    if (writer.status == LivelinessData::WriterStatus::ALIVE)
                    {
                        callback_(writer.guid,
                                writer.kind,
                                writer.lease_duration,
                                -1,
                                0);
                    }
                    else if (writer.status == LivelinessData::WriterStatus::NOT_ALIVE)
                    {
                        callback_(writer.guid,
                                writer.kind,
                                writer.lease_duration,
                                0,
                                -1);
                    }
                }

                if (timer_owner_ != nullptr && timer_owner_->guid == guid)
                {
                    timer_owner_ = nullptr;
                    if (!calculate_next())
                    {
                        timer_.cancel_timer();
                        return true;
                    }

                    // Some times the interval could be negative if a writer expired during the call to this function
                    // Once in this situation there is not much we can do but let asio timers expire inmediately
                    auto interval = timer_owner_->time - steady_clock::now();
                    timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
                    timer_.restart_timer();
                }
                return true;
            }
        }
    }

    return false;
}

bool LivelinessManager::assert_liveliness(
        GUID_t guid,
        LivelinessQosPolicyKind kind,
        Duration_t lease_duration)
{
    std::unique_lock<std::mutex> lock(mutex_);

    ResourceLimitedVector<LivelinessData>::iterator wit;
    if (!find_writer(
                guid,
                kind,
                lease_duration,
                &wit))
    {
        return false;
    }

    timer_.cancel_timer();

    if (wit->kind == LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS ||
            wit->kind == LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        for (LivelinessData& w: writers_)
        {
            if (w.kind == wit->kind)
            {
                assert_writer_liveliness(w);
            }
        }
    }
    else if (wit->kind == LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        assert_writer_liveliness(*wit);
    }

    // Updates the timer owner
    if (!calculate_next())
    {
        logError(RTPS_WRITER, "Error when restarting liveliness timer");
        return false;
    }

    // Some times the interval could be negative if a writer expired during the call to this function
    // Once in this situation there is not much we can do but let asio timers expire inmediately
    auto interval = timer_owner_->time - steady_clock::now();
    timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
    timer_.restart_timer();

    return true;
}

bool LivelinessManager::assert_liveliness(
        LivelinessQosPolicyKind kind)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!manage_automatic_ && kind == LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        logWarning(RTPS_WRITER, "Liveliness manager not managing automatic writers, writer not added");
        return false;
    }

    if (writers_.empty())
    {
        return true;
    }

    timer_.cancel_timer();

    for (LivelinessData& writer: writers_)
    {
        if (writer.kind == kind)
        {
            assert_writer_liveliness(writer);
        }
    }

    // Updates the timer owner
    if (!calculate_next())
    {
        logInfo(RTPS_WRITER,
                "Error when restarting liveliness timer: " << writers_.size() << " writers, liveliness " <<
                kind);
        return false;
    }

    // Some times the interval could be negative if a writer expired during the call to this function
    // Once in this situation there is not much we can do but let asio timers expire inmediately
    auto interval = timer_owner_->time - steady_clock::now();
    timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
    timer_.restart_timer();

    return true;
}

bool LivelinessManager::calculate_next()
{

    timer_owner_ = nullptr;

    steady_clock::time_point min_time = steady_clock::now() + nanoseconds(c_TimeInfinite.to_ns());

    bool any_alive = false;

    for (LivelinessDataIterator it=writers_.begin(); it!=writers_.end(); ++it)
    {
        if (it->status == LivelinessData::WriterStatus::ALIVE)
        {
            if (it->time < min_time)
            {
                min_time = it->time;
                timer_owner_ = &*it;
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
        logError(RTPS_WRITER, "Liveliness timer expired but there is no writer");
        return false;
    }

    if (callback_ != nullptr)
    {
        callback_(timer_owner_->guid,
                timer_owner_->kind,
                timer_owner_->lease_duration,
                -1,
                1);
    }
    timer_owner_->status = LivelinessData::WriterStatus::NOT_ALIVE;

    if (calculate_next())
    {
        // Some times the interval could be negative if a writer expired during the call to this function
        // Once in this situation there is not much we can do but let asio timers expire inmediately
        auto interval = timer_owner_->time - steady_clock::now();
        timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        return true;
    }

    return false;
}

bool LivelinessManager::find_writer(
        const GUID_t& guid,
        const LivelinessQosPolicyKind& kind,
        const Duration_t& lease_duration,
        ResourceLimitedVector<LivelinessData>::iterator* wit_out)
{
    for (LivelinessDataIterator it=writers_.begin(); it!=writers_.end(); ++it)
    {
        if (it->guid == guid &&
                it->kind == kind &&
                it->lease_duration == lease_duration)
        {
            *wit_out = it;
            return true;
        }
    }
    return false;
}

bool LivelinessManager::is_any_alive(
        LivelinessQosPolicyKind kind)
{
    std::unique_lock<std::mutex> lock(mutex_);

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
    if (callback_ != nullptr)
    {
        if (writer.status == LivelinessData::WriterStatus::NOT_ASSERTED)
        {
            callback_(writer.guid,
                    writer.kind,
                    writer.lease_duration,
                    1,
                    0);
        }
        else if (writer.status == LivelinessData::WriterStatus::NOT_ALIVE)
        {
            callback_(writer.guid,
                    writer.kind,
                    writer.lease_duration,
                    1,
                    -1);
        }
    }

    writer.status = LivelinessData::WriterStatus::ALIVE;
    writer.time = steady_clock::now() + nanoseconds(writer.lease_duration.to_ns());
}

const ResourceLimitedVector<LivelinessData>& LivelinessManager::get_liveliness_data() const
{
    return writers_;
}

}
}
}
