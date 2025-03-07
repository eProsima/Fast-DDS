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

#ifndef FASTDDS_DDS_SUBSCRIBER__DATAREADER_HPP
#define FASTDDS_DDS_SUBSCRIBER__DATAREADER_HPP

#include <cstdint>
#include <vector>

#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

namespace dds {
namespace sub {

class DataReader;

} // namespace sub
} // namespace dds

namespace eprosima {
namespace fastdds {
namespace rtps {
struct GUID_t;
} // namespace rtps

namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class DataReaderQos;
class TopicDescription;
struct LivelinessChangedStatus;

// Not yet implemented
class QueryCondition;

using SampleInfoSeq = LoanableSequence<SampleInfo>;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *
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
    DataReader(
            DataReaderImpl* impl,
            const StatusMask& mask = StatusMask::all());

    DataReader(
            Subscriber* s,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    /**
     * @brief Destructor.
     */
    virtual ~DataReader();

    /**
     * @brief This operation enables the DataReader.
     *
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the Subscriber creating this
     *         DataReader is not enabled.
     */
    FASTDDS_EXPORTED_API ReturnCode_t enable() override;

    /**
     * Method to block the current thread until an unread message is available.
     *
     * @param [in] timeout Max blocking time for this operation.
     *
     * @return true if there is new unread message, false if timeout
     */
    FASTDDS_EXPORTED_API bool wait_for_unread_message(
            const fastdds::dds::Duration_t& timeout);

    /**
     * NOT YET IMPLEMENTED
     *
     * @brief Method to block the current thread until an unread message is available.
     *
     * @param [in] max_wait Max blocking time for this operation.
     * @return RETCODE_OK if there is new unread message, RETCODE_TIMEOUT if timeout
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t wait_for_historical_data(
            const fastdds::dds::Duration_t& max_wait) const;


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
     *
     * 2. If @ref PresentationQosPolicy::access_scope is @ref TOPIC_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c false, then the returned collection is a
     *    'list' where samples belonging to the same data-instance are consecutive.
     *
     * 3. If @ref PresentationQosPolicy::access_scope is @ref TOPIC_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c true, then the returned collection is a
     *    'list' where samples belonging to the same instance may or may not be consecutive. This is because to
     *    preserve order it may be necessary to mix samples from different instances.
     *
     * 4. If @ref PresentationQosPolicy::access_scope is @ref GROUP_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c false, then the returned collection is a
     *    'list' where samples belonging to the same data instance are consecutive.
     *
     * 5. If @ref PresentationQosPolicy::access_scope is @ref GROUP_PRESENTATION_QOS and
     *    @ref PresentationQosPolicy::ordered_access is set to @c true, then the returned collection contains at
     *    most one sample. The difference in this case is due to the fact that it is required that the application
     *    is able to read samples belonging to different DataReader objects in a specific order.
     *
     * In any case, the relative order between the samples of one instance is consistent with the
     * @ref eprosima::fastdds::dds::DestinationOrderQosPolicy "DestinationOrderQosPolicy":
     *
     * - If @ref DestinationOrderQosPolicy::kind is @ref BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, samples
     *   belonging to the same instances will appear in the relative order in which there were received (FIFO,
     *   earlier samples ahead of the later samples).
     *
     * - If @ref DestinationOrderQosPolicy::kind is @ref BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS, samples
     *   belonging to the same instances will appear in the relative order implied by the source_timestamp (FIFO,
     *   smaller values of source_timestamp ahead of the larger values).
     *
     * The actual number of samples returned depends on the information that has been received by the middleware
     * as well as the @ref HistoryQosPolicy, @ref ResourceLimitsQosPolicy, and
     * @ref eprosima::fastdds::dds::ReaderResourceLimitsQos "ReaderResourceLimitsQos":
     *
     * - In the case where the @ref HistoryQosPolicy::kind is KEEP_LAST_HISTORY_QOS, the call will return at most
     *   @ref HistoryQosPolicy::depth samples per instance.
     *
     * - The maximum number of samples returned is limited by @ref ResourceLimitsQosPolicy::max_samples, and by
     *   @ref ReaderResourceLimitsQos::max_samples_per_read.
     *
     * - For multiple instances, the number of samples returned is additionally limited by the product
     *   (@ref ResourceLimitsQosPolicy::max_samples_per_instance * @ref ResourceLimitsQosPolicy::max_instances).
     *
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
     *
     * - the current length (@c len, see @ref LoanableCollection::length())
     *
     * - the maximum length (@c max_len, see @ref LoanableCollection::maximum())
     *
     * - whether the collection container owns the memory of the elements within
     *   (@c owns, see @ref LoanableCollection::has_ownership())
     *
     * The initial (input) values of the @c len, @c max_len, and @c owns properties for the @c data_values and
     * @c sample_infos collections govern the behavior of the read operation as specified by the following rules:
     *
     * 1. The values of @c len, @c max_len, and @c owns for the two collections must be identical. Otherwise read
     *    will fail with RETCODE_PRECONDITION_NOT_MET.
     *
     * 2. On successful output, the values of @c len, @c max_len, and @c owns will be the same for both collections.
     *
     * 3. If the input <tt> max_len == 0 </tt>, then the @c data_values and @c sample_infos collections will be
     *    filled with elements that are 'loaned' by the DataReader. On output, @c owns will be @c false, @c len will
     *    be set to the number of values returned, and @c max_len will be set to a value
     *    verifying <tt> max_len >= len </tt>. The use of this variant allows for zero-copy access to the data and the
     *    application will need to return the loan to the DataReader using the @ref return_loan operation.
     *
     * 4. If the input <tt> max_len > 0 </tt> and the input <tt> owns == false </tt>, then the read operation will
     *    fail with RETCODE_PRECONDITION_NOT_MET. This avoids the potential hard-to-detect memory leaks caused by an
     *    application forgetting to return the loan.
     *
     * 5. If input <tt> max_len > 0 </tt> and the input <tt> owns == true </tt>, then the read operation will copy
     *    the Data values and SampleInfo values into the elements already inside the collections. On output, @c owns
     *    will be @c true, @c len will be set to the number of values copied, and @c max_len will remain unchanged.
     *    The use of this variant forces a copy but the application can control where the copy is placed and the
     *    application will not need to return the loan. The number of samples copied depends on the values of
     *    @c max_len and @c max_samples:
     *
     *    - If <tt> max_samples == LENGTH_UNLIMITED </tt>, then at most @c max_len values will be copied. The use of
     *      this variant lets the application limit the number of samples returned to what the sequence can
     *      accommodate.
     *
     *    - If <tt> max_samples <= max_len </tt>, then at most @c max_samples values will be copied. The use of this
     *      variant lets the application limit the number of samples returned to fewer that what the sequence can
     *      accommodate.
     *
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
    FASTDDS_EXPORTED_API ReturnCode_t read(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * This operation accesses via ‘read’ the samples that match the criteria specified in the ReadCondition.
     * This operation is especially useful in combination with QueryCondition to filter data samples based on the
     * content.
     *
     * The specified ReadCondition must be attached to the DataReader; otherwise the operation will fail and return
     * RETCODE_PRECONDITION_NOT_MET.
     *
     * In case the ReadCondition is a ‘plain’ ReadCondition and not the specialized QueryCondition, the
     * operation is equivalent to calling read and passing as @c sample_states, @c view_states and @c instance_states
     * the value of the corresponding attributes in @c a_condition. Using this operation the application can avoid
     * repeating the same parameters specified when creating the ReadCondition.
     *
     * The samples are accessed with the same semantics as the read operation. If the DataReader has no samples that
     * meet the constraints, the return value will be RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned.
     * @param [in]     a_condition     A ReadCondition that returned @c data_values must pass
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t read_w_condition(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            ReadCondition* a_condition);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data values from the DataReader. The behavior is identical to
     * @ref read, except that all samples returned belong to the single specified instance whose handle is
     * @c a_handle.
     *
     * Upon successful completion, the data collection will contain samples all belonging to the same instance.
     * The corresponding @ref SampleInfo verifies @ref SampleInfo::instance_handle == @c a_handle.
     *
     * This operation is semantically equivalent to the @ref read operation, except in building the collection.
     * The DataReader will check that the sample belongs to the specified instance and otherwise it will not place
     * the sample in the returned collection.
     *
     * The behavior of this operation follows the same rules as the @ref read operation regarding the pre-conditions and
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
    FASTDDS_EXPORTED_API ReturnCode_t read_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& a_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data values from the DataReader where all the samples belong to a
     * single instance. The behavior is similar to @ref read_instance, except that the actual instance is not
     * directly specified. Rather, the samples will all belong to the 'next' instance with @c instance_handle
     * 'greater' than the specified 'previous_handle' that has available samples.
     *
     * This operation implies the existence of a total order 'greater-than' relationship between the instance
     * handles. The specifics of this relationship are not all important and are implementation specific. The
     * important thing is that, according to the middleware, all instances are ordered relative to each other.
     * This ordering is between the instance handles, and should not depend on the state of the instance (e.g.
     * whether it has data or not) and must be defined even for instance handles that do not correspond to instances
     * currently managed by the DataReader. For the purposes of the ordering, it should be 'as if' each instance
     * handle was represented as an integer.
     *
     * The behavior of this operation is 'as if' the DataReader invoked @ref read_instance, passing the smallest
     * @c instance_handle among all the ones that: (a) are greater than @c previous_handle, and (b) have available
     * samples (i.e. samples that meet the constraints imposed by the specified states).
     *
     * The special value @ref HANDLE_NIL is guaranteed to be 'less than' any valid @c instance_handle. So the use
     * of the parameter value @c previous_handle == @ref HANDLE_NIL will return the samples for the instance which
     * has the smallest @c instance_handle among all the instances that contain available samples.
     *
     * This operation is intended to be used in an application-driven iteration, where the application starts by
     * passing @c previous_handle == @ref HANDLE_NIL, examines the samples returned, and then uses the
     * @c instance_handle returned in the @ref SampleInfo as the value of the @c previous_handle argument to the
     * next call to @ref read_next_instance. The iteration continues until @ref read_next_instance fails with
     * RETCODE_NO_DATA.
     *
     * Note that it is possible to call the @ref read_next_instance operation with a @c previous_handle that does not
     * correspond to an instance currently managed by the DataReader. This is because as stated earlier the
     * 'greater-than' relationship is defined even for handles not managed by the DataReader. One practical situation
     * where this may occur is when an application is iterating through all the instances, takes all the samples of a
     * @ref NOT_ALIVE_NO_WRITERS_INSTANCE_STATE instance, returns the loan (at which point the instance information
     * may be removed, and thus the handle becomes invalid), and tries to read the next instance.
     *
     * The behavior of this operation follows the same rules as the @ref read operation regarding the pre-conditions and
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
     * @param [in]     previous_handle The 'next smallest' instance with a value greater than this value that has
     *                                 available samples will be returned.
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t read_next_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& previous_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * This operation accesses a collection of Data values from the DataReader. The behavior is identical to
     * @ref read_next_instance except that all samples returned satisfy the specified condition. In other words, on
     * success all returned samples belong to the same instance, and the instance is the instance with
     * ‘smallest’ @c instance_handle among the ones that verify (a) @c instance_handle >= @c previous_handle and (b) have samples
     * for which the specified ReadCondition evaluates to TRUE.
     *
     * Similar to the operation @ref read_next_instance it is possible to call
     * @ref read_next_instance_w_condition with a @c previous_handle that does not correspond to an instance currently
     * managed by the DataReader.
     *
     * The behavior of the @ref read_next_instance_w_condition operation follows the same rules than the read operation
     * regarding the pre-conditions and post-conditions for the @c data_values and @c sample_infos collections. Similar
     * to read, the @ref read_next_instance_w_condition operation may ‘loan’ elements to the output collections which
     * must then be returned by means of @ref return_loan.
     *
     * If the DataReader has no samples that meet the constraints, the return value will be RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                available, up to the limits described in the documentation for @ref read().
     * @param [in]     previous_handle The 'next smallest' instance with a value greater than this value that has
     *                                available samples will be returned.
     * @param [in]     a_condition     A ReadCondition that returned @c data_values must pass
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t read_next_instance_w_condition(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            const InstanceHandle_t& previous_handle,
            ReadCondition* a_condition);

    /**
     * @brief This operation copies the next, non-previously accessed Data value from the DataReader; the operation
     * also copies the corresponding SampleInfo. The implied order among the samples stored in the DataReader is the
     * same as for the read operation.
     *
     * The read_next_sample operation is semantically equivalent to the read operation where the input Data sequence
     * has <tt> max_length = 1 </tt>, the <tt> sample_states = NOT_READ_SAMPLE_STATE </tt>,
     * the <tt> view_states = ANY_VIEW_STATE </tt>, and the <tt> instance_states = ANY_INSTANCE_STATE </tt>.
     *
     * The read_next_sample operation provides a simplified API to ‘read’ samples avoiding the need for the
     * application to manage sequences and specify states.
     *
     * If there is no unread data in the DataReader, the operation will return RETCODE_NO_DATA and nothing is copied
     *
     * @param [out] data Data pointer to store the sample
     * @param [out] info SampleInfo pointer to store the sample information
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t read_next_sample(
            void* data,
            SampleInfo* info);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data-samples from the DataReader and a corresponding collection of
     * SampleInfo structures, and 'removes' them from the DataReader. The operation will return either a 'list' of
     * samples or else a single sample. This is controlled by the @ref PresentationQosPolicy using the same logic
     * as for the @ref read operation.
     *
     * The act of taking a sample removes it from the DataReader so it cannot be 'read' or 'taken' again. If the
     * sample belongs to the most recent generation of the instance, it will also set the @c view_state of the
     * instance to NOT_NEW. It will not affect the @c instance_state of the instance.
     *
     * The behavior of the take operation follows the same rules than the @ref read operation regarding the
     * pre-conditions and post-conditions for the @c data_values and @c sample_infos collections. Similar to
     * @ref read, the take operation may 'loan' elements to the output collections which must then be returned by
     * means of @ref return_loan. The only difference with @ref read is that, as stated, the samples returned by
     * take will no longer be accessible to successive calls to read or take.
     *
     * If the DataReader has no samples that meet the constraints, the operations fails with RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                 @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                 available, up to the limits described in the documentation for @ref read().
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * This operation is analogous to @ref read_w_condition except it accesses samples via the ‘take’ operation.
     *
     * The specified ReadCondition must be attached to the DataReader; otherwise the operation will fail and return
     * RETCODE_PRECONDITION_NOT_MET.
     *
     * The samples are accessed with the same semantics as the @ref take operation.
     *
     * This operation is especially useful in combination with QueryCondition to filter data samples based on the
     * content.
     *
     * If the DataReader has no samples that meet the constraints, the return value will be RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are.
     * @param [in]     a_condition     A ReadCondition that returned @c data_values must pass
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take_w_condition(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            ReadCondition* a_condition);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data values from the DataReader and 'removes' them from the DataReader.
     *
     * This operation has the same behavior as @ref read_instance, except that the samples are 'taken' from the
     * DataReader such that they are no longer accessible via subsequent 'read' or 'take' operations.
     *
     * The behavior of this operation follows the same rules as the @ref read operation regarding the pre-conditions and
     * post-conditions for the @c data_values and @c sample_infos. Similar to @ref read, this operation may 'loan'
     * elements to the output collections, which must then be returned by means of @ref return_loan.
     *
     * If the DataReader has no samples that meet the constraints, the operations fails with RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                available, up to the limits described in the documentation for @ref read().
     * @param [in]     a_handle        The specified instance to return samples for. The method will fail with
     *                                RETCODE_BAD_PARAMETER if the handle does not correspond to an existing
     *                                data-object known to the DataReader.
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& a_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * Access a collection of data samples from the DataReader.
     *
     * This operation accesses a collection of data values from the DataReader and 'removes' them from the DataReader.
     *
     * This operation has the same behavior as @ref read_next_instance, except that the samples are 'taken' from the
     * DataReader such that they are no longer accessible via subsequent 'read' or 'take' operations.
     *
     * Similar to the operation @ref read_next_instance, it is possible to call this operation with a
     * @c previous_handle that does not correspond to an instance currently managed by the DataReader.
     *
     * The behavior of this operation follows the same rules as the @ref read operation regarding the pre-conditions and
     * post-conditions for the @c data_values and @c sample_infos. Similar to @ref read, this operation may 'loan'
     * elements to the output collections, which must then be returned by means of @ref return_loan.
     *
     * If the DataReader has no samples that meet the constraints, the operations fails with RETCODE_NO_DATA.
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                available, up to the limits described in the documentation for @ref read().
     * @param [in]     previous_handle The 'next smallest' instance with a value greater than this value that has
     *                                available samples will be returned.
     * @param [in]     sample_states   Only data samples with @c sample_state matching one of these will be returned.
     * @param [in]     view_states     Only data samples with @c view_state matching one of these will be returned.
     * @param [in]     instance_states Only data samples with @c instance_state matching one of these will be returned.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take_next_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& previous_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    /**
     * This operation accesses a collection of Data values from the DataReader. The behavior is identical to
     * @ref read_next_instance except that all samples returned satisfy the specified condition. In other words, on
     * success all returned samples belong to the same instance, and the instance is the instance with ‘smallest’
     * @c instance_handle among the ones that verify (a) @c instance_handle >= @c previous_handle and (b) have
     * samples for which the specified ReadCondition evaluates to TRUE.
     *
     * Similar to the operation @ref read_next_instance it is possible to call @ref read_next_instance_w_condition with
     * a @c previous_handle that does not correspond to an instance currently managed by the DataReader.
     *
     * The behavior of the @ref read_next_instance_w_condition operation follows the same rules than the read operation
     * regarding the pre-conditions and post-conditions for the @c data_values and @c sample_infos collections. Similar
     * to read, the @ref read_next_instance_w_condition operation may ‘loan’ elements to the output collections which
     * must then be returned by means of @ref return_loan.
     *
     * If the DataReader has no samples that meet the constraints, the return value will be RETCODE_NO_DATA
     *
     * @param [in,out] data_values     A LoanableCollection object where the received data samples will be returned.
     * @param [in,out] sample_infos    A SampleInfoSeq object where the received sample info will be returned.
     * @param [in]     max_samples     The maximum number of samples to be returned. If the special value
     *                                @ref LENGTH_UNLIMITED is provided, as many samples will be returned as are
     *                                available, up to the limits described in the documentation for @ref read().
     * @param [in]     previous_handle The 'next smallest' instance with a value greater than this value that has
     *                                available samples will be returned.
     * @param [in]     a_condition     A ReadCondition that returned @c data_values must pass
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take_next_instance_w_condition(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            const InstanceHandle_t& previous_handle,
            ReadCondition* a_condition);

    /**
     * @brief This operation copies the next, non-previously accessed Data value from the DataReader and ‘removes’ it
     * from the DataReader so it is no longer accessible. The operation also copies the corresponding SampleInfo.
     *
     * This operation is analogous to @ref read_next_sample except for the fact that the sample is ‘removed’ from the
     * DataReader.
     *
     * This operation is semantically equivalent to the @ref take operation where the input sequence has
     * <tt> max_length = 1 </tt>, the <tt> sample_states = NOT_READ_SAMPLE_STATE </tt>, the
     * <tt> view_states = ANY_VIEW_STATE </tt>, and the <tt> instance_states = ANY_INSTANCE_STATE </tt>.
     *
     * This operation provides a simplified API to ’take’ samples avoiding the need for the application to manage
     * sequences and specify states.
     *
     * If there is no unread data in the DataReader, the operation will return RETCODE_NO_DATA and nothing is copied.
     *
     * @param [out] data Data pointer to store the sample
     * @param [out] info SampleInfo pointer to store the sample information
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t take_next_sample(
            void* data,
            SampleInfo* info);

    ///@}

    /**
     * This operation indicates to the DataReader that the application is done accessing the collection of
     * @c data_values and @c sample_infos obtained by some earlier invocation of @ref read or @ref take on the
     * DataReader.
     *
     * The @c data_values and @c sample_infos must belong to a single related ‘pair’; that is, they should correspond
     * to a pair returned from a single call to read or take. The @c data_values and @c sample_infos must also have
     * been obtained from the same DataReader to which they are returned. If either of these conditions is not met,
     * the operation will fail and return RETCODE_PRECONDITION_NOT_MET.
     *
     * This operation allows implementations of the @ref read and @ref take operations to "loan" buffers from the
     * DataReader to the application and in this manner provide "zero-copy" access to the data. During the loan, the
     * DataReader will guarantee that the data and sample-information are not modified.
     *
     * It is not necessary for an application to return the loans immediately after the read or take calls. However,
     * as these buffers correspond to internal resources inside the DataReader, the application should not retain them
     * indefinitely.
     *
     * The use of the @ref return_loan operation is only necessary if the read or take calls "loaned" buffers to the
     * application. This only occurs if the @c data_values and @c sample_infos collections had <tt> max_len == 0 </tt>
     * at the time read or take was called. The application may also examine the @c has_ownership property of the
     * collection to determine if there is an outstanding loan. However, calling @ref return_loan on a collection that
     * does not have a loan is safe, has no side effects, and returns RETCODE_OK.
     *
     * If the collections had a loan, upon return from return_loan the collections will have <tt> max_len == 0 </tt>.
     *
     * @param [in,out] data_values   A LoanableCollection object where the received data samples were obtained from
     *                               an earlier invocation of read or take on this DataReader.
     * @param [in,out] sample_infos  A SampleInfoSeq object where the received sample infos were obtained from
     *                               an earlier invocation of read or take on this DataReader.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t return_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos);

    /**
     * NOT YET IMPLEMENTED
     *
     * This operation can be used to retrieve the instance key that corresponds to an @c instance_handle. The operation
     * will only fill the fields that form the key inside the key_holder instance.
     *
     * This operation may return BAD_PARAMETER if the InstanceHandle_t a_handle does not correspond to an existing
     * data-object known to the DataReader. If the implementation is not able to check invalid handles then the result
     * in this situation is unspecified.
     *
     * @param [in,out] key_holder
     * @param [in] handle
     *
     * @return Any of the standard return codes.
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_key_value(
            void* key_holder,
            const InstanceHandle_t& handle);

    /**
     * Takes as a parameter an instance and returns a handle that can be used in subsequent operations that accept an
     * instance handle as an argument. The instance parameter is only used for the purpose of examining the fields that
     * define the key.
     *
     * @param [in] instance Data pointer to the sample
     *
     * @return handle of the given @c instance.
     * @return HANDLE_NIL if @c instance is nullptr.
     * @return HANDLE_NIL if there is no instance on the DataReader's history with the same key as @c instance.
     */
    FASTDDS_EXPORTED_API InstanceHandle_t lookup_instance(
            const void* instance) const;

    /**
     * @brief Returns information about the first untaken sample. This method is meant to be called prior to
     * a read() or take() operation as it does not modify the status condition of the entity.
     *
     *
     * @param [out] info Pointer to a SampleInfo_t structure to store first untaken sample information.
     *
     * @return RETCODE_OK if sample info was returned. RETCODE_NO_DATA if there is no sample to take.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_first_untaken_info(
            SampleInfo* info);

    /**
     * Get the number of samples pending to be read.
     * The number includes samples that may not yet be available to be read or taken by the user, due to samples
     * being received out of order.
     *
     * @return the number of samples on the reader history that have never been read.
     */
    FASTDDS_EXPORTED_API uint64_t get_unread_count() const;

    /**
     * Get the number of samples pending to be read.
     *
     * @param mark_as_read  Whether the unread samples should be marked as read or not.
     *
     * @return the number of samples on the reader history that have never been read.
     */
    FASTDDS_EXPORTED_API uint64_t get_unread_count(
            bool mark_as_read) const;

    /**
     * Get associated GUID.
     *
     * @return Associated GUID
     */
    FASTDDS_EXPORTED_API const fastdds::rtps::GUID_t& guid();

    /**
     * Get associated GUID.
     *
     * @return Associated GUID
     */
    FASTDDS_EXPORTED_API const fastdds::rtps::GUID_t& guid() const;

    /**
     * @brief Getter for the associated InstanceHandle.
     *
     * @return Copy of the InstanceHandle
     */
    FASTDDS_EXPORTED_API InstanceHandle_t get_instance_handle() const;

    /**
     * Getter for the data type.
     *
     * @return Copy of the TypeSupport associated to the DataReader.
     */
    FASTDDS_EXPORTED_API TypeSupport type() const;

    /**
     * Get TopicDescription.
     *
     * @return TopicDescription pointer.
     */
    FASTDDS_EXPORTED_API const TopicDescription* get_topicdescription() const;

    /**
     * @brief Get the requested deadline missed status.
     *
     * @return The deadline missed status.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    /**
     * @brief Get the requested incompatible qos status.
     *
     * @param [out] status Requested incompatible qos status.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status);

    /**
     * @brief Setter for the DataReaderQos.
     *
     * @param [in] qos new value for the DataReaderQos.
     *
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is
     *         not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const DataReaderQos& qos);

    /**
     * @brief Getter for the DataReaderQos.
     *
     * @return Pointer to the DataReaderQos.
     */
    FASTDDS_EXPORTED_API const DataReaderQos& get_qos() const;

    /**
     * @brief Getter for the DataReaderQos.
     *
     * @param [in] qos DataReaderQos where the qos is returned.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            DataReaderQos& qos) const;

    /**
     * Modifies the DataReaderListener, sets the mask to StatusMask::all().
     *
     * @param [in] listener new value for the DataReaderListener.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DataReaderListener* listener);

    /**
     * Modifies the DataReaderListener.
     *
     * @param [in] listener new value for the DataReaderListener.
     * @param [in] mask StatusMask that holds statuses the listener responds to (default: all).
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DataReaderListener* listener,
            const StatusMask& mask);
    /**
     * @brief Getter for the DataReaderListener
     *
     * @return Pointer to the DataReaderListener
     */
    FASTDDS_EXPORTED_API const DataReaderListener* get_listener() const;

    /* TODO
       FASTDDS_EXPORTED_API bool get_key_value(
            void* data,
            const InstanceHandle_t& handle);
     */

    /**
     * @brief Get the liveliness changed status.
     *
     * @param [out] status LivelinessChangedStatus object where the status is returned.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status) const;


    /**
     * @brief Get the SAMPLE_LOST communication status
     *
     * @param [out] status SampleLostStatus object where the status is returned.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_sample_lost_status(
            SampleLostStatus& status) const;

    /**
     * @brief Get the SAMPLE_REJECTED communication status
     *
     * @param [out] status SampleRejectedStatus object where the status is returned.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_sample_rejected_status(
            SampleRejectedStatus& status) const;

    /**
     * @brief Returns the subscription matched status
     *
     * @param [out] status subscription matched status struct
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_subscription_matched_status(
            SubscriptionMatchedStatus& status) const;

    /**
     * @brief Retrieves in a publication associated with the DataWriter
     *
     * @param [out] publication_data publication data struct
     * @param publication_handle InstanceHandle_t of the publication
     * @return RETCODE_OK
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_matched_publication_data(
            PublicationBuiltinTopicData& publication_data,
            const fastdds::rtps::InstanceHandle_t& publication_handle) const;

    /**
     * @brief Fills the given vector with the InstanceHandle_t of matched DataReaders
     *
     * @param [out] publication_handles Vector where the InstanceHandle_t are returned
     * @return RETCODE_OK
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_matched_publications(
            std::vector<InstanceHandle_t>& publication_handles) const;

    /**
     * @brief This operation creates a ReadCondition. The returned ReadCondition will be attached and belong to the
     * DataReader.
     *
     * @param [in] sample_states   Only data samples with @c sample_state matching one of these will trigger the created condition.
     * @param [in] view_states     Only data samples with @c view_state matching one of these will trigger the created condition.
     * @param [in] instance_states Only data samples with @c instance_state matching one of these will trigger the created condition.
     *
     * @return pointer to the created ReadCondition, nullptr in case of error.
     */
    FASTDDS_EXPORTED_API ReadCondition* create_readcondition(
            SampleStateMask sample_states,
            ViewStateMask view_states,
            InstanceStateMask instance_states);

    /**
     * @brief This operation creates a QueryCondition. The returned QueryCondition will be attached and belong to the
     * DataReader.
     *
     * @param [in] sample_states    Only data samples with @c sample_state matching one of these will trigger the created condition.
     * @param [in] view_states      Only data samples with @c view_state matching one of these will trigger the created condition.
     * @param [in] instance_states  Only data samples with @c instance_state matching one of these will trigger the created condition.
     * @param [in] query_expression Only data samples matching this query will trigger the created condition.
     * @param [in] query_parameters Value of the parameters on the query expression.
     *
     * @return pointer to the created QueryCondition, nullptr in case of error.
     */
    FASTDDS_EXPORTED_API QueryCondition* create_querycondition(
            SampleStateMask sample_states,
            ViewStateMask view_states,
            InstanceStateMask instance_states,
            const std::string& query_expression,
            const std::vector<std::string>& query_parameters);

    /**
     * @brief This operation deletes a ReadCondition attached to the DataReader.
     *
     * @param a_condition pointer to a ReadCondition belonging to the DataReader
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_readcondition(
            ReadCondition* a_condition);

    /**
     * @brief Getter for the Subscriber
     * @return Subscriber pointer
     */
    FASTDDS_EXPORTED_API const Subscriber* get_subscriber() const;

    /**
     * This operation deletes all the entities that were created by means of the “create” operations on the DataReader.
     * That is, it deletes all contained ReadCondition and QueryCondition objects.
     *
     * The operation will return PRECONDITION_NOT_MET if the any of the contained entities is in a state where it cannot
     * be deleted.
     *
     * @return Any of the standard return codes.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_contained_entities();

    /**
     * Checks whether a loaned sample is still valid or is corrupted.
     * Calling this method on a sample which has not been loaned, or one for which the loan has been returned
     * yields undefined behavior.
     *
     * @param data Pointer to the sample data to check
     * @param info Pointer to the SampleInfo related to \c data
     *
     * @return true if the sample is valid
     */
    FASTDDS_EXPORTED_API bool is_sample_valid(
            const void* data,
            const SampleInfo* info) const;

    /**
     * Get the list of locators on which this DataReader is listening.
     *
     * @param [out] locators  LocatorList where the list of locators will be stored.
     *
     * @return NOT_ENABLED if the reader has not been enabled.
     * @return OK if a list of locators is returned.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_listening_locators(
            rtps::LocatorList& locators) const;

    /**
     * Retrieve the subscription data discovery information.
     *
     * @param [out] subscription_data The subscription data discovery information.
     *
     * @return NOT_ENABLED if the reader has not been enabled.
     * @return OK if the subscription data is returned.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_subscription_builtin_topic_data(
            SubscriptionBuiltinTopicData& subscription_data) const;

protected:

    DataReaderImpl* impl_;

    friend class ::dds::sub::DataReader;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER__DATAREADER_HPP
