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
 * @file SubscriptionMatchedStatus.h
*/

#ifndef _SUBSCRIPTION_MATCHED_STATUS_H_
#define _SUBSCRIPTION_MATCHED_STATUS_H_

#include <cstdint>

#include <fastrtps/rtps/common/InstanceHandle.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

namespace eprosima{
namespace fastdds{
namespace dds{

//! @brief A structure storing the subscription status
struct SubscriptionMatchedStatus
{
	//! @brief Constructor
    SubscriptionMatchedStatus()
        : total_count()
        , total_count_change()
        , current_count()
        , current_count_change()
        , last_publication_handle()
        , status(eprosima::fastrtps::rtps::MATCHED_MATCHING)
    {} 

    //! @brief Destructor
    ~SubscriptionMatchedStatus()
    {}

	//! @brief Total cumulative count the concerned reader discovered a match with a writer
	//! @details It found a writer for te same topic with a requested QoS that is compatible with that offered by the reader
	int32_t total_count = 0;

	//! @brief The change in total_count since the last time the listener was called or the status was read
	int32_t total_count_change = 0;

	//! @brief The number of writers currently matched to the concerned reader
	int32_t current_count = 0;

	//! @brief The change in current_count since the last time the listener was called or the status was read
	int32_t current_count_change = 0;

	//! @brief Handle to the last writer that matched the reader causing the status change
	eprosima::fastrtps::rtps::InstanceHandle_t last_publication_handle;

	//!Status
	eprosima::fastrtps::rtps::MatchingStatus status;
};

} //end of namespace dds
} //end of namespace fastdds
} //end of namespace eprosima
#endif
