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
 * @file ChangeKind_t.hpp
 */

#ifndef _FASTDDS_RTPS_COMMON_CHANGEKINDT_HPP_
#define _FASTDDS_RTPS_COMMON_CHANGEKINDT_HPP_

#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @enum ChangeKind_t, different types of CacheChange_t.
 * @ingroup COMMON_MODULE
 */
enum RTPS_DllAPI ChangeKind_t
{
    ALIVE,                            //!< ALIVE
    NOT_ALIVE_DISPOSED,               //!< NOT_ALIVE_DISPOSED
    NOT_ALIVE_UNREGISTERED,           //!< NOT_ALIVE_UNREGISTERED
    NOT_ALIVE_DISPOSED_UNREGISTERED   //!< NOT_ALIVE_DISPOSED_UNREGISTERED
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_CHANGEKINDT_HPP_ */
