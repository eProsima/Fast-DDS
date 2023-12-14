// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPES_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPES_HPP

#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/rtps/common/Types.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace eprosima {

namespace fastcdr {
class Cdr;
} // namespace fastcdr

namespace fastdds {
namespace dds {

//! A special value indicating an unlimited quantity
constexpr uint32_t BOUND_UNLIMITED = 0;

inline namespace xtypes_names {

const char* const ANNOTATION_KEY_ID = "key";
const char* const ANNOTATION_EPKEY_ID = "Key";
const char* const ANNOTATION_TOPIC_ID = "Topic";
const char* const ANNOTATION_EXTENSIBILITY_ID = "extensibility";
const char* const ANNOTATION_FINAL_ID = "final";
const char* const ANNOTATION_APPENDABLE_ID = "appendable";
const char* const ANNOTATION_MUTABLE_ID = "mutable";
const char* const ANNOTATION_NESTED_ID = "nested";
const char* const ANNOTATION_OPTIONAL_ID = "optional";
const char* const ANNOTATION_MUST_UNDERSTAND_ID = "must_understand";
const char* const ANNOTATION_NON_SERIALIZED_ID = "non_serialized";
const char* const ANNOTATION_BIT_BOUND_ID = "bit_bound";
const char* const ANNOTATION_DEFAULT_ID = "default";
const char* const ANNOTATION_DEFAULT_LITERAL_ID = "default_literal";
const char* const ANNOTATION_VALUE_ID = "value";
const char* const ANNOTATION_POSITION_ID = "position";
const char* const ANNOTATION_EXTERNAL_ID = "external";

const char* const EXTENSIBILITY_FINAL = "FINAL";
const char* const EXTENSIBILITY_APPENDABLE = "APPENDABLE";
const char* const EXTENSIBILITY_MUTABLE = "MUTABLE";

const char* const TKNAME_BOOLEAN = "bool";
const char* const TKNAME_INT16 = "int16_t";
const char* const TKNAME_UINT16 = "uint16_t";
const char* const TKNAME_INT32 = "int32_t";
const char* const TKNAME_UINT32 = "uint32_t";
const char* const TKNAME_INT64 = "int64_t";
const char* const TKNAME_UINT64 = "uint64_t";
const char* const TKNAME_CHAR8 = "char";
const char* const TKNAME_BYTE = "octet";
const char* const TKNAME_UINT8 = "uint8_t";
const char* const TKNAME_CHAR16 = "wchar";
const char* const TKNAME_CHAR16T = "wchar_t";
const char* const TKNAME_FLOAT32 = "float";
const char* const TKNAME_FLOAT64 = "double";
const char* const TKNAME_FLOAT128 = "longdouble";

const char* const TKNAME_STRING8 = "string";
const char* const TKNAME_STRING16 = "wstring";
const char* const TKNAME_ALIAS = "alias";
const char* const TKNAME_ENUM = "enum";
const char* const TKNAME_BITMASK = "bitmask";
const char* const TKNAME_ANNOTATION = "annotation";
const char* const TKNAME_STRUCTURE = "structure";
const char* const TKNAME_UNION = "union";
const char* const TKNAME_BITSET = "bitset";
const char* const TKNAME_SEQUENCE = "sequence";
const char* const TKNAME_ARRAY = "array";
const char* const TKNAME_MAP = "map";

} // namespace xtypes_names

namespace typekind_detail {

template<TypeKind kind, class CharT, class Traits>
struct TypeKindName {};

#define XTYPENAME(type)                                                                           \
    template<>                                                                                    \
    struct TypeKindName<eprosima::fastdds::dds::type, char, std::char_traits<char>>       \
    {                                                                                             \
        RTPS_DllAPI static const char* name;                                                      \
    };                                                                                            \
                                                                                                  \
    template<>                                                                                    \
    struct TypeKindName<eprosima::fastdds::dds::type, wchar_t, std::char_traits<wchar_t>> \
    {                                                                                             \
        RTPS_DllAPI static const wchar_t* name;                                                   \
    };                                                                                            \

XTYPENAME(TK_BOOLEAN)
XTYPENAME(TK_BYTE)
XTYPENAME(TK_INT16)
XTYPENAME(TK_INT32)
XTYPENAME(TK_INT64)
XTYPENAME(TK_UINT16)
XTYPENAME(TK_UINT32)
XTYPENAME(TK_UINT64)
XTYPENAME(TK_FLOAT32)
XTYPENAME(TK_FLOAT64)
XTYPENAME(TK_FLOAT128)
XTYPENAME(TK_CHAR8)
XTYPENAME(TK_CHAR16)
XTYPENAME(TK_STRING8)
XTYPENAME(TK_STRING16)
XTYPENAME(TK_ALIAS)
XTYPENAME(TK_ENUM)
XTYPENAME(TK_BITMASK)
XTYPENAME(TK_ANNOTATION)
XTYPENAME(TK_STRUCTURE)
XTYPENAME(TK_UNION)
XTYPENAME(TK_BITSET)
XTYPENAME(TK_SEQUENCE)
XTYPENAME(TK_ARRAY)
XTYPENAME(TK_MAP)

#undef XTYPENAME

} // namespace typekind_detail

#define XTYPECASE(type)                                                                                    \
    case eprosima::fastdds::dds::type:                                                             \
        name = typekind_detail::TypeKindName<eprosima::fastdds::dds::type, CharT, Traits>::name;   \
        break;                                                                                             \

template< class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
operator <<(
        std::basic_ostream<CharT, Traits>& os,
        TypeKind kind)
{
    const CharT* name = nullptr;
    switch (kind)
    {
        XTYPECASE(TK_BOOLEAN)
        XTYPECASE(TK_BYTE)
        XTYPECASE(TK_INT16)
        XTYPECASE(TK_INT32)
        XTYPECASE(TK_INT64)
        XTYPECASE(TK_UINT16)
        XTYPECASE(TK_UINT32)
        XTYPECASE(TK_UINT64)
        XTYPECASE(TK_FLOAT32)
        XTYPECASE(TK_FLOAT64)
        XTYPECASE(TK_FLOAT128)
        XTYPECASE(TK_CHAR8)
        XTYPECASE(TK_CHAR16)
        XTYPECASE(TK_STRING8)
        XTYPECASE(TK_STRING16)
        XTYPECASE(TK_ALIAS)
        XTYPECASE(TK_ENUM)
        XTYPECASE(TK_BITMASK)
        XTYPECASE(TK_ANNOTATION)
        XTYPECASE(TK_STRUCTURE)
        XTYPECASE(TK_UNION)
        XTYPECASE(TK_BITSET)
        XTYPECASE(TK_SEQUENCE)
        XTYPECASE(TK_ARRAY)
        XTYPECASE(TK_MAP)
        default:
            return os;
    }

    return os << name;
}

#undef XTYPECASE

const int32_t MEMBER_NAME_MAX_LENGTH = 256;
typedef std::string MemberName;

// Qualified type name includes the name of containing modules
// using "::" as separator. No leading "::". E.g. "MyModule::MyType"
const int32_t TYPE_NAME_MAX_LENGTH = 256;
typedef std::string QualifiedTypeName;

// Every type has an ID. Those of the primitive types are pre-defined.
typedef eprosima::fastrtps::rtps::octet PrimitiveTypeId;

// First 4 bytes of MD5 of of a member name converted to bytes
// using UTF-8 encoding and without a 'nul' terminator.
// Example: the member name "color" has NameHash {0x70, 0xDD, 0xA5, 0xDF}
typedef std::array<uint8_t, 4> NameHash;

// Mask used to remove the flags that do no affect assignability
// Selects  T1, T2, O, M, K, D
const uint16_t MemberFlagMinimalMask = 0x003f;

const int32_t MAX_BITMASK_LENGTH = 64;
const int32_t MAX_ELEMENTS_COUNT = 100;
const int32_t MAX_STRING_LENGTH = 255;

// Long Bound of a collection type
typedef uint32_t LBound;
typedef std::vector<LBound> LBoundSeq;
const LBound INVALID_LBOUND = 0;

// Short Bound of a collection type
typedef eprosima::fastrtps::rtps::octet SBound;
typedef std::vector<SBound> SBoundSeq;
const SBound INVALID_SBOUND = 0;

// Flags that apply to struct/union/collection/enum/bitmask/bitset
// members/elements and DO affect type assignability
// Depending on the flag it may not apply to members of all types

// When not all, the applicable member types are listed
class MemberFlag
{
private:

