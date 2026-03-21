// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReadCondition.hpp
 */

#ifndef FASTDDS_DDS_SUBSCRIBER__READCONDITION_HPP
#define FASTDDS_DDS_SUBSCRIBER__READCONDITION_HPP

#include <cassert>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace detail {

class ReadConditionImpl;

} // namespace detail

class DataReader;

/**
 * @brief A Condition specifically dedicated to read operations and attached to one DataReader.
 *
 * ReadCondition objects allow an application to specify the data samples it is interested in (by specifying the
 * desired sample_states, view_states, and instance_states).
 * The condition will only be triggered when suitable information is available.
 * They are to be used in conjunction with a WaitSet as normal conditions.
 * More than one ReadCondition may be attached to the same DataReader.
 */
class ReadCondition : public Condition
{
    friend class detail::ReadConditionImpl;

public:

    ReadCondition();

    ~ReadCondition() override;

    // Non-copyable
    ReadCondition(
            const ReadCondition&) = delete;
    ReadCondition& operator =(
            const ReadCondition&) = delete;

    // Non-movable
    ReadCondition(
            ReadCondition&&) = delete;
    ReadCondition& operator =(
            ReadCondition&&) = delete;

    /**
     * @brief Retrieves the trigger_value of the Condition
     * @return true if trigger_value is set to 'true', 'false' otherwise
     */
    FASTDDS_EXPORTED_API bool get_trigger_value() const noexcept override;

    /**
     * @brief Retrieves the DataReader associated with the ReadCondition.
     *
     * Note that there is exactly one DataReader associated with each ReadCondition.
     *
     * @return pointer to the DataReader associated with this ReadCondition.
     */
    FASTDDS_EXPORTED_API DataReader* get_datareader() const noexcept;

    /**
     * @brief Retrieves the set of sample_states taken into account to determine the trigger_value of this condition.
     *
     * @return the sample_states specified when the ReadCondition was created.
     */
    FASTDDS_EXPORTED_API SampleStateMask get_sample_state_mask() const noexcept;

    /**
     * @brief Retrieves the set of view_states taken into account to determine the trigger_value of this condition.
     *
     * @return the view_states specified when the ReadCondition was created.
     */
    FASTDDS_EXPORTED_API ViewStateMask get_view_state_mask() const noexcept;

    /**
     * @brief Retrieves the set of instance_states taken into account to determine the trigger_value of this condition.
     *
     * @return the instance_states specified when the ReadCondition was created.
     */
    FASTDDS_EXPORTED_API InstanceStateMask get_instance_state_mask() const noexcept;

    detail::ReadConditionImpl* get_impl() const noexcept
    {
        assert((bool)impl_);
        return impl_.get();
    }

protected:

    //! Class implementation
    std::shared_ptr<detail::ReadConditionImpl> impl_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER__READCONDITION_HPP
