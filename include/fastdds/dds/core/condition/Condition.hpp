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
 * @file Condition.hpp
 *
 */

#ifndef FASTDDS_DDS_CORE_CONDITION__CONDITION_HPP
#define FASTDDS_DDS_CORE_CONDITION__CONDITION_HPP

#include <memory>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

// Forward declaration of implementation details
namespace detail {
struct ConditionNotifier;
} // namespace detail

/**
 * @brief The Condition class is the root base class for all the conditions that may be attached to a WaitSet.
 */
class Condition
{
public:

    /**
     * @brief Retrieves the trigger_value of the Condition
     * @return true if trigger_value is set to 'true', 'false' otherwise
     */
    FASTDDS_EXPORTED_API virtual bool get_trigger_value() const
    {
        EPROSIMA_LOG_WARNING(CONDITION, "get_trigger_value public member function not implemented");
        return false; // TODO return trigger value
    }

    detail::ConditionNotifier* get_notifier() const
    {
        return notifier_.get();
    }

protected:

    Condition();
    virtual ~Condition();

    std::unique_ptr<detail::ConditionNotifier> notifier_;
};

using ConditionSeq = std::vector<Condition*>;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE_CONDITION__CONDITION_HPP
