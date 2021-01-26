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
#include <fastdds/rtps/reader/RTPSReader.h>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {


DataSharingListener::DataSharingListener(
        std::shared_ptr<DataSharingNotification> notification,
        const std::string& datasharing_pools_directory,
        ResourceLimitedContainerConfig limits,
        RTPSReader* reader)
    : notification_(notification)
    , is_running_(false)
    , reader_(reader)
    , writer_pools_(limits)
    , writer_pools_changed_(false)
    , datasharing_pools_directory_(datasharing_pools_directory)
{
}

DataSharingListener::~DataSharingListener()
{
    stop();
    notification_->destroy();
}

void DataSharingListener::run()
{
    std::unique_lock<Segment::mutex> lock(notification_->notification_->notification_mutex, std::defer_lock);
    while (is_running_.load())
    {
        lock.lock();
        notification_->notification_->notification_cv.wait(lock, [&]
                {
                    return !is_running_.load() || notification_->notification_->new_data.load();
                });

        lock.unlock();

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
    // Check the thread
    bool was_running = is_running_.exchange(true);
    if (was_running)
    {
        return;
    }

    // Initialize the thread
    listening_thread_ = new std::thread(&DataSharingListener::run, this);
}

void DataSharingListener::stop()
{
    // Notify the listening thread that is no longer running
    bool was_running = is_running_.exchange(false);
    if (!was_running)
    {
        return;
    }

    // Notify the thread and wait for it to finish
    notification_->notify();
    listening_thread_->join();
    delete listening_thread_;
}

void DataSharingListener::process_new_data ()
{
    logInfo(RTPS_READER, "Received new data notification");

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

            if (has_new_payload)
            {
                if (last_sequence != c_SequenceNumber_Unknown && ch.sequenceNumber != last_sequence + 1)
                {
                    logWarning(RTPS_READER, "GAP (" << last_sequence + 1 << " - " << ch.sequenceNumber - 1 << ")"
                                                    << " detected on datasharing writer " << pool->writer());
                    reader_->processGapMsg(pool->writer(), last_sequence + 1, SequenceNumberSet_t(ch.sequenceNumber));
                }

                if (last_sequence == c_SequenceNumber_Unknown && ch.sequenceNumber != SequenceNumber_t(0, 1))
                {
                    logInfo(RTPS_READER, "First change with SN " << ch.sequenceNumber
                                                                 << " detected on datasharing writer " <<
                            pool->writer());
                    reader_->processGapMsg(pool->writer(), SequenceNumber_t(0, 1), SequenceNumberSet_t(
                                ch.sequenceNumber));
                }

                logInfo(RTPS_READER, "New data found on writer " << pool->writer()
                                                                 << " with SN " << ch.sequenceNumber);

                if (reader_->processDataMsg(&ch))
                {
                    pool->release_payload(ch);
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
        bool is_volatile)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (writer_is_matched(writer_guid))
    {
        logInfo(RTPS_READER, "Attempting to add existing datasharing writer " << writer_guid);
        return false;
    }

    std::shared_ptr<ReaderPool> pool =
            std::static_pointer_cast<ReaderPool>(DataSharingPayloadPool::get_reader_pool(is_volatile));
    pool->init_shared_memory(writer_guid, datasharing_pools_directory_);
    writer_pools_.emplace_back(pool, pool->last_liveliness_sequence());
    writer_pools_changed_.store(true);

    return true;
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

void DataSharingListener::change_removed_with_timestamp(
        int64_t timestamp)
{
    // This method should be called from the RTPSReader,
    // then, the reader's lock is protecting the concurrency on the value updates.
    if (timestamp > notification_->notification_->ack_timestamp)
    {
        notification_->notification_->ack_timestamp = timestamp;
        for (auto it = writer_pools_.begin(); it != writer_pools_.end(); ++it)
        {
            // Notify all writers in case any is waiting for a recyclable payload
            it->pool->notify();
        }
    }
}

void DataSharingListener::change_added_with_timestamp(
        int64_t timestamp)
{
    // This method should be called from the RTPSReader,
    // then, the reader's lock is protecting the concurrency on the value updates.
    if (timestamp < notification_->notification_->ack_timestamp)
    {
        notification_->notification_->ack_timestamp = timestamp;
    }
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
