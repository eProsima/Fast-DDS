// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataSharingNotification.hpp
 */

#ifndef RTPS_HISTORY_DATASHARINGNOTIFICATION_HPP
#define RTPS_HISTORY_DATASHARINGNOTIFICATION_HPP

#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <utils/shared_memory/SharedMemSegment.hpp>
#include <utils/shared_memory/SharedDir.hpp>
#include <fastdds/rtps/common/Guid.h>

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingNotification
{

    friend class DataSharingListener;
    friend class DataSharingNotifier;

public:

    typedef fastdds::rtps::SharedMemSegment Segment;

    DataSharingNotification() = default;

    virtual ~DataSharingNotification() = default;

    /**
     * Notifies of new data
     */
    inline void notify()
    {
        notification_->new_data.store(true);
        notification_->notification_cv.notify_all();
    }

    /**
     * Returns the GUID of the reader listening to the notifications
     */
    inline const GUID_t&  reader() const
    {
        return segment_id_;
    }

    static std::shared_ptr<DataSharingNotification> create_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());

    static std::shared_ptr<DataSharingNotification> open_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());

    void destroy();

    static std::string get_default_directory()
    {
        std::string dir;
        fastdds::rtps::SharedDir::get_default_shared_dir(dir);
        return dir;
    }

    constexpr static const char* domain_name()
    {
        return "fast_datasharing";
    }

protected:

#pragma warning(push)
#pragma warning(disable:4324)
    struct alignas (void*) Notification
    {
        //! CV to wait for new notifications
        Segment::condition_variable notification_cv;

        //! synchronization mutex
        Segment::mutex notification_mutex;

        //! New data available
        std::atomic<bool> new_data;

        //! Timestamp of the reader's first sample NOT ack'd
        std::atomic<int64_t> ack_timestamp;
    };
#pragma warning(pop)

    static std::string generate_segment_name(
            const std::string& /*shared_dir*/,
            const GUID_t& reader_guid)
    {
        std::stringstream ss;
        ss << DataSharingNotification::domain_name() << "_" << reader_guid.guidPrefix << "_" << reader_guid.entityId;
        return ss.str();
    }

    bool create_and_init_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());


    bool open_and_init_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());


    GUID_t segment_id_;         //< The ID of the segment is the GUID of the reader
    std::string segment_name_;  //< Segment name

    std::unique_ptr<Segment> segment_;  //< Shared memory segment
    Notification* notification_;        //< The notification data
    bool owned_ = false;                //< Whether the shared segment is owned by this instance
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGNOTIFICATION_HPP
