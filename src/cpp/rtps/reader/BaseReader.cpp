// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * BaseReader.cpp
 */

#include <rtps/reader/BaseReader.hpp>

#include <fastdds/rtps/reader/RTPSReader.h>

#include <rtps/DataSharing/DataSharingListener.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

BaseReader::BaseReader(
        fastrtps::rtps::RTPSParticipantImpl* pimpl,
        const fastrtps::rtps::GUID_t& guid,
        const fastrtps::rtps::ReaderAttributes& att,
        fastrtps::rtps::ReaderHistory* hist,
        fastrtps::rtps::ReaderListener* listen)
    : fastrtps::rtps::RTPSReader(pimpl, guid, att, hist, listen)
{
    setup_datasharing(att);
}

BaseReader::BaseReader(
        fastrtps::rtps::RTPSParticipantImpl* pimpl,
        const fastrtps::rtps::GUID_t& guid,
        const fastrtps::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
        fastrtps::rtps::ReaderHistory* hist,
        fastrtps::rtps::ReaderListener* listen)
    : fastrtps::rtps::RTPSReader(pimpl, guid, att, payload_pool, hist, listen)
{
    setup_datasharing(att);
}

BaseReader::BaseReader(
        fastrtps::rtps::RTPSParticipantImpl* pimpl,
        const fastrtps::rtps::GUID_t& guid,
        const fastrtps::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
        const std::shared_ptr<fastrtps::rtps::IChangePool>& change_pool,
        fastrtps::rtps::ReaderHistory* hist,
        fastrtps::rtps::ReaderListener* listen)
    : fastrtps::rtps::RTPSReader(pimpl, guid, att, payload_pool, change_pool, hist, listen)
{
    setup_datasharing(att);
}

BaseReader::~BaseReader()
{
}

#ifdef FASTDDS_STATISTICS

bool BaseReader::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return add_statistics_listener_impl(listener);
}

bool BaseReader::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return remove_statistics_listener_impl(listener);
}

void BaseReader::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    set_enabled_statistics_writers_mask_impl(enabled_writers);
}

#endif // FASTDDS_STATISTICS

void BaseReader::setup_datasharing(
        const fastrtps::rtps::ReaderAttributes& att)
{
    using namespace fastrtps::rtps;

    if (att.endpoint.data_sharing_configuration().kind() != fastdds::dds::DataSharingKind::OFF)
    {
        using std::placeholders::_1;
        std::shared_ptr<DataSharingNotification> notification =
            DataSharingNotification::create_notification(
                getGuid(), att.endpoint.data_sharing_configuration().shm_directory());
        if (notification)
        {
            is_datasharing_compatible_ = true;
            datasharing_listener_.reset(new DataSharingListener(
                notification,
                att.endpoint.data_sharing_configuration().shm_directory(),
                att.data_sharing_listener_thread,
                att.matched_writers_allocation,
                this));

            // We can start the listener here, as no writer can be matched already,
            // so no notification will occur until the non-virtual instance is constructed.
            // But we need to stop the listener in the non-virtual instance destructor.
            datasharing_listener_->start();
        }
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
