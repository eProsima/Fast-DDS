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
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicTypePtr.h>

//#define DYNAMIC_TYPES_CHECKING

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicType;
class MemberDescriptor;

class DynamicData
{
public:

    ResponseCode GetDescriptor(MemberDescriptor& value, MemberId id);
    ResponseCode SetDescriptor(MemberId id, const MemberDescriptor* value);

    RTPS_DllAPI ResponseCode ClearAllValues();
    RTPS_DllAPI ResponseCode ClearNonkeyValues();
    RTPS_DllAPI ResponseCode ClearValue(MemberId id);

    RTPS_DllAPI bool Equals(const DynamicData* other) const;
    RTPS_DllAPI inline TypeKind GetKind() const;
    RTPS_DllAPI uint32_t GetItemCount() const;
    RTPS_DllAPI std::string GetName();

    RTPS_DllAPI MemberId GetMemberIdByName(const std::string& name) const;
    RTPS_DllAPI MemberId GetMemberIdAtIndex(uint32_t index) const;

    RTPS_DllAPI DynamicData* LoanValue(MemberId id);
    RTPS_DllAPI ResponseCode ReturnLoanedValue(const DynamicData* value);

    RTPS_DllAPI MemberId GetArrayIndex(const std::vector<uint32_t>& position);
    RTPS_DllAPI ResponseCode InsertSequenceData(MemberId& outId);
    RTPS_DllAPI ResponseCode InsertInt32Value(int32_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertUint32Value(uint32_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertInt16Value(int16_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertUint16Value(uint16_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertInt64Value(int64_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertUint64Value(uint64_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertFloat32Value(float value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertFloat64Value(double value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertFloat128Value(long double value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertChar8Value(char value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertChar16Value(wchar_t value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertByteValue(octet value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertBoolValue(bool value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertStringValue(const std::string& value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertWstringValue(const std::wstring& value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertEnumValue(const std::string& value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertComplexValue(const DynamicData* value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertComplexValue(DynamicData* value, MemberId& outId);
    RTPS_DllAPI ResponseCode InsertComplexValue(DynamicData_ptr value, MemberId& outId);

    RTPS_DllAPI ResponseCode RemoveSequenceData(MemberId id);
    RTPS_DllAPI ResponseCode ClearData();

    RTPS_DllAPI ResponseCode ClearArrayData(MemberId indexId);

    RTPS_DllAPI ResponseCode InsertMapData(const DynamicData* key, MemberId& outKeyId, MemberId& outValueId);
    RTPS_DllAPI ResponseCode InsertMapData(const DynamicData* key, DynamicData* value, MemberId& outKey, MemberId& outValue);
    RTPS_DllAPI ResponseCode InsertMapData(const DynamicData* key, const DynamicData* value, MemberId& outKey, MemberId& outValue);
    RTPS_DllAPI ResponseCode InsertMapData(const DynamicData* key, DynamicData_ptr value, MemberId& outKey, MemberId& outValue);
    RTPS_DllAPI ResponseCode RemoveMapData(MemberId keyId);

    RTPS_DllAPI ResponseCode GetInt32Value(int32_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetInt32Value(int32_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetUint32Value(uint32_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetUint32Value(uint32_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetInt16Value(int16_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetInt16Value(int16_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetUint16Value(uint16_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetUint16Value(uint16_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetInt64Value(int64_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetInt64Value(int64_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetUint64Value(uint64_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetUint64Value(uint64_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetFloat32Value(float& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetFloat32Value(float value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetFloat64Value(double& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetFloat64Value(double value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetFloat128Value(long double& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetFloat128Value(long double value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetChar8Value(char& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetChar8Value(char value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetChar16Value(wchar_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetChar16Value(wchar_t value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetByteValue(octet& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetByteValue(octet value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetBoolValue(bool& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetBoolValue(bool value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetStringValue(std::string& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetStringValue(const std::string& value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetWstringValue(std::wstring& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetWstringValue(const std::wstring& value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetEnumValue(std::string& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetEnumValue(const std::string& value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetEnumValue(uint32_t& value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetEnumValue(const uint32_t& value, MemberId id = MEMBER_ID_INVALID);
    RTPS_DllAPI ResponseCode GetBitmaskValue(const std::string& name, bool& value) const;
    RTPS_DllAPI ResponseCode SetBitmaskValue(bool value, const std::string& name);

    RTPS_DllAPI ResponseCode GetComplexValue(DynamicData** value, MemberId id) const;
    RTPS_DllAPI ResponseCode SetComplexValue(DynamicData* value, MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ResponseCode GetUnionLabel(uint64_t& value) const;

    // Basic types returns (copy)
    RTPS_DllAPI int32_t GetInt32Value(MemberId id) const
    {
        int32_t value;
        if (GetInt32Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint32_t GetUint32Value(MemberId id) const
    {
        uint32_t value;
        if (GetUint32Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI int16_t GetInt16Value(MemberId id) const
    {
        int16_t value;
        if (GetInt16Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint16_t GetUint16Value(MemberId id) const
    {
        uint16_t value;
        if (GetUint16Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI int64_t GetInt64Value(MemberId id) const
    {
        int64_t value;
        if (GetInt64Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint64_t GetUint64Value(MemberId id) const
    {
        uint64_t value;
        if (GetUint64Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI float GetFloat32Value(MemberId id) const
    {
        float value;
        if (GetFloat32Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI double GetFloat64Value(MemberId id) const
    {
        double value;
        if (GetFloat64Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI long double GetFloat128Value(MemberId id) const
    {
        long double value;
        if (GetFloat128Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI char GetChar8Value(MemberId id) const
    {
        char value;
        if (GetChar8Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI wchar_t GetChar16Value(MemberId id) const
    {
        wchar_t value;
        if (GetChar16Value(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI octet GetByteValue(MemberId id) const
    {
        octet value;
        if (GetByteValue(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI bool GetBoolValue(MemberId id) const
    {
        bool value;
        if (GetBoolValue(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::string GetStringValue(MemberId id) const
    {
        std::string value;
        if (GetStringValue(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::wstring GetWstringValue(MemberId id) const
    {
        std::wstring value;
        if (GetWstringValue(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::string GetEnumValue(MemberId id) const
    {
        std::string value;
        if (GetEnumValue(value, id) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI bool GetBitmaskValue(const std::string& name) const
    {
        bool value;
        if (GetBitmaskValue(name, value) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint64_t GetUnionLabel() const
    {
        uint64_t value;
        if (GetUnionLabel(value) != RETCODE_OK)
        {
            throw RETCODE_BAD_PARAMETER;
        }
        return value;
    }

protected:

    DynamicData();
    DynamicData(const DynamicData* pData);
    DynamicData(DynamicType_ptr pType);

    ~DynamicData();

    void AddValue(TypeKind kind, MemberId id);
    void CreateMembers(DynamicType_ptr pType);
    void CreateMembers(const DynamicData* pData);
    void Clean();
    void CleanMembers();
    void* CloneValue(MemberId id, TypeKind kind) const;
    bool CompareValues(TypeKind kind, void* left, void* right) const;
    ResponseCode InsertArrayData(MemberId indexId);
    void SerializeEmptyData(const DynamicType_ptr pType, eprosima::fastcdr::Cdr &cdr) const;
    void SetDefaultValue(MemberId id);
    void GetValue(std::string& sOutValue, MemberId id = MEMBER_ID_INVALID) const;
    void SetValue(const std::string& sValue, MemberId id = MEMBER_ID_INVALID);
    void SetTypeName(const std::string& name);
    ResponseCode SetUnionId(MemberId id);
    void UpdateUnionDiscriminator();
    void SortMemberIds(MemberId startId);
    void SetUnionDiscriminator(DynamicData* pData);

    // Serializes and deserializes the Dynamic Data.
    bool deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getCdrSerializedSize(const DynamicData* data, size_t current_alignment = 0);
    static size_t getEmptyCdrSerializedSize(const DynamicType* type, size_t current_alignment = 0);
    static size_t getKeyMaxCdrSerializedSize(const DynamicType_ptr type, size_t current_alignment = 0);
    static size_t getMaxCdrSerializedSize(const DynamicType_ptr type, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

    DynamicType_ptr mType;
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
    DynamicData* mDefaultArrayValue;
    uint64_t mUnionLabel;
    MemberId mUnionId;
    DynamicData* mUnionDiscriminator;

    friend class DynamicDataFactory;
    friend class DynamicPubSubType;
};


} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_DATA_H
