// Copyright 2016-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LivelinessManager.h
 */
#ifndef _FASTDDS_RTPS_LIVELINESS_MANAGER_H_
#define _FASTDDS_RTPS_LIVELINESS_MANAGER_H_

#include <fastdds/rtps/writer/LivelinessData.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using LivelinessCallback = std::function<void(
        const GUID_t&,
        const LivelinessQosPolicyKind&,
        const Duration_t&,
        int32_t alive_change,
        int32_t not_alive_change)>;

/**
 * @brief A class managing the liveliness of a set of writers. Writers are represented by their LivelinessData
 * @details Uses a shared timed event and informs outside classes on liveliness changes
 * @ingroup WRITER_MODULE
 */
class LivelinessManager
{
public:

    /**
     * @brief Constructor
     * @param callback A callback that will be invoked when a writer changes its liveliness status
     * @param service ResourceEvent object that will operate with the events.
     * @param manage_automatic True to manage writers with automatic liveliness, false otherwise
     */
    LivelinessManager(
            const LivelinessCallback& callback,
            ResourceEvent& service,
            bool manage_automatic = true);

    /**
     * @brief Constructor
     */
    ~LivelinessManager();

    /**
     * @brief LivelinessManager
     * @param other
     */
    LivelinessManager(const LivelinessManager& other) = delete;

    /**
     * @brief Adds a writer to the set
     * @param guid GUID of the writer
     * @param kind Liveliness kind
     * @param lease_duration Liveliness lease duration
     * @return True if the writer was successfully added
     */
    bool add_writer(
            GUID_t guid,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration);

    /**
     * @brief Removes a writer
     * @param guid GUID of the writer
     * @param kind Liveliness kind
     * @param lease_duration Liveliness lease duration
     * @return True if the writer was successfully removed
     */
    bool remove_writer(
            GUID_t guid,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration);

    /**
     * @brief Asserts liveliness of a writer in the set
     * @param guid The writer to assert liveliness of
     * @param kind The kind of the writer
     * @param lease_duration The lease duration
     * @return True if liveliness was successfully asserted
     */
    bool assert_liveliness(
            GUID_t guid,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration);

    /**
     * @brief Asserts liveliness of writers with given liveliness kind
     * @param kind Liveliness kind
     * @return True if liveliness was successfully asserted
     */
    bool assert_liveliness(LivelinessQosPolicyKind kind);

    /**
     * @brief A method to check any writer of the given kind is alive
     * @param kind The liveliness kind to check for
     * @return True if at least one writer of this kind is alive. False otherwise
     */
    bool is_any_alive(LivelinessQosPolicyKind kind);

    /**
     * @brief A method to return liveliness data
     * @details Should only be used for testing purposes
     * @return Vector of liveliness data
     */
    const ResourceLimitedVector<LivelinessData> &get_liveliness_data() const;

private:

    //! @brief A method responsible for invoking the callback when liveliness is asserted
    //! @param writer The liveliness data of the writer asserting liveliness
    //!
    void assert_writer_liveliness(LivelinessData& writer);

    /**
     * @brief A method to calculate the time when the next writer is going to lose liveliness
     * @details This method is public for testing purposes but it should not be used from outside this class
     * @return True if at least one writer is alive
     */
    bool calculate_next();

    //! @brief A method to find a writer from a guid, liveliness kind and lease duration
    //! @param guid The guid of the writer
    //! @param kind The liveliness kind
    //! @param lease_duration The lease duration
    //! @param wit_out Returns an iterator to the writer liveliness data
    //! @return Returns true if writer was found, false otherwise
    bool find_writer(
            const GUID_t &guid,
            const LivelinessQosPolicyKind &kind,
            const Duration_t &lease_duration,
            ResourceLimitedVector<LivelinessData>::iterator* wit_out);


    //! @brief A method called if the timer expires
    //! @return True if the timer should be restarted
    bool timer_expired();

    //! A callback to inform outside classes that a writer changed its liveliness status
    LivelinessCallback callback_;

    //! A boolean indicating whether we are managing writers with automatic liveliness
    bool manage_automatic_;

    //! A vector of liveliness data
    ResourceLimitedVector<LivelinessData> writers_;

    //! A mutex to protect the liveliness data
    std::mutex mutex_;

    //! The timer owner, i.e. the writer which is next due to lose its liveliness
    LivelinessData* timer_owner_;

    //! A timed callback expiring when a writer (the timer owner) loses its liveliness
    TimedEvent timer_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_LIVELINESS_MANAGER_H_ */
