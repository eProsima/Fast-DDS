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

#include "gen/alias_struct/alias_struct.hpp"
#include "gen/alias_struct/alias_structPubSubTypes.hpp"

#include "gen/arrays_and_sequences/arrays_and_sequences.hpp"
#include "gen/arrays_and_sequences/arrays_and_sequencesPubSubTypes.hpp"

#include "gen/basic_array_struct/basic_array_struct.hpp"
#include "gen/basic_array_struct/basic_array_structPubSubTypes.hpp"

#include "gen/basic_struct/basic_struct.hpp"
#include "gen/basic_struct/basic_structPubSubTypes.hpp"

#include "gen/bitmask_struct/bitmask_struct.hpp"
#include "gen/bitmask_struct/bitmask_structPubSubTypes.hpp"

#include "gen/bitset_struct/bitset_struct.hpp"
#include "gen/bitset_struct/bitset_structPubSubTypes.hpp"

#include "gen/char_sequence/char_sequence.hpp"
#include "gen/char_sequence/char_sequencePubSubTypes.hpp"

#include "gen/complex_nested_arrays/complex_nested_arrays.hpp"
#include "gen/complex_nested_arrays/complex_nested_arraysPubSubTypes.hpp"

#include "gen/enum_struct/enum_struct.hpp"
#include "gen/enum_struct/enum_structPubSubTypes.hpp"

#include "gen/extensibility_struct/extensibility_struct.hpp"
#include "gen/extensibility_struct/extensibility_structPubSubTypes.hpp"

#include "gen/float_bounded_sequence/float_bounded_sequence.hpp"
#include "gen/float_bounded_sequence/float_bounded_sequencePubSubTypes.hpp"

#include "gen/hello_world/hello_world.hpp"
#include "gen/hello_world/hello_worldPubSubTypes.hpp"

#include "gen/inheritance_struct/inheritance_struct.hpp"
#include "gen/inheritance_struct/inheritance_structPubSubTypes.hpp"

#include "gen/key_struct/key_struct.hpp"
#include "gen/key_struct/key_structPubSubTypes.hpp"

#include "gen/map_bounded_struct/map_bounded_struct.hpp"
#include "gen/map_bounded_struct/map_bounded_structPubSubTypes.hpp"

#include "gen/map_struct/map_struct.hpp"
#include "gen/map_struct/map_structPubSubTypes.hpp"

#include "gen/multi_array_struct/multi_array_struct.hpp"
#include "gen/multi_array_struct/multi_array_structPubSubTypes.hpp"

#include "gen/nested_struct/nested_struct.hpp"
#include "gen/nested_struct/nested_structPubSubTypes.hpp"

#include "gen/numeric_array/numeric_array.hpp"
#include "gen/numeric_array/numeric_arrayPubSubTypes.hpp"

#include "gen/string_struct/string_struct.hpp"
#include "gen/string_struct/string_structPubSubTypes.hpp"

#include "gen/union_struct/union_struct.hpp"
#include "gen/union_struct/union_structPubSubTypes.hpp"

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
const std::string EXTENSIBILITY_STRUCT{"extensibility_struct"};
const std::string FLOAT_BOUNDED_SEQUENCE{"float_bounded_sequence"};
const std::string HELLO_WORLD{"hello_world"};
const std::string INHERITANCE_STRUCT{"inheritance_struct"};
const std::string KEY_STRUCT{"key_struct"};
const std::string MAP_BOUNDED_STRUCT{"map_bounded_struct"};
const std::string MAP_STRUCT{"map_struct"};
const std::string MULTI_ARRAY_STRUCT{"multi_array_struct"};
const std::string NESTED_STRUCT{"nested_struct"};
const std::string NUMERIC_ARRAY{"numeric_array"};
const std::string STRING_STRUCT{"string_struct"};
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
    SupportedTypes::EXTENSIBILITY_STRUCT,
    SupportedTypes::FLOAT_BOUNDED_SEQUENCE,
    SupportedTypes::HELLO_WORLD,
    SupportedTypes::INHERITANCE_STRUCT,
    SupportedTypes::KEY_STRUCT,
    SupportedTypes::MAP_BOUNDED_STRUCT,
    SupportedTypes::MAP_STRUCT,
    SupportedTypes::MULTI_ARRAY_STRUCT,
    SupportedTypes::NESTED_STRUCT,
    SupportedTypes::NUMERIC_ARRAY,
    SupportedTypes::STRING_STRUCT,
    SupportedTypes::UNION_STRUCT
};

