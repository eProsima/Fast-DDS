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

#ifndef _FASTRTPS_DATAREADER_HPP_
#define _FASTRTPS_DATAREADER_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastrtps/types/TypesBase.h>


#include <vector>
#include <cstdint>

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
struct InstanceHandle_t;
} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class TypeSupport;
class DataReaderQos;
class TopicDescription;
struct LivelinessChangedStatus;
struct SampleInfo;

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

    /* TODO
       RTPS_DllAPI bool read(
            std::vector<void*>& data_values,
            std::vector<SampleInfo>& sample_infos,
            uint32_t max_samples);
     */

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
     * Get associated GUID
     * @return Associated GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid() const;

    /**
     * @brief Getter for the associated InstanceHandle
     * @return Copy of the InstanceHandle
     */
    RTPS_DllAPI fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

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
            fastrtps::RequestedDeadlineMissedStatus& status);

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
            const fastrtps::rtps::InstanceHandle_t& handle);
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
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTRTPS_DATAREADER_HPP_*/
