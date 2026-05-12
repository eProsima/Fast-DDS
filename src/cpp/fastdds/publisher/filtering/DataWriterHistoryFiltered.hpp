/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file DataWriterHistoryFiltered.hpp
 */

#ifndef FASTDDS_PUBLISHER_FILTERING__DATAWRITERHISTORYFILTERED_HPP
#define FASTDDS_PUBLISHER_FILTERING__DATAWRITERHISTORYFILTERED_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <fastdds/publisher/DataWriterHistory.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataWriterHistoryFiltered, extending DataWriterHistory to support writer-side filtering.
 * This class is created by the PublisherImpl and should not be used by the user directly.
 * @ingroup FASTDDS_MODULE
 */
class DataWriterHistoryFiltered : public DataWriterHistory
{

public:

    /**
     * Constructor of the DataWriterHistoryFiltered.
     *
     * @param payload_pool                 Pool to use for allocation of payloads.
     * @param change_pool                  Pool to use for allocation of changes.
     * @param history_qos                  HistoryQosPolicy of the DataWriter creating this history.
     * @param resource_limits_qos          ResourceLimitsQosPolicy of the DataWriter creating this history.
     * @param topic_kind                   TopicKind of the DataWriter creating this history.
     * @param payloadMax                   Maximum payload size.
     * @param mempolicy                    Set whether the payloads can dynamically resized or not.
     * @param unack_sample_remove_functor  Functor to call DDS listener callback on_unacknowledged_sample_removed
     * @param filters_max_overhead         Maximum overhead in bytes that filtering information can add to a change.
     * @param reserve_filters_max_overhead Whether the maximum filtering overhead should be reserved for a change.
     */
    DataWriterHistoryFiltered(
            const std::shared_ptr<rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<rtps::IChangePool>& change_pool,
            const HistoryQosPolicy& history_qos,
            const ResourceLimitsQosPolicy& resource_limits_qos,
            const rtps::TopicKind_t& topic_kind,
            uint32_t payloadMax,
            rtps::MemoryManagementPolicy_t mempolicy,
            std::function<void (const rtps::InstanceHandle_t&)> unack_sample_remove_functor,
            uint16_t filters_max_overhead,
            bool reserve_filters_max_overhead);

    virtual ~DataWriterHistoryFiltered();

protected:

    virtual bool get_inline_qos_overhead(
            const rtps::CacheChange_t* change,
            uint32_t& inline_qos_overhead) const override;

    //! Maximum overhead in bytes that filtering information can add to a change
    uint16_t filters_max_overhead_;

    //! Whether the maximum filtering overhead should be reserved for a change
    bool reserve_filters_max_overhead_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_PUBLISHER_FILTERING__DATAWRITERHISTORYFILTERED_HPP
