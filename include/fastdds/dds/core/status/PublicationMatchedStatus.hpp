// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublicationMatchedStatus.hpp
*/

#ifndef _PUBLICATION_MATCHED_STATUS_HPP_
#define _PUBLICATION_MATCHED_STATUS_HPP_

#include <cstdint>
#include <fastrtps/rtps/common/InstanceHandle.h>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A structure storing the publication status
struct PublicationMatchedStatus
{
	//! @brief Total cumulative count the concerned writer discovered a match with a reader
	//! @details It found a reader for the same topic with a requested Qos that is compatible with that offered by the writer
    int32_t total_count = 0;

	//! @brief The change in total_count since the last time the listener was called or the status was read
    int32_t total_count_change = 0;

	//! @brief The number of readers currently matched to the concerned writer
    int32_t current_count = 0;

	//! @brief The change in current_count since the last time the listener was called or the status was read
    int32_t current_count_change = 0;

	//! @brief Handle to the last reader that matched the writer causing the status to change
	eprosima::fastrtps::rtps::InstanceHandle_t last_subscription_handle;
};

} //end of namespace dds
} //end of namespace fastdds
} //end of namespace eprosima
#endif
