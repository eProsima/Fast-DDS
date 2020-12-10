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
 * @file DataReader.hpp
 *
 */

#ifndef _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_

#include <vector>
#include <cstdint>

#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Time_t.h>

#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace sub {

class DataReader;

} // namespace sub
} // namespace dds

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

namespace rtps {
class ReaderAttributes;
struct GUID_t;
} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class DataReaderQos;
class TopicDescription;
struct LivelinessChangedStatus;

using SampleInfoSeq = LoanableSequence<SampleInfo>;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReader : public DomainEntity
{
protected:

    friend class DataReaderImpl;
    friend class SubscriberImpl;

    /**
     * Create a data reader, assigning its pointer to the associated implementation.
     * Don't use directly, create DataReader using create_datareader from Subscriber.
     */
    RTPS_DllAPI DataReader(
            DataReaderImpl* impl,
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI DataReader(
            Subscriber* s,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~DataReader();

    /**
     * @brief This operation enables the DataReader
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the Subscriber creating this
     *         DataReader is not enabled.
     */
    RTPS_DllAPI ReturnCode_t enable() override;

    /**
     * Method to block the current thread until an unread message is available
     * @param timeout Max blocking time for this operation
     * @return true if there is new unread message, false if timeout
     */
    RTPS_DllAPI bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of Data values from the DataReader. The caller can limit the size
     * of the returned collection with the @c max_samples parameter.
     *
     * The properties of the @c data_values collection and the setting of the @ref PresentationQosPolicy may
     * impose further limits on the size of the returned ‘list.’
     *
     * 1. If @ref PresentationQosPolicy::access_scope is @ref INSTANCE_PRESENTATION_QOS, then the returned
     *    collection is a 'list' where samples belonging to the same data-instance are consecutive.
     * 2. If @ref PresentationQosPolicy::access_scope is @ref TOPIC_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c false, then the returned collection is a
     *    'list' where samples belonging to the same data-instance are consecutive.
     * 3. If @ref PresentationQosPolicy::access_scope is @ref TOPIC_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c true, then the returned collection is a
     *    'list' where samples belonging to the same instance may or may not be consecutive. This is because to
     *    preserve order it may be necessary to mix samples from different instances.
     * 4. If @ref PresentationQosPolicy::access_scope is @ref GROUP_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c false, then the returned collection is a
     *    'list' where samples belonging to the same data instance are consecutive.
     * 5. If @ref PresentationQosPolicy::access_scope is @ref GROUP_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c true, then the returned collection contains at
     *    most one sample. The difference in this case is due to the fact that it is required that the application
     *    is able to read samples belonging to different DataReader objects in a specific order.
     *
     * In any case, the relative order between the samples of one instance is consistent with the
     * @ref DestinationOrderQosPolicy:
     *
     * - If @ref DestinationOrderQosPolicy::kind is @ref BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, samples
     *   belonging to the same instances will appear in the relative order in which there were received (FIFO,
     *   earlier samples ahead of the later samples).
     * - If @ref DestinationOrderQosPolicy::kind is @ref BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS, samples
     *   belonging to the same instances will appear in the relative order implied by the source_timestamp (FIFO,
     *   smaller values of source_timestamp ahead of the larger values).
     *
     * The actual number of samples returned depends on the information that has been received by the middleware
     * as well as the @ref HistoryQosPolicy, @ref ResourceLimitsQosPolicy, and @ref ReaderResourceLimitsQos:
     *
     * - In the case where the @ref HistoryQosPolicy::kind is KEEP_LAST_HISTORY_QOS, the call will return at most
     *   @ref HistoryQosPolicy::depth samples per instance.
     * - The maximum number of samples returned is limited by @ref ResourceLimitsQosPolicy::max_samples, and by
     *   @ref ReaderResourceLimitsQos::max_samples_per_read.
     * - For multiple instances, the number of samples returned is additionally limited by the product
     *   (@ref ResourceLimitsQosPolicy::max_samples_per_instance * @ref ResourceLimitsQosPolicy::max_instances).
     * - If ReaderResourceLimitsQos::sample_infos_allocation has a maximum limit, the number of samples returned
     *   may also be limited if insufficient @ref SampleInfo resources are available.
     *
     * If the operation succeeds and the number of samples returned has been limited (by means of a maximum limit,
     * as listed above, or insufficient @ref SampleInfo resources), the call will complete successfully and provide
     * those samples the reader is able to return. The user may need to make additional calls, or return outstanding
     * loaned buffers in the case of insufficient resources, in order to access remaining samples.
     *
     * In addition to the collection of samples, the read operation also uses a collection of @ref SampleInfo
     * structures (@c sample_infos).
     *
     * The initial (input) properties of the @c data_values and @c sample_infos collections will determine the
     * precise behavior of this operation. For the purposes of this description the collections are modeled as having
     * three properties:
     * - the current length (@c len, see @ref LoanableCollection::length())
     * - the maximum length (@c max_len, see @ref LoanableCollection::maximum())
     * - whether the collection container owns the memory of the elements within
     *   (@c owns, see @ref LoanableCollection::has_ownership())
     *
     * The initial (input) values of the @c len, @c max_len, and @c owns properties for the @c data_values and
     * @c sample_infos collections govern the behavior of the read operation as specified by the following rules:
     *
     * 1. The values of @c len, @c max_len, and @c owns for the two collections must be identical. Otherwise read
     *    will fail with RETCODE_PRECONDITION_NOT_MET.
     * 2. On successful output, the values of @c len, @c max_len, and @c owns will be the same for both collections.
     * 3. If the input <tt> max_len == 0 </tt>, then the @c data_values and @c sample_infos collections will be
     *    filled with elements that are 'loaned' by the DataReader. On output, @c owns will be @c false, @c len will
     *    be set to the number of values returned, and @c max_len will be set to a value verifying
     *    <tt> max_len >= len </tt>. The use of this variant allows for zero-copy access to the data and the
     *    application will need to return the loan to the DataReader using the @ref return_loan operation.
     * 4. If the input <tt> max_len > 0 </tt> and the input <tt> owns == false </tt>, then the read operation will
     *    fail with RETCODE_PRECONDITION_NOT_MET. This avoids the potential hard-to-detect memory leaks caused by an
     *    application forgetting to return the loan.
     * 5. If input <tt> max_len > 0 </tt> and the input <tt> owns == true </tt>, then the read operation will copy
     *    the Data values and SampleInfo values into the elements already inside the collections. On output, @c owns
     *    will be @c true, @c len will be set to the number of values copied, and @c max_len will remain unchanged.
     *    The use of this variant forces a copy but the application can control where the copy is placed and the
     *    application will not need to return the loan. The number of samples copied depends on the values of
     *    @c max_len and @c max_samples:
     *    - If <tt> max_samples == LENGTH_UNLIMITED </tt>, then at most @c max_len values will be copied. The use of
     *      this variant lets the application limit the number of samples returned to what the sequence can
     *      accommodate.
     *    - If <tt> max_samples <= max_len </tt>, then at most @c max_samples values will be copied. The use of this
     *      variant lets the application limit the number of samples returned to fewer that what the sequence can
     *      accommodate.
     *    - If <tt> max_samples > max_len </tt>, then the read operation will fail with RETCODE_PRECONDITION_NOT_MET.
     *      This avoids the potential confusion where the application expects to be able to access up to
     *      @c max_samples, but that number can never be returned, even if they are available in the DataReader,
     *      because the output sequence cannot accommodate them.
     *
     * As described above, upon return the @c data_values and @c sample_infos collections may contain elements
     * 'loaned' from the DataReader. If this is the case, the application will need to use the @ref return_loan
     * operation to return the loan once it is no longer using the Data in the collection. Upon return from
     * @ref return_loan, the collection will have <tt> max_len == 0 </tt> and <tt> owns == false </tt>.
     *
     * The application can determine whether it is necessary to return the loan or not based on the state of the
     * collections when the read operation was called, or by accessing the @c owns property. However, in many cases
     * it may be simpler to always call @ref return_loan, as this operation is harmless (i.e., leaves all elements
     * unchanged) if the collection does not have a loan.
     *
     * On output, the collection of Data values and the collection of SampleInfo structures are of the same length
     * and are in a one-to-one correspondence. Each SampleInfo provides information, such as the @c source_timestamp,
     * the @c sample_state, @c view_state, and @c instance_state, etc., about the corresponding sample.
     *
     * Some elements in the returned collection may not have valid data. If the @c instance_state in the SampleInfo is
     * @ref NOT_ALIVE_DISPOSED_INSTANCE_STATE or @ref NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, then the last sample for
     * that instance in the collection, that is, the one whose SampleInfo has <tt> sample_rank == 0 </tt> does not
     * contain valid data. Samples that contain no data do not count towards the limits imposed by the
     * @ref ResourceLimitsQosPolicy.
     *
     * The act of reading a sample changes its @c sample_state to @ref READ_SAMPLE_STATE. If the sample belongs
     * to the most recent generation of the instance, it will also set the @c view_state of the instance to be
     * @ref NOT_NEW_VIEW_STATE. It will not affect the @c instance_state of the instance.
     *
     * If the DataReader has no samples that meet the constraints, the operations fails with RETCODE_NO_DATA.
     *
     * @em Important: If the samples "returned" by this method are loaned from the middleware (see @ref take
     * for more information on memory loaning), it is important that their contents not be changed. Because the
     * memory in which the data is stored belongs to the middleware, any modifications made to the data will be
     * seen the next time the same samples are read or taken; the samples will no longer reflect the state that
     * was received from the network.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                 @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                 available, up to the limits described above.
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    RTPS_DllAPI ReturnCode_t read(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data values from the DataReader. The behavior is identical to
     * @ref read, except that all samples returned belong to the single specified instance whose handle is
     * @c a_handle.
     *
     * Upon successful completion, the data collection will contain samples all belonging to the same instance.
     * The corresponding @ref SampleInfo verifies ef SampleInfo::instance_handle == @c a_handle.
     *
     * This operation is semantically equivalent to the @ref read operation, except in building the collection.
     * The DataReader will check that the sample belongs to the specified instance and otherwise it will not place
     * the sample in the returned collection.
     *
     * The behavior of this operation follows the same rules as the @read operation regarding the pre-conditions and
     * post-conditions for the @c data_values and @c sample_infos. Similar to @ref read, this operation may 'loan'
     * elements to the output collections, which must then be returned by means of @ref return_loan.
     *
     * If the DataReader has no samples that meet the constraints, the operations fails with RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                 @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                 available, up to the limits described in the documentation for @ref read().
     * @param [in]     a_handle        The specified instance to return samples for. The method will fail with
     *                                 RETCODE_BAD_PARAMETER if the handle does not correspond to an existing
     *                                 data-object known to the DataReader.
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    RTPS_DllAPI ReturnCode_t read_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& a_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * @brief This operation copies the next, non-previously accessed Data value from the DataReader; the operation also
     * copies the corresponding SampleInfo. The implied order among the samples stored in the DataReader is the same as for
     * the read operation.
     *
     * The read_next_sample operation is semantically equivalent to the read operation where the input Data sequence has
     * max_length=1, the sample_states=NOT_READ, the view_states=ANY_VIEW_STATE, and the instance_states=ANY_INSTANCE_STATE.
     *
     * The read_next_sample operation provides a simplified API to ‘read’ samples avoiding the need for the application to
     * manage sequences and specify states.
     *
     * If there is no unread data in the DataReader, the operation will return NO_DATA and nothing is copied
     * @param data Data pointer to store the sample
     * @param info SampleInfo pointer to store the sample information
     * @return RETCODE_NO_DATA if the history is empty, RETCODE_OK if the next sample is returned and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t read_next_sample(
            void* data,
            SampleInfo* info);

    /* TODO
       RTPS_DllAPI bool take(
            std::vector<void*>& data_values,
            std::vector<SampleInfo>& sample_infos,
            uint32_t max_samples);
     */

    /**
     * @brief This operation copies the next, non-previously accessed Data value from the DataReader and ‘removes’ it from
     * the DataReader so it is no longer accessible. The operation also copies the corresponding SampleInfo. This operation
     * is analogous to the read_next_sample except for the fact that the sample is ‘removed’ from the DataReader.
     *
     * The take_next_sample operation is semantically equivalent to the take operation where the input sequence has
     * max_length=1, the sample_states=NOT_READ, the view_states=ANY_VIEW_STATE, and the instance_states=ANY_INSTANCE_STATE.
     *
     * This operation provides a simplified API to ’take’ samples avoiding the need for the application to manage sequences
     * and specify states.
     *
     * If there is no unread data in the DataReader, the operation will return NO_DATA and nothing is copied.
     * @param data Data pointer to store the sample
     * @param info SampleInfo pointer to store the sample information
     * @return RETCODE_NO_DATA if the history is empty, RETCODE_OK if the next sample is returned and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t take_next_sample(
            void* data,
            SampleInfo* info);

    ///@}

    /**
     * @brief Returns information about the first untaken sample.
     * @param [out] info Pointer to a SampleInfo_t structure to store first untaken sample information.
     * @return RETCODE_OK if sample info was returned. RETCODE_NO_DATA if there is no sample to take.
     */
    RTPS_DllAPI ReturnCode_t get_first_untaken_info(
            SampleInfo* info);

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid();

    /**
     * @brief Getter for the associated InstanceHandle
     * @return Copy of the InstanceHandle
     */
    RTPS_DllAPI InstanceHandle_t get_instance_handle() const;

    /**
     * Getter for the data type
     * @return TypeSupport associated to the DataReader
     */
    TypeSupport type();

    /**
     * Get TopicDescription
     * @return TopicDescription pointer
     */
    RTPS_DllAPI const TopicDescription* get_topicdescription() const;

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    RTPS_DllAPI ReturnCode_t get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    /**
     * @brief Get the requested incompatible qos status
     * @param[out] status Requested incompatible qos status
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status);

    /**
     * @brief Setter for the DataReaderQos
     * @param qos new value for the DataReaderQos
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DataReaderQos& qos);

    /**
     * @brief Getter for the DataReaderQos
     * @return Pointer to the DataReaderQos
     */
    RTPS_DllAPI const DataReaderQos& get_qos() const;

    /**
     * @brief Getter for the DataReaderQos
     * @param qos DataReaderQos where the qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            DataReaderQos& qos) const;

    /**
     * Modifies the DataReaderListener, sets the mask to StatusMask::all()
     * @param listener new value for the DataReaderListener
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DataReaderListener* listener);

    /**
     * Modifies the DataReaderListener.
     * @param listener new value for the DataReaderListener
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DataReaderListener* listener,
            const StatusMask& mask);
    /**
     * @brief Getter for the DataReaderListener
     * @return Pointer to the DataReaderListener
     */
    RTPS_DllAPI const DataReaderListener* get_listener() const;

    /* TODO
       RTPS_DllAPI bool get_key_value(
            void* data,
            const InstanceHandle_t& handle);
     */

    /**
     * @brief Get the liveliness changed status
     * @param status LivelinessChangedStatus object where the status is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status) const;

    /* TODO
       RTPS_DllAPI bool get_requested_incompatible_qos_status(
            fastrtps::RequestedIncompatibleQosStatus& status) const;
     */

    /* TODO
       RTPS_DllAPI bool get_sample_lost_status(
            fastrtps::SampleLostStatus& status) const;
     */

    /* TODO
       RTPS_DllAPI bool get_sample_rejected_status(
            fastrtps::SampleRejectedStatus& status) const;
     */

    /**
     * @brief Getter for the Subscriber
     * @return Subscriber pointer
     */
    RTPS_DllAPI const Subscriber* get_subscriber() const;

    /* TODO
       RTPS_DllAPI bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
     */

protected:

    DataReaderImpl* impl_;

    friend class ::dds::sub::DataReader;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_*/
