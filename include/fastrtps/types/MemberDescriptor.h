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

#include <set>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;
class AnnotationDescriptor;


/**
 * This class packages together the state of a @ref DynamicTypeMember.
 * @remark This class has value semantics, allowing it to be deeply copied and compared.
 */
class MemberDescriptor
{
protected:
    std::string name_;                  // Name of the member
    MemberId id_ = MEMBER_ID_INVALID;   // MemberId, it should be filled automatically when the member is added.
    DynamicType_ptr type_;              // Member's Type.
    std::string default_value_;         // Default value of the member in string.
    uint32_t index_ = INDEX_INVALID;    // Definition order of the member inside it's parent.
    std::set<uint64_t> labels_;         // Case Labels for unions.
    bool default_label_ = false;        // TRUE if it's the default option of a union.

    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;
    friend class DynamicData;
    friend class DynamicTypeMember;
    friend class TypeObjectFactory;

    bool is_default_value_consistent(const std::string& sDefaultValue) const;

    bool is_type_name_consistent(const std::string& sName) const;

public:

    //! Default constructor
    RTPS_DllAPI MemberDescriptor() = default;

    //! Default copy constructor
    RTPS_DllAPI MemberDescriptor(const MemberDescriptor& descriptor) = default;

    //! Default move constructor
    RTPS_DllAPI MemberDescriptor(MemberDescriptor&& descriptor) = default;

    /**
     * convenience constructor
     * @param[in] name std::string new member's name
     */
    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name);

    /**
     * convenience constructor
     * @param[in] index desired position in the collection (zero based)
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name std::string new member's name
     */
    RTPS_DllAPI MemberDescriptor(
            uint32_t index,
            MemberId id,
            const std::string& name);

    /**
     * convenience constructor
     * @param[in] index desired position in the collection (zero based)
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name std::string new member's name
     * @param[in] type @ref DynamicType new member's type
     */
    RTPS_DllAPI MemberDescriptor(
            uint32_t index,
            MemberId id,
            const std::string& name,
            DynamicType_ptr type);

    /**
     * convenience constructor
     * @remark Default index value assures it is appended to the collection.
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name std::string new member's name
     * @param[in] type @ref DynamicType new member's type
     */
    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type);

    /**
     * convenience constructor
     * @remark Default index value assures it is appended to the collection.
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name std::string new member's name
     * @param[in] type @ref DynamicType new member's type
     * @param[in] defaultValue std::string member default value as a string representation
     */
    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type,
            const std::string& defaultValue);

    // TODO:Barro doxygen
    RTPS_DllAPI MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    /**
     * Default copy assignment
     * @remark Note that the no member uses mutable references, thus the default
     *         copy operator provides an actual deep copy.
     * @param[in] descriptor l-value @ref MemberDescriptor reference to copy from
     * @result own @ref MemberDescriptor reference
     */
    RTPS_DllAPI MemberDescriptor& operator=(const MemberDescriptor& descriptor) = default;

    //! Default move assignment
    RTPS_DllAPI MemberDescriptor& operator=(MemberDescriptor&& descriptor) = default;

    RTPS_DllAPI ~MemberDescriptor() = default;

    bool check_union_labels(const std::vector<uint64_t>& labels) const;

    /**
     * Overwrite the contents of this descriptor with those of another descriptor
     * @remark subsequent calls to equals, passing the same argument as to this method, return true
     * @remark The other descriptor shall not be changed by this operation
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t copy_from(const MemberDescriptor& other);

    bool operator==(const MemberDescriptor& other) const;

    bool operator!=(const MemberDescriptor& other) const;

    /**
     * State comparisson
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] other @ref MemberDescriptor object whose state to compare to
     * @return \b bool `true` on equality
     */
    RTPS_DllAPI bool equals(const MemberDescriptor& other) const;

    RTPS_DllAPI TypeKind get_kind() const;

    /**
     * Queries the desired or actual member id
     * @return MemberId id
     */
    RTPS_DllAPI MemberId get_id() const;

    /**
     * Queries the desired or actual member position in the collection.
     * @return uint32_t position
     */
    RTPS_DllAPI  uint32_t get_index() const;

    /**
     * Queries the desired or actual member name
     * @return std::string name
     */
    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI std::vector<uint64_t> get_union_labels() const;

    // TODO: doxygen
    RTPS_DllAPI std::string get_default_value() const;

    RTPS_DllAPI bool is_default_union_value() const;

    /**
     * Tests state consistency
     * @remark A MemberDescriptor shall be considered consistent if and only if all of the values
     *         of its properties are considered consistent with its collection owner
     * @param[in] parentKind @ref TypeKind collection's owner kind
     * @return bool `true` if consistent
     */
    RTPS_DllAPI bool is_consistent(TypeKind parentKind) const;

    RTPS_DllAPI void add_union_case_index(uint64_t value);

    /**
     * Set member @ref MemberId
     * @param[in] id desired MemberId
     */
    RTPS_DllAPI void set_id(MemberId id);

    /**
     * Set member index
     * @remark Only modifiable for elements detach from any collection
     * @remark `DynamicTypeBuilder::add_member(...)` methods will query it in order
     *         to position this element in their member collection. According with
     *         [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.7.6
     * @param[in] index uint32_t desired position
     */
    RTPS_DllAPI void set_index(uint32_t index);

    /**
     * Set member name
     * @param[in] name std::string desired name
     */
    RTPS_DllAPI void set_name(const std::string& name);

    /**
     * Set member type by move
     * @param[in] type @ref DynamicType r-value
     */
    RTPS_DllAPI void set_type(DynamicType_ptr&& type);

    /**
     * Set member type by copy
     * @param[in] type @ref DynamicType l-value
     */
    RTPS_DllAPI void set_type(const DynamicType_ptr& type);

    /**
     * Queries the desired or actual member type
     * @return @ref DynamicType
     */
    RTPS_DllAPI DynamicType_ptr get_type() const;

    RTPS_DllAPI void set_default_union_value(bool bDefault);

    // TODO: doxygen
    RTPS_DllAPI void set_default_value(const std::string& value)
    {
        default_value_ = value;
    }

    // TODO: getters and setters for labels & default_label
};

std::ostream& operator<<( std::ostream& os, const MemberDescriptor & md);

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
