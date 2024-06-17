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

/*!
 * @file common.hpp
 * This file contains common definitions for the different XTypes modules.
 */

#ifndef FASTDDS_DDS_XTYPES__COMMON_HPP
#define FASTDDS_DDS_XTYPES__COMMON_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

enum class ExtensibilityKind : uint32_t
{
    FINAL,
    APPENDABLE,
    MUTABLE
};

enum class TryConstructFailAction : uint32_t
{
    DISCARD,
    USE_DEFAULT,
    TRIM
};

/*!
 * @brief PlacementKind values (verbatim annotation)
 */
enum class PlacementKind : uint32_t
{
    BEGIN_FILE,
    BEFORE_DECLARATION,
    BEGIN_DECLARATION,
    END_DECLARATION,
    AFTER_DECLARATION,
    END_FILE
};

/*!
 * @brief Valid placement values
 */
constexpr const char* begin_declaration_file_str = "begin-declaration-file";
constexpr const char* before_declaration_str = "before-declaration";
constexpr const char* begin_declaration_str = "begin-declaration";
constexpr const char* end_declaration_str = "end-declaration";
constexpr const char* after_declaration_str = "after-declaration";
constexpr const char* end_declaration_file_str = "end-declaration-file";

// Primitive type names
constexpr const char* boolean_type_name = "_bool";
constexpr const char* byte_type_name = "_byte";
constexpr const char* int16_type_name = "_int16_t";
constexpr const char* int32_type_name = "_int32_t";
constexpr const char* int64_type_name = "_int64_t";
constexpr const char* uint16_type_name = "_uint16_t";
constexpr const char* uint32_type_name = "_uint32_t";
constexpr const char* uint64_type_name = "_uint64_t";
constexpr const char* float32_type_name = "_float";
constexpr const char* float64_type_name = "_double";
constexpr const char* float128_type_name = "_longdouble";
constexpr const char* int8_type_name = "_int8_t";
constexpr const char* uint8_type_name = "_uint8_t";
constexpr const char* char8_type_name = "_char";
constexpr const char* char16_type_name = "_wchar_t";

// Builtin annotation names
constexpr const char* id_annotation_name = "id";
constexpr const char* autoid_annotation_name = "autoid";
constexpr const char* optional_annotation_name = "optional";
constexpr const char* position_annotation_name = "position";
constexpr const char* value_annotation_name = "value"; // Pending implementation
constexpr const char* extensibility_annotation_name = "extensibility";
constexpr const char* final_annotation_name = "final";
constexpr const char* appendable_annotation_name = "appendable";
constexpr const char* mutable_annotation_name = "mutable";
constexpr const char* key_annotation_name = "key";
constexpr const char* must_understand_annotation_name = "must_understand";
constexpr const char* default_literal_annotation_name = "default_literal";
constexpr const char* default_annotation_name = "default"; // Pending implementation
constexpr const char* range_annotation_name = "range"; // Pending implementation
constexpr const char* min_annotation_name = "min"; // Pending implementation
constexpr const char* max_annotation_name = "max"; // Pending implementation
constexpr const char* unit_annotation_name = "unit";
constexpr const char* bit_bound_annotation_name = "bit_bound";
constexpr const char* external_annotation_name = "external";
constexpr const char* nested_annotation_name = "nested";
constexpr const char* verbatim_annotation_name = "verbatim";
constexpr const char* service_annotation_name = "service";
constexpr const char* oneway_annotation_name = "oneway";
constexpr const char* ami_annotation_name = "ami";
constexpr const char* hashid_annotation_name = "hashid";
constexpr const char* default_nested_annotation_name = "default_nested";
constexpr const char* ignore_literal_names_annotation_name = "ignore_literal_names";
constexpr const char* try_construct_annotation_name = "try_construct";
constexpr const char* non_serialized_annotation_name = "non_serialized";
constexpr const char* data_representation_annotation_name = "data_representation";
constexpr const char* topic_annotation_name = "topic";

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_XTYPES__COMMON_HPP
