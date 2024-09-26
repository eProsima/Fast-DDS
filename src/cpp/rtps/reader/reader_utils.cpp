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
 * @file reader_utils.cpp
 */

#include "reader_utils.hpp"

#include <fastdds/rtps/common/ChangeKind_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool change_is_relevant_for_filter(
        const CacheChange& change,
        const GUID& reader_guid,
        const IReaderDataFilter* filter)
{
    bool ret = true;

    // If the change has no payload, it should have an instanceHandle.
    // This is only allowed for UNREGISTERED and DISPOSED changes, where the instanceHandle is used to identify the
    // instance to unregister or dispose.
    if ((nullptr == change.serializedPayload.data) &&
            ((fastdds::rtps::ALIVE == change.kind) || !change.instanceHandle.isDefined()))
    {
        ret = false;
    }

    // Only evaluate filter on ALIVE changes, as UNREGISTERED and DISPOSED are always relevant
    if ((nullptr != filter) && (fastdds::rtps::ALIVE == change.kind) && (!filter->is_relevant(change, reader_guid)))
    {
        ret = false;
    }

    return ret;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
