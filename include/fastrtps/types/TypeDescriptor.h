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

#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypesBase.h>

#include <limits>
#include <list>
#include <string>
#include <vector>

#ifndef __has_cpp_attribute
#    define __has_cpp_attribute(x) 0
#endif

#if __has_cpp_attribute(fallthrough) && (__has_cpp_attribute(fallthrough) < __cplusplus \
                                      || __has_cpp_attribute(fallthrough) < _MSVC_LANG )
#    define eprosima_fallthrough [[fallthrough]];
#elif defined(__GNUC__) || defined(__clang__)
#    define eprosima_fallthrough __attribute__((fallthrough));
#else
#    define eprosima_fallthrough
#endif

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicType;

struct TypeDescriptorData
{
    std::string name_;                      //!< Type Name.
    TypeKind kind_ = TypeKind::TK_NONE;     //!< Type Kind.
    DynamicType_ptr base_type_;             //!< SuperType of an structure or base type of an alias type.
    DynamicType_ptr discriminator_type_;    //!< Discrimination type for a union.
    std::vector<uint32_t> bound_;           //!< Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr element_type_;          //!< Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr key_element_type_;      //!< Key Type for maps.
    std::list<DynamicTypeMember> members_;  //!< Member descriptors sequence
};

/**
 * The purpose of this class is providing a common state for DynamicTypes and DynamicTypeBuilsers
 * @attention
 *    TypeDescriptor only provides public const getters because is only meant for
 *    informational purposes. Use a @ref DynamicTypeBuilder to access setters.
 */
