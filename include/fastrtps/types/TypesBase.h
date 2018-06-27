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

#include <fastrtps/rtps/common/Types.h>
#include <string>
#include <map>
#include <vector>

namespace eprosima{
namespace fastrtps{

using namespace rtps;

namespace types{

// ---------- Equivalence Kinds ------------------
typedef octet EquivalenceKind;
const octet EK_MINIMAL   = 0xF1; // 0x1111 0001
const octet EK_COMPLETE  = 0xF2; // 0x1111 0010
const octet EK_BOTH      = 0xF3; // 0x1111 0011

// ---------- TypeKinds (begin) ------------------
typedef octet TypeKind;        // Primitive TKs

const octet TK_NONE       = 0x00;
const octet TK_BOOLEAN    = 0x01;
const octet TK_BYTE       = 0x02;
const octet TK_INT16      = 0x03;
const octet TK_INT32      = 0x04;
const octet TK_INT64      = 0x05;
const octet TK_UINT16     = 0x06;
const octet TK_UINT32     = 0x07;
const octet TK_UINT64     = 0x08;
const octet TK_FLOAT32    = 0x09;
const octet TK_FLOAT64    = 0x0A;
const octet TK_FLOAT128   = 0x0B;
const octet TK_CHAR8      = 0x10;
const octet TK_CHAR16     = 0x11;

// String TKs
const octet TK_STRING8    = 0x20;
const octet TK_STRING16   = 0x21;


// Constructed/Named types
const octet TK_ALIAS      = 0x30;

// Enumerated TKs
const octet TK_ENUM       = 0x40;
const octet TK_BITMASK    = 0x41;

// Structured TKs
const octet TK_ANNOTATION = 0x50;
const octet TK_STRUCTURE  = 0x51;
const octet TK_UNION      = 0x52;
const octet TK_BITSET     = 0x53;

// Collection TKs
const octet TK_SEQUENCE   = 0x60;
const octet TK_ARRAY      = 0x61;
const octet TK_MAP        = 0x62;

// ---------- TypeKinds (end) ------------------

/*!
 * @brief This class represents the enumeration ResponseCode.
 */
enum ResponseCode : uint32_t
{
    RETCODE_ERROR = (uint32_t)(-1),
    RETCODE_OK = 0,
    RETCODE_BAD_PARAMETER
};


class TypeSupport
{
public:
    //int register_type(DomainParticipant domain, std::string type_name);
    //std::string get_type_name();
    //DynamicType get_type();
};

class DynamicTypeSupport : public TypeSupport
{
public:
    //static DynamicTypeSupport create_type_support(DynamicType type);
    //static ResponseCode delete_type_support(DynamicTypeSupport type_support);

    //ResponseCode register_type(DomainParticipant participant, std::string type_name);
    //std::string get_type_name();
    //DynamicType get_type();
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_BASE_H


