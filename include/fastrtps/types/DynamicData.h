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

#ifndef TYPES_DYNAMIC_DATA_H
#define TYPES_DYNAMIC_DATA_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/MemberId.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;
class MemberDescriptor;

class DynamicData
{
public:
    DynamicData();

	ResponseCode get_descriptor(MemberDescriptor& value, MemberId id);
	ResponseCode set_descriptor(MemberId id, const MemberDescriptor& value);

	bool equals(const DynamicData& other);

	MemberId get_member_id_by_name(const std::string& name);
	MemberId get_member_id_at_index(uint32_t index);

	uint32_t get_item_count();

	ResponseCode clear_all_values();
	ResponseCode clear_nonkey_values();
	ResponseCode clear_value(MemberId id);

	DynamicData* loan_value(MemberId id);
	ResponseCode return_loaned_value(const DynamicData& value);

	DynamicData* clone();

	ResponseCode get_int32_value(int32_t& value, MemberId id);
	ResponseCode set_int32_value(MemberId id, int32_t value);
	ResponseCode get_uint32_value(uint32_t value, MemberId id);
	ResponseCode set_uint32_value(MemberId id, uint32_t value);
	ResponseCode get_int16_value(int16_t& value, MemberId id);
	ResponseCode set_int16_value(MemberId id, int16_t value);
	ResponseCode get_uint16_value(uint16_t& value, MemberId id);
	ResponseCode set_uint16_value(MemberId id, uint16_t value);
	ResponseCode get_int64_value(int64_t& value, MemberId id);
	ResponseCode set_int64_value(MemberId id, int64_t value);
	ResponseCode get_uint64_value(uint64_t& value, MemberId id);
	ResponseCode set_uint64_value(MemberId id, uint64_t value);
	ResponseCode get_float32_value(float& value, MemberId id);
	ResponseCode set_float32_value(MemberId id, float value);
	ResponseCode get_float64_value(double& value, MemberId id);
	ResponseCode set_float64_value(MemberId id, double value);
	ResponseCode get_float128_value(long double& value, MemberId id);
	ResponseCode set_float128_value(MemberId id, long double value);
	ResponseCode get_char8_value(char& value, MemberId id);
	ResponseCode set_char8_value(MemberId id, char value);
	ResponseCode get_char16_value(wchar_t& value, MemberId id);
	ResponseCode set_char16_value(MemberId id, wchar_t value);
	ResponseCode get_byte_value(octet& value, MemberId id);
	ResponseCode set_byte_value(MemberId id, octet value);
	ResponseCode get_boolean_value(bool& value, MemberId id);
	ResponseCode set_boolean_value(MemberId id, bool value);
	ResponseCode get_string_value(std::string& value, MemberId id);
	ResponseCode set_string_value(MemberId id, std::string value);
	//ResponseCode get_wstring_value( inout wstring value, MemberId id);
	//ResponseCode set_wstring_value(MemberId id, in wstring value);

	ResponseCode get_complex_value(DynamicData& value, MemberId id);
	ResponseCode set_complex_value(MemberId id, DynamicData value);

	ResponseCode get_int32_values(std::vector<int32_t>& value, MemberId id);
	ResponseCode set_int32_values(MemberId id, const std::vector<int32_t>& value);
	ResponseCode get_uint32_values(std::vector<uint32_t>& value, MemberId id);
	ResponseCode set_uint32_values(MemberId id, const std::vector<uint32_t>& value);
	ResponseCode get_int16_values(std::vector<int16_t>& value, MemberId id);
	ResponseCode set_int16_values(MemberId id, const std::vector<int16_t>& value);
	ResponseCode get_uint16_values(std::vector<uint16_t>& value, MemberId id);
	ResponseCode set_uint16_values(MemberId id, const std::vector<uint16_t>& value);
	ResponseCode get_int64_values(std::vector<int64_t>& value, MemberId id);
	ResponseCode set_int64_values(MemberId id, const std::vector<int64_t>& value);
	ResponseCode get_uint64_values(std::vector<uint64_t>& value, MemberId id);
	ResponseCode set_uint64_values(MemberId id, const std::vector<uint64_t>& value);
	ResponseCode get_float32_values(std::vector<float>& value, MemberId id);
	ResponseCode set_float32_values(MemberId id, const std::vector<float>& value);
	ResponseCode get_float64_values(std::vector<double>& value, MemberId id);
	ResponseCode set_float64_values(MemberId id, const std::vector<double>& value);
	ResponseCode get_float128_values(std::vector<long double>& value, MemberId id);
	ResponseCode set_float128_values(MemberId id, const std::vector<long double>& value);
	ResponseCode get_char8_values(std::vector<char>& value, MemberId id);
	ResponseCode set_char8_values(MemberId id, const std::vector<char>& value);
	ResponseCode get_char16_values(std::vector<wchar_t>& value, MemberId id);
	ResponseCode set_char16_values(MemberId id, const std::vector<wchar_t>& value);
	ResponseCode get_byte_values(std::vector<octet>& value, MemberId id);
	ResponseCode set_byte_values(MemberId id, const std::vector<octet>& value);
	ResponseCode get_boolean_values(std::vector<bool>& value, MemberId id);
	ResponseCode set_boolean_values(MemberId id, const std::vector<bool>& value);
	ResponseCode get_string_values(std::vector<std::string>& value, MemberId id);
	ResponseCode set_string_values(MemberId id, const std::vector<std::string>& value);
	//ResponseCode get_wstring_values( inout WstringSeq value, MemberId id);
	//ResponseCode set_wstring_values(MemberId id, in WstringSeq value);
protected:
	DynamicType* mType;
};


} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_DATA_H
