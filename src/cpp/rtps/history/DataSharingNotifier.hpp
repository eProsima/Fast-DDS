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

#ifndef RTPS_HISTORY_DATASHARINGNOTIFIER_HPP
#define RTPS_HISTORY_DATASHARINGNOTIFIER_HPP

#include "rtps/history/DataSharingNotification.hpp"

#include <map>
#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingNotifier
{

public:

    DataSharingNotifier() = default;

    ~DataSharingNotifier() = default;

    /**
     * Initializes a datasharing notifier for the reader
     * 
     * @param reader_guid GUID of the reader to add to the notification list
     * @return Whether the initialization was  successful
     */
    bool add_reader(
            const GUID_t& reader_guid);

    /**
     * Stops the datasharing notifier for the reader
     * 
     * @param reader_guid GUID of the reader to remove from the notification list
     * @return Whether the stop was successful
     */
    bool remove_reader(
            const GUID_t& reader_guid);

    /**
     * Checks if the reader with the given GUID is subscribed to data sharing notifications
     * 
     * @param reader_guid GUID of the reader to check
     * @return Whether the reader is subscribed or not
     */
    inline bool reader_is_subscribed(
            const GUID_t& reader_guid) const
    {
        return shared_notifications_.find(reader_guid) != shared_notifications_.end();
    }

    /**
     * Notifies to all subscribed readers
     */
    void notify();

    /**
     * Checks if there is any listener subscribed
     * 
     * @return True if there is no listener subscribed
     */
    inline bool empty() const
    {
        return shared_notifications_.empty();
    }

protected:

    std::map<GUID_t, std::shared_ptr<DataSharingNotification>> shared_notifications_;
};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_DATASHARINGNOTIFIER_HPP
