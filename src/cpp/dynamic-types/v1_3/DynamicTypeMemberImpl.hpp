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

#ifndef TYPES_1_3_DYNAMIC_TYPE_MEMBER_IMPL_H
#define TYPES_1_3_DYNAMIC_TYPE_MEMBER_IMPL_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/DynamicTypeMember.hpp>
#include <dynamic-types/v1_3/MemberDescriptorImpl.hpp>
#include <dynamic-types/v1_3/AnnotationManager.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicTypeImpl;
class MemberDescriptor;

class DynamicTypeMemberImpl final
    : public MemberDescriptorImpl
    , protected AnnotationManager
{
    DynamicTypeMember interface_;

    friend class DynamicData;

public:

    using AnnotationManager::apply_annotation;
    using AnnotationManager::get_all_annotations;
    using AnnotationManager::get_annotation_count;
    using AnnotationManager::get_annotations;
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
    using AnnotationManager::annotation_set_position;
    using AnnotationManager::annotation_get_bit_bound;

public:

    using MemberDescriptorImpl::MemberDescriptorImpl;
    using MemberDescriptorImpl::operator =;

    using MemberDescriptorImpl::operator ==;
    using MemberDescriptorImpl::operator !=;

    using MemberDescriptorImpl::get_descriptor;

    const DynamicTypeMember& get_interface() const
    {
       return interface_;
    }

    /**
     * Getter for \b default_value property (see [standard] section 7.5.2.7.3)
     * @return std::string
     * @remarks fallbacks to any value kept as an annotation
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::string get_default_value() const;

    /**
     * equality operator following [standard] section 7.5.2.7.4 guidelines
     * @param[in] other @ref DynamicTypeMemberImpl reference to compare to
     * @return `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    bool operator ==(
            const DynamicTypeMemberImpl& other) const;

    /**
     * This operation provides a summary of the state of this type (see [standard] section 7.5.2.8.7 guidelines)
     * @param[out] descriptor @ref MemberDescriptor to populate
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t get_descriptor(
            MemberDescriptor& descriptor) const;

    //! more accurate that base class because it has access to annotations
    bool is_consistent(
            TypeKind parentKind) const;

    //! get implementation from public interface
    static const DynamicTypeMemberImpl& get_implementation(const DynamicTypeMember& t)
    {
        return *(DynamicTypeMemberImpl*)((const char*)&t -
                (::size_t)&reinterpret_cast<char const volatile&>((((DynamicTypeMemberImpl*)0)->interface_)));
    }
};

//! @ref DynamicTypeMemberImpl expected `std::ostream` non-member override of `operator<<`
std::ostream& operator <<(
        std::ostream& os,
        const DynamicTypeMemberImpl& dm);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_MEMBER_IMPL_H
