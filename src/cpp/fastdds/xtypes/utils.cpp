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
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "dynamic_types/DynamicDataImpl.hpp"
#include "dynamic_types/DynamicTypeImpl.hpp"
#include "dynamic_types/DynamicTypeMemberImpl.hpp"
#include "dynamic_types/MemberDescriptorImpl.hpp"
#include "dynamic_types/TypeDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

////////////////////////////////////////
// Dynamic Data to JSON serialization //
////////////////////////////////////////

//// Forward declarations

static ReturnCode_t json_serialize(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const traits<DynamicTypeMember>::ref_type& type_member,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_basic_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_collection(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept;

static ReturnCode_t json_serialize_array(
        const traits<DynamicDataImpl>::ref_type& data,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        nlohmann::json& j_array,
        DynamicDataJsonFormat format) noexcept;

template <typename T>
static void json_insert(
        const std::string& key,
        const T& value,
        nlohmann::json& j);

//// Implementation

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        std::ostream& output,
        DynamicDataJsonFormat format) noexcept
{
    ReturnCode_t ret;
    nlohmann::json j;
    if (RETCODE_OK == (ret = json_serialize(traits<DynamicData>::narrow<DynamicDataImpl>(data), j, format)))
    {
        output << j;
    }
    else
    {
        EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while performing DynamicData to JSON serialization.");
    }
    return ret;
}

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        std::string& output,
        DynamicDataJsonFormat format) noexcept
{
    ReturnCode_t ret;
    std::stringstream ss;

    if (RETCODE_OK == (ret = json_serialize(data, ss, format)))
    {
        output = ss.str();
    }
    else
    {
        EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while performing DynamicData to JSON serialization.");
    }
    return ret;
}

ReturnCode_t json_serialize(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    if (nullptr != data)
    {
        switch (data->type()->get_kind())
        {
            case TK_STRUCTURE:
            {
                ReturnCode_t ret = RETCODE_OK;
                DynamicTypeMembersById members;
                if (RETCODE_OK != (ret = data->type()->get_all_members(members)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                            "Error encountered while serializing structure to JSON: get_all_members failed.");
                    return ret;
                }
                for (auto it : members)
                {
                    if (RETCODE_OK != (ret = json_serialize_member(data, it.second, output, format)))
                    {
                        EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                                "Error encountered while serializing structure member to JSON.");
                        break;
                    }
                }
                return ret;
            }
            default:
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Only structs are supported by json_serialize method.");
                return RETCODE_BAD_PARAMETER;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                "Encountered null data value while performing DynamicData to JSON serialization.");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const traits<DynamicTypeMember>::ref_type& type_member,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    MemberDescriptorImpl& member_desc =
            traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(type_member)->get_descriptor();

    return json_serialize_member(data, type_member->get_id(),
                   traits<DynamicType>::narrow<DynamicTypeImpl>(
                       member_desc.type())->resolve_alias_enclosed_type()->get_kind(),
                   type_member->get_name().to_string(), output, format);
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
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
        case TK_BITMASK:
        {
            return json_serialize_basic_member(data, member_id, member_kind, member_name, output, format);
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr != st_data)
            {
                ReturnCode_t ret = RETCODE_OK;
                nlohmann::json j_struct;
                DynamicTypeMembersById members;
                if (RETCODE_OK != (ret = st_data->enclosing_type()->get_all_members(members)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                            "Error encountered while serializing structure/bitset member to JSON: get_all_members failed.");
                }
                else
                {
                    for (auto it : members)
                    {
                        if (RETCODE_OK != (ret = json_serialize_member(st_data, it.second, j_struct, format)))
                        {
                            EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                                    "Error encountered while serializing structure/bitset member's member to JSON.");
                            break;
                        }
                    }
                }
                if (RETCODE_OK == ret)
                {
                    json_insert(member_name, j_struct, output);
                }
                // Return loaned value
                ReturnCode_t ret_return_loan;
                if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while returning loaned value.");
                }
                // Give priority to prior error if occurred
                if (RETCODE_OK != ret)
                {
                    return ret;
                }
                else
                {
                    return ret_return_loan;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing structure/bitset member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_UNION:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr != st_data)
            {
                ReturnCode_t ret = RETCODE_OK;
                nlohmann::json j_union;

                DynamicTypeMember::_ref_type active_type_member;
                if (RETCODE_OK !=
                        (ret =
                        st_data->enclosing_type()->get_member(active_type_member,
                        st_data->selected_union_member())))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                            "Error encountered while serializing union member to JSON: get_member failed.");
                }
                else
                {
                    if (RETCODE_OK == (ret = json_serialize_member(st_data, active_type_member, j_union, format)))
                    {
                        json_insert(member_name, j_union, output);
                    }
                }
                // Return loaned value
                ReturnCode_t ret_return_loan;
                if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while returning loaned value.");
                }
                // Give priority to prior error if occurred
                if (RETCODE_OK != ret)
                {
                    return ret;
                }
                else
                {
                    return ret_return_loan;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing union member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr != st_data)
            {
                ReturnCode_t ret = json_serialize_collection(st_data, member_name, output, format);
                // Return loaned value
                ReturnCode_t ret_return_loan;
                if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while returning loaned value.");
                }
                // Give priority to prior error if occurred
                if (RETCODE_OK != ret)
                {
                    return ret;
                }
                else
                {
                    return ret_return_loan;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing sequence/array member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_MAP:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr != st_data)
            {
                ReturnCode_t ret = RETCODE_OK;
                nlohmann::json j_map;
                TypeDescriptorImpl& map_desc = st_data->enclosing_type()->get_descriptor();
                traits<DynamicTypeImpl>::ref_type key_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
                    map_desc.key_element_type())->resolve_alias_enclosed_type();
                traits<DynamicTypeImpl>::ref_type value_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
                    map_desc.element_type())->resolve_alias_enclosed_type();
                uint32_t size = st_data->get_item_count();
                for (uint32_t i = 0; i < size; ++i)
                {
                    // TODO: use actual map key as dictionary key once available in API
                    MemberId id = st_data->get_member_id_at_index(i);
                    if (RETCODE_OK !=
                            (ret =
                            json_serialize_member(st_data, id, value_type->get_kind(), std::to_string(id), j_map,
                            format)))
                    {
                        EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                                "Error encountered while serializing map member'S member to JSON.");
                        break;
                    }
                }
                if (RETCODE_OK == ret)
                {
                    json_insert(member_name, j_map, output);
                }
                // Return loaned value
                ReturnCode_t ret_return_loan;
                if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while returning loaned value.");
                }
                // Give priority to prior error if occurred
                if (RETCODE_OK != ret)
                {
                    return ret;
                }
                else
                {
                    return ret_return_loan;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing map member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_ALIAS:
        {
            EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                    "Error encountered while serializing member to JSON: unexpected TK_ALIAS kind.");
            return RETCODE_BAD_PARAMETER;
        }
        default:
            EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                    "Error encountered while serializing map member to JSON: unexpected kind " << member_kind <<
                    " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    return json_serialize_member(data, member_id, member_kind, "", output, format);
}

