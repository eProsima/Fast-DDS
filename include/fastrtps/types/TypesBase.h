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

#include <string>

#include <fastdds/rtps/common/Types.h>

namespace eprosima {
namespace fastrtps {
namespace types {

//! A special value indicating an unlimited quantity
constexpr uint32_t BOUND_UNLIMITED = 0;

const std::string CONST_TRUE = "true";
const std::string CONST_FALSE = "false";

const std::string ANNOTATION_KEY_ID = "key";
const std::string ANNOTATION_EPKEY_ID = "Key";
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
const std::string TKNAME_INT8 = "int8_t";
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

typedef eprosima::fastrtps::rtps::octet TypeKind;

typedef uint32_t MemberId;
constexpr uint32_t MEMBER_ID_INVALID {0X0FFFFFFF};
#define INDEX_INVALID UINT32_MAX

const int32_t MAX_BITMASK_LENGTH = 64;
const int32_t MAX_ELEMENTS_COUNT = 100;
const int32_t MAX_STRING_LENGTH = 255;

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_BASE_H
