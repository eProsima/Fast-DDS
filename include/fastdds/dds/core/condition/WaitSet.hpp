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
 * @file WaitSet.hpp
 *
 */

#ifndef FASTDDS_DDS_CORE_CONDITION__WAITSET_HPP
#define FASTDDS_DDS_CORE_CONDITION__WAITSET_HPP

#include <memory>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

// Forward declaration of implementation details
namespace detail {
struct WaitSetImpl;
} // namespace detail

/**
 * @brief The WaitSet class allows an application to wait until one or more of the attached Condition objects
 * has a trigger_value of TRUE or until timeout expires.
 *
 */
class WaitSet
{
public:

    FASTDDS_EXPORTED_API WaitSet();

    FASTDDS_EXPORTED_API ~WaitSet();

    WaitSet(
            const WaitSet&) = delete;
    WaitSet(
            WaitSet&&) = delete;
    WaitSet& operator = (
            const WaitSet&) = delete;
    WaitSet& operator = (
            WaitSet&&) = delete;

    /**
     * @brief Attaches a Condition to the Wait Set.
     * @param cond Condition
     * @return RETCODE_OK if attached correctly, error code otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t attach_condition(
            const Condition& cond);


    /**
     * @brief Detaches a Condition from the WaitSet
     * @param cond Condition
     * @return RETCODE_OK if detached correctly, PRECONDITION_NOT_MET if condition was not attached
     */
    FASTDDS_EXPORTED_API ReturnCode_t detach_condition(
            const Condition& cond);

    /**
     * @brief Allows an application thread to wait for the occurrence of certain conditions.
     * If none of the conditions attached to the WaitSet have a trigger_value of true,
     * the wait operation will block suspending the calling thread
     * @param active_conditions Reference to the collection of conditions which trigger_value are true
     * @param timeout Maximum time of the wait
     * @return RETCODE_OK if everything correct, PRECONDITION_NOT_MET if WaitSet already waiting, TIMEOUT if maximum
     * time expired, error code otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t wait(
            ConditionSeq& active_conditions,
            const dds::Duration_t timeout) const;

    /**
     * @brief Retrieves the list of attached conditions
     * @param attached_conditions Reference to the collection of attached conditions
     * @return RETCODE_OK if everything correct, error code otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_conditions(
            ConditionSeq& attached_conditions) const;

private:

    std::unique_ptr<detail::WaitSetImpl> impl_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE_CONDITION__WAITSET_HPP
