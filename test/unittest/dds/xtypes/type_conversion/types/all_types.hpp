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

/**
 * @file all_types.hpp
 */

#include <string>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include "type_objects/alias_struct/alias_struct.hpp"
#include "type_objects/alias_struct/alias_structPubSubTypes.h"

#include "type_objects/arrays_and_sequences/arrays_and_sequences.hpp"
#include "type_objects/arrays_and_sequences/arrays_and_sequencesPubSubTypes.h"

#include "type_objects/basic_array_struct/basic_array_struct.hpp"
#include "type_objects/basic_array_struct/basic_array_structPubSubTypes.h"

#include "type_objects/basic_struct/basic_struct.hpp"
#include "type_objects/basic_struct/basic_structPubSubTypes.h"

#include "type_objects/bitmask_struct/bitmask_struct.hpp"
#include "type_objects/bitmask_struct/bitmask_structPubSubTypes.h"

#include "type_objects/bitset_struct/bitset_struct.hpp"
#include "type_objects/bitset_struct/bitset_structPubSubTypes.h"

#include "type_objects/char_sequence/char_sequence.hpp"
#include "type_objects/char_sequence/char_sequencePubSubTypes.h"

#include "type_objects/complex_nested_arrays/complex_nested_arrays.hpp"
#include "type_objects/complex_nested_arrays/complex_nested_arraysPubSubTypes.h"

#include "type_objects/enum_struct/enum_struct.hpp"
#include "type_objects/enum_struct/enum_structPubSubTypes.h"

#include "type_objects/float_bounded_sequence/float_bounded_sequence.hpp"
#include "type_objects/float_bounded_sequence/float_bounded_sequencePubSubTypes.h"

#include "type_objects/hello_world/hello_world.hpp"
#include "type_objects/hello_world/hello_worldPubSubTypes.h"

#include "type_objects/key_struct/key_struct.hpp"
#include "type_objects/key_struct/key_structPubSubTypes.h"

#include "type_objects/map_struct/map_struct.hpp"
#include "type_objects/map_struct/map_structPubSubTypes.h"

#include "type_objects/numeric_array/numeric_array.hpp"
#include "type_objects/numeric_array/numeric_arrayPubSubTypes.h"

#include "type_objects/union_struct/union_struct.hpp"
#include "type_objects/union_struct/union_structPubSubTypes.h"

namespace test {

namespace SupportedTypes {

const std::string ALIAS_STRUCT{"alias_struct"};
const std::string ARRAYS_AND_SEQUENCES{"arrays_and_sequences"};
const std::string BASIC_ARRAY_STRUCT{"basic_array_struct"};
const std::string BASIC_STRUCT{"basic_struct"};
const std::string BITMASK_STRUCT{"bitmask_struct"};
const std::string BITSET_STRUCT{"bitset_struct"};
const std::string CHAR_SEQUENCE{"char_sequence"};
const std::string COMPLEX_NESTED_ARRAYS{"complex_nested_arrays"};
const std::string ENUM_STRUCT{"enum_struct"};
const std::string FLOAT_BOUNDED_SEQUENCE{"float_bounded_sequence"};
const std::string HELLO_WORLD{"hello_world"};
const std::string KEY_STRUCT{"key_struct"};
const std::string MAP_STRUCT{"map_struct"};
const std::string NUMERIC_ARRAY{"numeric_array"};
const std::string UNION_STRUCT{"union_struct"};

} // namespace SupportedTypes

const std::vector<std::string> supported_types = {
    SupportedTypes::ALIAS_STRUCT,
    SupportedTypes::ARRAYS_AND_SEQUENCES,
    SupportedTypes::BASIC_ARRAY_STRUCT,
    SupportedTypes::BASIC_STRUCT,
    SupportedTypes::BITMASK_STRUCT,
    SupportedTypes::BITSET_STRUCT,
    SupportedTypes::CHAR_SEQUENCE,
    SupportedTypes::COMPLEX_NESTED_ARRAYS,
    SupportedTypes::ENUM_STRUCT,
    SupportedTypes::FLOAT_BOUNDED_SEQUENCE,
    SupportedTypes::HELLO_WORLD,
    SupportedTypes::KEY_STRUCT,
    SupportedTypes::MAP_STRUCT,
    SupportedTypes::NUMERIC_ARRAY,
    SupportedTypes::UNION_STRUCT
};

void register_dynamic_types()
{
    using namespace eprosima::fastdds::dds;

    TypeSupport type_alias_struct(new alias_structPubSubType());
    type_alias_struct->register_type_object_representation();

    TypeSupport type_arrays_and_sequences(new arrays_and_sequencesPubSubType());
    type_arrays_and_sequences->register_type_object_representation();

    TypeSupport type_basic_array_struct(new basic_array_structPubSubType());
    type_basic_array_struct->register_type_object_representation();

    TypeSupport type_basic_struct(new basic_structPubSubType());
    type_basic_struct->register_type_object_representation();

    TypeSupport type_bitmask_struct(new bitmask_structPubSubType());
    type_bitmask_struct->register_type_object_representation();

    TypeSupport type_bitset_struct(new bitset_structPubSubType());
    type_bitset_struct->register_type_object_representation();

    TypeSupport type_char_sequence(new char_sequencePubSubType());
    type_char_sequence->register_type_object_representation();

    TypeSupport type_complex_nested_arrays(new complex_nested_arraysPubSubType());
    type_complex_nested_arrays->register_type_object_representation();

    TypeSupport type_enum_struct(new enum_structPubSubType());
    type_enum_struct->register_type_object_representation();

    TypeSupport type_float_bounded_sequence(new float_bounded_sequencePubSubType());
    type_float_bounded_sequence->register_type_object_representation();

    TypeSupport type_hello_world(new hello_worldPubSubType());
    type_hello_world->register_type_object_representation();

    TypeSupport type_key_struct(new key_structPubSubType());
    type_key_struct->register_type_object_representation();

    TypeSupport type_map_struct(new map_structPubSubType());
    type_map_struct->register_type_object_representation();

    TypeSupport type_numeric_array(new numeric_arrayPubSubType());
    type_numeric_array->register_type_object_representation();

    TypeSupport type_union_struct(new union_structPubSubType());
    type_union_struct->register_type_object_representation();
}

} /* namespace test */
