// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <bitset>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include "json_dynamic_data.hpp"

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "../../dynamic_types/DynamicDataImpl.hpp"
#include "../../dynamic_types/DynamicTypeImpl.hpp"
#include "../../dynamic_types/DynamicTypeMemberImpl.hpp"
#include "../../dynamic_types/MemberDescriptorImpl.hpp"
#include "../../dynamic_types/TypeDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t json_deserialize(
        const nlohmann::json& j,
        const DynamicType::_ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    if (nullptr == dynamic_type)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null DynamicType value while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    // TODO: check dynamic_type kind is TK_STRUCT

    // TODO: should instead change API to return DynamicData and avoid cases such as passed data being non-null
    if (nullptr != data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "DynamicData to be filled must be null to perform JSON deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    // Create DynamicData instance through factory
    data = DynamicDataFactory::get_instance()->create_data(dynamic_type);

    if (nullptr == data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Failed to create DynamicData instance while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    if (j.is_null())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null JSON value while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    if (!j.is_object())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered invalid JSON object while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        // TODO: reuse code from json_deserialize_member (struct/bitset case, pay attention to logs) -> e.g. create json_deserialize_struct
        DynamicTypeMember::_ref_type type_member;
        if (RETCODE_OK != (ret = dynamic_type->get_member_by_name(type_member, it.key())))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing member '" << it.key() << "' from JSON: get_member_by_name failed.");
            break;
        }

        if (RETCODE_OK != (ret = json_deserialize_member(it.value(), type_member, format, data)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing member '" << it.key() << "' from JSON.");
            break;
        }
    }

    return ret;
}

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const traits<DynamicTypeMember>::ref_type& type_member,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    MemberDescriptorImpl& member_desc = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(type_member)->get_descriptor();
    return json_deserialize_member(j, type_member->get_id(), member_desc.type()->get_kind(), format, data);
}

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        // const DynamicType::_ref_type& dynamic_type,  // TODO: DynamicType or DynamicTypeMember?? Or not needed at all (could be obtained from data)??
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    switch (member_kind)
    {
        case TK_NONE:
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT8:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
        case TK_UINT8:
        case TK_UINT16:
        case TK_UINT32:
        case TK_UINT64:
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_CHAR8:
        case TK_CHAR16:
        case TK_STRING8:
        case TK_STRING16:
        case TK_ENUM:
        {
            return json_deserialize_basic_member(j, member_id, member_kind, format, data);
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            std::string kind_str = (member_kind == TK_STRUCTURE) ? "structure" : "bitset";
            traits<DynamicData>::ref_type st_data = data->loan_value(member_id);
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing " << kind_str << " member: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = RETCODE_OK;
            for (auto it = j.begin(); it != j.end(); ++it)
            {
                DynamicTypeMember::_ref_type type_member;
                if (RETCODE_OK != (ret = st_data->type()->get_member_by_name(type_member, it.key())))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing " << kind_str << " member '" << it.key() << "' from JSON: get_member_by_name failed.");
                    break;
                }

                if (RETCODE_OK != (ret = json_deserialize_member(it.value(), type_member, format, st_data)))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while deserializing " << kind_str << " member '" << it.key() << "'.");
                    break;
                }
            }

            // Return loaned value
            // NOTE: this should always be done, even if something went wrong before
            ReturnCode_t ret_return_loan;
            if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while returning " << kind_str << " loaned value.");
            }
            // Give priority to prior error if occurred
            return RETCODE_OK != ret ? ret : ret_return_loan;
        }
        // case TK_UNION:
        // {
        // }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            std::string kind_str = (member_kind == TK_SEQUENCE) ? "sequence" : "array";
            // traits<DynamicDataImpl>::ref_type st_data =
            //         traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            traits<DynamicData>::ref_type st_data = data->loan_value(member_id);
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing " << kind_str << " member: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = json_deserialize_collection(j, format, st_data);

            // Return loaned value
            // NOTE: this should always be done, even if something went wrong before
            ReturnCode_t ret_return_loan;
            if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while returning " << kind_str << " loaned value.");
            }
            // Give priority to prior error if occurred
            return RETCODE_OK != ret ? ret : ret_return_loan;
        }
        // case TK_MAP:
        // {
        // }
        // case TK_BITMASK:
        // {
        // }
        // case TK_ALIAS:
        // {
        // }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing member: unexpected kind " << member_kind <<
                    " found.");
            return RETCODE_BAD_PARAMETER;
    }

    return RETCODE_OK;
}

