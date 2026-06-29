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
#ifndef MINGW_COMPILER
    #include <codecvt>
#endif  // ifndef MINGW_COMPILER
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include "dynamic_data_json.hpp"

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
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

// Build a compact label that replaces a collection data content
// (usage example: collapse a collection exceeding a size threshold).
static ReturnCode_t collection_label(
        const DynamicType::_ref_type& collection_type,
        uint32_t count,
        std::string& out) noexcept
{
    if (nullptr == collection_type)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Failed to build collection label for null type.");
        return RETCODE_BAD_PARAMETER;
    }

    const TypeKind kind = collection_type->get_kind();

    if (TK_ARRAY == kind)
    {
        // type_to_string already renders "element[d1,d2,...]"; just wrap it.
        std::string body;
        ReturnCode_t ret = type_to_string(collection_type, body, 1);
        if (RETCODE_OK == ret)
        {
            out = "<array: " + body + ">";
        }
        return ret;
    }

    if (TK_SEQUENCE == kind)
    {
        // Sequences report their runtime item count, which is not part of the type:
        // describe the element and splice the count in -> "<sequence: N element>".
        const auto element_type =
                traits<DynamicType>::narrow<DynamicTypeImpl>(
            collection_type)->get_descriptor().element_type();
        std::string element;
        ReturnCode_t ret = type_to_string(element_type, element, 1);
        if (RETCODE_OK == ret)
        {
            out = "<sequence: " + std::to_string(count) + " " + element + ">";
        }
        return ret;
    }

    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Failed to build collection label for non-collection type (" << kind << ").");
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t json_serialize(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    if (nullptr == data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null data value while performing DynamicData to JSON serialization.");
        return RETCODE_BAD_PARAMETER;
    }

    if (TK_STRUCTURE != data->type()->get_kind())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Only structs are supported by json_serialize method.");
        return RETCODE_BAD_PARAMETER;
    }

    return json_serialize_aggregate(data, output, format);
}

ReturnCode_t json_serialize(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    FormatOptions options;
    options.mapping = (format == DynamicDataJsonFormat::OMG)
            ? DynamicDataJsonMapping::OMG
            : DynamicDataJsonMapping::EPROSIMA;
    return json_serialize(data, output, options);
}

ReturnCode_t json_serialize_aggregate(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    std::string kind_str = (data->enclosing_type()->get_kind() == TK_STRUCTURE) ? "structure" : "bitset";
    DynamicTypeMembersById members;
    ReturnCode_t ret = data->enclosing_type()->get_all_members(members);
    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing " << kind_str
                                                       << " to JSON: get_all_members failed.");
        return ret;
    }

    // Serialize each member
    for (const auto& it : members)
    {
        if (RETCODE_OK != (ret = json_serialize_member(data, it.second, output, format)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing " << kind_str << " member '" << it.second->get_name()
                                                           << "' to JSON.");
            return ret;
        }
    }
    return ret;
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const traits<DynamicTypeMember>::ref_type& type_member,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(type_member)};

    if (member_impl)
    {
        MemberDescriptorImpl& member_desc {member_impl->get_descriptor()};

        TypeKind parent_kind = member_desc.parent_kind();
        MemberId member_id;

        if (TK_BITMASK == parent_kind)
        {
            member_id = member_desc.position();
        }
        else
        {
            member_id = member_desc.id();
        }

        // Absent optional members are serialized as JSON null.
        if (member_desc.is_optional() && !data->is_member_present(member_id))
        {
            json_insert(type_member->get_name().to_string(), nullptr, output);
            return RETCODE_OK;
        }

        return json_serialize_member(data, member_id,
                       traits<DynamicType>::narrow<DynamicTypeImpl>(
                           member_desc.type())->resolve_alias_enclosed_type()->get_kind(),
                       type_member->get_name().to_string(), output, format);
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
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
            return json_serialize_basic_member(data, member_id, member_kind, member_name, output, format);
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            return json_serialize_member_with_loan(data, member_id,
                           (member_kind == TK_STRUCTURE) ? "structure" : "bitset", json_serialize_aggregate_member,
                           member_name, output, format);
        }
        case TK_UNION:
        {
            return json_serialize_member_with_loan(data, member_id, "union", json_serialize_union_member, member_name,
                           output, format);
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            return json_serialize_member_with_loan(data, member_id, (member_kind == TK_SEQUENCE) ? "sequence" : "array",
                           json_serialize_collection_member, member_name, output, format);
        }
        case TK_MAP:
        {
            return json_serialize_member_with_loan(data, member_id, "map", json_serialize_map_member, member_name,
                           output, format);
        }
        case TK_BITMASK:
        {
            return json_serialize_member_with_loan(data, member_id, "bitmask", json_serialize_bitmask_member,
                           member_name, output, format);
        }
        case TK_ALIAS:
        {
            // This should not happen, as this method should always be called with the enclosed type
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing member to JSON: unexpected TK_ALIAS kind.");
            return RETCODE_BAD_PARAMETER;
        }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing member to JSON: unexpected kind " << member_kind
                                                                                           << " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    return json_serialize_member(data, member_id, member_kind, "", output, format);
}

