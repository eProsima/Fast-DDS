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

bool DDSFilterField::set_value(
        DynamicData::_ref_type data,
        size_t n)
{
    uint32_t index = static_cast<uint32_t>(access_path_[n].member_index);
    auto member_id = data->get_member_id_at_index(index);
    bool last_step = access_path_.size() - 1 == n;
    bool ret = false;

    if (access_path_[n].array_index < MEMBER_ID_INVALID)
    {
        DynamicData::_ref_type array_data = data->loan_value(member_id);
        if (array_data)
        {
            member_id = static_cast<MemberId>(access_path_[n].array_index);
            if (array_data->get_item_count() > member_id)
            {
                if (last_step)
                {
                    ret = set_value_using_member_id(array_data, member_id);
                }
                else
                {
                    DynamicData::_ref_type struct_data = array_data->loan_value(member_id);
                    if (struct_data)
                    {
                        ret = set_value(struct_data, n + 1);
                        array_data->return_loaned_value(struct_data);
                    }
                }
            }
            data->return_loaned_value(array_data);
        }
    }
    else
    {
        if (last_step)
        {
            ret = set_value_using_member_id(data, member_id);
        }
        else
        {
            DynamicData::_ref_type struct_data = data->loan_value(member_id);
            if (struct_data)
            {
                ret = set_value(struct_data, n + 1);
                data->return_loaned_value(struct_data);
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

bool DDSFilterField::set_value_using_member_id(
        DynamicData::_ref_type data,
        MemberId member_id)
{
    bool ret = false;
    switch (type_id_->_d())
    {
        case eprosima::fastdds::dds::xtypes::TK_BOOLEAN:
            ret = RETCODE_OK == data->get_boolean_value(boolean_value, member_id);
            break;

        case eprosima::fastdds::dds::xtypes::TK_CHAR8:
            ret = RETCODE_OK == data->get_char8_value(char_value, member_id);
            break;

        case eprosima::fastdds::dds::xtypes::TK_STRING8:
        case eprosima::fastdds::dds::xtypes::TI_STRING8_SMALL:
        case eprosima::fastdds::dds::xtypes::TI_STRING8_LARGE:
        {
            std::string tmp;
            ret = RETCODE_OK == data->get_string_value(tmp, member_id);
            string_value = tmp.c_str();
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_INT8:
        {
            int8_t value8 {0};
            ret = RETCODE_OK == data->get_int8_value(value8, member_id);
            signed_integer_value = value8;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_INT16:
        {
            int16_t value16 {0};
            ret = RETCODE_OK == data->get_int16_value(value16, member_id);
            signed_integer_value = value16;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_INT32:
        {
            int32_t value32 {0};
            ret = RETCODE_OK == data->get_int32_value(value32, member_id);
            signed_integer_value = value32;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_INT64:
            ret = RETCODE_OK == data->get_int64_value(signed_integer_value, member_id);
            break;

        case eprosima::fastdds::dds::xtypes::TK_BYTE:
        {
            fastdds::rtps::octet byte {0};
            ret = RETCODE_OK == data->get_byte_value(byte, member_id);
            unsigned_integer_value = byte;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_UINT8:
        {
            uint8_t valueu8 {0};
            ret = RETCODE_OK == data->get_uint8_value(valueu8, member_id);
            unsigned_integer_value = valueu8;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_UINT16:
        {
            uint16_t valueu16 {0};
            ret = RETCODE_OK == data->get_uint16_value(valueu16, member_id);
            unsigned_integer_value = valueu16;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_UINT32:
        {
            uint32_t valueu32 {0};
            ret = RETCODE_OK == data->get_uint32_value(valueu32, member_id);
            unsigned_integer_value = valueu32;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_UINT64:
            ret = RETCODE_OK == data->get_uint64_value(unsigned_integer_value, member_id);
            break;

        case eprosima::fastdds::dds::xtypes::TK_FLOAT32:
        {
            float valuef32 {0};
            ret = RETCODE_OK == data->get_float32_value(valuef32, member_id);
            float_value = valuef32;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_FLOAT64:
        {
            double valuef64 {0};
            ret = RETCODE_OK == data->get_float64_value(valuef64, member_id);
            float_value = valuef64;
        }
        break;

        case eprosima::fastdds::dds::xtypes::TK_FLOAT128:
            ret = RETCODE_OK == data->get_float128_value(float_value, member_id);
            break;

        case eprosima::fastdds::dds::xtypes::EK_COMPLETE:
        {
            int32_t valueenum {0};
            ret = RETCODE_OK == data->get_int32_value(valueenum, member_id);
            signed_integer_value = valueenum;
            break;
        }

        default:
            break;
    }

    return ret;
}

}  // namespace DDSSQLFilter

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
