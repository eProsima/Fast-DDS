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

#ifndef _FASTDDS_RTPS_COMMON_LOCATORSELECTOR_HPP_
#define _FASTDDS_RTPS_COMMON_LOCATORSELECTOR_HPP_

#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>
#include <fastrtps/utils/IPLocator.h>

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
    LocatorSelector(
            const ResourceLimitedContainerConfig& entries_allocation)
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
    bool add_entry(
            LocatorSelectorEntry* entry)
    {
        return entries_.push_back(entry) != nullptr;
    }

    /**
     * Remove an entry from this selector.
     * @param guid Identifier of the entry to be removed.
     */
    bool remove_entry(
            const GUID_t& guid)
    {
        return entries_.remove_if(
            [&guid](LocatorSelectorEntry* entry)
            {
                return entry->remote_guid == guid;
            });
    }

    /**
     * Reset the enabling state of the selector.
     *
     * @param enable_all Indicates whether entries should be initially enabled.
     */
    void reset(
            bool enable_all)
    {
        last_state_.clear();
        for (LocatorSelectorEntry* entry : entries_)
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
    void enable(
            const GUID_t& guid)
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
    bool state_has_changed() const
    {
        if (entries_.size() != last_state_.size())
        {
            return true;
        }

        for (size_t i = 0; i < entries_.size(); ++i)
        {
            if (last_state_.at(i) != (entries_.at(i)->enabled ? 1 : 0))
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
    void select(
            size_t index)
    {
        if (index < entries_.size() &&
                std::find(selections_.begin(), selections_.end(), index) == selections_.end())
        {
            selections_.push_back(index);
        }
    }

    /**
     * Count the number of selected locators.
     *
     * @return the number of selected locators.
     */
    size_t selected_size() const
    {
        size_t result = 0;

        for (size_t index : selections_)
        {
            LocatorSelectorEntry* entry = entries_.at(index);
            result += entry->state.multicast.size();
            result += entry->state.unicast.size();
        }

        return result;
    }

    /**
     * Check if a locator is present in the selections of this object.
     *
     * @param locator The locator to be checked.
     *
     * @return True if the locator has been selected, false otherwise.
     */
    bool is_selected(
            const Locator_t locator) const
    {
        if (IPLocator::isMulticast(locator))
        {
            for (size_t index : selections_)
            {
                LocatorSelectorEntry* entry = entries_.at(index);
                for (size_t loc_index : entry->state.multicast)
                {
                    if (entry->multicast.at(loc_index) == locator)
                    {
                        return true;
                    }
                }
            }
        }
        else
        {
            for (size_t index : selections_)
            {
                LocatorSelectorEntry* entry = entries_.at(index);
                for (size_t loc_index : entry->state.unicast)
                {
                    if (entry->unicast.at(loc_index) == locator)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     * Performs an action on each selected locator.
     *
     * @param action   Unary function that accepts a locator as argument.
     *                 The function shall not modify its argument.
     *                 This can either be a function pointer or a function object.
     */
    template<class UnaryPredicate>
    void for_each(
            UnaryPredicate action) const
    {
        for (size_t index : selections_)
        {
            LocatorSelectorEntry* entry = entries_.at(index);
            for (size_t loc_index : entry->state.multicast)
            {
                action(entry->multicast.at(loc_index));
            }
            for (size_t loc_index : entry->state.unicast)
            {
                action(entry->unicast.at(loc_index));
            }
        }
    }

    struct IteratorIndex
    {
        size_t selections_index;
        size_t state_index;
        bool state_multicast_done;
        Locator_t* locator;
    };

    class iterator :
        public LocatorsIterator
    {
        // use of std::iterator to introduce the following aliases is deprecated
        using iterator_category = std::input_iterator_tag;
        using value_type        = Locator_t;
        using difference_type   = IteratorIndex;
        using pointer           = Locator_t*;
        using reference         = Locator_t&;

        const LocatorSelector& locator_selector_;
        IteratorIndex current_;

        void go_to_next_entry()
        {
            // While entries selected
            while (++current_.selections_index < locator_selector_.selections_.size())
            {
                LocatorSelectorEntry* entry =
                        locator_selector_.entries_.at(locator_selector_.selections_[current_.selections_index]);

                // No multicast locators in this entry
                if (entry->state.multicast.size() == 0)
                {
                    // But there's unicast
                    if (entry->state.unicast.size() > 0)
                    {
                        current_.locator = &entry->unicast[entry->state.unicast.at(0)];
                        return;
                    }
                }
                else     // process multicast
                {
                    current_.state_multicast_done = false;
                    current_.locator = &entry->multicast[entry->state.multicast.at(0)];
                    return;
                }
            }

            current_.locator = nullptr;
        }

    public:

        enum class Position
        {
            Begin,
            End
        };

        explicit iterator(
                const LocatorSelector& locator_selector,
                Position index_pos)
            : locator_selector_(locator_selector)
        {
            current_ = {(std::numeric_limits<size_t>::max)(), 0, true, nullptr};

            if (index_pos == Position::Begin)
            {
                go_to_next_entry();
            }
        }

        iterator(
                const iterator& other)
            : locator_selector_(other.locator_selector_)
            , current_(other.current_)
        {
        }

        iterator& operator ++()
        {
            // Shouldn't call ++ when index already at the end
            assert(current_.selections_index < locator_selector_.selections_.size());

            LocatorSelectorEntry* entry =
                    locator_selector_.entries_.at(locator_selector_.selections_[current_.selections_index]);

            // Index at unicast locators
            if (current_.state_multicast_done)
            {
                // No more unicast locators selected
                if (++current_.state_index >= entry->state.unicast.size())
                {
                    current_.state_index = 0;
                    go_to_next_entry();
                }
                else     // current unicast locator
                {
                    current_.locator = &entry->unicast[entry->state.unicast.at(current_.state_index)];
                }
            }
            else     // Index at multicast locators
            {
                // No more multicast locators selected
                if (++current_.state_index >= entry->state.multicast.size())
                {
                    // Reset index to process unicast
                    current_.state_multicast_done = true;
                    current_.state_index = 0;
                    // No unicast locators
                    if (current_.state_index >= entry->state.unicast.size())
                    {
                        go_to_next_entry();
                    }
                    else     // current unicast locator
                    {
                        current_.locator = &entry->unicast[entry->state.unicast.at(current_.state_index)];
                    }
                }
                else     // current multicast locator
                {
                    current_.locator = &entry->multicast[entry->state.multicast.at(current_.state_index)];
                }
            }

            return *this;
        }

        bool operator ==(
                const LocatorsIterator& other) const
        {
            return *this == static_cast<const iterator&>(other);
        }

        bool operator !=(
                const LocatorsIterator& other) const
        {
            return !(*this == other);
        }

        bool operator ==(
                const iterator& other) const
        {
            return (current_.locator == other.current_.locator);
        }

        bool operator !=(
                const iterator& other) const
        {
            return !(*this == other);
        }

        pointer operator ->() const
        {
            return current_.locator;
        }

        reference operator *() const
        {
            return *current_.locator;
        }

    };

    iterator begin() const
    {
        return iterator(*this, iterator::Position::Begin);
    }

    iterator end() const
    {
        return iterator(*this, iterator::Position::End);
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

#endif /* _FASTDDS_RTPS_COMMON_LOCATORSELECTOR_H_ */
