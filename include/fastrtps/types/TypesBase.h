// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_BASE_H
#define TYPES_BASE_H

#include <fastdds/rtps/common/Types.h>

#include <algorithm>
#include <bitset>
#include <cctype>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {
using DomainId_t = uint32_t;
} // dds
} // namespace fastdds

namespace fastcdr {
class Cdr;
} // namespace fastcdr
namespace fastrtps {
namespace types {

//! A special value indicating an unlimited quantity
constexpr uint32_t BOUND_UNLIMITED = 0;

using eprosima::fastrtps::rtps::octet;

using OctetSeq = std::vector<octet>;

OctetSeq& operator ++(
        OctetSeq&);

OctetSeq operator ++(
        OctetSeq&,
        int);

size_t to_size_t(
        const OctetSeq&);

inline namespace xtypes_names {

const std::string CONST_TRUE = "true";
const std::string CONST_FALSE = "false";

const std::string ANNOTATION_KEY_ID = "key";
const std::string ANNOTATION_EPKEY_ID = "Key";
const std::string ANNOTATION_TOPIC_ID = "Topic";
const std::string ANNOTATION_EXTENSIBILITY_ID = "extensibility";
const std::string ANNOTATION_FINAL_ID = "final";
const std::string ANNOTATION_APPENDABLE_ID = "appendable";
const std::string ANNOTATION_MUTABLE_ID = "mutable";
const std::string ANNOTATION_NESTED_ID = "nested";
const std::string ANNOTATION_OPTIONAL_ID = "optional";
const std::string ANNOTATION_MUST_UNDERSTAND_ID = "must_understand";
const std::string ANNOTATION_NON_SERIALIZED_ID = "non_serialized";
const std::string ANNOTATION_BIT_BOUND_ID = "bit_bound";
const std::string ANNOTATION_DEFAULT_ID = "default";
const std::string ANNOTATION_DEFAULT_LITERAL_ID = "default_literal";
const std::string ANNOTATION_VALUE_ID = "value";
const std::string ANNOTATION_POSITION_ID = "position";
const std::string ANNOTATION_EXTERNAL_ID = "external";

const std::string EXTENSIBILITY_FINAL = "FINAL";
const std::string EXTENSIBILITY_APPENDABLE = "APPENDABLE";
const std::string EXTENSIBILITY_MUTABLE = "MUTABLE";

const std::string TKNAME_BOOLEAN = "bool";
const std::string TKNAME_INT16 = "int16_t";
const std::string TKNAME_UINT16 = "uint16_t";
const std::string TKNAME_INT32 = "int32_t";
const std::string TKNAME_UINT32 = "uint32_t";
const std::string TKNAME_INT64 = "int64_t";
const std::string TKNAME_UINT64 = "uint64_t";
const std::string TKNAME_CHAR8 = "char";
const std::string TKNAME_BYTE = "octet";
const std::string TKNAME_UINT8 = "uint8_t";
const std::string TKNAME_CHAR16 = "wchar";
const std::string TKNAME_CHAR16T = "wchar_t";
const std::string TKNAME_FLOAT32 = "float";
const std::string TKNAME_FLOAT64 = "double";
const std::string TKNAME_FLOAT128 = "longdouble";

const std::string TKNAME_STRING8 = "string";
const std::string TKNAME_STRING16 = "wstring";
const std::string TKNAME_ALIAS = "alias";
const std::string TKNAME_ENUM = "enum";
const std::string TKNAME_BITMASK = "bitmask";
const std::string TKNAME_ANNOTATION = "annotation";
const std::string TKNAME_STRUCTURE = "structure";
const std::string TKNAME_UNION = "union";
const std::string TKNAME_BITSET = "bitset";
const std::string TKNAME_SEQUENCE = "sequence";
const std::string TKNAME_ARRAY = "array";
const std::string TKNAME_MAP = "map";

} // namespace xtypes_names

// ---------- TypeKinds (begin) ------------------

enum class TypeKind : octet
{
    // invalid
    TK_NONE = 0x00,

