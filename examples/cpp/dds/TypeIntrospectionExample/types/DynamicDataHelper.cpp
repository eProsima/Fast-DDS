// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cassert>

#include <nlohmann/json.hpp>

// #include <fastrtps/types/MemberDescriptor.h>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>

#include "DynamicDataHelper.hpp"

using namespace eprosima::fastdds::dds;
// using namespace eprosima::fastdds::dds::xtypes;

// using DynamicTypeMember = eprosima::fastdds::dds::DynamicTypeMember;
// using TypeDescriptor = eprosima::fastdds::dds::TypeDescriptor;
// using MemberDescriptor = eprosima::fastdds::dds::MemberDescriptor;
// using TypeKind = eprosima::fastdds::dds::xtypes::TypeKind;
// using MemberId = eprosima::fastdds::dds::xtypes::MemberId;

void DynamicDataHelper::print(
        const DynamicData::_ref_type& data)
{
    if (nullptr != data)
    {
        switch (data->type()->get_kind())
        {
            case TK_STRUCTURE:
            {
                DynamicTypeMembersById members;
                data->type()->get_all_members(members);
                for (auto it : members)
                {
                    print_member(data, it.second);
                }
                break;
            }
            default:
            {
                std::cout << "Only structs are supported by DynamicDataHelper::print method." << std::endl;
            }
        }
    }
    else
    {
        std::cout << "<NULL>" << std::endl;
    }
}

void DynamicDataHelper::print_json(
        const DynamicData::_ref_type& data)
{
    if (nullptr != data)
    {
        switch (data->type()->get_kind())
        {
            case TK_STRUCTURE:
            {
                nlohmann::json j; // TODO: use array instead if want to preserve original ordering
                DynamicTypeMembersById members;
                data->type()->get_all_members(members);
                for (auto it : members)
                {
                    print_member_json(data, it.second, j);
                }
                std::cout << std::setw(2) << j << std::endl;
                break;
            }
            default:
            {
                std::cout << "Only structs are supported by DynamicDataHelper::print method." << std::endl;
            }
        }
    }
    else
    {
        std::cout << "<NULL>" << std::endl;
    }
}

std::ostream& DynamicDataHelper::print(
        std::ostream& output,
        const DynamicData::_ref_type& data)
{
    if (nullptr != data)
    {
        switch (data->type()->get_kind())
        {
            case TK_STRUCTURE:
            {
                DynamicTypeMembersById members;
                data->type()->get_all_members(members);
                for (auto it : members)
                {
                    print_member(data, output, it.second);
                }
                break;
            }
            default:
            {
                output << "Only structs are supported by DynamicDataHelper::print method.\n";
            }
        }
    }
    else
    {
        output << "<NULL>\n";
    }
    return output;
}

void DynamicDataHelper::print_member(
        DynamicData::_ref_type data,
        const traits<DynamicTypeMember>::ref_type& type,
        const std::string& tabs)
{
    std::cout << tabs << type->get_name() << ": ";
    MemberDescriptor::_ref_type desc {traits<MemberDescriptor>::make_shared()};
    type->get_descriptor(desc);

    switch (desc->type()->get_kind())
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
            print_basic_element(data, type->get_id(), desc->type()->get_kind());
            std::cout << std::endl;
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            std::cout << "<struct/bitset>" << std::endl;
            DynamicTypeMembersById members;
            // desc->type()->get_all_members(members);
            st_data->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member(st_data, it.second, tabs + "\t");
            }
            data->return_loaned_value(st_data);
            break;
        }
        case TK_UNION:
        {
            // std::cout << "<union>" << std::endl;
            // DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            // traits<DynamicTypeMember>::ref_type member;
            // desc->type()->get_member(member, st_data->union_id_);
            // print_member(st_data, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            print_collection(st_data, tabs + "\t");
            data->return_loaned_value(st_data);
            break;
        }
        case TK_MAP:
        {
            std::cout << "<map>" << std::endl;
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            TypeDescriptor::_ref_type map_desc {traits<TypeDescriptor>::make_shared()};
            st_data->type()->get_descriptor(map_desc);
            DynamicType::_ref_type key_type = map_desc->key_element_type();
            DynamicType::_ref_type value_type = map_desc->element_type();
            size_t size = st_data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                // size_t index = i * 2;
                // std::cout << "\tKey: ";
                // print_basic_element(st_data, index, key_type->get_kind());
                // std::cout << "\tValue: ";
                // print_basic_element(st_data, index + 1, value_type->get_kind());

                std::cout << tabs << "\tKey: ";
                auto id = st_data->get_member_id_at_index(i);
                std::cout << std::to_string(id); // TODO: print key value string (currently missing API)
                std::cout << tabs << "\tValue: ";
                // print_basic_element(st_data, id, value_type->get_kind());
                print_element(st_data, id, value_type, tabs + "\t");
                std::cout << std::endl;
            }
            data->return_loaned_value(st_data);
            break;
        }
        // case TK_ALIAS:  // TODO: move to helper that gets actual kind from alias
        // {
        //     traits<DynamicTypeMember>::ref_type aliased_type = type;
        //     MemberDescriptor::_ref_type aliased_desc {traits<MemberDescriptor>::make_shared()};
        //     do {
        //         aliased_type->get_descriptor(aliased_desc);
        //         aliased_type = aliased_desc->base_type();
        //     } while (TK_ALIAS == aliased_type->get_kind());
        //     print_member(data, aliased_type, tabs);
        //     break;
        // }
        default:
            break;
    }
}

