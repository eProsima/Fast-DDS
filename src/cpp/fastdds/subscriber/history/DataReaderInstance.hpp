// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataReaderInstance.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERINSTANCE_HPP_
#define _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERINSTANCE_HPP_

#include <chrono>
#include <cstdint>

#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>

#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

#include "DataReaderCacheChange.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/// Book-keeping information for an instance
struct DataReaderInstance
{
    using ChangeCollection = eprosima::fastrtps::ResourceLimitedVector<DataReaderCacheChange, std::true_type>;

    //! A vector of DataReader changes belonging to the same instance
    ChangeCollection cache_changes;
    //! The time when the group will miss the deadline
    std::chrono::steady_clock::time_point next_deadline_us;
    //! Current view state of the instance
    ViewStateKind view_state = ViewStateKind::NEW_VIEW_STATE;
    //! Current instance state of the instance
    InstanceStateKind instance_state = InstanceStateKind::ALIVE_INSTANCE_STATE;
    //! Current disposed generation of the instance
    int32_t disposed_generation_count = 0;
    //! Current no_writers generation of the instance
    int32_t no_writers_generation_count = 0;

    bool update_state(
            const fastrtps::rtps::ChangeKind_t change_kind,
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength = 0)
    {
        bool ret_val = false;

        switch (change_kind)
        {
            case fastrtps::rtps::ALIVE:
                ret_val = writer_alive(writer_guid, ownership_strength);
                break;

            case fastrtps::rtps::NOT_ALIVE_DISPOSED:
                ret_val = writer_dispose(writer_guid, ownership_strength);
                break;

            case fastrtps::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED:
                ret_val = writer_dispose(writer_guid, ownership_strength);
                ret_val |= writer_unregister(writer_guid);
                break;

            case fastrtps::rtps::NOT_ALIVE_UNREGISTERED:
                ret_val = writer_unregister(writer_guid);
                break;

            default:
                // TODO (Miguel C): log error / assert
                break;
        }

        return ret_val;
    }

private:

    bool writer_alive(
            const fastrtps::rtps::GUID_t& /*writer_guid*/,
            const uint32_t /*ownership_strength*/)
    {
        return false;
    }

    bool writer_dispose(
            const fastrtps::rtps::GUID_t& /*writer_guid*/,
            const uint32_t /*ownership_strength*/)
    {
        return false;
    }

    bool writer_unregister(
            const fastrtps::rtps::GUID_t& /*writer_guid*/)
    {
        return false;
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_
