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
 * @file DataSharingNotifier.hpp
 */

#include "rtps/DataSharing/DataSharingNotifier.hpp"

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool DataSharingNotifier::add_reader(
            const GUID_t& reader_guid)
{
    if (reader_is_subscribed(reader_guid))
    {
        logInfo(RTPS_WRITER, "Attempting to add existing datasharing reader " << reader_guid);
        return false;
    }

    auto notification = DataSharingNotification::open_notification(reader_guid, data_sharing_directory_);
    if (!shared_notifications_.push_back(notification))
    {
        logInfo(RTPS_WRITER, "Maximum number of matched data sharing readers reached. Fail to add reader " << reader_guid);
        return false;
    }
    return true;
}

bool DataSharingNotifier::remove_reader(
            const GUID_t& reader_guid)
{
    return shared_notifications_.remove_if (
            [reader_guid](const std::shared_ptr<DataSharingNotification> notification)
            {
                return notification->reader() == reader_guid;
            }
    );
}

void DataSharingNotifier::notify()
{
    for (auto it = shared_notifications_.begin(); it != shared_notifications_.end(); ++it)
    {
        logInfo(RTPS_WRITER, "Notifying reader " << (*it)->reader());
        (*it)->notify();
    }
}

bool DataSharingNotifier::reader_is_subscribed(
        const GUID_t& reader_guid) const
{
    auto it = std::find_if(shared_notifications_.begin(), shared_notifications_.end(),
        [reader_guid](const std::shared_ptr<DataSharingNotification> notification)
        {
            return notification->reader() == reader_guid;
        }
    );
    return (it != shared_notifications_.end());
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

