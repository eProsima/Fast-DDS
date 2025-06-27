// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERTAGS_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERTAGS_HPP

#include "../TypeValueConverter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

    static constexpr const char* IDL_TRUE_TAG = "true";
    static constexpr const char* IDL_FALSE_TAG = "false";
    static constexpr const char* IDL_VALUE_TAG = "value";

    // @id
    static constexpr const char* IDL_BUILTIN_ANN_ID_TAG = "id";

    // @autoid
    static constexpr const char* IDL_BUILTIN_ANN_AUTOID_TAG = "autoid";
    static constexpr const char* IDL_BUILTIN_ANN_AUTOID_KIND_TAG = "AutoidKind";
    static constexpr const char* IDL_BUILTIN_ANN_AUTOID_KIND_SEQUENTIAL_TAG = "SEQUENTIAL";
    static constexpr const char* IDL_BUILTIN_ANN_AUTOID_KIND_HASH_TAG = "HASH";

    // @optional
    static constexpr const char* IDL_BUILTIN_ANN_OPTIONAL_TAG = "optional";

    // @position
    static constexpr const char* IDL_BUILTIN_ANN_POSITION_TAG = "position";

    // @value
    static constexpr const char* IDL_BUILTIN_ANN_VALUE_TAG = "value";

    // @extensibility
    static constexpr const char* IDL_BUILTIN_ANN_EXTENSIBILITY_TAG = "extensibility";
    static constexpr const char* IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG = "ExtensibilityKind";
    static constexpr const char* IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_FINAL_TAG = "FINAL";
    static constexpr const char* IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_APPENDABLE_TAG = "APPENDABLE";
    static constexpr const char* IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_MUTABLE_TAG = "MUTABLE";

    // @final
    static constexpr const char* IDL_BUILTIN_ANN_FINAL_TAG = "final";

    // @appendable
    static constexpr const char* IDL_BUILTIN_ANN_APPENDABLE_TAG = "appendable";

    // @mutable
    static constexpr const char* IDL_BUILTIN_ANN_MUTABLE_TAG = "mutable";

    // @key
    static constexpr const char* IDL_BUILTIN_ANN_KEY_TAG = "key";

    // @must_understand
    static constexpr const char* IDL_BUILTIN_ANN_MUST_UNDERSTAND_TAG = "must_understand";

    // @default_literal
    static constexpr const char* IDL_BUILTIN_ANN_DEFAULT_LITERAL_TAG = "default_literal";

    // @default
    static constexpr const char* IDL_BUILTIN_ANN_DEFAULT_TAG = "default";

    // @range
    static constexpr const char* IDL_BUILTIN_ANN_RANGE_TAG = "range";
    static constexpr const char* IDL_BUILTIN_ANN_RANGE_MIN_TAG = "min";
    static constexpr const char* IDL_BUILTIN_ANN_RANGE_MAX_TAG = "max";

    // @min
    static constexpr const char* IDL_BUILTIN_ANN_MIN_TAG = "min";

    // @max
    static constexpr const char* IDL_BUILTIN_ANN_MAX_TAG = "max";

    // @unit
    static constexpr const char* IDL_BUILTIN_ANN_UNIT_TAG = "unit";

    // @bit_bound
    static constexpr const char* IDL_BUILTIN_ANN_BIT_BOUND_TAG = "bit_bound";

    // @external
    static constexpr const char* IDL_BUILTIN_ANN_EXTERNAL_TAG = "external";

    // @nested
    static constexpr const char* IDL_BUILTIN_ANN_NESTED_TAG = "nested";

    // @verbatim
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_TAG = "verbatim";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_TAG = "PlacementKind";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_BEGIN_FILE_TAG = "BEGIN_FILE";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_BEFORE_DECLARATION_TAG = "BEFORE_DECLARATION";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_BEGIN_DECLARATION_TAG = "BEGIN_DECLARATION";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_END_DECLARATION_TAG = "END_DECLARATION";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_AFTER_DECLARATION_TAG = "AFTER_DECLARATION";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_KIND_END_FILE_TAG = "END_FILE";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_LANGUAGE_TAG = "language";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_PLACEMENT_TAG = "placement";
    static constexpr const char* IDL_BUILTIN_ANN_VERBATIM_TEXT_TAG = "text";

    // @service
    static constexpr const char* IDL_BUILTIN_ANN_SERVICE_TAG = "service";
    static constexpr const char* IDL_BUILTIN_ANN_SERVICE_PLATFORM_TAG = "platform";

    //  @oneway
    static constexpr const char* IDL_BUILTIN_ANN_ONEWAY_TAG = "oneway";

    // @ami
    static constexpr const char* IDL_BUILTIN_ANN_AMI_TAG = "ami";

    // @try_construct
    static constexpr const char* IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG = "try_construct";
    static constexpr const char* IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG = "TryConstructFailAction";
    static constexpr const char* IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_DISCARD_TAG = "DISCARD";
    static constexpr const char* IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG = "USE_DEFAULT";
    static constexpr const char* IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TRIM_TAG = "TRIM";

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima
/* Built-in annotations related tags */

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERTAGS_HPP