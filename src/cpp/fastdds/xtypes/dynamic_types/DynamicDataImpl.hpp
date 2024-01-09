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

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include <map>
#include <vector>

#include "DynamicTypeImpl.hpp"

namespace eprosima {

namespace fastcdr {
class Cdr;
class CdrSizeCalculator;
} // namespace fastcdr

namespace fastdds {
namespace dds {

class DynamicDataImpl : public traits<DynamicData>::base_type
{
    traits<DynamicTypeImpl>::ref_type type_;

    std::map<MemberId, std::shared_ptr<void>> value_;

    std::vector<MemberId> loaned_values_;

public:

    DynamicDataImpl(
            traits<DynamicType>::ref_type type) noexcept;

    traits<DynamicType>::ref_type type() noexcept override;

    ReturnCode_t get_descriptor(
            traits<MemberDescriptor>::ref_type& value,
            MemberId id) noexcept override;

    bool equals(
            traits<DynamicData>::ref_type other) noexcept override;

    MemberId get_member_id_by_name(
            const ObjectName& name) noexcept override;

    MemberId get_member_id_at_index(
            uint32_t index) noexcept override;

    uint32_t get_item_count() noexcept override;

    ReturnCode_t clear_all_values() noexcept override;

    ReturnCode_t clear_nonkey_values() noexcept override;

    ReturnCode_t clear_value(
            MemberId id) noexcept override;

    traits<DynamicData>::ref_type loan_value(
            MemberId id) noexcept override;

    ReturnCode_t return_loaned_value(
            traits<DynamicData>::ref_type value) noexcept override;

    traits<DynamicData>::ref_type clone() noexcept override;