    std::bitset<16> m_MemberFlag;

public:

    MemberFlag()
    {
    }

    MemberFlag(
            const MemberFlag& x)
        : m_MemberFlag(x.m_MemberFlag)
    {
    }

    MemberFlag(
            MemberFlag&& x)
        : m_MemberFlag(std::move(x.m_MemberFlag))
    {
    }

    MemberFlag& operator =(
            const MemberFlag& x)
    {
        m_MemberFlag = x.m_MemberFlag;
        return *this;
    }

    MemberFlag& operator =(
            MemberFlag&& x)
    {
        m_MemberFlag = std::move(x.m_MemberFlag);
        return *this;
    }

    // T1 | 00 = INVALID, 01 = DISCARD
    bool TRY_CONSTRUCT1() const
    {
        return m_MemberFlag.test(0);
    }

    void TRY_CONSTRUCT1(
            bool b)
    {
        b ? m_MemberFlag.set(0) : m_MemberFlag.reset(0);
    }

    // T2 | 10 = USE_DEFAULT, 11 = TRIM
    bool TRY_CONSTRUCT2() const
    {
        return m_MemberFlag.test(1);
    }

    void TRY_CONSTRUCT2(
            bool b)
    {
        b ? m_MemberFlag.set(1) : m_MemberFlag.reset(1);
    }

