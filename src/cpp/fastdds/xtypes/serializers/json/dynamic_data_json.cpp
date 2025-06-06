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
#include <iomanip>
#include <iostream>
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

ReturnCode_t json_serialize(
        const traits<DynamicDataImpl>::ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format) noexcept
{
    if (nullptr == data)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Encountered null data value while performing DynamicData to JSON serialization.");
        return RETCODE_BAD_PARAMETER;
    }

    switch (data->type()->get_kind())
    {
        case TK_STRUCTURE:
        {
            DynamicTypeMembersById members;
            ReturnCode_t ret = data->type()->get_all_members(members);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing structure to JSON: get_all_members failed.");
                return ret;
            }
            for (const auto& it : members)
            {
                if (RETCODE_OK != (ret = json_serialize_member(data, it.second, output, format)))
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing structure member '" << it.second->get_name() <<
                            "' to JSON.");
                    break;
                }
            }
            return ret;
        }
        default:
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Only structs are supported by json_serialize method.");
            return RETCODE_BAD_PARAMETER;
        }
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
        {
            return json_serialize_basic_member(data, member_id, member_kind, member_name, output, format);
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            std::string kind_str = (member_kind == TK_STRUCTURE) ? "structure" : "bitset";
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing " << kind_str << " member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            // Fill JSON object with loaned value
            nlohmann::json j_struct;
            DynamicTypeMembersById members;
            ReturnCode_t ret = st_data->enclosing_type()->get_all_members(members);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing " << kind_str <<
                        " member to JSON: get_all_members failed.");
            }
            else
            {
                for (const auto& it : members)
                {
                    if (RETCODE_OK != (ret = json_serialize_member(st_data, it.second, j_struct, format)))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing " << kind_str << " member '" << it.second->get_name() <<
                                "' to JSON.");
                        break;
                    }
                }
                // Insert into JSON object if all members were serialized successfully
                if (RETCODE_OK == ret)
                {
                    json_insert(member_name, j_struct, output);
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
        case TK_UNION:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing union member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            // Fill JSON object with loaned value
            nlohmann::json j_union;
            ReturnCode_t ret = RETCODE_OK;
            MemberId selected_member = st_data->selected_union_member();

            if (MEMBER_ID_INVALID == selected_member)
            {
                // No member selected, insert empty JSON object
                json_insert(member_name, j_union, output);
            }
            else
            {
                DynamicTypeMember::_ref_type active_type_member;
                ret = st_data->enclosing_type()->get_member(active_type_member, selected_member);
                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                            "Error encountered while serializing union member to JSON: get_member failed.");
                }
                else
                {
                    if (RETCODE_OK == (ret = json_serialize_member(st_data, active_type_member, j_union, format)))
                    {
                        json_insert(member_name, j_union, output);
                    }
                }
            }

            // Return loaned value
            // NOTE: this should always be done, even if something went wrong before
            ReturnCode_t ret_return_loan;
            if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while returning union loaned value.");
            }
            // Give priority to prior error if occurred
            return RETCODE_OK != ret ? ret : ret_return_loan;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            std::string kind_str = (member_kind == TK_SEQUENCE) ? "sequence" : "array";
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing " << kind_str << " member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            // Fill JSON object with loaned value
            ReturnCode_t ret = json_serialize_collection(st_data, member_name, output, format);

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
        case TK_MAP:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing map member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = RETCODE_OK;
            nlohmann::json j_map;
            const TypeDescriptorImpl& map_desc = st_data->enclosing_type()->get_descriptor();
            traits<DynamicTypeImpl>::ref_type key_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
                map_desc.key_element_type())->resolve_alias_enclosed_type();
            traits<DynamicTypeImpl>::ref_type value_type = traits<DynamicType>::narrow<DynamicTypeImpl>(
                map_desc.element_type())->resolve_alias_enclosed_type();

            std::map<std::string, MemberId> key_to_id;
            if (RETCODE_OK != (ret = st_data->get_keys(key_to_id)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing map member to JSON: get_keys failed.");
            }

            if (RETCODE_OK == ret)
            {
                std::map<MemberId, std::string> id_to_key;
                for (const auto& it : key_to_id)
                {
                    id_to_key[it.second] = it.first;
                }
                assert(id_to_key.size() == key_to_id.size());

                uint32_t size = st_data->get_item_count();
                assert(size == key_to_id.size());
                for (uint32_t i = 0; i < size; ++i)
                {
                    MemberId id = st_data->get_member_id_at_index(i);
                    if (MEMBER_ID_INVALID == id)
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing map member's member to JSON: invalid member id.");
                        ret = RETCODE_BAD_PARAMETER;
                        break;
                    }

                    auto it = id_to_key.find(id);
                    if (it == id_to_key.end())
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing map member's member to JSON: key not found.");
                        ret = RETCODE_BAD_PARAMETER;
                        break;
                    }

                    if (RETCODE_OK !=
                            (ret =
                            json_serialize_member(st_data, id, value_type->get_kind(), it->second, j_map,
                            format)))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing map member's member to JSON.");
                        break;
                    }
                }
            }

            // Insert into JSON object if all members were serialized successfully
            if (RETCODE_OK == ret)
            {
                json_insert(member_name, j_map, output);
            }

            // Return loaned value
            // NOTE: this should always be done, even if something went wrong before
            ReturnCode_t ret_return_loan;
            if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while returning map loaned value.");
            }
            // Give priority to prior error if occurred
            return RETCODE_OK != ret ? ret : ret_return_loan;
        }
        case TK_BITMASK:
        {
            traits<DynamicDataImpl>::ref_type st_data =
                    traits<DynamicData>::narrow<DynamicDataImpl>(data->loan_value(member_id));
            if (nullptr == st_data)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing bitmask member to JSON: loan_value failed.");
                return RETCODE_BAD_PARAMETER;
            }

            ReturnCode_t ret = RETCODE_OK;
            traits<DynamicTypeImpl>::ref_type bitmask_type = st_data->enclosing_type();
            const TypeDescriptorImpl& bitmask_desc = bitmask_type->get_descriptor();

            auto bound = bitmask_desc.bound().at(0);

            if (format == DynamicDataJsonFormat::OMG)
            {
                if (9 > bound)
                {
                    uint8_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint8_value(value, MEMBER_ID_INVALID)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else if (17 > bound)
                {
                    uint16_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint16_value(value, MEMBER_ID_INVALID)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else if (33 > bound)
                {
                    uint32_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint32_value(value, MEMBER_ID_INVALID)))
                    {
                        json_insert(member_name, value, output);
                    }
                }
                else
                {
                    uint64_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint64_value(value, MEMBER_ID_INVALID)))
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
            else if (format == DynamicDataJsonFormat::EPROSIMA)
            {
                nlohmann::json bitmask_dict;
                uint64_t u64_value{0}; // Auxiliar variable to check active bits afterwards
                if (9 > bound)
                {
                    uint8_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint8_value(value, MEMBER_ID_INVALID)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<8>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else if (17 > bound)
                {
                    uint16_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint16_value(value, MEMBER_ID_INVALID)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<16>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else if (33 > bound)
                {
                    uint32_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint32_value(value, MEMBER_ID_INVALID)))
                    {
                        bitmask_dict["value"] = value;
                        bitmask_dict["binary"] = std::bitset<32>(value).to_string();
                        u64_value = static_cast<uint64_t>(value);
                    }
                }
                else
                {
                    uint64_t value;
                    if (RETCODE_OK == (ret = st_data->get_uint64_value(value, MEMBER_ID_INVALID)))
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
                }
                else
                {
                    // Check active bits
                    DynamicTypeMembersById bitmask_members;
                    if (RETCODE_OK != (ret = bitmask_type->get_all_members(bitmask_members)))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                                "Error encountered while serializing bitmask member to JSON: get_all_members failed.");
                    }
                    else
                    {
                        std::vector<std::string> active_bits;
                        for (const auto& it : bitmask_members)
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

            // Return loaned value
            // NOTE: this should always be done, even if something went wrong before
            ReturnCode_t ret_return_loan;
            if (RETCODE_OK != (ret_return_loan = data->return_loaned_value(st_data)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while returning bitmask loaned value.");
            }
            // Give priority to prior error if occurred
            return RETCODE_OK != ret ? ret : ret_return_loan;
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
#if defined(MINGW_COMPILER)
                std::wstring aux_wstring_value({value});
                std::string utf8_value;
                int size_needed = std::wcstombs(nullptr, aux_wstring_value.data(), 0);
                if (size_needed > 0)
                {
                    utf8_value.resize(size_needed);
                    std::wcstombs(&utf8_value[0], aux_wstring_value.data(), size_needed);
                }
#else
                std::wstring aux_wstring_value({value});
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                std::string utf8_value = converter.to_bytes(aux_wstring_value);

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
#ifdef MINGW_COMPILER
                std::string utf8_value;
                int size_needed = std::wcstombs(nullptr, value.data(), 0);
                if (size_needed > 0)
                {
                    utf8_value.resize(size_needed);
                    std::wcstombs(&utf8_value[0], value.data(), size_needed);
                }
#else
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                std::string utf8_value = converter.to_bytes(value);
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
            int32_t value;
            ReturnCode_t ret = data->get_int32_value(value, member_id);
            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing TK_ENUM member to JSON.");
                return ret;
            }

            // Get enumeration type to obtain the names of the different values
            // NOTE: a different approach is required for collections and other "holder" types (e.g. structures),
            // as unlike with DynamicData::loan_value or DynamicData::get_X_value, DynamicData::get_descriptor method
            // is not meant to work with sequences nor arrays according to XTypes standard.
            traits<DynamicType>::ref_type enum_type;
            TypeKind holder_kind = data->enclosing_type()->get_kind();
            if (TK_ARRAY == holder_kind || TK_SEQUENCE == holder_kind)
            {
                const TypeDescriptorImpl& collection_descriptor = data->enclosing_type()->get_descriptor();
                enum_type = collection_descriptor.element_type();
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
                enum_type = enum_desc->type();
            }

            DynamicTypeMembersByName all_members;
            if (RETCODE_OK !=
                    (ret =
                    traits<DynamicType>::narrow<DynamicTypeImpl>(enum_type)->resolve_alias_enclosed_type()->
                            get_all_members_by_name(all_members)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing TK_ENUM member to JSON: get_all_members_by_name failed.");
                return ret;
            }

            ObjectName name;
            ret = RETCODE_BAD_PARAMETER;
            for (const auto& it : all_members)
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
                EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                        "Error encountered while serializing TK_ENUM member to JSON: enum value not found.");
            }
            return ret;
        }
        default:
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while serializing basic member to JSON: unexpected kind " << member_kind <<
                    " found.");
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
        const TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();

        auto count = data->get_item_count();
        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t index = 0; index < count; ++index)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index),
                    traits<DynamicType>::narrow<DynamicTypeImpl>(descriptor.element_type())->resolve_alias_enclosed_type()
                            ->get_kind(), j_array,
                    format)))
            {
                EPROSIMA_LOG_ERROR(XTYPES_UTILS, "Error encountered while serializing sequence collection to JSON.");
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
        const TypeDescriptorImpl& descriptor = data->enclosing_type()->get_descriptor();

        const BoundSeq& bounds = descriptor.bound();
        nlohmann::json j_array = nlohmann::json::array();
        unsigned int index = 0;
        if (RETCODE_OK != (ret = json_serialize_array(data, traits<DynamicType>::narrow<DynamicTypeImpl>(
                    descriptor.element_type())->resolve_alias_enclosed_type()->get_kind(), index, bounds, j_array,
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
                    json_serialize_array(data, member_kind, index,
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

} // namespace dds
} // namespace fastdds
} // namespace eprosima
