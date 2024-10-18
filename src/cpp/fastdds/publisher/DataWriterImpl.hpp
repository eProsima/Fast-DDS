// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterImpl.hpp
 */

#ifndef _FASTDDS_DATAWRITERIMPL_HPP_
#define _FASTDDS_DATAWRITERIMPL_HPP_

#include <memory>

#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <fastdds/publisher/DataWriterHistory.hpp>
#include <fastdds/publisher/filtering/ReaderFilterCollection.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter;
class RTPSWriter;
class RTPSParticipant;
class TimedEvent;

} // namespace rtps

#ifdef FASTDDS_STATISTICS
namespace statistics {
namespace dds {
class DomainParticipantImpl;
} // namespace dds
} // namespace statistics
#endif // FASTDDS_STATISTICS

namespace dds {

class PublisherListener;
class PublisherImpl;
class Publisher;

/**
 * Class DataWriterImpl, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTDDS_MODULE
 */
class DataWriterImpl : protected rtps::IReaderDataFilter
{
    using LoanInitializationKind = DataWriter::LoanInitializationKind;
    using SerializedPayload_t = eprosima::fastdds::rtps::SerializedPayload_t;
    using CacheChange_t = eprosima::fastdds::rtps::CacheChange_t;
    class LoanCollection;

protected:

    friend class PublisherImpl;

#ifdef FASTDDS_STATISTICS
    friend class eprosima::fastdds::statistics::dds::DomainParticipantImpl;
#endif // FASTDDS_STATISTICS

    /**
     * Create a data writer, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    DataWriterImpl(
            PublisherImpl* p,
            TypeSupport type,
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    DataWriterImpl(
            PublisherImpl* p,
            TypeSupport type,
            Topic* topic,
            const DataWriterQos& qos,
            const fastdds::rtps::EntityId_t& entity_id,
            DataWriterListener* listener = nullptr);

public:

    virtual ~DataWriterImpl();

    /**
     * Enable this object.
     * The required lower layer entities will be created.
     *
     * @pre This method has not previously returned RETCODE_OK
     *
     * @return RETCODE_OK if all the lower layer entities have been correctly created.
     * @return Other standard return codes on error.
     */
    virtual ReturnCode_t enable();

    /**
     * Check if the preconditions to delete this object are met.
     *
     * @return RETCODE_PRECONDITION_NOT_MET if the preconditions to delete this object are not met.
     * @return RETCODE_OK if it is safe to delete this object.
     */
    ReturnCode_t check_delete_preconditions();

    /**
     * Get a pointer to the internal pool where the user could directly write.
     *
     * @param [out] sample          Pointer to the sample on the internal pool.
     * @param [in]  initialization  How to initialize the loaned sample.
     *
     * @return RETCODE_ILLEGAL_OPERATION when the type does not support loans.
     * @return RETCODE_OUT_OF_RESOURCES if the pool has been exhausted.
     * @return RETCODE_OK if a pointer to a sample is successfully obtained.
     */
    ReturnCode_t loan_sample(
            void*& sample,
            LoanInitializationKind initialization);

    /**
     * Discards a loaned sample pointer.
     *
     * @param [in,out] sample  Pointer to the previously loaned sample.
     *
     * @return RETCODE_ILLEGAL_OPERATION when the type does not support loans.
     * @return RETCODE_BAD_PARAMETER if the pointer does not correspond to a loaned sample.
     * @return RETCODE_OK if the loan is successfully discarded.
     */
    ReturnCode_t discard_loan(
            void*& sample);

    /**
     * Write data to the topic.
     *
     * @param data Pointer to the data.
     *
     * @return any of the standard return codes.
     */
    ReturnCode_t write(
            const void* const data);

    /**
     * Write data with params to the topic.
     *
     * @param data Pointer to the data.
     * @param params Extra write parameters.
     *
     * @return any of the standard return codes.
     */
    ReturnCode_t write(
            const void* const data,
            fastdds::rtps::WriteParams& params);

