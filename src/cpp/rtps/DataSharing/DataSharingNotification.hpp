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
#include <fastdds/rtps/common/Guid.hpp>

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace fastdds {
namespace rtps {

class DataSharingNotification
{

    friend class DataSharingListener;
    friend class DataSharingNotifier;

public:

    typedef fastdds::rtps::SharedSegmentBase Segment;

    DataSharingNotification() = default;

    virtual ~DataSharingNotification() = default;

    /**
     * Notifies of new data
     */
    inline void notify()
    {
        try
        {
            std::unique_lock<Segment::mutex> lock(notification_->notification_mutex);
            notification_->new_data.store(true);
            lock.unlock();
            notification_->notification_cv.notify_all();
        }
        catch (const boost::interprocess::interprocess_exception& /*e*/)
        {
            // Timeout when locking
        }
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
    struct alignas (8) Notification
    {
        //! CV to wait for new notifications
        Segment::condition_variable notification_cv;

        //! synchronization mutex
        Segment::mutex notification_mutex;

        //! New data available
        std::atomic<bool> new_data;
    };
#pragma warning(pop)

    static std::string generate_segment_name(
            const std::string& shared_dir,
            const GUID_t& reader_guid)
    {
        std::stringstream ss;
        if (!shared_dir.empty())
        {
            ss << shared_dir << "/";
        }
        ss << DataSharingNotification::domain_name() << "_" << reader_guid.guidPrefix << "_" << reader_guid.entityId;
        return ss.str();
    }

    bool create_and_init_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());

    bool open_and_init_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir = std::string());

    template <typename T>
    bool create_and_init_shared_segment_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir)
    {
        segment_id_ = reader_guid;
        segment_name_ = generate_segment_name(shared_dir, reader_guid);
        std::unique_ptr<T> local_segment;

        try
        {
            uint32_t per_allocation_extra_size = T::compute_per_allocation_extra_size(
                alignof(Notification), DataSharingNotification::domain_name());
            uint32_t segment_size = static_cast<uint32_t>(sizeof(Notification)) + per_allocation_extra_size;

            //Open the segment
            T::remove(segment_name_);

            local_segment.reset(
                new T(boost::interprocess::create_only,
                segment_name_,
                segment_size + T::EXTRA_SEGMENT_SIZE));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_LISTENER, "Failed to create segment " << segment_name_
                                                                                         << ": " << e.what());
            return false;
        }

        try
        {
            // Alloc and initialize the Node
            notification_ = local_segment->get().template construct<Notification>("notification_node")();
            notification_->new_data.store(false);
        }
        catch (std::exception& e)
        {
            T::remove(segment_name_);

            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_LISTENER, "Failed to create listener queue " << segment_name_
                                                                                                << ": " << e.what());
            return false;
        }

        segment_ = std::move(local_segment);
        owned_ = true;
        return true;
    }

    template <typename T>
    bool open_and_init_shared_segment_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir)
    {
        segment_id_ = reader_guid;
        segment_name_ = generate_segment_name(shared_dir, reader_guid);

        //Open the segment
        std::unique_ptr<T> local_segment;
        try
        {
            local_segment = std::unique_ptr<T>(
                new T(boost::interprocess::open_only,
                segment_name_.c_str()));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_LISTENER, "Failed to open segment " << segment_name_
                                                                                       << ": " << e.what());
            return false;
        }

        // Initialize values from the segment
        notification_ = (local_segment->get().template find<Notification>(
                    "notification_node")).first;
        if (!notification_)
        {
            local_segment.reset();

            EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_LISTENER, "Failed to open listener queue " << segment_name_);
            return false;
        }

        segment_ = std::move(local_segment);
        return true;
    }

    GUID_t segment_id_;         //< The ID of the segment is the GUID of the reader
    std::string segment_name_;  //< Segment name

    std::unique_ptr<Segment> segment_;  //< Shared memory segment
    Notification* notification_;        //< The notification data
    bool owned_ = false;                //< Whether the shared segment is owned by this instance
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGNOTIFICATION_HPP
