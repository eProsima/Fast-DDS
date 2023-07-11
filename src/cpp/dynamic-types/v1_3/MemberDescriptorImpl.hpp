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

#ifndef TYPES_1_3_MEMBER_DESCRIPTOR_IMPL_H
#define TYPES_1_3_MEMBER_DESCRIPTOR_IMPL_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/MemberId.hpp>

#include <set>
#include <type_traits>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeObjectFactory;

namespace v1_3 {

class MemberDescriptor;
class AnnotationDescriptorImpl;
class DynamicTypeImpl;

using types::TypeKind;

/**
 * This class packages together the state of a DynamicTypeMemberImpl.
 * @remark This class has value semantics, allowing it to be deeply copied and compared.
 */
class MemberDescriptorImpl
{
protected:

    std::string name_;                              // Name of the member
    MemberId id_;                                   // MemberId, it should be filled automatically when the member is added.
    std::shared_ptr<const DynamicTypeImpl> type_;   // Member's Type.
    std::string default_value_;                     // Default value of the member in string.
    uint32_t index_ = INDEX_INVALID;                // Definition order of the member inside it's parent.
    std::set<uint64_t> labels_;                     // Case Labels for unions.
    bool default_label_ = false;                    // TRUE if it's the default option of a union.

    friend class DynamicTypeBuilderImpl;
    friend class DynamicTypeBuilderFactoryImpl;
    friend class DynamicData;
    friend class DynamicTypeMemberImpl;
    friend class types::TypeObjectFactory;

    bool is_default_value_consistent(
            const std::string& sDefaultValue) const;

    bool is_type_name_consistent(
            const std::string& sName) const;

    //! builtin cast inherited on subclasses
    MemberDescriptor get_descriptor() const;

public:

    //! Default constructor
    MemberDescriptorImpl() = default;

    //! Default copy constructor
    MemberDescriptorImpl(
            const MemberDescriptorImpl& descriptor) = default;

    //! Default move constructor
    MemberDescriptorImpl(
            MemberDescriptorImpl&& descriptor) = default;

    //! build from interface
    MemberDescriptorImpl(
            const MemberDescriptor& descriptor);

    /**
     * convenience constructor
     * @param[in] id MemberId new member's identifier
     * @param[in] name new member's name
     */
    template<class S,
             typename std::enable_if<
                 std::is_constructible<std::string, S>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            const MemberId& id,
            const S& name)
        : name_(name)
        , id_(id)
    {}

