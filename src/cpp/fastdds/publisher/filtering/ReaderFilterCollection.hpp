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
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/filtering/DataWriterFilteredChange.hpp>
#include <fastdds/publisher/filtering/ReaderFilterInformation.hpp>
#include <fastdds/topic/TopicProxy.hpp>
#include <fastdds/topic/ContentFilterInfo.hpp>
#include <fastdds/topic/ContentFilterUtils.hpp>

#include <utils/collections/node_size_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class responsible for writer side filtering.
 * Contains a resource-limited map associating a reader GUID with its filtering information.
 * Performs the evaluation of filters when a change is added to the DataWriter's history.
 */
class ReaderFilterCollection
{
    using reader_filter_map_helper =
            utilities::collections::map_size_helper<fastdds::rtps::GUID_t, ReaderFilterInformation>;

public:

    /**
     * Construct a ReaderFilterCollection.
     *
     * @param allocation  Allocation configuration for reader filtering information.
     */
    explicit ReaderFilterCollection(
            const fastdds::ResourceLimitedContainerConfig& allocation)
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

    /**
     * @return true when there are no reader filters registered.
     */
    bool empty() const
    {
        return reader_filters_.empty();
    }

    /**
     * Performs filter evaluation on a DataWriterFilteredChange.
     *
     * @param [in,out] change                   DataWriterFilteredChange being filtered.
     *                                          This method updates two of its properties:
     *                                          - @c filtered_out_readers will contain the GUIDs of the readers to
     *                                            which the change should not be delivered.
     *                                          - @c inline_qos will be updated with a relevant ContentFilterInfo
     *                                            parameter informing about the applied filters.
     * @param [in]     related_sample_identity  SampleIdentity of a related sample, received from on the DataWriter's
     *                                          write call.
     */
    void update_filter_info(
            DataWriterFilteredChange& change,
            const fastdds::rtps::SampleIdentity& related_sample_identity) const
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

                        // Only evaluate filter on ALIVE changes, as UNREGISTERED and DISPOSED are always relevant
                        bool filter_result = true;
                        if (fastdds::rtps::ALIVE == change.kind)
                        {
                            // Evaluate filter and update filtered_out_readers
                            filter_result = entry.filter->evaluate(change.serializedPayload, info, it->first);
                            if (!filter_result)
                            {
                                change.filtered_out_readers.emplace_back(it->first);
                            }
                        }
                        return filter_result;
                    };

            // Perform ContentFilterInfo serialization and filter evaluation
            ContentFilterInfo::cdr_serialize(change.inline_qos, num_filters, filter_process);
        }
    }

    /**
     * Remove filtering information for all readers using certain factory.
     * Called when a custom filter factory is removed.
     *
     * @param [in] filter_class_name  Class name used to create the filters that should be removed.
     */
    void remove_filters(
            const char* filter_class_name)
    {
        auto it = reader_filters_.begin();
        while (it != reader_filters_.end())
        {
            if (0 == strcmp(it->second.filter_class_name.c_str(), filter_class_name))
            {
                it = reader_filters_.erase(it);
                continue;
            }
            ++it;
        }
    }

    /**
     * Unregister a reader from writer-side filtering.
     * Called when the reader is unmatched or when its filtering information is
     * updated to indicate it stopped filtering.
     *
     * @param [in] guid  GUID of the reader to remove.
     */
    void remove_reader(
            const fastdds::rtps::GUID_t& guid)
    {
        auto it = reader_filters_.find(guid);
        if (it != reader_filters_.end())
        {
            destroy_filter(it->second);
            reader_filters_.erase(it);
        }
    }

    /**
     * Process filtering information about a reader.
     * Called whenever the discovery information about a reader changes.
     *
     * @param [in] guid         GUID of the reader for which the discovery information has changed.
     * @param [in] filter_info  Content filter discovery information.
     * @param [in] participant  DomainParticipantImpl of the writer calling this method.
     * @param [in] topic        Topic on which the writer calling this method is writing.
     */
    void process_reader_filter_info(
            const fastdds::rtps::GUID_t& guid,
            const rtps::ContentFilterProperty& filter_info,
            DomainParticipantImpl* participant,
            Topic* topic)
    {
        TopicProxy* writer_topic = static_cast<TopicProxy*>(topic->get_impl());

        if (0 == filter_info.filter_class_name.size() ||
                0 != writer_topic->get_rtps_topic_name().compare(filter_info.related_topic_name.c_str()))
        {
            // This reader does not report an applicable filter. Remove the filter in case it had one previously.
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
                if (update_entry(entry, filter_info, participant, writer_topic->get_type()))
                {
                    reader_filters_.emplace(std::make_pair(guid, std::move(entry)));
                }
            }
            else
            {
                // Update entry
                if (!update_entry(it->second, filter_info, participant, writer_topic->get_type()))
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

    /**
     * Ensure a filter instance is removed before an information entry is removed.
     *
     * @param [in,out] entry  The ReaderFilterInformation entry being removed.
     */
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

    /**
     * Update an information entry.
     *
     * @param [in,out] entry        The ReaderFilterInformation entry to update.
     * @param [in]     filter_info  Content filter discovery information to apply.
     * @param [in]     participant  DomainParticipantImpl where the filter factory should be looked up.
     * @param [in]     type         Type to use for the creation of the content filter.
     */
    bool update_entry(
            ReaderFilterInformation& entry,
            const rtps::ContentFilterProperty& filter_info,
            DomainParticipantImpl* participant,
            const TypeSupport& type)
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
            type.get_type_name().c_str(),
            type.get(),
            filter_info.filter_expression.c_str(),
            filter_parameters,
            new_filter);

        if (RETCODE_OK != ret)
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

    foonathan::memory::map<fastdds::rtps::GUID_t, ReaderFilterInformation, pool_allocator_t> reader_filters_;

    std::size_t max_filters_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_
