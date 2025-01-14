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
 * @file DataSharingListener.cpp
 */

#include <rtps/DataSharing/DataSharingListener.hpp>

#include <rtps/reader/BaseReader.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

using BaseReader = fastdds::rtps::BaseReader;
using ThreadSettings = fastdds::rtps::ThreadSettings;

DataSharingListener::DataSharingListener(
        std::shared_ptr<DataSharingNotification> notification,
        const std::string& datasharing_pools_directory,
        const ThreadSettings& thr_config,
        ResourceLimitedContainerConfig limits,
        BaseReader* reader)
    : notification_(notification)
    , is_running_(false)
    , reader_(reader)
    , writer_pools_(limits)
    , writer_pools_changed_(false)
    , datasharing_pools_directory_(datasharing_pools_directory)
    , thread_config_(thr_config)
{
}

DataSharingListener::~DataSharingListener()
{
    stop();
    notification_->destroy();
}

void DataSharingListener::run()
{
    while (is_running_.load())
    {
        try
        {
            std::unique_lock<Segment::mutex> lock(notification_->notification_->notification_mutex);
            notification_->notification_->notification_cv.wait(lock, [&]
                    {
                        return !is_running_.load() || notification_->notification_->new_data.load();
                    });
        }
        catch (const boost::interprocess::interprocess_exception& /*e*/)
        {
            // Timeout when locking
            continue;
        }

        if (!is_running_.load())
        {
            // Woke up because listener is stopped
            return;
        }

        do
        {
            process_new_data();

            // If some writer added new data, there may be something to read.
            // If there were matching/unmatching, we may not have finished our last loop
        } while (is_running_.load() &&
        (notification_->notification_->new_data.load() || writer_pools_changed_.load(std::memory_order_relaxed)));
    }
}

void DataSharingListener::start()
{
    std::lock_guard<std::mutex> guard(mutex_);

    // Check the thread
    bool was_running = is_running_.exchange(true);
    if (was_running)
    {
        return;
    }

    // Initialize the thread
    uint32_t thread_id = reader_->getGuid().entityId.to_uint32() & 0x0000FFFF;
    listening_thread_ = create_thread([this]()
                    {
                        run();
                    }, thread_config_, "dds.dsha.%u", thread_id);
}

void DataSharingListener::stop()
{
    {
        std::lock_guard<std::mutex> guard(mutex_);

        // Notify the listening thread that is no longer running
        bool was_running = is_running_.exchange(false);
        if (!was_running)
        {
            return;
        }
    }

    // Notify the thread and wait for it to finish
    notification_->notify();
    listening_thread_.join();
}