ReturnCode_t json_serialize_basic_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    switch (member_kind)
    {
        case TK_NONE:
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing basic member to JSON: unexpected TK_NONE kind.");
            return RETCODE_BAD_PARAMETER;
        }
        case TK_BOOLEAN:
        {
            bool value;
            ReturnCode_t ret = data->get_boolean_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                if (TK_BITSET == data->enclosing_type()->get_kind())
                {
                    json_insert(member_name, static_cast<int>(value), output);
                }
                else
                {
                    json_insert(member_name, value, output);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_BOOLEAN member to JSON.");
            }
            return ret;
        }
        case TK_BYTE:
        {
            fastdds::rtps::octet value;
            ReturnCode_t ret = data->get_byte_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_BYTE member to JSON.");
            }
            return ret;
        }
        case TK_INT8:
        {
            int8_t value;
            ReturnCode_t ret = data->get_int8_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_INT8 member to JSON.");
            }
            return ret;
        }
        case TK_INT16:
        {
            int16_t value;
            ReturnCode_t ret = data->get_int16_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_INT16 member to JSON.");
            }
            return ret;
        }
        case TK_INT32:
        {
            int32_t value;
            ReturnCode_t ret = data->get_int32_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_INT32 member to JSON.");
            }
            return ret;
        }
        case TK_INT64:
        {
            int64_t value;
            ReturnCode_t ret = data->get_int64_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_INT64 member to JSON.");
            }
            return ret;
        }
        case TK_UINT8:
        {
            uint8_t value;
            ReturnCode_t ret = data->get_uint8_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_UINT8 member to JSON.");
            }
            return ret;
        }
        case TK_UINT16:
        {
            uint16_t value;
            ReturnCode_t ret = data->get_uint16_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_UINT16 member to JSON.");
            }
            return ret;
        }
        case TK_UINT32:
        {
            uint32_t value;
            ReturnCode_t ret = data->get_uint32_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_UINT32 member to JSON.");
            }
            return ret;
        }
        case TK_UINT64:
        {
            uint64_t value;
            ReturnCode_t ret = data->get_uint64_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_UINT64 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT32:
        {
            float value;
            ReturnCode_t ret = data->get_float32_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT32 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT64:
        {
            double value;
            ReturnCode_t ret = data->get_float64_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT64 member to JSON.");
            }
            return ret;
        }
        case TK_FLOAT128:
        {
            long double value;
            ReturnCode_t ret = data->get_float128_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                // Fail if value exceeds double limits, as JSON does not support long double
                if (value < std::numeric_limits<double>::lowest() ||
                        value > std::numeric_limits<double>::max())
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing TK_FLOAT128 member to JSON: value out of range.");
                    return RETCODE_BAD_PARAMETER;
                }
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_FLOAT128 member to JSON.");
            }
            return ret;
        }
        case TK_CHAR8:
        {
            char value;
            ReturnCode_t ret = data->get_char8_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                std::string aux_string_value({value});
                json_insert(member_name, aux_string_value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_CHAR8 member to JSON.");
            }
            return ret;
        }
        case TK_CHAR16:
        {
            wchar_t value;
            ReturnCode_t ret = data->get_char16_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                // Insert UTF-8 converted value
                std::wstring aux_wstring_value({value});
                std::string utf8_value("\0", 1);
#if defined(MINGW_COMPILER)
                // WARNING: it is the user responsibility to set the appropriate UTF-8 locale before calling this method
                size_t size_needed = std::wcstombs(nullptr, aux_wstring_value.c_str(), 0);
                if (size_needed == static_cast<size_t>(-1))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing TK_CHAR16 member to JSON: encountered invalid character.");
                    return RETCODE_BAD_PARAMETER;
                }
                else if (size_needed > 0)
                {
                    utf8_value.resize(size_needed);
                    if (std::wcstombs(&utf8_value[0], aux_wstring_value.c_str(),
                            size_needed) == static_cast<std::size_t>(-1))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing TK_CHAR16 member to JSON: encountered invalid character.");
                        return RETCODE_BAD_PARAMETER;
                    }
                }
