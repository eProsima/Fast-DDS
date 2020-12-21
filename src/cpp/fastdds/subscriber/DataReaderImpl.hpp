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
 * @file DataReaderImpl.hpp
 *
 */

#ifndef _FASTRTPS_DATAREADERIMPL_HPP_
#define _FASTRTPS_DATAREADERIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/reader/ReaderListener.h>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastrtps/types/TypesBase.h>

#include <rtps/history/ITopicPayloadPool.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;
class TimedEvent;

} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class TopicDescription;

using SampleInfoSeq = LoanableSequence<SampleInfo>;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReaderImpl
{
protected:

    using ITopicPayloadPool = eprosima::fastrtps::rtps::ITopicPayloadPool;
    using IPayloadPool = eprosima::fastrtps::rtps::IPayloadPool;

    friend class SubscriberImpl;

    /**
     * Creates a DataReader. Don't use it directly, but through Subscriber.
     */
    DataReaderImpl(
            SubscriberImpl* s,
            TypeSupport& type,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr);

public:

    virtual ~DataReaderImpl();

    ReturnCode_t enable();

    /**
     * Method to block the current thread until an unread message is available
     */
    bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    ReturnCode_t read(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t read_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& a_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t read_next_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& previous_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t read_next_sample(
            void* data,
            SampleInfo* info);

    ReturnCode_t take(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t take_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& a_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t take_next_instance(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            const InstanceHandle_t& previous_handle = HANDLE_NIL,
            SampleStateMask sample_states = ANY_SAMPLE_STATE,
            ViewStateMask view_states = ANY_VIEW_STATE,
            InstanceStateMask instance_states = ANY_INSTANCE_STATE);

    ReturnCode_t take_next_sample(
            void* data,
            SampleInfo* info);

    ///@}

    ReturnCode_t return_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos);

    /**
     * @brief Returns information about the first untaken sample.
     * @param [out] info Pointer to a SampleInfo structure to store first untaken sample information.
     * @return true if sample info was returned. false if there is no sample to take.
     */
    ReturnCode_t get_first_untaken_info(
            SampleInfo* info);

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    const fastrtps::rtps::GUID_t& guid() const;

    fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport type();

    /**
     * Get TopicDescription
     * @return TopicDescription
     */
    const TopicDescription* get_topicdescription() const;

    ReturnCode_t get_requested_deadline_missed_status(
            fastrtps::RequestedDeadlineMissedStatus& status);

    ReturnCode_t set_qos(
            const DataReaderQos& qos);

    const DataReaderQos& get_qos() const;

    ReturnCode_t set_listener(
            DataReaderListener* listener);

    const DataReaderListener* get_listener() const;

    /* TODO
       bool get_key_value(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t get_liveliness_changed_status(
            fastrtps::LivelinessChangedStatus& status);

    ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status);

    /* TODO
       bool get_sample_lost_status(
            fastrtps::SampleLostStatus& status) const;
     */

    /* TODO
       bool get_sample_rejected_status(
            fastrtps::SampleRejectedStatus& status) const;
     */

    const Subscriber* get_subscriber() const;

    /* TODO
       bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
     */

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    /* Check whether values in the DataReaderQos are compatible among them or not
     * @return True if correct.
     */
    static ReturnCode_t check_qos (
            const DataReaderQos& qos);

    /* Check whether the DataReaderQos can be updated with the values provided. This method DOES NOT update anything.
     * @param to Reference to the qos instance to be changed.
     * @param from Reference to the qos instance with the new values.
     * @return True if they can be updated.
     */
    static bool can_qos_be_updated(
            const DataReaderQos& to,
            const DataReaderQos& from);

    /* Update a DataReaderQos with new values
     * @param to Reference to the qos instance to be changed.
     * @param from Reference to the qos instance with the new values.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    static void set_qos(
            DataReaderQos& to,
            const DataReaderQos& from,
            bool first_time);

protected:

    //!Subscriber
    SubscriberImpl* subscriber_ = nullptr;

    //!Pointer to associated RTPSReader
    fastrtps::rtps::RTPSReader* reader_ = nullptr;

    //! Pointer to the TopicDataType object.
    TypeSupport type_;

    TopicDescription* topic_ = nullptr;

    DataReaderQos qos_;

    //!History
    fastrtps::SubscriberHistory history_;

    //!Listener
    DataReaderListener* listener_ = nullptr;

    class InnerDataReaderListener : public fastrtps::rtps::ReaderListener
    {
    public:

        InnerDataReaderListener(
                DataReaderImpl* s)
            : data_reader_(s)
        {
        }

        virtual ~InnerDataReaderListener() override
        {
        }

        void onReaderMatched(
                fastrtps::rtps::RTPSReader* reader,
                const SubscriptionMatchedStatus& info) override;

        void onNewCacheChangeAdded(
                fastrtps::rtps::RTPSReader* reader,
                const fastrtps::rtps::CacheChange_t* const change) override;

        void on_liveliness_changed(
                fastrtps::rtps::RTPSReader* reader,
                const fastrtps::LivelinessChangedStatus& status) override;

        void on_requested_incompatible_qos(
                fastrtps::rtps::RTPSReader* reader,
                fastdds::dds::PolicyMask qos) override;

        DataReaderImpl* data_reader_;
    }
    reader_listener_;

    //! A timer used to check for deadlines
    fastrtps::rtps::TimedEvent* deadline_timer_ = nullptr;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    fastrtps::rtps::InstanceHandle_t timer_owner_;

    //! Liveliness changed status
    LivelinessChangedStatus liveliness_changed_status_;

    //! Requested deadline missed status
    fastrtps::RequestedDeadlineMissedStatus deadline_missed_status_;

    //! Requested incompatible QoS status
    RequestedIncompatibleQosStatus requested_incompatible_qos_status_;

    //! A timed callback to remove expired samples
    fastrtps::rtps::TimedEvent* lifespan_timer_ = nullptr;

    //! The lifespan duration
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    DataReader* user_datareader_ = nullptr;

    std::shared_ptr<ITopicPayloadPool> payload_pool_;

    ReturnCode_t check_collection_preconditions_and_calc_max_samples(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t& max_samples);

    ReturnCode_t prepare_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t& max_samples);

    /**
     * @brief A method called when a new cache change is added
     * @param change The cache change that has been added
     * @return True if the change was added (due to some QoS it could have been 'rejected')
     */
    bool on_new_cache_change_added(
            const fastrtps::rtps::CacheChange_t* const change);

    /**
     * @brief Method called when an instance misses the deadline
     */
    bool deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    bool deadline_timer_reschedule();

    /**
     * @brief A method called when the lifespan timer expires
     */
    bool lifespan_expired();

    fastrtps::TopicAttributes topic_attributes() const;

    void subscriber_qos_updated();

    RequestedIncompatibleQosStatus& update_requested_incompatible_qos(
            PolicyMask incompatible_policies);

    LivelinessChangedStatus& update_liveliness_status(
            const fastrtps::LivelinessChangedStatus& status);

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    DataReaderListener* get_listener_for(
            const StatusMask& status);

    std::shared_ptr<IPayloadPool> get_payload_pool();

    void release_payload_pool();

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTRTPS_DATAREADERIMPL_HPP_*/
