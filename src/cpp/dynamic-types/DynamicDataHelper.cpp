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

#include <fastrtps/types/v1_3/MemberDescriptor.h>
#include <fastrtps/types/DynamicDataHelper.hpp>

// TODO: fix when the v1_1 files are included
using namespace eprosima::fastrtps::types;
using namespace eprosima::fastrtps::types::v1_3;

void DynamicDataHelper::print(
        const DynamicData_ptr& data)
{
    print(data.get());
}

void DynamicDataHelper::print(
        const DynamicData* data)
{
    if (nullptr != data)
    {
        switch (data->get_kind())
        {
            case TypeKind::TK_STRUCTURE:
            {
                for (auto it : data->get_type()->get_all_members_by_id())
                {
                    print_member(const_cast<DynamicData*>(data), it.second->get_descriptor());
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

void DynamicDataHelper::print_basic_element(
        DynamicData* data,
        MemberId id,
        TypeKind kind)
{
    switch (kind)
    {
        case TypeKind::TK_NONE:
        {
            std::cout << "<type not defined!>";
            break;
        }
        case TypeKind::TK_BOOLEAN:
        {
            std::cout << (data->get_bool_value(id) ? "true" : "false");
            break;
        }
        case TypeKind::TK_BYTE:
        {
            std::cout << static_cast<uint32_t>(data->get_byte_value(id));
            break;
        }
        case TypeKind::TK_INT16:
        {
            std::cout << data->get_int16_value(id);
            break;
        }
        case TypeKind::TK_INT32:
        {
            std::cout << data->get_int32_value(id);
            break;
        }
        case TypeKind::TK_INT64:
        {
            std::cout << data->get_int64_value(id);
            break;
        }
        case TypeKind::TK_UINT16:
        {
            std::cout << data->get_uint16_value(id);
            break;
        }
        case TypeKind::TK_UINT32:
        {
            std::cout << data->get_uint32_value(id);
            break;
        }
        case TypeKind::TK_UINT64:
        {
            std::cout << data->get_uint64_value(id);
            break;
        }
        case TypeKind::TK_FLOAT32:
        {
            std::cout << data->get_float32_value(id);
            break;
        }
        case TypeKind::TK_FLOAT64:
        {
            std::cout << data->get_float64_value(id);
            break;
        }
        case TypeKind::TK_FLOAT128:
        {
            std::cout << data->get_float128_value(id);
            break;
        }
        case TypeKind::TK_CHAR8:
        {
            std::cout << data->get_char8_value(id);
            break;
        }
        case TypeKind::TK_CHAR16:
        {
            std::cout << data->get_char16_value(id);
            break;
        }
        case TypeKind::TK_STRING8:
        {
            std::cout << data->get_string_value(id);
            break;
        }
        case TypeKind::TK_STRING16:
        {
            std::wcout << data->get_wstring_value(id);
            break;
        }
        case TypeKind::TK_BITMASK:
        {
            size_t size = data->get_type()->get_size();
            switch (size)
            {
                case 1: std::cout << data->get_uint8_value(id); break;
                case 2: std::cout << data->get_uint16_value(id); break;
                case 3: std::cout << data->get_uint32_value(id); break;
                case 4: std::cout << data->get_uint64_value(id); break;
            }
            break;
        }
        case TypeKind::TK_ENUM:
        {
            std::cout << data->get_uint32_value(id);
            break;
        }
        default:
            break;
    }
}

void DynamicDataHelper::print_collection(
        DynamicData* data,
        const std::string& tabs)
{
    switch (data->get_type()->get_element_type()->get_kind())
    {
        case TypeKind::TK_NONE:
        case TypeKind::TK_BOOLEAN:
        case TypeKind::TK_BYTE:
        case TypeKind::TK_INT16:
        case TypeKind::TK_INT32:
        case TypeKind::TK_INT64:
        case TypeKind::TK_UINT16:
        case TypeKind::TK_UINT32:
        case TypeKind::TK_UINT64:
        case TypeKind::TK_FLOAT32:
        case TypeKind::TK_FLOAT64:
        case TypeKind::TK_FLOAT128:
        case TypeKind::TK_CHAR8:
        case TypeKind::TK_CHAR16:
        case TypeKind::TK_STRING8:
        case TypeKind::TK_STRING16:
        case TypeKind::TK_ENUM:
        case TypeKind::TK_BITMASK:
        {
            print_basic_collection(data);
            break;
        }
        case TypeKind::TK_STRUCTURE:
        case TypeKind::TK_BITSET:
        case TypeKind::TK_UNION:
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_ARRAY:
        case TypeKind::TK_MAP:
        {
            print_complex_collection(data, tabs);
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
        DynamicData* data)
{
    if (data->get_type()->get_kind() == TypeKind::TK_SEQUENCE)
    {
        auto count = data->get_item_count();
        std::cout << "[";
        for (MemberId i{0}; i < count; ++i)
        {
            print_basic_element(data, i, data->get_type()->get_element_type()->get_kind());
            std::cout << (i == count - 1 ? "]" : ", ");
        }
        if (count == 0)
        {
            std::cout << "]";
        }
    }
    else
    {
        const std::vector<uint32_t>& bounds = data->get_type()->get_descriptor().bound_;

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        std::cout << "[";
        for (size_t i = 0; i < positions.size(); ++i)
        {
            print_basic_element(data, data->get_array_index(positions[i]), data->get_type()->get_element_type()->get_kind());
            std::cout << (i == positions.size() - 1 ? "]" : ", ");
        }
    }
    std::cout << std::endl;
}

void DynamicDataHelper::print_complex_collection(
        DynamicData* data,
        const std::string& tabs)
{
    std::cout << std::endl;
    if (data->get_type()->get_kind() == TypeKind::TK_SEQUENCE)
    {
        auto count = data->get_item_count();

        for (MemberId i{0}; i < count; ++i)
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
        const std::vector<uint32_t>& bounds = data->get_type()->get_descriptor().bound_;

        std::vector<std::vector<uint32_t>> positions;
        fill_array_positions(bounds, positions);

        for (size_t i = 0; i < positions.size(); ++i)
        {
            std::cout << tabs << "[" << i << "] = ";
            print_complex_element(data, data->get_array_index(positions[i]), tabs);
            std::cout << std::endl;
        }
    }
}

void DynamicDataHelper::print_complex_element(
        DynamicData* data,
        MemberId id,
        const std::string& tabs)
{
    using namespace v1_3;

    DynamicData* st_data = data->loan_value(id);
    const TypeDescriptor& desc = st_data->get_type()->get_descriptor();
    switch (desc.get_kind())
    {
        case TypeKind::TK_STRUCTURE:
        case TypeKind::TK_BITSET:
        {
            std::cout << "<struct/bitset>" << std::endl;
            for (auto it : data->get_type()->get_all_members_by_id())
            {
                print_member(st_data, it.second->get_descriptor(), tabs + "\t");
            }
            break;
        }
        case TypeKind::TK_UNION:
        {
            std::cout << "<union>" << std::endl;
            MemberDescriptor member;
            st_data->get_type()->get_member(member, st_data->union_id_);
            print_member(st_data, member, tabs + "\t");
            break;
        }
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_ARRAY:
        {
            print_collection(st_data, tabs + "\t");
            break;
        }
        case TypeKind::TK_MAP:
        {
            std::cout << "<map>" << std::endl;
            auto members = st_data->get_type()->get_all_members_by_id();
            size_t size = st_data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                size_t index = i * 2;
                MemberId member_id = st_data->get_member_id_at_index(static_cast<uint32_t>(index));
                std::cout << "Key: ";
                print_member(st_data, members[member_id]->get_descriptor(), tabs + "\t");
                member_id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
                std::cout << "Value: ";
                print_member(st_data, members[member_id]->get_descriptor(), tabs + "\t");
            }
            break;
        }
        default:
            break;
    }
    data->return_loaned_value(st_data);
}

void DynamicDataHelper::print_member(
        DynamicData* data,
        const MemberDescriptor& desc,
        const std::string& tabs)
{
    std::cout << tabs << desc.get_name() << ": ";
    switch (desc.get_kind())
    {
        case TypeKind::TK_NONE:
        case TypeKind::TK_BOOLEAN:
        case TypeKind::TK_BYTE:
        case TypeKind::TK_INT16:
        case TypeKind::TK_INT32:
        case TypeKind::TK_INT64:
        case TypeKind::TK_UINT16:
        case TypeKind::TK_UINT32:
        case TypeKind::TK_UINT64:
        case TypeKind::TK_FLOAT32:
        case TypeKind::TK_FLOAT64:
        case TypeKind::TK_FLOAT128:
        case TypeKind::TK_CHAR8:
        case TypeKind::TK_CHAR16:
        case TypeKind::TK_STRING8:
        case TypeKind::TK_STRING16:
        case TypeKind::TK_ENUM:
        case TypeKind::TK_BITMASK:
        {
            print_basic_element(data, desc.get_id(), desc.get_kind());
            std::cout << std::endl;
            break;
        }
        case TypeKind::TK_STRUCTURE:
        case TypeKind::TK_BITSET:
        {
            DynamicData* st_data = data->loan_value(desc.get_id());
            std::cout << "<struct/bitset>" << std::endl;
            for (auto it : data->get_type()->get_all_members_by_id())
            {
                print_member(st_data, it.second->get_descriptor(), tabs + "\t");
            }
            data->return_loaned_value(st_data);
            break;
        }
        case TypeKind::TK_UNION:
        {
            std::cout << "<union>" << std::endl;
            DynamicData* st_data = data->loan_value(desc.get_id());
            MemberDescriptor member;
            desc.get_type()->get_member(member, data->union_id_);
            print_member(st_data, member, tabs + "\t");
            break;
        }
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_ARRAY:
        {
            DynamicData* st_data = data->loan_value(desc.get_id());
            print_collection(st_data, tabs + "\t");
            data->return_loaned_value(st_data);
            break;
        }
        case TypeKind::TK_MAP:
        {
            std::cout << "<map>" << std::endl;
            DynamicData* st_data = data->loan_value(desc.get_id());
            auto members = desc.get_type()->get_all_members_by_id();
            size_t size = data->get_item_count();
            for (size_t i = 0; i < size; ++i)
            {
                size_t index = i * 2;
                MemberId id = data->get_member_id_at_index(static_cast<uint32_t>(index));
                std::cout << "Key: ";
                print_member(st_data, members[id]->get_descriptor(), tabs + "\t");
                id = data->get_member_id_at_index(static_cast<uint32_t>(index + 1));
                std::cout << "Value: ";
                print_member(st_data, members[id]->get_descriptor(), tabs + "\t");
            }
            data->return_loaned_value(st_data);
            break;
        }
        default:
            break;
    }
}
