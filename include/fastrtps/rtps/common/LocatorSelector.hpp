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
 * @file LocatorSelector.hpp
 */

#ifndef FASTRTPS_RTPS_COMMON_LOCATORSELECTOR_HPP_
#define FASTRTPS_RTPS_COMMON_LOCATORSELECTOR_HPP_

#include "./LocatorSelectorEntry.hpp"
#include "./Guid.h"
#include "./Locator.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"

#include <algorithm>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * A class used for the efficient selection of locators when sending data to multiple entities.
 *
 * Algorithm:
 *   - Entries are added/removed with add_entry/remove_entry when matched/unmatched.
 *   - When data is to be sent:
 *     - A reference to this object is passed to the message group
 *     - For each submessage:
 *       - A call to reset is performed
 *       - A call to enable is performed per desired destination
 *       - If state_has_changed() returns true:
 *         - the message group is flushed
 *         - selection_start is called
 *         - for each transport:
 *           - transport_starts is called
 *           - transport handles the selection state of each entry
 *           - select may be called
 *       - Submessage is added to the message group
 */
class LocatorSelector
{
public:

    /**
     * Construct a LocatorSelector.
     *
     * @param entries_allocation Allocation configuration regarding the number of remote entities.
     */
    LocatorSelector(const ResourceLimitedContainerConfig& entries_allocation)
        : entries_(entries_allocation)
        , selections_(entries_allocation)
        , last_state_(entries_allocation)
    {
    }

    /**
     * Clears all internal data.
     */
    void clear()
    {
        entries_.clear();
        selections_.clear();
        last_state_.clear();
    }

    /**
     * Add an entry to this selector. 
     *
     * @param entry Pointer to the LocatorSelectorEntry to add.
     */
    bool add_entry(LocatorSelectorEntry* entry)
    {
        return entries_.push_back(entry) != nullptr;
    }

    /**
     * Remove an entry from this selector.
     *
     * @param entry Pointer to the LocatorSelectorEntry to remove.
     */
    bool remove_entry(const GUID_t& guid)
    {
        return entries_.remove_if(
            [guid](LocatorSelectorEntry* entry)
            {
                return entry->remote_guid == guid;
            });
    }

    /**
     * Reset the enabling state of the selector.
     *
     * @param enable_all Indicates whether entries should be initially enabled.
     */
    void reset(bool enable_all)
    {
        last_state_.clear();
        for(LocatorSelectorEntry* entry : entries_)
        {
            last_state_.push_back(entry->enabled ? 1 : 0);
            entry->enable(enable_all);
        }
    }

    /**
     * Enable an entry given its GUID.
     *
     * @param guid GUID of the entry to enable.
     */
    void enable(const GUID_t& guid)
    {
        for (LocatorSelectorEntry* entry : entries_)
        {
            if (entry->remote_guid == guid)
            {
                entry->enabled = true;
                break;
            }
        }
    }

    /**
     * Check if enabling state has changed.
     *
     * @return true if the enabling state has changed, false otherwise.
     */
    bool state_has_changed()
    {
        if (entries_.size() != last_state_.size())
        {
            return true;
        }

        for (size_t i = 0; i < entries_.size(); ++i)
        {
            if (last_state_.at(i) != (entries_.at(i)->enabled ? 1 : 0) )
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Reset the selection state of the selector.
     */
    void selection_start()
    {
        selections_.clear();
        for (LocatorSelectorEntry* entry : entries_)
        {
            entry->reset();
        }
    }

    /**
     * Called when the selection algorithm starts for a specific transport.
     *
     * Will set the temporary transport_should_process flag for all enabled entries.
     *
     * @return a reference to the entries collection.
     */
    ResourceLimitedVector<LocatorSelectorEntry*>& transport_starts()
    {
        for (LocatorSelectorEntry* entry : entries_)
        {
            entry->transport_should_process = entry->enabled;
        }

        return entries_;
    }

    /**
     * Marks an entry as selected.
     *
     * @param index The index of the entry to mark as selected.
     */
    void select(size_t index)
    {
        if (index < entries_.size() &&
            std::find(selections_.begin(), selections_.end(), index) == selections_.end())
        {
            selections_.push_back(index);
        }
    }

private:
    //! Entries collection.
    ResourceLimitedVector<LocatorSelectorEntry*> entries_;
    //! List of selected indexes.
    ResourceLimitedVector<size_t> selections_;
    //! Enabling state when reset was called.
    ResourceLimitedVector<int> last_state_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* FASTRTPS_RTPS_COMMON_LOCATORSELECTOR_H_ */
