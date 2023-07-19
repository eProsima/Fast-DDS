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

#ifndef TYPES_1_3_MEMBER_DESCRIPTOR_H
#define TYPES_1_3_MEMBER_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/MemberId.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;

class RTPS_DllAPI MemberDescriptor final
{
    std::string* name_ = nullptr;             //!< Name of the member
    MemberId id_;                             //!< MemberId, it should be filled automatically when the member is added.
    const DynamicType* type_ = nullptr;       //!< Member's Type.
    std::string* default_value_;              //!< Default value of the member in string.
    uint32_t index_ = INDEX_INVALID;          //!< Definition order of the member inside it's parent.
    std::vector<uint32_t>* labels_ = nullptr; //!< Case Labels for unions.
bool default_label_ = false;              //!< TRUE if it's the default option of a union.

public:

    MemberDescriptor() noexcept;

    MemberDescriptor(const MemberDescriptor& type) noexcept;

    MemberDescriptor(MemberDescriptor&& type) noexcept;

    ~MemberDescriptor() noexcept;

    MemberDescriptor& operator=(const MemberDescriptor& type) noexcept;

    MemberDescriptor& operator=(MemberDescriptor&& type) noexcept;

    bool operator ==(
            const MemberDescriptor& descriptor) const noexcept;

    bool operator !=(
            const MemberDescriptor& descriptor) const noexcept;

    /**
     * Returns the name of this member
     * @attention The returned value may not persist in time
     * @return const char* member name
     */
    const char* get_name() const noexcept;

    /**
     * Modifies the underlying type name by copy
     * @param[in] name reference
     */
    void set_name(
            const char* name) noexcept;

    /**
     * Getter for @b id property (see [standard] table 53)
     * @return @ref MemberId
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    MemberId get_id() const noexcept
    {
        return id_;
    }

    /**
     * Setter for @b id property (see [standard] table 53)
     * @param const @ref MemberId
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    void set_id(
            const MemberId id) noexcept
    {
        id_ = id;
    }

    /**
     * Getter for @b type property (see [standard] table 53)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_type() const noexcept;

    /**
     * Modifies the underlying type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_type(
                const DynamicType& type) noexcept;

    /**
     * Modifies the underlying type by copy
     * @param[in] type reference
     * @attention There is ownership transference.
     */
    void set_type(
                const DynamicType* type) noexcept;

    //! Clears the base type reference
    void reset_type() noexcept;

    /**
     * Getter for @b default value property (see [standard] table 53)
     * The string is well formed if is valid as IDL literal for the type (see [standard] section 7.5.2.7.3)
     * @attention The returned value may not persist in time
     * @return const char*
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    const char* get_default_value() const noexcept;

    /**
     * Setter for @b default value property (see [standard] table 53)
     * The string is well formed if is valid as IDL literal for the type (see [standard] section 7.5.2.7.3)
     * @param[in] default const char*
     */
    void set_default_value(
            const char* value) noexcept;

    /**
     * Getter for @b index property (see [standard] table 53)
     * @return @ref uint32_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    uint32_t get_index() const noexcept
    {
        return index_;
    }

    /**
     * Setter for @b index property (see [standard] table 53)
     * @param index @ref uint32_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    void set_index(
            const uint32_t index) noexcept
    {
        index_ = index;
    }

    /**
     * Getter for @b label property (see [standard] table 53)
     * @param[out] count uint32_t variable to populate with the number of labels
     * @return uint32_t* array of labels
     * @attention The returned value may not persist in time
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    const uint32_t* get_labels(
            uint32_t& count) const noexcept;

    /**
     * Setter for @b label property (see [standard] table 53)
     * @param[in] const uint32_t* labels references an array of labesl to copy
     * @param[in] uint32_t count number of labels
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    void set_labels(
            const uint32_t* labels,
            uint32_t count) noexcept;

    /**
     * Getter for @b default label property (see [standard] table 53)
     * @return bool that is true if the member represents the default value of a union
     *         see [standard] section \b 7.5.2.7.2
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    bool get_default_label() const noexcept
    {
        return default_label_;
    }

    /**
     * Setter for @b default label property (see [standard] table 53)
     * @param[in] value bool that is true if the member represents the default value of a union
     *            see [standard] section \b 7.5.2.7.2
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    void set_default_label(
            bool value) noexcept
    {
        default_label_ = value;
    }

    /**
     * Overwrite the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.7.1)
     * @param[in] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t copy_from(
        const MemberDescriptor& descriptor) noexcept;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(
            const MemberDescriptor& descriptor) const noexcept;

    /**
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @param[in] parentKind @ref eprosima::fastrtps::types::TypeKind collection's owner kind
     * @return \b bool `true` if consistent
     */
    bool is_consistent(TypeKind parentKind = TypeKind::TK_STRUCTURE) const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_MEMBER_DESCRIPTOR_H
