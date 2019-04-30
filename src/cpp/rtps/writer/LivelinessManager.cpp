#include <fastrtps/rtps/writer/LivelinessManager.h>
#include <fastrtps/log/Log.h>

#include <algorithm>

using namespace std::chrono;

namespace eprosima {
namespace fastrtps {
namespace rtps {

LivelinessManager::LivelinessManager(
        const std::function<void(GUID_t)>& liveliness_lost_callback,
        const std::function<void(GUID_t)>& liveliness_recovered_callback,
        asio::io_service& service,
        const std::thread& event_thread)
    : timer_(
          std::bind(&LivelinessManager::timer_expired, this),
          0,
          service,
          event_thread)
    , liveliness_lost_callback_(liveliness_lost_callback)
    , liveliness_recovered_callback_(liveliness_recovered_callback)
{
}

LivelinessManager::~LivelinessManager()
{
}

bool LivelinessManager::add_writer(
        GUID_t guid,
        LivelinessQosPolicyKind kind,
        Duration_t lease_duration)
{
    std::unique_lock<std::mutex> lock(mutex_);

    for (const auto&writer : writers_)
    {
        if (writer.guid == guid)
        {
            return false;
        }
    }
    writers_.push_back(LivelinessData(guid, kind, lease_duration));
    return true;
}

bool LivelinessManager::remove_writer(GUID_t guid)
{
    std::unique_lock<std::mutex> lock(mutex_);

    for (const auto& writer: writers_)
    {
        if (writer.guid == guid)
        {
            writers_.remove(writer);

            if (timer_owner_->guid == guid)
            {
                timer_owner_ = nullptr;
                if (!calculate_next())
                {
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
    return false;
}

bool LivelinessManager::assert_liveliness(GUID_t guid)
{
    std::unique_lock<std::mutex> lock(mutex_);

    ResourceLimitedVector<LivelinessData>::iterator wit;
    if (!find_writer(
                guid,
                &wit))
    {
        return false;
    }

    if (wit->kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS ||
        wit->kind == AUTOMATIC_LIVELINESS_QOS)
    {
        for (auto& w: writers_)
        {
            if (w.kind == wit->kind)
            {
                if (w.alive == false)
                {
                    if (liveliness_recovered_callback_ != nullptr)
                    {
                        liveliness_recovered_callback_(w.guid);
                    }
                }
                w.alive = true;
                w.time = steady_clock::now() + nanoseconds(w.lease_duration.to_ns());
            }
        }
    }
    else if (wit->kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        if (liveliness_recovered_callback_ != nullptr)
        {
            liveliness_recovered_callback_(guid);
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

    for (auto& writer: writers_)
    {
        if (writer.kind == kind)
        {
            if (writer.alive == false)
            {
                if (liveliness_recovered_callback_ != nullptr)
                {
                    liveliness_recovered_callback_(writer.guid);
                }
            }
            writer.alive = true;
            writer.time = steady_clock::now() + nanoseconds(writer.lease_duration.to_ns());
        }
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

bool LivelinessManager::calculate_next()
{
    timer_owner_ = nullptr;

    steady_clock::time_point min_time = steady_clock::now() + nanoseconds(c_TimeInfinite.to_ns());

    for (auto it=writers_.begin(); it!=writers_.end(); ++it)
    {
        if (it->alive)
        {
            if (it->time < min_time)
            {
                min_time = it->time;
                timer_owner_ = &*it;
                return true;
            }
        }
    }
    return false;
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
        liveliness_lost_callback_(timer_owner_->guid);
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
        GUID_t guid,
        ResourceLimitedVector<LivelinessData>::iterator *wit_out)
{
    for (auto it=writers_.begin(); it!=writers_.end(); ++it)
    {
        if (it->guid == guid)
        {
            *wit_out = it;
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