void DynamicDataHelper::print_member_json(
        DynamicData::_ref_type data,
        const traits<DynamicTypeMember>::ref_type& type,
        nlohmann::json& j)
{
    std::string member_name = type->get_name().to_string();
    MemberDescriptor::_ref_type desc {traits<MemberDescriptor>::make_shared()};
    type->get_descriptor(desc);

    switch (desc->type()->get_kind())
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
            print_basic_element_json(data, type->get_id(), desc->type()->get_kind(), member_name, j);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            nlohmann::json j_struct;
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member_json(st_data, it.second, j_struct);
            }
            j[member_name] = j_struct;
            data->return_loaned_value(st_data);
            break;
        }
        case TK_UNION:
        {
            // std::cout << "<union>" << std::endl;
            // DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            // traits<DynamicTypeMember>::ref_type member;
            // desc->type()->get_member(member, st_data->union_id_);
            // print_member(st_data, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            print_collection_json(st_data, member_name, j);
            data->return_loaned_value(st_data);
            break;
        }
        case TK_MAP:
        {
            nlohmann::json j_map;
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            TypeDescriptor::_ref_type map_desc {traits<TypeDescriptor>::make_shared()};
            st_data->type()->get_descriptor(map_desc);
            DynamicType::_ref_type key_type = map_desc->key_element_type();
            DynamicType::_ref_type value_type = map_desc->element_type();
            size_t size = st_data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                auto id = st_data->get_member_id_at_index(i);
                print_element_json(st_data, id, value_type, std::to_string(id), j_map);
            }
            j[member_name] = j_map;
            data->return_loaned_value(st_data);
            break;
        }
        // case TK_ALIAS:  // TODO: move to helper that gets actual kind from alias
        // {
        //     traits<DynamicTypeMember>::ref_type aliased_type = type;
        //     MemberDescriptor::_ref_type aliased_desc {traits<MemberDescriptor>::make_shared()};
        //     do {
        //         aliased_type->get_descriptor(aliased_desc);
        //         aliased_type = aliased_desc->base_type();
        //     } while (TK_ALIAS == aliased_type->get_kind());
        //     print_member(data, aliased_type, tabs);
        //     break;
        // }
        default:
            break;
    }
}

void DynamicDataHelper::print_member(
        DynamicData::_ref_type data,
        std::ostream& output,
        const traits<DynamicTypeMember>::ref_type& type,
        const std::string& tabs)
{
    output << tabs;
    output << type->get_name();
    output << ": ";
    MemberDescriptor::_ref_type desc {traits<MemberDescriptor>::make_shared()};
    type->get_descriptor(desc);
    switch (desc->type()->get_kind())
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
            print_basic_element(data, type->get_id(), desc->type()->get_kind(), output);
            output << "\n";
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            output << "<struct/bitset>\n";
            DynamicTypeMembersById members;
            desc->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member(st_data, output, it.second, tabs + "\t");
            }
            data->return_loaned_value(st_data);
            break;
        }
        case TK_UNION:
        {
            // output << "<union>\n";
            // DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            // traits<DynamicTypeMember>::ref_type member;
            // desc->type()->get_member(member, data->union_id_);
            // print_member(st_data, output, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            print_collection(st_data, output, tabs + "\t");
            data->return_loaned_value(st_data);
            break;
        }
        case TK_MAP:
        {
            std::cout << "TODO" << std::endl;
            // output << "<map>\n";
            // DynamicData::_ref_type st_data = data->loan_value(type->get_id());
            // DynamicTypeMembersById members;
            // // desc->type()->get_all_members(members);
            // type->get_all_members(members);
            // size_t size = data->get_item_count();
            // for (size_t i = 0; i < size; ++i)
            // {
            //     size_t index = i * 2;
            //     MemberId id = data->get_member_id_at_index(static_cast<uint32_t>(index));
            //     output << "Key: ";
            //     print_member(st_data, output, members[id], tabs + "\t");
            //     id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
            //     output << "Value: ";
            //     print_member(st_data, output, members[id], tabs + "\t");
            // }
            // data->return_loaned_value(st_data);
            break;
        }
        default:
            break;
    }
}