    /**
     * @brief Implementation of the DDS `write` operation.
     *
     * @param [in] data    Pointer to the data to publish.
     * @param [in] handle  Handle of the instance to update. The special value @c HANDLE_NIL can be used to indicate
     *                    that the instance should be automatically calculated.
     *
     * @return any of the standard return codes.
     */
    ReturnCode_t write(
            const void* const data,
            const InstanceHandle_t& handle);

    /**
     * @brief Implementation of the DDS `write_w_timestamp` operation.
     *
     * @param [in] data        Pointer to the data to publish.
     * @param [in] handle      Handle of the instance to update. The special value @c HANDLE_NIL can be used to indicate
     *                        that the instance should be automatically calculated.
     * @param [in] timestamp   Timestamp to associate to the sample info of the published data.
     *
     * @return any of the standard return codes.
     */
    ReturnCode_t write_w_timestamp(
            const void* const data,
            const InstanceHandle_t& handle,
            const fastdds::dds::Time_t& timestamp);

    /**
     * @brief Implementation of the DDS `register_instance` operation.
     * It deduces the instance's key and tries to get resources in the DataWriterHistory.
     *
     * @param [in] instance Sample used to get the instance's key.
     *
     * @return Handle containing the instance's key.
     * This handle could be used in successive `write` or `dispose` operations.
     * In case of error, HANDLE_NIL will be returned.
     */
    InstanceHandle_t register_instance(
            const void* const instance);

    /**
     * @brief Implementation of the DDS `register_instance_w_timestamp` operation.
     * It deduces the instance's key and tries to get resources in the DataWriterHistory.
     *
     * @param [in] instance Sample used to get the instance's key.
     * @param [in] timestamp Timestamp to set on the instance registration operation.
     *
     * @return Handle containing the instance's key.
     * This handle could be used in successive `write` or `dispose` operations.
     * In case of error, HANDLE_NIL will be returned.
     */
    InstanceHandle_t register_instance_w_timestamp(
            const void* const instance,
            const fastdds::dds::Time_t& timestamp);

    /**
     * @brief Implementation of the DDS `unregister_instance` and `dispose` operations.
     * It sends a CacheChange_t with a kind that depends on the `dispose` parameter and
     * `writer_data_lifecycle` QoS.
     *
     * @param [in] instance  Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param [in] handle    Instance's key to be unregistered or disposed.
     * @param [in] dispose   If `dispose` is `false`, a CacheChange_t with kind set to NOT_ALIVE_UNREGISTERED is sent,
     *                      or if `writer_data_lifecycle.autodispose_unregistered_instances` is `true` then it is sent
     *                      with kind set to NOT_ALIVE_DISPOSED_UNREGISTERED.
     *                      If `dispose` is `true`, a CacheChange_t with kind set to NOT_ALIVE_DISPOSED is sent.
     *
     * @return Returns the operation's result.
     * If the operation finishes successfully, RETCODE_OK is returned.
     */
    ReturnCode_t unregister_instance(
            const void* const instance,
            const InstanceHandle_t& handle,
            bool dispose = false);

    /**
     * @brief Implementation of the DDS `unregister_instance_w_timestamp` and `dispose_w_timestamp` operations.
     * It sends a CacheChange_t with a kind that depends on the `dispose` parameter and
     * `writer_data_lifecycle` QoS.
     *
     * @param [in] instance  Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param [in] handle    Instance's key to be unregistered or disposed.
     * @param [in] timestamp Source timestamp to set on the CacheChange_t being sent.
     * @param [in] dispose   If `dispose` is `false`, a CacheChange_t with kind set to NOT_ALIVE_UNREGISTERED is sent,
     *                      or if `writer_data_lifecycle.autodispose_unregistered_instances` is `true` then it is sent
     *                      with kind set to NOT_ALIVE_DISPOSED_UNREGISTERED.
     *                      If `dispose` is `true`, a CacheChange_t with kind set to NOT_ALIVE_DISPOSED is sent.
     *
     * @return Returns the operation's result.
     * If the operation finishes successfully, RETCODE_OK is returned.
     */
    ReturnCode_t unregister_instance_w_timestamp(
            const void* const instance,
            const InstanceHandle_t& handle,
            const fastdds::dds::Time_t& timestamp,
            bool dispose = false);