#else
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                try
                {
                    utf8_value = converter.to_bytes(aux_wstring_value);
                }
                catch (const std::exception& e)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing TK_CHAR16 member to JSON: " << e.what());
                    return RETCODE_BAD_PARAMETER;
                }
#endif  // defined(MINGW_COMPILER)
                json_insert(member_name, utf8_value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_CHAR16 member to JSON.");
            }
            return ret;
        }
        case TK_STRING8:
        {
            std::string value;
            ReturnCode_t ret = data->get_string_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_STRING8 member to JSON.");
            }
            return ret;
        }
        case TK_STRING16:
        {
            std::wstring value;
            ReturnCode_t ret = data->get_wstring_value(value, member_id);
            if (RETCODE_OK == ret)
            {
                // Insert UTF-8 converted value
                std::string utf8_value;
#ifdef MINGW_COMPILER
                // WARNING: it is the user responsibility to set the appropriate UTF-8 locale before calling this method
                size_t size_needed = std::wcstombs(nullptr, value.c_str(), 0);
                if (size_needed == static_cast<size_t>(-1))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing TK_STRING16 member to JSON: encountered invalid character.");
                    return RETCODE_BAD_PARAMETER;
                }
                else if (size_needed > 0)
                {
                    utf8_value.resize(size_needed);
                    if (std::wcstombs(&utf8_value[0], value.c_str(), size_needed) == static_cast<std::size_t>(-1))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing TK_STRING16 member to JSON: encountered invalid character.");
                        return RETCODE_BAD_PARAMETER;
                    }
                }
#else
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                try
                {
                    utf8_value = converter.to_bytes(value);
                }
                catch (const std::exception& e)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing TK_STRING16 member to JSON: " << e.what());
                    return RETCODE_BAD_PARAMETER;
                }
