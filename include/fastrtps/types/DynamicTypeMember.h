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

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class DynamicType;

class DynamicTypeMember
    : protected MemberDescriptor
{
    std::set<AnnotationDescriptor> annotation_; // Annotations to apply
    using annotation_iterator = std::set<AnnotationDescriptor>::iterator;

protected:

    friend class DynamicTypeBuilder;
    friend class DynamicType;
    friend class DynamicData;

    using MemberDescriptor::operator=;

    const MemberDescriptor& get_descriptor() const
    {
        return static_cast<const MemberDescriptor&>(*this);
    }

    std::pair<annotation_iterator, bool> get_annotation(const std::string& name) const;

    // Annotations application
    bool annotation_is_optional() const;

    bool annotation_is_key() const;

    bool annotation_is_must_understand() const;

    bool annotation_is_non_serialized() const;

    bool annotation_is_value() const;

    bool annotation_is_default_literal() const;

    bool annotation_is_position() const;

    bool annotation_is_bit_bound() const;

    // Annotations getters
    bool annotation_get_key() const;

    std::string annotation_get_value() const;

    std::string annotation_get_default() const;

    uint16_t annotation_get_position() const;

    uint16_t annotation_get_bit_bound() const;

    // Annotations setters

    //! auxiliary method for all bellow
    template<typename C, typename M>
    void annotation_set(const std::string& id, const C& c, const M& m);

    //! auxiliary method for all bellow
    void annotation_set(const std::string& id, const std::string& new_val);
    void annotation_set(const std::string& id, const char* new_val);

    void annotation_set_optional(bool optional);

    void annotation_set_key(bool key);

    void annotation_set_must_understand(bool must_understand);

    void annotation_set_non_serialized(bool non_serialized);

    void annotation_set_value(const std::string& value);

    void annotation_set_default(const std::string& default_value);

    void annotation_set_default_literal();

    void annotation_set_position(uint16_t position);

    void annotation_set_bit_bound(uint16_t bit_bound);

    RTPS_DllAPI ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);
public:

    using MemberDescriptor::get_kind;

    using MemberDescriptor::get_id;

    using MemberDescriptor::get_index;

    using MemberDescriptor::get_name;

    // TODO: doxygen
    RTPS_DllAPI std::string get_default_value() const;

    using MemberDescriptor::get_type;

    // TODO: doxygen
    RTPS_DllAPI MemberId get_annotation_count();

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            MemberId idx);

    bool operator==(const DynamicTypeMember& other) const;

    // TODO: doxygen
    RTPS_DllAPI bool equals(
            const DynamicTypeMember&) const;

    using MemberDescriptor::get_union_labels;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_descriptor(
            MemberDescriptor& descriptor) const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_MEMBER_H
