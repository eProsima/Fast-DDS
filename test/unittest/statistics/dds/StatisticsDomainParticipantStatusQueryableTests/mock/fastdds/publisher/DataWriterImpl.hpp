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

#ifndef _FASTRTPS_DATAWRITERIMPL_HPP_
#define _FASTRTPS_DATAWRITERIMPL_HPP_

#include <memory>

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/WriteParams.h>
#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/WriterListener.h>

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/LivelinessLostStatus.h>

#include <fastrtps/types/TypesBase.h>

#include <fastdds/publisher/DataWriterHistory.hpp>
#include <fastdds/publisher/filtering/ReaderFilterCollection.hpp>

#include <rtps/common/PayloadInfo_t.hpp>
#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>


namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;
class RTPSParticipant;
class TimedEvent;

} // namespace rtps

} // namespace fastrtps

namespace fastdds {

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
    using PayloadInfo_t = eprosima::fastrtps::rtps::detail::PayloadInfo_t;
    using CacheChange_t = eprosima::fastrtps::rtps::CacheChange_t;
    class LoanCollection;

protected:

    friend class PublisherImpl;

#ifdef FASTDDS_STATISTICS
    friend class eprosima::fastdds::statistics::dds::DomainParticipantImpl;
#endif // FASTDDS_STATISTICS

    DataWriterImpl()
        : history_(atts_,
                500,
                fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                [](const InstanceHandle_t&)
                {
                })
    {
        gen_guid();
    }

    DataWriterImpl(
            PublisherImpl*,
            TypeSupport,
            Topic*,
            const DataWriterQos&,
            DataWriterListener* listener = nullptr,
            std::shared_ptr<fastrtps::rtps::IPayloadPool> payload_pool = nullptr)
        : history_(atts_,
                500,
                fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                [](const InstanceHandle_t&)
                {
                })
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
            const fastrtps::rtps::EntityId_t&,
            DataWriterListener* listener = nullptr)
        : history_(atts_,
                500,
                fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                [](const InstanceHandle_t&)
                {
                })
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
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t check_delete_preconditions()
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t loan_sample(
            void*&,
            LoanInitializationKind )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t discard_loan(
            void*& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    bool write(
            void* )
    {
        return true;
    }

    bool write(
            void*,
            fastrtps::rtps::WriteParams& )
    {
        return true;
    }

    ReturnCode_t write(
            void*,
            const InstanceHandle_t& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t write_w_timestamp(
            void*,
            const InstanceHandle_t&,
            const fastrtps::Time_t& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    InstanceHandle_t register_instance(
            void* )
    {
        return InstanceHandle_t();
    }

    InstanceHandle_t register_instance_w_timestamp(
            void*,
            const fastrtps::Time_t& )
    {
        return InstanceHandle_t();
    }

    ReturnCode_t unregister_instance(
            void*,
            const InstanceHandle_t&,
            bool = false)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t unregister_instance_w_timestamp(
            void*,
            const InstanceHandle_t&,
            const fastrtps::Time_t&,
            bool  = false)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    void gen_guid()
    {
        static uint16_t entitiy_id_counter;
        entitiy_id_counter += 1;
        guid_.entityId.value[2] = (fastrtps::rtps::octet)entitiy_id_counter;
        guid_.entityId.value[3] = 0x2;
    }

    const fastrtps::rtps::GUID_t& guid() const
    {

        return guid_;
    }

    InstanceHandle_t get_instance_handle() const
    {
        return fastrtps::rtps::InstanceHandle_t();
    }

    TypeSupport get_type() const
    {
        return TypeSupport();
    }

    ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t wait_for_acknowledgments(
            void*,
            const InstanceHandle_t&,
            const fastrtps::Duration_t& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_publication_matched_status(
            PublicationMatchedStatus&)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_offered_deadline_missed_status(
            fastrtps::OfferedDeadlineMissedStatus& status)
    {
        status = deadline_missed_status_;
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status)
    {
        status = offered_incompatible_qos_status_;
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t set_qos(
            const DataWriterQos&)
    {
        return ReturnCode_t::RETCODE_OK;
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
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_key_value(
            void*,
            const InstanceHandle_t& )
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status)
    {
        status = liveliness_lost_status_;
        return ReturnCode_t::RETCODE_OK;
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
        return ReturnCode_t::RETCODE_OK;
    }

    virtual void disable()
    {

    }

    ReturnCode_t clear_history(
            size_t*)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_sending_locators(
            rtps::LocatorList&) const
    {
        return ReturnCode_t::RETCODE_OK;
    }

    void filter_is_being_removed(
            const char*)
    {

    }

    bool is_relevant(
            const fastrtps::rtps::CacheChange_t&,
            const fastrtps::rtps::GUID_t&) const override
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

    static ReturnCode_t check_qos(
            const ::eprosima::fastdds::dds::DataWriterQos&)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    static ReturnCode_t check_qos_including_resource_limits(
            const DataWriterQos&,
            const TypeSupport& )
    {
        return fastrtps::types::ReturnCode_t::RETCODE_OK;
    }

    static void set_qos(
            DataWriterQos&,
            const DataWriterQos&,
            bool )
    {

    }

    //! Pointer to the associated Data Writer.
    fastrtps::rtps::RTPSWriter* writer_ = nullptr;
    Topic* topic_ = nullptr;
    Publisher* pub_ = nullptr;
    fastrtps::rtps::GUID_t guid_;
    DataWriterQos qos_;

    DataWriter* user_datawriter_ = nullptr;

    OfferedDeadlineMissedStatus deadline_missed_status_;
    LivelinessLostStatus liveliness_lost_status_;
    OfferedIncompatibleQosStatus offered_incompatible_qos_status_;
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;
    DataWriterHistory history_;
    fastrtps::TopicAttributes atts_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITERIMPL_HPP_
