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
 * @file DataSharingListener.hpp
 */

#ifndef RTPS_DATASHARING_DATASHARINGLISTENER_HPP
#define RTPS_DATASHARING_DATASHARINGLISTENER_HPP

#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/IDataSharingListener.hpp>
#include <rtps/DataSharing/DataSharingNotification.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

#include <memory>
#include <atomic>
#include <map>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataSharingListener : public IDataSharingListener
{

public:

    typedef DataSharingNotification::Notification Notification;
    typedef DataSharingNotification::Segment Segment;

    DataSharingListener(
        std::shared_ptr<DataSharingNotification> notification,
        const std::string& datasharing_pools_directory,
        ResourceLimitedContainerConfig limits,
        std::function<void(CacheChange_t*)> callback);

    virtual ~DataSharingListener();

    /**
     * Starts the listening thread.
     * @throw std::exception on error
     */
    void start() override;

    /**
     * Stops the listening thread.
     * @throw std::exception on error
     */
    void stop() override;

    bool add_datasharing_writer(
            const GUID_t& writer_guid,
            bool is_volatile) override;

    bool remove_datasharing_writer(
            const GUID_t& writer_guid) override;

    bool writer_is_matched(
            const GUID_t& writer_guid) const override;

    void notify() override;

protected:

    /**
     * The body for the listening thread
     */
    void run();

    /**
     * Processes a notification
     */
    void process_new_data();


    std::shared_ptr<DataSharingNotification> notification_;
    std::atomic<bool> is_running_;
    std::function<void(CacheChange_t*)> callback_;
    std::thread* listening_thread_;
    ResourceLimitedVector<std::shared_ptr<DataSharingPayloadPool>> writer_pools_;
    std::string datasharing_pools_directory_;

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGLISTENER_HPP
