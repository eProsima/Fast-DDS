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

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/Guid.h>

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/filtering/DataWriterCacheChange.hpp>
#include <fastdds/publisher/filtering/ReaderFilterInformation.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/topic/ContentFilterInfo.hpp>
#include <fastdds/topic/ContentFilterUtils.hpp>

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

    bool empty() const
    {
        return reader_filters_.empty();
    }

    void update_filter_info(
            DataWriterCacheChange& change,
            const fastrtps::rtps::SampleIdentity& related_sample_identity) const
    {
        change.filtered_out_readers.clear();

        size_t num_filters = std::min(change.filtered_out_readers.max_size(), reader_filters_.size());
        uint16_t cdr_size = 0;
        if ((0 < num_filters) && ContentFilterInfo::cdr_serialized_size(num_filters, cdr_size))
        {
            // Prepare inline_qos to hold the ContentFilterInfo parameter.
            change.inline_qos.reserve(change.inline_qos.length + cdr_size);

            // Prepare the filter info to be used on the evaluation of filters
            IContentFilter::FilterSampleInfo info;
            info.related_sample_identity = related_sample_identity;
            info.sample_identity.writer_guid(change.writerGUID);
            info.sample_identity.sequence_number(change.sequenceNumber);

            // Functor used from the serialization process to evaluate each filter and write its signature.
            auto filter_process = [this, &change, &info](
                std::size_t i,
                uint8_t* signature) -> bool
                    {
                        // Point to the corresponding entry
                        auto it = reader_filters_.cbegin();
                        std::advance(it, i);
                        const ReaderFilterInformation& entry = it->second;

                        // Copy the signature
                        std::copy(entry.filter_signature.begin(), entry.filter_signature.end(), signature);

                        // Evaluate filter and update filtered_out_readers
                        bool filter_result = entry.filter->evaluate(change.serializedPayload, info, it->first);
                        if (!filter_result)
                        {
                            change.filtered_out_readers.emplace_back(it->first);
                        }

                        return filter_result;
                    };

            // Perform ContentFilterInfo serialization and filter evaluation
            ContentFilterInfo::cdr_serialize(change.inline_qos, num_filters, filter_process);
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
            DomainParticipantImpl* participant,
            Topic* topic)
    {
        TopicImpl* writer_topic = static_cast<TopicImpl*>(topic->get_impl());

        if (0 == filter_info.filter_class_name.size() ||
                0 != writer_topic->get_rtps_topic_name().compare(filter_info.related_topic_name.c_str()))
        {
            // This reader does not report an aplicable filter. Remove the filter in case it had one previously.
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

                // Prepare and insert element
                ReaderFilterInformation entry;
                if (update_entry(entry, filter_info, participant, writer_topic))
                {
                    reader_filters_.emplace(std::make_pair(guid, std::move(entry)));
                }
            }
            else
            {
                // Update entry
                if (!update_entry(it->second, filter_info, participant, writer_topic))
                {
                    // If the entry could not be updated, it means we cannot use the filter information, so
                    // we remove the old information
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
            DomainParticipantImpl* participant,
            TopicImpl* topic)
    {
        const char* class_name = filter_info.filter_class_name.c_str();
        IContentFilterFactory* new_factory = participant->find_content_filter_factory(class_name);
        if (nullptr == new_factory)
        {
            return false;
        }

        std::array<uint8_t, 16> new_signature;
        ContentFilterUtils::compute_signature(filter_info, new_signature);
        if (new_signature == entry.filter_signature &&
                new_factory == entry.filter_factory &&
                nullptr != entry.filter)
        {
            return true;
        }

        LoanableSequence<const char*>::size_type n_params;
        n_params = static_cast<LoanableSequence<const char*>::size_type>(filter_info.expression_parameters.size());
        LoanableSequence<const char*> filter_parameters(n_params);
        filter_parameters.length(n_params);
        while (n_params > 0)
        {
            n_params--;
            filter_parameters[n_params] = filter_info.expression_parameters[n_params].c_str();
        }

        IContentFilter* new_filter = entry.filter_factory == new_factory ? entry.filter : nullptr;
        ReturnCode_t ret = new_factory->create_content_filter(
            class_name,
            topic->get_type().get_type_name().c_str(),
            topic->get_type().get(),
            filter_info.filter_expression.c_str(), filter_parameters, new_filter);

        if (ReturnCode_t::RETCODE_OK != ret)
        {
            return false;
        }

        if ((new_factory != entry.filter_factory) && (nullptr != entry.filter_factory))
        {
            entry.filter_factory->delete_content_filter(entry.filter_class_name.c_str(), entry.filter);
        }
        entry.filter_class_name = filter_info.filter_class_name;
        entry.filter_signature = new_signature;
        entry.filter_factory = new_factory;
        entry.filter = new_filter;

        return true;
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
