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

#ifndef TYPES_1_3_TYPE_DESCRIPTOR_HPP
#define TYPES_1_3_TYPE_DESCRIPTOR_HPP

#include <fastrtps/types/TypesBase.h>

#include <cstdint>
#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;

/**
 * TypeDescriptor definition according to [standard] section \b 7.5.2.4
 */
class RTPS_DllAPI TypeDescriptor final
{
    std::string* name_ = nullptr;                       //!< Type Name.
    TypeKind kind_ = TypeKind::TK_NONE;                 //!< Type Kind.
    const DynamicType* base_type_ = nullptr;            //!< SuperType of an structure or base type of an alias type.
    const DynamicType* discriminator_type_ = nullptr;   //!< Discrimination type for a union.
    std::vector<uint32_t>* bounds_ = nullptr;            //!< Length for strings, arrays, sequences, maps and bitmasks.
    const DynamicType* element_type_ = nullptr;         //!< Value Type for arrays, sequences, maps, bitmasks.
    const DynamicType* key_element_type_ = nullptr;     //!< Key Type for maps.

public:

    TypeDescriptor() noexcept;

    TypeDescriptor(const char* name, TypeKind kind) noexcept;

    TypeDescriptor(const TypeDescriptor& type) noexcept;

    TypeDescriptor(TypeDescriptor&& type) noexcept;

    ~TypeDescriptor() noexcept;

    TypeDescriptor& operator=(const TypeDescriptor& type) noexcept;

    TypeDescriptor& operator=(TypeDescriptor&& type) noexcept;

    bool operator ==(
            const TypeDescriptor& descriptor) const noexcept;

    bool operator !=(
            const TypeDescriptor& descriptor) const noexcept;

    /**
     * Returns the fully qualified name of this type
     * @attention The returned value may not persist in time
     * @return const char* type name
     */
    const char* get_name() const noexcept;

    /**
     * Modifies the underlying type name by copy
     * @param[in] name reference
     */
    void set_name(
            const char* name) noexcept;

    /**
     * Returns the @ref eprosima::fastrtps::types::TypeKind associated
     * @return standard @ref eprosima::fastrtps::types::TypeKind
     */
    TypeKind get_kind() const noexcept;

    //! Modifies the underlying kind
    void set_kind(
            TypeKind kind) noexcept;

    /**
     * Getter for @b base type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_base_type() const noexcept;

    /**
     * Modifies the underlying base type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_base_type(
            const DynamicType& type) noexcept;

    /**
     * Modifies the underlying base type by copy
     * @param[in] type reference
     * @attention There is ownership transference.
     */
    void set_base_type(
            const DynamicType* type) noexcept;

    //! Clears the base type reference
    void reset_base_type() noexcept;

    /**
     * Getter for @b discriminator type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_discriminator_type() const noexcept;

    /**
     * Modifies the underlying discriminator type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_discriminator_type(
            const DynamicType& type) noexcept;

    /**
     * Modifies the underlying discriminator type by copy
     * @param[in] type reference
     * @attention There is ownership transference.
     */
    void set_discriminator_type(
            const DynamicType* type) noexcept;

    //! Clears the discriminator type reference
    void reset_discriminator_type() noexcept;

    /**
     * Getter for @b element type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_element_type() const noexcept;

    /**
     * Modifies the underlying element type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_element_type(
            const DynamicType& type) noexcept;

    /**
     * Modifies the underlying element type by copy
     * @param[in] type reference
     * @attention There is ownership transference.
     */
    void set_element_type(
            const DynamicType* type) noexcept;

    //! Clears the element type reference
    void reset_element_type() noexcept;

    /**
     * Getter for @b key element type property (see [standard] table 50)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_key_element_type() const noexcept;

    /**
     * Modifies the underlying key element type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_key_element_type(
            const DynamicType& type) noexcept;

    /**
     * Modifies the underlying key element type by copy
     * @param[in] type reference
     * @attention There is ownership transference.
     */
    void set_key_element_type(
            const DynamicType* type) noexcept;

    //! Clears the key element type reference
    void reset_key_element_type() noexcept;

    /**
     * Getter for @b bound property (see [standard] table 50)
     * @param[out] uint32_t variable to populate with the dimensions on multidimensional collections
     * @return uint32_t* array of dimension lenghts
     * @attention The returned value may not persist in time
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    const uint32_t* get_bounds(
            uint32_t& dims) const noexcept;

    /**
     * Setter for @b bound property (see [standard] table 50)
     * @param[in] const uint32_t* lengths references an array of dimension lengths to copy
     * @param[in] uint32_t dims dimensions on multidimensional collections
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    void set_bounds(
            const uint32_t* lengths,
            uint32_t dims) noexcept;

    /**
     * Overwrite the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.3.1)
     * @param[in] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t copy_from(
        const TypeDescriptor& descriptor) noexcept;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(
            const TypeDescriptor& descriptor) const noexcept;

    /**
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @return \b bool `true` if consistent
     */
    bool is_consistent() const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_TYPE_DESCRIPTOR_HPP
