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

#ifndef TYPES_DYNAMIC_TYPE_MEMBER_H
#define TYPES_DYNAMIC_TYPE_MEMBER_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/AnnotationManager.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class DynamicType;

class DynamicTypeMember
    : public MemberDescriptor
    , protected AnnotationManager
{
protected:

    friend class DynamicTypeBuilder;
    friend class DynamicType;
    friend class DynamicData;

    using MemberDescriptor::MemberDescriptor;
    using MemberDescriptor::operator=;

public:

    using AnnotationManager::get_all_annotations;
    using AnnotationManager::get_annotation_count;
    using AnnotationManager::get_annotation;

    using AnnotationManager::annotation_is_bit_bound;
    using AnnotationManager::annotation_is_key;
    using AnnotationManager::annotation_is_non_serialized;

    using AnnotationManager::annotation_is_appendable;
    using AnnotationManager::key_annotation;
    using AnnotationManager::annotation_is_default_literal;
    using AnnotationManager::annotation_is_optional;
    using AnnotationManager::annotation_is_must_understand;
    using AnnotationManager::annotation_get_position;
    using AnnotationManager::annotation_get_bit_bound;

public:

    using MemberDescriptor::operator==;
    using MemberDescriptor::operator!=;

    const MemberDescriptor& get_descriptor() const
    {
        return static_cast<const MemberDescriptor&>(*this);
    }

    // TODO: doxygen
    RTPS_DllAPI std::string get_default_value() const;

    bool operator==(const DynamicTypeMember& other) const;

    // TODO: doxygen
    RTPS_DllAPI bool equals(
            const DynamicTypeMember&) const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_descriptor(
            MemberDescriptor& descriptor) const;

    //! more accurate that base class because it has access to annotations
    bool is_consistent(TypeKind parentKind) const;
};

RTPS_DllAPI std::ostream& operator<<( std::ostream& os, const DynamicTypeMember& dm);

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_MEMBER_H
