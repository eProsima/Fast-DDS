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
 * @file DDSFilterField.cpp
 */

#include "DDSFilterField.hpp"

#include <cassert>
#include <unordered_set>
#include <vector>

#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypesBase.h>

#include "DDSFilterPredicate.hpp"
#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

bool DDSFilterField::set_value(
        eprosima::fastrtps::types::DynamicData& data,
        size_t n)
{
    using namespace eprosima::fastrtps::types;

    uint32_t index = static_cast<uint32_t>(access_path_[n].member_index);
    auto member_id = data.get_member_id_at_index(index);
    bool last_step = access_path_.size() - 1 == n;
    bool ret = false;

    if (access_path_[n].array_index < MEMBER_ID_INVALID)
    {
        DynamicData* array_data = data.loan_value(member_id);
        if (nullptr != array_data)
        {
            member_id = static_cast<MemberId>(access_path_[n].array_index);
            if (array_data->get_item_count() > member_id)
            {
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
                        array_data->return_loaned_value(struct_data);
                    }
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

bool DDSFilterField::set_value(
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
                // WARNING: this assumes EK_COMPLETE is always an enumeration, aliases should be resolved when parsing
                uint32_t enum_value;
                ret = !!data->get_enum_value(enum_value, member_id);
                signed_integer_value = enum_value;
            }
            break;

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

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