void DataSharingListener::process_new_data ()
{
    EPROSIMA_LOG_INFO(RTPS_READER, "Received new data notification");

    std::unique_lock<std::mutex> lock(mutex_);

    // It is safe to 'forget' any change now
    notification_->notification_->new_data.store(false);
    // All places where this is set to true is locked by the same mutex, memory_order_relaxed is enough
    writer_pools_changed_.store(false, std::memory_order_relaxed);

    // Loop on the writers looking for data not read yet
    for (auto it = writer_pools_.begin(); it != writer_pools_.end(); ++it)
    {
        //First see if we have some liveliness asertion pending
        bool liveliness_assertion_needed = false;
        uint32_t new_assertion_sequence = it->pool->last_liveliness_sequence();
        if (it->last_assertion_sequence != new_assertion_sequence)
        {
            liveliness_assertion_needed = true;
            it->last_assertion_sequence = new_assertion_sequence;
        }

        // Take the pool to free the lock
        std::shared_ptr<ReaderPool> pool = it->pool;
        lock.unlock();

        if (liveliness_assertion_needed)
        {
            reader_->assert_writer_liveliness(pool->writer());
        }

        uint64_t last_payload = pool->end();
        bool has_new_payload = true;
        while (has_new_payload)
        {
            CacheChange_t ch;
            SequenceNumber_t last_sequence = c_SequenceNumber_Unknown;
            pool->get_next_unread_payload(ch, last_sequence, last_payload);
            has_new_payload = ch.sequenceNumber != c_SequenceNumber_Unknown;

            if (has_new_payload && ch.sequenceNumber > SequenceNumber_t(0, 0))
            {
                if (last_sequence != c_SequenceNumber_Unknown && ch.sequenceNumber > last_sequence + 1)
                {
                    EPROSIMA_LOG_WARNING(RTPS_READER, "GAP (" << last_sequence + 1 << " - " << ch.sequenceNumber - 1 << ")"
                                                              << " detected on datasharing writer " << pool->writer());
                    reader_->process_gap_msg(pool->writer(), last_sequence + 1,
                            SequenceNumberSet_t(ch.sequenceNumber), c_VendorId_eProsima);
                }

                if (last_sequence == c_SequenceNumber_Unknown && ch.sequenceNumber > SequenceNumber_t(0, 1))
                {
                    EPROSIMA_LOG_INFO(RTPS_READER, "First change with SN " << ch.sequenceNumber
                                                                           << " detected on datasharing writer " <<
                            pool->writer());
                    reader_->process_gap_msg(pool->writer(), SequenceNumber_t(0, 1),
                            SequenceNumberSet_t(ch.sequenceNumber), c_VendorId_eProsima);
                }

                EPROSIMA_LOG_INFO(RTPS_READER, "New data found on writer " << pool->writer()
                                                                           << " with SN " << ch.sequenceNumber);

                if (reader_->process_data_msg(&ch))
                {
                    pool->release_payload(ch.serializedPayload);
                    pool->advance_to_next_payload();
                }
            }

            if (writer_pools_changed_.load(std::memory_order_relaxed))
            {
                // Break the while on the current writer (it may have been removed)
                break;
            }
        }

        // Lock again for the next loop
        lock.lock();

        if (writer_pools_changed_.load(std::memory_order_relaxed))
        {
            // Break the loop over the writers (itearators may have been invalidated)
            break;
        }
    }
}

bool DataSharingListener::add_datasharing_writer(
        const GUID_t& writer_guid,
        bool is_volatile,
        int32_t reader_history_max_samples)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (writer_is_matched(writer_guid))
    {
        EPROSIMA_LOG_INFO(RTPS_READER, "Attempting to add existing datasharing writer " << writer_guid);
        return false;
    }

    std::shared_ptr<ReaderPool> pool =
            std::static_pointer_cast<ReaderPool>(DataSharingPayloadPool::get_reader_pool(is_volatile));
    if (pool->init_shared_memory(writer_guid, datasharing_pools_directory_))
    {
        if (0 >= reader_history_max_samples ||
                reader_history_max_samples >= static_cast<int32_t>(pool->history_size()))
        {
            EPROSIMA_LOG_WARNING(RTPS_READER,
                    "Reader " << reader_->getGuid() << " was configured to have a large history (" <<
                    reader_history_max_samples << " max samples), but the history size used with writer " <<
                    writer_guid << " will be " << pool->history_size() << " max samples.");
        }
        writer_pools_.emplace_back(pool, pool->last_liveliness_sequence());
        writer_pools_changed_.store(true);
        return true;
    }

    return false;
}

bool DataSharingListener::remove_datasharing_writer(
        const GUID_t& writer_guid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool found = writer_pools_.remove_if (
        [writer_guid](const WriterInfo& info)
        {
            return info.pool->writer() == writer_guid;
        }
        );

    if (found)
    {
        writer_pools_changed_.store(true);
    }

    return found;
}

bool DataSharingListener::writer_is_matched(
        const GUID_t& writer_guid) const
{
    auto it = std::find_if(writer_pools_.begin(), writer_pools_.end(),
                    [&writer_guid](const WriterInfo& info)
                    {
                        return info.pool->writer() == writer_guid;
                    }
                    );
    return (it != writer_pools_.end());
}

void DataSharingListener::notify(
        bool same_thread)
{
    if (same_thread)
    {
        process_new_data();
    }
    else
    {
        notification_->notify();
    }
}

std::shared_ptr<ReaderPool> DataSharingListener::get_pool_for_writer(
        const GUID_t& writer_guid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(writer_pools_.begin(), writer_pools_.end(),
                    [&writer_guid](const WriterInfo& info)
                    {
                        return info.pool->writer() == writer_guid;
                    }
                    );
    if (it != writer_pools_.end())
    {
        return it->pool;
    }
    return std::shared_ptr<ReaderPool>(nullptr);
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
