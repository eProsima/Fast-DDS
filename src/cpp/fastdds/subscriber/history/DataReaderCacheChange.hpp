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
 * @file DataReaderCacheChange.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_
#define _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_

#include <cstdint>

#include <fastdds/rtps/common/CacheChange.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/// A DataReader cache entry
using DataReaderCacheChange = fastrtps::rtps::CacheChange_t*;

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_