class TypeDescriptor
      : protected TypeDescriptorData
      , protected AnnotationManager
{

    using TypeDescriptorData::TypeDescriptorData;

protected:

    TypeDescriptor(
            const std::string& name,
            TypeKind kind);
public:

    //! Default constructor
    RTPS_DllAPI TypeDescriptor() = default;

    //! Copy constructor
    RTPS_DllAPI TypeDescriptor(
            const TypeDescriptor& other) ;

    //! Move constructor
    RTPS_DllAPI TypeDescriptor(
            TypeDescriptor&& other) = default;

    /**
     * Default copy assignment
     * @remark Note that the no member uses mutable references, thus the default
     *         copy operator provides an actual deep copy.
     * @param[in] descriptor l-value @ref TypeDescriptor reference to copy from
     * @result own @ref TypeDescriptor reference
     */
    RTPS_DllAPI TypeDescriptor& operator=(
            const TypeDescriptor& descriptor) noexcept;

    //! Move assignment
    RTPS_DllAPI TypeDescriptor& operator=(
            TypeDescriptor&& descriptor) noexcept = default;

    /**
     * Destructor
     * @attention
     *    The destructor is not virtual despite of this class been \b superclass
     *    of @ref DynamicType and @ref DynamicTypeBuilder because
     *    the \b subclasses are not supposed to introduce members requiring
     *    complex clean-up. State is constrained to the \b superclass.
     */
    RTPS_DllAPI ~TypeDescriptor() noexcept;

    static bool is_type_name_consistent(
            const std::string& sName);

protected:

    bool is_key_defined_ = false;
    std::map<MemberId, DynamicTypeMember*> member_by_id_;       //!< members references indexed by id
    std::map<std::string, DynamicTypeMember*> member_by_name_;  //!< members references indexed by name

    void refresh_indexes();

    static const DynamicType& resolve_alias_type(const DynamicType& alias);

    /**
     * Returns the state of the @ref DynamicType or @ref DynamicTypeBuilder object
     * @param[out] descriptor object state
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_descriptor(
            TypeDescriptor& descriptor) const noexcept;

    /**
     * Returns the state of the @ref DynamicType or @ref DynamicTypeBuilder object
     * @return @ref TypeDescriptor object state reference
     */
    RTPS_DllAPI const TypeDescriptor& get_descriptor() const noexcept
    {
        return *this;
    }

    using member_iterator = std::list<DynamicTypeMember>::iterator;

    void clean();

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;
    friend class DynamicDataHelper;

    /**
     * Modifies the underlying type name by copy
     * @param[in] name \b string l-value reference
     */
    RTPS_DllAPI void set_name(
            const std::string& name);

    /**
     * Modifies the underlying type name by move
     * @param[in] name \b string r-value reference
     */
    RTPS_DllAPI void set_name(
            std::string&& name);

    //! Modifies the underlying kind
    RTPS_DllAPI void set_kind(
            TypeKind kind);

    /**
     * Modifies the underlying base type by copy
     * @param[in] type DynamicType_ptr l-value reference
     */
    RTPS_DllAPI void set_base_type(
            const DynamicType_ptr& type);

    /**
     * Modifies the underlying base type by copy
     * @param[in] type DynamicType_ptr r-value reference
     */
    RTPS_DllAPI void set_base_type(
            DynamicType_ptr&& type);

public:

    using AnnotationManager::get_all_annotations;
    using AnnotationManager::get_annotation_count;

    using AnnotationManager::annotation_is_bit_bound;
    using AnnotationManager::annotation_is_key;
    using AnnotationManager::annotation_is_non_serialized;

    using AnnotationManager::annotation_is_extensibility;
    using AnnotationManager::annotation_is_mutable;
    using AnnotationManager::annotation_is_final;
    using AnnotationManager::annotation_is_appendable;
    using AnnotationManager::annotation_is_nested;
    using AnnotationManager::key_annotation;

public:

    /**
     * Queries the member associated to a given label
     * @param[in] label uint64_t value to query
     * @return MEMBER_ID_INVALID on failure
     */
    MemberId get_id_from_label(uint64_t label) const;

    /**
     * Checks if there is a member with the given name.
     * @param[in] name string
     * @return true if exists
     */
    RTPS_DllAPI bool exists_member_by_name(
            const std::string& name) const;

    /**
     * Checks if there is a member with the given id.
     * @param[in] id MemberId
     * @return true if exists
     */
    RTPS_DllAPI bool exists_member_by_id(
            MemberId id) const;

    /**
     * Queries members by name
     * @param[in] name string
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    RTPS_DllAPI MemberId get_member_id_by_name(
            const std::string& name) const;

    /**
     * Queries members by index
     * @param[in] index uint32_t
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    RTPS_DllAPI MemberId get_member_id_at_index(
            uint32_t index) const;

    /**
     * Queries members by identifier
     * @param[in] id MemberId
     * @return std::pair where second if `second == true` then first is a reference to an associated @ref DynamicTypeMember
     */
    RTPS_DllAPI std::pair<const DynamicTypeMember*, bool> get_member(
            MemberId id) const;

    /**
     * Returns the state of the @ref DynamicType or @ref DynamicTypeBuilder object
     * @param[in] descriptor object state
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t copy_from(
            const TypeDescriptor& descriptor) noexcept;

    /**
     * Checks equality according [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @param[in] descriptor reference to the @ref TypeDescriptor to compare to
     * @return true on equality
     * @remarks Note that @ref TypeDescriptor is superclass of @ref DynamicType and @ref DynamicTypeBuilder.
                The subclasses doesn't add any extra data to the state. The subclasses inherit this operator.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI bool operator==(const TypeDescriptor& descriptor) const;

    /**
     * Checks inequality according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @param[in] descriptor reference to the @ref TypeDescriptor to compare to
     * @return true on equality
     * @remarks Note that @ref TypeDescriptor is superclass of @ref DynamicType and @ref DynamicTypeBuilder.
                The subclasses doesn't add any extra data to the state. The subclasses inherit this operator.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI bool operator!=(const TypeDescriptor& descriptor) const;

    /**
     * State comparisson according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI bool equals(
            const TypeDescriptor& descriptor) const noexcept;

    /**
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @return \b bool `true` if consistent
     */
    RTPS_DllAPI bool is_consistent() const;

    /**
     * Checks if the kind is a primitive one according to the [standard] section \b 7.2.2.2
     * @return \b bool `true` if primitive
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI bool is_primitive() const;

    //! Checks if the underlying aggregate type is a subclass of the given one
    RTPS_DllAPI bool is_subclass(const TypeDescriptor& descriptor) const;

    /**
     * Getter for @b base_type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI DynamicType_ptr get_base_type() const;

    /**
     * Getter for @b bound property (see [standard] table 50)
     * @param[in] index dimension bound to retrieve on multidimensional collections
     * @return uint32_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI uint32_t get_bounds(
            uint32_t index = 0) const;

    //! Number of dimensions in the underlying collection
    RTPS_DllAPI uint32_t get_bounds_size() const;

    //! Number of elements in the underlying collection
    RTPS_DllAPI uint32_t get_total_bounds() const;

    /**
     * Getter for @b discriminator_type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI DynamicType_ptr get_discriminator_type() const;

    /**
     * Getter for @b element_type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI DynamicType_ptr get_element_type() const;

    /**
     * Getter for @b key_element_type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI DynamicType_ptr get_key_element_type() const;

    /**
     * Returns the @ref TypeKind associated
     * @return standard @ref TypeKind
     */
    RTPS_DllAPI TypeKind get_kind() const noexcept;

    /**
     * Returns the fully qualified name of this type
     * @return std::string type name
     */
    RTPS_DllAPI std::string get_name() const;

    /**
     * Returns a member sequence collection
     * @attention This method is not thread safe.
     *            The returned collection may be modified afterwards.
     *            The collection use should not outlive this Dynamic object.
     * @return list<@ref DynamicTypeMember>
     */
    RTPS_DllAPI const std::list<const DynamicTypeMember*> get_all_members() const;

    /**
     * Populates an associative collection of member references indexed by @ref MemberId
     * @attention This method is not thread safe. The returned collection use should not outlive this Dynamic object.
     * @return members map<@ref MemberId, @ref DynamicTypeMember> collection to populate

     */
    RTPS_DllAPI std::map<MemberId, const DynamicTypeMember*> get_all_members_by_id() const;

    /**
     * Populates an associative collection of member references indexed by name
     * @attention This method is not thread safe. The returned collection use should not outlive this Dynamic object.
     * @return members map<std::string, @ref DynamicTypeMember> collection to populate
     */
    RTPS_DllAPI std::map<std::string, const DynamicTypeMember*> get_all_members_by_name() const;

    /**
     * Queries current number of members
     * @return uint32_t number of members
     */
    RTPS_DllAPI uint32_t get_member_count() const;

    /**
     * This operation returns the member that corresponds to the specified member ID
     * @param[in, out] member MemberDescriptor to fill in
     * @param[in] id MemberId identifier to query
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_member(
            MemberDescriptor& member,
            MemberId id) const noexcept;

    /**
     * This operation returns the member that corresponds to the specified index
     * @param[in, out] member MemberDescriptor to fill in
     * @param[in] index uint32_t collection position to query
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_member_by_index(
            MemberDescriptor& member,
            uint32_t index) const noexcept;

    /**
     * This operation returns the member that corresponds to the specified name
     * @param[in, out] member MemberDescriptor to fill in
     * @param[in] name std::string collection member name to query
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_member_by_name(
            MemberDescriptor& member,
            const std::string& name) const noexcept;
};

//! @ref TypeDescriptor expected `std::ostream` non-member override of `operator<<`
RTPS_DllAPI std::ostream& operator<<(std::ostream& os, const TypeDescriptor& md);

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
