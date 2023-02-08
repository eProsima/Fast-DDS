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

#include "DynamicDataImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

namespace eprosima {
namespace fastdds  {
namespace dds {

bool DynamicData::operator ==(
        const DynamicData& other) const noexcept
{
    return DynamicDataImpl::get_implementation(*this) == DynamicDataImpl::get_implementation(other);
}

bool DynamicData::operator !=(
        const DynamicData& other) const noexcept
{
    return DynamicDataImpl::get_implementation(*this) != DynamicDataImpl::get_implementation(other);
}

ReturnCode_t DynamicData::get_descriptor(
        MemberDescriptor& value,
        MemberId id) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_descriptor(value, id);
}

const DynamicType& DynamicData::get_type() const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_type().get_interface();
}

MemberId DynamicData::get_member_id_by_name(
        const char* name) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_member_id_by_name(name);
}

MemberId DynamicData::get_member_id_at_index(
        uint32_t index) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_member_id_at_index(index);
}

uint32_t DynamicData::get_item_count() const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_item_count();
}

bool DynamicData::equals(
        const DynamicData& other) const noexcept
{
    return *this == other;
}

ReturnCode_t DynamicData::clear_all_values() noexcept
{
    return DynamicDataImpl::get_implementation(*this).clear_all_values();
}

ReturnCode_t DynamicData::clear_nonkey_values() noexcept
{
    return DynamicDataImpl::get_implementation(*this).clear_nonkey_values();
}

ReturnCode_t DynamicData::clear_value(
        MemberId id) noexcept
{
    return DynamicDataImpl::get_implementation(*this).clear_value(id);
}

DynamicData* DynamicData::loan_value(
        MemberId id) noexcept
{
    auto sp = DynamicDataImpl::get_implementation(*this).loan_value(id);
    if (sp)
    {
        // is kept alive by the outstanding loan (don't use delete_data())
        return &sp->get_interface();
    }

    return nullptr;
}

ReturnCode_t DynamicData::return_loaned_value(
        DynamicData* value) noexcept
{
    if (nullptr == value)
    {
        return RETCODE_BAD_PARAMETER;
    }

    auto sp = DynamicDataImpl::get_implementation(*value).shared_from_this();
    return DynamicDataImpl::get_implementation(*this).return_loaned_value(sp);
}

DynamicData* DynamicData::clone() const noexcept
{
    return DynamicDataFactory::get_instance().create_copy(*this);
}

ReturnCode_t DynamicData::get_int32_value(
        int32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_int32_value(value, id);
}

ReturnCode_t DynamicData::set_int32_value(
        int32_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_int32_value(value, id);
}

ReturnCode_t DynamicData::get_uint32_value(
        uint32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_uint32_value(value, id);
}

ReturnCode_t DynamicData::set_uint32_value(
        uint32_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_uint32_value(value, id);
}

ReturnCode_t DynamicData::get_int16_value(
        int16_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_int16_value(value, id);
}

ReturnCode_t DynamicData::set_int16_value(
        int16_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_int16_value(value, id);
}

ReturnCode_t DynamicData::get_uint16_value(
        uint16_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_uint16_value(value, id);
}

ReturnCode_t DynamicData::set_uint16_value(
        uint16_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_uint16_value(value, id);
}

ReturnCode_t DynamicData::get_int64_value(
        int64_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_int64_value(value, id);
}

ReturnCode_t DynamicData::set_int64_value(
        int64_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_int64_value(value, id);
}

ReturnCode_t DynamicData::get_uint64_value(
        uint64_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_uint64_value(value, id);
}

ReturnCode_t DynamicData::set_uint64_value(
        uint64_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_uint64_value(value, id);
}

ReturnCode_t DynamicData::get_float32_value(
        float& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_float32_value(value, id);
}

ReturnCode_t DynamicData::set_float32_value(
        float value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_float32_value(value, id);
}

ReturnCode_t DynamicData::get_float64_value(
        double& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_float64_value(value, id);
}

ReturnCode_t DynamicData::set_float64_value(
        double value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_float64_value(value, id);
}

ReturnCode_t DynamicData::get_float128_value(
        long double& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_float128_value(value, id);
}

ReturnCode_t DynamicData::set_float128_value(
        long double value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_float128_value(value, id);
}

ReturnCode_t DynamicData::get_char8_value(
        char& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_char8_value(value, id);
}

