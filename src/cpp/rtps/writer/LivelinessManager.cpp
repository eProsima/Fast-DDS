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

bool LivelinessManager::add_writer(LivelinessData *writer)
{

    // Check if the writer is already being managed
    if (std::find(writers_.begin(),
                  writers_.end(),
                  writer) != writers_.end())
    {
        return false;
    }

    writers_.push_back(writer);

    return true;
}

bool LivelinessManager::remove_writer(LivelinessData *writer)
{

    // Check if the writer is being managed
    auto wit = std::find(writers_.begin(),
                         writers_.end(),
                         writer);

    if (wit != writers_.end())
    {
        return false;
    }

    writers_.erase(wit);

    return true;
}

void LivelinessManager::assert_liveliness(LivelinessData *writer)
{

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
