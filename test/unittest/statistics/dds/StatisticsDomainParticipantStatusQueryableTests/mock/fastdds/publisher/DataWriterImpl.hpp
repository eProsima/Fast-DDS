// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterImpl.hpp
 */

#ifndef _FASTDDS_DATAWRITERIMPL_HPP_
#define _FASTDDS_DATAWRITERIMPL_HPP_

#include <memory>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/publisher/DataWriterHistory.hpp>
#include <fastdds/publisher/filtering/ReaderFilterCollection.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/history/ITopicPayloadPool.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSWriter;
class RTPSParticipant;
class TimedEvent;

} // namespace rtps

#ifdef FASTDDS_STATISTICS
namespace statistics {
namespace dds {
class DomainParticipantImpl;
} // namespace dds
} // namespace statistics
#endif // FASTDDS_STATISTICS

namespace dds {

class PublisherListener;
class PublisherImpl;
class Publisher;

class DataWriterImpl : protected rtps::IReaderDataFilter
{
    using LoanInitializationKind = DataWriter::LoanInitializationKind;
    using CacheChange_t = eprosima::fastdds::rtps::CacheChange_t;
    class LoanCollection;

protected:

    friend class PublisherImpl;

#ifdef FASTDDS_STATISTICS
    friend class eprosima::fastdds::statistics::dds::DomainParticipantImpl;
#endif // FASTDDS_STATISTICS

    DataWriterImpl()
        : history_()
    {
        gen_guid();
    }

    DataWriterImpl(
            PublisherImpl*,
            TypeSupport,
            Topic*,
            const DataWriterQos&,
            DataWriterListener* listener = nullptr,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr)
        : history_()
    {
        gen_guid();
        static_cast<void>(listener);
        static_cast<void>(payload_pool);
    }

    DataWriterImpl(
            PublisherImpl*,
            TypeSupport,
            Topic*,
            const DataWriterQos&,
            const fastdds::rtps::EntityId_t&,
            DataWriterListener* listener = nullptr)
        : history_()
    {
        gen_guid();
        static_cast<void>(listener);
    }

public:

    virtual ~DataWriterImpl()
    {

    }

    virtual ReturnCode_t enable()
    {
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool;
        std::shared_ptr<fastdds::rtps::IChangePool> change_pool;

        history_.reset(new DataWriterHistory(
                    payload_pool,
                    change_pool,
                    qos_.history(),
                    qos_.resource_limits(),
                    topic_kind_,
                    500,
                    fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                    [](const InstanceHandle_t&)
                    {
                    }));
        return RETCODE_OK;
    }

    ReturnCode_t check_delete_preconditions()
    {
        return RETCODE_OK;
    }

    ReturnCode_t loan_sample(
            void*&,
            LoanInitializationKind )
    {
        return RETCODE_OK;
    }