    // Primitive TKs
    TK_BOOLEAN = 0x01,
    TK_BYTE = 0x02,
    TK_INT16 = 0x03,
    TK_INT32 = 0x04,
    TK_INT64 = 0x05,
    TK_UINT16 = 0x06,
    TK_UINT32 = 0x07,
    TK_UINT64 = 0x08,
    TK_FLOAT32 = 0x09,
    TK_FLOAT64 = 0x0A,
    TK_FLOAT128 = 0x0B,
    TK_CHAR8 = 0x10,
    TK_CHAR16 = 0x11,

    // String TKs
    TK_STRING8 = 0x20,
    TK_STRING16 = 0x21,

    // Constructed/Named types
    TK_ALIAS = 0x30,

    // Enumerated TKs
    TK_ENUM = 0x40,
    TK_BITMASK = 0x41,

    // Structured TKs
    TK_ANNOTATION = 0x50,
    TK_STRUCTURE = 0x51,
    TK_UNION = 0x52,
    TK_BITSET = 0x53,

    // Collection TKs
    TK_SEQUENCE = 0x60,
    TK_ARRAY = 0x61,
    TK_MAP = 0x62,

    // TypeIdentifiers
    TI_STRING8_SMALL = 0x70,
    TI_STRING8_LARGE = 0x71,
    TI_STRING16_SMALL = 0x72,
    TI_STRING16_LARGE = 0x73,
    TI_PLAIN_SEQUENCE_SMALL = 0x80,
    TI_PLAIN_SEQUENCE_LARGE = 0x81,
    TI_PLAIN_ARRAY_SMALL = 0x90,
    TI_PLAIN_ARRAY_LARGE = 0x91,
    TI_PLAIN_MAP_SMALL = 0xA0,
    TI_PLAIN_MAP_LARGE = 0xA1,
    TI_STRONGLY_CONNECTED_COMPONENT = 0xB0,

    // Equivalence Kinds
    EK_MINIMAL = 0xF1, // 0x1111 0001
    EK_COMPLETE = 0xF2, // 0x1111 0010
    EK_BOTH = 0xF3, // 0x1111 0011
};

inline bool operator ==(
        octet a,
        TypeKind b)
{
    return a == static_cast<octet>(b);
}

inline bool operator ==(
        TypeKind a,
        octet b)
{
    return b == a;
}

namespace typekind_detail {

template<TypeKind kind, class CharT, class Traits>
struct TypeKindName {};

#define XTYPENAME(type)                                                \
    template<>                                                             \
    struct TypeKindName<TypeKind::type, char, std::char_traits<char>>      \
    {                                                                      \
        RTPS_DllAPI static const char* name;                               \
    };                                                                     \
                                                                       \
    template<>                                                             \
    struct TypeKindName<TypeKind::type, wchar_t, std::char_traits<wchar_t>> \
    {                                                                      \
        RTPS_DllAPI static const wchar_t* name;                            \
    };                                                                     \

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
XTYPENAME(TI_STRING8_SMALL)
XTYPENAME(TI_STRING8_LARGE)
XTYPENAME(TI_STRING16_SMALL)
XTYPENAME(TI_STRING16_LARGE)
XTYPENAME(TI_PLAIN_SEQUENCE_SMALL)
XTYPENAME(TI_PLAIN_SEQUENCE_LARGE)
XTYPENAME(TI_PLAIN_ARRAY_SMALL)
XTYPENAME(TI_PLAIN_ARRAY_LARGE)
XTYPENAME(TI_PLAIN_MAP_SMALL)
XTYPENAME(TI_PLAIN_MAP_LARGE)
XTYPENAME(TI_STRONGLY_CONNECTED_COMPONENT)

#undef XTYPENAME

} // namespace typekind_detail

#define XTYPECASE(type)                                                                  \
    case TypeKind::type:                                                             \
        name = typekind_detail::TypeKindName<TypeKind::type, CharT, Traits>::name;   \
        break;                                                                       \

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
        XTYPECASE(TI_STRING8_SMALL)
        XTYPECASE(TI_STRING8_LARGE)
        XTYPECASE(TI_STRING16_SMALL)
        XTYPECASE(TI_STRING16_LARGE)
        XTYPECASE(TI_PLAIN_SEQUENCE_SMALL)
        XTYPECASE(TI_PLAIN_SEQUENCE_LARGE)
        XTYPECASE(TI_PLAIN_ARRAY_SMALL)
        XTYPECASE(TI_PLAIN_ARRAY_LARGE)
        XTYPECASE(TI_PLAIN_MAP_SMALL)
        XTYPECASE(TI_PLAIN_MAP_LARGE)
        XTYPECASE(TI_STRONGLY_CONNECTED_COMPONENT)
        default:
            return os;
    }

