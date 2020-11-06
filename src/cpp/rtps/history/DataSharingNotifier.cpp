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

#include "rtps/history/DataSharingNotifier.hpp"

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool DataSharingNotifier::add_reader(
            const GUID_t& reader_guid)
{
    if (shared_notifications_.find(reader_guid) != shared_notifications_.end())
    {
        logInfo(RTPS_WRITER, "Attempting to add existing datasharing reader " << reader_guid);
        return false;
    }

    auto notification = DataSharingNotification::open_notification(reader_guid);
    shared_notifications_[reader_guid] = notification;
    return true;
}

bool DataSharingNotifier::remove_reader(
            const GUID_t& reader_guid)
{
    auto it = shared_notifications_.find(reader_guid);
    if (it != shared_notifications_.end())
    {
        shared_notifications_[reader_guid].reset();
        return true;
    }
    return false;
}

void DataSharingNotifier::notify()
{
    for (auto it : shared_notifications_)
    {
        logInfo(RTPS_WRITER, "Notifying reader " << it.first);
        it.second->notify();
    }
}


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

