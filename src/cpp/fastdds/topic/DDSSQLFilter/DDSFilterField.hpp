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
 * @file DDSFilterField.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERFIELD_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERFIELD_HPP_

#include <vector>

#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/TypeObject.h>

#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterField final : public DDSFilterValue
{
    struct FieldAccessor final
    {
        size_t member_index;
        size_t array_index;
    };

    DDSFilterField(
            const eprosima::fastrtps::types::TypeObject* type_object,
            const std::vector<FieldAccessor>& access_path,
            ValueKind data_kind)
        : DDSFilterValue(data_kind)
        , access_path_(access_path)
        , type_object_(type_object)
    {
    }

    virtual ~DDSFilterField() = default;

    bool has_value() const noexcept final
    {
        return has_value_;
    }

    void reset() noexcept final
    {
        has_value_ = false;
    }

    void set_value(
            const eprosima::fastrtps::rtps::SerializedPayload_t& payload)
    {
        // TODO: Set value from payload
        static_cast<void>(payload);

        has_value_ = true;

        // TODO: Inform parent predicates
    }

private:

    bool has_value_ = false;
    std::vector<FieldAccessor> access_path_;
    const eprosima::fastrtps::types::TypeObject* type_object_ = nullptr;
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERFIELD_HPP_