ReturnCode_t DynamicData::set_char8_value(
        char value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_char8_value(value, id);
}

ReturnCode_t DynamicData::get_char16_value(
        wchar_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_char16_value(value, id);
}

ReturnCode_t DynamicData::set_char16_value(
        wchar_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_char16_value(value, id);
}

ReturnCode_t DynamicData::get_byte_value(
        eprosima::fastrtps::rtps::octet& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_byte_value(value, id);
}

ReturnCode_t DynamicData::set_byte_value(
        eprosima::fastrtps::rtps::octet value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_byte_value(value, id);
}

ReturnCode_t DynamicData::get_int8_value(
        int8_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_int8_value(value, id);
}

ReturnCode_t DynamicData::set_int8_value(
        int8_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_int8_value(value, id);
}

ReturnCode_t DynamicData::get_uint8_value(
        uint8_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_uint8_value(value, id);
}

ReturnCode_t DynamicData::set_uint8_value(
        uint8_t value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_uint8_value(value, id);
}

ReturnCode_t DynamicData::get_bool_value(
        bool& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_bool_value(value, id);
}

ReturnCode_t DynamicData::set_bool_value(
        bool value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_bool_value(value, id);
}

ReturnCode_t DynamicData::set_bool_value(
        bool value,
        const char* name)

{
    return DynamicDataImpl::get_implementation(*this).set_bool_value(value, name);
}

ReturnCode_t DynamicData::get_string_value(
        const char*& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_string_value(value, id);
}

ReturnCode_t DynamicData::set_string_value(
        const char* value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_string_value(value, id);
}

ReturnCode_t DynamicData::get_wstring_value(
        const wchar_t*& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_wstring_value(value, id);
}

ReturnCode_t DynamicData::set_wstring_value(
        const wchar_t* value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_wstring_value(value, id);
}

ReturnCode_t DynamicData::get_enum_value(
        const char*& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_enum_value(value, id);
}

ReturnCode_t DynamicData::set_enum_value(
        const char* value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_enum_value(value, id);
}

ReturnCode_t DynamicData::get_enum_value(
        uint32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_enum_value(value, id);
}

ReturnCode_t DynamicData::set_enum_value(
        const uint32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept

{
    return DynamicDataImpl::get_implementation(*this).set_enum_value(value, id);
}

ReturnCode_t DynamicData::get_bitmask_value(
        uint64_t& value) const
{
    return DynamicDataImpl::get_implementation(*this).get_bitmask_value(value);
}

uint64_t DynamicData::get_bitmask_value() const
{
    return DynamicDataImpl::get_implementation(*this).get_bitmask_value();
}

ReturnCode_t DynamicData::set_bitmask_value(
        uint64_t value)
{
    return DynamicDataImpl::get_implementation(*this).set_bitmask_value(value);
}

ReturnCode_t DynamicData::get_complex_value(
        const DynamicData*& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept
{
    try
    {
        auto sp = DynamicDataImpl::get_implementation(*this).get_complex_value(id);
        value = sp ? &sp->get_interface() : nullptr;
    }
    catch (ReturnCode_t ec)
    {
        return ec;
    }

    return {};
}

ReturnCode_t DynamicData::set_complex_value(
        const DynamicData& value,
        MemberId id /*= MEMBER_ID_INVALID*/) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_complex_value(DynamicDataImpl::get_implementation(value), id);
}

int32_t DynamicData::get_int32_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_int32_value(id);
}

uint32_t DynamicData::get_uint32_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_uint32_value(id);
}

int16_t DynamicData::get_int16_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_int16_value(id);
}

uint16_t DynamicData::get_uint16_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_uint16_value(id);
}

int64_t DynamicData::get_int64_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_int64_value(id);
}

uint64_t DynamicData::get_uint64_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_uint64_value(id);
}

float DynamicData::get_float32_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_float32_value(id);
}

double DynamicData::get_float64_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_float64_value(id);
}

long double DynamicData::get_float128_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_float128_value(id);
}

char DynamicData::get_char8_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_char8_value(id);
}

wchar_t DynamicData::get_char16_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_char16_value(id);
}

eprosima::fastrtps::rtps::octet DynamicData::get_byte_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_byte_value(id);
}

int8_t DynamicData::get_int8_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_int8_value(id);
}

uint8_t DynamicData::get_uint8_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_uint8_value(id);
}

