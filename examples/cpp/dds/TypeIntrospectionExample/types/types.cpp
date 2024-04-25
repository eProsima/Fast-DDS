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
 * @file types.cpp
 *
 */

#include <memory>

#include "DynamicDataHelper.hpp"

#include "types.hpp"

using namespace eprosima::fastdds::dds;

std::unique_ptr<IDataType> data_type_factory(
        const DataTypeKind data_kind,
        const GeneratorKind gen_kind)
{
    if (data_kind == DataTypeKind::HELLO_WORLD)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::HELLO_WORLD, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::HELLO_WORLD, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::HELLO_WORLD, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::HELLO_WORLD, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::ARRAY)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::SEQUENCE)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SEQUENCE, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SEQUENCE, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SEQUENCE, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SEQUENCE, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::STRUCT)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::STRUCT, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::STRUCT, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::STRUCT, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::STRUCT, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::PLAIN)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::SIMPLELARGE)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SIMPLELARGE, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SIMPLELARGE, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SIMPLELARGE, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SIMPLELARGE, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::KEY)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::KEY, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::KEY, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::KEY, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::KEY, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::COMPLEX_ARRAY)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_ARRAY, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_ARRAY, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            // TODO: fail for unsupported
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_ARRAY, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_ARRAY, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::COMPLEX_SEQUENCE)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_SEQUENCE, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_SEQUENCE, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_SEQUENCE, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX_SEQUENCE, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::SUPER_COMPLEX)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SUPER_COMPLEX, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SUPER_COMPLEX, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SUPER_COMPLEX, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::SUPER_COMPLEX, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::DATA_TEST)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::DATA_TEST, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::GEN_DYN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::DATA_TEST, GeneratorKind::GEN_DYN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::DATA_TEST, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::DATA_TEST, GeneratorKind::XML>());
        }
    }

    throw std::invalid_argument("Invalid data type kind");
}

bool is_basic_type(
        const TypeKind kind)
{
    switch (kind)
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
        case TK_BITMASK:
        case TK_ENUM:
            return true;

        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
            return false;

        default:
            throw std::runtime_error("Unsupported Dynamic Types kind");
    }
}

DynamicType::_ref_type internal_array_type(
        const DynamicType::_ref_type& type)
{
    TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
    type->get_descriptor(descriptor);
    return descriptor->element_type();
}

std::ostream& operator <<(
        std::ostream& output,
        const GeneratorKind& gen)
{
    switch (gen)
    {
        case GeneratorKind::GEN:
            output << "Fast DDS Gen (non-dynamic data)";
            break;
        case GeneratorKind::GEN_DYN:
            output << "Fast DDS Gen (dynamic data)";
            break;
        case GeneratorKind::CODE:
            output << "Code";
            break;
        case GeneratorKind::XML:
            output << "XML";
            break;
        default:
            throw std::runtime_error("Unsupported Generator type");
    }

    return output;
}
std::ostream& operator <<(
        std::ostream& output,
        const DynamicData::_ref_type& data)
{
    // DynamicDataHelper::print(output, data);
    // DynamicDataHelper::print(data);
    DynamicDataHelper::print_json(data);
    return output;
}

// TODO: move also to utils with dynamic data printer??
std::ostream& operator <<(
        std::ostream& output,
        const DynamicType::_ref_type& type)
{
    TypeKind kind = type->get_kind();

    // Add key annotation
    // std::string annotations = "";
    // if (type->key_annotation())
    // {
    //     annotations = " @key";
    // }

    switch (kind)
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
        case TK_BITMASK:
        case TK_ENUM:
            // TODO: review, before it was not blank
            // output << type->get_name();
            output << kind;
            break;

        case TK_ARRAY:
            // output << "[ " << internal_array_type(type) << " (" << type->get_total_bounds() << ") " << annotations <<
            // output << "[ " << internal_array_type(type) << " (" << type->get_total_bounds() << ") " <<
            output << "[ " << internal_array_type(type) << " (" << ") " <<
                " ]";
            break;

        case TK_SEQUENCE:
            // output << "[ " << internal_array_type(type) << annotations << " ]";
            output << "[ " << internal_array_type(type) << " ]";
            break;

        case TK_STRUCTURE:
        {
            DynamicTypeMembersByName members;
            type->get_all_members_by_name(members); // TODO: maybe better get members by index so they preserve original order

            output << type->get_name() << " { ";

            bool first = true;  // avoid printing comma after the first member
            for (auto it : members)
            {
                if (!first)
                {
                    output << " ; ";
                }
                else
                {
                    first = false;
                }

                MemberDescriptor::_ref_type descriptor {traits<MemberDescriptor>::make_shared()};
                it.second->get_descriptor(descriptor);

                output << it.first << ": " << descriptor->type();
            }
            // output << annotations << " } ";
            output << " } ";
            break;
        }

        default:
            break;
    }

    return output;
}

