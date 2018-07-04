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

#define DYNAMIC_TYPES_CHECKING

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;
class MemberDescriptor;

class DynamicData
{
public:
    DynamicData();
    ~DynamicData();

    ResponseCode get_descriptor(MemberDescriptor& value, MemberId id);
    ResponseCode set_descriptor(MemberId id, const MemberDescriptor* value);

    RTPS_DllAPI bool equals(const DynamicData* other);

    RTPS_DllAPI MemberId get_member_id_by_name(const std::string& name);
    RTPS_DllAPI MemberId get_member_id_at_index(uint32_t index);

    RTPS_DllAPI uint32_t get_item_count();

	RTPS_DllAPI ResponseCode clear_all_values();
	RTPS_DllAPI ResponseCode clear_nonkey_values();
	RTPS_DllAPI ResponseCode clear_value(MemberId id);

    RTPS_DllAPI DynamicData* loan_value(MemberId id);
    RTPS_DllAPI ResponseCode return_loaned_value(const DynamicData* value);

    RTPS_DllAPI DynamicData* clone() const;

    RTPS_DllAPI ResponseCode insert_new_data(MemberId& outId);
    RTPS_DllAPI ResponseCode remove_data(MemberId id);
    RTPS_DllAPI ResponseCode clear_data();

    RTPS_DllAPI ResponseCode get_int32_value(int32_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_int32_value(MemberId id, int32_t value);
	RTPS_DllAPI ResponseCode get_uint32_value(uint32_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_uint32_value(MemberId id, uint32_t value);
	RTPS_DllAPI ResponseCode get_int16_value(int16_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_int16_value(MemberId id, int16_t value);
	RTPS_DllAPI ResponseCode get_uint16_value(uint16_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_uint16_value(MemberId id, uint16_t value);
	RTPS_DllAPI ResponseCode get_int64_value(int64_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_int64_value(MemberId id, int64_t value);
	RTPS_DllAPI ResponseCode get_uint64_value(uint64_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_uint64_value(MemberId id, uint64_t value);
	RTPS_DllAPI ResponseCode get_float32_value(float& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_float32_value(MemberId id, float value);
	RTPS_DllAPI ResponseCode get_float64_value(double& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_float64_value(MemberId id, double value);
	RTPS_DllAPI ResponseCode get_float128_value(long double& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_float128_value(MemberId id, long double value);
	RTPS_DllAPI ResponseCode get_char8_value(char& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_char8_value(MemberId id, char value);
	RTPS_DllAPI ResponseCode get_char16_value(wchar_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_char16_value(MemberId id, wchar_t value);
	RTPS_DllAPI ResponseCode get_byte_value(octet& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_byte_value(MemberId id, octet value);
	RTPS_DllAPI ResponseCode get_bool_value(bool& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_bool_value(MemberId id, bool value);
	RTPS_DllAPI ResponseCode get_string_value(std::string& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_string_value(MemberId id, const std::string& value);
	RTPS_DllAPI ResponseCode get_wstring_value(std::wstring& value, MemberId id) const;
	RTPS_DllAPI ResponseCode set_wstring_value(MemberId id, const std::wstring& value);

    //ResponseCode get_complex_value(DynamicData* value, MemberId id);
	//ResponseCode set_complex_value(MemberId id, const DynamicData* value);

protected:

    DynamicData(DynamicType* pType);

    void AddValue(TypeKind kind, MemberId id);

    void Clean();

    void* CloneValue(MemberId id, TypeKind kind) const;

    bool CompareValues(TypeKind kind, void* left, void* right);

    void SetDefaultValue(MemberId id);

    void SetTypeName(const std::string& name);

    void SortMemberIds(MemberId startId);

	DynamicType* mType;
    std::map<MemberId, MemberDescriptor*> mDescriptors;

#ifdef DYNAMIC_TYPES_CHECKING
    int32_t mInt32Value;
    uint32_t mUInt32Value;
    int16_t mInt16Value;
    uint16_t mUInt16Value;
    int64_t mInt64Value;
    uint64_t mUInt64Value;
    float mFloat32Value;
    double mFloat64Value;
    long double mFloat128Value;
    char mChar8Value;
    wchar_t mChar16Value;
    octet mByteValue;
    bool mBoolValue;
    std::string mStringValue;
    std::wstring mWStringValue;
    std::map<MemberId, DynamicData*> mComplexValues;
#else
    std::map<MemberId, void*> mValues;
#endif
    std::vector<MemberId> mLoanedValues;
    bool mIsKeyElement;
    uint32_t mItemCount;

    friend class DynamicDataFactory;
};


} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_DATA_H