bool DynamicData::get_bool_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    return DynamicDataImpl::get_implementation(*this).get_bool_value(id);
}

bool DynamicData::get_bool_value(
        const char* name) const
{
    return DynamicDataImpl::get_implementation(*this).get_bool_value(name);
}

const char* DynamicData::get_string_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    const char* str = nullptr;
    auto ret = DynamicDataImpl::get_implementation(*this).get_string_value(str, id);
    if (!ret)
    {
        throw ret;
    }
    return str;
}

const wchar_t* DynamicData::get_wstring_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    const wchar_t* str = nullptr;
    auto ret = DynamicDataImpl::get_implementation(*this).get_wstring_value(str, id);
    if (!ret)
    {
        throw ret;
    }
    return str;
}

const char* DynamicData::get_enum_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    const char* str = nullptr;
    auto ret = DynamicDataImpl::get_implementation(*this).get_enum_value(str, id);
    if (!ret)
    {
        throw ret;
    }
    return str;
}

ReturnCode_t DynamicData::get_union_label(
        uint64_t& value) const
{
    return DynamicDataImpl::get_implementation(*this).get_union_label(value);
}

uint64_t DynamicData::get_union_label() const
{
    return DynamicDataImpl::get_implementation(*this).get_union_label();
}

MemberId DynamicData::get_discriminator_value() const
{
    return DynamicDataImpl::get_implementation(*this).get_discriminator_value();
}

ReturnCode_t DynamicData::get_discriminator_value(
        MemberId& id) const noexcept
{
    return DynamicDataImpl::get_implementation(*this).get_discriminator_value(id);
}

ReturnCode_t DynamicData::set_discriminator_value(
        MemberId value) noexcept
{
    return DynamicDataImpl::get_implementation(*this).set_discriminator_value(value);
}

MemberId DynamicData::get_array_index(
        const uint32_t* pos,
        uint32_t size) const noexcept
{
    std::vector<uint32_t> v(pos, pos + size);
    return DynamicDataImpl::get_implementation(*this).get_array_index(v);
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData& key,
        MemberId& outKeyId,
        MemberId& outValueId)
{
    const DynamicDataImpl& keyimpl = DynamicDataImpl::get_implementation(key);
    return DynamicDataImpl::get_implementation(*this).insert_map_data(keyimpl, outKeyId, outValueId);
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData& key,
        const DynamicData& value,
        MemberId& outKey,
        MemberId& outValue)
{
    const DynamicDataImpl& keyimpl = DynamicDataImpl::get_implementation(key),
            valueimpl = DynamicDataImpl::get_implementation(value);
    return DynamicDataImpl::get_implementation(*this).insert_map_data(keyimpl, valueimpl, outKey, outValue);
}

ReturnCode_t DynamicData::remove_map_data(
        MemberId keyId)
{
    return DynamicDataImpl::get_implementation(*this).remove_map_data(keyId);
}

void DynamicData::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    DynamicDataImpl::get_implementation(*this).serialize(cdr);
}

bool DynamicData::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    return DynamicDataImpl::get_implementation(*this).deserialize(cdr);
}

void DynamicData::serializeKey(
        eprosima::fastcdr::Cdr& cdr) const
{
    DynamicDataImpl::get_implementation(*this).serializeKey(cdr);
}

size_t DynamicData::getCdrSerializedSize(
        const DynamicData& data,
        size_t current_alignment /*= 0*/)
{
    return DynamicDataImpl::getCdrSerializedSize(
        DynamicDataImpl::get_implementation(data),
        current_alignment);
}

size_t DynamicData::getEmptyCdrSerializedSize(
        const DynamicType& type,
        size_t current_alignment /*= 0*/)
{
    return DynamicDataImpl::getEmptyCdrSerializedSize(
        DynamicTypeImpl::get_implementation(type),
        current_alignment);
}

size_t DynamicData::getKeyMaxCdrSerializedSize(
        const DynamicType& type,
        size_t current_alignment /*= 0*/)
{
    return DynamicDataImpl::getKeyMaxCdrSerializedSize(
        DynamicTypeImpl::get_implementation(type),
        current_alignment);
}

size_t DynamicData::getMaxCdrSerializedSize(
        const DynamicType& type,
        size_t current_alignment /*= 0*/)
{
    return DynamicDataImpl::getMaxCdrSerializedSize(
        DynamicTypeImpl::get_implementation(type),
        current_alignment);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
