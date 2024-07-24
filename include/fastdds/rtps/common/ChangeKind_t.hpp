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

#ifndef FASTDDS_RTPS_COMMON__CHANGEKIND_T_HPP
#define FASTDDS_RTPS_COMMON__CHANGEKIND_T_HPP

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Enumerates the different types of CacheChange_t.
 */
// *INDENT-OFF* : Does not understand the #if correctly and ends up removing the ;
//                at the end of the enum, which does not build.
#if defined(_WIN32)  // Doxygen does not understand exported enums
enum FASTDDS_EXPORTED_API ChangeKind_t
#else
enum ChangeKind_t
#endif // if defined(_WIN32)
{
    ALIVE,                          //!< ALIVE
    NOT_ALIVE_DISPOSED,             //!< NOT_ALIVE_DISPOSED
    NOT_ALIVE_UNREGISTERED,         //!< NOT_ALIVE_UNREGISTERED
    NOT_ALIVE_DISPOSED_UNREGISTERED //!< NOT_ALIVE_DISPOSED_UNREGISTERED
};
// *INDENT-ON*

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__CHANGEKIND_T_HPP
