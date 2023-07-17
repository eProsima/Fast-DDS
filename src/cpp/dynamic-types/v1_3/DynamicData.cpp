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

#include <dynamic-types/v1_3/DynamicDataImpl.hpp>
#include <dynamic-types/v1_3/DynamicTypeImpl.hpp>
#include <fastrtps/types/v1_3/DynamicData.hpp>
#include <fastrtps/types/v1_3/DynamicDataFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps::types::v1_3;
using eprosima::fastrtps::types::ReturnCode_t;

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
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
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
        octet& value,
        MemberId id /*= MEMBER_ID_INVALID*/) const noexcept

{
    return DynamicDataImpl::get_implementation(*this).get_byte_value(value, id);
}

ReturnCode_t DynamicData::set_byte_value(
        octet value,
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
        const const char* value,
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