ReturnCode_t json_serialize_basic_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    switch (member_kind)
    {
        case TK_NONE:
        {
            EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                    "Error encountered while serializing basic member to JSON: unexpected TK_NONE kind.");
            return RETCODE_BAD_PARAMETER;
        }
        case TK_BOOLEAN:
        {
            ReturnCode_t ret;
            bool value;
            if (RETCODE_OK == (ret = data->get_boolean_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_BOOLEAN member to JSON.");
            }
            return ret;
        }
        case TK_BYTE:
        {
            ReturnCode_t ret;
            eprosima::fastrtps::rtps::octet value;
            if (RETCODE_OK == (ret = data->get_byte_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_BYTE member to JSON.");
            }
            return ret;
        }
        case TK_INT8:
        {
            ReturnCode_t ret;
            int8_t value;
            if (RETCODE_OK == (ret = data->get_int8_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_INT8 member to JSON.");
            }
            return ret;
        }
        case TK_INT16:
        {
            ReturnCode_t ret;
            int16_t value;
            if (RETCODE_OK == (ret = data->get_int16_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_INT16 member to JSON.");
            }
            return ret;
        }
        case TK_INT32:
        {
            ReturnCode_t ret;
            int32_t value;
            if (RETCODE_OK == (ret = data->get_int32_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_INT32 member to JSON.");
            }
            return ret;
        }
        case TK_INT64:
        {
            ReturnCode_t ret;
            int64_t value;
            if (RETCODE_OK == (ret = data->get_int64_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_INT64 member to JSON.");
            }
            return ret;
        }
        case TK_UINT8:
        {
            ReturnCode_t ret;
            uint8_t value;
            if (RETCODE_OK == (ret = data->get_uint8_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_UINT8 member to JSON.");
            }
            return ret;
        }
        case TK_UINT16:
        {
            ReturnCode_t ret;
            uint16_t value;
            if (RETCODE_OK == (ret = data->get_uint16_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_UINT16 member to JSON.");
            }
            return ret;
        }
        case TK_UINT32:
        {
            ReturnCode_t ret;
            uint32_t value;
            if (RETCODE_OK == (ret = data->get_uint32_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_UINT32 member to JSON.");
            }
            return ret;
        }
        case TK_UINT64:
        {
            ReturnCode_t ret;
            uint64_t value;
            if (RETCODE_OK == (ret = data->get_uint64_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_UINT64 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT32:
        {
            ReturnCode_t ret;
            float value;
            if (RETCODE_OK == (ret = data->get_float32_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT32 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT64:
        {
            ReturnCode_t ret;
            double value;
            if (RETCODE_OK == (ret = data->get_float64_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT64 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT128:
        {
            ReturnCode_t ret;
            long double value;
            if (RETCODE_OK == (ret = data->get_float128_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT128 member to JSON.");
            }
            return ret;
        }
        case TK_CHAR8:
        {
            ReturnCode_t ret;
            char value;
            if (RETCODE_OK == (ret = data->get_char8_value(value, member_id)))
            {
                std::string aux_string_value({value});
                json_insert(member_name, aux_string_value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_CHAR8 member to JSON.");
            }
            return ret;
        }
        case TK_CHAR16:
        {
            ReturnCode_t ret;
            wchar_t value;
            if (RETCODE_OK == (ret = data->get_char16_value(value, member_id)))
            {
                // Insert UTF-8 converted value
                std::wstring aux_wstring_value({value});
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                std::string utf8_value = converter.to_bytes(aux_wstring_value);
                json_insert(member_name, utf8_value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_CHAR16 member to JSON.");
            }
            return ret;
        }
        case TK_STRING8:
        {
            ReturnCode_t ret;
            std::string value;
            if (RETCODE_OK == (ret = data->get_string_value(value, member_id)))
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_STRING8 member to JSON.");
            }
            return ret;
        }
        case TK_STRING16:
        {
            ReturnCode_t ret;
            std::wstring value;
            if (RETCODE_OK == (ret = data->get_wstring_value(value, member_id)))
            {
                // Insert UTF-8 converted value
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                std::string utf8_value = converter.to_bytes(value);
                json_insert(member_name, utf8_value, output);
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_STRING16 member to JSON.");
            }
            return ret;
        }
        case TK_ENUM:
        {
            ReturnCode_t ret;
            int32_t value;
            if (RETCODE_OK != (ret = data->get_int32_value(value, member_id)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing TK_ENUM member to JSON.");
                return ret;
            }

            MemberDescriptor::_ref_type enum_desc{traits<MemberDescriptor>::make_shared()};
            if (RETCODE_OK != (ret = data->get_descriptor(enum_desc, member_id)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing TK_ENUM member to JSON: get_descriptor failed.");
                return ret;
            }

            DynamicTypeMembersByName all_members;
            if (RETCODE_OK !=
                    (ret =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(enum_desc->type())->resolve_alias_enclosed_type()
                            ->
                            get_all_members_by_name(all_members)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing TK_ENUM member to JSON: get_all_members_by_name failed.");
                return ret;
            }

            ObjectName name;
            ret = RETCODE_BAD_PARAMETER;
            for (auto it : all_members)
            {
                MemberDescriptorImpl& enum_member_desc = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                    it.second)->get_descriptor();
                if (enum_member_desc.default_value() == std::to_string(value))
                {
                    name = it.first;
                    assert(name == it.second->get_name());
                    ret = RETCODE_OK;
                    break;
                }
            }
            if (RETCODE_OK == ret)
            {
                if (format == DynamicDataJsonFormat::OMG)
                {
                    json_insert(member_name, name, output);
                }
                else if (format == DynamicDataJsonFormat::EPROSIMA)
                {
                    nlohmann::json enum_dict = {{"name", name}, {"value", value}};
                    json_insert(member_name, enum_dict, output);
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing TK_ENUM member to JSON: enum value not found.");
            }
            return ret;
        }
        case TK_BITMASK:
        {
            ReturnCode_t ret;

            MemberDescriptor::_ref_type bitmask_member_desc{traits<MemberDescriptor>::make_shared()};
            if (RETCODE_OK != (ret = data->get_descriptor(bitmask_member_desc, member_id)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing TK_BITMASK member to JSON: get_descriptor failed.");
                return ret;
            }

            traits<DynamicTypeImpl>::ref_type bitmask_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
                bitmask_member_desc->type())->resolve_alias_enclosed_type();
            TypeDescriptorImpl& bitmask_desc = bitmask_type->get_descriptor();
            auto bound = bitmask_desc.bound().at(0);

            if (format == DynamicDataJsonFormat::OMG)
            {
                if (9 > bound)
                {
                    uint8_t value;
                    if (RETCODE_OK == (ret = data->get_uint8_value(value, member_id)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else if (17 > bound)
                {
                    uint16_t value;
                    if (RETCODE_OK == (ret = data->get_uint16_value(value, member_id)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else if (33 > bound)
                {
                    uint32_t value;
                    if (RETCODE_OK == (ret = data->get_uint32_value(value, member_id)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else
                {
                    uint64_t value;
                    if (RETCODE_OK == (ret = data->get_uint64_value(value, member_id)))
                    {
                        json_insert(member_name, value, output);
                    }
                }

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                            "Error encountered while serializing TK_BITMASK member to JSON: failed to get value.");
                }
            }
            else if (format == DynamicDataJsonFormat::EPROSIMA)
            {
                nlohmann::json bitmask_dict;
                uint64_t u64_value; // Auxiliar variable to check active bits afterwards
                if (9 > bound)
                {
                    uint8_t value;
                    if (RETCODE_OK == (ret = data->get_uint8_value(value, member_id)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<8>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else if (17 > bound)
                {
                    uint16_t value;
                    if (RETCODE_OK == (ret = data->get_uint16_value(value, member_id)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<16>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else if (33 > bound)
                {
                    uint32_t value;
                    if (RETCODE_OK == (ret = data->get_uint32_value(value, member_id)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<32>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else
                {
                    uint64_t value;
                    if (RETCODE_OK == (ret = data->get_uint64_value(value, member_id)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<64>(value).to_string();
                        u64_value = value;
                    }
                }

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                            "Error encountered while serializing TK_BITMASK member to JSON: failed to get value.");
                }
                else
                {
                    // Check active bits
                    DynamicTypeMembersById bitmask_members;
                    if (RETCODE_OK != (ret = bitmask_type->get_all_members(bitmask_members)))
                    {
                        EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                                "Error encountered while serializing TK_BITMASK member to JSON: get_all_members failed.");
                    }
                    else
                    {
                        std::vector<std::string> active_bits;
                        for (auto it : bitmask_members)
                        {
                            if (u64_value & (0x01ull << it.second->get_id()))
                            {
                                active_bits.push_back(it.second->get_name().to_string());
                            }
                        }
                        bitmask_dict["active"] = active_bits;

                        // Insert custom bitmask value
                        json_insert(member_name, bitmask_dict, output);
                    }
                }
            }
            return ret;
        }
        default:
            // logWarning
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_collection(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    if (data->enclosing_type()->get_kind() == TK_SEQUENCE)
    {
        TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();

        auto count = data->get_item_count();
        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t index = 0; index < count; ++index)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index),
                    traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())->get_kind(), j_array,
                    format)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing sequence collection to JSON.");
                break;
            }
        }
        if (RETCODE_OK == ret)
        {
            json_insert(member_name, j_array, output);
        }
    }
    else
    {
        TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();

        const BoundSeq& bounds = descriptor.bound();
        nlohmann::json j_array = nlohmann::json::array();
        unsigned int index = 0;
        if (RETCODE_OK != (ret = json_serialize_array(data, traits<DynamicType>::narrow<DynamicTypeImpl>(
                    descriptor.element_type())->get_kind(), index, bounds, j_array, format)))
        {
            EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing array collection to JSON.");
        }
        else
        {
            json_insert(member_name, j_array, output);
        }
    }
    return ret;
}

ReturnCode_t json_serialize_array(
        const traits<DynamicDataImpl>::ref_type& data,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        nlohmann::json& j_array,
        DynamicDataJsonFormat format) noexcept
{
    assert(j_array.is_array());
    ReturnCode_t ret = RETCODE_OK;
    if (bounds.size() == 1)
    {
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index++), member_kind, j_array,
                    format)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS, "Error encountered while serializing array element to JSON.");
                break;
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            nlohmann::json inner_array = nlohmann::json::array();
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_array(data, member_kind, index,
                    std::vector<unsigned int>(bounds.begin() + 1, bounds.end()), inner_array, format)))
            {
                EPROSIMA_LOG_WARNING(XTYPES_UTILS,
                        "Error encountered while serializing array's array element to JSON.");
                break;
            }
            j_array.push_back(inner_array);
        }
    }
    return ret;
}

template <typename T>
void json_insert(
        const std::string& key,
        const T& value,
        nlohmann::json& j)
{
    if (j.is_array())
    {
        j.push_back(value);
    }
    else
    {
        j[key] = value;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Dynamic Data to JSON serialization //// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}         // namespace dds
}     // namespace fastdds
} // namespace eprosima
