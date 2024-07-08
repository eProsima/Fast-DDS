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

#include "alias_struct/gen/alias_struct.hpp"
#include "alias_struct/gen/alias_structPubSubTypes.hpp"

#include "array_struct/gen/array_struct.hpp"
#include "array_struct/gen/array_structPubSubTypes.hpp"

#include "bitmask_struct/gen/bitmask_struct.hpp"
#include "bitmask_struct/gen/bitmask_structPubSubTypes.hpp"

#include "bitset_struct/gen/bitset_struct.hpp"
#include "bitset_struct/gen/bitset_structPubSubTypes.hpp"

#include "enum_struct/gen/enum_struct.hpp"
#include "enum_struct/gen/enum_structPubSubTypes.hpp"

#include "extensibility_struct/gen/extensibility_struct.hpp"
#include "extensibility_struct/gen/extensibility_structPubSubTypes.hpp"

#include "key_struct/gen/key_struct.hpp"
#include "key_struct/gen/key_structPubSubTypes.hpp"

#include "map_struct/gen/map_struct.hpp"
#include "map_struct/gen/map_structPubSubTypes.hpp"

#include "primitives_struct/gen/primitives_struct.hpp"
#include "primitives_struct/gen/primitives_structPubSubTypes.hpp"

#include "sequence_struct/gen/sequence_struct.hpp"
#include "sequence_struct/gen/sequence_structPubSubTypes.hpp"

#include "string_struct/gen/string_struct.hpp"
#include "string_struct/gen/string_structPubSubTypes.hpp"

#include "struct_struct/gen/struct_struct.hpp"
#include "struct_struct/gen/struct_structPubSubTypes.hpp"

#include "union_struct/gen/union_struct.hpp"
#include "union_struct/gen/union_structPubSubTypes.hpp"

namespace test {

namespace SupportedTypes {

const std::string ALIAS_STRUCT{"alias_struct"};
const std::string ARRAY_STRUCT{"array_struct"};
const std::string BITMASK_STRUCT{"bitmask_struct"};
const std::string BITSET_STRUCT{"bitset_struct"};
const std::string ENUM_STRUCT{"enum_struct"};
const std::string EXTENSIBILITY_STRUCT{"extensibility_struct"};
const std::string KEY_STRUCT{"key_struct"};
const std::string MAP_STRUCT{"map_struct"};
const std::string PRIMITIVE_STRUCT{"primitives_struct"};
const std::string SEQUENCE_STRUCT{"sequence_struct"};
const std::string STRING_STRUCT{"string_struct"};
const std::string STRUCT_STRUCT{"struct_struct"};
const std::string UNION_STRUCT{"union_struct"};

} // namespace SupportedTypes

const std::vector<std::string> supported_types = {
    SupportedTypes::ALIAS_STRUCT,
    SupportedTypes::ARRAY_STRUCT,
    SupportedTypes::BITMASK_STRUCT,
    SupportedTypes::BITSET_STRUCT,
    SupportedTypes::ENUM_STRUCT,
    SupportedTypes::EXTENSIBILITY_STRUCT,
    SupportedTypes::KEY_STRUCT,
    SupportedTypes::MAP_STRUCT,
    SupportedTypes::PRIMITIVE_STRUCT,
    SupportedTypes::SEQUENCE_STRUCT,
    SupportedTypes::STRING_STRUCT,
    SupportedTypes::STRUCT_STRUCT,
    SupportedTypes::UNION_STRUCT
};

void register_type_object_representation(
        const std::string& type_name)
{
    using namespace eprosima::fastdds::dds;

    if (type_name == SupportedTypes::ALIAS_STRUCT)
    {
        TypeSupport type_alias_struct(new AliasStructPubSubType());
        type_alias_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::ARRAY_STRUCT)
    {
        TypeSupport type_array_struct(new ArrayStructPubSubType());
        type_array_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BITMASK_STRUCT)
    {
        TypeSupport type_bitmask_struct(new BitmaskStructPubSubType());
        type_bitmask_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::BITSET_STRUCT)
    {
        TypeSupport type_bitset_struct(new BitsetStructPubSubType());
        type_bitset_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::ENUM_STRUCT)
    {
        TypeSupport type_enum_struct(new EnumStructPubSubType());
        type_enum_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::EXTENSIBILITY_STRUCT)
    {
        TypeSupport type_extensibility_struct(new ExtensibilityStructPubSubType());
        type_extensibility_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::KEY_STRUCT)
    {
        TypeSupport type_key_struct(new KeyStructPubSubType());
        type_key_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::MAP_STRUCT)
    {
        TypeSupport type_map_struct(new MapStructPubSubType());
        type_map_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::PRIMITIVE_STRUCT)
    {
        TypeSupport type_primitives_struct(new PrimitivesStructPubSubType());
        type_primitives_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::SEQUENCE_STRUCT)
    {
        TypeSupport type_sequence_struct(new SequenceStructPubSubType());
        type_sequence_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::STRING_STRUCT)
    {
        TypeSupport type_string_struct(new StringStructPubSubType());
        type_string_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::STRUCT_STRUCT)
    {
        TypeSupport type_struct_struct(new StructStructPubSubType());
        type_struct_struct->register_type_object_representation();
    }
    else if (type_name == SupportedTypes::UNION_STRUCT)
    {
        TypeSupport type_union_struct(new UnionStructPubSubType());
        type_union_struct->register_type_object_representation();
    }
    else
    {
        ASSERT_FALSE(true) << "Type not supported";
    }
}

} /* namespace test */