    /**
     * convenience constructor
     * @param[in] index desired position in the collection (zero based)
     * @param[in] name new member's name
     */
    template<class S,
             typename std::enable_if<
                 std::is_constructible<std::string, S>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            uint32_t index,
            const S& name)
        : name_(name)
        , index_(index)
    {
    }

    /**
     * convenience constructor
     * @param[in] index desired position in the collection (zero based)
     * @param[in] name new member's name
     * @param[in] type @ref DynamicType new member's type
     */
    template<class S,
             class D,
             typename std::enable_if<
                 std::is_constructible<std::string, S>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            uint32_t index,
            const S& name,
            const D& type)
        : name_(name)
        , type_(type)
        , index_(index)
    {}

    /**
     * convenience constructor
     * @remark Default index value assures it is appended to the collection.
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name new member's name
     * @param[in] type @ref DynamicType new member's type
     */
    template<class S,
             class D,
             typename std::enable_if<
                 std::is_constructible<std::string, S>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            const MemberId& id,
            const S& name,
            const D& type)
        : name_(name)
        , id_(id)
        , type_(type)
    {}

    /**
     * convenience constructor
     * @remark Default index value assures it is appended to the collection.
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name new member's name
     * @param[in] type @ref DynamicType new member's type
     * @param[in] defaultValue member default value as a string representation
     */
    template<class S1,
             class D,
             class S2,
             typename std::enable_if<
                 std::is_constructible<std::string, S1>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::string, S2>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            const MemberId& id,
            const S1& name,
            const D& type,
            const S2& defaultValue)
        : name_(name)
        , id_(id)
        , type_(type)
        , default_value_(defaultValue)
    {}

    /**
     * convenience constructor
     * @remark default index value assures it is appended to the collection.
     * @param[in] id @ref MemberId new member's identifier
     * @param[in] name new member's name
     * @param[in] type @ref DynamicType new member's type
     * @param[in] defaultValue member default value as a string representation
     * @param[in] unionLabels collection of label identifiers associated to this member
     * @param[in] isDefaultLabel is true if 'default' idl union case is enforced
     */
    template<class S1,
             class D,
             class S2,
             class Cont,
             typename std::enable_if<
                 std::is_constructible<std::string, S1>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_constructible<std::string, S2>::value,
                 bool>::type = true,
             typename std::enable_if<
                 std::is_same<uint64_t, typename Cont::value_type>::value,
                 bool>::type = true>
    MemberDescriptorImpl(
            const MemberId& id,
            const S1& name,
            const D& type,
            const S2& defaultValue,
            const Cont& unionLabels,
            bool isDefaultLabel)
        : name_(name)
        , id_(id)
        , type_(type)
        , default_value_(defaultValue)
        , labels_{unionLabels.begin(), unionLabels.end()}
        , default_label_(isDefaultLabel)
    {}

    /**
     * Default copy assignment
     * @remark Note that the no member uses mutable references, thus the default
     *         copy operator provides an actual deep copy.
     * @param[in] descriptor l-value @ref MemberDescriptorImpl reference to copy from
     * @result own @ref MemberDescriptorImpl reference
     */
    MemberDescriptorImpl& operator =(
            const MemberDescriptorImpl& descriptor) = default;

    //! Default move assignment
    MemberDescriptorImpl& operator =(
            MemberDescriptorImpl&& descriptor) = default;

    ~MemberDescriptorImpl() = default;

    //! check if any given labels are already set
    template<
        typename C,
        typename std::enable_if<std::is_same<uint64_t, typename C::value_type>::value, bool>::type = true>
    bool check_union_labels(
            C const& labels) const
    {
        return std::none_of(labels.begin(), labels.end(), [this](uint64_t l) -> bool
                       {
                           return labels_.find(l) != labels_.end();
                       });
    }

    /**
     * checks for equality according with [standard] section 7.5.2.7.4
     * @param[in] other @ref MemberDescriptorImpl reference to compare to
     * @return bool on equality
     * [standard]: https://www.omg.org/spec/dds-xtypes/1.3/ "OMG standard"
     */
    bool operator ==(
            const MemberDescriptorImpl& other) const;

    /**
     * checks for inequality according with [standard] section 7.5.2.7.4
     * @param[in] other @ref MemberDescriptorImpl reference to compare to
     * @return bool on inequality
     * [standard]: https://www.omg.org/spec/dds-xtypes/1.3/ "omg standard"
     */
    bool operator !=(
            const MemberDescriptorImpl& other) const;

    //! convenient getter for the associated \b type property
    TypeKind get_kind() const;

    /**
     * Queries the desired or actual member id
     * @return MemberId id
     */
    MemberId get_id() const;

    /**
     * Queries the desired or actual member position in the collection.
     * @return uint32_t position
     */
    uint32_t get_index() const;

    /**
     * Queries the desired or actual member name
     * @return std::string name
     */
    const std::string& get_name() const;

    //! getter for the labels member
    const std::set<uint64_t>& get_union_labels() const;

    //! getter for the \b default_value property
    std::string get_default_value() const;

    //! checks if the member is the 'default' idl case
    bool is_default_union_value() const;

    /**
     * Tests state consistency
     * @remark A MemberDescriptorImpl shall be considered consistent if and only if all of the values
     *         of its properties are considered consistent with its collection owner
     * @param[in] parentKind @ref eprosima::fastrtps::types::TypeKind collection's owner kind
     * @return bool `true` if consistent
     */
    bool is_consistent(
            TypeKind parentKind) const;

    //! insert a new label for this union member
    void add_union_case_index(
            uint64_t value);

    /**
     * Set member @ref MemberId
     * @param[in] id desired MemberId
     */
    void set_id(
            MemberId id);

    /**
     * Set member index
     * @remark Only modifiable for elements detach from any collection
     * @remark `DynamicTypeBuilder::add_member(...)` methods will query it in order
     *         to position this element in their member collection. According with
     *         [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.7.6
     * @param[in] index uint32_t desired position
     */
    void set_index(
            uint32_t index);

    /**
     * Set member name
     * @param[in] name desired name
     */
    template<class S,
             typename std::enable_if<
                 std::is_constructible<std::string, S>::value,
                 bool>::type = true>
    void set_name(
            const S& name)
    {
        name_ = name;
    }

    /**
     * Set member type by move
     * @param[in] type @ref DynamicType r-value
     */
    void set_type(
            std::shared_ptr<const DynamicTypeImpl>&& type);

    /**
     * Set member type by copy
     * @param[in] type @ref DynamicType l-value
     */
    template<class D,
             typename std::enable_if<
                 std::is_constructible<std::shared_ptr<const DynamicTypeImpl>, D>::value,
                 bool>::type = true>
    void set_type(
            const D& type)
    {
        std::shared_ptr<const DynamicTypeImpl> tmp{type};
        type_.swap(tmp);
    }

    /**
     * Queries the desired or actual member type
     * @return @ref DynamicType
     */
    std::shared_ptr<const DynamicTypeImpl> get_type() const;

    void set_default_union_value(
            bool bDefault);

    // setter for the \b default_value property
    void set_default_value(
            const std::string& value)
    {
        default_value_ = value;
    }

};

//! @ref DynamicTypeMember expected `std::ostream` non-member override of `operator<<`
std::ostream& operator <<(
        std::ostream& os,
        const MemberDescriptorImpl& md);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_MEMBER_DESCRIPTOR_IMPL_H