#endif  // defined(MINGW_COMPILER)
                json_insert(member_name, utf8_value, output);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_STRING16 member to JSON.");
            }
            return ret;
        }
        case TK_ENUM:
        {
            return json_serialize_enum_member(data, member_id, member_name, output, format);
        }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing basic member to JSON: unexpected kind " << member_kind
                                                                                                 << " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_enum_member(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    // Get enumeration type to obtain the names of the different values, and also the underlying primitive type
    // NOTE: a different approach is required for collections and other "holder" types (e.g. structures),
    // as unlike with DynamicData::loan_value or DynamicData::get_X_value, DynamicData::get_descriptor method
    // is not meant to work with sequences nor arrays according to XTypes standard.
    traits<DynamicTypeImpl>::ref_type enum_type;
    TypeKind holder_kind = data->enclosing_type()->get_kind();
    ReturnCode_t ret = RETCODE_OK;
    if (TK_ARRAY == holder_kind || TK_SEQUENCE == holder_kind)
    {
        const TypeDescriptorImpl& collection_descriptor = data->enclosing_type()->get_descriptor();
        enum_type =
                traits<DynamicType>::narrow<DynamicTypeImpl>(collection_descriptor.element_type())->
                        resolve_alias_enclosed_type();
    }
    else
    {
        MemberDescriptor::_ref_type enum_desc{traits<MemberDescriptor>::make_shared()};
        if (RETCODE_OK != (ret = data->get_descriptor(enum_desc, member_id)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing TK_ENUM member to JSON: get_descriptor failed.");
            return ret;
        }
        enum_type = traits<DynamicType>::narrow<DynamicTypeImpl>(enum_desc->type())->resolve_alias_enclosed_type();
    }

    // Get value depending on the enclosing type
    assert(enum_type->get_kind() == TK_ENUM);
    auto enclosing_type_impl = traits<DynamicType>::narrow<DynamicTypeImpl>(enum_type->get_all_members_by_index().at(
                        0)->get_descriptor().type()); // Unfortunately DynamicDataImpl::get_enclosing_typekind is private
    if (nullptr == enclosing_type_impl)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing TK_ENUM member to JSON: null enclosing type.");
        return RETCODE_BAD_PARAMETER;
    }
    TypeKind enclosing_kind = enclosing_type_impl->get_kind();

    nlohmann::json j_value;
    if (TK_INT8 == enclosing_kind)
    {
        int8_t value;
        ret = data->get_int8_value(value, member_id);
        j_value = value;
    }
    else if (TK_UINT8 == enclosing_kind)
    {
        uint8_t value;
        ret = data->get_uint8_value(value, member_id);
        j_value = value;
    }
    else if (TK_INT16 == enclosing_kind)
    {
        int16_t value;
        ret = data->get_int16_value(value, member_id);
        j_value = value;
    }
    else if (TK_UINT16 == enclosing_kind)
    {
        uint16_t value;
        ret = data->get_uint16_value(value, member_id);
        j_value = value;
    }
    else if (TK_INT32 == enclosing_kind)
    {
        int32_t value;
        ret = data->get_int32_value(value, member_id);
        j_value = value;
    }
    else if (TK_UINT32 == enclosing_kind)
    {
        uint32_t value;
        ret = data->get_uint32_value(value, member_id);
        j_value = value;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing TK_ENUM member to JSON: unexpected enclosing kind "
                << enclosing_kind << " found.");
        return RETCODE_BAD_PARAMETER;
    }

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_ENUM member to JSON.");
        return ret;
    }

    DynamicTypeMembersByName all_members;
    if (RETCODE_OK !=
            (ret =
            enum_type->get_all_members_by_name(all_members)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing TK_ENUM member to JSON: get_all_members_by_name failed.");
        return ret;
    }

    ObjectName name;
    ret = RETCODE_BAD_PARAMETER;
    for (const auto& it : all_members)
    {
        auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                              it.second)};

        if (member_impl)
        {
            MemberDescriptorImpl& enum_member_desc {member_impl->get_descriptor()};
            if (enum_member_desc.literal_value() == j_value.dump())
            {
                name = it.first;
                assert(name == it.second->get_name());
                ret = RETCODE_OK;
                break;
            }
        }
    }
    if (RETCODE_OK == ret)
    {
        if (DynamicDataJsonMapping::OMG == format.mapping)
        {
            json_insert(member_name, name, output);
        }
        else if (DynamicDataJsonMapping::EPROSIMA == format.mapping)
        {
            nlohmann::json enum_dict = {{"name", name}, {"value", j_value}};
            json_insert(member_name, enum_dict, output);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing TK_ENUM member to JSON: unsupported mapping.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing TK_ENUM member to JSON: enum value not found.");
    }
    return ret;
}

ReturnCode_t json_serialize_member_with_loan(
        const traits<DynamicDataImpl>::ref_type& data,
        MemberId member_id,
        const std::string& kind_str,
        MemberSerializer member_serializer,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    traits<DynamicDataImpl>::ref_type st_data =
            traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));

    if (nullptr == st_data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing " << kind_str
                                                       << " member to JSON: loan_value failed.");
        return RETCODE_BAD_PARAMETER;
    }

    // WARNING: make sure the serializer is noexcept as the compiler might not perform that check
    ReturnCode_t ret = member_serializer(st_data, member_name, output, format);

    // Return loaned value
    // NOTE: this should always be done, even if something went wrong before
    ReturnCode_t ret_return_loan;
    if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while returning " << kind_str << " loaned value.");
    }
    // Give priority to prior error if occurred
    return RETCODE_OK != ret ? ret : ret_return_loan;
}

ReturnCode_t json_serialize_aggregate_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    nlohmann::json j_struct;
    std::string kind_str = (data->enclosing_type()->get_kind() == TK_STRUCTURE) ? "structure" : "bitset";
    ReturnCode_t ret;
    if (RETCODE_OK != (ret = json_serialize_aggregate(data, j_struct, format)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing " << kind_str
                                                       << " member to JSON: json_serialize_aggregate failed.");
        return ret;
    }

    // Insert aggregate member into JSON object
    json_insert(member_name, j_struct, output);

    return ret;
}

