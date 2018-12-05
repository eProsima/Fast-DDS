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
public:
    TypeDescriptor();
    TypeDescriptor(const TypeDescriptor* other);
    TypeDescriptor(const std::string& name, TypeKind kind);

    ~TypeDescriptor();

    ResponseCode CopyFrom(const TypeDescriptor* descriptor);
    bool Equals(const TypeDescriptor* descriptor) const;
    bool IsConsistent() const;

    DynamicType_ptr GetBaseType() const;
    uint32_t GetBounds(uint32_t index = 0) const;
    uint32_t GetBoundsSize() const;
    DynamicType_ptr GetDiscriminatorType() const;
    DynamicType_ptr GetElementType() const;
    DynamicType_ptr GetKeyElementType() const;
    TypeKind GetKind() const;
    std::string GetName() const;
    uint32_t GetTotalBounds() const;

    void SetKind(TypeKind kind);
    void SetName(std::string name);

protected:

    void Clean();

    bool IsTypeNameConsistent(const std::string& sName) const;

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;

    TypeKind mKind;                         // Type Kind.
    std::string mName;                      // Type Name.
    DynamicType_ptr mBaseType;                 // SuperType of an structure or base type of an alias type.
    DynamicType_ptr mDiscriminatorType;        // Discrimination type for a union.
    std::vector<uint32_t> mBound;           // Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr mElementType;              // Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr mKeyElementType;           // Key Type for maps.
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
