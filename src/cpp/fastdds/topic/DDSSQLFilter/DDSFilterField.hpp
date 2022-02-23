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

#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/TypeObject.h>

#include "DDSFilterPredicate.hpp"
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
            const eprosima::fastrtps::types::TypeIdentifier* type_id,
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
     * @param[in]  data  The deserialization of the payload being filtered.
     *
     * @return Whether the deserialization process succeeded
     *
     * @post Method @c has_value returns true.
     */
    bool set_value(
            eprosima::fastrtps::types::DynamicData& data_value)
    {
        return set_value(data_value, 0);
    }

    bool set_value(
            eprosima::fastrtps::types::DynamicData& data,
            size_t n)
    {
        using namespace eprosima::fastrtps::types;

        uint32_t index = static_cast<uint32_t>(access_path_[n].member_index);
        auto member_id = data.get_member_id_at_index(index);
        bool last_step = access_path_.size() - 1 == n;
        bool ret = false;

        if (access_path_[n].array_index < std::numeric_limits<MemberId>::max())
        {
            DynamicData* array_data = data.loan_value(member_id);
            if (nullptr != array_data)
            {
                member_id = static_cast<MemberId>(access_path_[n].array_index);
                if (last_step)
                {
                    ret = set_value(array_data, member_id);
                }
                else
                {
                    DynamicData* struct_data = array_data->loan_value(member_id);
                    if (nullptr != struct_data)
                    {
                        ret = set_value(*struct_data, n + 1);
                        data.return_loaned_value(struct_data);
                    }
                }
                data.return_loaned_value(array_data);
            }
        }
        else
        {
            if (last_step)
            {
                ret = set_value(&data, member_id);
            }
            else
            {
                DynamicData* struct_data = data.loan_value(member_id);
                if (nullptr != struct_data)
                {
                    ret = set_value(*struct_data, n + 1);
                    data.return_loaned_value(struct_data);
                }
            }
        }

        if (ret && last_step)
        {
            has_value_ = true;
            value_has_changed();

            // Inform parent predicates
            for (DDSFilterPredicate* parent : parents_)
            {
                parent->value_has_changed();
            }
        }

        return ret;
    }

protected:

    void add_parent(
            DDSFilterPredicate* parent) final
    {
        assert(nullptr != parent);
        parents_.emplace(parent);
    }

private:

    bool set_value(
            const eprosima::fastrtps::types::DynamicData* data,
            eprosima::fastrtps::types::MemberId member_id)
    {
        using namespace eprosima::fastrtps::types;

        bool ret = true;
        try
        {
            switch (type_id_->_d())
            {
                case TK_BOOLEAN:
                    boolean_value = data->get_bool_value(member_id);
                    break;

                case TK_CHAR8:
                    char_value = data->get_char8_value(member_id);
                    break;

                case TK_STRING8:
                case TI_STRING8_SMALL:
                case TI_STRING8_LARGE:
                    string_value = data->get_string_value(member_id);
                    break;

                case TK_INT16:
                    signed_integer_value = data->get_int16_value(member_id);
                    break;

                case TK_INT32:
                    signed_integer_value = data->get_int32_value(member_id);
                    break;

                case TK_INT64:
                    signed_integer_value = data->get_int64_value(member_id);
                    break;

                case TK_BYTE:
                    unsigned_integer_value = data->get_uint8_value(member_id);
                    break;

                case TK_UINT16:
                    unsigned_integer_value = data->get_uint16_value(member_id);
                    break;

                case TK_UINT32:
                    unsigned_integer_value = data->get_uint32_value(member_id);
                    break;

                case TK_UINT64:
                    unsigned_integer_value = data->get_uint64_value(member_id);
                    break;

                case TK_FLOAT32:
                    float_value = data->get_float32_value(member_id);
                    break;

                case TK_FLOAT64:
                    float_value = data->get_float64_value(member_id);
                    break;

                case TK_FLOAT128:
                    float_value = data->get_float128_value(member_id);
                    break;

                case EK_COMPLETE:
                {
                    uint32_t enum_value;
                    ret = !!data->get_enum_value(enum_value, member_id);
                    signed_integer_value = enum_value;
                    break;
                }

                default:
                    ret = false;
                    break;
            }
        }
        catch (...)
        {
            ret = false;
        }

        return ret;
    }

    bool has_value_ = false;
    std::vector<FieldAccessor> access_path_;
    const eprosima::fastrtps::types::TypeIdentifier* type_id_ = nullptr;
    std::unordered_set<DDSFilterPredicate*> parents_;
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERFIELD_HPP_
