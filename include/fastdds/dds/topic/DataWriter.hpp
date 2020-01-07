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

#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/core/Entity.hpp>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/status/Status.hpp>
#include <dds/core/status/State.hpp>
#include <fastdds/dds/topic/SubscriptionBuiltinTopicData.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace pub {
template< typename T>
class DataWriter;
} // pub
} // ::dds

namespace eprosima {
namespace fastrtps {

//class TopicAttributes;

namespace rtps {

class WriteParams;
class WriterAttributes;
struct InstanceHandle_t;
struct GUID_t;

} // namespace rtps

} // namespace fastrtps

namespace fastdds {
namespace dds {

class PublisherListener;
class PublisherImpl;
class Publisher;

class TypeSupport;
class Topic;

class DataWriterImpl;
class DataWriterListener;
class DataWriterQos;

/**
 * Class DataWriter, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI DataWriter : public DomainEntity
{
    friend class PublisherImpl;
    friend class DataWriterImpl;
    template<typename T>
    friend class ::dds::pub::DataWriter;

    DataWriter(
            const Publisher* pub,
            const Topic& topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    /**
     * Create a data writer, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    DataWriter(
            DataWriterImpl* impl,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

public:

    virtual ~DataWriter();

    /**
     * Write data to the topic.
     * @param data Pointer to the data
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    ReturnCode_t write(
            void* data);

    /**
     * Write data with params to the topic.
     * @param data Pointer to the data
     * @param params Extra write parameters.
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    fastrtps::types::ReturnCode_t write(
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
     * Returns the DataWriter's GUID
     */
    const fastrtps::rtps::GUID_t& guid();

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport get_type() const;

    /**
     * Waits the current thread until all writers have received their acknowledgments.
     */
    ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param status Deadline missed status struct
     */
    ReturnCode_t get_offered_deadline_missed_status(
            fastrtps::OfferedDeadlineMissedStatus& status);

    bool set_attributes(
            const fastrtps::rtps::WriterAttributes& att);

    const fastrtps::rtps::WriterAttributes& get_attributes() const;

    /**
     * Establishes the DataWriterQos for this DataWriter.
     */
    ReturnCode_t set_qos(
            const DataWriterQos& qos);

    /**
     * Retrieves the DataWriterQos for this DataWriter.
     */
    const DataWriterQos& get_qos() const;

    /**
     * Fills the DataWriterQos with the values of this DataWriter.
     * @return true
     */
    ReturnCode_t get_qos(
            DataWriterQos& qos) const;

    /**
     * Establishes the topic for this DataWriter.
     */
    bool set_topic(
            const Topic& topic);

    /**
     * Retrieves the topic for this DataWriter.
     */
    const Topic& get_topic() const;

    /**
     * Retrieves the listener for this DataWriter.
     */
    const DataWriterListener* get_listener() const;

    /**
     * Establishes the listener for this DataWriter.
     */
    ReturnCode_t set_listener(
            DataWriterListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    /* TODO
       bool get_key_value(
            void* key_holder,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t dispose(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    ReturnCode_t dispose(
            void* data);

    ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status);

    ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status);

    ReturnCode_t get_publication_matched_status(
            PublicationMatchedStatus& status);

    const Publisher* get_publisher() const;

    ReturnCode_t assert_liveliness();

    ReturnCode_t get_matched_subscriptions(
            std::vector<fastrtps::rtps::InstanceHandle_t>& subscription_handles) const;

    ReturnCode_t get_matched_subscription_data(
            SubscriptionBuiltinTopicData& subscription_data,
            const fastrtps::rtps::InstanceHandle_t& subscription_handle) const;

    ReturnCode_t enable();

private:

    DataWriterImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITER_HPP_
