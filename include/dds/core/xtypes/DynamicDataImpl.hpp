/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_IMPL_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_IMPL_HPP_

#include <dds/core/xtypes/DynamicData.hpp>

#include <sstream>
#include <locale>
#include <codecvt>

namespace dds {
namespace core {
namespace xtypes {


inline std::string ReadableDynamicDataRef::to_string() const
{
    std::stringstream ss;
    for_each([&](const DynamicData::ReadableNode& node)
    {
        const std::string& type_name = node.data().type().name();
        ss << std::string(node.deep() * 4, ' ');
        if(node.has_parent())
        {
            ss << "["
                << (node.parent().type().is_aggregation_type()
                    ? node.from_member()->name()
                    : std::to_string(node.from_index()))
                << "] ";
        }
        switch(node.data().type().kind())
        {
            case TypeKind::CHAR_8_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<char>();
                break;
            case TypeKind::CHAR_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<char32_t>();
                break;
            case TypeKind::UINT_8_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint8_t>();
                break;
            case TypeKind::INT_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int16_t>();
                break;
            case TypeKind::UINT_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint16_t>();
                break;
            case TypeKind::INT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int32_t>();
                break;
            case TypeKind::UINT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint32_t>();
                break;
            case TypeKind::INT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int64_t>();
                break;
            case TypeKind::UINT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint64_t>();
                break;
            case TypeKind::FLOAT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<float>();
                break;
            case TypeKind::FLOAT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<double>();
                break;
            case TypeKind::FLOAT_128_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<long double>();
                break;
            case TypeKind::STRING_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<std::string>();
                break;
            case TypeKind::WSTRING_TYPE:
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                ss << "<" << type_name << ">  " << converter.to_bytes(node.data().value<std::wstring>());
                break;
            }
            case TypeKind::ARRAY_TYPE:
                ss << "<" << type_name << ">";
                break;
            case TypeKind::SEQUENCE_TYPE:
                ss << "<" << type_name << "[" << node.data().size() << "]>";
                break;
            case TypeKind::STRUCTURE_TYPE:
                ss << "Structure: <" << type_name << ">";
                break;
            default:
                ss << "Unsupported type: " << type_name;
        }
        ss << std::endl;
    });

    return ss.str();
}


} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_IMPL_HPP_
