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
 * @file DataWriter.hpp
 */

#ifndef _FASTRTPS_DATAWRITER_HPP_
#define _FASTRTPS_DATAWRITER_HPP_

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/common/WriteParams.h>

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/publisher/PublisherHistory.h>

#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>

namespace eprosima {
namespace fastrtps{
namespace rtps
{
class RTPSWriter;
class RTPSParticipant;
}

class TopicDataType;
class PublisherAttributes;
class PublisherListener;
class PublisherImpl;
class Publisher;

/**
 * Class DataWriter, contains the actual implementation of the behaviour of the DataWriter.
 * @ingroup FASTRTPS_MODULE
 */
class DataWriter
{
    friend class PublisherImpl;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    DataWriter(
            PublisherImpl* p,
            TopicDataType* topic,
            const PublisherAttributes& pub_att,
            const rtps::WriterAttributes& att,
            PublisherHistory&& history,
            PublisherListener* p_listen = nullptr);

public:

    virtual ~DataWriter();

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

    /**
     * Removes the cache change with the minimum sequence number
     * @return True if correct.
     */
    bool remove_min_seq_change();

    /**
     * Removes all changes from the History.
     * @param[out] removed Number of removed elements
     * @return True if correct.
     */
    bool remove_all_change(
            size_t* removed);

    /**
     *
     * @return
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

    bool wait_for_all_acked(
            const Duration_t& max_wait);

    /**
     * @brief Returns the offered deadline missed status
     * @param Deadline missed status struct
     */
    void get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status);

private:
    PublisherImpl* publisher_;

    //! Pointer to the associated Data Writer.
    rtps::RTPSWriter* writer_;

    //! Pointer to the TopicDataType object.
    TopicDataType* type_;

    rtps::WriterAttributes w_att_;

    //!Publisher History
    PublisherHistory history_;

    //!PublisherListener
    PublisherListener* listener_;

    //!Listener to capture the events of the Writer
    class DataWriterListener : public rtps::WriterListener
    {
        public:
            DataWriterListener(
                    DataWriter* w)
                : data_writer_(w)
            {

            }

            virtual ~DataWriterListener() {}

            void onWriterMatched(
                    rtps::RTPSWriter* writer,
                    rtps::MatchingInfo& info);

            void onWriterChangeReceivedByAll(
                    rtps::RTPSWriter* writer,
                    rtps::CacheChange_t* change);

            DataWriter* data_writer_;
    } writer_listener_;

    Publisher* user_publisher_;

    rtps::RTPSParticipant* rtps_participant_;

    uint32_t high_mark_for_frag_;

    //! A timer used to check for deadlines
    rtps::TimedCallback deadline_timer_;

    //! Deadline duration in microseconds
    std::chrono::duration<double, std::ratio<1,1000000>> deadline_duration_us_;

    //! The current timer owner, i.e. the instance which started the deadline timer
    rtps::InstanceHandle_t timer_owner_;

    //! The offered deadline missed status
    OfferedDeadlineMissedStatus deadline_missed_status_;

    //! A timed callback to remove expired samples for lifespan QoS
    rtps::TimedCallback lifespan_timer_;

    //! The lifespan duration, in microseconds
    std::chrono::duration<double, std::ratio<1, 1000000>> lifespan_duration_us_;

    /**
     * @brief A method called when an instance misses the deadline
     */
    void deadline_missed();

    /**
     * @brief A method to reschedule the deadline timer
     */
    void deadline_timer_reschedule();

    /**
     * @brief A method to remove expired samples, invoked when the lifespan timer expires
     */
    void lifespan_expired();
};

} /* namespace  */
} /* namespace eprosima */

#endif //_FASTRTPS_DATAWRITER_HPP_