void DynamicDataHelper::print_basic_element(
        DynamicData::_ref_type data,
        MemberId id,
        TypeKind kind)
{
    switch (kind)
    {
        case TK_NONE:
        {
            std::cout << "<type not defined!>";
            break;
        }
        case TK_BOOLEAN:
        {
            bool value;
            data->get_boolean_value(value, id);
            std::cout << (value ? "true" : "false");
            break;
        }
        case TK_BYTE:
        {
            eprosima::fastrtps::rtps::octet value;
            data->get_byte_value(value, id);
            std::cout << static_cast<uint16_t>(value); // TODO: print as char or as number? or even binary/hex?
            break;
        }
        case TK_INT8:
        {
            int8_t value;
            data->get_int8_value(value, id);
            std::cout << static_cast<int16_t>(value);
            break;
        }
        case TK_INT16:
        {
            int16_t value;
            data->get_int16_value(value, id);
            std::cout << value;
            break;
        }
        case TK_INT32:
        // case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);
            std::cout << value;
            break;
        }
        case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);

            // TODO: improve this shitty code, maybe requires extending API (get_all_members_by_index , not in standard though)
            MemberDescriptor::_ref_type enum_desc {traits<MemberDescriptor>::make_shared()};
            data->get_descriptor(enum_desc, id);

            DynamicTypeMembersByName all_members;
            enum_desc->type()->get_all_members_by_name(all_members);
            ObjectName name;
            for (auto it : all_members)
            {
                MemberDescriptor::_ref_type enum_member_desc {traits<MemberDescriptor>::make_shared()};
                it.second->get_descriptor(enum_member_desc);
                // TODO: perhaps should actually compare with default_value (string though)
                // if (static_cast<int32_t>(enum_member_desc->index()) == value) // TODO: assert there is not problem with indexing enums (possible to set int32_t value via annotation, but possibly stored in map with MemberId (uint32_t) as key)
                                                                                 // test with dynamic type annotated once available.
                                                                                 // Also note in XML parser, member type is set to uint32_t , verify if this is correct
                if (enum_member_desc->default_value() == std::to_string(value))
                {
                    name = it.first; // should also be possible to take it.second->name() , add assert
                    assert(name == it.second->get_name());
                    break;
                }
            }

            std::cout << name << " (" << value << ")";
            break;
        }
        case TK_INT64:
        {
            int64_t value;
            data->get_int64_value(value, id);
            std::cout << value;
            break;
        }
        case TK_UINT8:
        {
            uint8_t value;
            data->get_uint8_value(value, id);
            std::cout << static_cast<uint16_t>(value);
            break;
        }
        case TK_UINT16:
        {
            uint16_t value;
            data->get_uint16_value(value, id);
            std::cout << value;
            break;
        }
        case TK_UINT32:
        {
            uint32_t value;
            data->get_uint32_value(value, id);
            std::cout << value;
            break;
        }
        case TK_UINT64:
        {
            uint64_t value;
            data->get_uint64_value(value, id);
            std::cout << value;
            break;
        }
        case TK_FLOAT32:
        {
            float value;
            data->get_float32_value(value, id);
            std::cout << value;
            break;
        }
        case TK_FLOAT64:
        {
            double value;
            data->get_float64_value(value, id);
            std::cout << value;
            break;
        }
        case TK_FLOAT128:
        {
            long double value;
            data->get_float128_value(value, id);
            std::cout << value;
            break;
        }
        case TK_CHAR8:
        {
            char value;
            data->get_char8_value(value, id);
            std::cout << value;
            break;
        }
        case TK_CHAR16:
        {
            wchar_t value;
            data->get_char16_value(value, id);
            std::cout << value;
            break;
        }
        case TK_STRING8:
        {
            std::string value;
            data->get_string_value(value, id);
            std::cout << value;
            break;
        }
        case TK_STRING16:
        {
            std::wstring value;
            data->get_wstring_value(value, id);
            std::wcout << value;
            break;
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
            break;
    }
}

