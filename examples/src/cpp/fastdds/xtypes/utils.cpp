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

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

// TODO: include only what it's used
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

////////////////////////////////////////
// Dynamic Data to JSON serialization //
////////////////////////////////////////

//// Forward declarations

static ReturnCode_t json_serialize(
        const traits<DynamicData>::ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_member(
        const traits<DynamicData>::ref_type& data,
        const traits<DynamicTypeMember>::ref_type& type_member,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_member(
        const DynamicData::_ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_member(
        const DynamicData::_ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_basic_member(
        const DynamicData::_ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_collection(
        const DynamicData::_ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format);

static ReturnCode_t json_serialize_array(
        const DynamicData::_ref_type& data,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        nlohmann::json& j_array,
        DynamicDataJsonFormat format);

//// Implementation

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        std::ostream& output,
        DynamicDataJsonFormat format)
{
    ReturnCode_t ret;
    nlohmann::json j;

    if (RETCODE_OK == (ret = json_serialize(data, j, format)))
    {
        output << j;
    }
    else
    {
        // logWarning ????
    }
    return ret;
}

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        std::string& output,
        DynamicDataJsonFormat format)
{
    // TODO: is there any way to use overload above? i.e. create a std::ostream to from a std::string,
    // or instanciate a std::ostream in general (perhaps possible from sstream).

    ReturnCode_t ret;
    nlohmann::json j;

    if (RETCODE_OK == (ret = json_serialize(data, j, format)))
    {
        output = j.dump();
    }
    else
    {
        // logWarning ????
    }
    return ret;
}

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
{
    if (nullptr != data)
    {
        switch (data->type()->get_kind())
        {
            case TK_STRUCTURE:
            {
                ReturnCode_t ret = RETCODE_OK;
                DynamicTypeMembersById members;
                data->type()->get_all_members(members);
                for (auto it : members)
                {
                    if (RETCODE_OK != (ret = json_serialize_member(data, it.second, output, format)))
                    {
                        break;
                    }
                }
                return ret;
            }
            default:
            {
                // std::cout << "Only structs are supported by DynamicDataHelper::print method." << std::endl;
                // logWarning
                return RETCODE_BAD_PARAMETER;
            }
        }
    }
    else
    {
        // logWarning
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t json_serialize_member(
        const DynamicData::_ref_type& data,
        const traits<DynamicTypeMember>::ref_type& type_member,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
{
    MemberDescriptor::_ref_type member_desc {traits<MemberDescriptor>::make_shared()};
    type_member->get_descriptor(member_desc);

    return json_serialize_member(data, type_member->get_id(),
                   member_desc->type()->enclosed_type()->get_kind(),
                   type_member->get_name().to_string(), output, format);
}

ReturnCode_t json_serialize_member(
        const DynamicData::_ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
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
            ReturnCode_t ret = RETCODE_OK;
            nlohmann::json j_struct;
            DynamicData::_ref_type st_data = data->loan_value(member_id);
            DynamicTypeMembersById members;
            st_data->enclosed_type()->get_all_members(members);
            for (auto it : members)
            {
                if (RETCODE_OK != (ret = json_serialize_member(st_data, it.second, j_struct, format)))
                {
                    break;
                }
            }
            if (RETCODE_OK == ret)
            {
                if (output.is_array())
                {
                    output.push_back(j_struct);
                }
                else
                {
                    output[member_name] = j_struct;
                }
            }
            data->return_loaned_value(st_data);
            return ret;
        }
        // case TK_UNION:
        // {
        //     ReturnCode_t ret = RETCODE_OK;
        //     nlohmann::json j_union;
        //     DynamicData::_ref_type st_data = data->loan_value(type_member->get_id());

        //     // AD-HOC for short discriminator -> TODO: generalize
        //     int16_t aux_discriminator_label_value;
        //     st_data->get_int16_value(aux_discriminator_label_value, 0);
        //     int32_t discriminator_label_value = static_cast<int32_t>(aux_discriminator_label_value);

        //     DynamicTypeMembersById union_members;
        //     st_data->type()->get_all_members(union_members);
        //     MemberDescriptor::_ref_type union_member_desc {traits<MemberDescriptor>::make_shared()};
        //     traits<DynamicTypeMember>::ref_type selected_member_type;
        //     for (auto it : union_members)
        //     {
        //         if (it.first == 0)
        //         {
        //             // Skip discriminator member
        //             continue;
        //         }
        //         it.second->get_descriptor(union_member_desc);
        //         auto labels = union_member_desc->label();
        //         if (labels.size() && labels.at(0) == discriminator_label_value)
        //         {
        //             selected_member_type = it.second;
        //             break;
        //         }
        //         else if (union_member_desc->is_default_label())
        //         {
        //             selected_member_type = it.second;
        //         }
        //     }
        //     ret = json_serialize_member(st_data, selected_member_type, j_union, format);
        //     if (RETCODE_OK == ret)
        //     {
        //         output[member_name] = j_union;
        //     }
        //     data->return_loaned_value(st_data);
        //     return ret;
        // }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            DynamicData::_ref_type st_data = data->loan_value(member_id);
            // TODO: add checkers for nullptr return values
            ReturnCode_t ret = json_serialize_collection(st_data, member_name, output, format);
            data->return_loaned_value(st_data);
            return ret;
        }
        // case TK_MAP:
        // {
        //     nlohmann::json j_map;
        //     DynamicData::_ref_type st_data = data->loan_value(type->get_id());
        //     TypeDescriptor::_ref_type map_desc {traits<TypeDescriptor>::make_shared()};
        //     st_data->type()->get_descriptor(map_desc);
        //     DynamicType::_ref_type key_type = map_desc->key_element_type();
        //     DynamicType::_ref_type value_type = map_desc->element_type();
        //     size_t size = st_data->get_item_count();
        //     for (size_t i = 0; i < size; ++i)
        //     {
        //         auto id = st_data->get_member_id_at_index(i);
        //         print_element_json(st_data, id, value_type, std::to_string(id), j_map);
        //     }
        //     j[member_name] = j_map;
        //     data->return_loaned_value(st_data);
        //     break;
        // }
        case TK_ALIAS:
        {
            // should not happen
            return RETCODE_BAD_PARAMETER;
        }
        default:
            // logWarning
            // return RETCODE_BAD_PARAMETER;
            return RETCODE_OK;
    }

    // return RETCODE_BAD_PARAMETER;
}

ReturnCode_t json_serialize_member(
        const DynamicData::_ref_type& data,
        // const traits<DynamicTypeMember>::ref_type& type_member,
        MemberId member_id,
        TypeKind member_kind,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
{
    return json_serialize_member(data, member_id, member_kind, "", output, format);
}

ReturnCode_t json_serialize_basic_member(
        const DynamicData::_ref_type& data,
        MemberId member_id,
        TypeKind member_kind,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
{
    switch (member_kind)
    {
        case TK_NONE:
        {
            // std::cout << "<type not defined!>";
            // logWarning;
            return RETCODE_BAD_PARAMETER;
        }
        case TK_BOOLEAN:
        {
            ReturnCode_t ret;
            bool value;
            if (RETCODE_OK == (ret = data->get_boolean_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_BYTE:
        {
            ReturnCode_t ret;
            eprosima::fastrtps::rtps::octet value;
            if (RETCODE_OK == (ret = data->get_byte_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_INT8:
        {
            ReturnCode_t ret;
            int8_t value;
            if (RETCODE_OK == (ret = data->get_int8_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_INT16:
        {
            ReturnCode_t ret;
            int16_t value;
            if (RETCODE_OK == (ret = data->get_int16_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_INT32:
        {
            ReturnCode_t ret;
            int32_t value;
            if (RETCODE_OK == (ret = data->get_int32_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        // case TK_ENUM:
        // {
        //     ReturnCode_t ret;
        //     int32_t value;
        //     data->get_int32_value(value, member_id);

        //     // TODO: improve this shitty code, maybe requires extending API (get_all_members_by_index , not in standard though)
        //     MemberDescriptor::_ref_type enum_desc {traits<MemberDescriptor>::make_shared()};
        //     data->get_descriptor(enum_desc, member_id);

        //     DynamicTypeMembersByName all_members;
        //     enum_desc->type()->get_all_members_by_name(all_members);
        //     ObjectName name;
        //     for (auto it : all_members)
        //     {
        //         MemberDescriptor::_ref_type enum_member_desc {traits<MemberDescriptor>::make_shared()};
        //         it.second->get_descriptor(enum_member_desc);
        //         // TODO: perhaps should actually compare with default_value (string though)
        //         // if (static_cast<int32_t>(enum_member_desc->index()) == value) // TODO: assert there is not problem with indexing enums (possible to set int32_t value via annotation, but possibly stored in map with MemberId (uint32_t) as key)
        //                                                                          // test with dynamic type annotated once available.
        //                                                                          // Also note in XML parser, member type is set to uint32_t , verify if this is correct
        //         if (enum_member_desc->default_value() == std::to_string(value))
        //         {
        //             name = it.first; // should also be possible to take it.second->name() , add assert
        //             assert(name == it.second->get_name());
        //             break;
        //         }
        //     }

        //     output[member_name] = { {"name", name}, {"literal", value} };
        //     break;
        // }
        case TK_INT64:
        {
            ReturnCode_t ret;
            int64_t value;
            if (RETCODE_OK == (ret = data->get_int64_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_UINT8:
        {
            ReturnCode_t ret;
            uint8_t value;
            if (RETCODE_OK == (ret = data->get_uint8_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_UINT16:
        {
            ReturnCode_t ret;
            uint16_t value;
            if (RETCODE_OK == (ret = data->get_uint16_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_UINT32:
        {
            ReturnCode_t ret;
            uint32_t value;
            if (RETCODE_OK == (ret = data->get_uint32_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_UINT64:
        {
            ReturnCode_t ret;
            uint64_t value;
            if (RETCODE_OK == (ret = data->get_uint64_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_FLOAT32:
        {
            ReturnCode_t ret;
            float value;
            if (RETCODE_OK == (ret = data->get_float32_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_FLOAT64:
        {
            ReturnCode_t ret;
            double value;
            if (RETCODE_OK == (ret = data->get_float64_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_FLOAT128:
        {
            ReturnCode_t ret;
            long double value;
            if (RETCODE_OK == (ret = data->get_float128_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_CHAR8:
        {
            ReturnCode_t ret;
            char value;
            if (RETCODE_OK == (ret = data->get_char8_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_CHAR16:
        {
            ReturnCode_t ret;
            wchar_t value;
            if (RETCODE_OK == (ret = data->get_char16_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_STRING8:
        {
            ReturnCode_t ret;
            std::string value;
            if (RETCODE_OK == (ret = data->get_string_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        case TK_STRING16:
        {
            ReturnCode_t ret;
            std::wstring value;
            if (RETCODE_OK == (ret = data->get_wstring_value(value, member_id)))
            {
                if (output.is_array())
                {
                    output.push_back(value);
                }
                else
                {
                    output[member_name] = value;
                }
            }
            return ret;
        }
        // case TK_BITMASK:
        // {
        //     size_t size = data->type()->get_size();
        //     switch (size)
        //     {
        //         case 1: std::cout << data->get_uint8_value(id); break;
        //         case 2: std::cout << data->get_uint16_value(id); break;
        //         case 3: std::cout << data->get_uint32_value(id); break;
        //         case 4: std::cout << data->get_uint64_value(id); break;
        //     }
        //     break;
        // }
        default:
            // logWarning
            // return RETCODE_BAD_PARAMETER;
            return RETCODE_OK;
    }
}

ReturnCode_t json_serialize_collection(
        const DynamicData::_ref_type& data,
        const std::string& member_name,
        nlohmann::json& output,
        DynamicDataJsonFormat format)
{
    ReturnCode_t ret = RETCODE_OK;
    if (data->enclosed_type()->get_kind() == TK_SEQUENCE)
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->enclosed_type()->get_descriptor(descriptor);

        auto count = data->get_item_count();
        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t index = 0; index < count; ++index)
        {
            if (RETCODE_OK !=
                    (ret =
                    json_serialize_member(data, static_cast<MemberId>(index),
                    descriptor->element_type()->get_kind(), j_array, format)))
            {
                break;
            }
        }
        if (RETCODE_OK == ret)
        {
            if (output.is_array())
            {
                output.push_back(j_array);
            }
            else
            {
                output[member_name] = j_array;
            }
        }
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->enclosed_type()->get_descriptor(descriptor);

        const BoundSeq& bounds = descriptor->bound();
        nlohmann::json j_array = nlohmann::json::array();
        unsigned int index = 0;
        ret = json_serialize_array(data, descriptor->element_type()->get_kind(), index, bounds, j_array, format);
        if (RETCODE_OK == ret)
        {
            if (output.is_array())
            {
                output.push_back(j_array);
            }
            else
            {
                output[member_name] = j_array;
            }
        }
    }
    return ret;
}

ReturnCode_t json_serialize_array(
        const DynamicData::_ref_type& data,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        nlohmann::json& j_array,
        DynamicDataJsonFormat format)
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
                break;
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < bounds[0]; ++i)
        {
            nlohmann::json inner_array = nlohmann::json::array();
            if (RETCODE_OK ==
                    (ret =
                    json_serialize_array(data, member_kind, index,
                    std::vector<unsigned int>(bounds.begin() + 1, bounds.end()), inner_array, format)))
            {
                j_array.push_back(inner_array);
            }
            else
            {
                break;
            }
        }
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Dynamic Data to JSON serialization //// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace dds
} // namespace fastdds
} // namespace eprosima
