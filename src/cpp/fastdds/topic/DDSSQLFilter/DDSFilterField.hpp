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

/**
 * A DDSFilterValue for fieldname-based expression values.
 */
struct DDSFilterField final : public DDSFilterValue
{
    /**
     * An element on the access path to the final field.
     */
    struct FieldAccessor final
    {
        /// Index of the member to access
        size_t member_index;

        /// Element index for array / sequence members
        size_t array_index;
    };

    /**
     * Construct a DDSFilterField.
     *
     * @param[in]  type_object   TypeObject representing the data type the fieldname belongs to.
     * @param[in]  access_path   Access path to the field.
     * @param[in]  data_kind     Kind of data the field represents.
     */
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

    /**
     * This method is used by a DDSFilterPredicate to check if this DDSFilterField can be used.
     *
     * @return whether this DDSFilterField has a value that can be used on a predicate.
     */
    bool has_value() const noexcept final
    {
        return has_value_;
    }

    /**
     * Instruct this value to reset.
     */
    void reset() noexcept final
    {
        has_value_ = false;
    }

    /**
     * Performs the deserialization of the field represented by this DDSFilterField.
     * Will notify the predicates where this DDSFilterField is being used.
     *
     * @param[in]  payload  The payload from where to deserialize this field.
     *
     * @return Whether the deserialization process succeeded
     *
     * @post Method @c has_value returns true.
     */
    bool set_value(
            const eprosima::fastrtps::rtps::SerializedPayload_t& payload)
    {
        // TODO: Set value from payload
        static_cast<void>(payload);

        has_value_ = nullptr != type_object_;

        // TODO: Inform parent predicates

        return true;
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