    return os << name;
}

#undef XTYPECASE

// Version 1.1 per-compilation unit globals

const octet TK_NONE = static_cast<octet>(TypeKind::TK_NONE);
const octet TK_BOOLEAN = static_cast<octet>(TypeKind::TK_BOOLEAN);
const octet TK_BYTE = static_cast<octet>(TypeKind::TK_BYTE);
const octet TK_INT16 = static_cast<octet>(TypeKind::TK_INT16);
const octet TK_INT32 = static_cast<octet>(TypeKind::TK_INT32);
const octet TK_INT64 = static_cast<octet>(TypeKind::TK_INT64);
const octet TK_UINT16 = static_cast<octet>(TypeKind::TK_UINT16);
const octet TK_UINT32 = static_cast<octet>(TypeKind::TK_UINT32);
const octet TK_UINT64 = static_cast<octet>(TypeKind::TK_UINT64);
const octet TK_FLOAT32 = static_cast<octet>(TypeKind::TK_FLOAT32);
const octet TK_FLOAT64 = static_cast<octet>(TypeKind::TK_FLOAT64);
const octet TK_FLOAT128 = static_cast<octet>(TypeKind::TK_FLOAT128);
const octet TK_CHAR8 = static_cast<octet>(TypeKind::TK_CHAR8);
const octet TK_CHAR16 = static_cast<octet>(TypeKind::TK_CHAR16);

// String TKs
const octet TK_STRING8 = static_cast<octet>(TypeKind::TK_STRING8);
const octet TK_STRING16 = static_cast<octet>(TypeKind::TK_STRING16);

// String TIs
const octet TI_STRING8_SMALL = static_cast<octet>(TypeKind::TI_STRING8_SMALL);
const octet TI_STRING8_LARGE = static_cast<octet>(TypeKind::TI_STRING8_LARGE);
const octet TI_STRING16_SMALL = static_cast<octet>(TypeKind::TI_STRING16_SMALL);
const octet TI_STRING16_LARGE = static_cast<octet>(TypeKind::TI_STRING16_LARGE);

// Constructed/Named types
const octet TK_ALIAS = static_cast<octet>(TypeKind::TK_ALIAS);

// Enumerated TKs
const octet TK_ENUM = static_cast<octet>(TypeKind::TK_ENUM);
const octet TK_BITMASK = static_cast<octet>(TypeKind::TK_BITMASK);

// Structured TKs
const octet TK_ANNOTATION = static_cast<octet>(TypeKind::TK_ANNOTATION);
const octet TK_STRUCTURE = static_cast<octet>(TypeKind::TK_STRUCTURE);
const octet TK_UNION = static_cast<octet>(TypeKind::TK_UNION);
const octet TK_BITSET = static_cast<octet>(TypeKind::TK_BITSET);

// Collection TKs
const octet TK_SEQUENCE = static_cast<octet>(TypeKind::TK_SEQUENCE);
const octet TK_ARRAY = static_cast<octet>(TypeKind::TK_ARRAY);
const octet TK_MAP = static_cast<octet>(TypeKind::TK_MAP);

// Collection TIs
const octet TI_PLAIN_SEQUENCE_SMALL = static_cast<octet>(TypeKind::TI_PLAIN_SEQUENCE_SMALL);
const octet TI_PLAIN_SEQUENCE_LARGE = static_cast<octet>(TypeKind::TI_PLAIN_SEQUENCE_LARGE);
const octet TI_PLAIN_ARRAY_SMALL = static_cast<octet>(TypeKind::TI_PLAIN_ARRAY_SMALL);
const octet TI_PLAIN_ARRAY_LARGE = static_cast<octet>(TypeKind::TI_PLAIN_ARRAY_LARGE);
const octet TI_PLAIN_MAP_SMALL = static_cast<octet>(TypeKind::TI_PLAIN_MAP_SMALL);
const octet TI_PLAIN_MAP_LARGE = static_cast<octet>(TypeKind::TI_PLAIN_MAP_LARGE);
const octet TI_STRONGLY_CONNECTED_COMPONENT = static_cast<octet>(TypeKind::TI_STRONGLY_CONNECTED_COMPONENT);

