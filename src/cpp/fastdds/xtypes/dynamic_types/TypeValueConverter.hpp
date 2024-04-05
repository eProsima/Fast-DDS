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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEVALUECONVERTER_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEVALUECONVERTER_HPP

#include <string>

#include "DynamicTypeMemberImpl.hpp"
#include "TypeForKind.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

const char* const CONST_TRUE = "true";
const char* const CONST_FALSE = "false";

namespace detail {

struct converter
{
    const std::string& x;
    template<typename T> operator T()
    {
        return 0;
    }

};


template <> inline converter::operator TypeForKind<TK_UINT8>()
{
    return static_cast<uint8_t>(std::stoul(x));
}

template <> inline converter::operator TypeForKind<TK_INT16>()
{
    return static_cast<int16_t>(std::stol(x));
}

template <> inline converter::operator TypeForKind<TK_UINT16>()
{
    return static_cast<uint16_t>(std::stoul(x));
}

template <> inline converter::operator TypeForKind<TK_INT32>()
{
    return static_cast<int32_t>(std::stol(x));
}

template <> inline converter::operator TypeForKind<TK_UINT32>()
{
    return static_cast<uint32_t>(std::stoul(x));
}

template <> inline converter::operator TypeForKind<TK_INT64>()
{
    return static_cast<int64_t>(std::stoll(x));
}

template <> inline converter::operator TypeForKind<TK_UINT64>()
{
    return static_cast<uint64_t>(std::stoull(x));
}

template <> inline converter::operator TypeForKind<TK_FLOAT32>()
{
    return std::stof(x);
}

template <> inline converter::operator TypeForKind<TK_FLOAT64>()
{
    return std::stod(x);
}

template <> inline converter::operator TypeForKind<TK_FLOAT128>()
{
    return std::stold(x);
}

template <> inline converter::operator TypeForKind<TK_BOOLEAN>()
{
    if (0 == x.compare(CONST_TRUE))
    {
        return true;
    }
    if (0 == x.compare(CONST_FALSE))
    {
        return false;
    }

    return 0 < stoul(x) ? true : false;
}

} // namespace detail

class TypeValueConverter final
{
public:

    static detail::converter sto(
            const std::string& str)
    {
        return {str};
    }

    static bool is_string_consistent(
            TypeKind kind,
            const std::vector<traits<DynamicTypeMemberImpl>::ref_type>& members,
            const std::string& str)
    {

        if (str.empty())
        {
            return false;
        }
        bool ret_value = true;

        try
        {
            switch (kind)
            {
                case TK_INT32:
                {
                    TypeForKind<TK_INT32> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_UINT32:
                {
                    TypeForKind<TK_UINT32> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_INT8:
                {
                    TypeForKind<TK_INT8> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_INT16:
                {
                    TypeForKind<TK_INT16> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_UINT16:
                {
                    TypeForKind<TK_UINT16> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_INT64:
                {
                    TypeForKind<TK_INT64> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_UINT64:
                {
                    TypeForKind<TK_UINT64> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_FLOAT32:
                {
                    TypeForKind<TK_FLOAT32> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_FLOAT64:
                {
                    TypeForKind<TK_FLOAT64> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_FLOAT128:
                {
                    TypeForKind<TK_FLOAT128> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_CHAR8:
                case TK_CHAR16:
                {
                    if (str.size() != 1)
                    {
                        ret_value = false;
                    }
                }
                break;
                case TK_BOOLEAN:
                {
                    TypeForKind<TK_BOOLEAN> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_BYTE:
                {
                    TypeForKind<TK_BYTE> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_UINT8:
                {
                    TypeForKind<TK_UINT8> value = sto(str);
                    static_cast<void>(value);
                }
                break;
                case TK_STRING8:
                case TK_STRING16:
                    break;
                case TK_ENUM:
                    ret_value = false;
                    for (auto& member : members)
                    {
                        if (0 == str.compare(member->get_name().to_string()))
                        {
                            ret_value = true;
                            break;
                        }
                    }
                    break;
                default:
                    ret_value = false;
                    break;
            }
        }
        catch (...)
        {
            ret_value = false;
        }

        return ret_value;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEVALUECONVERTER_HPP
