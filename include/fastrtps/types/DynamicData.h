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

    ResponseCode GetDescriptor(MemberDescriptor& value, MemberId id);
    ResponseCode SetDescriptor(MemberId id, const MemberDescriptor* value);

    RTPS_DllAPI ResponseCode ClearAllValues();
    RTPS_DllAPI ResponseCode ClearNonkeyValues();
    RTPS_DllAPI ResponseCode ClearValue(MemberId id);

    RTPS_DllAPI DynamicData* Clone() const;
    RTPS_DllAPI bool Equals(const DynamicData* other) const;
    RTPS_DllAPI TypeKind GetKind();
    RTPS_DllAPI uint32_t GetItemCount() const;
    RTPS_DllAPI std::string GetName();

    RTPS_DllAPI MemberId GetMemberIdByName(const std::string& name) const;
    RTPS_DllAPI MemberId GetMemberIdAtIndex(uint32_t index) const;

    RTPS_DllAPI DynamicData* LoanValue(MemberId id);
    RTPS_DllAPI ResponseCode ReturnLoanedValue(const DynamicData* value);

    RTPS_DllAPI MemberId GetArrayIndex(const std::vector<uint32_t>& position);
    RTPS_DllAPI ResponseCode InsertSequenceData(MemberId& outId);
    RTPS_DllAPI ResponseCode RemoveSequenceData(MemberId id);
    RTPS_DllAPI ResponseCode ClearData();

    RTPS_DllAPI ResponseCode InsertArrayData(MemberId indexId);
    RTPS_DllAPI ResponseCode RemoveArrayData(MemberId indexId);

    RTPS_DllAPI ResponseCode InsertMapData(DynamicData* key, MemberId& outKeyId, MemberId& outValueId);
    RTPS_DllAPI ResponseCode RemoveMapData(MemberId keyId);

    RTPS_DllAPI ResponseCode GetInt32Value(int32_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetInt32Value(MemberId id, int32_t value);
	RTPS_DllAPI ResponseCode GetUint32Value(uint32_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetUint32Value(MemberId id, uint32_t value);
	RTPS_DllAPI ResponseCode GetInt16Value(int16_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetInt16Value(MemberId id, int16_t value);
	RTPS_DllAPI ResponseCode GetUint16Value(uint16_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetUint16Value(MemberId id, uint16_t value);
	RTPS_DllAPI ResponseCode GetInt64Value(int64_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetInt64Value(MemberId id, int64_t value);
	RTPS_DllAPI ResponseCode GetUint64Value(uint64_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetUint64Value(MemberId id, uint64_t value);
	RTPS_DllAPI ResponseCode GetFloat32Value(float& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetFloat32Value(MemberId id, float value);
	RTPS_DllAPI ResponseCode GetFloat64Value(double& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetFloat64Value(MemberId id, double value);
	RTPS_DllAPI ResponseCode GetFloat128Value(long double& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetFloat128Value(MemberId id, long double value);
	RTPS_DllAPI ResponseCode GetChar8Value(char& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetChar8Value(MemberId id, char value);
	RTPS_DllAPI ResponseCode GetChar16Value(wchar_t& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetChar16Value(MemberId id, wchar_t value);
	RTPS_DllAPI ResponseCode GetByteValue(octet& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetByteValue(MemberId id, octet value);
	RTPS_DllAPI ResponseCode GetBoolValue(bool& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetBoolValue(MemberId id, bool value);
	RTPS_DllAPI ResponseCode GetStringValue(std::string& value, MemberId id) const;
	RTPS_DllAPI ResponseCode SetStringValue(MemberId id, const std::string& value);
    RTPS_DllAPI ResponseCode GetWstringValue(std::wstring& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetWstringValue(MemberId id, const std::wstring& value);

    RTPS_DllAPI ResponseCode GetEnumValue(std::string& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetEnumValue(MemberId id, const std::string& value);
    RTPS_DllAPI ResponseCode GetBitmaskValue(const std::string& name, bool& value) const;
    RTPS_DllAPI ResponseCode SetBitmaskValue(bool value, const std::string& name);

    RTPS_DllAPI ResponseCode GetComplexValue(DynamicData** value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetComplexValue(MemberId id, DynamicData* value);

    RTPS_DllAPI ResponseCode GetUnionLabel(uint64_t& value) const;

    // Serializes and deserializes the Dynamic Data.
    bool deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getCdrSerializedSize(const DynamicData* data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;

protected:

    DynamicData(DynamicType* pType);

    void AddValue(TypeKind kind, MemberId id);

    void CreateMembers(DynamicType* pType);

    void Clean();

    void* CloneValue(MemberId id, TypeKind kind) const;

    bool CompareValues(TypeKind kind, void* left, void* right) const;

    void SetDefaultValue(MemberId id);

    void SetTypeName(const std::string& name);

    ResponseCode SetUnionId(MemberId id);

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
    uint64_t mUnionLabel;
    MemberId mUnionId;

    friend class DynamicDataFactory;
};


} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_DATA_H
