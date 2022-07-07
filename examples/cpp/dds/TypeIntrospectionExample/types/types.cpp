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

#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "types.hpp"

using namespace eprosima::fastrtps::types;

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
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::ARRAY, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::STRUCT)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::STRUCT, GeneratorKind::GEN>());
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
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::PLAIN, GeneratorKind::XML>());
        }
    }
    else if (data_kind == DataTypeKind::KEY)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::KEY, GeneratorKind::GEN>());
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
    else if (data_kind == DataTypeKind::COMPLEX)
    {
        if (gen_kind == GeneratorKind::GEN)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX, GeneratorKind::GEN>());
        }
        else if (gen_kind == GeneratorKind::CODE)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX, GeneratorKind::CODE>());
        }
        else if (gen_kind == GeneratorKind::XML)
        {
            return std::unique_ptr<IDataType>(new DataType<DataTypeKind::COMPLEX, GeneratorKind::XML>());
        }
    }

    throw std::invalid_argument("Invalid data type kind");
}

bool is_basic_type(
        const eprosima::fastrtps::types::TypeKind kind)
{
    switch(kind)
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

eprosima::fastrtps::types::DynamicType_ptr internal_array_type(
        const eprosima::fastrtps::types::DynamicType_ptr& type)
{
    return type->get_descriptor()->get_element_type();
}

std::ostream& operator <<(
        std::ostream& output,
        const GeneratorKind& gen)
{
    switch (gen)
    {
        case GeneratorKind::GEN:
            output << "Fast DDS Gen";
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
        const eprosima::fastrtps::types::DynamicData_ptr& data)
{
    // This is shitty, but it is what we have
    DynamicDataHelper::print(data);
    return output;
}

std::ostream& operator <<(
        std::ostream& output,
        const DynamicType_ptr& type)
{
    eprosima::fastrtps::types::TypeKind kind = type->get_kind();

    // Add key annotation
    std::string annotations = "";
    if (type->key_annotation())
    {
        annotations = " @key";
    }

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
            output << type->get_name();
            break;

        case TK_ARRAY:
            output << "[ " << internal_array_type(type) << " (" << type->get_total_bounds() << ") " << annotations << " ]";
            break;

        case TK_SEQUENCE:
            output << "[ " << internal_array_type(type) << annotations << " ]";
            break;

        case TK_STRUCTURE:
        {
            std::map<std::string, DynamicTypeMember*> members;
            type->get_all_members_by_name(members);

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

                output << it.first << ": " << it.second->get_descriptor()->get_type();
            }
            output << annotations << " } ";
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
    switch(kind)
    {
        case TK_NONE:
        {
            std::cout << "NONE";
            break;
        }
        case TK_BOOLEAN:
        {
            std::cout << "BOOLEAN";
            break;
        }
        case TK_BYTE:
        {
            std::cout << "BYTE";
            break;
        }
        case TK_INT16:
        {
            std::cout << "INT16";
            break;
        }
        case TK_INT32:
        {
            std::cout << "INT32";
            break;
        }
        case TK_INT64:
        {
            std::cout << "INT64";
            break;
        }
        case TK_UINT16:
        {
            std::cout << "UINT16";
            break;
        }
        case TK_UINT32:
        {
            std::cout << "UINT32";
            break;
        }
        case TK_UINT64:
        {
            std::cout << "UINT64";
            break;
        }
        case TK_FLOAT32:
        {
            std::cout << "FLOAT32";
            break;
        }
        case TK_FLOAT64:
        {
            std::cout << "FLOAT64";
            break;
        }
        case TK_FLOAT128:
        {
            std::cout << "FLOAT128";
            break;
        }
        case TK_CHAR8:
        {
            std::cout << "CHAR8";
            break;
        }
        case TK_CHAR16:
        {
            std::cout << "CHAR16";
            break;
        }
        case TK_STRING8:
        {
            std::cout << "STRING8";
            break;
        }
        case TK_STRING16:
        {
            std::cout << "STRING16";
            break;
        }
        case TK_BITMASK:
        {
            std::cout << "BITMASK";
            break;
        }
        case TK_ENUM:
        {
            std::cout << "ENUM";
            break;
        }
        case TK_STRUCTURE:
        {
            std::cout << "STRUCTURE";
            break;
        }
        case TK_BITSET:
        {
            std::cout << "BITSET";
            break;
        }
        case TK_UNION:
        {
            std::cout << "UNION";
            break;
        }
        case TK_SEQUENCE:
        {
            std::cout << "SEQUENCE";
            break;
        }
        case TK_ARRAY:
        {
            std::cout << "ARRAY";
            break;
        }
        case TK_MAP:
        {
            std::cout << "MAP";
            break;
        }
        default:
        {
            std::cout << "UNKNOWN";
            break;
        }
    }

    return output;
}

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastrtps::types::DynamicTypeMember* member)
{
    auto kind = member->get_descriptor()->get_kind();

    switch(kind)
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
            output << "< " << kind << " >";
            break;

        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_ARRAY:
            output << "< " << kind << " [ " << member->get_descriptor()->get_type()->get_descriptor()->get_element_type() << " ] >";
            break;

        case TK_MAP:
            output << "< " << kind << " >";
            break;

        default:
            throw std::runtime_error("Unsupported Dynamic Types kind");
            break;
    }

    return output;
}
