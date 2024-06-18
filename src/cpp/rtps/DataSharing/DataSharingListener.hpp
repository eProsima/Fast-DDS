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

#include <atomic>
#include <map>
#include <memory>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/DataSharing/IDataSharingListener.hpp>
#include <rtps/DataSharing/DataSharingNotification.hpp>
#include <rtps/DataSharing/ReaderPool.hpp>
#include <utils/thread.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {

class BaseReader;

} // namespace rtps
} // namespace fastdds

namespace fastdds {
namespace rtps {


class DataSharingListener : public IDataSharingListener
{

    using BaseReader = fastdds::rtps::BaseReader;
    using ThreadSettings = fastdds::rtps::ThreadSettings;

public:

    typedef DataSharingNotification::Notification Notification;
    typedef DataSharingNotification::Segment Segment;

    DataSharingListener(
            std::shared_ptr<DataSharingNotification> notification,
            const std::string& datasharing_pools_directory,
            const ThreadSettings& thr_config,
            ResourceLimitedContainerConfig limits,
            BaseReader* reader);

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
            bool is_volatile,
            int32_t reader_history_max_samples) override;

    bool remove_datasharing_writer(
            const GUID_t& writer_guid) override;

    bool writer_is_matched(
            const GUID_t& writer_guid) const override;

    void notify(
            bool same_thread) override;

    std::shared_ptr<ReaderPool> get_pool_for_writer(
            const GUID_t& writer_guid) override;

protected:

    /**
     * The body for the listening thread
     */
    void run();

    /**
     * Processes a notification
     */
    void process_new_data();

    struct WriterInfo
    {
        std::shared_ptr<ReaderPool> pool;
        uint32_t last_assertion_sequence = 0;

        WriterInfo() = default;
        WriterInfo(
                std::shared_ptr<ReaderPool> writer_pool,
                uint32_t assertion)
            : pool(writer_pool)
            , last_assertion_sequence(assertion)
        {
        }

    };

    std::shared_ptr<DataSharingNotification> notification_;
    std::atomic<bool> is_running_;
    BaseReader* reader_;
    eprosima::thread listening_thread_;
    ResourceLimitedVector<WriterInfo> writer_pools_;
    std::atomic<bool> writer_pools_changed_;
    std::string datasharing_pools_directory_;
    ThreadSettings thread_config_;
    mutable std::mutex mutex_;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_DATASHARINGLISTENER_HPP