    // X  StructMember, UnionMember,
    //    CollectionElement
    bool IS_EXTERNAL() const
    {
        return m_MemberFlag.test(2);
    }

    void IS_EXTERNAL(
            bool b)
    {
        b ? m_MemberFlag.set(2) : m_MemberFlag.reset(2);
    }

    // O  StructMember
    bool IS_OPTIONAL() const
    {
        return m_MemberFlag.test(3);
    }

    void IS_OPTIONAL(
            bool b)
    {
        b ? m_MemberFlag.set(3) : m_MemberFlag.reset(3);
    }

    // M  StructMember
    bool IS_MUST_UNDERSTAND() const
    {
        return m_MemberFlag.test(4);
    }

    void IS_MUST_UNDERSTAND(
            bool b)
    {
        b ? m_MemberFlag.set(4) : m_MemberFlag.reset(4);
    }

    // K  StructMember, UnionDiscriminator
    bool IS_KEY() const
    {
        return m_MemberFlag.test(5);
    }

    void IS_KEY(
            bool b)
    {
        b ? m_MemberFlag.set(5) : m_MemberFlag.reset(5);
    }

    // D  UnionMember, EnumerationLiteral
    bool IS_DEFAULT() const
    {
        return m_MemberFlag.test(6);
    }

    void IS_DEFAULT(
            bool b)
    {
        b ? m_MemberFlag.set(6) : m_MemberFlag.reset(6);
    }

    void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    static size_t getCdrSerializedSize(
            const MemberFlag&,
            size_t current_alignment = 0);

    bool operator ==(
            const MemberFlag& other) const
    {
        return m_MemberFlag == other.m_MemberFlag;
    }

    std::bitset<16> bitset() const
    {
        std::string str_value;

        str_value = m_MemberFlag.to_string() + str_value;

        return std::bitset<16>(str_value);
    }

    void bitset(
            const std::bitset<16>& bitset)
    {
        std::string str_value {bitset.to_string()};
        size_t base_diff {0};
        size_t last_post {std::string::npos};

        base_diff += 16;
        m_MemberFlag = std::bitset<16>(str_value.substr(str_value.length() - base_diff, last_post));
    }

};

typedef MemberFlag CollectionElementFlag;   // T1, T2, X
typedef MemberFlag StructMemberFlag;        // T1, T2, O, M, K, X
typedef MemberFlag UnionMemberFlag;         // T1, T2, D, X
typedef MemberFlag UnionDiscriminatorFlag;  // T1, T2, K
typedef MemberFlag EnumeratedLiteralFlag;   // D
typedef MemberFlag AnnotationParameterFlag; // Unused. No flags apply
typedef MemberFlag AliasMemberFlag;         // Unused. No flags apply
typedef MemberFlag BitflagFlag;             // Unused. No flags apply
typedef MemberFlag BitsetMemberFlag;        // Unused. No flags apply

// Flags that apply to type declarationa and DO affect assignability
// Depending on the flag it may not apply to all types
// When not all, the applicable  types are listed
class TypeFlag
{
private:

