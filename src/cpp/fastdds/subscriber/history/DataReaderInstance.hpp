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
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_
