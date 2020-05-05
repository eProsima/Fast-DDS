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
 * @file Subscriber.h
 */


#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>

namespace eprosima {
namespace fastrtps {

class SubscriberImpl;
class SampleInfo_t;

/**
 * Class Subscriber, contains the public API that allows the user to control the reception of messages.
 * This class should not be instantiated directly.
 * DomainRTPSParticipant class should be used to correctly create this element.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber
{
    friend class SubscriberImpl;

    virtual ~Subscriber()
    {
    }

public:

    /**
     * Constructor from a SubscriberImpl pointer
     * @param pimpl Actual implementation of the subscriber
     */
    Subscriber(
            SubscriberImpl* pimpl)
        : mp_impl(pimpl)
    {
    }

    /**
     * Get the associated GUID
     * @return Associated GUID
     */
    const rtps::GUID_t& getGuid();

    /**
     * Method to block the current thread until an unread message is available
     */
    inline void waitForUnreadMessage()
    {
        const Duration_t one_day{ 24 * 3600, 0 };
        while (!wait_for_unread_samples(one_day))
        {
        }
    }

    /*!
     * @brief Blocks the current thread until an unread sample is available.
     * @param timeout Maximum time the function will be blocked if any sample is received.
     * @return true in case unread samples are available.
     * In other case, false.
     */
    bool wait_for_unread_samples(
            const Duration_t& timeout);

    /**
     * @brief Reads next unread sample from the Subscriber.
     * @param sample Pointer to the object where you want the sample stored.
     * @param info Pointer to a SampleInfo_t structure that informs you about your sample.
     * @return True if a sample was read.
     * @note This method is blocked for a period of time.
     * ReliabilityQosPolicy.max_blocking_time on SubscriberAttributes defines this period of time.
     */
    bool readNextData(
            void* sample,
            SampleInfo_t* info);

    /**
     * @brief Takes next sample from the Subscriber. The sample is removed from the subscriber.
     * @param sample Pointer to the object where you want the sample stored.
     * @param info Pointer to a SampleInfo_t structure that informs you about your sample.
     * @return True if a sample was taken.
     * @note This method is blocked for a period of time.
     * ReliabilityQosPolicy.max_blocking_time on SubscriberAttributes defines this period of time.
     */
    bool takeNextData(
            void* sample,
            SampleInfo_t* info);

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
     * Get the Attributes of the Subscriber.
     * @return Attributes of the subscriber
     */
    const SubscriberAttributes& getAttributes() const;

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
    inline uint64_t getUnreadCount() const
    {
        return get_unread_count();
    }

    /**
     * Get the unread count.
     * @return Unread count
     */
    uint64_t get_unread_count() const;

    /**
     * @brief Get the requested deadline missed status
     * @param status The deadline missed status
     */
    void get_requested_deadline_missed_status(
            RequestedDeadlineMissedStatus& status);

    /**
     * @brief Returns the liveliness changed status
     * @param status Liveliness changed status
     */
    void get_liveliness_changed_status(
            LivelinessChangedStatus& status);

private:

    SubscriberImpl* mp_impl;
};



} /* namespace pubsub */
} /* namespace eprosima */

#endif /* SUBSCRIBER_H_ */
