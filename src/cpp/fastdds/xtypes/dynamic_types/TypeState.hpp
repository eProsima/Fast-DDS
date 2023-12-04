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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_TYPESTATE_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_TYPESTATE_HPP

#include "DynamicTypeMemberImpl.hpp"
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/Types.hpp>

#include <limits>
#include <list>
#include <string>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;
class DynamicTypeMember;
class DynamicTypeImpl;
class DynamicTypeBuilder;
class DynamicTypeBuilderImpl;
class DynamicTypeBuilderFactoryImpl;

struct TypeStateData
{
    std::string name_;                                    //!< Type Name.
    TypeKind kind_ = TK_NONE;                   //!< Type Kind.
    std::shared_ptr<const DynamicTypeImpl> base_type_;          //!< SuperType of an structure or base type of an alias type.
    std::shared_ptr<const DynamicTypeImpl> discriminator_type_; //!< Discrimination type for a union.
    std::vector<uint32_t> bound_;                         //!< Length for strings, arrays, sequences, maps and bitmasks.
    std::shared_ptr<const DynamicTypeImpl> element_type_;       //!< Value Type for arrays, sequences, maps, bitmasks.
    std::shared_ptr<const DynamicTypeImpl> key_element_type_;   //!< Key Type for maps.
    //TODO(richiware)std::list<DynamicTypeMemberImpl> members_;                //!< Member descriptors sequence
};

/**
 * The purpose of this class is providing a common state for DynamicTypes and DynamicTypeBuilsers
 * @attention
 *    TypeState only provides public const getters because is only meant for
 *    informational purposes. Use a @ref DynamicTypeBuilderImpl to access setters.
 */
