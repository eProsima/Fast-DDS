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

#ifndef FASTDDS_SUBSCRIBER__DATAREADERIMPL_HPP
#define FASTDDS_SUBSCRIBER__DATAREADERIMPL_HPP

#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <fastdds/subscriber/DataReaderImpl/DataReaderLoanManager.hpp>
#include <fastdds/subscriber/DataReaderImpl/SampleInfoPool.hpp>
#include <fastdds/subscriber/DataReaderImpl/SampleLoanManager.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>
#include <fastdds/subscriber/history/DataReaderHistory.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <rtps/history/ITopicPayloadPool.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSReader;
class TimedEvent;

} // namespace rtps

namespace dds {

class ContentFilteredTopicImpl;
class Subscriber;
class SubscriberImpl;
class TopicDescription;

using SampleInfoSeq = LoanableSequence<SampleInfo>;

namespace detail {

struct ReadTakeCommand;
class ReadConditionImpl;

} // namespace detail

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReaderImpl
{
    friend struct detail::ReadTakeCommand;
    friend class detail::ReadConditionImpl;

protected:

    using ITopicPayloadPool = eprosima::fastdds::rtps::ITopicPayloadPool;
    using IPayloadPool = eprosima::fastdds::rtps::IPayloadPool;

    friend class SubscriberImpl;

    /**
     * Creates a DataReader. Don't use it directly, but through Subscriber.
     */
    DataReaderImpl(
            SubscriberImpl* s,
            const TypeSupport& type,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

public:

    virtual ~DataReaderImpl();

    virtual ReturnCode_t enable();

    /**
     * Method to check if a DataReader can be deleted
     * @param recursive == true if is used from delete_contained_entities otherwise delete_datareader
     * @return true if can be deleted according to the standard rules
     */
    bool can_be_deleted(
            bool recursive = true) const;

    /**
     * Method to block the current thread until an unread message is available
     */
    bool wait_for_unread_message(
            const fastdds::dds::Duration_t& timeout);


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
     * @brief Returns information about the first untaken sample. This method is meant to be called prior to
     * a read() or take() operation as it does not modify the status condition of the entity.
     * @param [out] info Pointer to a SampleInfo structure to store first untaken sample information.
     * @return true if sample info was returned. false if there is no sample to take.
     */
    ReturnCode_t get_first_untaken_info(
            SampleInfo* info);

    /**
     * Get the number of samples pending to be read.
     *
     * @param mark_as_read  Whether the unread samples should be marked as read or not.
     *
     * @return the number of samples on the reader history that have never been read.
     */
    uint64_t get_unread_count(
            bool mark_as_read);

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    const fastdds::rtps::GUID_t& guid() const;

    fastdds::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport type() const;

    /**
     * Get TopicDescription
     * @return TopicDescription
     */
    const TopicDescription* get_topicdescription() const;

    ReturnCode_t get_subscription_matched_status(
            SubscriptionMatchedStatus& status);

    /**
     * @brief Retrieves in a publication associated with the DataWriter
     *
     * @param[out] publication_data publication data struct
     * @param publication_handle @ref InstanceHandle_t of the publication
     * @return @ref RETCODE_BAD_PARAMETER if the DataReader is not matched with
     * the given publication handle, @ref RETCODE_OK otherwise.
     *
     */
    ReturnCode_t get_matched_publication_data(
            rtps::PublicationBuiltinTopicData& publication_data,
            const InstanceHandle_t& publication_handle) const;

    /**
     * @brief Fills the given vector with the @ref InstanceHandle_t of matched DataReaders
     *
     * @param[out] publication_handles Vector where the @ref InstanceHandle_t are returned
     * @return @ref RETCODE_OK if the operation succeeds.
     *
     * @note Returning an empty list is not an error, it returns @ref RETCODE_OK.
     *
     */
    ReturnCode_t get_matched_publications(
            std::vector<InstanceHandle_t>& publication_handles) const;

    ReturnCode_t get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    ReturnCode_t set_qos(
            const DataReaderQos& qos);

    const DataReaderQos& get_qos() const;

    ReturnCode_t set_listener(
            DataReaderListener* listener);

    const DataReaderListener* get_listener() const;

    /* TODO
       bool get_key_value(
            void* data,
            const fastdds::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status);

    ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status);

    /*!
     * @brief Get the SAMPLE_LOST communication status
     *
     * @param [out] status SampleLostStatus object where the status is returned.
     *
     * @return RETCODE_OK
     */
    ReturnCode_t get_sample_lost_status(
            fastdds::dds::SampleLostStatus& status);

    /*!
     * @brief Get the SAMPLE_REJECTED communication status
     *
     * @param [out] status SampleRejectedStatus object where the status is returned.
     *
     * @return RETCODE_OK
     */
    ReturnCode_t get_sample_rejected_status(
            SampleRejectedStatus& status);

    const Subscriber* get_subscriber() const;

    /* TODO
       bool wait_for_historical_data(
       const fastdds::dds::Duration_t& max_wait) const;
     */

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    virtual void disable();

    /* Extends the check_qos() call, including the check for
     * resource limits policy.
     * @param qos Pointer to the qos to be checked.
     * @param type Pointer to the associated TypeSupport object.
     * @return True if correct.
     */
    static ReturnCode_t check_qos_including_resource_limits(
            const DataReaderQos& qos,
            const TypeSupport& type);

    /* Check whether values in the DataReaderQos are compatible among them or not
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_qos (
            const DataReaderQos& qos);

    /* Checks resource limits policy: Instance allocation consistency
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_allocation_consistency(
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

    /**
     * Checks whether the sample is still valid or is corrupted
     * @param data Pointer to the sample data to check
     * @param info Pointer to the SampleInfo related to \c data
     * @return true if the sample is valid
     */
    bool is_sample_valid(
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
    ReturnCode_t get_listening_locators(
            rtps::LocatorList& locators) const;

    ReturnCode_t delete_contained_entities();

    void filter_has_been_updated();

    InstanceHandle_t lookup_instance(
            const void* instance) const;

    ReadCondition* create_readcondition(
            SampleStateMask sample_states,
            ViewStateMask view_states,
            InstanceStateMask instance_states) noexcept;

    ReturnCode_t delete_readcondition(
            ReadCondition* a_condition) noexcept;

    const detail::StateFilter& get_last_mask_state() const;

    void try_notify_read_conditions() noexcept;

    std::recursive_mutex& get_conditions_mutex() const noexcept;

    /**
     * Retrieve the subscription data discovery information.
     *
     * @param [out] subscription_data The subscription data discovery information.
     *
     * @return NOT_ENABLED if the reader has not been enabled.
     * @return OK if the subscription data is returned.
     */
    ReturnCode_t get_subscription_builtin_topic_data(
            SubscriptionBuiltinTopicData& subscription_data) const;

protected:

    //!Subscriber
    SubscriberImpl* subscriber_ = nullptr;

    //!Pointer to associated RTPSReader
    fastdds::rtps::RTPSReader* reader_ = nullptr;

    //! Pointer to the TopicDataType object.
    TypeSupport type_;

    TopicDescription* topic_ = nullptr;

    DataReaderQos qos_;

    //!History
    detail::DataReaderHistory history_;

    //!Listener
    DataReaderListener* listener_ = nullptr;
    mutable std::mutex listener_mutex_;

    fastdds::rtps::GUID_t guid_;

    class InnerDataReaderListener : public fastdds::rtps::ReaderListener
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

        void on_reader_matched(
                fastdds::rtps::RTPSReader* reader,
                const fastdds::rtps::MatchingInfo& info) override;

        void on_data_available(
                fastdds::rtps::RTPSReader* reader,
                const fastdds::rtps::GUID_t& writer_guid,
                const fastdds::rtps::SequenceNumber_t& first_sequence,
                const fastdds::rtps::SequenceNumber_t& last_sequence,
                bool& should_notify_individual_changes) override;

        void on_liveliness_changed(
                fastdds::rtps::RTPSReader* reader,
                const LivelinessChangedStatus& status) override;

        void on_requested_incompatible_qos(
                fastdds::rtps::RTPSReader* reader,
                fastdds::dds::PolicyMask qos) override;

        void on_sample_lost(
                fastdds::rtps::RTPSReader* reader,
                int32_t sample_lost_since_last_update) override;

        void on_sample_rejected(
                fastdds::rtps::RTPSReader* reader,
                SampleRejectedStatusKind reason,
                const fastdds::rtps::CacheChange_t* const change) override;

#ifdef FASTDDS_STATISTICS
        void notify_status_observer(
                const uint32_t& status_id);
#endif //FASTDDS_STATISTICS

        DataReaderImpl* data_reader_;

    }
    reader_listener_;

    //! A timer used to check for deadlines
    fastdds::rtps::TimedEvent* deadline_timer_ = nullptr;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    fastdds::rtps::InstanceHandle_t timer_owner_;

    //! Subscription matched status
    SubscriptionMatchedStatus subscription_matched_status_;

    //! Liveliness changed status
    LivelinessChangedStatus liveliness_changed_status_;

    //! Requested deadline missed status
    RequestedDeadlineMissedStatus deadline_missed_status_;

    //! Requested incompatible QoS status
    RequestedIncompatibleQosStatus requested_incompatible_qos_status_;

    //! Sample lost status
    SampleLostStatus sample_lost_status_;
    //! Sample rejected status
    SampleRejectedStatus sample_rejected_status_;

    //! A timed callback to remove expired samples
    fastdds::rtps::TimedEvent* lifespan_timer_ = nullptr;

    //! The lifespan duration
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    DataReader* user_datareader_ = nullptr;

    std::shared_ptr<detail::SampleLoanManager> sample_pool_;
    std::shared_ptr<IPayloadPool> payload_pool_;

    bool is_custom_payload_pool_ = false;

    detail::SampleInfoPool sample_info_pool_;
    detail::DataReaderLoanManager loan_manager_;

    /**
     * Mutex to protect ReadCondition collection
     * is required because the RTPSReader mutex is only available when the object is enabled
     * @note use get_conditions_mutex() instead of directly referencing it
     * @note lock get_conditions_mutex() after lock reader_->getMutex() to avoid ABBAs because
     *       try_notify_read_conditions() will be called from the callbacks with the reader
     *       mutex locked
     */
    mutable std::recursive_mutex conditions_mutex_;

    // Order for the ReadCondition collection
    struct ReadConditionOrder
    {
        using is_transparent = void;

        bool operator ()(
                const detail::ReadConditionImpl* lhs,
                const detail::ReadConditionImpl* rhs) const;
        bool operator ()(
                const detail::ReadConditionImpl* lhs,
                const detail::StateFilter& rhs) const;
        bool operator ()(
                const detail::StateFilter& lhs,
                const detail::ReadConditionImpl* rhs) const;

        template<class S, class V, class I>
        static inline bool less(
                S&& s1,
                V&& v1,
                I&& i1,
                S&& s2,
                V&& v2,
                I&& i2)
        {
            return s1 < s2 || (s1 == s2 && (v1 < v2 || (v1 == v2 && i1 < i2)));
        }

    };

    // ReadConditions collection
    std::set<detail::ReadConditionImpl*, ReadConditionOrder> read_conditions_;

    // State of the History mask last time it was queried
    // protected with the RTPSReader mutex
    detail::StateFilter last_mask_state_ {};

    ReturnCode_t check_collection_preconditions_and_calc_max_samples(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t& max_samples);

    ReturnCode_t prepare_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t& max_samples);

    ReturnCode_t read_or_take(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            const InstanceHandle_t& handle,
            SampleStateMask sample_states,
            ViewStateMask view_states,
            InstanceStateMask instance_states,
            bool exact_instance,
            bool single_instance,
            bool should_take);

    ReturnCode_t read_or_take_next_sample(
            void* data,
            SampleInfo* info,
            bool should_take);

    void set_read_communication_status(
            bool trigger_value);

    void update_subscription_matched_status(
            const fastdds::rtps::MatchingInfo& status);

    bool on_data_available(
            const fastdds::rtps::GUID_t& writer_guid,
            const fastdds::rtps::SequenceNumber_t& first_sequence,
            const fastdds::rtps::SequenceNumber_t& last_sequence);

    /**
     * @brief A method called when a new cache change is added
     * @param change The cache change that has been added
     * @return True if the change was added (due to some QoS it could have been 'rejected')
     */
    bool on_new_cache_change_added(
            const fastdds::rtps::CacheChange_t* const change);

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

    void subscriber_qos_updated();

    RequestedIncompatibleQosStatus& update_requested_incompatible_qos(
            PolicyMask incompatible_policies);

    LivelinessChangedStatus& update_liveliness_status(
            const LivelinessChangedStatus& status);

    const SampleLostStatus& update_sample_lost_status(
            int32_t sample_lost_since_last_update);

    /*!
     * @brief Update SampleRejectedStatus with information about a new rejected sample.
     *
     * @param [in] Reason why the new sample was rejected.
     * @param [in] New sample which was rejected.
     */
    const SampleRejectedStatus& update_sample_rejected_status(
            SampleRejectedStatusKind reason,
            const fastdds::rtps::CacheChange_t* const change_in);

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    DataReaderListener* get_listener_for(
            const StatusMask& status);

    std::shared_ptr<IPayloadPool> get_payload_pool();

    void release_payload_pool();

    void stop();

    ReturnCode_t check_datasharing_compatible(
            const fastdds::rtps::ReaderAttributes& reader_attributes,
            bool& is_datasharing_compatible) const;

private:

    void update_rtps_reader_qos();

    DataReaderQos get_datareader_qos_from_settings(
            const DataReaderQos& qos);

    bool is_data_sharing_compatible_ = false;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* FASTDDS_SUBSCRIBER__DATAREADERIMPL_HPP*/