ReturnCode_t json_deserialize_basic_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    (void) format;
    switch (member_kind)
    {
        case TK_NONE:
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing basic member: unexpected TK_NONE kind.");
            return RETCODE_BAD_PARAMETER;
        }
        case TK_BOOLEAN:
        {
            ReturnCode_t ret = data->set_boolean_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_BOOLEAN member.");
            }
            return ret;
        }
        case TK_BYTE:
        {
            ReturnCode_t ret = data->set_byte_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_BYTE member.");
            }
            return ret;
        }
        case TK_INT8:
        {
            ReturnCode_t ret = data->set_int8_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT8 member.");
            }
            return ret;
        }
        case TK_INT16:
        {
            ReturnCode_t ret = data->set_int16_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT16 member.");
            }
            return ret;
        }
        case TK_INT32:
        {
            ReturnCode_t ret = data->set_int32_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT32 member.");
            }
            return ret;
        }
        case TK_INT64:
        {
            ReturnCode_t ret = data->set_int64_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT64 member.");
            }
            return ret;
        }
        case TK_UINT8:
        {
            ReturnCode_t ret = data->set_uint8_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT8 member.");
            }
            return ret;
        }
        case TK_UINT16:
        {
            ReturnCode_t ret = data->set_uint16_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT16 member.");
            }
            return ret;
        }
        case TK_UINT32:
        {
            ReturnCode_t ret = data->set_uint32_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT32 member.");
            }
            return ret;
        }
        case TK_UINT64:
        {
            ReturnCode_t ret = data->set_uint64_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT64 member.");
            }
            return ret;
        }
        case TK_FLOAT32:
        {
            ReturnCode_t ret = data->set_float32_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT32 member.");
            }
            return ret;
        }
        case TK_FLOAT64:
        {
            ReturnCode_t ret = data->set_float64_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT64 member.");
            }
            return ret;
        }
        case TK_FLOAT128:
        {
            ReturnCode_t ret = data->set_float128_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT128 member.");
            }
            return ret;
        }
        case TK_CHAR8:
        {
            if (!j.is_string())
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR8 member: expected string.");
                return RETCODE_BAD_PARAMETER;
            }

            std::string aux_string = j.get<std::string>();
            if (aux_string.size() != 1)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR8 member: expected single character.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = data->set_char8_value(member_id, aux_string[0]);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR8 member.");
            }
            return ret;
        }
        case TK_CHAR16:
        {
            // Convert UTF-8 value to wstring
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring aux_wstring = converter.from_bytes(j); // TODO: handle exception

            if (aux_wstring.size() != 1)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR16 member: expected single character.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = data->set_char16_value(member_id, aux_wstring[0]);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR16 member.");
            }
            return ret;
        }
        case TK_STRING8:
        {
            ReturnCode_t ret = data->set_string_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_STRING8 member.");
            }
            return ret;
        }
        case TK_STRING16:
        {
            // Convert UTF-8 value to wstring
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring value = converter.from_bytes(j); // TODO: handle exception

            ReturnCode_t ret = data->set_wstring_value(member_id, value);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_STRING16 member.");
            }
            return ret;
        }
        // case TK_ENUM:
        // {
        //     int32_t value;
        //     ObjectName enum_name;
        //     if (format == DynamicDataJsonFormat::OMG)
        //     {
        //         enum_name = j.to_string();  // TODO: what if this is trash? make resilient to this scenario
        //                                     // TODO: check that this method does not return the value with \"
        //     }
        //     else if (format == DynamicDataJsonFormat::EPROSIMA)
        //     {
        //         for (auto it = j.begin(); it != j.end(); ++it)
        //         {
        //             if ("name" == it.key())
        //             {
        //                 enum_name = it.value();
        //             }
        //             else if ("value" == it.key())
        //             {
        //                 value = it.value();
        //             }
        //             else
        //             {
        //                 EPROSIMA_LOG_ERROR(XTYPES_UTILS,
        //                         "Error encountered while deserializing TK_ENUM member: unexpected key '" << it.key() <<
        //                         "' found.");
        //                 return RETCODE_BAD_PARAMETER;
        //             }
        //         }
        //     }
        // }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing basic member to JSON: unexpected kind " << member_kind <<
                    " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_deserialize_collection(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    if (!j.is_array())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing collection member: expected JSON array.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    if (data->type()->get_kind() == TK_SEQUENCE)  // TODO: use enclosing type or similar for ALIAS case
    {
        // TODO: get bounds in case bounded and check size is not greater than that
        for (size_t i = 0; i < j.size(); ++i)
        {
            if (RETCODE_OK != (ret = json_deserialize_member(j[i], static_cast<MemberId>(i), traits<DynamicType>::narrow<DynamicTypeImpl>(data->type())->get_descriptor().element_type()->get_kind(), format, data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing sequence member.");
                break;
            }
        }
    }
    else
    {
        const TypeDescriptorImpl& descriptor = traits<DynamicType>::narrow<DynamicTypeImpl>(data->type())->get_descriptor();

        const BoundSeq& bounds = descriptor.bound();
        unsigned int index = 0;
        if (RETCODE_OK != (ret = json_deserialize_array(j, traits<DynamicType>::narrow<DynamicTypeImpl>(
                    descriptor.element_type())->resolve_alias_enclosed_type()->get_kind(), index, bounds, format, data)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing JSON array collection.");
        }
    }
    return ret;
}

ReturnCode_t json_deserialize_array(
        const nlohmann::json& j,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    assert(j.is_array());
    ReturnCode_t ret = RETCODE_OK;
    if (bounds.size() == 1)
    {
        assert(bounds[0] == j.size());
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_deserialize_member(j[i], static_cast<MemberId>(index++), member_kind, format, data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing JSON array element.");
                break;
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_deserialize_array(j[i], member_kind, index,
                    std::vector<unsigned int>(bounds.begin() + 1, bounds.end()), format, data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing JSON array's array element.");
                break;
            }
        }
    }
    return ret;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