    std::bitset<16> m_TypeFlag;

public:

    TypeFlag()
    {
    }

    TypeFlag(
            const TypeFlag& x)
        : m_TypeFlag(x.m_TypeFlag)
    {
    }

    TypeFlag(
            TypeFlag&& x)
        : m_TypeFlag(std::move(x.m_TypeFlag))
    {
    }

    TypeFlag& operator =(
            const TypeFlag& x)
    {
        m_TypeFlag = x.m_TypeFlag;
        return *this;
    }

    TypeFlag& operator =(
            TypeFlag&& x)
    {
        m_TypeFlag = std::move(x.m_TypeFlag);
        return *this;
    }

    // F |
    bool IS_FINAL() const
    {
        return m_TypeFlag.test(0);
    }

    void IS_FINAL(
            bool b)
    {
        b ? m_TypeFlag.set(0) : m_TypeFlag.reset(0);
    }

    // A |-  Struct, Union
    bool IS_APPENDABLE() const
    {
        return m_TypeFlag.test(1);
    }

    void IS_APPENDABLE(
            bool b)
    {
        b ? m_TypeFlag.set(1) : m_TypeFlag.reset(1);
    }

    // M |   (exactly one flag)
    bool IS_MUTABLE() const
    {
        return m_TypeFlag.test(2);
    }

    void IS_MUTABLE(
            bool b)
    {
        b ? m_TypeFlag.set(2) : m_TypeFlag.reset(2);
    }

    // N     Struct, Union
    bool IS_NESTED() const
    {
        return m_TypeFlag.test(3);
    }

    void IS_NESTED(
            bool b)
    {
        b ? m_TypeFlag.set(3) : m_TypeFlag.reset(3);
    }

    // H     Struct
    bool IS_AUTOID_HASH() const
    {
        return m_TypeFlag.test(4);
    }

    void IS_AUTOID_HASH(
            bool b)
    {
        b ? m_TypeFlag.set(4) : m_TypeFlag.reset(4);
    }

    void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    static size_t getCdrSerializedSize(
            const TypeFlag&,
            size_t current_alignment = 0);

    bool operator ==(
            const TypeFlag& other) const
    {
        return m_TypeFlag == other.m_TypeFlag;
    }

    std::bitset<16> bitset() const
    {
        std::string str_value;

        str_value = m_TypeFlag.to_string() + str_value;

        return std::bitset<16>(str_value);
    }

    void bitset(
            const std::bitset<16>& bitset)
    {
        std::string str_value {bitset.to_string()};
        size_t base_diff {0};
        size_t last_post {std::string::npos};

        base_diff += 16;
        m_TypeFlag = std::bitset<16>(str_value.substr(str_value.length() - base_diff, last_post));
    }

};

typedef TypeFlag StructTypeFlag;        // All flags apply
typedef TypeFlag UnionTypeFlag;         // All flags apply
typedef TypeFlag CollectionTypeFlag;    // Unused. No flags apply
typedef TypeFlag AnnotationTypeFlag;    // Unused. No flags apply
typedef TypeFlag AliasTypeFlag;         // Unused. No flags apply
typedef TypeFlag EnumTypeFlag;          // Unused. No flags apply
typedef TypeFlag BitmaskTypeFlag;       // Unused. No flags apply
typedef TypeFlag BitsetTypeFlag;        // Unused. No flags apply


// Mask used to remove the flags that do no affect assignability
const uint16_t TypeFlagMinimalMask = 0x0007; // Selects  M, A, F

// --- Annotation usage: ----------------------------------------------

// ID of a type member
const uint32_t ANNOTATION_STR_VALUE_MAX_LEN = 128;
const uint32_t ANNOTATION_OCTETSEC_VALUE_MAX_LEN = 128;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPES_HPP
