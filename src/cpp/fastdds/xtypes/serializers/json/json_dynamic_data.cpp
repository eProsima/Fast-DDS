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
#include <stdexcept>
#include <string>
#include <type_traits>

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
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    if (nullptr == dynamic_type)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null DynamicType value while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    if (TK_STRUCTURE != dynamic_type->get_kind())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Only structs are supported by json_deserialize method.");
        return RETCODE_BAD_PARAMETER;
    }

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

    // Downcast to access implementation methods
    auto data_impl = traits<DynamicData>::narrow<DynamicDataImpl>(data);

    if (j.is_null())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null JSON value while performing JSON to DynamicData deserialization.");
        return RETCODE_BAD_PARAMETER;
    }

    return json_deserialize_aggregate(j, format, data_impl);
}

ReturnCode_t json_deserialize_aggregate(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    std::string kind_str = (data->enclosing_type()->get_kind() == TK_STRUCTURE) ? "structure" : "bitset";
    if (!j.is_object())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing " << kind_str << ": expected JSON object.");
        return RETCODE_BAD_PARAMETER;
    }

    if (j.size() != data->enclosing_type()->get_member_count())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing " << kind_str << ": size is " << j.size() << ", "
                                                         << "but the expected number of members is " << data->enclosing_type()->get_member_count() <<
                ".");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        DynamicTypeMember::_ref_type type_member;
        if (RETCODE_OK != (ret = data->enclosing_type()->get_member_by_name(type_member, it.key())))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing " << kind_str << " member '" << it.key() <<
                    "' from JSON: get_member_by_name failed.");
            break;
        }

        if (RETCODE_OK != (ret = json_deserialize_member(it.value(), type_member, format, data)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing " << kind_str << " member '" << it.key() << "'.");
            break;
        }
    }
    return ret;
}

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const traits<DynamicTypeMember>::ref_type& type_member,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    MemberDescriptorImpl& member_desc =
            traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(type_member)->get_descriptor();

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

    return json_deserialize_member(j, member_id,
                   traits<DynamicType>::narrow<DynamicTypeImpl>(
                       member_desc.type())->resolve_alias_enclosed_type()->get_kind(), format, data);
}

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
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
            return json_deserialize_member_with_loan(j, member_id,
                           (member_kind == TK_STRUCTURE) ? "structure" : "bitset",
                           json_deserialize_aggregate, format, data);
        }
        case TK_UNION:
        {
            return json_deserialize_member_with_loan(j, member_id, "union", json_deserialize_union, format, data);
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            return json_deserialize_member_with_loan(j, member_id, (member_kind == TK_SEQUENCE) ? "sequence" : "array",
                           json_deserialize_collection, format, data);
        }
        case TK_MAP:
        {
            return json_deserialize_member_with_loan(j, member_id, "map", json_deserialize_map, format, data);
        }
        case TK_BITMASK:
        {
            return json_deserialize_member_with_loan(j, member_id, "bitmask", json_deserialize_bitmask, format, data);
        }
        case TK_ALIAS:
        {
            // This should not happen, as this method should always be called with the enclosed type
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing member from JSON: unexpected TK_ALIAS kind.");
            return RETCODE_BAD_PARAMETER;
        }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing member: unexpected kind " << member_kind <<
                    " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_deserialize_basic_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
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
            try
            {
                ReturnCode_t ret;
                if (TK_BITSET == data->enclosing_type()->get_kind())
                {
                    const auto value = numeric_get<uint8_t>(j);
                    if (value != 0 && value != 1)
                    {
                        throw std::invalid_argument(std::string{"Expected 0 or 1, got "} + std::to_string(value));
                    }
                    ret = data->set_boolean_value(member_id, value);
                }
                else
                {
                    if (!j.is_boolean())
                    {
                        throw std::invalid_argument(std::string{"Expected boolean value"});
                    }
                    ret = data->set_boolean_value(member_id, j);
                }

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_BOOLEAN member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_BOOLEAN member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_BYTE:
        {
            try
            {
                ReturnCode_t ret = data->set_byte_value(member_id, numeric_get<fastdds::rtps::octet>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_BYTE member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_BYTE member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_INT8:
        {
            try
            {
                ReturnCode_t ret = data->set_int8_value(member_id, numeric_get<int8_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT8 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_INT8 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_INT16:
        {
            try
            {
                ReturnCode_t ret = data->set_int16_value(member_id, numeric_get<int16_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT16 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_INT16 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_INT32:
        {
            try
            {
                ReturnCode_t ret = data->set_int32_value(member_id, numeric_get<int32_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT32 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_INT32 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_INT64:
        {
            try
            {
                ReturnCode_t ret = data->set_int64_value(member_id, numeric_get<int64_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_INT64 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_INT64 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_UINT8:
        {
            try
            {
                ReturnCode_t ret = data->set_uint8_value(member_id, numeric_get<uint8_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT8 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_UINT8 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_UINT16:
        {
            try
            {
                ReturnCode_t ret = data->set_uint16_value(member_id, numeric_get<uint16_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT16 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_UINT16 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_UINT32:
        {
            try
            {
                ReturnCode_t ret = data->set_uint32_value(member_id, numeric_get<uint32_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT32 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_UINT32 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_UINT64:
        {
            try
            {
                ReturnCode_t ret = data->set_uint64_value(member_id, numeric_get<uint64_t>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_UINT64 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_UINT64 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_FLOAT32:
        {
            try
            {
                ReturnCode_t ret = data->set_float32_value(member_id, numeric_get<float>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT32 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_FLOAT32 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_FLOAT64:
        {
            try
            {
                ReturnCode_t ret = data->set_float64_value(member_id, numeric_get<double>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT64 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_FLOAT64 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_FLOAT128:
        {
            try
            {
                ReturnCode_t ret = data->set_float128_value(member_id, numeric_get<long double>(j));
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_FLOAT128 member.");
                }
                return ret;
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for TK_FLOAT128 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
        }
        case TK_CHAR8:
        {
            if (!j.is_string() || j.get<std::string>().size() > 1)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_CHAR8 member: expected 1-character string.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = data->set_char8_value(member_id, j.get<std::string>()[0]);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_CHAR8 member.");
            }
            return ret;
        }
        case TK_CHAR16:
        {
            if (!j.is_string())
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_CHAR16 member: expected 1-character string.");
                return RETCODE_BAD_PARAMETER;
            }

            // Convert UTF-8 value to wstring
            std::string j_string = j.get<std::string>();
            std::wstring aux_wstring({L'\0'});
#if defined(MINGW_COMPILER)
            // WARNING: it is the user responsibility to set the appropriate UTF-8 locale before calling this method
            size_t size_needed = std::mbstowcs(nullptr, j_string.c_str(), 0);
            if (size_needed == static_cast<size_t>(-1))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_CHAR16 member: invalid UTF-8 string.");
                return RETCODE_BAD_PARAMETER;
            }
            else if (size_needed > 0)
            {
                aux_wstring.resize(size_needed);
                if (std::mbstowcs(&aux_wstring[0], j_string.c_str(), size_needed) == static_cast<size_t>(-1))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing TK_CHAR16 member: invalid UTF-8 string.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            try
            {
                aux_wstring = converter.from_bytes(j_string);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_CHAR16 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
#endif  // defined(MINGW_COMPILER)

            if (aux_wstring.size() > 1)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_CHAR16 member: expected 1-character string.");
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
            if (!j.is_string())
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_STRING8 member: expected string.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = data->set_string_value(member_id, j);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_STRING8 member.");
            }
            return ret;
        }
        case TK_STRING16:
        {
            if (!j.is_string())
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_STRING16 member: expected string.");
                return RETCODE_BAD_PARAMETER;
            }

            // Convert UTF-8 value to wstring
            std::string j_string = j.get<std::string>();
            std::wstring value;
#if defined(MINGW_COMPILER)
            // WARNING: it is the user responsibility to set the appropriate UTF-8 locale before calling this method
            size_t size_needed = std::mbstowcs(nullptr, j_string.c_str(), 0);
            if (size_needed == static_cast<size_t>(-1))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_STRING16 member: invalid UTF-8 string.");
                return RETCODE_BAD_PARAMETER;
            }
            else if (size_needed > 0)
            {
                value.resize(size_needed);
                if (std::mbstowcs(&value[0], j_string.c_str(), size_needed) == static_cast<size_t>(-1))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing TK_STRING16 member: invalid UTF-8 string.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            try
            {
                value = converter.from_bytes(j_string);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_STRING16 member: " << e.what());
                return RETCODE_BAD_PARAMETER;
            }
#endif  // defined(MINGW_COMPILER)

            ReturnCode_t ret = data->set_wstring_value(member_id, value);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_STRING16 member.");
            }
            return ret;
        }
        case TK_ENUM:
        {
            return json_deserialize_enum_member(j, member_id, format, data);
        }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing basic member to JSON: unexpected kind " << member_kind <<
                    " found.");
            return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_deserialize_enum_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    ObjectName enum_name;
    uint32_t u32_value{0};      // Value container (of max precision) to be casted to the appropriate type
    bool is_value_set = false;  // Flag to indicate if 'value' was set in EPROSIMA format to later verify
                                // it coincides with the one obtained from 'name' (if provided)

    // Get enumeration type to obtain the names of the different values, and also the underlying primitive type
    // NOTE: a different approach is required for collections and other "holder" types (e.g. structures),
    // as unlike with DynamicData::loan_value or DynamicData::get_X_value, DynamicData::get_descriptor method
    // is not meant to work with sequences nor arrays according to XTypes standard.
    traits<DynamicTypeImpl>::ref_type enum_type;
    TypeKind holder_kind = data->enclosing_type()->get_kind();
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
                    "Error encountered while deserializing TK_ENUM member from JSON: get_descriptor failed.");
            return ret;
        }
        enum_type = traits<DynamicType>::narrow<DynamicTypeImpl>(enum_desc->type())->resolve_alias_enclosed_type();
    }

    // Get enclosing type kind to parse the value accordingly, and later user the appropriate setter
    assert(enum_type->get_kind() == TK_ENUM);
    auto enclosing_type_impl = traits<DynamicType>::narrow<DynamicTypeImpl>(enum_type->get_all_members_by_index().at(
                        0)->get_descriptor().type()); // Unfortunately DynamicDataImpl::get_enclosing_typekind is private
    if (nullptr == enclosing_type_impl)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing TK_ENUM member from JSON: null enclosing type.");
        return RETCODE_BAD_PARAMETER;
    }
    TypeKind enclosing_kind = enclosing_type_impl->get_kind();

    if (DynamicDataJsonFormat::OMG == format)
    {
        if (!j.is_string())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing TK_ENUM member: expected string.");
            return RETCODE_BAD_PARAMETER;
        }
        enum_name = j.get<std::string>();
    }
    else if (DynamicDataJsonFormat::EPROSIMA == format)
    {
        if (!j.is_object() || j.empty())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing TK_ENUM member: expected non-empty JSON object.");
            return RETCODE_BAD_PARAMETER;
        }
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            if ("name" == it.key())
            {
                if (!it.value().is_string())
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing TK_ENUM member: expected string for 'name'.");
                    return RETCODE_BAD_PARAMETER;
                }
                enum_name = it.value();
            }
            else if ("value" == it.key())
            {
                try
                {
                    if (TK_INT8 == enclosing_kind)
                    {
                        u32_value = static_cast<uint32_t>(numeric_get<int8_t>(it.value()));
                    }
                    else if (TK_UINT8 == enclosing_kind)
                    {
                        u32_value = static_cast<uint32_t>(numeric_get<uint8_t>(it.value()));
                    }
                    else if (TK_INT16 == enclosing_kind)
                    {
                        u32_value = static_cast<uint32_t>(numeric_get<int16_t>(it.value()));
                    }
                    else if (TK_UINT16 == enclosing_kind)
                    {
                        u32_value = static_cast<uint32_t>(numeric_get<uint16_t>(it.value()));
                    }
                    else if (TK_INT32 == enclosing_kind)
                    {
                        u32_value = static_cast<uint32_t>(numeric_get<int32_t>(it.value()));
                    }
                    else if (TK_UINT32 == enclosing_kind)
                    {
                        u32_value = numeric_get<uint32_t>(it.value());
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while deserializing TK_ENUM member: unexpected enclosing kind " <<
                                enclosing_kind << " found.");
                        return RETCODE_BAD_PARAMETER;
                    }
                    is_value_set = true;
                }
                catch (const std::exception& e)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing TK_ENUM member's 'value': " << e.what());
                    return RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing TK_ENUM member: unexpected key '" << it.key() <<
                        "' found.");
                return RETCODE_BAD_PARAMETER;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing TK_ENUM member from JSON: unsupported format.");
        return RETCODE_BAD_PARAMETER;
    }

    if (!enum_name.to_string().empty())
    {
        // Find value corresponding to name

        DynamicTypeMembersByName all_members;
        if (RETCODE_OK != (ret = enum_type->get_all_members_by_name(all_members)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing TK_ENUM member from JSON: get_all_members_by_name failed.");
            return ret;
        }

        ret = RETCODE_BAD_PARAMETER;
        for (const auto& it : all_members)
        {
            MemberDescriptorImpl& enum_member_desc = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                it.second)->get_descriptor();
            if (it.first == enum_name)
            {
                const auto value_from_name = std::stoi(enum_member_desc.literal_value());
                if (DynamicDataJsonFormat::EPROSIMA == format && is_value_set)
                {
                    // Check if value coincides with the one obtained from name
                    if (u32_value != static_cast<uint32_t>(value_from_name))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while deserializing TK_ENUM member from JSON: name-value mismatch.");
                        return RETCODE_BAD_PARAMETER;
                    }
                }
                else
                {
                    u32_value = static_cast<uint32_t>(value_from_name);
                }
                ret = RETCODE_OK;
                break;
            }
        }
        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing TK_ENUM member from JSON: enum value not found.");
            return ret;
        }
    }

    assert(RETCODE_OK == ret);
    if (TK_INT8 == enclosing_kind)
    {
        ret = data->set_int8_value(member_id, static_cast<int8_t>(u32_value));
    }
    else if (TK_UINT8 == enclosing_kind)
    {
        ret = data->set_uint8_value(member_id, static_cast<uint8_t>(u32_value));
    }
    else if (TK_INT16 == enclosing_kind)
    {
        ret = data->set_int16_value(member_id, static_cast<int16_t>(u32_value));
    }
    else if (TK_UINT16 == enclosing_kind)
    {
        ret = data->set_uint16_value(member_id, static_cast<uint16_t>(u32_value));
    }
    else if (TK_INT32 == enclosing_kind)
    {
        ret = data->set_int32_value(member_id, static_cast<int32_t>(u32_value));
    }
    else if (TK_UINT32 == enclosing_kind)
    {
        ret = data->set_uint32_value(member_id, u32_value);
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing TK_ENUM member from JSON: unexpected enclosing kind " <<
                enclosing_kind << " found.");
        return RETCODE_BAD_PARAMETER;
    }

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing TK_ENUM member from JSON: set value failed.");
    }
    return ret;
}

ReturnCode_t json_deserialize_member_with_loan(
        const nlohmann::json& j,
        const MemberId& member_id,
        const std::string& kind_str,
        MemberDeserializer member_deserializer,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    traits<DynamicDataImpl>::ref_type st_data =
            traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));

    if (nullptr == st_data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing " << kind_str
                                                         << " member to JSON: loan_value failed.");
        return RETCODE_BAD_PARAMETER;
    }

    // WARNING: make sure the deserializer is noexcept as the compiler might not perform that check
    ReturnCode_t ret = member_deserializer(j, format, st_data);

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

ReturnCode_t json_deserialize_union(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    if (j.empty())
    {
        // This corresponds to a union with no member selected, so nothing needs to be done
        // NOTE: it is assumed that this union type has no default active member, and since at the time of this
        // writing there is no way to modify a union to have no member selected, it is enough to leave the type
        // in its default state (no active members). If in the future a way of deactivating all members is added,
        // this method will need to be modified and use it.
        return RETCODE_OK;
    }

    if (!j.is_object() || j.size() > 1)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing union member from JSON: expected a single-key JSON object.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    auto key = j.begin().key();
    auto value = j.begin().value();
    DynamicTypeMember::_ref_type type_member;
    if (RETCODE_OK != (ret = data->enclosing_type()->get_member_by_name(type_member, key)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing union member '" << key <<
                "' from JSON: get_member_by_name failed.");
    }
    else if (RETCODE_OK != (ret = json_deserialize_member(value, type_member, format, data)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing union member '" << key << "'.");
    }
    return ret;
}

ReturnCode_t json_deserialize_collection(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    if (!j.is_array())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing collection member: expected JSON array.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    const TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();
    auto element_kind =
            traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())->resolve_alias_enclosed_type()
                    ->get_kind();
    if (TK_SEQUENCE == data->enclosing_type()->get_kind())
    {
        assert(descriptor.bound().size() == 1);
        if (j.size() > descriptor.bound()[0])
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing collection member: size " << j.size() <<
                    " does not match bound " << descriptor.bound()[0] << ".");
            return RETCODE_BAD_PARAMETER;
        }

        for (size_t i = 0; i < j.size(); ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_deserialize_member(j[i], static_cast<MemberId>(i), element_kind, format, data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing sequence member.");
                break;
            }
        }
    }
    else
    {
        const BoundSeq& bounds = descriptor.bound();
        unsigned int index = 0;
        if (RETCODE_OK != (ret = json_deserialize_array(j, element_kind, index, bounds, format, data)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while deserializing JSON array collection.");
        }
    }
    return ret;
}

ReturnCode_t json_deserialize_array(
        const nlohmann::json& j,
        TypeKind element_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    assert(j.is_array());
    ReturnCode_t ret = RETCODE_OK;
    if (bounds.size() == 1)
    {
        if (bounds[0] != j.size())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing JSON array: size does not match bound.");
            return RETCODE_BAD_PARAMETER;
        }
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_deserialize_member(j[i], static_cast<MemberId>(index++), element_kind, format, data)))
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
                    json_deserialize_array(j[i], element_kind, index,
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

ReturnCode_t json_deserialize_map(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    if (j.empty())
    {
        // This corresponds to a map with no entries, so nothing needs to be done
        return RETCODE_OK;
    }

    if (!j.is_object())
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing map member from JSON: expected JSON object.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;
    const TypeDescriptorImpl& map_desc = data->enclosing_type()->get_descriptor();
    traits<DynamicTypeImpl>::ref_type key_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
        map_desc.key_element_type())->resolve_alias_enclosed_type();
    traits<DynamicTypeImpl>::ref_type value_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
        map_desc.element_type())->resolve_alias_enclosed_type();

    assert(map_desc.bound().size() == 1);
    if (j.size() > map_desc.bound().at(0))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing map member from JSON: "
                "JSON object size exceeds map bound.");
        return RETCODE_BAD_PARAMETER;
    }

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        MemberId id = data->get_member_id_by_name(it.key());
        if (RETCODE_OK != (ret = json_deserialize_member(it.value(), id, value_type->get_kind(), format, data)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing map member '" << it.key() << "'.");
            break;
        }
    }
    return ret;
}

ReturnCode_t json_deserialize_bitmask(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    ReturnCode_t ret = RETCODE_OK;
    traits<DynamicTypeImpl>::ref_type bitmask_type = data->enclosing_type();
    const TypeDescriptorImpl& bitmask_desc = bitmask_type->get_descriptor();

    // Get the bitmask bound to determine the value precision
    auto bound = bitmask_desc.bound().at(0);

    // Determine the actual bound to use for the bitmask value
    uint32_t true_bound;
    if (9 > bound)
    {
        true_bound = 8;
    }
    else if (17 > bound)
    {
        true_bound = 16;
    }
    else if (33 > bound)
    {
        true_bound = 32;
    }
    else
    {
        true_bound = 64;
    }

    // Flags to indicate which fields are present in the JSON
    bool has_value{false};
    bool has_binary{false};
    bool has_active{false};

    // Variables to store parsed values, to be processed later
    nlohmann::json j_value;
    std::string j_binary;
    std::vector<std::string> j_active_bits;

    if (DynamicDataJsonFormat::OMG == format)
    {
        if (!j.is_number())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing bitmask member: value is not a number.");
            return RETCODE_BAD_PARAMETER;
        }
        j_value = j;
        has_value = true;
    }
    else if (DynamicDataJsonFormat::EPROSIMA == format)
    {
        if (!j.is_object() || j.empty())
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing bitmask member: expected non-empty JSON object.");
            return RETCODE_BAD_PARAMETER;
        }

        for (auto it = j.begin(); it != j.end(); ++it)
        {
            if ("value" == it.key())
            {
                if (!it.value().is_number())
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing bitmask member: value is not a number.");
                    return RETCODE_BAD_PARAMETER;
                }
                j_value = it.value();
                has_value = true;
            }
            else if ("binary" == it.key())
            {
                if (!it.value().is_string())
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing bitmask member: binary value is not a string.");
                    return RETCODE_BAD_PARAMETER;
                }
                j_binary = it.value();
                has_binary = true;
            }
            else if ("active" == it.key())
            {
                if (!it.value().is_array() || (!it.value().empty() && !it.value().at(0).is_string()))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while deserializing bitmask member: active bits is not an array of strings.");
                    return RETCODE_BAD_PARAMETER;
                }
                j_active_bits = it.value();
                has_active = true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while deserializing bitmask member: unexpected key '" << it.key() <<
                        "' found.");
                return RETCODE_BAD_PARAMETER;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing bitmask member from JSON: unsupported format.");
        return RETCODE_BAD_PARAMETER;
    }

    uint64_t u64_from_value{0};
    if (has_value)
    {
        try
        {
            if (9 > bound)
            {
                u64_from_value = static_cast<uint64_t>(numeric_get<uint8_t>(j_value));
            }
            else if (17 > bound)
            {
                u64_from_value = static_cast<uint64_t>(numeric_get<uint16_t>(j_value));
            }
            else if (33 > bound)
            {
                u64_from_value = static_cast<uint64_t>(numeric_get<uint32_t>(j_value));
            }
            else
            {
                u64_from_value = numeric_get<uint64_t>(j_value);
            }
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for bitmask member value: " << e.what());
            return RETCODE_BAD_PARAMETER;
        }
    }

    uint64_t u64_from_binary{0};
    if (has_binary)
    {
        if (j_binary.size() > true_bound)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing bitmask member from JSON: binary value '" <<
                    j_binary << "' does not match bound " << bound << ".");
            return RETCODE_BAD_PARAMETER;
        }
        try
        {
            u64_from_binary = std::bitset<64>(j_binary).to_ullong();
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Parsing error for bitmask member binary: " << e.what());
            return RETCODE_BAD_PARAMETER;
        }
    }

    uint64_t u64_from_active{0};
    if (has_active)
    {
        DynamicTypeMembersById bitmask_members;
        if (RETCODE_OK != (ret = bitmask_type->get_all_members(bitmask_members)))
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while deserializing bitmask member from JSON: get_all_members failed.");
            return ret;
        }
        else
        {
            for (const auto& it : bitmask_members)
            {
                if (std::find(j_active_bits.begin(), j_active_bits.end(),
                        it.second->get_name().to_string()) != j_active_bits.end())
                {
                    MemberDescriptorImpl& member_desc =
                            traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(it.second)->get_descriptor();
                    u64_from_active |= (0x01ull << member_desc.position());
                }
            }
        }
    }

    if ((has_value && has_binary && u64_from_value != u64_from_binary) ||
            (has_value && has_active && u64_from_value != u64_from_active) ||
            (has_binary && has_active && u64_from_binary != u64_from_active))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing bitmask member from JSON: value, binary and active bits do not match.");
        return RETCODE_BAD_PARAMETER;
    }

    // Safe operation after having checked values are consistent
    uint64_t value = u64_from_value | u64_from_binary | u64_from_active;

    if (9 > bound)
    {
        ret = data->set_uint8_value(MEMBER_ID_INVALID, static_cast<uint8_t>(value));
    }
    else if (17 > bound)
    {
        ret = data->set_uint16_value(MEMBER_ID_INVALID, static_cast<uint16_t>(value));
    }
    else if (33 > bound)
    {
        ret = data->set_uint32_value(MEMBER_ID_INVALID, static_cast<uint32_t>(value));
    }
    else
    {
        ret = data->set_uint64_value(MEMBER_ID_INVALID, value);
    }

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while deserializing bitmask member from JSON: failed to set value.");
        return RETCODE_BAD_PARAMETER;
    }

    return ret;
}