    /**
     *
     * @return
     */
    const fastdds::rtps::GUID_t& guid() const;

    InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport get_type() const
    {
        return type_;
    }

    ReturnCode_t wait_for_acknowledgments(
            const fastdds::dds::Duration_t& max_wait);

    ReturnCode_t wait_for_acknowledgments(
            const void* const instance,
            const InstanceHandle_t& handle,
            const fastdds::dds::Duration_t& max_wait);

    ReturnCode_t get_publication_matched_status(
            PublicationMatchedStatus& status);

    ReturnCode_t get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status);

    ReturnCode_t get_offered_incompatible_qos_status(
            OfferedIncompatibleQosStatus& status);

    ReturnCode_t set_qos(
            const DataWriterQos& qos);

    const DataWriterQos& get_qos() const;

    Topic* get_topic() const;

    const DataWriterListener* get_listener() const;

    ReturnCode_t set_listener(
            DataWriterListener* listener);

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
    ReturnCode_t get_key_value(
            void* key_holder,
            const InstanceHandle_t& handle);

    ReturnCode_t get_liveliness_lost_status(
            LivelinessLostStatus& status);

    const Publisher* get_publisher() const;

    ReturnCode_t assert_liveliness();

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    virtual void disable();

    /**
     * Removes all changes from the History.
     * @param [out] removed Number of removed elements
     * @return RETCODE_OK if correct, RETCODE_ERROR if not.
     */
    ReturnCode_t clear_history(
            size_t* removed);

    /**
     * @brief Get the list of locators from which this DataWriter may send data.
     *
     * @param [out] locators  LocatorList where the list of locators will be stored.
     *
     * @return NOT_ENABLED if the reader has not been enabled.
     * @return OK if a list of locators is returned.
     */
    ReturnCode_t get_sending_locators(
            rtps::LocatorList& locators) const;

    /**
     * Called from the DomainParticipant when a filter factory is being unregistered.
     *
     * @param filter_class_name  The class name under which the factory was registered.
     */
    void filter_is_being_removed(
            const char* filter_class_name);

    /**
     * @brief Retrieves in a subscription associated with the @ref DataWriter
     *
     * @param[out] subscription_data subscription data struct
     * @param subscription_handle @ref InstanceHandle_t of the subscription
     * @return @ref RETCODE_BAD_PARAMETER if the DataWriter is not matched with
     * the given subscription handle, @ref RETCODE_OK otherwise.
     *
     */
    ReturnCode_t get_matched_subscription_data(
            SubscriptionBuiltinTopicData& subscription_data,
            const InstanceHandle_t& subscription_handle) const;

    /**
     * @brief Fills the given vector with the @ref InstanceHandle_t of matched DataReaders
     *
     * @param[out] subscription_handles Vector where the @ref InstanceHandle_t are returned
     * @return @ref RETCODE_OK if the operation succeeds.
     *
     * @note Returning an empty list is not an error, it returns @ref RETCODE_OK.
     *
     */
    ReturnCode_t get_matched_subscriptions(
            std::vector<InstanceHandle_t>& subscription_handles) const;

    /**
     * Retrieve the publication data discovery information.
     *
     * @param [out] publication_data The publication data discovery information.
     *
     * @return NOT_ENABLED if the writer has not been enabled.
     * @return OK if the publication data is returned.
     */
    ReturnCode_t get_publication_builtin_topic_data(
            PublicationBuiltinTopicData& publication_data) const;

protected:

    using IChangePool = eprosima::fastdds::rtps::IChangePool;
    using IPayloadPool = eprosima::fastdds::rtps::IPayloadPool;
    using ITopicPayloadPool = eprosima::fastdds::rtps::ITopicPayloadPool;

    PublisherImpl* publisher_ = nullptr;

    //! Pointer to the associated RTPS Writer.
    fastdds::rtps::BaseWriter* writer_ = nullptr;

    //! Pointer to the TopicDataType object.
    TypeSupport type_;

    Topic* topic_ = nullptr;