// Equivalence Kinds
const octet EK_MINIMAL = static_cast<octet>(TypeKind::EK_MINIMAL);
const octet EK_COMPLETE = static_cast<octet>(TypeKind::EK_COMPLETE);
const octet EK_BOTH = static_cast<octet>(TypeKind::EK_BOTH);

// ---------- TypeKinds (end) ------------------

// Auxiliary metadata

template<TypeKind kind>
using is_primitive = std::conditional<(kind > TypeKind::TK_NONE && kind <= TypeKind::TK_CHAR16), std::true_type,
                std::false_type>;

template<TypeKind kind>
using is_primitive_t = typename is_primitive<kind>::type;

// The name of some element (e.g. type, type member, module)
// Valid characters are alphanumeric plus the "_" cannot start with digit

const int32_t MEMBER_NAME_MAX_LENGTH = 256;
typedef std::string MemberName;

// Qualified type name includes the name of containing modules
// using "::" as separator. No leading "::". E.g. "MyModule::MyType"
const int32_t TYPE_NAME_MAX_LENGTH = 256;
typedef std::string QualifiedTypeName;

// Every type has an ID. Those of the primitive types are pre-defined.
typedef octet PrimitiveTypeId;

// First 4 bytes of MD5 of of a member name converted to bytes
// using UTF-8 encoding and without a 'nul' terminator.
// Example: the member name "color" has NameHash {0x70, 0xDD, 0xA5, 0xDF}
typedef std::array<uint8_t, 4> NameHash;

// Mask used to remove the flags that do no affect assignability
// Selects  T1, T2, O, M, K, D
const uint16_t MemberFlagMinimalMask = 0x003f;

/*!
 * @brief This class represents the enumeration ReturnCode_t.
 */

class RTPS_DllAPI ReturnCode_t
{
    uint32_t value_;

public:

    enum ReturnCodeValue
    {
        RETCODE_OK = 0,
        RETCODE_ERROR = 1,
        RETCODE_UNSUPPORTED = 2,
        RETCODE_BAD_PARAMETER = 3,
        RETCODE_PRECONDITION_NOT_MET = 4,
        RETCODE_OUT_OF_RESOURCES = 5,
        RETCODE_NOT_ENABLED = 6,
        RETCODE_IMMUTABLE_POLICY = 7,
        RETCODE_INCONSISTENT_POLICY = 8,
        RETCODE_ALREADY_DELETED = 9,
        RETCODE_TIMEOUT = 10,
        RETCODE_NO_DATA = 11,
        RETCODE_ILLEGAL_OPERATION = 12,
        RETCODE_NOT_ALLOWED_BY_SECURITY = 13
    };

    ReturnCode_t()
        : value_(RETCODE_OK)
    {
    }

    ReturnCode_t(
            uint32_t e)
    {
        value_ = e;
    }

    bool operator ==(
            const ReturnCode_t& c) const
    {
        return value_ == c.value_;
    }

    bool operator !=(
            const ReturnCode_t& c) const
    {
        return value_ != c.value_;
    }

    explicit operator bool() = delete;

    uint32_t operator ()() const
    {
        return value_;
    }

    bool operator !() const
    {
        return value_ != 0;
    }

};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

// linking ReturnCode_t to std::error_code
// The specializations must be in the outer namespace (see N3730)
namespace std {

template <>
struct is_error_code_enum<eprosima::fastrtps::types::ReturnCode_t> : true_type {};

template <>
struct is_error_code_enum<eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue> : true_type {};

} // namespace std

namespace eprosima {
namespace fastrtps {
namespace types {

// Integrating ReturnCode_t into STL error framework
namespace {

struct FastDDSErrCategory : std::error_category
{
    const char* name() const noexcept
    {
        return "Fast-DDS error reporting";
    }