template<class Target>
Target numeric_get(
        const nlohmann::json& j)
{
    static_assert(std::is_arithmetic<Target>::value,
            "Target must be an arithmetic type");

    if (!j.is_number())
    {
        throw std::invalid_argument(std::string{"Expected numeric value"});
    }

    if (std::is_integral<Target>::value)
    {
        if (!j.is_number_integer())
        {
            throw std::invalid_argument(std::string{"Expected integer value"});
        }

        int64_t v = j.get<int64_t>();  // Integers are stored as int64 or uint64 in JSON
        if (std::is_unsigned<Target>::value)
        {
            if (v < 0 || static_cast<uint64_t>(v) > static_cast<uint64_t>(std::numeric_limits<Target>::max()))
            {
                throw std::out_of_range(std::string{"Unsigned value " + std::to_string(v) + " out of range"});
            }
        }
        else
        {
            if (v < static_cast<int64_t>(std::numeric_limits<Target>::min()) ||
                    v > static_cast<int64_t>(std::numeric_limits<Target>::max()))
            {
                throw std::out_of_range(std::string{"Signed value " + std::to_string(v) + " out of range"});
            }
        }
        return static_cast<Target>(v);
    }
    else if (std::is_floating_point<Target>::value)
    {
        if (!j.is_number_float() && !j.is_number_integer())
        {
            throw std::invalid_argument(std::string{"Expected floating-point value"});
        }

        double v = j.get<double>();  // Floating-point numbers are stored as double in JSON
        if (!std::isfinite(v))
        {
            throw std::out_of_range(std::string{"Floating-point value " + std::to_string(v) + " is not finite"});
        }

        if (v < static_cast<double>(std::numeric_limits<Target>::lowest()) ||
                v > static_cast<double>(std::numeric_limits<Target>::max()))
        {
            throw std::out_of_range(std::string{"Floating-point value " + std::to_string(v) + " out of range"});
        }
        return static_cast<Target>(v);
    }
    else
    {
        throw std::logic_error("Unsupported Target type");
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
