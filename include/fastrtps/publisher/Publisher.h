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
 *
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/LivelinessLostStatus.h>

namespace eprosima {
namespace fastrtps {

namespace rtps {
struct GUID_t;
class WriteParams;
class RTPSParticipant;
}  // namespace rtps

class Participant;
class PublisherImpl;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Publisher
{
    friend class PublisherImpl;
    virtual ~Publisher();

public:

    /**
     * Constructor from a PublisherImpl pointer
     * @param pimpl Actual implementation of the publisher
     */
    Publisher(
            PublisherImpl* pimpl);

    /*!
     * @brief Writes a sample of the topic.
     * @param sample Pointer to the sample.
     * @return true when operation works successfully.
     * @note This method is blocked for a period of time.
     * ReliabilityQosPolicy.max_blocking_time on PublisherAttributes defines this period of time.
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    bool write(
            void* sample);

    /*!
     * @brief Writes a sample of the topic with additional options.
     * @param sample Pointer to the sample.
     * @param wparams Extra write parameters.
     * @return true when operation works successfully.
     * @note This method is blocked for a period of time.
     * ReliabilityQosPolicy.max_blocking_time on PublisherAttributes defines this period of time.
     * @par Calling example:
     * @snippet fastrtps_example.cpp ex_PublisherWrite
     */
    bool write(
            void* sample,
            rtps::WriteParams& wparams);

    /*!
     * @brief Informs that the application will be modifying a particular instance.
     * It gives and opportunity to the middleware to pre-configure itself to improve performance.
     * @param[in] instance Sample used to get the instance's key.
     * @return Handle containing the instance's key.
     * This handle could be used in successive `write` or `dispose` operations.
     * In case of error, HANDLE_NIL will be returned.
     */
    fastrtps::rtps::InstanceHandle_t register_instance(
            void* instance);

    /*!
     * @brief Requests the middleware to delete the instance.
     * Applications are made aware of the deletion through the DataReader objects.
     * @param[in] data Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param[in] handle Instance's key to be unregistered.
     * @return Returns the operation's result.
     * If the operation finishes successfully, `true` is returned.
     */
    bool dispose(
            void* data,
            const rtps::InstanceHandle_t& handle);
    /*!
     * @brief This operation reserves the action of `register_instance`.
     * Informs the middleware that the DataWriter is not intending to modify any more of that data instance.
     * Also indicates that the middleware can locally remove all information regarding that instance.
     * @param[in] instance Sample used to deduce instance's key in case of `handle` parameter is HANDLE_NIL.
     * @param[in] handle Instance's key to be unregistered.
     * @return Returns the operation's result.
     * If the operation finishes successfully, `true` is returned.
     */
    bool unregister_instance(
            void* instance,
            const rtps::InstanceHandle_t& handle);

    /**
     * Remove all the Changes in the associated RTPSWriter.
     * @param[out] removed Number of elements removed
     * @return True if all elements were removed.
     */
    bool removeAllChange(
            size_t* removed = nullptr);

    /**
     * Waits until all changes were acknowledged or max_wait.
     * @param max_wait Maximum time to wait until all changes are acknowledged.
     * @return True if all were acknowledged.
     */
    bool wait_for_all_acked(
            const Duration_t& max_wait);

    /**
     * Get the GUID_t of the associated RTPSWriter.
     * @return GUID_t.
     */
    const rtps::GUID_t& getGuid();

    /**
     * Get the Attributes of the Publisher.
     * @return Attributes of the publisher
     */
    const PublisherAttributes& getAttributes() const;

    /**
     * Update the Attributes of the publisher.
     * @param att Reference to a PublisherAttributes object to update the parameters.
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated.
     */
    bool updateAttributes(
            const PublisherAttributes& att);

    /**
     * @brief Returns the offered deadline missed status
     * @param status missed status struct
     */
    void get_offered_deadline_missed_status(
            OfferedDeadlineMissedStatus& status);

    /**
     * @brief Asserts liveliness
     */
    void assert_liveliness();

    /**
     * @brief Returns the liveliness lost status
     * @param status Liveliness lost status
     */
    void get_liveliness_lost_status(
            LivelinessLostStatus& status);

    /**
     * Get the list of locators from which this publisher may send data.
     *
     * @param [out] locators  LocatorList_t where the list of locators will be stored.
     */
    void get_sending_locators(
            rtps::LocatorList_t& locators) const;

private:

    PublisherImpl* mp_impl;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
