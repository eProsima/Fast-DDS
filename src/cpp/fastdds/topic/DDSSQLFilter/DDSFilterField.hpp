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

#include <cassert>
#include <unordered_set>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

#include "DDSFilterPredicate.hpp"
#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

/**
 * A DDSFilterValue for fieldname-based expression values.
 */
class DDSFilterField final : public DDSFilterValue
{

public:

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
     * @param [in]  type_id       TypeIdentifier representing the primitive data type of the fieldname.
     * @param [in]  access_path   Access path to the field.
     * @param [in]  data_kind     Kind of data the field represents.
     */
    DDSFilterField(
            const std::shared_ptr<xtypes::TypeIdentifier>& type_id,
            const std::vector<FieldAccessor>& access_path,
            ValueKind data_kind)
        : DDSFilterValue(data_kind)
        , access_path_(access_path)
        , type_id_(type_id)
    {
    }

    virtual ~DDSFilterField() = default;

    /**
     * This method is used by a DDSFilterPredicate to check if this DDSFilterField can be used.
     *
     * @return whether this DDSFilterField has a value that can be used on a predicate.
     */
    inline bool has_value() const noexcept final
    {
        return has_value_;
    }

    /**
     * Instruct this value to reset.
     */
    inline void reset() noexcept final
    {
        has_value_ = false;
    }

    /**
     * Perform the deserialization of the field represented by this DDSFilterField.
     * Will notify the predicates where this DDSFilterField is being used.
     *
     * @param [in]  data  The dynamic representation of the payload being filtered.
     *
     * @return Whether the deserialization process succeeded.
     *
     * @post Method @c has_value returns true.
     */
    inline bool set_value(
            DynamicData::_ref_type data_value)
    {
        return set_value(data_value, 0);
    }

    /**
     * Perform the deserialization of a specific step of the access path.
     *
     * @param [in]  data  The dynamic representation of the step being processed.
     * @param [in]  n     The step on the access path being processed.
     *
     * @return Whether the deserialization process succeeded.
     *
     * @post Method @c has_value returns true.
     */
    bool set_value(
            DynamicData::_ref_type data,
            size_t n);

protected:

    inline void add_parent(
            DDSFilterPredicate* parent) final
    {
        assert(nullptr != parent);
        parents_.emplace(parent);
    }

private:

    bool set_value_using_member_id(
            DynamicData::_ref_type data,
            MemberId member_id);

    bool has_value_ = false;
    std::vector<FieldAccessor> access_path_;
    const std::shared_ptr<xtypes::TypeIdentifier> type_id_;
    std::unordered_set<DDSFilterPredicate*> parents_;
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERFIELD_HPP_
