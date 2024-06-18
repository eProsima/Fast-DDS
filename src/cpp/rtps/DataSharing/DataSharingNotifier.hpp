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

#ifndef RTPS_DATASHARING_DATASHARINGNOTIFIER_HPP
#define RTPS_DATASHARING_DATASHARINGNOTIFIER_HPP

#include "rtps/DataSharing/IDataSharingNotifier.hpp"
#include "rtps/DataSharing/DataSharingNotification.hpp"

#include <map>
#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {

class DataSharingNotifier : public IDataSharingNotifier
{

public:

    /**
     * Initializes a datasharing notifier for a reader
     * @param directory Sahred memory directory to use to open the shared notification
     */
    DataSharingNotifier(
            std::string directory)
        : directory_(directory)
    {
    }

    ~DataSharingNotifier() = default;

    /**
     * Links the notifier to a reader that will be notified
     * @param reader_guid GUID of the reader to notify
     */
    void enable(
            const GUID_t& reader_guid) override
    {
        shared_notification_ = DataSharingNotification::open_notification(reader_guid, directory_);
    }

    /**
     * Unlinks the notifier and its reader
     */
    void disable() override
    {
        shared_notification_.reset();
    }

    /**
     * @return whether the notifier is linked to a reader or not
     */
    bool is_enabled() override
    {
        return shared_notification_ != nullptr;
    }

    /**
     * Notifies to the reader
     */
    void notify() override
    {
        if (is_enabled())
        {
            EPROSIMA_LOG_INFO(RTPS_WRITER, "Notifying reader " << shared_notification_->reader());
            shared_notification_->notify();
        }
    }

protected:

    std::shared_ptr<DataSharingNotification> shared_notification_;
    std::string directory_;
};


}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGNOTIFIER_HPP
