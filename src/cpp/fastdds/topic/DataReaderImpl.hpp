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

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/PublicationBuiltinTopicData.hpp>

#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastdds/dds/topic/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/status/State.hpp>

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

class SubscriberImpl;
class SampleInfo_t;
class DomainParticipant;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReaderImpl
{
    friend class SubscriberImpl;

    /**
     * Creates a DataReader. Don't use it directly, but through Subscriber.
     */
    DataReaderImpl(
            SubscriberImpl* s,
            TypeSupport type,
            const Topic& topic,
            const fastrtps::TopicAttributes& topic_att,
            const fastrtps::rtps::ReaderAttributes& att,
            const DataReaderQos& qos,
            const fastrtps::rtps::MemoryManagementPolicy_t memory_policy,
            DataReaderListener* listener = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

public:

    virtual ~DataReaderImpl();

    /**
     * Method to block the current thread until an unread message is available
     */
    bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    /* TODO
       bool read(
            std::vector<void*>& data_values,
            std::vector<SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    ReturnCode_t read_next_sample(
            void* data,
            SampleInfo_t* info);

    /* TODO
       bool take(
            std::vector<void*>& data_values,
            std::vector<SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    ReturnCode_t take_next_sample(
            void* data,
            SampleInfo_t* info);

    ///@}

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
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    ReturnCode_t get_requested_deadline_missed_status(
            fastrtps::RequestedDeadlineMissedStatus& status);

    bool set_attributes(
            const fastrtps::rtps::ReaderAttributes& att);

    const fastrtps::rtps::ReaderAttributes& get_attributes() const;

    ReturnCode_t set_qos(
            const DataReaderQos& qos);

    const DataReaderQos& get_qos() const;


    bool set_topic(
            const Topic& topic);

    bool set_topic_attributes(
            const fastrtps::TopicAttributes& att);

    const fastrtps::TopicAttributes& get_topic_attributes() const;

    const Topic& get_topic() const;

    TopicDescription* get_topicdescription() const;

    ReturnCode_t set_listener(
            DataReaderListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    const DataReaderListener* get_listener() const;

    /* TODO
       bool get_key_value(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status) const;

    ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status) const;

    /* TODO
       bool get_sample_lost_status(
            fastrtps::SampleLostStatus& status) const;
     */

    ReturnCode_t get_sample_rejected_status(
            fastrtps::SampleRejectedStatus& status) const;

    ReturnCode_t get_subscription_matched_status(
            SubscriptionMatchedStatus& status) const;

    Subscriber* get_subscriber() const;

    /* TODO
       bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
     */

    /* TODO
       ReturnCode_t get_matched_publication_data(
            PublicationBuiltinTopicData publication_data,
            fastrtps::rtps::InstanceHandle_t publication_handle);
     */

    ReturnCode_t get_matched_publications(
            std::vector<fastrtps::rtps::InstanceHandle_t>& publication_handles) const;

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

private:

    //!Subscriber
    SubscriberImpl* subscriber_;

    //!Pointer to associated RTPSReader
    fastrtps::rtps::RTPSReader* reader_;

    //! Pointer to the TopicDataType object.
    TypeSupport type_;

    fastrtps::TopicAttributes topic_att_;

    Topic topic_;

    //!Attributes of the Subscriber
    fastrtps::rtps::ReaderAttributes att_;

    DataReaderQos qos_;

    ReaderQos rqos_;

    //!History
    fastrtps::SubscriberHistory history_;

    //!Listener
    DataReaderListener* listener_;

    ::dds::core::status::StatusMask mask_;

    class InnerDataReaderListener : public fastrtps::rtps::ReaderListener
    {
public:

        InnerDataReaderListener(
                DataReaderImpl* s)
            : data_reader_(s)
        {
        }

        virtual ~InnerDataReaderListener() override {}

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
                const RequestedIncompatibleQosStatus& status) override;

        void on_sample_rejected(
                fastrtps::rtps::RTPSReader* reader,
                const SampleRejectedStatus& status) override;

        DataReaderImpl* data_reader_;
    } reader_listener_;

    //! A timer used to check for deadlines
    fastrtps::rtps::TimedEvent* deadline_timer_;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000> > deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    fastrtps::rtps::InstanceHandle_t timer_owner_;

    //! Requested deadline missed status
    fastrtps::RequestedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples
    fastrtps::rtps::TimedEvent* lifespan_timer_;

    //! The lifespan duration
    std::chrono::duration<double, std::ratio<1, 1000000> > lifespan_duration_us_;

    DataReader* user_datareader_;

    std::vector<fastrtps::rtps::InstanceHandle_t> matched_publications_;

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

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif
#endif /* _FASTRTPS_DATAREADERIMPL_HPP_*/