class TypeState
    : protected TypeStateData
    , protected AnnotationManager
{
protected:

    TypeState(
            const std::string& name,
            TypeKind kind);

public:

    //! Default constructor
    TypeState() = default;

    //! Copy constructor
    TypeState(
            const TypeState& other);

    //! Move constructor
    TypeState(
            TypeState&& other) = default;

    //! Copy assignment
    TypeState& operator =(
            const TypeState& other) noexcept;

    //! Move assignment
    TypeState& operator =(
            TypeState&& descriptor) noexcept = default;

    /**
     * Destructor
     * @attention
     *    The destructor is not virtual despite of this class been \b superclass
     *    of @ref DynamicTypeImpl and @ref DynamicTypeBuilderImpl because
     *    the \b subclasses are not supposed to introduce members requiring
     *    complex clean-up. State is constrained to the \b superclass.
     */
    ~TypeState() noexcept;

    //! Create from descriptor
    TypeState(
            const TypeDescriptor& descriptor);

    /**
     * Returns the TypeDescriptor object that partially describes the state
     * @return @ref TypeDescriptor object
     */
    //TypeDescriptor get_descriptor() const noexcept;

    static bool is_type_name_consistent(
            const std::string& sName);

protected:

    bool is_key_defined_ = false;
    std::map<MemberId, DynamicTypeMemberImpl*> member_by_id_;       //!< members references indexed by id
    std::map<std::string, DynamicTypeMemberImpl*> member_by_name_;  //!< members references indexed by name

    void refresh_indexes();

    static const DynamicTypeImpl& resolve_alias_type(
            const DynamicTypeImpl& alias);

    using member_iterator = std::list<DynamicTypeMemberImpl>::iterator;

    void clean();

    friend class DynamicTypeBuilderFactoryImpl;

    /**
     * Modifies the underlying type name by copy
     * @param[in] name \b string l-value reference
     */
    void set_name(
            const std::string& name);

    /**
     * Modifies the underlying type name by move
     * @param[in] name \b string r-value reference
     */
    void set_name(
            std::string&& name);

    //! Modifies the underlying kind
    void set_kind(
            TypeKind kind);

    /**
     * Modifies the underlying base type by copy
     * @param[in] type std::shared_ptr<const DynamicTypeImpl> l-value reference
     */
    void set_base_type(
            const std::shared_ptr<const DynamicTypeImpl>& type);

    /**
     * Modifies the underlying base type by copy
     * @param[in] type std::shared_ptr<const DynamicTypeImpl> r-value reference
     */
    void set_base_type(
            std::shared_ptr<const DynamicTypeImpl>&& type);

    //! public interface common implementation for DynamicType and DynamicTypeBuilder
    const DynamicTypeMember* get_member_by_name(
            const char* name,
            ReturnCode_t* ec) const noexcept;

    //! public interface common implementation for DynamicType and DynamicTypeBuilder
    DynamicTypeMembersByName get_all_members_by_name(
            ReturnCode_t* ec) const noexcept;

    //! public interface common implementation for DynamicType and DynamicTypeBuilder
    DynamicTypeMembersById get_all_members_by_id(
            ReturnCode_t* ec) const noexcept;

public:

    using AnnotationManager::get_all_annotations;
    using AnnotationManager::get_annotation_count;
    using AnnotationManager::get_annotations;

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
    MemberId get_id_from_label(
            uint64_t label) const;

    /**
     * Checks if there is a member with the given name.
     * @param[in] name string
     * @return true if exists
     */
    bool exists_member_by_name(
            const ObjectName& name) const;

    /**
     * Checks if there is a member with the given id.
     * @param[in] id MemberId
     * @return true if exists
     */
    bool exists_member_by_id(
            MemberId id) const;

    /**
     * Queries members by name
     * @param[in] name string
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    MemberId get_member_id_by_name(
            const std::string& name) const;

    /**
     * Queries members by index
     * @param[in] index uint32_t
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    MemberId get_member_id_at_index(
            uint32_t index) const;

    /**
     * Returns the state of the @ref DynamicTypeImpl or @ref DynamicTypeBuilderImpl object
     * @param[in] descriptor object state
     * @return standard @ref ReturnCode_t
     */
    ReturnCode_t copy_from(
            const TypeState& descriptor) noexcept;

    /**
     * Checks equality according [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @param[in] descriptor reference to the @ref TypeState to compare to
     * @return true on equality
     * @remarks Note that @ref TypeState is superclass of @ref DynamicTypeImpl and @ref DynamicTypeBuilderImpl.
                The subclasses doesn't add any extra data to the state. The subclasses inherit this operator.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool operator ==(
            const TypeState& descriptor) const;

    /**
     * Checks inequality according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @param[in] descriptor reference to the @ref TypeState to compare to
     * @return true on equality
     * @remarks Note that @ref TypeState is superclass of @ref DynamicTypeImpl and @ref DynamicTypeBuilderImpl.
                The subclasses doesn't add any extra data to the state. The subclasses inherit this operator.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool operator !=(
            const TypeState& descriptor) const;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(
            const TypeState& descriptor) const noexcept;

    /**
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @param type bool `true` if we search a consistent type
     * @remark consistency for a type is more restrictive than for a builder which may require
     *         members and annotations.
     * @return \b bool `true` if consistent
     */
    bool is_consistent(
            bool type = false) const;

    /**
     * Checks if the kind is a primitive one according to the [standard] section \b 7.2.2.2
     * @return \b bool `true` if primitive
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool is_primitive() const;

    //! Checks if the underlying aggregate type is a subclass of the given one
    bool is_subclass(
            const TypeState& descriptor) const;

    /**
     * Getter for @b base_type property (see [standard] table 50)
     * @return @ref DynamicTypeImpl
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::shared_ptr<const DynamicTypeImpl> get_base_type() const;

    /**
     * Getter for @b bound property (see [standard] table 50)
     * @param[in] index dimension bound to retrieve on multidimensional collections
     * @return uint32_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    uint32_t get_bounds(
            uint32_t index = 0) const;

    //! Number of dimensions in the underlying collection
    uint32_t get_bounds_size() const;

    //! Number of elements in the underlying collection
    uint32_t get_total_bounds() const;

    /**
     * Getter for @b discriminator_type property (see [standard] table 50)
     * @return @ref DynamicTypeImpl
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::shared_ptr<const DynamicTypeImpl> get_discriminator_type() const;

    /**
     * Getter for @b element_type property (see [standard] table 50)
     * @return @ref DynamicTypeImpl
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::shared_ptr<const DynamicTypeImpl> get_element_type() const;

    /**
     * Getter for @b key_element_type property (see [standard] table 50)
     * @return @ref DynamicTypeImpl
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    std::shared_ptr<const DynamicTypeImpl> get_key_element_type() const;

    /**
     * Returns the @ref TypeKind associated
     * @return standard @ref TypeKind
     */
    TypeKind get_kind() const noexcept;

    /**
     * Returns the fully qualified name of this type
     * @return std::string type name
     */
    const std::string& get_name() const;

    /**
     * Returns a member sequence collection
     * @attention This method is not thread safe.
     *            The returned collection may be modified afterwards.
     *            The collection use should not outlive this Dynamic object.
     * @return list<@ref DynamicTypeMemberImpl>
     */
    const std::list<const DynamicTypeMemberImpl*> get_all_members() const;

    /**
     * Populates an associative collection of member references indexed by @ref MemberId
     * @attention This method is not thread safe. The returned collection use should not outlive this Dynamic object.
     * @return members map<@ref MemberId, @ref DynamicTypeMemberImpl> collection to populate

     */
    std::map<MemberId, const DynamicTypeMemberImpl*> get_all_members_by_id() const;

    /**
     * Populates an associative collection of member references indexed by name
     * @attention This method is not thread safe. The returned collection use should not outlive this Dynamic object.
     * @return members map<std::string, @ref DynamicTypeMemberImpl> collection to populate
     */
    std::map<std::string, const DynamicTypeMemberImpl*> get_all_members_by_name() const;

    /**
     * Queries current number of members
     * @return uint32_t number of members
     */
    uint32_t get_member_count() const;

    /**
     * This operation returns the member that corresponds to the specified member ID
     * @param[in] id MemberId identifier to query
     * @return standard MemberDescriptorImpl
     * @throw std::system_error
     */
    const DynamicTypeMemberImpl& get_member(
            MemberId id) const;

    /**
     * This operation returns the member that corresponds to the specified index
     * @param[in] index uint32_t collection position to query
     * @return standard MemberDescriptorImpl
     */
    const DynamicTypeMemberImpl& get_member_by_index(
            uint32_t index) const;

    /**
     * This operation returns the member that corresponds to the specified name
     * @param[in] name std::string collection member name to query
     * @return standard MemberDescriptorImpl
     */
    const DynamicTypeMemberImpl& get_member_by_name(
            const std::string& name) const;
};

//! @ref TypeState expected `std::ostream` non-member override of `operator<<`
std::ostream& operator <<(
        std::ostream& os,
        const TypeState& md);

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_TYPESTATE_HPP
