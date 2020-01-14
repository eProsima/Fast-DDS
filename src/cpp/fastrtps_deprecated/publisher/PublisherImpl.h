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
 * @file Publisher.h
 */



#ifndef PUBLISHERIMPL_H_
#define PUBLISHERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>

#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastrtps/publisher/PublisherHistory.h>

#include <fastdds/rtps/writer/WriterListener.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>

#include <fastdds/dds/topic/TopicDataType.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {
class RTPSWriter;
class RTPSParticipant;
class TimedEvent;
}

class PublisherListener;
class ParticipantImpl;
class Participant;
class Publisher;


/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherImpl
{
    friend class ParticipantImpl;

public:

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    PublisherImpl(
            ParticipantImpl* p,
            fastdds::dds::TopicDataType* ptype,
            const PublisherAttributes& att,
            PublisherListener* p_listen = nullptr);

    virtual ~PublisherImpl();

    /**
     *
     * @param kind
     * @param  Data
     * @return
     */
    bool create_new_change(
            rtps::ChangeKind_t kind,
            void* Data);

    /**
     *
     * @param kind
     * @param  Data
     * @param wparams
     * @return
     */
    bool create_new_change_with_params(
            rtps::ChangeKind_t kind,
            void* Data,
            rtps::WriteParams& wparams);

    /*!
     * @param sample
     * @return
     */
    rtps::InstanceHandle_t register_instance(
            void* sample);

    /**
     * Removes the cache change with the minimum sequence number
     * @return True if correct.
     */
    bool removeMinSeqChange();
    /**
     * Removes all changes from the History.
     * @param[out] removed Number of removed elements
     * @return True if correct.
     */
    bool removeAllChange(
            size_t* removed);

    /**
     *
     * @return
     */
    const rtps::GUID_t& getGuid();

    /**
     * Update the Attributes of the publisher;
     * @param att Reference to a PublisherAttributes object to update the parameters;
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
     */
    bool updateAttributes(
            const PublisherAttributes& att);

    /**
     * Get the Attributes of the Subscriber.
     * @return Attributes of the Subscriber.
     */
    inline const PublisherAttributes& getAttributes()
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

    bool wait_for_all_acked(
            const Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param Deadline missed status struct
     */
    void get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status);

    /**
     * @brief Returns the liveliness lost status
     * @param status Liveliness lost status
     */
    void get_liveliness_lost_status(
            LivelinessLostStatus& status);

    /**
     * @brief Asserts liveliness
     */
    void assert_liveliness();

private:

    ParticipantImpl* mp_participant;
    //! Pointer to the associated Data Writer.
    rtps::RTPSWriter* mp_writer;
    //! Pointer to the TopicDataType object.
    fastdds::dds::TopicDataType* mp_type;
    //!Attributes of the Publisher
    PublisherAttributes m_att;
    //!Publisher History
    PublisherHistory m_history;
    //!PublisherListener
    PublisherListener* mp_listener;
    //!Listener to capture the events of the Writer
    class PublisherWriterListener : public rtps::WriterListener
    {
public:

        PublisherWriterListener(
                PublisherImpl* p)
            : mp_publisherImpl(p)
        {
        }

        virtual ~PublisherWriterListener()
        {
        }

        void onWriterMatched(
                rtps::RTPSWriter* writer,
                rtps::MatchingInfo& info) override;
        void onWriterChangeReceivedByAll(
                rtps::RTPSWriter* writer,
                rtps::CacheChange_t* change) override;
        void on_liveliness_lost(
                rtps::RTPSWriter* writer,
                const LivelinessLostStatus& status) override;

        PublisherImpl* mp_publisherImpl;
    } m_writerListener;

    Publisher* mp_userPublisher;

    rtps::RTPSParticipant* mp_rtpsParticipant;

    uint32_t high_mark_for_frag_;

    //! A timer used to check for deadlines
    rtps::TimedEvent* deadline_timer_;
    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000> > deadline_duration_us_;
    //! The current timer owner, i.e. the instance which started the deadline timer
    rtps::InstanceHandle_t timer_owner_;
    //! The offered deadline missed status
    OfferedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples for lifespan QoS
    rtps::TimedEvent* lifespan_timer_;
    //! The lifespan duration, in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000> > lifespan_duration_us_;

    /**
     * @brief A method called when an instance misses the deadline
     */
    bool deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     * @return true value when the event has to be rescheduled. false value if not.
     */
    bool deadline_timer_reschedule();

    /**
     * @brief A method to remove expired samples, invoked when the lifespan timer expires
     * @return true value when the event has to be rescheduled. false value if not.
     */
    bool lifespan_expired();
};


} /* namespace  */
} /* namespace eprosima */
#endif
#endif /* PUBLISHER_H_ */
