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

#ifndef SUBSCRIBERIMPL_H_
#define SUBSCRIBERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>

#include <fastrtps/qos/ReaderQos.h>

#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastrtps/topic/DataReaderListener.hpp>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>

namespace eprosima {
namespace fastrtps {
namespace rtps
{
class RTPSReader;
class RTPSParticipant;
}

class TopicDataType;
class ParticipantImpl;
class SampleInfo_t;
class Subscriber;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTRTPS_MODULE
 */
class DataReader {
    friend class ParticipantImpl;

    /**
    * Creates a DataReader. Don't use it directly, but through Subscriber.
    */
    DataReader(
        ParticipantImpl* p,
        TopicDataType* topic,
        const TopicAttributes& topic_att,
        const rtps::ReaderAttributes& att,
        DataReaderListener* listener = nullptr);

public:
    virtual ~DataReader();

    /**
     * Method to block the current thread until an unread message is available
     */
    void wait_for_unread_message();


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    bool read_next_data(
            void* data,
            SampleInfo_t* info);

    bool take_next_data(
            void* data,
            SampleInfo_t* info);

    ///@}

    /**
    * Get associated GUID
    * @return Associated GUID
    */
    const rtps::GUID_t& guid();

    /**
    * Get topic data type
    * @return Topic data type
    */
    TopicDataType* type()
    {
        return type_;
    }

    /*!
    * @brief Returns there is a clean state with all Publishers.
    * It occurs when the Subscriber received all samples sent by Publishers. In other words,
    * its WriterProxies are up to date.
    * @return There is a clean state with all Publishers.
    */
    bool is_in_clean_state() const;

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
    bool on_new_cache_change_added(
            const rtps::CacheChange_t* const change);

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    void get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    bool update_attributes(const rtps::ReaderAttributes& att);

    bool update_qos(const ReaderQos& qos);

    bool update_topic(const TopicAttributes& att);

private:

    //!Participant
    ParticipantImpl* participant_;

    //!Pointer to associated RTPSReader
    rtps::RTPSReader* reader_;

    //! Pointer to the TopicDataType object.
    TopicDataType* type_;

    TopicAttributes topic_att_;

    //!Attributes of the Subscriber
    rtps::ReaderAttributes att_;

    ReaderQos qos_;

    //!History
    SubscriberHistory history_;

    //!Listener
    DataReaderListener* listener_;

    class InnerDataReaderListener : public rtps::ReaderListener
    {
    public:
        InnerDataReaderListener(
                DataReader* s)
            : data_reader_(s)
        {
        }

        virtual ~InnerDataReaderListener() override {}

        void on_reader_matched(
                rtps::RTPSReader* reader,
                rtps::MatchingInfo& info) override;

        void on_new_cache_change_added(
                rtps::RTPSReader* reader,
                const rtps::CacheChange_t* const change) override;

        DataReader* data_reader_;
    } reader_listener_;

    //!RTPSParticipant
    rtps::RTPSParticipant* rtps_participant_;

    //! A timer used to check for deadlines
    rtps::TimedCallback deadline_timer_;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    rtps::InstanceHandle_t timer_owner_;

    //! Requested deadline missed status
    RequestedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples
    rtps::TimedCallback lifespan_timer_;

    //! The lifespan duration
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    /**
     * @brief Method called when an instance misses the deadline
     */
    void deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    void deadline_timer_reschedule();

    /**
     * @brief A method called when the lifespan timer expires
     */
    void lifespan_expired();

};


} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* SUBSCRIBERIMPL_H_ */
