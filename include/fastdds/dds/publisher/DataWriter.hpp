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
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
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
protected:

    friend class PublisherImpl;
    friend class DataWriterImpl;

    /**
     * Create a data writer, assigning its pointer to the associated implementation.
     * Don't use directly, create DataWriter using create_datawriter from Publisher.
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
     * @brief This operation enables the DataWriter
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the Publisher creating this
     *         DataWriter is not enabled.
     */
    RTPS_DllAPI ReturnCode_t enable() override;

    /**
     * Write data to the topic.
     * @param data Pointer to the data
     * @return True if correct, false otherwise
     */
    RTPS_DllAPI bool write(
            void* data);

    /**
     * Write data with params to the topic.
     * @param data Pointer to the data
     * @param params Extra write parameters.
     * @return True if correct, false otherwise
     */
    RTPS_DllAPI bool write(
            void* data,
            fastrtps::rtps::WriteParams& params);

    /**
     * Write data with handle.
     *
     * The special value HANDLE_NIL can be used for the parameter handle.This indicates that the identity of the
     * instance should be automatically deduced from the instance_data (by means of the key).
     *
     * @param data Pointer to the data
     * @param handle InstanceHandle_t.
     * @return RETCODE_PRECONDITION_NOT_MET if the handle introduced does not match with the one associated to the data,
     * RETCODE_OK if the data is correctly sent and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t write(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /*!
     * @brief Informs that the application will be modifying a particular instance.
     * It gives an opportunity to the middleware to pre-configure itself to improve performance.
     * @param[in] instance Sample used to get the instance's key.
     * @return Handle containing the instance's key.
     * This handle could be used in successive `write` or `dispose` operations.
     * In case of error, HANDLE_NIL will be returned.
     */
    RTPS_DllAPI fastrtps::rtps::InstanceHandle_t register_instance(
            void* instance);

    /*!
     * @brief This operation reverses the action of `register_instance`.
     * It should only be called on an instance that is currently registered.
     * Informs the middleware that the DataWriter is not intending to modify any more of that data instance.
     * Also indicates that the middleware can locally remove all information regarding that instance.
     * @param[in] instance Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param[in] handle Instance's key to be unregistered.
     * @return Returns the operation's result.
     * If the operation finishes successfully, ReturnCode_t::RETCODE_OK is returned.
     */
    RTPS_DllAPI ReturnCode_t unregister_instance(
            void* instance,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /**
     * Returns the DataWriter's GUID
     * @return Reference to the DataWriter GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid();

    /**
     * Returns the DataWriter's GUID
     * @return Reference to the DataWriter GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid() const;

    /**
     * Returns the DataWriter's InstanceHandle
     * @return Copy of the DataWriter InstanceHandle
     */
    RTPS_DllAPI fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get data type associated to the DataWriter
     * @return Copy of the TypeSupport
     */
    RTPS_DllAPI TypeSupport get_type() const;

    /**
     * Waits the current thread until all writers have received their acknowledgments.
     * @param max_wait Maximum blocking time for this operation
     * @return RETCODE_OK if the DataWriter receive the acknowledgments before the time expires and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param[out] status Deadline missed status struct
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_offered_deadline_missed_status(
            fastrtps::OfferedDeadlineMissedStatus& status);

    /**
     * @brief Returns the offered incompatible qos status
     * @param[out] status Offered incompatible qos status struct
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status);

    /**
     * Establishes the DataWriterQos for this DataWriter.
     * @param qos DataWriterQos to be set
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DataWriterQos& qos);

    /**
     * Retrieves the DataWriterQos for this DataWriter.
     * @return Reference to the current DataWriterQos
     */
    RTPS_DllAPI const DataWriterQos& get_qos() const;

    /**
     * Fills the DataWriterQos with the values of this DataWriter.
     * @param qos DataWriterQos object where the qos is returned.
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            DataWriterQos& qos) const;

    /**
     * Retrieves the topic for this DataWriter.
     * @return Pointer to the associated Topic
     */
    RTPS_DllAPI Topic* get_topic() const;

    /**
     * Retrieves the listener for this DataWriter.
     * @return Pointer to the DataWriterListener
     */
    RTPS_DllAPI const DataWriterListener* get_listener() const;

    /**
     * Modifies the DataWriterListener, sets the mask to StatusMask::all()
     * @param listener new value for the DataWriterListener
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DataWriterListener* listener);

    /**
     * Modifies the DataWriterListener.
     * @param listener new value for the DataWriterListener
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DataWriterListener* listener,
            const StatusMask& mask);

    /* TODO
       bool get_key_value(
            void* key_holder,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /**
     * @brief This operation requests the middleware to delete the data (the actual deletion is postponed until there is no
     * more use for that data in the whole system). In general, applications are made aware of the deletion by means of
     * operations on the DataReader objects that already knew that instance. This operation does not modify the value of
     * the instance. The instance parameter is passed just for the purposes of identifying the instance.
     * When this operation is used, the Service will automatically supply the value of the source_timestamp that is made
     * available to DataReader objects by means of the source_timestamp attribute inside the SampleInfo. The constraints
     * on the values of the handle parameter and the corresponding error behavior are the same specified for the
     * unregister_instance operation.
     * @param[in] data Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param[in] handle InstanceHandle of the data
     * @return RETCODE_PRECONDITION_NOT_MET if the handle introduced does not match with the one associated to the data,
     * RETCODE_OK if the data is correctly sent and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t dispose(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);

    /**
     * @brief Returns the liveliness lost status
     * @param status Liveliness lost status struct
     * @return RETCODE_OK
     */
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

    /**
     * @brief Getter for the Publisher that creates this DataWriter
     * @return Pointer to the Publisher
     */
    RTPS_DllAPI const Publisher* get_publisher() const;

    /**
     * @brief This operation manually asserts the liveliness of the DataWriter. This is used in combination with the
     * LivelinessQosPolicy to indicate to the Service that the entity remains active.
     * This operation need only be used if the LIVELINESS setting is either MANUAL_BY_PARTICIPANT or MANUAL_BY_TOPIC.
     * Otherwise, it has no effect.
     * @note Writing data via the write operation on a DataWriter asserts liveliness on the DataWriter itself and its
     * DomainParticipant. Consequently the use of assert_liveliness is only needed if the application is not writing data
     * regularly.
     * @return RETCODE_OK if asserted, RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t assert_liveliness();

    /**
     * @brief Clears the DataWriter history
     * @param removed size_t pointer to return the size of the data removed
     * @return RETCODE_OK if the samples are removed and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t clear_history(
            size_t* removed);

protected:

    DataWriterImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITER_HPP_
