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
 * @file reader_utils.hpp
 */

#ifndef _FASTDDS_RTPS_READER_READERUTILS_H_
#define _FASTDDS_RTPS_READER_READERUTILS_H_

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/common/ChangeKind_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using CacheChange = fastdds::rtps::CacheChange_t;
using GUID = fastdds::rtps::GUID_t;

/**
 * @brief Check if a change is relevant for a reader.
 *
 * @param change The CacheChange_t to be evaluated.
 * @param reader_guid Reader's GUID_t.
 * @param filter The IReaderDataFilter to be used.
 *
 * @return true if relevant, false otherwise.
 */
bool change_is_relevant_for_filter(
        const CacheChange& change,
        const GUID& reader_guid,
        const IReaderDataFilter* filter);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima


#endif // _FASTDDS_RTPS_READER_READERUTILS_H_
