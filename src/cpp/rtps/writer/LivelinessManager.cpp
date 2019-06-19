#include <fastrtps/rtps/writer/LivelinessManager.h>
#include <fastrtps/log/Log.h>

#include <algorithm>

using namespace std::chrono;

namespace eprosima {
namespace fastrtps {
namespace rtps {

using LivelinessDataIterator = ResourceLimitedVector<LivelinessData>::iterator;

LivelinessManager::LivelinessManager(
        const std::function<void(
            const GUID_t&,
            const LivelinessQosPolicyKind&,
            const Duration_t&)>& liveliness_lost_callback,
        const std::function<void(
            const GUID_t&,
            const LivelinessQosPolicyKind&,
            const Duration_t&)>& liveliness_recovered_callback,
        asio::io_service& service,
        const std::thread& event_thread,
        bool manage_automatic)
    : liveliness_lost_callback_(liveliness_lost_callback)
    , liveliness_recovered_callback_(liveliness_recovered_callback)
    , manage_automatic_(manage_automatic)
    , writers_()
    , mutex_()
    , timer_owner_(nullptr)
    , timer_(
          std::bind(&LivelinessManager::timer_expired, this),
          0,
          service,
          event_thread)
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

    if (!manage_automatic_ && kind == AUTOMATIC_LIVELINESS_QOS)
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

                if (timer_owner_ != nullptr && timer_owner_->guid == guid)
                {
                    timer_owner_ = nullptr;
                    if (!calculate_next())
                    {
                        timer_.cancel_timer();
                        return true;
                    }

                    auto interval = timer_owner_->time - steady_clock::now();
                    assert(interval.count() > 0);
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

    if (wit->kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS ||
        wit->kind == AUTOMATIC_LIVELINESS_QOS)
    {
        for (LivelinessData& w: writers_)
        {
            if (w.kind == wit->kind)
            {
                if (w.alive == false)
                {
                    if (liveliness_recovered_callback_ != nullptr)
                    {
                        liveliness_recovered_callback_(
                                    w.guid,
                                    w.kind,
                                    w.lease_duration);
                    }
                }
                w.alive = true;
                w.time = steady_clock::now() + nanoseconds(w.lease_duration.to_ns());
            }
        }
    }
    else if (wit->kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        if (wit->alive == false)
        {
            if (liveliness_recovered_callback_ != nullptr)
            {
                liveliness_recovered_callback_(
                            wit->guid,
                            wit->kind,
                            wit->lease_duration);
            }
        }
        wit->alive = true;
        wit->time = steady_clock::now() + nanoseconds(wit->lease_duration.to_ns());
    }

    // Updates the timer owner
    if (!calculate_next())
    {
        logError(RTPS_WRITER, "Error when restarting liveliness timer");
        return false;
    }

    auto interval = timer_owner_->time - steady_clock::now();
    assert(interval.count() > 0);
    timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
    timer_.restart_timer();

    return true;
}

bool LivelinessManager::assert_liveliness(LivelinessQosPolicyKind kind)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!manage_automatic_ && kind == AUTOMATIC_LIVELINESS_QOS)
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
            if (writer.alive == false)
            {
                if (liveliness_recovered_callback_ != nullptr)
                {
                    liveliness_recovered_callback_(
                                writer.guid,
                                writer.kind,
                                writer.lease_duration);
                }
            }
            writer.alive = true;
            writer.time = steady_clock::now() + nanoseconds(writer.lease_duration.to_ns());
        }
    }

    // Updates the timer owner
    if (!calculate_next())
    {
        logError(RTPS_WRITER, "Error when restarting liveliness timer: " << writers_.size() << " writers, liveliness " << kind);
        return false;
    }

    auto interval = timer_owner_->time - steady_clock::now();
    assert(interval.count() > 0);
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
        if (it->alive)
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

void LivelinessManager::timer_expired()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (timer_owner_ == nullptr)
    {
        logError(RTPS_WRITER, "Liveliness timer expired but there is no writer");
        return;
    }

    if (liveliness_lost_callback_ != nullptr)
    {
        liveliness_lost_callback_(
                    timer_owner_->guid,
                    timer_owner_->kind,
                    timer_owner_->lease_duration);
    }
    timer_owner_->alive = false;

    if (calculate_next())
    {
        auto interval = timer_owner_->time - steady_clock::now();
        timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        timer_.restart_timer();
    }
}

bool LivelinessManager::find_writer(
        const GUID_t& guid,
        const LivelinessQosPolicyKind& kind,
        const Duration_t& lease_duration,
        ResourceLimitedVector<LivelinessData>::iterator *wit_out)
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

bool LivelinessManager::is_any_alive(LivelinessQosPolicyKind kind)
{
    for (const auto& writer : writers_)
    {
        if (writer.kind == kind && writer.alive == true)
        {
            return true;
        }
    }
    return false;
}

const ResourceLimitedVector<LivelinessData>& LivelinessManager::get_liveliness_data() const
{
    return writers_;
}

}
}
}