    DataWriterQos qos_;

    //! DataWriterListener
    DataWriterListener* listener_ = nullptr;

    //! Mutex to protect listener_
    std::mutex listener_mutex_;

    //!History
    std::unique_ptr<DataWriterHistory> history_;

    //!Listener to capture the events of the Writer
    class InnerDataWriterListener : public fastdds::rtps::WriterListener
    {
    public:

        InnerDataWriterListener(
                DataWriterImpl* w)
            : data_writer_(w)
        {
        }

        virtual ~InnerDataWriterListener() override
        {
        }

        void on_writer_matched(
                fastdds::rtps::RTPSWriter* writer,
                const fastdds::rtps::MatchingInfo& info) override;

        void on_offered_incompatible_qos(
                fastdds::rtps::RTPSWriter* writer,
                fastdds::dds::PolicyMask qos) override;

        void on_writer_change_received_by_all(
                fastdds::rtps::RTPSWriter* writer,
                fastdds::rtps::CacheChange_t* change) override;

        void on_liveliness_lost(
                fastdds::rtps::RTPSWriter* writer,
                const LivelinessLostStatus& status) override;

        void on_reader_discovery(
                fastdds::rtps::RTPSWriter* writer,
                fastdds::rtps::ReaderDiscoveryStatus reason,
                const fastdds::rtps::GUID_t& reader_guid,
                const fastdds::rtps::SubscriptionBuiltinTopicData* reader_info) override;

#ifdef FASTDDS_STATISTICS
        void notify_status_observer(
                const uint32_t& status_id);
#endif //FASTDDS_STATISTICS

        DataWriterImpl* data_writer_;
    }
    writer_listener_;

    //! A timer used to check for deadlines
    fastdds::rtps::TimedEvent* deadline_timer_ = nullptr;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    InstanceHandle_t timer_owner_;

    //! The publication matched status
    PublicationMatchedStatus publication_matched_status_;

    //! The offered deadline missed status
    OfferedDeadlineMissedStatus deadline_missed_status_;

    //! The liveliness lost status
    LivelinessLostStatus liveliness_lost_status_;

    //! The offered incompatible qos status
    OfferedIncompatibleQosStatus offered_incompatible_qos_status_;

    //! A timed callback to remove expired samples for lifespan QoS
    fastdds::rtps::TimedEvent* lifespan_timer_ = nullptr;

    //! The lifespan duration, in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    DataWriter* user_datawriter_ = nullptr;

    bool is_data_sharing_compatible_ = false;

    uint32_t fixed_payload_size_ = 0u;

    rtps::PoolConfig pool_config_ {};

    std::shared_ptr<IPayloadPool> payload_pool_;

    bool is_custom_payload_pool_ = false;

    std::unique_ptr<LoanCollection> loans_;

    fastdds::rtps::GUID_t guid_;

    std::unique_ptr<ReaderFilterCollection> reader_filters_;

    DataRepresentationId_t data_representation_ {DEFAULT_DATA_REPRESENTATION};

    ReturnCode_t check_write_preconditions(
            const void* const data,
            const InstanceHandle_t& handle,
            InstanceHandle_t& instance_handle);

    ReturnCode_t check_instance_preconditions(
            const void* const data,
            const InstanceHandle_t& handle,
            InstanceHandle_t& instance_handle);

    InstanceHandle_t do_register_instance(
            const void* const key,
            const InstanceHandle_t instance_handle,
            fastdds::rtps::WriteParams& wparams);

    /**
     *
     * @param kind
     * @param  data
     * @return
     */
    ReturnCode_t create_new_change(
            fastdds::rtps::ChangeKind_t kind,
            const void* const data);

    /**
     *
     * @param kind
     * @param  data
     * @param wparams
     * @return
     */
    ReturnCode_t create_new_change_with_params(
            fastdds::rtps::ChangeKind_t kind,
            const void* const data,
            fastdds::rtps::WriteParams& wparams);