void register_type_object_representation(
        const std::string& type_name)
{
    using namespace eprosima::fastdds::dds;

    if (type_name == SupportedTypes::ALIAS_STRUCT)
    {
        TypeSupport type_alias_struct(new alias_structPubSubType());
        type_alias_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::ARRAYS_AND_SEQUENCES)
    {
        TypeSupport type_arrays_and_sequences(new arrays_and_sequencesPubSubType());
        type_arrays_and_sequences->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BASIC_ARRAY_STRUCT)
    {
        TypeSupport type_basic_array_struct(new basic_array_structPubSubType());
        type_basic_array_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BASIC_STRUCT)
    {
        TypeSupport type_basic_struct(new basic_structPubSubType());
        type_basic_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BITMASK_STRUCT)
    {
        TypeSupport type_bitmask_struct(new bitmask_structPubSubType());
        type_bitmask_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BITSET_STRUCT)
    {
        TypeSupport type_bitset_struct(new bitset_structPubSubType());
        type_bitset_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::CHAR_SEQUENCE)
    {
        TypeSupport type_char_sequence(new char_sequencePubSubType());
        type_char_sequence->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::COMPLEX_NESTED_ARRAYS)
    {
        TypeSupport type_complex_nested_arrays(new complex_nested_arraysPubSubType());
        type_complex_nested_arrays->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::ENUM_STRUCT)
    {
        TypeSupport type_enum_struct(new enum_structPubSubType());
        type_enum_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::EXTENSIBILITY_STRUCT)
    {
        TypeSupport type_extensibility_struct(new extensibility_structPubSubType());
        type_extensibility_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::FLOAT_BOUNDED_SEQUENCE)
    {
        TypeSupport type_float_bounded_sequence(new float_bounded_sequencePubSubType());
        type_float_bounded_sequence->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::HELLO_WORLD)
    {
        TypeSupport type_hello_world(new hello_worldPubSubType());
        type_hello_world->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::INHERITANCE_STRUCT)
    {
        TypeSupport type_inheritance_struct(new inheritance_structPubSubType());
        type_inheritance_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::KEY_STRUCT)
    {
        TypeSupport type_key_struct(new key_structPubSubType());
        type_key_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::MAP_BOUNDED_STRUCT)
    {
        TypeSupport type_map_bounded_struct(new map_bounded_structPubSubType());
        type_map_bounded_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::MAP_STRUCT)
    {
        TypeSupport type_map_struct(new map_structPubSubType());
        type_map_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::MULTI_ARRAY_STRUCT)
    {
        TypeSupport type_multi_array_struct(new multi_array_structPubSubType());
        type_multi_array_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::NESTED_STRUCT)
    {
        TypeSupport type_nested_struct(new nested_structPubSubType());
        type_nested_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::NUMERIC_ARRAY)
    {
        TypeSupport type_numeric_array(new numeric_arrayPubSubType());
        type_numeric_array->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::STRING_STRUCT)
    {
        TypeSupport type_string_struct(new string_structPubSubType());
        type_string_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::UNION_STRUCT)
    {
        TypeSupport type_union_struct(new union_structPubSubType());
        type_union_struct->register_type_object_representation();
    }
    else
    {
        ASSERT_FALSE(true) << "Type not supported";
    }
}

} /* namespace test */
