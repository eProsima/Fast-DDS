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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAIMPL_HPP

#include <map>
#include <vector>

#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include "DynamicTypeImpl.hpp"
#include "TypeForKind.hpp"

namespace eprosima {

namespace fastcdr {
class Cdr;
class CdrSizeCalculator;
} // namespace fastcdr

namespace fastdds {
namespace dds {

class DynamicDataImpl : public traits<DynamicData>::base_type
{
    //{{{ Implementation members of DynamicData

    //! The associated type.
    traits<DynamicTypeImpl>::ref_type type_;

    //! Enclosed type in case of `type_` is TK_ALIAS or TK_ENUM. In other case, the same value as `type_`.
    traits<DynamicTypeImpl>::ref_type enclosing_type_;

    //! Contains the values of the current sample.
    std::map<MemberId, std::shared_ptr<void>> value_;

    //! Used in TK_MAP to maintain correlation between keys and MemberIds.
    std::map<std::string, MemberId> key_to_id_;

    //! Used in TK_MAP to know which is the next MemberId to be used.
    MemberId next_map_member_id_ {0};

    //! Stores the loaned values by the user.
    std::vector<MemberId> loaned_values_;

    //! Points to the current selected member in the union.
    MemberId selected_union_member_ {MEMBER_ID_INVALID};

    //}}}

public:

    //{{{ Public functions

    DynamicDataImpl() noexcept = default;

    DynamicDataImpl(
            traits<DynamicType>::ref_type type) noexcept;

    ReturnCode_t clear_all_values() noexcept override;

    ReturnCode_t clear_nonkey_values() noexcept override;

    ReturnCode_t clear_value(
            MemberId id) noexcept override;

    traits<DynamicData>::ref_type clone() noexcept override;

    bool equals(
            traits<DynamicData>::ref_type other) noexcept override;

    ReturnCode_t get_descriptor(
            traits<MemberDescriptor>::ref_type& value,
            MemberId id) noexcept override;

    uint32_t get_item_count() noexcept override;

    MemberId get_member_id_at_index(
            uint32_t index) noexcept override;

    MemberId get_member_id_by_name(
            const ObjectName& name) noexcept override;

    MemberId selected_union_member() noexcept;

    //{{{ Getters

    //{{{ Primitive getters

