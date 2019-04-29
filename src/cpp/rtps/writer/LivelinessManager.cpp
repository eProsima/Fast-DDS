#include <fastrtps/rtps/writer/LivelinessManager.h>
#include <fastrtps/log/Log.h>

#include <algorithm>

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
{}

LivelinessManager::~LivelinessManager()
{}

bool LivelinessManager::add_writer(
        GUID_t guid,
        LivelinessQosPolicyKind kind,
        Duration_t lease_duration)
{
    for (const auto&writer : writers_)
    {
        if (writer.writer_guid == guid)
        {
            return false;
        }
    }
    writers_.push_back(LivelinessData(guid, kind, lease_duration));
    return true;
}

bool LivelinessManager::remove_writer(GUID_t guid)
{
    for (const auto& writer: writers_)
    {
        if (writer.writer_guid == guid)
        {
            writers_.remove(writer);
            return true;
        }
    }
    return false;
}

bool LivelinessManager::assert_liveliness(GUID_t guid)
{
    return false;
}

void LivelinessManager::timer_expired()
{

    // Notify external classes
    if (liveliness_lost_callback_ != nullptr)
    {
        liveliness_lost_callback_(timer_owner_->writer_guid);
    }
}

}
}
}
