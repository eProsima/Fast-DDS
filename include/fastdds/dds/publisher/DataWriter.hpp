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

#ifndef FASTDDS_DDS_PUBLISHER__DATAWRITER_HPP
#define FASTDDS_DDS_PUBLISHER__DATAWRITER_HPP

#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriteParams;
struct GUID_t;

} // namespace rtps

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
 *
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
    DataWriter(
            DataWriterImpl* impl,
            const StatusMask& mask = StatusMask::all());

    DataWriter(
            Publisher* pub,
            Topic* topic,
            const DataWriterQos& qos = DATAWRITER_QOS_DEFAULT,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    /**
     * How to initialize samples loaned with @ref loan_sample
     */
    enum class LoanInitializationKind
    {
        /**
         * @brief Do not perform initialization of sample.
         *
         * This is the default initialization scheme of loaned samples.
         * It is the fastest scheme, but implies the user should take care of writing
         * every field on the data type before calling @ref write on the loaned sample.
         */
        NO_LOAN_INITIALIZATION,

        /**
         * @brief Initialize all memory with zero-valued bytes.
         *
         * The contents of the loaned sample will be zero-initialized upon return of
         * @ref loan_sample.
         */
        ZERO_LOAN_INITIALIZATION,

        /**
         * @brief Use in-place constructor initialization.
         *
         * This will call the constructor of the data type over the memory space being
         * returned by @ref loan_sample.
         */
        CONSTRUCTED_LOAN_INITIALIZATION
    };

    virtual ~DataWriter();

    /**
     * @brief This operation enables the DataWriter
     *
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the Publisher creating this
     *         DataWriter is not enabled.
     */
    FASTDDS_EXPORTED_API ReturnCode_t enable() override;

    /**
     * Write data to the topic.
     *
     * @param data Pointer to the data
     * @return RETCODE_OK if the data is correctly sent or a ReturnCode related to the specific error otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t write(
            const void* const data);

    /**
     * Write data with params to the topic.
     *
     * @param data Pointer to the data
     * @param params Extra write parameters.
     * @return RETCODE_OK if the data is correctly sent or a ReturnCode related to the specific error otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t write(
            const void* const data,
            fastdds::rtps::WriteParams& params);

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
    FASTDDS_EXPORTED_API ReturnCode_t write(
            const void* const data,
            const InstanceHandle_t& handle);

    /**
     * @brief This operation performs the same function as write except that it also provides the value for the
     * @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp" that is made available to DataReader
     * objects by means of the @ref eprosima::fastdds::dds::SampleInfo::source_timestamp attribute "source_timestamp"
     * inside the SampleInfo.
     * The constraints on the values of the @c handle parameter and the corresponding error behavior are the same
     * specified for the @ref write operation. This operation may block and return RETCODE_TIMEOUT under the same
     * circumstances described for the @ref write operation.
     * This operation may return RETCODE_OUT_OF_RESOURCES, RETCODE_PRECONDITION_NOT_MET or RETCODE_BAD_PARAMETER under
     * the same circumstances described for the write operation.
     *
     * @param data Pointer to the data
     * @param handle InstanceHandle_t
     * @param timestamp Time_t used to set the source_timestamp.
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t write_w_timestamp(
            const void* const data,
            const InstanceHandle_t& handle,
            const fastdds::dds::Time_t& timestamp);

    /*!
     * @brief Informs that the application will be modifying a particular instance.
     * It gives an opportunity to the middleware to pre-configure itself to improve performance.
     *
     * @param [in] instance Sample used to get the instance's key.
     * @return Handle containing the instance's key.
     * This handle could be used in successive `write` or `dispose` operations.
     * In case of error, HANDLE_NIL will be returned.
     */
    FASTDDS_EXPORTED_API InstanceHandle_t register_instance(
            const void* const instance);

    /**
     * @brief This operation performs the same function as register_instance and can be used instead of
     * @ref register_instance in the cases where the application desires to specify the value for the
     * @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp".
     * The @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp" potentially affects the relative
     * order in which readers observe events from multiple writers. See the QoS policy
     * @ref eprosima::fastdds::dds::DataWriterQos::destination_order "DESTINATION_ORDER".
     *
     * This operation may block and return RETCODE_TIMEOUT under the same circumstances described for the @ref write
     * operation.
     *
     * This operation may return RETCODE_OUT_OF_RESOURCES under the same circumstances described for the
     * @ref write operation.
     *
     * @param instance  Sample used to get the instance's key.
     * @param timestamp Time_t used to set the source_timestamp.
     * @return Handle containing the instance's key.
     */
    FASTDDS_EXPORTED_API InstanceHandle_t register_instance_w_timestamp(
            const void* const instance,
            const fastdds::dds::Time_t& timestamp);

    /*!
     * @brief This operation reverses the action of `register_instance`.
     * It should only be called on an instance that is currently registered.
     * Informs the middleware that the DataWriter is not intending to modify any more of that data instance.
     * Also indicates that the middleware can locally remove all information regarding that instance.
     *
     * @param [in] instance Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param [in] handle Instance's key to be unregistered.
     * @return Returns the operation's result.
     * If the operation finishes successfully, RETCODE_OK is returned.
     */
    FASTDDS_EXPORTED_API ReturnCode_t unregister_instance(
            const void* const instance,
            const InstanceHandle_t& handle);

    /**
     * @brief This operation performs the same function as @ref unregister_instance and can be used instead of
     * @ref unregister_instance in the cases where the application desires to specify the value for the
     * @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp".
     * The @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp" potentially affects the relative
     * order in which readers observe events from multiple writers. See the QoS policy
     * @ref eprosima::fastdds::dds::DataWriterQos::destination_order "DESTINATION_ORDER".
     *
     * The constraints on the values of the @c handle parameter and the corresponding error behavior are the same
     * specified for the @ref unregister_instance operation.
     *
     * This operation may block and return RETCODE_TIMEOUT under the same circumstances described for the write
     * operation
     *
     * @param instance  Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param handle Instance's key to be unregistered.
     * @param timestamp Time_t used to set the source_timestamp.
     * @return Handle containing the instance's key.
     */
    FASTDDS_EXPORTED_API ReturnCode_t unregister_instance_w_timestamp(
            const void* const instance,
            const InstanceHandle_t& handle,
            const fastdds::dds::Time_t& timestamp);

    /**
     * This operation can be used to retrieve the instance key that corresponds to an
     * @ref eprosima::fastdds::dds::Entity::instance_handle_ "instance_handle".
     * The operation will only fill the fields that form the key inside the key_holder instance.
     *
     * This operation may return BAD_PARAMETER if the InstanceHandle_t handle does not correspond to an existing
     * data-object known to the DataWriter. If the implementation is not able to check invalid handles then the result
     * in this situation is unspecified.
     *
     * @param [in,out] key_holder  Sample where the key fields will be returned.
     * @param [in] handle          Handle to the instance to retrieve the key values from.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_key_value(
            void* key_holder,
            const InstanceHandle_t& handle);

    /**
     * NOT YET IMPLEMENTED
     *
     * Takes as a parameter an instance and returns a handle that can be used in subsequent operations that accept an
     * instance handle as an argument. The instance parameter is only used for the purpose of examining the fields that
     * define the key.
     *
     * @param [in] instance Data pointer to the sample
     *
     * @return handle of the given instance
     */
    FASTDDS_EXPORTED_API InstanceHandle_t lookup_instance(
            const void* const instance) const;

    /**
     * Returns the DataWriter's GUID
     *
     * @return Reference to the DataWriter GUID
     */
    FASTDDS_EXPORTED_API const fastdds::rtps::GUID_t& guid() const;

    /**
     * Returns the DataWriter's InstanceHandle
     *
     * @return Copy of the DataWriter InstanceHandle
     */
    FASTDDS_EXPORTED_API InstanceHandle_t get_instance_handle() const;

    /**
     * Get data type associated to the DataWriter
     *
     * @return Copy of the TypeSupport
     */
    FASTDDS_EXPORTED_API TypeSupport get_type() const;

    /**
     * Waits the current thread until all writers have received their acknowledgments.
     *
     * @param max_wait Maximum blocking time for this operation
     * @return RETCODE_OK if the DataWriter receive the acknowledgments before the time expires and RETCODE_ERROR otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t wait_for_acknowledgments(
            const fastdds::dds::Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     *
     * @param [out] status Deadline missed status struct
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status);

    /**
     * @brief Returns the offered incompatible qos status
     *
     * @param [out] status Offered incompatible qos status struct
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status);

    /**
     * @brief Returns the publication matched status
     *
     * @param [out] status publication matched status struct
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_publication_matched_status(
            PublicationMatchedStatus& status) const;

    /**
     * Establishes the DataWriterQos for this DataWriter.
     *
     * @param qos DataWriterQos to be set
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const DataWriterQos& qos);

    /**
     * Retrieves the DataWriterQos for this DataWriter.
     *
     * @return Reference to the current DataWriterQos
     */
    FASTDDS_EXPORTED_API const DataWriterQos& get_qos() const;

    /**
     * Fills the DataWriterQos with the values of this DataWriter.
     *
     * @param qos DataWriterQos object where the qos is returned.
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            DataWriterQos& qos) const;

    /**
     * Retrieves the topic for this DataWriter.
     *
     * @return Pointer to the associated Topic
     */
    FASTDDS_EXPORTED_API Topic* get_topic() const;

    /**
     * Retrieves the listener for this DataWriter.
     *
     * @return Pointer to the DataWriterListener
     */
    FASTDDS_EXPORTED_API const DataWriterListener* get_listener() const;

    /**
     * Modifies the DataWriterListener, sets the mask to StatusMask::all()
     *
     * @param listener new value for the DataWriterListener
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DataWriterListener* listener);

    /**
     * Modifies the DataWriterListener.
     *
     * @param listener new value for the DataWriterListener
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DataWriterListener* listener,
            const StatusMask& mask);

    /**
     * @brief This operation requests the middleware to delete the data (the actual deletion is postponed until there is no
     * more use for that data in the whole system). In general, applications are made aware of the deletion by means of
     * operations on the DataReader objects that already knew that instance. This operation does not modify the value of
     * the instance. The instance parameter is passed just for the purposes of identifying the instance.
     * When this operation is used, the Service will automatically supply the value of the source_timestamp that is made
     * available to DataReader objects by means of the source_timestamp attribute inside the SampleInfo. The constraints
     * on the values of the handle parameter and the corresponding error behavior are the same specified for the
     * unregister_instance operation.
     *
     * @param [in] data Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param [in] handle InstanceHandle of the data
     * @return RETCODE_PRECONDITION_NOT_MET if the handle introduced does not match with the one associated to the data,
     * RETCODE_OK if the data is correctly sent and RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t dispose(
            const void* const data,
            const InstanceHandle_t& handle);

    /**
     * @brief This operation performs the same functions as @ref dispose except that the application provides the value
     * for the @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp" that is made available to
     * DataReader objects by means of the @ref eprosima::fastdds::dds::SampleInfo::source_timestamp "source_timestamp"
     * attribute inside the SampleInfo.
     *
     * The constraints on the values of the @c handle parameter and the corresponding error behavior are the same
     * specified for the @ref dispose operation.
     *
     * This operation may return RETCODE_PRECONDITION_NOT_MET and RETCODE_BAD_PARAMETER under the same circumstances
     * described for the @ref dispose operation.
     *
     * This operation may return RETCODE_TIMEOUT and RETCODE_OUT_OF_RESOURCES under the same circumstances described
     * for the @ref write operation.
     *
     * @param instance  Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param handle Instance's key to be disposed.
     * @param timestamp Time_t used to set the source_timestamp.
     * @return FASTDDS_EXPORTED_API
     */
    FASTDDS_EXPORTED_API ReturnCode_t dispose_w_timestamp(
            const void* const instance,
            const InstanceHandle_t& handle,
            const fastdds::dds::Time_t& timestamp);
    /**
     * @brief Returns the liveliness lost status
     *
     * @param status Liveliness lost status struct
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status);

    /**
     * @brief Getter for the Publisher that creates this DataWriter
     *
     * @return Pointer to the Publisher
     */
    FASTDDS_EXPORTED_API const Publisher* get_publisher() const;

    /**
     * @brief This operation manually asserts the liveliness of the DataWriter. This is used in combination with the
     * LivelinessQosPolicy to indicate to the Service that the entity remains active.
     * This operation need only be used if the LIVELINESS setting is either MANUAL_BY_PARTICIPANT or MANUAL_BY_TOPIC.
     * Otherwise, it has no effect.
     *
     * @note Writing data via the write operation on a DataWriter asserts liveliness on the DataWriter itself and its
     * DomainParticipant. Consequently the use of assert_liveliness is only needed if the application is not writing data
     * regularly.
     *
     * @return RETCODE_OK if asserted, RETCODE_ERROR otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t assert_liveliness();

    /**
     * @brief Retrieves in a subscription associated with the DataWriter
     *
     * @param [out] subscription_data subscription data struct
     * @param subscription_handle InstanceHandle_t of the subscription
     * @return RETCODE_OK
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_matched_subscription_data(
            SubscriptionBuiltinTopicData& subscription_data,
            const InstanceHandle_t& subscription_handle) const;

    /**
     * @brief Fills the given vector with the InstanceHandle_t of matched DataReaders
     *
     * @param [out] subscription_handles Vector where the InstanceHandle_t are returned
     * @return RETCODE_OK
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_matched_subscriptions(
            std::vector<InstanceHandle_t>& subscription_handles) const;

    /**
     * @brief Clears the DataWriter history
     *
     * @param removed size_t pointer to return the size of the data removed
     * @return RETCODE_OK if the samples are removed and RETCODE_ERROR otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t clear_history(
            size_t* removed);

    /**
     * @brief Get a pointer to the internal pool where the user could directly write.
     *
     * This method can only be used on a DataWriter for a plain data type. It will provide the
     * user with a pointer to an internal buffer where the data type can be prepared for sending.
     *
     * When using NO_LOAN_INITIALIZATION on the initialization parameter, which is the default,
     * no assumptions should be made on the contents where the pointer points to, as it may be an
     * old pointer being reused. See @ref LoanInitializationKind for more details.
     *
     * Once the sample has been prepared, it can then be published by calling @ref write.
     * After a successful call to @ref write, the middleware takes ownership of the loaned pointer again,
     * and the user should not access that memory again.
     *
     * If, for whatever reason, the sample is not published, the loan can be returned by calling
     * @ref discard_loan.
     *
     * @param [out] sample          Pointer to the sample on the internal pool.
     * @param [in]  initialization  How to initialize the loaned sample.
     *
     * @return RETCODE_ILLEGAL_OPERATION when the data type does not support loans.
     * @return RETCODE_NOT_ENABLED if the writer has not been enabled.
     * @return RETCODE_OUT_OF_RESOURCES if the pool has been exhausted.
     * @return RETCODE_OK if a pointer to a sample is successfully obtained.
     */
    FASTDDS_EXPORTED_API ReturnCode_t loan_sample(
            void*& sample,
            LoanInitializationKind initialization = LoanInitializationKind::NO_LOAN_INITIALIZATION);

    /**
     * @brief Discards a loaned sample pointer.
     *
     * See the description on @ref loan_sample for how and when to call this method.
     *
     * @param [in,out] sample  Pointer to the previously loaned sample.
     *
     * @return RETCODE_ILLEGAL_OPERATION when the data type does not support loans.
     * @return RETCODE_NOT_ENABLED if the writer has not been enabled.
     * @return RETCODE_BAD_PARAMETER if the pointer does not correspond to a loaned sample.
     * @return RETCODE_OK if the loan is successfully discarded.
     */
    FASTDDS_EXPORTED_API ReturnCode_t discard_loan(
            void*& sample);

    /**
     * @brief Get the list of locators from which this DataWriter may send data.
     *
     * @param [out] locators  LocatorList where the list of locators will be stored.
     *
     * @return NOT_ENABLED if the reader has not been enabled.
     * @return OK if a list of locators is returned.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_sending_locators(
            rtps::LocatorList& locators) const;

    /**
     * Block the current thread until the writer has received the acknowledgment corresponding to the given instance.
     * Operations performed on the same instance while the current thread is waiting will not be taken into
     * consideration, i.e. this method may return `RETCODE_OK` with those operations unacknowledged.
     *
     * @param instance Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param handle Instance handle of the data.
     * @param max_wait Maximum blocking time for this operation.
     *
     * @return RETCODE_NOT_ENABLED if the writer has not been enabled.
     * @return RETCODE_BAD_PARAMETER if `instance` is not a valid pointer.
     * @return RETCODE_PRECONDITION_NOT_MET if the topic does not have a key, the key is unknown to the writer,
     *         or the key is not consistent with `handle`.
     * @return RETCODE_OK if the DataWriter received the acknowledgments before the time expired.
     * @return RETCODE_TIMEOUT otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t wait_for_acknowledgments(
            const void* const instance,
            const InstanceHandle_t& handle,
            const fastdds::dds::Duration_t& max_wait);

    /**
     * Retrieve the publication data discovery information.
     *
     * @param [out] publication_data The publication data discovery information.
     *
     * @return NOT_ENABLED if the writer has not been enabled.
     * @return OK if the publication data is returned.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_publication_builtin_topic_data(
            PublicationBuiltinTopicData& publication_data) const;

protected:

    DataWriterImpl* impl_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER__DATAWRITER_HPP
