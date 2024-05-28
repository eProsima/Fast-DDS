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

/**
 * @file BaseReader.hpp
 */

#ifndef RTPS_READER__BASEREADER_HPP
#define RTPS_READER__BASEREADER_HPP

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader
    : public fastrtps::rtps::RTPSReader
    , public fastdds::statistics::StatisticsReaderImpl
{
protected:

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen)
        : fastrtps::rtps::RTPSReader(pimpl, guid, att, hist, listen)
    {
    }

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen)
        : fastrtps::rtps::RTPSReader(pimpl, guid, att, payload_pool, hist, listen)
    {
    }

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastrtps::rtps::IChangePool>& change_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen)
        : fastrtps::rtps::RTPSReader(pimpl, guid, att, payload_pool, change_pool, hist, listen)
    {
    }

    virtual ~BaseReader() = default;

public:

#ifdef FASTDDS_STATISTICS

    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override
    {
        return add_statistics_listener_impl(listener);
    }

    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override
    {
        return remove_statistics_listener_impl(listener);
    }

    void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) override
    {
        set_enabled_statistics_writers_mask_impl(enabled_writers);
    }

#endif // FASTDDS_STATISTICS

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif /* RTPS_READER__BASEREADER_HPP */
