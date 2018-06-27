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

#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/MemberDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicData::DynamicData()
{
}

ResponseCode DynamicData::get_descriptor(MemberDescriptor& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_descriptor(MemberId id, const MemberDescriptor& value)
{
    return ResponseCode::RETCODE_OK;
}

bool DynamicData::equals(const DynamicData& other)
{
    return ResponseCode::RETCODE_OK;
}

MemberId DynamicData::get_member_id_by_name(const std::string& name)
{
    return 0;
}

MemberId DynamicData::get_member_id_at_index(uint32_t index)
{
    return 0;
}

uint32_t DynamicData::get_item_count()
{
    return 0;
}

ResponseCode DynamicData::clear_all_values()
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_nonkey_values()
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_value(MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

DynamicData* DynamicData::loan_value(MemberId id)
{
    return nullptr;
}

ResponseCode DynamicData::return_loaned_value(const DynamicData& value)
{
    return ResponseCode::RETCODE_OK;
}

DynamicData* DynamicData::clone()
{
    return this; //TODO
}

ResponseCode DynamicData::get_int32_value(int32_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int32_value(MemberId id, int32_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint32_value(uint32_t value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint32_value(MemberId id, uint32_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_int16_value(int16_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int16_value(MemberId id, int16_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint16_value(uint16_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint16_value(MemberId id, uint16_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_int64_value(int64_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int64_value(MemberId id, int64_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint64_value(uint64_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint64_value(MemberId id, uint64_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float32_value(float& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float32_value(MemberId id, float value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float64_value(double& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float64_value(MemberId id, double value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float128_value(long double& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float128_value(MemberId id, long double value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_char8_value(char& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_char8_value(MemberId id, char value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_char16_value(wchar_t& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_char16_value(MemberId id, wchar_t value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_byte_value(octet& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_byte_value(MemberId id, octet value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_boolean_value(bool& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_boolean_value(MemberId id, bool value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_string_value(std::string& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_string_value(MemberId id, std::string value)
{
    return ResponseCode::RETCODE_OK;
}

//ResponseCode DynamicData::get_wstring_value( inout wstring value, MemberId id)
//{
//return ResponseCode::RETCODE_OK;
//}

//ResponseCode DynamicData::set_wstring_value(MemberId id, in wstring value)
//{
//return ResponseCode::RETCODE_OK;
//}


ResponseCode DynamicData::get_complex_value(DynamicData& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_complex_value(MemberId id, DynamicData value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_int32_values(std::vector<int32_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int32_values(MemberId id, const std::vector<int32_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint32_values(std::vector<uint32_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint32_values(MemberId id, const std::vector<uint32_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_int16_values(std::vector<int16_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int16_values(MemberId id, const std::vector<int16_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint16_values(std::vector<uint16_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint16_values(MemberId id, const std::vector<uint16_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_int64_values(std::vector<int64_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_int64_values(MemberId id, const std::vector<int64_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_uint64_values(std::vector<uint64_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_uint64_values(MemberId id, const std::vector<uint64_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float32_values(std::vector<float>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float32_values(MemberId id, const std::vector<float>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float64_values(std::vector<double>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float64_values(MemberId id, const std::vector<double>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_float128_values(std::vector<long double>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_float128_values(MemberId id, const std::vector<long double>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_char8_values(std::vector<char>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_char8_values(MemberId id, const std::vector<char>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_char16_values(std::vector<wchar_t>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_char16_values(MemberId id, const std::vector<wchar_t>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_byte_values(std::vector<octet>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_byte_values(MemberId id, const std::vector<octet>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_boolean_values(std::vector<bool>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_boolean_values(MemberId id, const std::vector<bool>& value)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::get_string_values(std::vector<std::string>& value, MemberId id)
{
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::set_string_values(MemberId id, const std::vector<std::string>& value)
{
    return ResponseCode::RETCODE_OK;
}

//ResponseCode get_wstring_values( inout WstringSeq value, MemberId id)
//{
//return ResponseCode::RETCODE_OK;
//}

//ResponseCode set_wstring_values(MemberId id, in WstringSeq value)
//{
//return ResponseCode::RETCODE_OK;
//}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
