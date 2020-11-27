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
 * @file DataSharingNotification.cpp
 */

#include <rtps/history/DataSharingNotification.hpp>

#include <memory>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

std::shared_ptr<DataSharingNotification> DataSharingNotification::create_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    std::shared_ptr<DataSharingNotification> notification = std::make_shared<DataSharingNotification>();
    if (!notification->create_and_init_notification(reader_guid, shared_dir))
    {
        notification.reset();
    }
    return notification;
}

std::shared_ptr<DataSharingNotification> DataSharingNotification::open_notification(
        const GUID_t& writer_guid,
        const std::string& shared_dir)
{
    std::shared_ptr<DataSharingNotification> notification = std::make_shared<DataSharingNotification>();
    if (!notification->open_and_init_notification(writer_guid, shared_dir))
    {
        notification.reset();
    }
    return notification;
}

bool DataSharingNotification::create_and_init_notification(
        const GUID_t& reader_guid,
        const std::string& shared_dir)
{
    segment_id_ = reader_guid;
    segment_name_ = generate_segment_name(shared_dir, reader_guid);

    // Extra size for the internal allocator structures (512bytes estimated)
    uint32_t extra = 512;
    uint32_t per_allocation_extra_size = Segment::compute_per_allocation_extra_size(
        alignof(Notification), DataSharingNotification::domain_name());
    uint32_t segment_size = Notification::aligned_size() + per_allocation_extra_size;

    //Open the segment
    Segment::remove(segment_name_);
    try
    {
        segment_ = std::unique_ptr<Segment>(
            new Segment(boost::interprocess::create_only,
                segment_name_,
                segment_size + extra));
    }
    catch (const std::exception& e)
    {
        logError(HISTORY_DATASHARING_LISTENER, "Failed to create segment " << segment_name_
                                                                            << ": " << e.what());
        return false;
    }

    try
    {
        // Memset the whole segment to zero in order to force physical map of the buffer
        auto payload = segment_->get().allocate(segment_size);
        memset(payload, 0, segment_size);
        segment_->get().deallocate(payload);

        // Alloc and initialize the Node
        notification_ = segment_->get().construct<Notification>("notification_node")();
        notification_->new_data.store(false);
    }
    catch (std::exception& e)
    {
        Segment::remove(segment_name_);

        logError(HISTORY_DATASHARING_LISTENER, "Failed to create listener queue " << segment_name_
                                                                                    << ": " << e.what());
        return false;
    }

    return true;
}

bool DataSharingNotification::open_and_init_notification(
            const GUID_t& reader_guid,
            const std::string& shared_dir)
{
    segment_id_ = reader_guid;
    segment_name_ = generate_segment_name(shared_dir, reader_guid);

    //Open the segment
    try
    {
        segment_ = std::unique_ptr<Segment>(
            new Segment(boost::interprocess::open_only,
                segment_name_.c_str()));
    }
    catch (const std::exception& e)
    {
        logError(HISTORY_DATASHARING_LISTENER, "Failed to open segment " << segment_name_
                                                                            << ": " << e.what());
        return false;
    }

    // Initialize values from the segment
    notification_ = segment_->get().find<Notification>(
            "notification_node").first;
    if (!notification_)
    {
        segment_.reset();

        logError(HISTORY_DATASHARING_LISTENER, "Failed to open listener queue " << segment_name_);
        return false;
    }

    return true;
}


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
