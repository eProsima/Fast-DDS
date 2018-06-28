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

class MemberDescriptor;
class DynamicType;

namespace eprosima{
namespace fastrtps{
namespace types{

class TypeDescriptor
{
public:
    TypeDescriptor();
    TypeDescriptor(const TypeDescriptor* other);

    ~TypeDescriptor();

	ResponseCode copy_from(const TypeDescriptor* descriptor);

	bool equals(const TypeDescriptor* descriptor) const;

	bool isConsistent() const;

    std::string getName() const;
    TypeKind getKind() const;

protected:

    friend class DynamicTypeBuilderFactory;

	TypeKind mKind;
	std::string mName;
	DynamicType* mBaseType;
	DynamicType* mDiscriminatorType;
	std::vector<uint32_t> mBound;
	DynamicType* mElementType;
	DynamicType* mKeyElementType;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
