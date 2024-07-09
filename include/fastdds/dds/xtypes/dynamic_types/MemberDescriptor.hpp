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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__MEMBERDESCRIPTOR_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__MEMBERDESCRIPTOR_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;

class FASTDDS_EXPORTED_API MemberDescriptor
{
public:

    using _ref_type = typename traits<MemberDescriptor>::ref_type;

    /*!
     * Returns the name of this member.
     * @return Member's name.
     */
    virtual ObjectName& name() = 0;

    /*!
     * Returns the name of this member.
     * @return Member's name.
     */
    virtual const ObjectName& name() const = 0;

    /*!
     * Modifies the underlying member's name by copy.
     * @param [in] name Member's name.
     */
    virtual void name(
            const ObjectName& name) = 0;

    /*!
     * Modifies the underlying member's name by move.
     * @param [in] name Member's name.
     */
    virtual void name(
            ObjectName&& name) = 0;

    /*!
     * Returns the @ref MemberId of the member.
     * @return @ref MemberId.
     */
    virtual MemberId id() const = 0;

    /*!
     * Returns the @ref MemberId of the member.
     * @return @ref MemberId.
     */
    virtual MemberId& id() = 0;

    /*!
     * Modifies the underlying @ref MemberId.
     * @param [in] id @ref MemberId to be set.
     */
    virtual void id(
            MemberId id) = 0;

    /*!
     * Returns a reference to the member's type.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type type() const = 0;

    /*!
     * Returns a reference to the member's type.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& type() = 0;

    /*!
     * Modifies the underlying member's type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void type(
            traits<DynamicType>::ref_type type) = 0;

    /*!
     * Returns the default value.
     * @return Default value.
     */
    virtual std::string& default_value() = 0;

    /*!
     * Returns the default value.
     * @return Default value.
     */
    virtual const std::string& default_value() const = 0;

    /*!
     * Modifies the underlying default value by copy.
     * @param [in] default_value Default value.
     */
    virtual void default_value(
            const std::string& default_value) = 0;

    /*!
     * Modifies the underlying default value by move.
     * @param [in] default_value Default value.
     */
    virtual void default_value(
            std::string&& default_value) = 0;

    /*!
     * Returns the order of definition of the member.
     * @return Order of definition.
     */
    virtual uint32_t& index() = 0;

    /*!
     * Returns the order of definition of the member.
     * @return Order of definition.
     */
    virtual uint32_t index() const = 0;

    /*!
     * Returns the labels the member belongs to.
     * @return @ref UnionCaseLabelSeq.
     */
    virtual const UnionCaseLabelSeq& label() const = 0;

    /*!
     * Returns the labels the member belongs to.
     * @return @ref UnionCaseLabelSeq.
     */
    virtual UnionCaseLabelSeq& label() = 0;

    /*!
     * Modifies the labels the member belongs to by copy.
     * @param [in] label @ref UnionCaseLabelSeq
     */
    virtual void label(
            const UnionCaseLabelSeq& label) = 0;

    /*!
     * Modifies the labels the member belongs to by move.
     * @param [in] label @ref UnionCaseLabelSeq
     */
    virtual void label(
            UnionCaseLabelSeq&& label) = 0;

    /*!
     * Returns the @ref TryConstructKind of the member.
     * @return @ref TryConstructKind.
     */
    virtual TryConstructKind try_construct_kind() const = 0;

    /*!
     * Returns the @ref TryConstructKind of the member.
     * @return @ref TryConstructKind.
     */
    virtual TryConstructKind& try_construct_kind() = 0;

    /*!
     * Modifies the @ref TryConstructKind of the member.
     * @param [in] try_construct_kind @ref TryConstructKind.
     */
    virtual void try_construct_kind(
            TryConstructKind try_construct_kind) = 0;

    /*!
     * Returns the if the member is key.
     * @return If the member is key.
     */
    virtual bool is_key() const = 0;

    /*!
     * Returns the if the member is key.
     * @return If the member is key.
     */
    virtual bool& is_key() = 0;

    /*!
     * Modifies if the member is key.
     * @param [in] is_key Boolean
     */
    virtual void is_key(
            bool is_key) = 0;

    /*!
     * Returns the if the member is optional.
     * @return If the member is optional.
     */
    virtual bool is_optional() const = 0;

    /*!
     * Returns the if the member is optional.
     * @return If the member is optional.
     */
    virtual bool& is_optional() = 0;

    /*!
     * Modifies if the member is optional.
     * @param [in] is_optional Boolean
     */
    virtual void is_optional(
            bool is_optional) = 0;

    /*!
     * Returns the if the member is must_understand.
     * @return If the member is must_understand.
     */
    virtual bool is_must_understand() const = 0;

    /*!
     * Returns the if the member is must_understand.
     * @return If the member is must_understand.
     */
    virtual bool& is_must_understand() = 0;

    /*!
     * Modifies if the member is must_understand.
     * @param [in] is_must_understand Boolean
     */
    virtual void is_must_understand(
            bool is_must_understand) = 0;

    /*!
     * Returns the if the member is shared.
     * @return If the member is shared.
     */
    virtual bool is_shared() const = 0;

    /*!
     * Returns the if the member is shared.
     * @return If the member is shared.
     */
    virtual bool& is_shared() = 0;

    /*!
     * Modifies if the member is shared.
     * @param [in] is_shared Boolean
     */
    virtual void is_shared(
            bool is_shared) = 0;

    /*!
     * Returns the if the member is default_label.
     * @return If the member is default_label.
     */
    virtual bool is_default_label() const = 0;

    /*!
     * Returns the if the member is default_label.
     * @return If the member is default_label.
     */
    virtual bool& is_default_label() = 0;

    /*!
     * Modifies if the member is default_label.
     * @param [in] is_default_label Boolean
     */
    virtual void is_default_label(
            bool is_default_label) = 0;

    /*!
     * Overwrites the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.7.1).
     * @param [in] descriptor reference.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    virtual ReturnCode_t copy_from(
            traits<MemberDescriptor>::ref_type descriptor) = 0;

    /*!
     * Compares according with the [standard] section \b 7.5.2.7.4.
     * @param [in] descriptor reference to compare to.
     * @return \b bool `true` on equality
     */
    virtual bool equals(
            traits<MemberDescriptor>::ref_type descriptor) = 0;

    /*!
     * Indicates whether the states of all of this descriptor's properties are consistent according with the [standard]
     * section \b 7.5.2.7.7.
     * @return \b bool `true` if consistent.
     */
    virtual bool is_consistent() = 0;

protected:

    MemberDescriptor() = default;

    MemberDescriptor(
            const MemberDescriptor& type) = default;

    MemberDescriptor(
            MemberDescriptor&& type) = default;

    virtual ~MemberDescriptor() = default;

private:

    MemberDescriptor& operator =(
            const MemberDescriptor& type) = delete;

    MemberDescriptor& operator =(
            MemberDescriptor&& type) = delete;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__MEMBERDESCRIPTOR_HPP
