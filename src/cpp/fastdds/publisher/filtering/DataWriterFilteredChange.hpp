// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterFilteredChange.hpp
 */

#ifndef _FASTDDS_PUBLISHER_FILTERING_DATAWRITERFILTEREDCHANGE_HPP_
#define _FASTDDS_PUBLISHER_FILTERING_DATAWRITERFILTEREDCHANGE_HPP_

#include <fastdds/rtps/common/CacheChange.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A cache change that holds writer-side filtering information.
 */
struct DataWriterFilteredChange final : public fastdds::rtps::CacheChange_t
{
    /**
     * Construct a DataWriterFilteredChange.
     *
     * @param filter_allocation  Allocation configuration for the collection of filtered out readers.
     */
    explicit DataWriterFilteredChange(
            const fastdds::ResourceLimitedContainerConfig& filter_allocation)
        : fastdds::rtps::CacheChange_t()
        , filtered_out_readers(filter_allocation)
    {
    }

    ~DataWriterFilteredChange() override = default;

    /**
     * Query about the relevance of this change for certain reader.
     *
     * @param reader_guid  GUID of the reader for which relevance information should be returned.
     *
     * @return whether this change is relevant for the specified reader.
     */
    inline bool is_relevant_for(
            const fastdds::rtps::GUID_t& reader_guid) const
    {
        for (const fastdds::rtps::GUID_t& guid : filtered_out_readers)
        {
            if (guid == reader_guid)
            {
                return false;
            }
        }

        return true;
    }

    /// Collection with the GUIDs of the readers for which this change is not relevant.
    fastdds::ResourceLimitedVector<fastdds::rtps::GUID_t> filtered_out_readers;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_DATAWRITERFILTEREDCHANGE_HPP_