    ReturnCode_t discard_loan(
            void*& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t write(
            const void* const )
    {
        return RETCODE_OK;
    }

    ReturnCode_t write(
            const void* const,
            fastdds::rtps::WriteParams& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t write(
            const void* const,
            const InstanceHandle_t& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t write_w_timestamp(
            const void* const,
            const InstanceHandle_t&,
            const fastdds::dds::Time_t& )
    {
        return RETCODE_OK;
    }

    InstanceHandle_t register_instance(
            const void* const )
    {
        return InstanceHandle_t();
    }

    InstanceHandle_t register_instance_w_timestamp(
            const void* const,
            const fastdds::dds::Time_t& )
    {
        return InstanceHandle_t();
    }

    ReturnCode_t unregister_instance(
            const void* const,
            const InstanceHandle_t&,
            bool = false)
    {
        return RETCODE_OK;
    }

    ReturnCode_t unregister_instance_w_timestamp(
            const void* const,
            const InstanceHandle_t&,
            const fastdds::dds::Time_t&,
            bool  = false)
    {
        return RETCODE_OK;
    }

    void gen_guid()
    {
        static uint16_t entitiy_id_counter;
        entitiy_id_counter += 1;
        guid_.entityId.value[2] = (fastdds::rtps::octet)entitiy_id_counter;
        guid_.entityId.value[3] = 0x2;
    }

    const fastdds::rtps::GUID_t& guid() const
    {

        return guid_;
    }

    InstanceHandle_t get_instance_handle() const
    {
        return fastdds::rtps::InstanceHandle_t();
    }

    TypeSupport get_type() const
    {
        return TypeSupport();
    }

    ReturnCode_t wait_for_acknowledgments(
            const fastdds::dds::Duration_t& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t wait_for_acknowledgments(
            const void* const,
            const InstanceHandle_t&,
            const fastdds::dds::Duration_t& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_publication_matched_status(
            PublicationMatchedStatus&)
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status)
    {
        status = deadline_missed_status_;
        return RETCODE_OK;
    }

    ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status)
    {
        status = offered_incompatible_qos_status_;
        return RETCODE_OK;
    }

    ReturnCode_t set_qos(
            const DataWriterQos&)
    {
        return RETCODE_OK;
    }

    const DataWriterQos& get_qos() const
    {
        return qos_;
    }

    Topic* get_topic() const
    {
        return topic_;
    }

    const DataWriterListener* get_listener() const
    {
        return nullptr;
    }

    ReturnCode_t set_listener(
            DataWriterListener*)
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_key_value(
            void*,
            const InstanceHandle_t& )
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status)
    {
        status = liveliness_lost_status_;
        return RETCODE_OK;
    }

    const Publisher* get_publisher() const
    {
        return pub_;
    }

    void set_publisher(
            Publisher* pub)
    {
        pub_ = pub;
    }

    ReturnCode_t assert_liveliness()
    {
        return RETCODE_OK;
    }

    virtual void disable()
    {

    }

    ReturnCode_t clear_history(
            size_t*)
    {
        return RETCODE_OK;
    }

    ReturnCode_t get_sending_locators(
            rtps::LocatorList&) const
    {
        return RETCODE_OK;
    }

    void filter_is_being_removed(
            const char*)
    {

    }

    bool is_relevant(
            const fastdds::rtps::CacheChange_t&,
            const fastdds::rtps::GUID_t&) const override
    {
        return true;
    }

    void publisher_qos_updated()
    {

    }

    void insert_policy_violation(
            const fastdds::dds::PolicyMask& policy)
    {
        if (policy[LIVELINESS_QOS_POLICY_ID])
        {
            liveliness_lost_status_.total_count++;
        }
        else if (policy[DEADLINE_QOS_POLICY_ID])
        {
            deadline_missed_status_.total_count++;
            deadline_missed_status_.total_count_change++;
        }
        else if (policy[RELIABILITY_QOS_POLICY_ID])
        {
            ++offered_incompatible_qos_status_.total_count;
            ++offered_incompatible_qos_status_.total_count_change;

            for (uint32_t id = 1; id < ::eprosima::fastdds::dds::NEXT_QOS_POLICY_ID; ++id)
            {
                if (policy.test(id))
                {
                    ++offered_incompatible_qos_status_.policies[static_cast<fastdds::dds::QosPolicyId_t>(id)].count;
                    offered_incompatible_qos_status_.last_policy_id = static_cast<fastdds::dds::QosPolicyId_t>(id);
                }
            }
        }
    }

    inline ReturnCode_t get_publication_builtin_topic_data(
            PublicationBuiltinTopicData& publication_data) const
    {
        publication_data = PublicationBuiltinTopicData{};
        return RETCODE_OK;
    }

    static ReturnCode_t check_qos(
            const ::eprosima::fastdds::dds::DataWriterQos&)
    {
        return RETCODE_OK;
    }

    static ReturnCode_t check_qos_including_resource_limits(
            const DataWriterQos&,
            const TypeSupport& )
    {
        return RETCODE_OK;
    }

    static void set_qos(
            DataWriterQos&,
            const DataWriterQos&,
            bool )
    {

    }

    ReturnCode_t get_matched_subscription_data(
            SubscriptionBuiltinTopicData&,
            const InstanceHandle_t& ) const
    {
        return RETCODE_ERROR;
    }

    ReturnCode_t get_matched_subscriptions(
            std::vector<InstanceHandle_t>&) const
    {
        return RETCODE_ERROR;
    }

    ReturnCode_t get_matched_subscriptions(
            std::vector<InstanceHandle_t*>&) const
    {
        return RETCODE_ERROR;
    }

    //! Pointer to the associated Data Writer.
    fastdds::rtps::RTPSWriter* writer_ = nullptr;
    Topic* topic_ = nullptr;
    Publisher* pub_ = nullptr;
    fastdds::rtps::GUID_t guid_;
    DataWriterQos qos_;

    DataWriter* user_datawriter_ = nullptr;

    OfferedDeadlineMissedStatus deadline_missed_status_;
    LivelinessLostStatus liveliness_lost_status_;
    OfferedIncompatibleQosStatus offered_incompatible_qos_status_;
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;
    std::unique_ptr<DataWriterHistory> history_;

    rtps::TopicKind_t topic_kind_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTDDS_DATAWRITERIMPL_HPP_
