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
 * @file LivelinessChangedStatus.hpp
 */

#ifndef FASTDDS_DDS_CORE_STATUS__LIVELINESSCHANGEDSTATUS_HPP
#define FASTDDS_DDS_CORE_STATUS__LIVELINESSCHANGEDSTATUS_HPP

#include <fastdds/dds/common/InstanceHandle.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the liveliness changed status
struct LivelinessChangedStatus
{
    //! @brief The total number of currently active publishers that write the topic read by the subscriber
    //! @details This count increases when a newly matched publisher asserts its liveliness for the first time
    //! or when a publisher previously considered to be not alive reasserts its liveliness. The count decreases
    //! when a publisher considered alive fails to assert its liveliness and becomes not alive, whether because
    //! it was deleted normally or for some other reason
    int32_t alive_count = 0;

    //! @brief The total count of current publishers that write the topic read by the subscriber that are no longer
    //! asserting their liveliness
    //! @details This count increases when a publisher considered alive fails to assert its liveliness and becomes
    //! not alive for some reason other than the normal deletion of that publisher. It decreases when a previously
    //! not alive publisher either reasserts its liveliness or is deleted normally
    int32_t not_alive_count = 0;

    //! @brief The change in the alive_count since the last time the listener was called or the status was read
    int32_t alive_count_change = 0;

    //! @brief The change in the not_alive_count since the last time the listener was called or the status was read
    int32_t not_alive_count_change = 0;

    //! @brief Handle to the last publisher whose change in liveliness caused this status to change
    InstanceHandle_t last_publication_handle;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_STATUS__LIVELINESSCHANGEDSTATUS_HPP