    std::string message(
            int ev) const
    {
        switch (ReturnCode_t(ev)())
        {
            case ReturnCode_t::RETCODE_OK:
                return "Success";
            case ReturnCode_t::RETCODE_UNSUPPORTED:
                return "Unsupported feature";
            case ReturnCode_t::RETCODE_BAD_PARAMETER:
                return "Bad parameter";
            case ReturnCode_t::RETCODE_PRECONDITION_NOT_MET:
                return "Precondition not met";
            case ReturnCode_t::RETCODE_OUT_OF_RESOURCES:
                return "Out of resources";
            case ReturnCode_t::RETCODE_NOT_ENABLED:
                return "Disabled";
            case ReturnCode_t::RETCODE_IMMUTABLE_POLICY:
                return "Immutable policy";
            case ReturnCode_t::RETCODE_INCONSISTENT_POLICY:
                return "Inconsistent policy";
            case ReturnCode_t::RETCODE_ALREADY_DELETED:
                return "Already deleted";
            case ReturnCode_t::RETCODE_TIMEOUT:
                return "Timeout";
            case ReturnCode_t::RETCODE_NO_DATA:
                return "No data";
            case ReturnCode_t::RETCODE_ILLEGAL_OPERATION:
                return "Illegal operation";
            case ReturnCode_t::RETCODE_NOT_ALLOWED_BY_SECURITY:
                return "Security preemption";
            case ReturnCode_t::RETCODE_ERROR:
            default:
                return "Unrecognized error";
        }
    }

};

} // anonymous namespace

inline std::error_code make_error_code(
        ReturnCode_t::ReturnCodeValue r)
{
    static FastDDSErrCategory eprosima_fastdds;
    return { r, eprosima_fastdds };
}

inline std::error_code make_error_code(
        const ReturnCode_t& r)
{
    return make_error_code(static_cast<ReturnCode_t::ReturnCodeValue>(r()));
}

template<class T>
typename std::enable_if<std::is_arithmetic<T>::value
        || std::is_same<T, ReturnCode_t::ReturnCodeValue>::value, bool>::type
operator ==(
        T a,
        const ReturnCode_t& b)
{
    return b.operator ==(a);
}

template<class T>
typename std::enable_if<std::is_arithmetic<T>::value
        || std::is_same<T, ReturnCode_t::ReturnCodeValue>::value, bool>::type
operator !=(
        T a,
        const ReturnCode_t& b)
{
    return b.operator !=(a);
}

#define INDEX_INVALID UINT32_MAX

const int32_t MAX_BITMASK_LENGTH = 64;
const int32_t MAX_ELEMENTS_COUNT = 100;
const int32_t MAX_STRING_LENGTH = 255;

// Long Bound of a collection type
typedef uint32_t LBound;
typedef std::vector<LBound> LBoundSeq;
const LBound INVALID_LBOUND = 0;

// Short Bound of a collection type
typedef octet SBound;
typedef std::vector<SBound> SBoundSeq;
const SBound INVALID_SBOUND = 0;

// Auxiliar function to compare sequences (std::vector)
template<class T>
bool compareSequence(
        const std::vector<T>& a,
        const std::vector<T>& b)
{
    if (a.size() == b.size())
    {
        auto aIt = a.begin();
        auto bIt = b.begin();
        while (aIt != a.end() && bIt != b.end())
        {
            if (*aIt == *bIt)
            {
                ++aIt;
                ++bIt;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

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

inline namespace v1_1 {

using MemberId = uint32_t;
const MemberId MEMBER_ID_INVALID = 0x0FFFFFFF;

class DynamicType;
class DynamicTypeBuilder;
class DynamicType_ptr;

} // namespace v1_1

namespace v1_3 {

class DynamicType;
class DynamicTypeBuilder;

template<class T>
std::function<void(const T*)> dynamic_object_deleter(const T*);

using namespace xtypes_names;

constexpr uint32_t LENGTH_UNLIMITED = std::numeric_limits<uint32_t>::max();

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

namespace std {

template<> class weak_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>;
template<> class shared_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>;
template<> class weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>;
template<> class shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>;

} // namespace std

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

using DynamicType_ptr = std::shared_ptr<DynamicType>;
using DynamicTypeBuilder_ptr = std::shared_ptr<DynamicTypeBuilder>;
using DynamicTypeBuilder_cptr = std::shared_ptr<const DynamicTypeBuilder>;

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_BASE_H