ReturnCode_t json_serialize_union_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    nlohmann::json j_union;
    ReturnCode_t ret = RETCODE_OK;
    MemberId selected_member = data->selected_union_member();

    if (MEMBER_ID_INVALID == selected_member)
    {
        // No member selected, insert empty JSON object
        json_insert(member_name, j_union, output);
    }
    else
    {
        DynamicTypeMember::_ref_type active_type_member;
        if (RETCODE_OK != (ret = data->enclosing_type()->get_member(active_type_member, selected_member)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing union member to JSON: get_member failed.");
        }
        else if (RETCODE_OK != (ret = json_serialize_member(data, active_type_member, j_union, format)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing union member '" << active_type_member->get_name()
                                                                         << "' to JSON.");
        }
        else
        {
            // Insert union member into JSON object
            json_insert(member_name, j_union, output);
        }
    }
    return ret;
}

ReturnCode_t json_serialize_collection_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    const TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();
    const auto dyn_type =
            traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())->resolve_alias_enclosed_type();
    auto element_kind = dyn_type->get_kind();
    const bool is_sequence = (data->enclosing_type()->get_kind() == TK_SEQUENCE);

    // If a max_collection_items limit is configured and the collection exceeds it,
    // emit a compact label instead of the full element list.
    if (format.max_collection_items > 0)
    {
        const uint32_t count = data->get_item_count();
        if (count > format.max_collection_items)
        {
            std::string label;
            if (RETCODE_OK != (ret = collection_label(data->enclosing_type(), count, label)))
            {
                return ret;
            }
            json_insert(member_name, label, output);
            return ret;
        }
    }

    if (is_sequence)
    {
        assert(descriptor.bound().size() == 1);
        auto count = data->get_item_count();
        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t index = 0; index < count; ++index)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index), element_kind, j_array, format)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing sequence collection to JSON.");
                return ret;
            }
        }
        json_insert(member_name, j_array, output);
    }
    else
    {
        const BoundSeq& bounds = descriptor.bound();
        nlohmann::json j_array = nlohmann::json::array();
        unsigned int index = 0;
        if (RETCODE_OK != (ret = json_serialize_array(data, element_kind, index, bounds, j_array,
                format)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing array collection to JSON.");
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
        TypeKind element_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        nlohmann::json& j_array,
        const FormatOptions& format) noexcept
{
    assert(j_array.is_array());
    ReturnCode_t ret = RETCODE_OK;
    if (bounds.size() == 1)
    {
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index++), element_kind, j_array,
                    format)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing array element to JSON.");
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
                    json_serialize_array(data, element_kind, index,
                    std::vector<unsigned int>(bounds.begin() + 1, bounds.end()), inner_array, format)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing array's array element to JSON.");
                break;
            }
            j_array.push_back(inner_array);
        }
    }
    return ret;
}

ReturnCode_t json_serialize_map_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    nlohmann::json j_map = nlohmann::json::object();
    const TypeDescriptorImpl& map_desc = data->enclosing_type()->get_descriptor();
    traits<DynamicTypeImpl>::ref_type key_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
        map_desc.key_element_type())->resolve_alias_enclosed_type();
    traits<DynamicTypeImpl>::ref_type value_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
        map_desc.element_type())->resolve_alias_enclosed_type();

    std::map<std::string, MemberId> key_to_id;
    if (RETCODE_OK != (ret = data->get_keys(key_to_id)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing map member to JSON: get_keys failed.");
        return ret;
    }

    std::map<MemberId, std::string> id_to_key;
    for (const auto& it : key_to_id)
    {
        id_to_key[it.second] = it.first;
    }
    assert(id_to_key.size() == key_to_id.size());

    uint32_t size = data->get_item_count();
    assert(size == key_to_id.size());
    for (uint32_t i = 0; i < size; ++i)
    {
        MemberId id = data->get_member_id_at_index(i);
        if (MEMBER_ID_INVALID == id)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing map member's member to JSON: invalid member id.");
            return RETCODE_BAD_PARAMETER;
        }

        auto it = id_to_key.find(id);
        if (it == id_to_key.end())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing map member's member to JSON: key not found.");
            return RETCODE_BAD_PARAMETER;
        }

        if (RETCODE_OK !=
                (ret =
                json_serialize_member(data, id, value_type->get_kind(), it->second, j_map,
                format)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing map member's member to JSON.");
            return ret;
        }
    }

    // All map entries were serialized successfully -> Insert into JSON object
    assert(RETCODE_OK == ret);
    json_insert(member_name, j_map, output);

    return ret;
}

