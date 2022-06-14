// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataReaderInstance.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERINSTANCE_HPP_
#define _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERINSTANCE_HPP_

#include <chrono>
#include <cstdint>

#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>

#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

#include "DataReaderCacheChange.hpp"
#include "DataReaderHistoryCounters.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/// Book-keeping information for an instance
struct DataReaderInstance
{
    using ChangeCollection = eprosima::fastrtps::ResourceLimitedVector<DataReaderCacheChange, std::true_type>;
    using WriterOwnership = std::pair<fastrtps::rtps::GUID_t, uint32_t>;
    using WriterCollection = eprosima::fastrtps::ResourceLimitedVector<WriterOwnership, std::false_type>;

    //! A vector of DataReader changes belonging to the same instance
    ChangeCollection cache_changes;
    //! The list of alive writers for this instance
    WriterCollection alive_writers;
    //! GUID and strength of the current maximum strength writer
    WriterOwnership current_owner{ {}, 0 };
    //! The time when the group will miss the deadline
    std::chrono::steady_clock::time_point next_deadline_us;
    //! Current view state of the instance
    ViewStateKind view_state = ViewStateKind::NEW_VIEW_STATE;
    //! Current instance state of the instance
    InstanceStateKind instance_state = InstanceStateKind::ALIVE_INSTANCE_STATE;
    //! Current disposed generation of the instance
    int32_t disposed_generation_count = 0;
    //! Current no_writers generation of the instance
    int32_t no_writers_generation_count = 0;

    DataReaderInstance(
            const eprosima::fastrtps::ResourceLimitedContainerConfig& changes_allocation,
            const eprosima::fastrtps::ResourceLimitedContainerConfig& writers_allocation)
        : cache_changes(changes_allocation)
        , alive_writers(writers_allocation)
    {
    }

    void writer_update_its_ownership_strength(
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        if (writer_guid == current_owner.first)
        {
            // Check it is an "alive" writer.
            auto writer_it = std::find_if(alive_writers.begin(), alive_writers.end(),
                            [&writer_guid](const WriterOwnership& item)
                            {
                                return item.first == writer_guid;
                            });

            assert(alive_writers.end() != writer_it);

            // Update writer info
            (*writer_it).second = ownership_strength;
            current_owner.second = ownership_strength;
            update_owner();
        }

        return;
    }

    bool update_state(
            DataReaderHistoryCounters& counters,
            const fastrtps::rtps::ChangeKind_t change_kind,
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength = 0)
    {
        bool ret_val = false;

        if (!has_been_accounted_)
        {
            has_been_accounted_ = true;
            assert(ViewStateKind::NEW_VIEW_STATE == view_state);
            ++counters.instances_new;
            assert(InstanceStateKind::ALIVE_INSTANCE_STATE == instance_state);
            ++counters.instances_alive;
        }

        switch (change_kind)
        {
            case fastrtps::rtps::ALIVE:
                ret_val = writer_alive(counters, writer_guid, ownership_strength);
                break;

            case fastrtps::rtps::NOT_ALIVE_DISPOSED:
                ret_val = writer_dispose(counters, writer_guid, ownership_strength);
                break;

            case fastrtps::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED:
                ret_val = writer_dispose(counters, writer_guid, ownership_strength);
                ret_val |= writer_unregister(counters, writer_guid);
                break;

            case fastrtps::rtps::NOT_ALIVE_UNREGISTERED:
                ret_val = writer_unregister(counters, writer_guid);
                break;

            default:
                // TODO (Miguel C): log error / assert
                break;
        }

        return ret_val;
    }

    bool writer_removed(
            DataReaderHistoryCounters& counters,
            const fastrtps::rtps::GUID_t& writer_guid)
    {
        return has_been_accounted_ && writer_unregister(counters, writer_guid);
    }

private:

    //! Whether this instance has ever been included in the history counters
    bool has_been_accounted_ = false;

    bool writer_alive(
            DataReaderHistoryCounters& counters,
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        bool ret_val = false;

        if (ownership_strength >= current_owner.second)
        {
            current_owner.first = writer_guid;
            current_owner.second = ownership_strength;

            if (InstanceStateKind::NOT_ALIVE_DISPOSED_INSTANCE_STATE == instance_state)
            {
                counters_update(counters.instances_disposed, counters.instances_alive, counters, true);

                ++disposed_generation_count;
                alive_writers.clear();
                view_state = ViewStateKind::NEW_VIEW_STATE;
                ret_val = true;
            }
            else if (InstanceStateKind::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE == instance_state)
            {
                counters_update(counters.instances_no_writers, counters.instances_alive, counters, true);

                ++no_writers_generation_count;
                alive_writers.clear();
                view_state = ViewStateKind::NEW_VIEW_STATE;
                ret_val = true;
            }

            instance_state = InstanceStateKind::ALIVE_INSTANCE_STATE;
        }

        writer_set(writer_guid, ownership_strength);

        return ret_val;
    }

    bool writer_dispose(
            DataReaderHistoryCounters& counters,
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        bool ret_val = false;

        writer_set(writer_guid, ownership_strength);
        if (ownership_strength >= current_owner.second)
        {
            current_owner.first = writer_guid;
            current_owner.second = ownership_strength;

            if (InstanceStateKind::ALIVE_INSTANCE_STATE == instance_state)
            {
                ret_val = true;
                instance_state = InstanceStateKind::NOT_ALIVE_DISPOSED_INSTANCE_STATE;
                counters_update(counters.instances_alive, counters.instances_disposed, counters, false);
            }
        }

        return ret_val;
    }

    bool writer_unregister(
            DataReaderHistoryCounters& counters,
            const fastrtps::rtps::GUID_t& writer_guid)
    {
        bool ret_val = false;

        alive_writers.remove_if([&writer_guid](const WriterOwnership& item)
                {
                    return item.first == writer_guid;
                });

        if (writer_guid == current_owner.first)
        {
            current_owner.second = 0;
            current_owner.first = fastrtps::rtps::c_Guid_Unknown;
        }

        if (alive_writers.empty() && (InstanceStateKind::ALIVE_INSTANCE_STATE == instance_state))
        {
            ret_val = true;
            instance_state = InstanceStateKind::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
            counters_update(counters.instances_alive, counters.instances_no_writers, counters, false);
        }

        return ret_val;
    }

    void writer_set(
            const fastrtps::rtps::GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        auto it = std::find_if(alive_writers.begin(), alive_writers.end(), [&writer_guid](const WriterOwnership& item)
                        {
                            return item.first == writer_guid;
                        });
        if (it == alive_writers.end())
        {
            alive_writers.emplace_back(writer_guid, ownership_strength);
        }
        else
        {
            it->second = ownership_strength;
        }
    }

    void counters_update(
            uint64_t& decremented_counter,
            uint64_t& incremented_counter,
            DataReaderHistoryCounters& counters,
            bool set_as_new_view_state)
    {
        --decremented_counter;
        ++incremented_counter;
        if (set_as_new_view_state && (ViewStateKind::NEW_VIEW_STATE != view_state))
        {
            ++counters.instances_new;
            --counters.instances_not_new;
        }
    }

    void update_owner()
    {
        std::for_each(alive_writers.begin(), alive_writers.end(),
                [&](const WriterOwnership& item)
                {
                    if (item.second > current_owner.second)
                    {
                        current_owner = item;
                    }
                });
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_HISTORY_DATAREADERCACHECHANGE_HPP_
