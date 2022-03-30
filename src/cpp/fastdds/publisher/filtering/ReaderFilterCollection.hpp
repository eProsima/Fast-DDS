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
 * @file ReaderFilterCollection.hpp
 */

#ifndef _FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_
#define _FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_

#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/Guid.h>

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/filtering/ReaderFilterInformation.hpp>

#include <utils/collections/node_size_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class ReaderFilterCollection
{
    using reader_filter_map_helper =
            utilities::collections::map_size_helper<fastrtps::rtps::GUID_t, ReaderFilterInformation>;

public:

    explicit ReaderFilterCollection(
            const fastrtps::ResourceLimitedContainerConfig& allocation)
        : reader_filter_allocator_(
            reader_filter_map_helper::node_size,
            reader_filter_map_helper::min_pool_size<pool_allocator_t>(allocation.initial))
        , reader_filters_(reader_filter_allocator_)
        , max_filters_(allocation.maximum)
    {
    }

    ~ReaderFilterCollection()
    {
        for (auto& item : reader_filters_)
        {
            destroy_filter(item.second);
        }
    }

    void remove_reader(
            const fastrtps::rtps::GUID_t& guid)
    {
        auto it = reader_filters_.find(guid);
        if (it != reader_filters_.end())
        {
            destroy_filter(it->second);
            reader_filters_.erase(it);
        }
    }

    void update_reader(
            const fastrtps::rtps::GUID_t& guid,
            const rtps::ContentFilterProperty& filter_info,
            DomainParticipantImpl* participant)
    {
        if (0 == filter_info.filter_class_name.size())
        {
            // This reader does not report a filter. Remove the filter in case it had one previously.
            remove_reader(guid);
        }
        else
        {
            auto it = reader_filters_.find(guid);
            if (it == reader_filters_.end())
            {
                if (reader_filters_.size() >= max_filters_)
                {
                    // Maximum number of filters reached.
                    return;
                }

                // Insert element
                ReaderFilterInformation entry;
                if (update_entry(entry, filter_info, participant))
                {
                    reader_filters_.emplace(std::make_pair(guid, std::move(entry)));
                }
            }
            else
            {
                if (!update_entry(it->second, filter_info, participant))
                {
                    destroy_filter(it->second);
                    reader_filters_.erase(it);
                }
            }
        }
    }

private:

    void destroy_filter(
            ReaderFilterInformation& entry)
    {
        if (nullptr != entry.filter_factory && nullptr != entry.filter)
        {
            entry.filter_factory->delete_content_filter(entry.filter_class_name.c_str(), entry.filter);
            entry.filter_factory = nullptr;
            entry.filter = nullptr;
        }
    }

    bool update_entry(
            ReaderFilterInformation& entry,
            const rtps::ContentFilterProperty& filter_info,
            DomainParticipantImpl* participant)
    {
        static_cast<void>(entry);
        static_cast<void>(filter_info);
        static_cast<void>(participant);

        return false;
    }

    using pool_allocator_t =
            foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    pool_allocator_t reader_filter_allocator_;

    foonathan::memory::map<fastrtps::rtps::GUID_t, ReaderFilterInformation, pool_allocator_t> reader_filters_;

    std::size_t max_filters_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_