ReturnCode_t json_serialize_bitmask_member(
        const traits<DynamicDataImpl>::ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        const FormatOptions& format) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    traits<DynamicTypeImpl>::ref_type bitmask_type = data->enclosing_type();
    const TypeDescriptorImpl& bitmask_desc = bitmask_type->get_descriptor();

    // Get the bitmask bound to determine the value precision
    auto bound = bitmask_desc.bound().at(0);

    if (DynamicDataJsonMapping::OMG == format.mapping)
    {
        if (9 > bound)
        {
            uint8_t value;
            if (RETCODE_OK == (ret = data->get_uint8_value(value, MEMBER_ID_INVALID)))
            {
                json_insert(member_name, value, output);
            }
        }
        else if (17 > bound)
        {
            uint16_t value;
            if (RETCODE_OK == (ret = data->get_uint16_value(value, MEMBER_ID_INVALID)))
            {
                json_insert(member_name, value, output);
            }
        }
        else if (33 > bound)
        {
            uint32_t value;
            if (RETCODE_OK == (ret = data->get_uint32_value(value, MEMBER_ID_INVALID)))
            {
                json_insert(member_name, value, output);
            }
        }
        else
        {
            uint64_t value;
            if (RETCODE_OK == (ret = data->get_uint64_value(value, MEMBER_ID_INVALID)))
            {
                json_insert(member_name, value, output);
            }
        }

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing bitmask member to JSON: failed to get value.");
        }
    }
    else if (DynamicDataJsonMapping::EPROSIMA == format.mapping)
    {
        nlohmann::json bitmask_dict;
        uint64_t u64_value{0}; // Auxiliar variable to check active bits afterwards
        if (9 > bound)
        {
            uint8_t value;
            if (RETCODE_OK == (ret = data->get_uint8_value(value, MEMBER_ID_INVALID)))
            {
                bitmask_dict["value"] = value;
                bitmask_dict["binary"] = std::bitset<8>(value).to_string();
                u64_value = static_cast<uint64_t>(value);
            }
        }
        else if (17 > bound)
        {
            uint16_t value;
            if (RETCODE_OK == (ret = data->get_uint16_value(value, MEMBER_ID_INVALID)))
            {
                bitmask_dict["value"] = value;
                bitmask_dict["binary"] = std::bitset<16>(value).to_string();
                u64_value = static_cast<uint64_t>(value);
            }
        }
        else if (33 > bound)
        {
            uint32_t value;
            if (RETCODE_OK == (ret = data->get_uint32_value(value, MEMBER_ID_INVALID)))
            {
                bitmask_dict["value"] = value;
                bitmask_dict["binary"] = std::bitset<32>(value).to_string();
                u64_value = static_cast<uint64_t>(value);
            }
        }
        else
        {
            uint64_t value;
            if (RETCODE_OK == (ret = data->get_uint64_value(value, MEMBER_ID_INVALID)))
            {
                bitmask_dict["value"] = value;
                bitmask_dict["binary"] = std::bitset<64>(value).to_string();
                u64_value = value;
            }
        }

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing bitmask member to JSON: failed to get value.");
            return ret;
        }

        // Check active bits
        DynamicTypeMembersById bitmask_members;
        if (RETCODE_OK != (ret = bitmask_type->get_all_members(bitmask_members)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing bitmask member to JSON: get_all_members failed.");
            return ret;
        }
        std::vector<std::string> active_bits;
        for (const auto& it : bitmask_members)
        {
            auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(it.second)};
            if (member_impl)
            {
                MemberDescriptorImpl& member_desc {member_impl->get_descriptor()};

                if (u64_value & (0x01ull << member_desc.position()))
                {
                    active_bits.push_back(it.second->get_name().to_string());
                }
            }
        }
        bitmask_dict["active"] = active_bits;

        // Insert custom bitmask value
        json_insert(member_name, bitmask_dict, output);
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while serializing bitmask member to JSON: unsupported mapping.");
        return RETCODE_BAD_PARAMETER;
    }
    return ret;
}