    ReturnCode_t get_int32_value(
            int32_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int32_value(
            MemberId id,
            int32_t value) noexcept override;

    ReturnCode_t get_uint32_value(
            uint32_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint32_value(
            MemberId id,
            uint32_t value) noexcept override;

    ReturnCode_t get_int8_value(
            int8_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int8_value(
            MemberId id,
            int8_t value) noexcept override;

    ReturnCode_t get_uint8_value(
            uint8_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint8_value(
            MemberId id,
            uint8_t value) noexcept override;

    ReturnCode_t get_int16_value(
            int16_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int16_value(
            MemberId id,
            int16_t value) noexcept override;

    ReturnCode_t get_uint16_value(
            uint16_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint16_value(
            MemberId id,
            uint16_t value) noexcept override;

    ReturnCode_t get_int64_value(
            int64_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int64_value(
            MemberId id,
            int64_t value) noexcept override;

    ReturnCode_t get_uint64_value(
            uint64_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint64_value(
            MemberId id,
            uint64_t value) noexcept override;

    ReturnCode_t get_float32_value(
            float& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float32_value(
            MemberId id,
            float value) noexcept override;

    ReturnCode_t get_float64_value(
            double& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float64_value(
            MemberId id,
            double value) noexcept override;

    ReturnCode_t get_float128_value(
            long double& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float128_value(
            MemberId id,
            long double value) noexcept override;

    ReturnCode_t get_char8_value(
            char& value,
            MemberId id) noexcept override;

    ReturnCode_t set_char8_value(
            MemberId id,
            char value) noexcept override;

    ReturnCode_t get_char16_value(
            wchar_t& value,
            MemberId id) noexcept override;

    ReturnCode_t set_char16_value(
            MemberId id,
            wchar_t value) noexcept override;

    ReturnCode_t get_byte_value(
            eprosima::fastrtps::rtps::octet& value,
            MemberId id) noexcept override;

    ReturnCode_t set_byte_value(
            MemberId id,
            eprosima::fastrtps::rtps::octet value) noexcept override;

    ReturnCode_t get_boolean_value(
            bool& value,
            MemberId id) noexcept override;

    ReturnCode_t set_boolean_value(
            MemberId id,
            bool value) noexcept override;

    ReturnCode_t get_string_value(
            std::string& value,
            MemberId id) noexcept override;

    ReturnCode_t set_string_value(
            MemberId id,
            const std::string& value) noexcept override;

    ReturnCode_t get_wstring_value(
            std::wstring& value,
            MemberId id) noexcept override;

    ReturnCode_t set_wstring_value(
            MemberId id,
            const std::wstring& value) noexcept override;

    ReturnCode_t get_complex_value(
            traits<DynamicData>::ref_type value,
            MemberId id) noexcept override;

    ReturnCode_t set_complex_value(
            MemberId id,
            traits<DynamicData>::ref_type value) noexcept override;

    ReturnCode_t get_int32_values(
            Int32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int32_values(
            MemberId id,
            const Int32Seq& value) noexcept override;

    ReturnCode_t get_uint32_values(
            UInt32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint32_values(
            MemberId id,
            const UInt32Seq& value) noexcept override;

    ReturnCode_t get_int8_values(
            Int8Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int8_values(
            MemberId id,
            const Int8Seq& value) noexcept override;

    ReturnCode_t get_uint8_values(
            UInt8Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint8_values(
            MemberId id,
            const UInt8Seq& value) noexcept override;

    ReturnCode_t get_int16_values(
            Int16Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int16_values(
            MemberId id,
            const Int16Seq& value) noexcept override;

    ReturnCode_t get_uint16_values(
            UInt16Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint16_values(
            MemberId id,
            const UInt16Seq& value) noexcept override;

    ReturnCode_t get_int64_values(
            Int64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_int64_values(
            MemberId id,
            const Int64Seq& value) noexcept override;

    ReturnCode_t get_uint64_values(
            UInt64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_uint64_values(
            MemberId id,
            const UInt64Seq& value) noexcept override;

    ReturnCode_t get_float32_values(
            Float32Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float32_values(
            MemberId id,
            const Float32Seq& value) noexcept override;

    ReturnCode_t get_float64_values(
            Float64Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float64_values(
            MemberId id,
            const Float64Seq& value) noexcept override;

    ReturnCode_t get_float128_values(
            Float128Seq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_float128_values(
            MemberId id,
            const Float128Seq& value) noexcept override;

    ReturnCode_t get_char8_values(
            CharSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_char8_values(
            MemberId id,
            const CharSeq& value) noexcept override;

    ReturnCode_t get_char16_values(
            WcharSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_char16_values(
            MemberId id,
            const WcharSeq& value) noexcept override;

    ReturnCode_t get_byte_values(
            ByteSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_byte_values(
            MemberId id,
            const ByteSeq& value) noexcept override;

    ReturnCode_t get_boolean_values(
            BooleanSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_boolean_values(
            MemberId id,
            const BooleanSeq& value) noexcept override;

    ReturnCode_t get_string_values(
            StringSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_string_values(
            MemberId id,
            const StringSeq& value) noexcept override;

    ReturnCode_t get_wstring_values(
            WstringSeq& value,
            MemberId id) noexcept override;

    ReturnCode_t set_wstring_values(
            MemberId id,
            const WstringSeq& value) noexcept override;

    void serialize(
            eprosima::fastcdr::Cdr& cdr) const noexcept;

    bool deserialize(
            eprosima::fastcdr::Cdr& cdr) noexcept;

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            size_t& current_alignment) const noexcept
    {
        return calculate_serialized_size(calculator, type_, current_alignment);
    }

    void serialize_key(
            eprosima::fastcdr::Cdr& cdr) const noexcept;

    static size_t get_key_max_cdr_serialized_size(
            traits<DynamicType>::ref_type type,
            size_t current_alignment = 0);

    static size_t get_max_cdr_serialized_size(
            traits<DynamicType>::ref_type type,
            size_t current_alignment = 0);

protected:

    traits<DynamicDataImpl>::ref_type _this();

private:

    void add_value(
            TypeKind kind,
            MemberId id) noexcept;

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            traits<DynamicTypeImpl>::ref_type type,
            size_t& current_alignment) const noexcept;

    ReturnCode_t clear_data() noexcept;

    bool compare_values(
            TypeKind kind,
            std::shared_ptr<void> left,
            std::shared_ptr<void> right) const noexcept;

    bool deserialize(
            eprosima::fastcdr::Cdr& cdr,
            traits<DynamicTypeImpl>::ref_type type) noexcept;

    void set_value(
            const ObjectName& value,
            MemberId id) noexcept;

    void set_default_value(
            MemberId id) noexcept;

    void serialize(
            eprosima::fastcdr::Cdr& cdr,
            traits<DynamicTypeImpl>::ref_type type) const noexcept;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAIMPL_HPP
