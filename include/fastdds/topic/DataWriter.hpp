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
 * @file DataWriter.hpp
 */

#ifndef _FASTRTPS_DATAWRITER_HPP_
#define _FASTRTPS_DATAWRITER_HPP_

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/common/WriteParams.h>

#include <fastrtps/qos/WriterQos.h>

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastdds/topic/DataWriterListener.hpp>
#include <fastrtps/publisher/PublisherHistory.h>
#include <fastrtps/topic/attributes/TopicAttributes.h>

#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/IncompatibleQosStatus.hpp>
#include <fastrtps/qos/LivelinessLostStatus.h>
#include "../../fastrtps/types/TypesBase.h"

using namespace eprosima::fastrtps::types;

namespace eprosima {
namespace fastrtps{
namespace rtps
{
class RTPSWriter;
class RTPSParticipant;
}

class TopicDataType;

} // namespace fastrtps

namespace fastdds {

class PublisherListener;
class PublisherImpl;
class Publisher;

/**
 * Class DataWriter, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTRTPS_MODULE
 */
class DataWriter
{
    friend class PublisherImpl;

    /**
     * Create a data writer, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    DataWriter(
            PublisherImpl* p,
            fastrtps::TopicDataType* topic,
            const fastrtps::TopicAttributes& topic_att,
            const fastrtps::rtps::WriterAttributes& att,
            fastrtps::WriterQos& qos,
            const fastrtps::rtps::MemoryManagementPolicy_t memory_policy,
            DataWriterListener* listener = nullptr);

public:

    virtual ~DataWriter();

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
     *
     * @return
     */
    const fastrtps::rtps::GUID_t& guid();

    fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    const fastrtps::TopicDataType* get_type() const
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

    bool set_attributes(const fastrtps::rtps::WriterAttributes& att);

    const fastrtps::rtps::WriterAttributes& get_attributes() const;

    ReturnCode_t set_qos(const fastrtps::WriterQos& qos);

    ReturnCode_t get_qos(fastrtps::WriterQos& qos) const;

    bool set_topic(const fastrtps::TopicAttributes& att);

    const fastrtps::TopicAttributes& get_topic() const;

    const DataWriterListener* get_listener() const;

    ReturnCode_t set_listener(DataWriterListener* listener);

    ReturnCode_t get_key_value(
            void* key_holder,
            const fastrtps::rtps::InstanceHandle_t& handle);

    ReturnCode_t dispose(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool dispose(
            void* data);

    ReturnCode_t get_liveliness_lost_status(
            fastrtps::LivelinessLostStatus& status);

    ReturnCode_t get_offered_incompatible_qos_status(
            fastrtps::OfferedIncompatibleQosStatus& status)
    {
        // Not implemented
        (void)status;
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }

    const Publisher* get_publisher() const;

    ReturnCode_t assert_liveliness();

private:
    PublisherImpl* publisher_;

    //! Pointer to the associated Data Writer.
    fastrtps::rtps::RTPSWriter* writer_;

    //! Pointer to the TopicDataType object.
    fastrtps::TopicDataType* type_;

    fastrtps::TopicAttributes topic_att_;

    fastrtps::rtps::WriterAttributes w_att_;

    fastrtps::WriterQos qos_;

    //!Publisher History
    fastrtps::PublisherHistory history_;

    //! DataWriterListener
    DataWriterListener* listener_;

    //!Listener to capture the events of the Writer
    class InnerDataWriterListener : public fastrtps::rtps::WriterListener
    {
        public:
            InnerDataWriterListener(
                    DataWriter* w)
                : data_writer_(w)
            {
            }

            virtual ~InnerDataWriterListener() override {}

            void on_writer_matched(
                    fastrtps::rtps::RTPSWriter* writer,
                    fastrtps::rtps::MatchingInfo& info) override;

            void on_writer_change_received_by_all(
                    fastrtps::rtps::RTPSWriter* writer,
                    fastrtps::rtps::CacheChange_t* change) override;

            void on_liveliness_lost(
                    fastrtps::rtps::RTPSWriter* writer,
                    const fastrtps::LivelinessLostStatus& status) override;

            DataWriter* data_writer_;
    } writer_listener_;

    uint32_t high_mark_for_frag_;

    //! A timer used to check for deadlines
    fastrtps::rtps::TimedCallback deadline_timer_;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1,1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    fastrtps::rtps::InstanceHandle_t timer_owner_;

    //! The offered deadline missed status
    fastrtps::OfferedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples for lifespan QoS
    fastrtps::rtps::TimedCallback lifespan_timer_;

    //! The lifespan duration, in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    /**
     * @brief A method called when an instance misses the deadline
     */
    void deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    void deadline_timer_reschedule();

    /**
     * @brief A method to remove expired samples, invoked when the lifespan timer expires
     */
    void lifespan_expired();

    bool check_new_change_preconditions(
            fastrtps::rtps::ChangeKind_t change_kind,
            void* data);

    bool perform_create_new_change(
            fastrtps::rtps::ChangeKind_t change_kind,
            void* data,
            fastrtps::rtps::WriteParams& wparams,
            const fastrtps::rtps::InstanceHandle_t& handle);
};

} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITER_HPP_
