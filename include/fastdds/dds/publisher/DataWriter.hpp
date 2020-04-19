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
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

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

class DataWriterImpl;
class DataWriterListener;
class DataWriterQos;
class Topic;

/**
 * Class DataWriter, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTDDS_MODULE
 */
class DataWriter : public DomainEntity
{
    friend class PublisherImpl;
    friend class DataWriterImpl;

    /**
     * Create a data writer, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    RTPS_DllAPI DataWriter(
            DataWriterImpl* impl,
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI DataWriter(
            Publisher* pub,
            Topic* topic,
            const DataWriterQos& qos = DATAWRITER_QOS_DEFAULT,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    RTPS_DllAPI virtual ~DataWriter();

    /**
     * Write data to the topic.
     * @param data Pointer to the data
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    RTPS_DllAPI bool write(
            void* data);

    /**
     * Write data with params to the topic.
     * @param data Pointer to the data
     * @param params Extra write parameters.
     * @return True if correct
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    RTPS_DllAPI bool write(
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
    RTPS_DllAPI ReturnCode_t write(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /**
     * Returns the DataWriter's GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid();

    /**
     * Returns the DataWriter's InstanceHandle
     */
    RTPS_DllAPI fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    RTPS_DllAPI TypeSupport get_type() const;

    /**
     * Waits the current thread until all writers have received their acknowledgments.
     */
    RTPS_DllAPI ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param status Deadline missed status struct
     */
    RTPS_DllAPI ReturnCode_t get_offered_deadline_missed_status(
            fastrtps::OfferedDeadlineMissedStatus& status);

    /**
     * Establishes the DataWriterQos for this DataWriter.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DataWriterQos& qos);

    /**
     * Retrieves the DataWriterQos for this DataWriter.
     */
    RTPS_DllAPI const DataWriterQos& get_qos() const;

    /**
     * Fills the DataWriterQos with the values of this DataWriter.
     * @return true
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            DataWriterQos& qos) const;

    /**
     * Retrieves the topic for this DataWriter.
     */
    RTPS_DllAPI Topic* get_topic() const;

    /**
     * Retrieves the listener for this DataWriter.
     */
    RTPS_DllAPI const DataWriterListener* get_listener() const;

    /**
     * Establishes the listener for this DataWriter.
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DataWriterListener* listener);

    /* TODO
       bool get_key_value(
            void* key_holder,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    RTPS_DllAPI ReturnCode_t dispose(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    RTPS_DllAPI bool dispose(
            void* data);

    RTPS_DllAPI ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status);

    /* TODO
       bool get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status)
       {
        // Not implemented
        (void)status;
        return false;
       }
     */

    RTPS_DllAPI const Publisher* get_publisher() const;

    RTPS_DllAPI ReturnCode_t assert_liveliness();

private:

    DataWriterImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITER_HPP_