    /**
     *
     * @param kind
     * @param  data
     * @param wparams
     * @param handle
     * @return
     */
    ReturnCode_t create_new_change_with_params(
            fastdds::rtps::ChangeKind_t kind,
            const void* const data,
            fastdds::rtps::WriteParams& wparams,
            const InstanceHandle_t& handle);

    /**
     * Removes the cache change with the minimum sequence number
     * @return True if correct.
     */
    bool remove_min_seq_change();

    void update_publication_matched_status(
            const fastdds::rtps::MatchingInfo& status);

    /**
     * @brief A method called when an instance misses the deadline
     */
    bool deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    bool deadline_timer_reschedule();

    /**
     * @brief A method to remove expired samples, invoked when the lifespan timer expires
     */
    bool lifespan_expired();

    ReturnCode_t check_new_change_preconditions(
            fastdds::rtps::ChangeKind_t change_kind,
            const void* const data);

    ReturnCode_t perform_create_new_change(
            fastdds::rtps::ChangeKind_t change_kind,
            const void* const data,
            fastdds::rtps::WriteParams& wparams,
            const InstanceHandle_t& handle);

    static void set_qos(
            DataWriterQos& to,
            const DataWriterQos& from,
            bool update_immutable);

    /**
     * Extends the check_qos() call, including the check for
     * resource limits policy.
     * @param qos Pointer to the qos to be checked.
     * @param type Pointer to the associated TypeSupport object.
     * @return True if correct.
     */
    static ReturnCode_t check_qos_including_resource_limits(
            const DataWriterQos& qos,
            const TypeSupport& type);

    /**
     * Checks the consistency of the qos configuration.
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_qos(
            const DataWriterQos& qos);

    /**
     * Checks resource limits policy: Instance allocation consistency
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_allocation_consistency(
            const DataWriterQos& qos);

    static bool can_qos_be_updated(
            const DataWriterQos& to,
            const DataWriterQos& from);

    void publisher_qos_updated();

    OfferedIncompatibleQosStatus& update_offered_incompatible_qos(
            PolicyMask incompatible_policies);

    /*!
     * @brief Updates liveliness lost status.
     *
     * @param [in] liveliness_lost_status Liveliness lost status coming from RTPS layer.
     * @return Current liveliness lost status.
     */
    LivelinessLostStatus& update_liveliness_lost_status(
            const LivelinessLostStatus& liveliness_lost_status);

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    DataWriterListener* get_listener_for(
            const StatusMask& status);

    void set_fragment_size_on_change(
            fastdds::rtps::WriteParams& wparams,
            fastdds::rtps::CacheChange_t* ch,
            const uint32_t& high_mark_for_frag);

    std::shared_ptr<IChangePool> get_change_pool() const;

    std::shared_ptr<IPayloadPool> get_payload_pool();

    bool release_payload_pool();

    ReturnCode_t check_datasharing_compatible(
            const fastdds::rtps::WriterAttributes& writer_attributes,
            bool& is_datasharing_compatible) const;

    bool get_free_payload_from_pool(
            uint32_t size,
            SerializedPayload_t& payload);

    bool add_loan(
            const void* const data,
            SerializedPayload_t& payload);

    bool check_and_remove_loan(
            const void* const data,
            SerializedPayload_t& payload);

    /**
     * Remove internal filtering information about a reader.
     * Called whenever a non-intra-process reader is unmatched.
     *
     * @param reader_guid  GUID of the reader that has been unmatched.
     */
    void remove_reader_filter(
            const fastdds::rtps::GUID_t& reader_guid);

    /**
     * Process filtering information for a reader.
     * Called when a new reader is matched, and whenever the discovery information of a matched reader changes.
     *
     * @param reader_guid  The GUID of the reader for which the discovery information changed.
     * @param reader_info  The reader's discovery information.
     */
    void process_reader_filter_info(
            const fastdds::rtps::GUID_t& reader_guid,
            const fastdds::rtps::SubscriptionBuiltinTopicData& reader_info);

    bool is_relevant(
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const override;

private:

    void create_history(
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool);

    DataWriterQos get_datawriter_qos_from_settings(
            const DataWriterQos& qos);

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif //_FASTDDS_DATAWRITERIMPL_HPP_