void DynamicDataHelper::print_basic_element_json(
        DynamicData::_ref_type data,
        MemberId id,
        TypeKind kind,
        const std::string& member_name,
        nlohmann::json& j)
{
    switch (kind)
    {
        case TK_NONE:
        {
            // std::cout << "<type not defined!>";
            // logWarning;
            break;
        }
        case TK_BOOLEAN:
        {
            bool value;
            data->get_boolean_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_BYTE:
        {
            eprosima::fastrtps::rtps::octet value;
            data->get_byte_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_INT8:
        {
            int8_t value;
            data->get_int8_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_INT16:
        {
            int16_t value;
            data->get_int16_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_INT32:
        // case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);

            // TODO: improve this shitty code, maybe requires extending API (get_all_members_by_index , not in standard though)
            MemberDescriptor::_ref_type enum_desc {traits<MemberDescriptor>::make_shared()};
            data->get_descriptor(enum_desc, id);

            DynamicTypeMembersByName all_members;
            enum_desc->type()->get_all_members_by_name(all_members);
            ObjectName name;
            for (auto it : all_members)
            {
                MemberDescriptor::_ref_type enum_member_desc {traits<MemberDescriptor>::make_shared()};
                it.second->get_descriptor(enum_member_desc);
                // TODO: perhaps should actually compare with default_value (string though)
                // if (static_cast<int32_t>(enum_member_desc->index()) == value) // TODO: assert there is not problem with indexing enums (possible to set int32_t value via annotation, but possibly stored in map with MemberId (uint32_t) as key)
                                                                                 // test with dynamic type annotated once available.
                                                                                 // Also note in XML parser, member type is set to uint32_t , verify if this is correct
                if (enum_member_desc->default_value() == std::to_string(value))
                {
                    name = it.first; // should also be possible to take it.second->name() , add assert
                    assert(name == it.second->get_name());
                    break;
                }
            }

            j[member_name] = { {"name", name}, {"value", value} };
            break;
        }
        case TK_INT64:
        {
            int64_t value;
            data->get_int64_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_UINT8:
        {
            uint8_t value;
            data->get_uint8_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_UINT16:
        {
            uint16_t value;
            data->get_uint16_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_UINT32:
        {
            uint32_t value;
            data->get_uint32_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_UINT64:
        {
            uint64_t value;
            data->get_uint64_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_FLOAT32:
        {
            float value;
            data->get_float32_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_FLOAT64:
        {
            double value;
            data->get_float64_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_FLOAT128:
        {
            long double value;
            data->get_float128_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_CHAR8:
        {
            char value;
            data->get_char8_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_CHAR16:
        {
            wchar_t value;
            data->get_char16_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_STRING8:
        {
            std::string value;
            data->get_string_value(value, id);
            j[member_name] = value;
            break;
        }
        case TK_STRING16:
        {
            std::wstring value;
            data->get_wstring_value(value, id);
            j[member_name] = value;
            break;
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
            break;
    }
}

void DynamicDataHelper::print_basic_element_json(
        DynamicData::_ref_type data,
        MemberId id,
        TypeKind kind,
        nlohmann::json& j)
{
    switch (kind)
    {
        case TK_NONE:
        {
            // std::cout << "<type not defined!>";
            // logWarning;
            break;
        }
        case TK_BOOLEAN:
        {
            bool value;
            data->get_boolean_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_BYTE:
        {
            eprosima::fastrtps::rtps::octet value;
            data->get_byte_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_INT8:
        {
            int8_t value;
            data->get_int8_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_INT16:
        {
            int16_t value;
            data->get_int16_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_INT32:
        // case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);

            // TODO: improve this shitty code, maybe requires extending API (get_all_members_by_index , not in standard though)
            MemberDescriptor::_ref_type enum_desc {traits<MemberDescriptor>::make_shared()};
            data->get_descriptor(enum_desc, id);

            DynamicTypeMembersByName all_members;
            enum_desc->type()->get_all_members_by_name(all_members);
            ObjectName name;
            for (auto it : all_members)
            {
                MemberDescriptor::_ref_type enum_member_desc {traits<MemberDescriptor>::make_shared()};
                it.second->get_descriptor(enum_member_desc);
                // TODO: perhaps should actually compare with default_value (string though)
                // if (static_cast<int32_t>(enum_member_desc->index()) == value) // TODO: assert there is not problem with indexing enums (possible to set int32_t value via annotation, but possibly stored in map with MemberId (uint32_t) as key)
                                                                                 // test with dynamic type annotated once available.
                                                                                 // Also note in XML parser, member type is set to uint32_t , verify if this is correct
                if (enum_member_desc->default_value() == std::to_string(value))
                {
                    name = it.first; // should also be possible to take it.second->name() , add assert
                    assert(name == it.second->get_name());
                    break;
                }
            }
            j.push_back({ {"name", name}, {"value", value} });
            break;
        }
        case TK_INT64:
        {
            int64_t value;
            data->get_int64_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_UINT8:
        {
            uint8_t value;
            data->get_uint8_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_UINT16:
        {
            uint16_t value;
            data->get_uint16_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_UINT32:
        {
            uint32_t value;
            data->get_uint32_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_UINT64:
        {
            uint64_t value;
            data->get_uint64_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_FLOAT32:
        {
            float value;
            data->get_float32_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_FLOAT64:
        {
            double value;
            data->get_float64_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_FLOAT128:
        {
            long double value;
            data->get_float128_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_CHAR8:
        {
            char value;
            data->get_char8_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_CHAR16:
        {
            wchar_t value;
            data->get_char16_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_STRING8:
        {
            std::string value;
            data->get_string_value(value, id);
            j.push_back(value);
            break;
        }
        case TK_STRING16:
        {
            std::wstring value;
            data->get_wstring_value(value, id);
            j.push_back(value);
            break;
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
            break;
    }
}

void DynamicDataHelper::print_basic_element(
        DynamicData::_ref_type data,
        MemberId id,
        TypeKind kind,
        std::ostream& output)
{
    switch (kind)
    {
        case TK_NONE:
        {
            output << "<type not defined!>";
            break;
        }
        case TK_BOOLEAN:
        {
            bool value;
            data->get_boolean_value(value, id);
            output << (value ? "true" : "false");
            break;
        }
        case TK_BYTE:
        {
            eprosima::fastrtps::rtps::octet value;
            data->get_byte_value(value, id);
            output << static_cast<uint16_t>(value);
            break;
        }
        case TK_INT16:
        {
            int16_t value;
            data->get_int16_value(value, id);
            output << value;
            break;
        }
        case TK_INT32:
        case TK_ENUM: // TODO: print descriptor name instead of raw value
        {
            int32_t value;
            data->get_int32_value(value, id);
            output << value;
            break;
        }
        case TK_INT64:
        {
            int64_t value;
            data->get_int64_value(value, id);
            output << value;
            break;
        }
        case TK_UINT16:
        {
            uint16_t value;
            data->get_uint16_value(value, id);
            output << value;
            break;
        }
        case TK_UINT32:
        {
            uint32_t value;
            data->get_uint32_value(value, id);
            output << value;
            break;
        }
        case TK_UINT64:
        {
            uint64_t value;
            data->get_uint64_value(value, id);
            output << value;
            break;
        }
        case TK_FLOAT32:
        {
            float value;
            data->get_float32_value(value, id);
            output << value;
            break;
        }
        case TK_FLOAT64:
        {
            double value;
            data->get_float64_value(value, id);
            output << value;
            break;
        }
        case TK_FLOAT128:
        {
            long double value;
            data->get_float128_value(value, id);
            output << value;
            break;
        }
        case TK_CHAR8:
        {
            char value;
            data->get_char8_value(value, id);
            std::cout << value;
            break;
        }
        case TK_CHAR16:
        {
            wchar_t value;
            data->get_char16_value(value, id);
            std::cout << value;
            break;
        }
        case TK_STRING8:
        {
            std::string value;
            data->get_string_value(value, id);
            output << value;
            break;
        }
        case TK_STRING16:
        {
            std::wstring value;
            data->get_wstring_value(value, id);
            std::wcout << value;    // TODO: review
            break;
        }
        // case TK_BITMASK:
        // {
        //     size_t size = data->type()->get_size();
        //     switch (size)
        //     {
        //         case 1: output << std::to_string(data->get_uint8_value(id)); break;
        //         case 2: output << std::to_string(data->get_uint16_value(id)); break;
        //         case 3: output << std::to_string(data->get_uint32_value(id)); break;
        //         case 4: output << std::to_string(data->get_uint64_value(id)); break;
        //     }
        //     break;
        // }
        default:
            break;
    }
}

void DynamicDataHelper::print_collection(
        DynamicData::_ref_type data,
        const std::string& tabs)
{
    TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
    data->type()->get_descriptor(descriptor);
    switch (descriptor->element_type()->get_kind())
    {
        case TK_NONE:
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
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
            print_basic_collection(data);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_collection(data, tabs);
            break;
        }
        default:
            break;

    }
}

void DynamicDataHelper::print_collection_json(
        DynamicData::_ref_type data,
        const std::string& member_name,
        nlohmann::json& j)
{
    TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
    data->type()->get_descriptor(descriptor);
    switch (descriptor->element_type()->get_kind())
    {
        case TK_NONE:
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
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
            print_basic_collection_json(data, member_name, j);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_collection_json(data, member_name, j);
            break;
        }
        default:
            break;

    }
}

void DynamicDataHelper::print_collection(
        DynamicData::_ref_type data,
        std::ostream& output,
        const std::string& tabs)
{
    TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
    data->type()->get_descriptor(descriptor);
    switch (descriptor->element_type()->get_kind())
    {
        case TK_NONE:
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
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
            print_basic_collection(data, output);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_collection(data, output, tabs);
            break;
        }
        default:
            break;

    }
}

void DynamicDataHelper::fill_array_positions(
        const std::vector<uint32_t>& bounds,
        std::vector<std::vector<uint32_t>>& positions)
{
    uint32_t total_size = 1;
    for (size_t i = 0; i < bounds.size(); ++i)
    {
        total_size *= bounds[i];
    }

    for (uint32_t idx = 0; idx < total_size; ++idx)
    {
        positions.push_back({});
        get_index_position(idx, bounds, positions[idx]);
    }
}

void DynamicDataHelper::get_index_position(
        uint32_t index,
        const std::vector<uint32_t>& bounds,
        std::vector<uint32_t>& position)
{
    position.resize(bounds.size());
    if (bounds.size() > 0)
    {
        aux_index_position(index, static_cast<uint32_t>(bounds.size() - 1), bounds, position);
    }
}

void DynamicDataHelper::aux_index_position(
        uint32_t index,
        uint32_t inner_index,
        const std::vector<uint32_t>& bounds,
        std::vector<uint32_t>& position)
{
    uint32_t remainder = index % bounds[inner_index];
    position[inner_index] = remainder;
    if (inner_index > 0)
    {
        aux_index_position(index / bounds[inner_index], inner_index - 1, bounds, position);
    }
}

void DynamicDataHelper::print_basic_collection(
        DynamicData::_ref_type data)
{
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        auto count = data->get_item_count();
        std::cout << "[";
        for (uint32_t i = 0; i < count; ++i)
        {
            print_basic_element(data, i, descriptor->element_type()->get_kind());
            std::cout << (i == count - 1 ? "]" : ", ");
        }
        if (count == 0)
        {
            std::cout << "]";
        }
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};

        data->type()->get_descriptor(descriptor);
        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        // TODO: print multidimensional arrays differently? using ; separator for example
        std::cout << "[";
        for (size_t i = 0; i < positions.size(); ++i) // TODO: maybe enough using bounds size
        {
            // print_basic_element(data, data->get_array_index(positions[i]), descriptor->element_type()->get_kind());
            print_basic_element(data, i, descriptor->element_type()->get_kind()); // TODO: see if this would be enough, if not add get_array_index to helper
            std::cout << (i == positions.size() - 1 ? "]" : ", ");
        }
    }
    std::cout << std::endl;
}

void DynamicDataHelper::print_basic_collection_json(
        DynamicData::_ref_type data,
        const std::string& member_name,
        nlohmann::json& j)
{
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        auto count = data->get_item_count();
        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t i = 0; i < count; ++i)
        {
            print_basic_element_json(data, i, descriptor->element_type()->get_kind(), j_array);
        }
        j[member_name] = j_array;
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};

        data->type()->get_descriptor(descriptor);
        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        nlohmann::json j_array = nlohmann::json::array();
        for (size_t i = 0; i < positions.size(); ++i) // TODO: maybe enough using bounds size
        {
            print_basic_element_json(data, i, descriptor->element_type()->get_kind(), j_array);
        }
        j[member_name] = j_array;
    }
}

void DynamicDataHelper::print_basic_collection(
        DynamicData::_ref_type data,
        std::ostream& output)
{
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        auto count = data->get_item_count();
        output << "[";
        for (uint32_t i = 0; i < count; ++i)
        {
            print_basic_element(data, i, descriptor->element_type()->get_kind(), output);
            output << (i == count - 1 ? "]" : ", ");
        }
        if (count == 0)
        {
            output << "]";
        }
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        output << "[";
        for (size_t i = 0; i < positions.size(); ++i)
        {
            // print_basic_element(data, data->get_array_index(positions[i]),
            //         descriptor->element_type()->get_kind(), output);
            print_basic_element(data, i,
                    descriptor->element_type()->get_kind(), output);
            output << (i == positions.size() - 1 ? "]" : ", ");
        }
    }
    output << "\n";
}

void DynamicDataHelper::print_complex_collection(
        DynamicData::_ref_type data,
        const std::string& tabs)
{
    // TODO: test and fix empty collection printing
    std::cout << std::endl;
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        auto count = data->get_item_count();

        for (uint32_t i = 0; i < count; ++i)
        {
            std::cout << tabs << "[" << i << "] = ";
            print_complex_element(data, i, tabs);
            std::cout << std::endl;
        }

        if (count == 0)
        {
            std::cout << "[]";
        }
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        for (size_t i = 0; i < positions.size(); ++i)
        {
            std::cout << tabs << "[" << i << "] = ";
            // print_complex_element(data, data->get_array_index(positions[i]), tabs);
            print_complex_element(data, i, tabs);
            std::cout << std::endl;
        }
    }
}

void DynamicDataHelper::print_complex_collection_json(
        DynamicData::_ref_type data,
        const std::string& member_name,
        nlohmann::json& j)
{
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        auto count = data->get_item_count();

        nlohmann::json j_array = nlohmann::json::array();
        for (uint32_t i = 0; i < count; ++i)
        {
            nlohmann::json aux;
            print_complex_element_json(data, i, std::to_string(i), aux);
            j_array.push_back(aux);
        }
        j[member_name] = j_array;
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        nlohmann::json j_array = nlohmann::json::array();
        for (size_t i = 0; i < positions.size(); ++i)
        {
            nlohmann::json aux;
            print_complex_element_json(data, i, std::to_string(i), aux);
            j_array.push_back(aux);
        }
        j[member_name] = j_array;
    }
}

void DynamicDataHelper::print_complex_collection(
        DynamicData::_ref_type data,
        std::ostream& output,
        const std::string& tabs)
{
    output << "\n";
    if (data->type()->get_kind() == TK_SEQUENCE)
    {
        auto count = data->get_item_count();

        for (uint32_t i = 0; i < count; ++i)
        {
            output << tabs;
            output << "[";
            output << std::to_string(i);
            output << "] = ";
            print_complex_element(data, i, output, tabs);
            output << "\n";
        }

        if (count == 0)
        {
            output << "[]";
        }
    }
    else
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        data->type()->get_descriptor(descriptor);

        const BoundSeq& bounds = descriptor->bound();

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        for (size_t i = 0; i < positions.size(); ++i)
        {
            output << tabs;
            output << "[";
            output << std::to_string(i);
            output << "] = ";
            // print_complex_element(data, data->get_array_index(positions[i]), output, tabs);
            print_complex_element(data, i, output, tabs);
            output << "\n";
        }
    }
}

void DynamicDataHelper::print_complex_element(
        DynamicData::_ref_type data,
        MemberId id,
        const std::string& tabs)
{
    DynamicData::_ref_type st_data = data->loan_value(id);
    switch (st_data->type()->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            // TODO: differentiate between struct and bitset??
            std::cout << "<struct/bitset>" << std::endl;
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member(st_data, it.second, tabs + "\t");
            }
            break;
        }
        case TK_UNION:
        {
            // std::cout << "<union>" << std::endl;
            // traits<DynamicTypeMember>::ref_type member;
            // st_data->type()->get_member(member, st_data->union_id_);
            // print_member(st_data, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            print_collection(st_data, tabs + "\t");
            break;
        }
        case TK_MAP:
        {
            std::cout << "<map>" << std::endl;
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            size_t size = st_data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                size_t index = i * 2;
                MemberId member_id = st_data->get_member_id_at_index(static_cast<uint32_t>(index));
                std::cout << "Key: ";
                print_member(st_data, members[member_id], tabs + "\t");
                member_id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
                std::cout << "Value: ";
                print_member(st_data, members[member_id], tabs + "\t");
            }
            break;
        }
        default:
            break;
    }
    data->return_loaned_value(st_data);
}

void DynamicDataHelper::print_complex_element_json(
        DynamicData::_ref_type data,
        MemberId id,
        const std::string& member_name,
        nlohmann::json& j)
{
    DynamicData::_ref_type st_data = data->loan_value(id);
    switch (st_data->type()->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            nlohmann::json j_struct;
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member_json(st_data, it.second, j_struct);
            }
            j[member_name] = j_struct;
            break;
        }
        case TK_UNION:
        {
            // std::cout << "<union>" << std::endl;
            // traits<DynamicTypeMember>::ref_type member;
            // st_data->type()->get_member(member, st_data->union_id_);
            // print_member(st_data, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            print_collection_json(st_data, member_name, j);
            break;
        }
        case TK_MAP:
        {
            // TODO
            // std::cout << "<map>" << std::endl;
            // DynamicTypeMembersById members;
            // st_data->type()->get_all_members(members);
            // size_t size = st_data->get_item_count();
            // for (size_t i = 0; i < size; ++i)
            // {
            //     size_t index = i * 2;
            //     MemberId member_id = st_data->get_member_id_at_index(static_cast<uint32_t>(index));
            //     std::cout << "Key: ";
            //     print_member(st_data, members[member_id], tabs + "\t");
            //     member_id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
            //     std::cout << "Value: ";
            //     print_member(st_data, members[member_id], tabs + "\t");
            // }
            break;
        }
        default:
            break;
    }
    data->return_loaned_value(st_data);
}

void DynamicDataHelper::print_complex_element(
        DynamicData::_ref_type data,
        MemberId id,
        std::ostream& output,
        const std::string& tabs)
{
    DynamicData::_ref_type st_data = data->loan_value(id);
    switch (st_data->type()->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            output << "<struct/bitset>\n";
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            for (auto it : members)
            {
                print_member(st_data, output, it.second, tabs + "\t");
            }
            break;
        }
        case TK_UNION:
        {
            // output << "<union>\n";
            // traits<DynamicTypeMember>::ref_type member;
            // st_data->type()->get_member(member, st_data->union_id_);
            // print_member(st_data, output, &member, tabs + "\t");
            break;
        }
        case TK_SEQUENCE:
        case TK_ARRAY:
        {
            print_collection(st_data, output, tabs + "\t");
            break;
        }
        case TK_MAP:
        {
            output << "<map>\n";
            DynamicTypeMembersById members;
            st_data->type()->get_all_members(members);
            size_t size = st_data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                size_t index = i * 2;
                MemberId member_id = st_data->get_member_id_at_index(static_cast<uint32_t>(index));
                output << "Key: ";
                print_member(st_data, output, members[member_id], tabs + "\t");
                member_id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
                output << "Value: ";
                print_member(st_data, output, members[member_id], tabs + "\t");
            }
            break;
        }
        default:
            break;
    }
    data->return_loaned_value(st_data);
}

void DynamicDataHelper::print_element(
        DynamicData::_ref_type data,
        MemberId id,
        DynamicType::_ref_type type,
        const std::string& tabs)
{
    switch (type->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_element(data, id, tabs);
            break;
        }
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
            print_basic_element(data, id, type->get_kind());
            break;
        }

        default:
            break;
    }
}

void DynamicDataHelper::print_element_json(
        DynamicData::_ref_type data,
        MemberId id,
        DynamicType::_ref_type type,
        const std::string& member_name,
        nlohmann::json& j)
{
    switch (type->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_element_json(data, id, member_name, j);
            break;
        }
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
            print_basic_element_json(data, id, type->get_kind(), member_name, j);
            break;
        }

        default:
            break;
    }
}

void DynamicDataHelper::print_element(
        DynamicData::_ref_type data,
        MemberId id,
        DynamicType::_ref_type type,
        std::ostream& output,
        const std::string& tabs)
{
    switch (type->get_kind())
    {
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        {
            print_complex_element(data, id, output, tabs);
            break;
        }
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
            print_basic_element(data, id, type->get_kind(), output);
            break;
        }

        default:
            break;
    }
}
