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
 * @file DataSharingNotification.cpp
 */

#include <rtps/DataSharing/DataSharingNotification.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

std::shared_ptr<DataSharingNotification> DataSharingNotification::create_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    std::shared_ptr<DataSharingNotification> notification = std::make_shared<DataSharingNotification>();
    if (!notification->create_and_init_notification(reader_guid, shared_dir))
    {
        notification.reset();
    }
    return notification;
}

std::shared_ptr<DataSharingNotification> DataSharingNotification::open_notification(
        const GUID_t& writer_guid,
        const std::string& shared_dir)
{
    std::shared_ptr<DataSharingNotification> notification = std::make_shared<DataSharingNotification>();
    if (!notification->open_and_init_notification(writer_guid, shared_dir))
    {
        notification.reset();
    }
    return notification;
}

bool DataSharingNotification::create_and_init_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    if (shared_dir.empty())
    {
        return create_and_init_shared_segment_notification<fastdds::rtps::SharedMemSegment>(reader_guid,
                       shared_dir);
    }
    else
    {
        return create_and_init_shared_segment_notification<fastdds::rtps::SharedFileSegment>(reader_guid,
                       shared_dir);
    }
}

bool DataSharingNotification::open_and_init_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    if (shared_dir.empty())
    {
        return open_and_init_shared_segment_notification<fastdds::rtps::SharedMemSegment>(reader_guid,
                       shared_dir);
    }
    else
    {
        return open_and_init_shared_segment_notification<fastdds::rtps::SharedFileSegment>(reader_guid,
                       shared_dir);
    }
}

void DataSharingNotification::destroy()
{
    if (owned_)
    {
        // We cannot destroy the objects in the SHM, as the Writer may still be using them.
        // We just remove the segment, and when the Writer closes it, it will be removed from the system.
        segment_->remove();

        owned_ = false;
    }
    else
    {
        EPROSIMA_LOG_ERROR(HISTORY_DATASHARING_LISTENER,
                "Trying to destroy non-owned notification segment " << segment_name_);
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
