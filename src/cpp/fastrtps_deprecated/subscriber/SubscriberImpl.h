// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberImpl.h
 *
 */

#ifndef SUBSCRIBERIMPL_H_
#define SUBSCRIBERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Time_t.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>

#include <fastdds/dds/topic/TopicDataType.hpp>

#include <rtps/history/ITopicPayloadPool.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
class RTPSReader;
class RTPSParticipant;
class TimedEvent;
} // namespace rtps

class SubscriberListener;
class Participant;
class ParticipantImpl;
class SampleInfo_t;
class Subscriber;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTRTPS_MODULE
 */
class SubscriberImpl
{
    friend class ParticipantImpl;

public:

    /**
     * @param p
     * @param ptype
     * @param attr
     * @param listen
     */
    SubscriberImpl(
            ParticipantImpl* p,
            fastdds::dds::TopicDataType* ptype,
            const SubscriberAttributes& attr,
            SubscriberListener* listen = nullptr);

    virtual ~SubscriberImpl();

    /*!
     * Method to block the current thread until an unread sasmple is available.
     * @param timeout maximum time the function will be blocked if any sample is received.
     */
    bool wait_for_unread_samples(
            const eprosima::fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    bool readNextData(
            void* data,
            SampleInfo_t* info);
    bool takeNextData(
            void* data,
            SampleInfo_t* info);

    ///@}

    /**
     * @brief Returns information about the first untaken sample.
     * @param [out] info Pointer to a SampleInfo_t structure to store first untaken sample information.
     * @return true if sample info was returned. false if there is no sample to take.
     */
    bool get_first_untaken_info(
            SampleInfo_t* info);

    /**
     * Update the Attributes of the subscriber;
     * @param att Reference to a SubscriberAttributes object to update the parameters;
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
     */
    bool updateAttributes(
            const SubscriberAttributes& att);

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    const rtps::GUID_t& getGuid();

    /**
     * Get the Attributes of the Subscriber.
     * @return Attributes of the Subscriber.
     */
    const SubscriberAttributes& getAttributes() const
    {
        return m_att;
    }

    /**
     * Get topic data type
     * @return Topic data type
     */
    fastdds::dds::TopicDataType* getType()
    {
        return mp_type;
    }

    /*!
     * @brief Returns there is a clean state with all Publishers.
     * It occurs when the Subscriber received all samples sent by Publishers. In other words,
     * its WriterProxies are up to date.
     * @return There is a clean state with all Publishers.
     */
    bool isInCleanState() const;

    /**
     * Get the unread count.
     * @return Unread count
     */
    uint64_t get_unread_count() const;

    /**
     * @brief A method called when a new cache change is added
     * @param change The cache change that has been added
     * @return True if the change was added (due to some QoS it could have been 'rejected')
     */
    bool onNewCacheChangeAdded(
            const rtps::CacheChange_t* const change);

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    void get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    /**
     * @brief Returns the liveliness changed status
     * @param status Liveliness changed status
     */
    void get_liveliness_changed_status(
            LivelinessChangedStatus& status);

    std::shared_ptr<rtps::IPayloadPool> payload_pool();

    /**
     * @return Returns Endpoint associated LocatorList_t
     */
    rtps::LocatorList_t get_locators();

private:

    //!Participant
    ParticipantImpl* mp_participant;

    //!Pointer to associated RTPSReader
    rtps::RTPSReader* mp_reader;
    //! Pointer to the TopicDataType object.
    fastdds::dds::TopicDataType* mp_type;
    //!Attributes of the Subscriber
    SubscriberAttributes m_att;
    //!History
    SubscriberHistory m_history;
    //!Listener
    SubscriberListener* mp_listener;

    class SubscriberReaderListener : public rtps::ReaderListener
    {
    public:

        SubscriberReaderListener(
                SubscriberImpl* s)
            : mp_subscriberImpl(s)
        {
        }

        virtual ~SubscriberReaderListener()
        {
        }

        void onReaderMatched(
                rtps::RTPSReader* reader,
                rtps::MatchingInfo& info) override;
        void onNewCacheChangeAdded(
                rtps::RTPSReader* reader,
                const rtps::CacheChange_t* const change) override;
        void on_liveliness_changed(
                rtps::RTPSReader* reader,
                const LivelinessChangedStatus& status) override;
        SubscriberImpl* mp_subscriberImpl;
    }
    m_readerListener;

    Subscriber* mp_userSubscriber;
    //!RTPSParticipant
    rtps::RTPSParticipant* mp_rtpsParticipant;

    //! A timer used to check for deadlines
    rtps::TimedEvent* deadline_timer_;
    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> deadline_duration_us_;
    //! The current timer owner, i.e. the instance which started the deadline timer
    rtps::InstanceHandle_t timer_owner_;
    //! Requested deadline missed status
    RequestedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples
    rtps::TimedEvent* lifespan_timer_;
    //! The lifespan duration
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    std::shared_ptr<rtps::ITopicPayloadPool> payload_pool_;

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


} /* namespace fastrtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* SUBSCRIBERIMPL_H_ */
