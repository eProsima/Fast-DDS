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

#ifndef TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_IDL_TYPES__ALL_TYPES_HPP
#define TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_IDL_TYPES__ALL_TYPES_HPP

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

#include "module_struct/gen/module_struct.hpp"
#include "module_struct/gen/module_structPubSubTypes.hpp"

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
const std::string MODULE_STRUCT{"module_struct"};
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
    SupportedTypes::MODULE_STRUCT,
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

    TypeSupport type_support;

    if (type_name == SupportedTypes::ALIAS_STRUCT)
    {
        type_support.reset(new AliasStructPubSubType());
    }
    else if (type_name == SupportedTypes::ARRAY_STRUCT)
    {
        type_support.reset(new ArrayStructPubSubType());
    }
    else if (type_name == SupportedTypes::BITMASK_STRUCT)
    {
        type_support.reset(new BitmaskStructPubSubType());
    }
    else if (type_name == SupportedTypes::BITSET_STRUCT)
    {
        type_support.reset(new BitsetStructPubSubType());
    }
    else if (type_name == SupportedTypes::ENUM_STRUCT)
    {
        type_support.reset(new EnumStructPubSubType());
    }
    else if (type_name == SupportedTypes::EXTENSIBILITY_STRUCT)
    {
        type_support.reset(new ExtensibilityStructPubSubType());
    }
    else if (type_name == SupportedTypes::KEY_STRUCT)
    {
        type_support.reset(new KeyStructPubSubType());
    }
    else if (type_name == SupportedTypes::MAP_STRUCT)
    {
        type_support.reset(new MapStructPubSubType());
    }
    else if (type_name == SupportedTypes::MODULE_STRUCT)
    {
        type_support.reset(new ModuleStructPubSubType());
    }
    else if (type_name == SupportedTypes::PRIMITIVE_STRUCT)
    {
        type_support.reset(new PrimitivesStructPubSubType());
    }
    else if (type_name == SupportedTypes::SEQUENCE_STRUCT)
    {
        type_support.reset(new SequenceStructPubSubType());
    }
    else if (type_name == SupportedTypes::STRING_STRUCT)
    {
        type_support.reset(new StringStructPubSubType());
    }
    else if (type_name == SupportedTypes::STRUCT_STRUCT)
    {
        type_support.reset(new StructStructPubSubType());
    }
    else if (type_name == SupportedTypes::UNION_STRUCT)
    {
        type_support.reset(new UnionStructPubSubType());
    }
    else
    {
        ASSERT_FALSE(true) << "Type not supported";
    }

    type_support->register_type_object_representation();
}

} /* namespace test */

#endif /* TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_IDL_TYPES__ALL_TYPES_HPP */
