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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEFORKIND_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEFORKIND_HPP

#include <type_traits>

#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//{{{ Type traits

template <TypeKind TK>
struct TypeForKind_;

template <>
struct TypeForKind_<TK_INT8>
{
    using type = int8_t;
    using sequence_type = Int8Seq;
};

template <>
struct TypeForKind_<TK_UINT8>
{
    using type = uint8_t;
    using sequence_type = UInt8Seq;
};

template <>
struct TypeForKind_<TK_INT16>
{
    using type = int16_t;
    using sequence_type = Int16Seq;
};

template <>
struct TypeForKind_<TK_UINT16>
{
    using type = uint16_t;
    using sequence_type = UInt16Seq;
};

template <>
struct TypeForKind_<TK_INT32>
{
    using type = int32_t;
    using sequence_type = Int32Seq;
};

template <>
struct TypeForKind_<TK_UINT32>
{
    using type = uint32_t;
    using sequence_type = UInt32Seq;
};

template <>
struct TypeForKind_<TK_INT64>
{
    using type = int64_t;
    using sequence_type = Int64Seq;
};

template <>
struct TypeForKind_<TK_UINT64>
{
    using type = uint64_t;
    using sequence_type = UInt64Seq;
};

template <>
struct TypeForKind_<TK_FLOAT32>
{
    using type = float;
    using sequence_type = Float32Seq;
};

template <>
struct TypeForKind_<TK_FLOAT64>
{
    using type = double;
    using sequence_type = Float64Seq;
};

template <>
struct TypeForKind_<TK_FLOAT128>
{
    using type = long double;
    using sequence_type = Float128Seq;
};

template <>
struct TypeForKind_<TK_CHAR8>
{
    using type = char;
    using sequence_type = CharSeq;
};

template <>
struct TypeForKind_<TK_CHAR16>
{
    using type = wchar_t;
    using sequence_type = WcharSeq;
};

template <>
struct TypeForKind_<TK_BYTE>
{
    using type = eprosima::fastdds::rtps::octet;
    using sequence_type = ByteSeq;
};


template <>
struct TypeForKind_<TK_BOOLEAN>
{
    using type = bool;
    using sequence_type = BooleanSeq;
};

template <>
struct TypeForKind_<TK_STRING8>
{
    using type = std::string;
    using sequence_type = StringSeq;
};

template <>
struct TypeForKind_<TK_STRING16>
{
    using type = std::wstring;
    using sequence_type = WstringSeq;
};

template <TypeKind TK>
using TypeForKind = typename TypeForKind_<TK>::type;

template <TypeKind TK>
using SequenceTypeForKind = typename TypeForKind_<TK>::sequence_type;

//}}}

//{{{ Promotion traits

template <TypeKind TK, TypeKind PromotedTK>
struct TypePromotion : std::false_type {};

template <>
struct TypePromotion<TK_INT8, TK_INT8> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT8, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT16, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_INT32, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_INT32, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT32, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT32, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_INT64, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_INT64, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_UINT8> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_UINT16> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_UINT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT8, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_UINT16> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_UINT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT16, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_UINT32, TK_UINT32> : std::true_type {};

template <>
struct TypePromotion<TK_UINT32, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT32, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT32, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT32, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_UINT64, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_UINT64, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT32, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT32, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT32, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT64, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT64, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_FLOAT128, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_CHAR8> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_CHAR16> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR8, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_CHAR16> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_CHAR16, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_BYTE> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_INT8> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_UINT8> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_UINT16> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_UINT32> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_FLOAT128> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_CHAR8> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_CHAR16> : std::true_type {};

template <>
struct TypePromotion<TK_BYTE, TK_BOOLEAN> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_BOOLEAN> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_INT8> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_UINT8> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_INT16> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_UINT16> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_INT32> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_UINT32> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_INT64> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_UINT64> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_FLOAT32> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_FLOAT64> : std::true_type {};

template <>
struct TypePromotion<TK_BOOLEAN, TK_FLOAT128> : std::true_type {};

//}}}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_TYPEFORKIND_HPP