template<typename T>
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

ReturnCode_t type_to_string(
        const DynamicType::_ref_type& dyn_type,
        std::string& out,
        int depth) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    const auto kind = dyn_type->get_kind();

    switch (kind)
    {
        case TK_BOOLEAN:
        {
            out = "boolean";
            break;
        }
        case TK_BYTE:
        {
            out = "octet";
            break;
        }
        case TK_INT8:
        {
            out = "int8";
            break;
        }
        case TK_INT16:
        {
            out = "short";
            break;
        }
        case TK_INT32:
        {
            out = "long";
            break;
        }
        case TK_INT64:
        {
            out = "long long";
            break;
        }
        case TK_UINT8:
        {
            out = "uint8";
            break;
        }
        case TK_UINT16:
        {
            out = "unsigned short";
            break;
        }
        case TK_UINT32:
        {
            out = "unsigned long";
            break;
        }
        case TK_UINT64:
        {
            out = "unsigned long long";
            break;
        }
        case TK_FLOAT32:
        {
            out = "float";
            break;
        }
        case TK_FLOAT64:
        {
            out = "double";
            break;
        }
        case TK_FLOAT128:
        {
            out = "long double";
            break;
        }
        case TK_CHAR8:
        {
            out = "char";
            break;
        }
        case TK_CHAR16:
        {
            out = "wchar";
            break;
        }
        case TK_STRING8:
        {
            out = "string";
            break;
        }
        case TK_STRING16:
        {
            out = "wstring";
            break;
        }
        case TK_ARRAY:
        case TK_SEQUENCE:
        {
            // Nested collections (depth exhausted) are reported by their kind word only.
            if (0 > depth)
            {
                out = (TK_SEQUENCE == kind) ? "sequence" : "array";
                break;
            }

            // The element type and (multidimensional) bounds are reachable from
            // the type descriptor, so no extra arguments are needed.
            const TypeDescriptorImpl& descriptor =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(dyn_type)->get_descriptor();
            const auto element_type =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())
                            ->resolve_alias_enclosed_type();
            std::string element_str;
            if (RETCODE_OK != (ret = type_to_string(element_type, element_str, depth - 1)))
            {
                break;
            }

            if (TK_SEQUENCE == kind)
            {
                // sequence<element>, e.g. "sequence<long>".
                out = "sequence<" + element_str + ">";
            }
            else
            {
                // element[d1,d2,...], e.g. "long[3,4]".
                std::string bounds_str;
                for (const auto& bound : descriptor.bound())
                {
                    bounds_str += (bounds_str.empty() ? "" : ",") + std::to_string(bound);
                }
                out = element_str + "[" + bounds_str + "]";
            }
            break;
        }
        case TK_MAP:
        {
            if (0 > depth)
            {
                out = "map";
                break;
            }

            const TypeDescriptorImpl& descriptor =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(dyn_type)->get_descriptor();
            const auto key_type =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.key_element_type())
                            ->resolve_alias_enclosed_type();
            const auto value_type =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())
                            ->resolve_alias_enclosed_type();
            std::string key_str;
            std::string value_str;
            if (RETCODE_OK != (ret = type_to_string(key_type, key_str, depth - 1)) ||
                    RETCODE_OK != (ret = type_to_string(value_type, value_str, depth - 1)))
            {
                break;
            }
            // map<key, value>, e.g. "map<long, string>".
            out = "map<" + key_str + ", " + value_str + ">";
            break;
        }
        case TK_ALIAS:
        {
            // Resolve through the alias chain so the label reflects the underlying type.
            // Alias resolution does not count against the recursion budget.
            auto resolved =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(dyn_type)->resolve_alias_enclosed_type();
            ret = type_to_string(resolved, out, depth);
            break;
        }
        case TK_BITMASK:
        case TK_BITSET:
        case TK_ENUM:
        case TK_STRUCTURE:
        case TK_UNION:
        {
            // Named constructs are described by their own type name.
            out = dyn_type->get_name().to_string();
            break;
        }
        case TK_NONE:
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Failed to convert TK_NONE to string.");
            ret = RETCODE_UNSUPPORTED;
            break;
        }
        default:
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Failed to convert unknown type (" << kind << ") to string.");
            ret = RETCODE_BAD_PARAMETER;
            break;
        }
    }
    return ret;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
