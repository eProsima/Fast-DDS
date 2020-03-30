// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/WriteParams.h>

#include <fastdds/dds/topic/qos/DataWriterQos.hpp>

#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/dds/topic/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/publisher/PublisherHistory.h>
#include <fastrtps/attributes/TopicAttributes.h>

#include <fastdds/rtps/writer/WriterListener.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastrtps/qos/LivelinessLostStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;
class RTPSParticipant;
class TimedEvent;

} // namespace rtps

} // namespace fastrtps

namespace fastdds {
namespace dds {

class PublisherListener;
class PublisherImpl;
class Publisher;

/**
 * Class DataWriterImpl, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTDDS_MODULE
 */
class DataWriterImpl
{
    friend class PublisherImpl;

    /**
     * Create a data writer, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    DataWriterImpl(
            PublisherImpl* p,
            TypeSupport type,
            const fastrtps::TopicAttributes& topic_att,
            const fastrtps::rtps::WriterAttributes& att,
            const DataWriterQos& qos,
            const fastrtps::rtps::MemoryManagementPolicy_t memory_policy,
            DataWriterListener* listener = nullptr);

public:

    virtual ~DataWriterImpl();

    /**
     * Write data to the topic.
     * @param data Pointer to the data
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    bool write(
            void* data);

    /**
     * Write data with params to the topic.
     * @param data Pointer to the data
     * @param params Extra write parameters.
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    bool write(
            void* data,
            fastrtps::rtps::WriteParams& params);

    /**
     * Write data with handle.
     * @param data Pointer to the data
     * @param handle InstanceHandle_t.
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    ReturnCode_t write(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /**
     *
     * @return
     */
    const fastrtps::rtps::GUID_t& guid();

    fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport get_type() const
    {
        return type_;
    }

    ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param Deadline missed status struct
     */
    ReturnCode_t get_offered_deadline_missed_status(
            fastrtps::OfferedDeadlineMissedStatus& status);

    bool set_attributes(
            const fastrtps::rtps::WriterAttributes& att);

    const fastrtps::rtps::WriterAttributes& get_attributes() const;

    ReturnCode_t set_qos(
            const DataWriterQos& qos);

    const DataWriterQos& get_qos() const;

    bool set_topic(
            const fastrtps::TopicAttributes& att);

    const fastrtps::TopicAttributes& get_topic() const;

    const DataWriterListener* get_listener() const;

    ReturnCode_t set_listener(
            DataWriterListener* listener);

    /* TODO
       bool get_key_value(
            void* key_holder,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t dispose(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool dispose(
            void* data);

    ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status);

    /* TODO
       ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status)
       {
        // Not implemented
        (void)status;
        return false;
       }
     */

    const Publisher* get_publisher() const;

    ReturnCode_t assert_liveliness();

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

private:

    PublisherImpl* publisher_;

    //! Pointer to the associated Data Writer.
    fastrtps::rtps::RTPSWriter* writer_;

    //! Pointer to the TopicDataType object.
    TypeSupport type_;

    fastrtps::TopicAttributes topic_att_;

    fastrtps::rtps::WriterAttributes w_att_;

    DataWriterQos qos_;

    //!Publisher History
    fastrtps::PublisherHistory history_;

    //! DataWriterListener
    DataWriterListener* listener_;

    //!Listener to capture the events of the Writer
    class InnerDataWriterListener : public fastrtps::rtps::WriterListener
    {
public:

        InnerDataWriterListener(
                DataWriterImpl* w)
            : data_writer_(w)
        {
        }

        virtual ~InnerDataWriterListener() override
        {
        }

        void onWriterMatched(
                fastrtps::rtps::RTPSWriter* writer,
                const fastdds::dds::PublicationMatchedStatus& info) override;

        void onWriterChangeReceivedByAll(
                fastrtps::rtps::RTPSWriter* writer,
                fastrtps::rtps::CacheChange_t* change) override;

        void on_liveliness_lost(
                fastrtps::rtps::RTPSWriter* writer,
                const fastrtps::LivelinessLostStatus& status) override;

        DataWriterImpl* data_writer_;
    } writer_listener_;

    uint32_t high_mark_for_frag_;

    //! A timer used to check for deadlines
    fastrtps::rtps::TimedEvent* deadline_timer_;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000> > deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    fastrtps::rtps::InstanceHandle_t timer_owner_;

    //! The offered deadline missed status
    fastrtps::OfferedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples for lifespan QoS
    fastrtps::rtps::TimedEvent* lifespan_timer_;

    //! The lifespan duration, in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000> > lifespan_duration_us_;

    DataWriter* user_datawriter_;

    /**
     *
     * @param kind
     * @param  data
     * @return
     */
    bool create_new_change(
            fastrtps::rtps::ChangeKind_t kind,
            void* data);

    /**
     *
     * @param kind
     * @param  data
     * @param wparams
     * @return
     */
    bool create_new_change_with_params(
            fastrtps::rtps::ChangeKind_t kind,
            void* data,
            fastrtps::rtps::WriteParams& wparams);

    /**
     *
     * @param kind
     * @param  data
     * @param wparams
     * @param handle
     * @return
     */
    bool create_new_change_with_params(
            fastrtps::rtps::ChangeKind_t kind,
            void* data,
            fastrtps::rtps::WriteParams& wparams,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /**
     * Removes the cache change with the minimum sequence number
     * @return True if correct.
     */
    bool remove_min_seq_change();

    /**
     * Removes all changes from the History.
     * @param[out] removed Number of removed elements
     * @return True if correct.
     */
    bool remove_all_change(
            size_t* removed);

    /**
     * @brief A method called when an instance misses the deadline
     */
    bool deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    bool deadline_timer_reschedule();

    /**
     * @brief A method to remove expired samples, invoked when the lifespan timer expires
     */
    bool lifespan_expired();

    bool check_new_change_preconditions(
            fastrtps::rtps::ChangeKind_t change_kind,
            void* data);

    bool perform_create_new_change(
            fastrtps::rtps::ChangeKind_t change_kind,
            void* data,
            fastrtps::rtps::WriteParams& wparams,
            const fastrtps::rtps::InstanceHandle_t& handle);
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITERIMPL_HPP_
