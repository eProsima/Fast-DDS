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

#include <rtps/history/DataSharingListener.hpp>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {


DataSharingListener::DataSharingListener(
        std::shared_ptr<DataSharingNotification> notification,
        const std::string& datasharing_pools_directory,
        std::function<void(CacheChange_t*)> callback)
    : notification_(notification)
    , is_running_(false)
    , callback_(callback)
    , datasharing_pools_directory_(datasharing_pools_directory)
{
}

DataSharingListener::~DataSharingListener()
{
    stop();
}

void DataSharingListener::run()
{
    std::unique_lock<Segment::mutex> lock(notification_->notification_->notification_mutex);
    while (is_running_.load())
    {
        notification_->notification_->notification_cv.wait(lock, [&]
                {
                    return !is_running_.load() || notification_->notification_->new_data.load();
                });
        
        if (!is_running_.load())
        {
            // Woke up because listener is stopped
            return;
        }

        do
        {
            // If during the processing some other writer adds a notification,
            // it will also set notification_->notification_->new_data
            notification_->notification_->new_data.store(false);
            lock.unlock();
            process_new_data();
            lock.lock();
        } while (is_running_.load() && notification_->notification_->new_data.load());
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

    // Loop on the writers looking for data not read yet
    for (auto it = writer_pools_.begin(); it != writer_pools_.end(); ++it)
    {
        bool has_new_payload = true;
        while(has_new_payload)
        {
            CacheChange_t ch;
            has_new_payload = it->second->get_next_unread_payload(ch);

            if (has_new_payload)
            {
                logInfo(RTPS_READER, "New data found on writer " << it->first
                        << " with SN " << ch.sequenceNumber);

                // TODO [ILG] process security attributes
/*
#if HAVE_SECURITY
               if (getAttributes().security_attributes().is_payload_protected)
                {
                   if (mp_RTPSParticipant->security_manager().decode_serialized_payload(ch.serializedPayload, crypto_payload_,
                          getGuid(), ch.writerGUID))
                   {
                        ch.serializedPayload.data = crypto_payload_.data;
                        ch.serializedPayload.length = crypto_payload_.length;
                    }
                }
#endif  // HAVE_SECURITY
*/
                callback_(&ch);
                it->second->release_payload(ch);
            }
        }
    }
}

bool DataSharingListener::add_datasharing_writer(
    const GUID_t& writer_guid,
    const PoolConfig& pool_config)
{
    // TODO [ULG] adding and removing must be protected
    if (writer_pools_.find(writer_guid) != writer_pools_.end())
    {
        logInfo(RTPS_READER, "Attempting to add existing datasharing writer " << writer_guid);
        return false;
    }

    std::shared_ptr<DataSharingPayloadPool> pool = DataSharingPayloadPool::get_reader_pool(pool_config);
    pool->init_shared_memory(writer_guid, datasharing_pools_directory_);
    writer_pools_[writer_guid] = pool;

    return true;
}

bool DataSharingListener::remove_datasharing_writer(
    const GUID_t& writer_guid)
{
    auto it = writer_pools_.find(writer_guid);
    if (it != writer_pools_.end())
    {
        writer_pools_.erase(it);
        return true;
    }
    return false;
}


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
