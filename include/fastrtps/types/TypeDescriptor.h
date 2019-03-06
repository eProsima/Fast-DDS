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

#ifndef TYPES_TYPE_DESCRIPTOR_H
#define TYPES_TYPE_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypePtr.h>

class MemberDescriptor;
class DynamicType;

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeDescriptor
{
protected:
    TypeKind kind_;                         // Type Kind.
    std::string name_;                      // Type Name.
    DynamicType_ptr base_type_;             // SuperType of an structure or base type of an alias type.
    DynamicType_ptr discriminator_type_;    // Discrimination type for a union.
    std::vector<uint32_t> bound_;           // Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr element_type_;          // Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr key_element_type_;      // Key Type for maps.

    void clean();

    bool is_type_name_consistent(const std::string& sName) const;

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;

public:
    TypeDescriptor();

    TypeDescriptor(const TypeDescriptor* other);

    TypeDescriptor(
            const std::string& name,
            TypeKind kind);

    ~TypeDescriptor();

    ResponseCode copy_from(const TypeDescriptor* descriptor);

    bool equals(const TypeDescriptor* descriptor) const;

    bool is_consistent() const;

    DynamicType_ptr get_base_type() const;

    uint32_t get_bounds(uint32_t index = 0) const;

    uint32_t get_bounds_size() const;

    DynamicType_ptr get_discriminator_type() const;

    DynamicType_ptr get_element_type() const;

    DynamicType_ptr get_key_element_type() const;

    TypeKind get_kind() const;

    std::string get_name() const;

    uint32_t get_total_bounds() const;

    void set_kind(TypeKind kind);

    void set_name(std::string name);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
