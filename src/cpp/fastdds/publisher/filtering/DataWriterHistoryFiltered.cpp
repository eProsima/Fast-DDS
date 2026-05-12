/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file DataWriterHistoryFiltered.cpp
 */

#include <fastdds/publisher/filtering/DataWriterHistoryFiltered.hpp>

#include <cstdint>
#include <limits>

#include <fastdds/publisher/filtering/DataWriterFilteredChange.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastdds::rtps;

DataWriterHistoryFiltered::DataWriterHistoryFiltered(
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        const HistoryQosPolicy& history_qos,
        const ResourceLimitsQosPolicy& resource_limits_qos,
        const rtps::TopicKind_t& topic_kind,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy,
        std::function<void (const fastdds::rtps::InstanceHandle_t&)> unack_sample_remove_functor,
        uint16_t filters_max_overhead,
        bool reserve_filters_max_overhead)
    : DataWriterHistory(
        payload_pool,
        change_pool,
        history_qos,
        resource_limits_qos,
        topic_kind,
        payloadMaxSize,
        mempolicy,
        unack_sample_remove_functor)
    , filters_max_overhead_(filters_max_overhead)
    , reserve_filters_max_overhead_(reserve_filters_max_overhead)
{
}

DataWriterHistoryFiltered::~DataWriterHistoryFiltered()
{
}

bool DataWriterHistoryFiltered::get_inline_qos_overhead(
        const CacheChange_t* change,
        uint32_t& inline_qos_overhead) const
{
    assert(change != nullptr);

    if (!DataWriterHistory::get_inline_qos_overhead(change, inline_qos_overhead))
    {
        return false;
    }

    if (reserve_filters_max_overhead_)
    {
        const DataWriterFilteredChange* filtered_change = static_cast<const DataWriterFilteredChange*>(change);
        assert(filtered_change != nullptr);

        if (filtered_change->filters_cdr_size > filters_max_overhead_)
        {
            // This should never happen, as both values are calculated using ContentFilterInfo::cdr_serialized_size
            // and the number of filters should never surpass the configured maximum in WriterResourceLimitsQos
            assert(false);
            return false;
        }

        uint16_t extra_overhead = filters_max_overhead_ - filtered_change->filters_cdr_size;
        if (inline_qos_overhead > std::numeric_limits<uint32_t>::max() - extra_overhead)
        {
            // Abort due to overflow in overhead calculation
            return false;
        }

        // Add extra overhead to base calculation
        inline_qos_overhead += extra_overhead;
    }

    return true;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
