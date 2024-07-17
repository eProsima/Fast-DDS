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
 * @file LocatorSelectorEntry.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__LOCATORSELECTORENTRY_HPP
#define FASTDDS_RTPS_COMMON__LOCATORSELECTORENTRY_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * An entry for the @ref LocatorSelector.
 *
 * This class holds the locators of a remote endpoint along with data required for the locator selection algorithm.
 */
struct LocatorSelectorEntry
{
    /**
     * Holds the selection state of the locators held by a LocatorSelectorEntry
     */
    struct EntryState
    {
        /**
         * Construct an EntryState object.
         *
         * @param max_unicast_locators    Maximum number of unicast locators to held by parent LocatorSelectorEntry.
         * @param max_multicast_locators  Maximum number of multicast locators to held by parent LocatorSelectorEntry.
         */
        EntryState(
                size_t max_unicast_locators,
                size_t max_multicast_locators)
            : unicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_unicast_locators))
            , multicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_multicast_locators))
        {
        }

        //! Unicast locators selection state
        ResourceLimitedVector<size_t> unicast;
        //! Multicast locators selection state
        ResourceLimitedVector<size_t> multicast;
    };

    /**
     * Construct a LocatorSelectorEntry.
     *
     * @param max_unicast_locators    Maximum number of unicast locators to hold.
     * @param max_multicast_locators  Maximum number of multicast locators to hold.
     */
    LocatorSelectorEntry(
            size_t max_unicast_locators,
            size_t max_multicast_locators)
        : remote_guid(c_Guid_Unknown)
        , unicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_unicast_locators))
        , multicast(ResourceLimitedContainerConfig::fixed_size_configuration(max_multicast_locators))
        , state(max_unicast_locators, max_multicast_locators)
        , enabled(false)
        , transport_should_process(false)
    {
    }

    /**
     * Set the enabled value.
     *
     * @param should_enable Whether this entry should be enabled.
     */
    void enable(
            bool should_enable)
    {
        enabled = should_enable && remote_guid != c_Guid_Unknown;
    }

    /**
     * Reset the selections.
     */
    void reset()
    {
        state.unicast.clear();
        state.multicast.clear();
    }

    static LocatorSelectorEntry create_fully_selected_entry(
            const LocatorList_t& unicast_locators,
            const LocatorList_t& multicast_locators)
    {
        // Create an entry with space for all locators
        LocatorSelectorEntry entry(unicast_locators.size(), multicast_locators.size());
        // Add and select unicast locators
        for (const Locator_t& locator : unicast_locators)
        {
            entry.state.unicast.push_back(entry.unicast.size());
            entry.unicast.push_back(locator);
        }
        // Add and select multicast locators
        for (const Locator_t& locator : multicast_locators)
        {
            entry.state.multicast.push_back(entry.multicast.size());
            entry.multicast.push_back(locator);
        }
        // Return created entry
        return entry;
    }

    static LocatorSelectorEntry create_fully_selected_entry(
            const LocatorList_t& unicast_locators)
    {
        // Use previous overload with an empty multicast list
        LocatorList_t empty_list {};
        return create_fully_selected_entry(unicast_locators, empty_list);
    }

    //! GUID of the remote entity.
    GUID_t remote_guid;
    //! List of unicast locators to send data to the remote entity.
    ResourceLimitedVector<Locator_t> unicast;
    //! List of multicast locators to send data to the remote entity.
    ResourceLimitedVector<Locator_t> multicast;
    //! State of the entry
    EntryState state;
    //! Indicates whether this entry should be taken into consideration.
    bool enabled;
    //! A temporary value for each transport to help optimizing some use cases.
    bool transport_should_process;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_COMMON__LOCATORSELECTORENTRY_HPP