std::ostream& operator <<(
        std::ostream& output,
        const TypeKind& kind)
{
    switch (kind)
    {
        case TK_NONE:
        {
            std::cout << "None";
            break;
        }
        case TK_BOOLEAN:
        {
            std::cout << "bool";
            break;
        }
        case TK_BYTE:
        {
            std::cout << "byte";
            break;
        }
        case TK_INT8:
        {
            std::cout << "int8";
            break;
        }
        case TK_INT16:
        {
            std::cout << "int16";
            break;
        }
        case TK_INT32:
        {
            std::cout << "int32";
            break;
        }
        case TK_INT64:
        {
            std::cout << "int64";
            break;
        }
        case TK_UINT8:
        {
            std::cout << "uint8";
            break;
        }
        case TK_UINT16:
        {
            std::cout << "uint16";
            break;
        }
        case TK_UINT32:
        {
            std::cout << "uint32";
            break;
        }
        case TK_UINT64:
        {
            std::cout << "uint64";
            break;
        }
        case TK_FLOAT32:
        {
            std::cout << "float32";
            break;
        }
        case TK_FLOAT64:
        {
            std::cout << "float64";
            break;
        }
        case TK_FLOAT128:
        {
            std::cout << "float128";
            break;
        }
        case TK_CHAR8:
        {
            std::cout << "char8";
            break;
        }
        case TK_CHAR16:
        {
            std::cout << "char16";
            break;
        }
        case TK_STRING8:
        {
            std::cout << "string8";
            break;
        }
        case TK_STRING16:
        {
            std::cout << "string16";
            break;
        }
        case TK_BITMASK:
        {
            std::cout << "bitmask";
            break;
        }
        case TK_ENUM:
        {
            std::cout << "enum";
            break;
        }
        case TK_STRUCTURE:
        {
            std::cout << "struct";
            break;
        }
        case TK_BITSET:
        {
            std::cout << "bitset";
            break;
        }
        case TK_UNION:
        {
            std::cout << "union";
            break;
        }
        case TK_SEQUENCE:
        {
            std::cout << "seq";
            break;
        }
        case TK_ARRAY:
        {
            std::cout << "array";
            break;
        }
        case TK_MAP:
        {
            std::cout << "map";
            break;
        }
        default:
        {
            std::cout << "Unknown";
            break;
        }
    }

    return output;
}

// std::ostream& operator <<(
//         std::ostream& output,
//         const eprosima::fastrtps::types::DynamicTypeMember* member)
// {
//     auto kind = member->get_descriptor()->get_kind();

//     switch (kind)
//     {
//         case TK_NONE:
//         case TK_BOOLEAN:
//         case TK_BYTE:
//         case TK_INT8:
//         case TK_INT16:
//         case TK_INT32:
//         case TK_INT64:
//         case TK_UINT8:
//         case TK_UINT16:
//         case TK_UINT32:
//         case TK_UINT64:
//         case TK_FLOAT32:
//         case TK_FLOAT64:
//         case TK_FLOAT128:
//         case TK_CHAR8:
//         case TK_CHAR16:
//         case TK_STRING8:
//         case TK_STRING16:
//         case TK_BITMASK:
//         case TK_ENUM:
//             output << "< " << kind << " >";
//             break;

//         case TK_STRUCTURE:
//         case TK_BITSET:
//         case TK_UNION:
//         case TK_SEQUENCE:
//         case TK_ARRAY:
//             output << "< " << kind << " [ " <<
//                 member->get_descriptor()->get_type()->get_descriptor()->get_element_type() << " ] >";
//             break;

//         case TK_MAP:
//             output << "< " << kind << " >";
//             break;

//         default:
//             throw std::runtime_error("Unsupported Dynamic Types kind");
//             break;
//     }

//     return output;
// }
