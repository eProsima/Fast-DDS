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
 * @file OutputTrafficManager.hpp
 */

#ifndef _STATISTICS_RTPS_MESSAGES_OUTPUTTRAFFICMANAGER_HPP_
#define _STATISTICS_RTPS_MESSAGES_OUTPUTTRAFFICMANAGER_HPP_

#include <algorithm>
#include <cstdint>
#include <list>
#include <utility>

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/config.h>

#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

/**
 * A container for output locators sequencing information.
 * @note This is non-thread-safe class.
 */
class OutputTrafficManager
{
public:

    /**
     * Adds an output locator to the collection.
     * If the locator is already present in the collection, this method is a no-op.
     * @param locator The locator for which sequencing information should be kept.
     */
    inline void add_entry(
            const eprosima::fastrtps::rtps::Locator_t& locator)
    {
        static_cast<void>(locator);

#ifdef FASTDDS_STATISTICS
        auto search = [locator](const entry_type& entry) -> bool
                {
                    return locator == entry.first;
                };
        auto it = std::find_if(collection_.cbegin(), collection_.cend(), search);
        if (it == collection_.cend())
        {
            collection_.emplace_back(locator, value_type{});
        }
#endif // FASTDDS_STATISTICS
    }

    /**
     *
     */
    inline void set_statistics_message_data(
            const eprosima::fastrtps::rtps::Locator_t& locator,
            const eprosima::fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size)
    {
        static_cast<void>(locator);
        static_cast<void>(send_buffer);
        static_cast<void>(send_buffer_size);

#ifdef FASTDDS_STATISTICS
        auto search = [locator](const entry_type& entry) -> bool
                {
                    return locator == entry.first;
                };
        auto it = std::find_if(collection_.begin(), collection_.end(), search);
        if (it == collection_.end())
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT,
                    "Locator '" << locator << "' not found in collection. Adding entry.");
            it = collection_.insert(it, entry_type(locator, value_type{}));
        }
        set_statistics_submessage_from_transport(locator, send_buffer, send_buffer_size, it->second);
#endif // FASTDDS_STATISTICS
    }

#ifdef FASTDDS_STATISTICS

private:

    using key_type = eprosima::fastrtps::rtps::Locator_t;
    using value_type = StatisticsSubmessageData::Sequence;
    using entry_type = std::pair<key_type, value_type>;

    std::list<entry_type> collection_;
#endif // FASTDDS_STATISTICS

};

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif  // _STATISTICS_RTPS_MESSAGES_OUTPUTTRAFFICMANAGER_HPP_
