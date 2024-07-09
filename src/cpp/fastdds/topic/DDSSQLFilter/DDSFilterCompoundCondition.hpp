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
 * @file DDSFilterCompoundCondition.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCOMPOUNDCONDITION_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCOMPOUNDCONDITION_HPP_

#include <memory>

#include "DDSFilterCondition.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

/**
 * A DDSFilterCondition that performs a logical operation over one or two DDSFilterCondition objects.
 */
class DDSFilterCompoundCondition final : public DDSFilterCondition
{

public:

    /**
     * Possible kinds of logical operations
     */
    enum class OperationKind : uint8_t
    {
        NOT,  ///< NOT left
        AND,  ///< left AND right
        OR    ///< left OR right
    };

    /**
     * Construct a DDSFilterCompoundCondition.
     *
     * @param [in]  op     Operation to perform.
     * @param [in]  left   Left operand.
     * @param [in]  right  Right operand.
     */
    DDSFilterCompoundCondition(
            OperationKind op,
            std::unique_ptr<DDSFilterCondition>&& left,
            std::unique_ptr<DDSFilterCondition>&& right);

    virtual ~DDSFilterCompoundCondition() = default;

protected:

    void propagate_reset() noexcept final;

    void child_has_changed(
            const DDSFilterCondition& child) noexcept final;

private:

    OperationKind op_;
    std::unique_ptr<DDSFilterCondition> left_;
    std::unique_ptr<DDSFilterCondition> right_;
    uint8_t num_children_decided_ = 0;

};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCOMPOUNDCONDITION_HPP_
