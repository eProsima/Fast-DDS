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

#ifndef TYPES_MEMBER_DESCRIPTOR_H
#define TYPES_MEMBER_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;

class MemberDescriptor
{
protected:
    std::string name_;                  // Name of the member
    MemberId id_;                       // MemberId, it should be filled automatically when the member is added.
    DynamicType_ptr type_;              // Member's Type.
    std::string default_value_;         // Default value of the member in string.
    uint32_t index_;                    // Definition order of the member inside it's parent.
    std::vector<uint64_t> labels_;      // Case Labels for unions.
    bool default_label_;                 // TRUE if it's the default option of a union.

    friend class DynamicTypeBuilderFactory;
    friend class DynamicData;
    friend class DynamicTypeMember;
    friend class TypeObjectFactory;

    bool is_default_value_consistent(const std::string& sDefaultValue) const;

    bool is_type_name_consistent(const std::string& sName) const;

public:
    RTPS_DllAPI MemberDescriptor();

    RTPS_DllAPI MemberDescriptor(
            uint32_t index,
            const std::string& name);

    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_);

    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue);

    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    RTPS_DllAPI MemberDescriptor(const MemberDescriptor* descriptor);

    RTPS_DllAPI ~MemberDescriptor();

    bool check_union_labels(const std::vector<uint64_t>& labels) const;

    RTPS_DllAPI ResponseCode copy_from(const MemberDescriptor* other);

    RTPS_DllAPI bool equals(const MemberDescriptor* other) const;

    RTPS_DllAPI TypeKind get_kind() const;

    RTPS_DllAPI MemberId get_id() const;

    RTPS_DllAPI  uint32_t get_index() const;

    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI std::vector<uint64_t> get_union_labels() const;

    RTPS_DllAPI std::string get_default_value() const
    {
        return default_value_;
    }

    RTPS_DllAPI bool is_default_union_value() const;

    RTPS_DllAPI bool is_consistent(TypeKind parentKind) const;

    RTPS_DllAPI void add_union_case_index(uint64_t value);

    RTPS_DllAPI void set_id(MemberId id);

    RTPS_DllAPI void set_index(uint32_t index);

    RTPS_DllAPI void set_name(const std::string& name);

    RTPS_DllAPI void set_type(DynamicType_ptr type);

    RTPS_DllAPI void set_default_union_value(bool bDefault);

    RTPS_DllAPI void set_default_value(const std::string& value)
    {
        default_value_ = value;
    }
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
