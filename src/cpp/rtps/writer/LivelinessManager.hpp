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
 * @file LivelinessManager.h
 */
#ifndef _FASTDDS_RTPS_LIVELINESS_MANAGER_H_
#define _FASTDDS_RTPS_LIVELINESS_MANAGER_H_

#include <mutex>

#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/writer/LivelinessData.hpp>
#include <utils/shared_mutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using LivelinessCallback = std::function<void (
                    const GUID_t&,
                    const fastdds::dds::LivelinessQosPolicyKind&,
                    const dds::Duration_t&,
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
    LivelinessManager(
            const LivelinessManager& other) = delete;

    /**
     * @brief Adds a writer to the set
     * @param guid GUID of the writer
     * @param kind Liveliness kind
     * @param lease_duration Liveliness lease duration
     * @return True if the writer was successfully added
     */
    bool add_writer(
            GUID_t guid,
            fastdds::dds::LivelinessQosPolicyKind kind,
            dds::Duration_t lease_duration);

    /**
     * @brief Removes a writer
     * @param [in] guid GUID of the writer
     * @param [in] kind Liveliness kind
     * @param [in] lease_duration Liveliness lease duration
     * @param [out] writer_liveliness_status The liveliness status of the writer
     * @return True if the writer was successfully removed
     */
    bool remove_writer(
            GUID_t guid,
            fastdds::dds::LivelinessQosPolicyKind kind,
            dds::Duration_t lease_duration,
            LivelinessData::WriterStatus& writer_liveliness_status);

    /**
     * @brief Asserts liveliness of a writer in the set
     * @param guid The writer to assert liveliness of
     * @param kind The kind of the writer
     * @param lease_duration The lease duration
     * @return True if liveliness was successfully asserted
     */
    bool assert_liveliness(
            GUID_t guid,
            fastdds::dds::LivelinessQosPolicyKind kind,
            dds::Duration_t lease_duration);

    /**
     * @brief Asserts liveliness of writers with given liveliness kind and GuidPrefix
     * @param kind Liveliness kind
     * @param guid_prefix The guid prefix of the writers to assert liveliness of
     * @return True if liveliness was successfully asserted
     */
    bool assert_liveliness(
            fastdds::dds::LivelinessQosPolicyKind kind,
            GuidPrefix_t guid_prefix);

    /**
     * @brief A method to check any writer of the given kind is alive
     * @param kind The liveliness kind to check for
     * @return True if at least one writer of this kind is alive. False otherwise
     */
    bool is_any_alive(
            fastdds::dds::LivelinessQosPolicyKind kind);

    /**
     * @brief A method to return liveliness data
     * @details Should only be used for testing purposes
     * @return Vector of liveliness data
     */
    const ResourceLimitedVector<LivelinessData>& get_liveliness_data() const;

private:

    /**
     * @brief A method responsible for invoking the callback when liveliness is asserted
     * @param writer The liveliness data of the writer asserting liveliness
     * @pre The collection shared_mutex must be taken for reading
     */
    void assert_writer_liveliness(
            LivelinessData& writer);

    /**
     * @brief A method to calculate the time when the next writer is going to lose liveliness
     * @details This method is public for testing purposes but it should not be used from outside this class
     * @pre std::mutex_ should not be taken on calling this method to avoid deadlock.
     * @return True if at least one writer is alive
     */
    bool calculate_next();

    //! @brief A method called if the timer expires
    //! @return True if the timer should be restarted
    bool timer_expired();

    //! A callback to inform outside classes that a writer changed its liveliness status
    const LivelinessCallback callback_;

    //! A boolean indicating whether we are managing writers with automatic liveliness
    const bool manage_automatic_;

    //! A vector of liveliness data
    ResourceLimitedVector<LivelinessData> writers_;

    //! A mutex to protect the liveliness data included LivelinessData objects
    std::mutex mutex_;

    //! A mutex devoted to protect the writers_ collection
    eprosima::shared_mutex col_mutex_;

    //! The timer owner, i.e. the writer which is next due to lose its liveliness
    LivelinessData* timer_owner_;

    //! A timed callback expiring when a writer (the timer owner) loses its liveliness
    TimedEvent timer_;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_LIVELINESS_MANAGER_H_ */
