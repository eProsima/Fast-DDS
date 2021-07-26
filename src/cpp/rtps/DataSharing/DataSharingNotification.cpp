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
#include <fastdds/rtps/common/Time_t.h>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

std::shared_ptr<DataSharingNotification> DataSharingNotification::create_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    std::shared_ptr<DataSharingNotification> notification = std::make_shared<DataSharingNotification>();
    bool create_result = false;
    if (shared_dir.empty())
    {
        create_result = notification->create_and_init_notification<fastdds::rtps::SharedMemSegment>(reader_guid, shared_dir);
    }
    else
    {
        create_result = notification->create_and_init_notification<fastdds::rtps::SharedFileSegment>(reader_guid, shared_dir);
    }

    if (!create_result)
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
    bool open_result = false;
    if (shared_dir.empty())
    {
        open_result = notification->open_and_init_notification<fastdds::rtps::SharedMemSegment>(writer_guid, shared_dir);
    }
    else
    {
        open_result = notification->open_and_init_notification<fastdds::rtps::SharedFileSegment>(writer_guid, shared_dir);
    }

    if (!open_result)
    {
        notification.reset();
    }
    return notification;
}

void DataSharingNotification::destroy()
{
    if (owned_)
    {
        // We cannot destroy the objects in the SHM, as the Writer may still be using them.
        // We just remove the segment, and when the Writer closes it, it will be removed from the system.
        segment_->remove(segment_name_);

        owned_ = false;
    }
    else
    {
        logError(HISTORY_DATASHARING_LISTENER, "Trying to destroy non-owned notification segment " << segment_name_);
    }
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