    ReturnCode_t get_int32_value(
            int32_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint32_value(
            uint32_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int8_value(
            int8_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint8_value(
            uint8_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int16_value(
            int16_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint16_value(
            uint16_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int64_value(
            int64_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint64_value(
            uint64_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float32_value(
            float& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float64_value(
            double& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float128_value(
            long double& value,
            MemberId id) noexcept override;

    ReturnCode_t get_char8_value(
            char& value,
            MemberId id) noexcept override;

    ReturnCode_t get_char16_value(
            wchar_t& value,
            MemberId id) noexcept override;

    ReturnCode_t get_byte_value(
            eprosima::fastdds::rtps::octet& value,
            MemberId id) noexcept override;

    ReturnCode_t get_boolean_value(
            bool& value,
            MemberId id) noexcept override;

    ReturnCode_t get_string_value(
            std::string& value,
            MemberId id) noexcept override;

    ReturnCode_t get_wstring_value(
            std::wstring& value,
            MemberId id) noexcept override;

    //}}}

    ReturnCode_t get_complex_value(
            traits<DynamicData>::ref_type& value,
            MemberId id) noexcept override;

    //{{{ Array getters

    ReturnCode_t get_int32_values(
            Int32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint32_values(
            UInt32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int8_values(
            Int8Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint8_values(
            UInt8Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int16_values(
            Int16Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint16_values(
            UInt16Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_int64_values(
            Int64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_uint64_values(
            UInt64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float32_values(
            Float32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float64_values(
            Float64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_float128_values(
            Float128Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_char8_values(
            CharSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_char16_values(
            WcharSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_byte_values(
            ByteSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_boolean_values(
            BooleanSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_string_values(
            StringSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t get_wstring_values(
            WstringSeq& value,
            MemberId id) noexcept override;

    //}}}

    //}}}

    traits<DynamicData>::ref_type loan_value(
            MemberId id) noexcept override;

    ReturnCode_t return_loaned_value(
            traits<DynamicData>::ref_type value) noexcept override;

    //{{{ Setters

    //{{{ Primitive getters

    ReturnCode_t set_int32_value(
            MemberId id,
            int32_t value) noexcept override;

    ReturnCode_t set_uint32_value(
            MemberId id,
            uint32_t value) noexcept override;

    ReturnCode_t set_int8_value(
            MemberId id,
            int8_t value) noexcept override;

    ReturnCode_t set_uint8_value(
            MemberId id,
            uint8_t value) noexcept override;

    ReturnCode_t set_int16_value(
            MemberId id,
            int16_t value) noexcept override;

    ReturnCode_t set_uint16_value(
            MemberId id,
            uint16_t value) noexcept override;

    ReturnCode_t set_int64_value(
            MemberId id,
            int64_t value) noexcept override;

    ReturnCode_t set_uint64_value(
            MemberId id,
            uint64_t value) noexcept override;

    ReturnCode_t set_float32_value(
            MemberId id,
            float value) noexcept override;

    ReturnCode_t set_float64_value(
            MemberId id,
            double value) noexcept override;

    ReturnCode_t set_float128_value(
            MemberId id,
            long double value) noexcept override;

    ReturnCode_t set_char8_value(
            MemberId id,
            char value) noexcept override;

    ReturnCode_t set_char16_value(
            MemberId id,
            wchar_t value) noexcept override;

    ReturnCode_t set_byte_value(
            MemberId id,
            eprosima::fastdds::rtps::octet value) noexcept override;

    ReturnCode_t set_boolean_value(
            MemberId id,
            bool value) noexcept override;

    ReturnCode_t set_string_value(
            MemberId id,
            const std::string& value) noexcept override;

    ReturnCode_t set_wstring_value(
            MemberId id,
            const std::wstring& value) noexcept override;

    //}}}

    ReturnCode_t set_complex_value(
            MemberId id,
            traits<DynamicData>::ref_type value) noexcept override;

    //{{{ Array getters

    ReturnCode_t set_int32_values(
            MemberId id,
            const Int32Seq& value) noexcept override;

    ReturnCode_t set_uint32_values(
            MemberId id,
            const UInt32Seq& value) noexcept override;

    ReturnCode_t set_int8_values(
            MemberId id,
            const Int8Seq& value) noexcept override;

    ReturnCode_t set_uint8_values(
            MemberId id,
            const UInt8Seq& value) noexcept override;

    ReturnCode_t set_int16_values(
            MemberId id,
            const Int16Seq& value) noexcept override;

    ReturnCode_t set_uint16_values(
            MemberId id,
            const UInt16Seq& value) noexcept override;

    ReturnCode_t set_int64_values(
            MemberId id,
            const Int64Seq& value) noexcept override;

    ReturnCode_t set_uint64_values(
            MemberId id,
            const UInt64Seq& value) noexcept override;

    ReturnCode_t set_float32_values(
            MemberId id,
            const Float32Seq& value) noexcept override;

    ReturnCode_t set_float64_values(
            MemberId id,
            const Float64Seq& value) noexcept override;

    ReturnCode_t set_float128_values(
            MemberId id,
            const Float128Seq& value) noexcept override;

    ReturnCode_t set_char8_values(
            MemberId id,
            const CharSeq& value) noexcept override;

    ReturnCode_t set_char16_values(
            MemberId id,
            const WcharSeq& value) noexcept override;

    ReturnCode_t set_byte_values(
            MemberId id,
            const ByteSeq& value) noexcept override;

    ReturnCode_t set_boolean_values(
            MemberId id,
            const BooleanSeq& value) noexcept override;

    ReturnCode_t set_string_values(
            MemberId id,
            const StringSeq& value) noexcept override;

    ReturnCode_t set_wstring_values(
            MemberId id,
            const WstringSeq& value) noexcept override;

    //}}}

    //}}}


    traits<DynamicType>::ref_type type() noexcept override;

    traits<DynamicTypeImpl>::ref_type enclosing_type() noexcept;

    /*!
     * Auxiliary function to get a copy of the `key_to_id_` attribute.
     * Only valid for TK_MAP.
     * @note: This is a solution to allow the user to get the keys of a map, currently not supported by the public API.
     */
    ReturnCode_t get_keys(
            std::map<std::string, MemberId>& key_to_id) noexcept;

    //{{{ Encoding/decoding functions

    size_t calculate_key_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            size_t& current_alignment) const noexcept;

    static size_t calculate_max_serialized_size(
            traits<DynamicType>::ref_type type,
            size_t current_alignment = 0);

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            size_t& current_alignment) const noexcept;

    bool deserialize(
            eprosima::fastcdr::Cdr& cdr);

    void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    void serialize_key(
            eprosima::fastcdr::Cdr& cdr) const noexcept;

    //}}}

    //}}}

protected:

    traits<DynamicData>::ref_type _this();

private:

    //{{{ Auxiliary functions

    /*!
     * Auxiliary function to apply the bitset mask when setting a value.
     */
    template<TypeKind TK>
    void apply_bitset_mask(
            MemberId member_id,
            TypeForKind<TK>& value) const noexcept;

    void add_sequence_value(
            const traits<DynamicTypeImpl>::ref_type& sequence_type,
            uint32_t sequence_size) noexcept;

    std::map<MemberId, std::shared_ptr<void>>::iterator add_value(
            TypeKind kind,
            MemberId id) noexcept;

    /*!
     * Auxiliary function for getting the initial number of elements for TK_ARRAY.
     */
    uint32_t calculate_array_max_elements(
            TypeKind type_kind) noexcept;

    /*!
     * Auxiliary function for checking the new discriminator value set by the user is correct.
     */
    template<typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    bool check_new_discriminator_value(
            const T& value)
    {
        bool ret_value = false;

        if (MEMBER_ID_INVALID != selected_union_member_) // There is a member selected by current discriminator.
        {
            traits<DynamicTypeMember>::ref_type selected_member;
            enclosing_type_->get_member(selected_member, selected_union_member_);
            auto sm_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(selected_member);

            for (auto label : sm_impl->get_descriptor().label())
            {
                if (static_cast<int32_t>(value) == label)
                {
                    ret_value = true;
                    break;
                }
            }
        }

        if (MEMBER_ID_INVALID == selected_union_member_ ||
                (MEMBER_ID_INVALID == enclosing_type_->default_union_member() && !ret_value)) // It is selected the implicit default member.
        {
            ret_value = true;

            if (enclosing_type_->default_value() != static_cast<int32_t>(value))
            {
                for (auto member : enclosing_type_->get_all_members_by_index())
                {
                    auto m_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);

                    for (auto label : m_impl->get_descriptor().label())
                    {
                        if (static_cast<int32_t>(value) == label)
                        {
                            ret_value = false;
                            break;
                        }
                    }
                }
            }

            if (ret_value)
            {
                selected_union_member_ = MEMBER_ID_INVALID;
            }
        }

        return ret_value;
    }

    template<typename T, typename std::enable_if<!std::is_integral<T>::value, bool>::type = true>
    bool check_new_discriminator_value(
            const T&)
    {
        return false;
    }

    /*!
     * Auxiliary function to clear completely a sequence.
     * Only valid for TK_ARRAY or TK_SEQUENCE.
     */
    ReturnCode_t clear_all_sequence(
            TypeKind type_kind) noexcept;

    ReturnCode_t clear_all_values(
            bool only_non_keyed) noexcept;

    /*!
     * Auxiliary function to clear a sequence element.
     * Only valid for TK_ARRAY and TK_SEQUENCE.
     */
    ReturnCode_t clear_sequence_element(
            TypeKind type_kind,
            MemberId id) noexcept;

    /*!
     * Auxiliary function to clone a primitive.
     */
    std::shared_ptr<void> clone_primitive(
            TypeKind type_kind,
            const std::shared_ptr<void>& primitive) const noexcept;

    /*!
     * Auxiliary function to clone a array/sequence.
     */
    std::shared_ptr<void> clone_sequence(
            TypeKind element_kind,
            const std::shared_ptr<void>& sequence) const noexcept;

    /*!
     * Auxiliary function to compare two sequence values.
     */
    bool compare_sequence_values(
            TypeKind kind,
            std::shared_ptr<void> left,
            std::shared_ptr<void> right) const noexcept;

    /*!
     * Auxiliary function to compare two primitive values.
     */
    bool compare_values(
            TypeKind kind,
            std::shared_ptr<void> left,
            std::shared_ptr<void> right) const noexcept;

    template<TypeKind TK>
    ReturnCode_t get_bitmask_bit(
            TypeForKind<TK>& value,
            MemberId id) noexcept;

    /*!
     * @brief Given a type, returns the enclosing type if exists.
     */
    static traits<DynamicTypeImpl>::ref_type get_enclosing_type(
            traits<DynamicTypeImpl>::ref_type type) noexcept;

    /*!
     * @brief Given a type, returns the enclosing type kind if exists.
     */
    static TypeKind get_enclosing_typekind(
            traits<DynamicTypeImpl>::ref_type type) noexcept;

    template<TypeKind TK >
    ReturnCode_t get_primitive_value(
            TypeKind element_kind,
            std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
            TypeForKind<TK>& value,
            MemberId member_id) noexcept;

    /*!
     * @brief Auxiliary template to retrieve the length of a internal sequence.
     * Only valid for TK_ARRAY and TK_SEQUENCE.
     */
    uint32_t get_sequence_length();

    /*!
     * Auxiliary template with the common code for getting the values of a sequence.
     */
    template<TypeKind TK>
    ReturnCode_t get_sequence_values(
            SequenceTypeForKind<TK>& value,
            MemberId id) noexcept;

    /*!
     * Auxiliary template with the common code for getting the values of a bitmask sequence from a TK_ARRAY or TK_SEQUENCE.
     * @param [in] number_of_elements Number of elements. 0 value means all elements.
     */
    template<TypeKind TK>
    ReturnCode_t get_sequence_values_bitmask(
            MemberId id,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            SequenceTypeForKind<TK>& value,
            size_t number_of_elements) noexcept;

    /*!
     * Auxiliary template with the common code for getting the values of a primitive sequence from a TK_ARRAY or TK_SEQUENCE.
     * @param [in] number_of_elements Number of elements. 0 value means all elements.
     */
    template<TypeKind TK>
    ReturnCode_t get_sequence_values_primitive(
            MemberId id,
            TypeKind element_kind,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            SequenceTypeForKind<TK>& value,
            size_t number_of_elements) noexcept;

    /*!
     * Auxiliary template with the common code for getting the values of a primitive sequence supporting promotion
     * from a TK_ARRAY or TK_SEQUENCE.
     * @param [in] number_of_elements Number of elements. 0 value means all elements.
     */
    template<TypeKind TK, TypeKind ToTK>
    ReturnCode_t get_sequence_values_promoting(
            MemberId id,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            SequenceTypeForKind<TK>& value,
            size_t number_of_elements) noexcept;

    template<TypeKind TK >
    ReturnCode_t get_value(
            TypeForKind<TK>& value,
            MemberId id) noexcept;

    /*!
     * Auxiliary function for setting a bitmask bit.
     */
    template<TypeKind TK>
    ReturnCode_t set_bitmask_bit(
            MemberId id,
            const TypeForKind<TK>& value) noexcept;

    void set_default_value(
            const traits<DynamicTypeMemberImpl>::ref_type member,
            traits<DynamicDataImpl>::ref_type data) noexcept;

    /*!
     * Auxiliary function for setting the discriminator value to a label of the member specified by the MemberId.
     */
    void set_discriminator_value(
            MemberId id) noexcept;

    /*!
     * Auxiliary function to set the discriminator value on already given discriminator DynamicData.
     */
    void set_discriminator_value(
            int32_t new_discriminator_value,
            const traits<DynamicTypeImpl>::ref_type& discriminator_type,
            traits<DynamicDataImpl>::ref_type& data) noexcept;

    /*!
     * Auxiliary function to set a primitive value taking into account if there is promotion of the type.
     */
    template<TypeKind TK>
    ReturnCode_t set_primitive_value(
            const traits<DynamicTypeImpl>::ref_type& element_type,
            std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
            const TypeForKind<TK>& value) noexcept;

    /*!
     * Auxiliary template with the common code for setting the values of a sequence.
     */
    template<TypeKind TK>
    ReturnCode_t set_sequence_values(
            MemberId id,
            const SequenceTypeForKind<TK>& value) noexcept;

    /*!
     * Auxiliary template with the common code for setting the values of a bitmask sequence into a TK_ARRAY or TK_SEQUENCE.
     */
    template<TypeKind TK>
    ReturnCode_t set_sequence_values_bitmask(
            MemberId id,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            const SequenceTypeForKind<TK>& value) noexcept;

    /*!
     * Auxiliary template with the common code for setting the values of a primitive sequence into a TK_ARRAY or TK_SEQUENCE.
     */
    template<TypeKind TK>
    ReturnCode_t set_sequence_values_primitive(
            MemberId id,
            TypeKind element_kind,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            const SequenceTypeForKind<TK>& value) noexcept;

    /*!
     * Auxiliary template with the common code for setting the values of a primitive sequence supporting promotion
     * into a TK_ARRAY or TK_SEQUENCE.
     */
    template<TypeKind TK, TypeKind ToTK>
    ReturnCode_t set_sequence_values_promoting(
            MemberId id,
            std::map<MemberId, std::shared_ptr<void>>::const_iterator value_iterator,
            const SequenceTypeForKind<TK>& value) noexcept;


    /*!
     * Auxiliary function to set the default value,specified by MemberDescriptor::default_value, to primitive types.
     */
    void set_value(
            const ObjectName& value) noexcept;

    /*!
     * Auxiliary template with the common code for setting the value of a primitive type
     */
    template<TypeKind TK>
    ReturnCode_t set_value(
            MemberId id,
            const TypeForKind<TK>& value) noexcept;

    //}}}

    //{{{ Encoding/decoding functions

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const traits<DynamicTypeImpl>::ref_type type,
            size_t& current_alignment) const noexcept;

    bool deserialize(
            eprosima::fastcdr::Cdr& cdr,
            const traits<DynamicTypeImpl>::ref_type type);

    void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const traits<DynamicTypeImpl>::ref_type type) const;

    //}}}

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAIMPL_HPP
