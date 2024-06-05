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
 * @file IDataSharingListener.hpp
 */

#ifndef RTPS_DATASHARING_IDATASHARINGLISTENER_HPP
#define RTPS_DATASHARING_IDATASHARINGLISTENER_HPP

#include <fastdds/dds/log/Log.hpp>
#include <rtps/DataSharing/DataSharingNotification.hpp>
#include <rtps/DataSharing/ReaderPool.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <memory>
#include <atomic>
#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct GUID_t;

class IDataSharingListener
{

public:

    IDataSharingListener() = default;

    virtual ~IDataSharingListener() = default;

    /**
     * Starts the listening operation.
     */
    virtual void start() = 0;

    /**
     * Stops the listening operation.
     */
    virtual void stop() = 0;

    /**
     * Add a new writer to the listening
     *
     * On volatile readers, the listener will discard any previous change published by the writer
     * and will notify only of the changes published from the moment the writer is added to the listening.
     * On transient readers, immediately after this method is called, the listener will notify
     * of all the changes available in the writer's history before starting the listening operation
     * for new changes.
     *
     * @param writer_guid The GUID of the writer to listen to
     * @param is_reader_volatile Whether the listening reader is volatile.
     *
     * @return true if the writer was added, false if the writer was already being listened.
     */
    virtual bool add_datasharing_writer(
            const GUID_t& writer_guid,
            bool is_reader_volatile,
            int32_t reader_history_max_samples) = 0;

    /**
     * Removes a writer from the listening. The changes in the writer's history will not be
     * notified anymore.
     *
     * @param writer_guid The GUID of the writer to remove
     *
     * @return true if the writer was removed, false if the writer was not being listened.
     */
    virtual bool remove_datasharing_writer(
            const GUID_t& writer_guid) = 0;

    /**
     * @return true if the writer is being listened.
     */
    virtual bool writer_is_matched(
            const GUID_t& writer_guid) const = 0;

    /**
     * Wakes the listener and signals that there is some new data in any of the writers
     * being listened.
     *
     * @param same_thread Whether the data processing should be done on the thread calling this method or
     *                    the listener thread.
     */
    virtual void notify(
            bool same_thread) = 0;

    /**
     * Returns the local datasharing pool for the specified remote writer
     *
     * @param writer_guid The GUID of the remote writer
     * @return the local pool for the given writer or null if the writer is not being listened
     */
    virtual std::shared_ptr<ReaderPool> get_pool_for_writer(
            const GUID_t& writer_guid) = 0;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_IDATASHARINGLISTENER_HPP
