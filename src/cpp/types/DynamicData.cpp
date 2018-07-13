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
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/types/DynamicDataFactory.h>

namespace eprosima {
namespace fastrtps {
namespace types {

template <typename Map>
bool map_compare(Map const &left, Map const &right)
{
    auto pred = [](decltype(*left.begin()) a, decltype(a) b)
    {
        return a.first == b.first && a.second == b.second;
    };

    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), pred);
}

template <>
bool map_compare(const std::map<MemberId, DynamicData*>& left, const std::map<MemberId, DynamicData*>& right)
{
    auto pred = [](decltype(*left.begin()) a, decltype(a) b)
    {
        return a.first == b.first && a.second->Equals(b.second);
    };

    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), pred);
}

DynamicData::DynamicData()
    : mType(nullptr)
#ifdef DYNAMIC_TYPES_CHECKING
    , mInt32Value(0)
    , mUInt32Value(0)
    , mInt16Value(0)
    , mUInt16Value(0)
    , mInt64Value(0)
    , mUInt64Value(0)
    , mFloat32Value(0.0f)
    , mFloat64Value(0.0)
    , mFloat128Value(0.0)
    , mChar8Value(0)
    , mChar16Value(0)
    , mByteValue(0)
    , mBoolValue(false)
#endif
    , mIsKeyElement(false)
    , mDefaultArrayValue(nullptr)
    , mUnionLabel(UINT64_MAX)
    , mUnionId(MEMBER_ID_INVALID)
{
}

DynamicData::DynamicData(DynamicType* pType)
    : mType(DynamicTypeBuilderFactory::GetInstance()->BuildType(pType))
#ifdef DYNAMIC_TYPES_CHECKING
    , mInt32Value(0)
    , mUInt32Value(0)
    , mInt16Value(0)
    , mUInt16Value(0)
    , mInt64Value(0)
    , mUInt64Value(0)
    , mFloat32Value(0.0f)
    , mFloat64Value(0.0)
    , mFloat128Value(0.0)
    , mChar8Value(0)
    , mChar16Value(0)
    , mByteValue(0)
    , mBoolValue(false)
#endif
    , mIsKeyElement(false)
    , mDefaultArrayValue(nullptr)
    , mUnionLabel(UINT64_MAX)
    , mUnionId(MEMBER_ID_INVALID)
{
    CreateMembers(mType);
}

DynamicData::~DynamicData()
{
    Clean();
}

void DynamicData::CreateMembers(DynamicType* pType)
{
    std::map<MemberId, DynamicTypeMember*> members;
    if (pType->GetAllMembers(members) == ResponseCode::RETCODE_OK)
    {
        if (pType->IsComplexKind())
        {
            // Bitmasks and enums register their members but only manages one value.
            if (pType->GetKind() == TK_BITMASK || pType->GetKind() == TK_ENUM)
            {
                AddValue(pType->GetKind(), MEMBER_ID_INVALID);
            }

            for (auto it = members.begin(); it != members.end(); ++it)
            {
                MemberDescriptor* newDescriptor = new MemberDescriptor();
                if (it->second->GetDescriptor(newDescriptor) == ResponseCode::RETCODE_OK)
                {
                    mDescriptors.insert(std::make_pair(it->first, newDescriptor));
                    if (pType->GetKind() != TK_BITMASK && pType->GetKind() != TK_ENUM)
                    {
#ifdef DYNAMIC_TYPES_CHECKING
                        mComplexValues.insert(std::make_pair(it->first, DynamicDataFactory::GetInstance()->CreateData(newDescriptor->mType)));
#else
                        mValues.insert(std::make_pair(it->first, DynamicDataFactory::GetInstance()->CreateData(newDescriptor->mType)));
#endif
                    }
                }
                else
                {
                    delete newDescriptor;
                }
            }

            // Set the default value for unions.
            if (pType->GetKind() == TK_UNION)
            {
                for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
                {
                    if (it->second->IsDefaultUnionValue())
                    {
                        SetUnionId(it->first);
                        break;
                    }
                }
            }
        }
        else
        {
            AddValue(pType->GetKind(), MEMBER_ID_INVALID);
        }
    }
}

ResponseCode DynamicData::GetDescriptor(MemberDescriptor& value, MemberId id)
{
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        value.CopyFrom(it->second);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting MemberDescriptor. MemberId not found.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::SetDescriptor(MemberId id, const MemberDescriptor* value)
{
    if (mDescriptors.find(id) == mDescriptors.end())
    {
        mDescriptors.insert(std::make_pair(id, new MemberDescriptor(value)));
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error setting MemberDescriptor. MemberId found.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicData::Equals(const DynamicData* other) const
{
    if (other != nullptr)
    {
        if (other == this)
        {
            return true;
        }
        else if (GetItemCount() == other->GetItemCount() && mType->Equals(other->mType) &&
            mDescriptors.size() == other->mDescriptors.size())
        {
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                auto otherDescIt = other->mDescriptors.find(it->first);
                if (otherDescIt == other->mDescriptors.end() || !it->second->Equals(otherDescIt->second))
                {
                    return false;
                }
            }

            // Optimization for unions, only check the selected element.
            if (GetKind() == TK_UNION)
            {
                if (mUnionId != other->mUnionId)
                {
                    return false;
                }
                else if (mUnionId != MEMBER_ID_INVALID)
                {
#ifdef DYNAMIC_TYPES_CHECKING
                    auto it = mComplexValues.find(mUnionId);
                    auto otherIt = other->mComplexValues.find(mUnionId);
#else
                    auto it = mValues.find(mUnionId);
                    auto otherIt = other->mValues.find(mUnionId);
#endif
                    if (!((DynamicData*)it->second)->Equals((DynamicData*)otherIt->second))
                    {
                        return false;
                    }
                }
            }
            else
            {
#ifdef DYNAMIC_TYPES_CHECKING
                if (mInt32Value != other->mInt32Value || mUInt32Value != other->mUInt32Value ||
                    mInt16Value != other->mInt16Value || mUInt16Value != other->mUInt16Value ||
                    mInt64Value != other->mInt64Value || mUInt64Value != other->mUInt64Value ||
                    mFloat32Value != other->mFloat32Value || mFloat64Value != other->mFloat64Value ||
                    mFloat128Value != other->mFloat128Value || mChar8Value != other->mChar8Value ||
                    mChar16Value != other->mChar16Value || mByteValue != other->mByteValue ||
                    mBoolValue != other->mBoolValue || mStringValue != other->mStringValue ||
                    mWStringValue != other->mWStringValue || mStringValue != other->mStringValue ||
                    !map_compare(mComplexValues, other->mComplexValues))
                {
                    return false;
                }
#else
                if (GetKind() == TK_ENUM)
                {
                    if (!CompareValues(TK_UINT32, mValues.begin()->second, other->mValues.begin()->second))
                    {
                        return false;
                    }
                }
                else if (GetKind() == TK_BITMASK || GetKind() == TK_BITSET)
                {
                    if (!CompareValues(TK_UINT32, mValues.begin()->second, other->mValues.begin()->second))
                    {
                        return false;
                    }
                }
                else if (mType->IsComplexKind())
                {
                    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
                    {
                        auto currentIt = mValues.find(it->first);
                        auto otherIt = other->mValues.find(it->first);
                        if (!((DynamicData*)currentIt->second)->Equals(((DynamicData*)otherIt->second)))
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
                    {
                        auto currentIt = mValues.find(it->first);
                        auto otherIt = other->mValues.find(it->first);
                        if (!CompareValues(it->second->GetKind(), currentIt->second, otherIt->second))
                        {
                            return false;
                        }
                    }
                }
#endif
            }
            return true;
        }
    }
    return false;
}

MemberId DynamicData::GetMemberIdByName(const std::string& name) const
{
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        if (it->second->GetName() == name)
        {
            return it->first;
        }
    }
    return MEMBER_ID_INVALID;
}

MemberId DynamicData::GetMemberIdAtIndex(uint32_t index) const
{
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        if (it->second->GetIndex() == index)
        {
            return it->first;
        }
    }
    return MEMBER_ID_INVALID;
}

TypeKind DynamicData::GetKind() const
{
    return mType->GetKind();
}

uint32_t DynamicData::GetItemCount() const
{
    if (GetKind() == TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(mComplexValues.size() / 2);
#else
        return static_cast<uint32_t>(mValues.size() / 2);
#endif
    }
    else if (GetKind() == TK_ARRAY)
    {
        return mType->GetTotalBounds();
    }
    else
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(mComplexValues.size());
#else
        return static_cast<uint32_t>(mValues.size());
#endif
    }
}

std::string DynamicData::GetName()
{
    return mType->GetName();
}

void DynamicData::AddValue(TypeKind kind, MemberId id)
{
    switch (kind)
    {
    default:
        break;
    case TK_INT32:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new int32_t()));
#endif
    }
    break;
    case TK_UINT32:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new uint32_t()));
#endif
    }
    break;
    case TK_INT16:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new int16_t()));
#endif
    }
    break;
    case TK_UINT16:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new uint16_t()));
#endif
    }
    break;
    case TK_INT64:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new int64_t()));
#endif
    }
    break;
    case TK_UINT64:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new uint64_t()));
#endif
    }
    break;
    case TK_FLOAT32:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new float()));
#endif
    }
    break;
    case TK_FLOAT64:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new double()));
#endif
    }
    break;
    case TK_FLOAT128:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new long double()));
#endif
    }
    break;
    case TK_CHAR8:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new char()));
#endif
    }
    break;
    case TK_CHAR16:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new wchar_t()));
#endif
    }
    break;
    case TK_BOOLEAN:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new bool()));
#endif
    }
    break;
    case TK_BYTE:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new octet()));
#endif
    }
    break;
    case TK_STRING8:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new std::string()));
#endif
    }
    break;
    case TK_STRING16:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new std::wstring()));
#endif
    }
    break;
    case TK_ENUM:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new uint32_t()));
#endif
    }
    break;
    case TK_BITSET:
    case TK_BITMASK:
    {
#ifndef DYNAMIC_TYPES_CHECKING
        mValues.insert(std::make_pair(id, new uint64_t()));
#endif
    }
    }
    SetDefaultValue(id);
}

void DynamicData::Clean()
{
    if (mDefaultArrayValue != nullptr)
    {
        DynamicDataFactory::GetInstance()->DeleteData(mDefaultArrayValue);
        mDefaultArrayValue = nullptr;
    }
#ifdef DYNAMIC_TYPES_CHECKING
    for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
    {
        DynamicDataFactory::GetInstance()->DeleteData(it->second);
    }
    mComplexValues.clear();
#else
    if (mType->HasChildren())
    {
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
        }
    }
    mValues.clear();
#endif

    if (mType != nullptr)
    {
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(mType);
        mType = nullptr;
    }

    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        delete it->second;
    }
    mDescriptors.clear();
}

ResponseCode DynamicData::ClearAllValues()
{
    if (mType->IsComplexKind())
    {
        if (GetKind() == TK_SEQUENCE || GetKind() == TK_MAP || GetKind() == TK_ARRAY)
        {
            return ClearData();
        }
        else
        {
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto itValue = mComplexValues.find(it->first);
                if (itValue != mComplexValues.end())
                {
                    itValue->second->ClearAllValues();
                }
#else
                auto itValue = mValues.find(it->first);
                if (itValue != mValues.end())
                {
                    ((DynamicData*)itValue->second)->ClearAllValues();
                }
#endif
            }
        }
    }
    else
    {
        SetDefaultValue(MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::ClearNonkeyValues()
{
    if (mType->IsComplexKind())
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = mComplexValues.find(it->first);
            if (itValue != mComplexValues.end())
            {
                itValue->second->ClearNonkeyValues();
            }
#else
            auto itValue = mValues.find(it->first);
            if (itValue != mValues.end())
            {
                ((DynamicData*)itValue->second)->ClearNonkeyValues();
            }
#endif
        }
    }
    else
    {
        if (!mIsKeyElement)
        {
            SetDefaultValue(MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::ClearValue(MemberId id)
{
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        if (mType->IsComplexKind())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = mComplexValues.find(it->first);
            if (itValue != mComplexValues.end())
            {
                itValue->second->ClearAllValues();
            }
#else
            auto itValue = mValues.find(it->first);
            if (itValue != mValues.end())
            {
                ((DynamicData*)itValue->second)->ClearAllValues();
            }
#endif
        }
        else
        {
            SetDefaultValue(id);
        }
    }
    else
    {
        SetDefaultValue(id);
    }
    return ResponseCode::RETCODE_OK;
}

void* DynamicData::CloneValue(MemberId id, TypeKind kind) const
{
    switch (kind)
    {
    default:
        break;
    case TK_INT32:
    {
        int32_t* newInt32 = new int32_t();
        GetInt32Value(*newInt32, id);
        return newInt32;
    }
    break;
    case TK_UINT32:
    {
        uint32_t* newUInt32 = new uint32_t();
        GetUint32Value(*newUInt32, id);
        return newUInt32;
    }
    break;
    case TK_INT16:
    {
        int16_t* newInt16 = new int16_t();
        GetInt16Value(*newInt16, id);
        return newInt16;
    }
    break;
    case TK_UINT16:
    {
        uint16_t* newUInt16 = new uint16_t();
        GetUint16Value(*newUInt16, id);
        return newUInt16;
    }
    break;
    case TK_INT64:
    {
        int64_t* newInt64 = new int64_t();
        GetInt64Value(*newInt64, id);
        return newInt64;
    }
    break;
    case TK_UINT64:
    {
        uint64_t* newUInt64 = new uint64_t();
        GetUint64Value(*newUInt64, id);
        return newUInt64;
    }
    break;
    case TK_FLOAT32:
    {
        float* newFloat32 = new float();
        GetFloat32Value(*newFloat32, id);
        return newFloat32;
    }
    break;
    case TK_FLOAT64:
    {
        double* newFloat64 = new double();
        GetFloat64Value(*newFloat64, id);
        return newFloat64;
    }
    break;
    case TK_FLOAT128:
    {
        long double* newFloat128 = new long double();
        GetFloat128Value(*newFloat128, id);
        return newFloat128;
    }
    break;
    case TK_CHAR8:
    {
        char* newChar8 = new char();
        GetChar8Value(*newChar8, id);
        return newChar8;
    }
    break;
    case TK_CHAR16:
    {
        wchar_t* newChar16 = new wchar_t();
        GetChar16Value(*newChar16, id);
        return newChar16;
    }
    break;
    case TK_BOOLEAN:
    {
        bool* newBool = new bool();
        GetBoolValue(*newBool, id);
        return newBool;
    }
    break;
    case TK_BYTE:
    {
        octet* newByte = new octet();
        GetByteValue(*newByte, id);
        return newByte;
    }
    break;
    case TK_STRING8:
    {
        std::string* newString = new std::string();
        GetStringValue(*newString, id);
        return newString;
    }
    break;
    case TK_STRING16:
    {
        std::wstring* newString = new std::wstring();
        GetWstringValue(*newString, id);
        return newString;
    }
    break;
    case TK_ENUM:
    {
        uint32_t* newUInt32 = new uint32_t();
        GetUint32Value(*newUInt32, id);
        return newUInt32;
    }
    break;
    case TK_BITSET:
    case TK_BITMASK:
    {
        uint64_t* newBitset = new uint64_t();
        GetUint64Value(*newBitset, id);
        return newBitset;
    }
    }
    return nullptr;
}

bool DynamicData::CompareValues(TypeKind kind, void* left, void* right) const
{
    switch (kind)
    {
    default:
        break;
    case TK_INT32:      {   return *((int32_t*)left) == *((int32_t*)right);    }
    case TK_UINT32:     {   return *((uint32_t*)left) == *((uint32_t*)right);    }
    case TK_INT16:      {   return *((int16_t*)left) == *((int16_t*)right);    }
    case TK_UINT16:     {   return *((uint16_t*)left) == *((uint16_t*)right);    }
    case TK_INT64:      {   return *((int64_t*)left) == *((int64_t*)right);    }
    case TK_UINT64:     {   return *((uint64_t*)left) == *((uint64_t*)right);    }
    case TK_FLOAT32:    {   return *((float*)left) == *((float*)right);    }
    case TK_FLOAT64:    {   return *((double*)left) == *((double*)right);    }
    case TK_FLOAT128:   {   return *((long double*)left) == *((long double*)right);    }
    case TK_CHAR8:      {   return *((char*)left) == *((char*)right);    }
    case TK_CHAR16:     {   return *((wchar_t*)left) == *((wchar_t*)right);    }
    case TK_BOOLEAN:    {   return *((bool*)left) == *((bool*)right);    }
    case TK_BYTE:       {   return *((octet*)left) == *((octet*)right);    }
    case TK_STRING8:    {   return *((std::string*)left) == *((std::string*)right);    }
    case TK_STRING16:   {   return *((std::wstring*)left) == *((std::wstring*)right);    }
    case TK_ENUM:       {   return *((uint32_t*)left) == *((uint32_t*)right);    }
    case TK_BITSET:     {   return *((uint64_t*)left) == *((uint64_t*)right);    }
    case TK_BITMASK:    {   return *((uint64_t*)left) == *((uint64_t*)right);    }
    }
    return false;
}

void DynamicData::SetDefaultValue(MemberId id)
{
    std::string defaultValue = "";
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        defaultValue = it->second->mDefaultValue;
    }

    switch (mType->mKind)
    {
    default:
        break;
    case TK_INT32:
    {
        int32_t value(0);
        try
        {
            value = stoi(defaultValue);
        }
        catch (...) {}
        SetInt32Value(value, id);
    }
    break;
    case TK_UINT32:
    {
        uint32_t value(0);
        try
        {
            value = stoul(defaultValue);
        }
        catch (...) {}
        SetUint32Value(value, id);
    }
    break;
    case TK_INT16:
    {
        int16_t value(0);
        try
        {
            value = static_cast<int16_t>(stoi(defaultValue));
        }
        catch (...) {}
        SetInt16Value(value, id);
    }
    break;
    case TK_UINT16:
    {
        uint16_t value(0);
        try
        {
            value = static_cast<uint16_t>(stoul(defaultValue));
        }
        catch (...) {}
        SetUint16Value(value, id);
    }
    break;
    case TK_INT64:
    {
        int64_t value(0);
        try
        {
            value = stoll(defaultValue);
        }
        catch (...) {}
        SetInt64Value(value, id);
    }
    break;
    case TK_UINT64:
    {
        uint64_t value(0);
        try
        {
            value = stoul(defaultValue);
        }
        catch (...) {}
        SetUint64Value(value, id);
    }
    break;
    case TK_FLOAT32:
    {
        float value(0.0f);
        try
        {
            value = stof(defaultValue);
        }
        catch (...) {}
        SetFloat32Value(value, id);
    }
    break;
    case TK_FLOAT64:
    {
        double value(0.0f);
        try
        {
            value = stod(defaultValue);
        }
        catch (...) {}
        SetFloat64Value(value, id);
    }
    break;
    case TK_FLOAT128:
    {
        long double value(0.0f);
        try
        {
            value = stold(defaultValue);
        }
        catch (...) {}
        SetFloat128Value(value, id);
    }
    break;
    case TK_CHAR8:
    {
        if (defaultValue.length() >= 1)
        {
            SetChar8Value(defaultValue[0], id);
        }
    }
    break;
    case TK_CHAR16:
    {
        wchar_t value(0);
        try
        {
            std::wstring temp = std::wstring(defaultValue.begin(), defaultValue.end());
            value = temp[0];
        }
        catch (...) {}

        SetChar16Value(value, id);
    }
    break;
    case TK_BOOLEAN:
    {
        int value(0);
        try
        {
            value = stoi(defaultValue);
        }
        catch (...) {}
        SetBoolValue(value == 1 ? true : false, id);
    }
    break;
    case TK_BYTE:
    {
        if (defaultValue.length() >= 1)
        {
            SetByteValue(defaultValue[0], id);
        }
    }
    break;
    case TK_STRING8:
    {
        SetStringValue(defaultValue, id);
    }
    break;
    case TK_STRING16:
    {
        SetWstringValue(std::wstring(defaultValue.begin(), defaultValue.end()), id);
    }
    break;
    case TK_ENUM:
    {
        uint32_t value(0);
        try
        {
            value = stoul(defaultValue);
        }
        catch (...) {}
        SetUint32Value(value, id);
    }
    break;
    case TK_BITSET:
    case TK_BITMASK:
    {
        int value(0);
        try
        {
            value = stoi(defaultValue);
        }
        catch (...) {}
        SetBoolValue(value == 1 ? true : false, id);
    }
    break;
    case TK_ARRAY:
    case TK_SEQUENCE:
    case TK_MAP:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto itValue = mComplexValues.find(id);
        if (itValue != mComplexValues.end())
        {
            if (!itValue->second->mIsKeyElement)
            {
                itValue->second->SetDefaultValue(MEMBER_ID_INVALID);
            }
        }
#else
        auto itValue = mValues.find(id);
        if (itValue != mValues.end())
        {
            if (!((DynamicData*)itValue->second)->mIsKeyElement)
            {
                ((DynamicData*)itValue->second)->SetDefaultValue(MEMBER_ID_INVALID);
            }
        }
#endif
    }
    break;
    }
}

DynamicData* DynamicData::LoanValue(MemberId id)
{
    if (id != MEMBER_ID_INVALID)
    {
        if (std::find(mLoanedValues.begin(), mLoanedValues.end(), id) == mLoanedValues.end())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = mComplexValues.find(id);
            if (it != mComplexValues.end())
            {
                if (GetKind() == TK_MAP && it->second->mIsKeyElement)
                {
                    logError(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    if (GetKind() == TK_UNION && mUnionId != id)
                    {
                        SetUnionId(id);
                    }
                    mLoanedValues.push_back(id);
                    return it->second;
                }
            }
            else if (GetKind() == TK_ARRAY)
            {
                if (InsertArrayData(id) == ResponseCode::RETCODE_OK)
                {
                    mLoanedValues.push_back(id);
                    return mComplexValues.at(id);
                }
            }
#else
            auto it = mValues.find(id);
            if (it != mValues.end())
            {
                if (GetKind() == TK_MAP && ((DynamicData*)it->second)->mIsKeyElement)
                {
                    logError(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    if (GetKind() == TK_UNION && mUnionId != id)
                    {
                        SetUnionId(id);
                    }
                    mLoanedValues.push_back(id);
                    return (DynamicData*)it->second;
                }
            }
            else if (GetKind() == TK_ARRAY)
            {
                if (InsertArrayData(id) == ResponseCode::RETCODE_OK)
                {
                    mLoanedValues.push_back(id);
                    return (DynamicData*)mValues.at(id);
                }
            }

#endif
            else
            {
                logError(DYN_TYPES, "Error loaning Value. MemberId not found.");
            }
        }
        else
        {
            logError(DYN_TYPES, "Error loaning Value. The value has been loaned previously.");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error loaning Value. Invalid MemberId.");
    }
    return nullptr;
}

ResponseCode DynamicData::ReturnLoanedValue(const DynamicData* value)
{
    for (auto loanIt = mLoanedValues.begin(); loanIt != mLoanedValues.end(); ++loanIt)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = mComplexValues.find(*loanIt);
        if (it != mComplexValues.end() && it->second == value)
        {
            mLoanedValues.erase(loanIt);
            return ResponseCode::RETCODE_OK;
        }
#else
        auto it = mValues.find(*loanIt);
        if (it != mValues.end() && it->second == value)
        {
            mLoanedValues.erase(loanIt);
            return ResponseCode::RETCODE_OK;
        }
#endif
    }

    logError(DYN_TYPES, "Error returning loaned Value. The value hasn't been loaned.");
    return ResponseCode::RETCODE_PRECONDITION_NOT_MET;
}

DynamicData* DynamicData::Clone() const
{
    DynamicData* newData = DynamicDataFactory::GetInstance()->CreateData(mType);

#ifdef DYNAMIC_TYPES_CHECKING
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        newData->mDescriptors.insert(std::make_pair(it->first, new MemberDescriptor(it->second)));
    }

    newData->mInt32Value = mInt32Value;
    newData->mUInt32Value = mUInt32Value;
    newData->mInt16Value = mInt16Value;
    newData->mUInt16Value = mUInt16Value;
    newData->mInt64Value = mInt64Value;
    newData->mUInt64Value = mUInt64Value;
    newData->mFloat32Value = mFloat32Value;
    newData->mFloat64Value = mFloat64Value;
    newData->mFloat128Value = mFloat128Value;
    newData->mChar8Value = mChar8Value;
    newData->mChar16Value = mChar16Value;
    newData->mByteValue = mByteValue;
    newData->mBoolValue = mBoolValue;
    newData->mStringValue = mStringValue;
    newData->mWStringValue = mWStringValue;

    for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
    {
        newData->mComplexValues.insert(std::make_pair(it->first, it->second->Clone()));
    }

#else
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        newData->mDescriptors.insert(std::make_pair(it->first, new MemberDescriptor(it->second)));
    }

    if (mType->IsComplexKind())
    {
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            newData->mValues.insert(std::make_pair(it->first, ((DynamicData*)it->second)->Clone()));
        }
    }
    else
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            newData->mValues.insert(std::make_pair(it->first, CloneValue(it->first, it->second->GetKind())));
        }
    }
#endif

    return newData;
}

ResponseCode DynamicData::GetInt32Value(int32_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        value = mInt32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetInt32Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetInt32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            value = *((int32_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetInt32Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetInt32Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt32Value(int32_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        mInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetInt32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt32Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            *((int32_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetInt32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt32Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint32Value(uint32_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        value = mUInt32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetUint32Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetUint32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            value = *((uint32_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetUint32Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetUint32Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint32Value(uint32_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        mUInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetUint32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint32Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            *((uint32_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetUint32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint32Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetInt16Value(int16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        value = mInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetInt16Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetInt16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            value = *((int16_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetInt16Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetInt16Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt16Value(int16_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        mInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetInt16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt16Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            *((int16_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetInt16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt16Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint16Value(uint16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        value = mUInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetUint16Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetUint16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            value = *((uint16_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetUint16Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetUint16Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint16Value(uint16_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        mUInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetUint16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint16Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            *((uint16_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetUint16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint16Value(value, id);
        }
        return insertResult;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetInt64Value(int64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        value = mInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetInt64Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetInt64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            value = *((int64_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetInt64Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetInt64Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt64Value(int64_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        mInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetInt64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt64Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            *((int64_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetInt64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt64Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint64Value(uint64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        value = mUInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetUint64Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetUint64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            value = *((uint64_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetUint64Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetUint64Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint64Value(uint64_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        mUInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetUint64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint64Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            *((uint64_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetUint64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint64Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat32Value(float& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        value = mFloat32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetFloat32Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetFloat32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            value = *((float*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetFloat32Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetFloat32Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat32Value(float value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        mFloat32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetFloat32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat32Value(value, id);
            }
            return insertResult;
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            *((float*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetFloat32Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat32Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat64Value(double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        value = mFloat64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetFloat64Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetFloat64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            value = *((double*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetFloat64Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetFloat64Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat64Value(double value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        mFloat64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetFloat64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat64Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            *((double*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetFloat64Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat64Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat128Value(long double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        value = mFloat128Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetFloat128Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetFloat128Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            value = *((long double*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetFloat128Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetFloat128Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat128Value(long double value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        mFloat128Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetFloat128Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat128Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            *((long double*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetFloat128Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat128Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetChar8Value(char& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        value = mChar8Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetChar8Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetChar8Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            value = *((char*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetChar8Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetChar8Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetChar8Value(char value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        mChar8Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetChar8Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetChar8Value(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            *((char*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetChar8Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetChar8Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetChar16Value(wchar_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        value = mChar16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetChar16Value(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetChar16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            value = *((wchar_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetChar16Value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetChar16Value(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetChar16Value(wchar_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        mChar16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetChar16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetChar16Value(value, id);
            }
            return insertResult;
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            *((wchar_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetChar16Value(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetChar16Value(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetByteValue(octet& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        value = mByteValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetByteValue(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetByteValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            value = *((octet*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetByteValue(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetByteValue(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetByteValue(octet value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        mByteValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetByteValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetByteValue(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            *((octet*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetByteValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetByteValue(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetBoolValue(bool& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        value = mBoolValue;
        return ResponseCode::RETCODE_OK;
    }
    else if ((GetKind() == TK_BITSET || GetKind() == TK_BITMASK) && id < mType->GetBounds())
    {
        value = (mUInt64Value & ((uint64_t)1 << id)) != 0;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetBoolValue(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetBoolValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.end();
    if (GetKind() == TK_BITSET || GetKind() == TK_BITMASK)
    {
        it = mValues.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = mValues.find(id);
    }
    if (it != mValues.end())
    {
        if (GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            value = *((bool*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if ((GetKind() == TK_BITSET || GetKind() == TK_BITMASK) && id < mType->GetBounds())
        {
            value = (*((uint64_t*)it->second) & ((uint64_t)1 << id)) != 0;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetBoolValue(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetBoolValue(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetBoolValue(bool value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        mBoolValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (GetKind() == TK_BITSET || GetKind() == TK_BITMASK)
    {
        if (id == MEMBER_ID_INVALID)
        {
            if (value)
            {
                mUInt64Value = ~((uint64_t)0);
            }
            else
            {
                mUInt64Value = 0;
            }
        }
        else if (mType->GetBounds() == LENGTH_UNLIMITED || id < mType->GetBounds())
        {
            if (value)
            {
                mUInt64Value |= ((uint64_t)1 << id);
            }
            else
            {
                mUInt64Value &= ~((uint64_t)1 << id);
            }
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error setting bool value. The given index is greather than the limit.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetBoolValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetBoolValue(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.end();
    if (GetKind() == TK_BITSET || GetKind() == TK_BITMASK)
    {
        it = mValues.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = mValues.find(id);
    }

    if (it != mValues.end())
    {
        if (GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            *((bool*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (GetKind() == TK_BITSET || GetKind() == TK_BITMASK)
        {
            if (id == MEMBER_ID_INVALID)
            {
                if (value)
                {
                    *((uint64_t*)it->second) = ~((uint64_t)0);
                }
                else
                {
                    *((uint64_t*)it->second) = 0;
                }
                return ResponseCode::RETCODE_OK;
            }
            else if (mType->GetBounds() == LENGTH_UNLIMITED || id < mType->GetBounds())
            {
                if (value)
                {
                    *((uint64_t*)it->second) |= ((uint64_t)1 << id);
                }
                else
                {
                    *((uint64_t*)it->second) &= ~((uint64_t)1 << id);
                }
                return ResponseCode::RETCODE_OK;
            }
            else
            {
                logError(DYN_TYPES, "Error setting bool value. The given index is greather than the limit.");
                return ResponseCode::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetBoolValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetBoolValue(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetStringValue(std::string& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        value = mStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetStringValue(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetStringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            value = *((std::string*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetStringValue(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetStringValue(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetStringValue(const std::string& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_STRING8 &&id == MEMBER_ID_INVALID)
    {
        if (value.length() <= mType->GetBounds())
        {
            mStringValue = value;
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error setting string value. The given string is greather than the length limit.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetStringValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetStringValue(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= mType->GetBounds())
            {
                *((std::string*)it->second) = value;
                return ResponseCode::RETCODE_OK;
            }
            else
            {
                logError(DYN_TYPES, "Error setting string value. The given string is greather than the length limit.");
                return ResponseCode::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetStringValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetStringValue(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

void DynamicData::SetTypeName(const std::string& name)
{
    if (mType != nullptr)
    {
        mType->SetName(name);
    }
}

ResponseCode DynamicData::SetUnionId(MemberId id)
{
    if (GetKind() == TK_UNION)
    {
        auto it = mDescriptors.find(id);
        if (id == MEMBER_ID_INVALID || it != mDescriptors.end())
        {
            mUnionId = id;
            if (it != mDescriptors.end())
            {
                std::vector<uint64_t> unionLabels = it->second->GetUnionLabels();
                if (unionLabels.size() > 0)
                {
                    mUnionLabel = unionLabels[0];
                }
            }
            return ResponseCode::RETCODE_OK;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error setting union id. The kind: " << GetKind() << " doesn't support it.");
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::GetWstringValue(std::wstring& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        value = mWStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetWstringValue(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetWstringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            value = *((std::wstring*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)it->second)->GetWstringValue(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetWstringValue(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}


ResponseCode DynamicData::SetWstringValue(const std::wstring& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= mType->GetBounds())
        {
            mWStringValue = value;
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error setting wstring value. The given string is greather than the length limit.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetWstringValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetWstringValue(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= mType->GetBounds())
            {
                *((std::wstring*)it->second) = value;
                return ResponseCode::RETCODE_OK;
            }
            else
            {
                logError(DYN_TYPES, "Error setting wstring value. The given string is greather than the length limit.");
                return ResponseCode::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)it->second)->SetWstringValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetWstringValue(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetEnumValue(std::string& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        auto it = mDescriptors.find(mUInt32Value);
        if (it != mDescriptors.end())
        {
            value = it->second->GetName();
            return ResponseCode::RETCODE_OK;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return it->second->GetEnumValue(value, MEMBER_ID_INVALID);
            }
        }
        else if (GetKind() == TK_ARRAY)
        {
            return mDefaultArrayValue->GetEnumValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto itValue = mValues.find(id);
    if (itValue != mValues.end())
    {
        if (GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            auto it = mDescriptors.find(*((uint32_t*)itValue->second));
            if (it != mDescriptors.end())
            {
                value = it->second->GetName();
                return ResponseCode::RETCODE_OK;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (GetKind() != TK_UNION || id == mUnionId)
            {
                return ((DynamicData*)itValue->second)->GetEnumValue(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return mDefaultArrayValue->GetEnumValue(value, MEMBER_ID_INVALID);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetEnumValue(const std::string& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            if (it->second->GetName() == value)
            {
                mUInt32Value = it->first;
                return ResponseCode::RETCODE_OK;
            }
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            ResponseCode result = it->second->SetEnumValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
        else if (GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetEnumValue(value, id);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto itValue = mValues.find(id);
    if (itValue != mValues.end())
    {
        if (GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                if (it->second->GetName() == value)
                {
                    *((uint32_t*)itValue->second) = it->first;
                    return ResponseCode::RETCODE_OK;
                }
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ResponseCode result = ((DynamicData*)itValue->second)->SetEnumValue(value, MEMBER_ID_INVALID);
            if (result == ResponseCode::RETCODE_OK && GetKind() == TK_UNION)
            {
                SetUnionId(id);
            }
            return result;
        }
    }
    else if (GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetEnumValue(value, id);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetBitmaskValue(bool value, const std::string& name)
{
    MemberId id = GetMemberIdByName(name);
    if (id != MEMBER_ID_INVALID)
    {
        return SetBoolValue(value, id);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::GetBitmaskValue(const std::string& name, bool& value) const
{
    MemberId id = GetMemberIdByName(name);
    if (id != MEMBER_ID_INVALID)
    {
        return GetBoolValue(value, id);
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

void DynamicData::SortMemberIds(MemberId startId)
{
    MemberId index = startId;
    MemberId curID = startId + 1;
    uint32_t distance = 1;
#ifdef DYNAMIC_TYPES_CHECKING
    while (index <= mComplexValues.size())
    {
        auto it = mComplexValues.find(curID);
        if (it != mComplexValues.end())
        {
            mComplexValues[curID - distance] = it->second;
            mComplexValues.erase(it);
        }
        else
        {
            ++distance;
        }
        ++index;
        ++curID;
    }
#else
    while (curID <= mValues.size())
    {
        auto it = mValues.find(curID);
        if (it != mValues.end())
        {
            mValues[curID - distance] = it->second;
            mValues.erase(it);
        }
        else
        {
            ++distance;
        }
        ++index;
        ++curID;
    }
#endif
}

MemberId DynamicData::GetArrayIndex(const std::vector<uint32_t>& position)
{
    if (GetKind() == TK_ARRAY)
    {
        MemberId outPosition(0);
        uint32_t offset(1);
        uint32_t boundsSize = mType->GetBoundsSize();
        if (position.size() == boundsSize)
        {
            for (uint32_t i = 0; i < position.size(); ++i)
            {
                outPosition += position[i] * offset;
                offset *= mType->GetBounds(i);
            }
            return outPosition;
        }
        else
        {
            logError(DYN_TYPES, "Error getting array index. Invalid dimension count.");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error getting array index. The kind " << GetKind() << "doesn't support it.");
    }
    return MEMBER_ID_INVALID;
}

ResponseCode DynamicData::InsertArrayData(MemberId indexId)
{
    if (GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < mType->GetTotalBounds())
        {
            auto it = mComplexValues.find(indexId);
            if (it != mComplexValues.end())
            {
                DynamicDataFactory::GetInstance()->DeleteData(it->second);
                mComplexValues.erase(it);
            }
            DynamicData* value = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            mComplexValues.insert(std::make_pair(indexId, value));
            return ResponseCode::RETCODE_OK;
        }
#else
        if (indexId < mType->GetTotalBounds())
        {
            auto it = mValues.find(indexId);
            if (it != mValues.end())
            {
                DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
                mValues.erase(it);
            }
            DynamicData* value = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            mValues.insert(std::make_pair(indexId, value));
            return ResponseCode::RETCODE_OK;
        }
#endif
        else
        {
            logError(DYN_TYPES, "Error inserting data. Index out of bounds");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error inserting data. The kind " << GetKind() << " doesn't support this method");
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::ClearArrayData(MemberId indexId)
{
    if (GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < mType->GetTotalBounds())
        {
            auto it = mComplexValues.find(indexId);
            if (it != mComplexValues.end())
            {
                DynamicDataFactory::GetInstance()->DeleteData(it->second);
                mComplexValues.erase(it);
            }
            return ResponseCode::RETCODE_OK;
        }
#else
        if (indexId < mType->GetTotalBounds())
        {
            auto it = mValues.find(indexId);
            if (it != mValues.end())
            {
                DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
                mValues.erase(it);
            }
            return ResponseCode::RETCODE_OK;
        }
#endif
        else
        {
            logError(DYN_TYPES, "Error removing data. Index out of bounds");
        }
    }
    else
    {
        logError(DYN_TYPES, "Error removing data. The kind " << GetKind() << " doesn't support this method");
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::InsertSequenceData(MemberId& outId)
{
    outId = MEMBER_ID_INVALID;
    if (GetKind() == TK_SEQUENCE)
    {
        if (mType->GetBounds() == LENGTH_UNLIMITED || GetItemCount() < mType->GetBounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            DynamicData* new_element = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            outId = static_cast<MemberId>(mComplexValues.size());
            mComplexValues.insert(std::make_pair(outId, new_element));
            return ResponseCode::RETCODE_OK;
#else
            DynamicData* new_element = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            outId = static_cast<MemberId>(mValues.size());
            mValues.insert(std::make_pair(outId, new_element));
            return ResponseCode::RETCODE_OK;
#endif
        }
        else
        {
            logError(DYN_TYPES, "Error inserting data. The container is full.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error inserting data. The kind " << GetKind() << " doesn't support this method");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::RemoveSequenceData(MemberId id)
{
    if (GetKind() == TK_SEQUENCE || GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            DynamicDataFactory::GetInstance()->DeleteData(it->second);
            mComplexValues.erase(it);
            SortMemberIds(id);
            return ResponseCode::RETCODE_OK;
        }
#else
        auto it = mValues.find(id);
        if (it != mValues.end())
        {
            DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
            mValues.erase(it);
            SortMemberIds(id);
            return ResponseCode::RETCODE_OK;
        }
#endif
        logError(DYN_TYPES, "Error removing data. Member not found");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }

    logError(DYN_TYPES, "Error removing data. The current Kind " << GetKind()
        << " doesn't support this method");

    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::InsertMapData(DynamicData* key, MemberId& outKeyId, MemberId& outValueId)
{
    if (GetKind() == TK_MAP)
    {
        if (mType->GetBounds() == LENGTH_UNLIMITED || GetItemCount() < mType->GetBounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
            {
                if (it->second->Equals(key))
                {
                    logError(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ResponseCode::RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = static_cast<MemberId>(mComplexValues.size());
            key->mIsKeyElement = true;
            mComplexValues.insert(std::make_pair(outKeyId, key));

            DynamicData* new_element = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            outValueId = static_cast<MemberId>(mComplexValues.size());
            mComplexValues.insert(std::make_pair(outValueId, new_element));
            return ResponseCode::RETCODE_OK;
#else
            for (auto it = mValues.begin(); it != mValues.end(); ++it)
            {
                if (it->second == key)
                {
                    logError(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ResponseCode::RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = static_cast<MemberId>(mValues.size());
            key->mIsKeyElement = true;
            mValues.insert(std::make_pair(outKeyId, key));

            DynamicData* new_element = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
            outValueId = static_cast<MemberId>(mValues.size());
            mValues.insert(std::make_pair(outValueId, new_element));
            return ResponseCode::RETCODE_OK;
#endif
        }
        else
        {
            logError(DYN_TYPES, "Error inserting to map. The map is full");
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error inserting to map. The current Kind " << GetKind()
            << " doesn't support this method");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::RemoveMapData(MemberId keyId)
{
    if (GetKind() == TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto itKey = mComplexValues.find(keyId);
        auto itValue = mComplexValues.find(keyId + 1);
        if (itKey != mComplexValues.end() && itValue != mComplexValues.end() && itKey->second->mIsKeyElement)
        {
            DynamicDataFactory::GetInstance()->DeleteData(itKey->second);
            DynamicDataFactory::GetInstance()->DeleteData(itValue->second);
            mComplexValues.erase(itKey);
            mComplexValues.erase(itValue);
            SortMemberIds(keyId);
            return ResponseCode::RETCODE_OK;
        }
#else
        auto itKey = mValues.find(keyId);
        auto itValue = mValues.find(keyId + 1);
        if (itKey != mValues.end() && itValue != mValues.end() && ((DynamicData*)itKey->second)->mIsKeyElement)
        {
            DynamicDataFactory::GetInstance()->DeleteData(((DynamicData*)itKey->second));
            DynamicDataFactory::GetInstance()->DeleteData(((DynamicData*)itValue->second));
            mValues.erase(itKey);
            mValues.erase(itValue);
            SortMemberIds(keyId);
            return ResponseCode::RETCODE_OK;
        }
#endif
        else
        {
            logError(DYN_TYPES, "Error removing from map. Invalid input KeyId");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error removing from map. The current Kind " << GetKind()
            << " doesn't support this method");
        return ResponseCode::RETCODE_ERROR;
    }
}

ResponseCode DynamicData::ClearData()
{
    if (GetKind() == TK_SEQUENCE || GetKind() == TK_MAP || GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
        {
            DynamicDataFactory::GetInstance()->DeleteData(it->second);
        }
        mComplexValues.clear();
#else
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
        }
        mValues.clear();
#endif
        return ResponseCode::RETCODE_OK;
    }

    logError(DYN_TYPES, "Error clearing data. The current Kind " << GetKind()
        << " doesn't support this method");

    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::GetComplexValue(DynamicData** value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(id);
    if (it != mComplexValues.end())
    {
        *value = it->second->Clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *value = ((DynamicData*)it->second)->Clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetComplexValue(DynamicData* value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(id);
    if (it != mComplexValues.end())
    {
        if (it->second != nullptr)
        {
            DynamicDataFactory::GetInstance()->DeleteData(it->second);
        }
        mComplexValues.erase(it);
    }
    mComplexValues.insert(std::make_pair(id, value));
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (it->second != nullptr)
        {
            DynamicDataFactory::GetInstance()->DeleteData((DynamicData*)it->second);
        }
        mValues.erase(it);
    }
    mValues.insert(std::make_pair(id, value));
#endif

    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::GetUnionLabel(uint64_t& value) const
{
    if (GetKind() == TK_UNION)
    {
        if (mUnionId != MEMBER_ID_INVALID)
        {
            value = mUnionLabel;
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error getting union label. There isn't any label selected");
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error getting union label. The kind " << GetKind() << "doesn't support it");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}


bool DynamicData::deserialize(eprosima::fastcdr::Cdr &cdr)
{
    switch (GetKind())
    {
    default:
        break;
    case TK_INT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mInt32Value;
#else
        auto it = mValues.begin();
        cdr >> *((int32_t*)it->second);
#endif
        break;
    }
    case TK_UINT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mUInt32Value;
#else
        auto it = mValues.begin();
        cdr >> *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_INT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mInt16Value;
#else
        auto it = mValues.begin();
        cdr >> *((int16_t*)it->second);
#endif
        break;
    }
    case TK_UINT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mUInt16Value;
#else
        auto it = mValues.begin();
        cdr >> *((uint16_t*)it->second);
#endif
        break;
    }
    case TK_INT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mInt64Value;
#else
        auto it = mValues.begin();
        cdr >> *((int64_t*)it->second);
#endif
        break;
    }
    case TK_UINT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mUInt64Value;
#else
        auto it = mValues.begin();
        cdr >> *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_FLOAT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mFloat32Value;
#else
        auto it = mValues.begin();
        cdr >> *((float*)it->second);
#endif
        break;
    }
    case TK_FLOAT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mFloat64Value;
#else
        auto it = mValues.begin();
        cdr >> *((double*)it->second);
#endif
        break;
    }
    case TK_FLOAT128:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mFloat128Value;
#else
        auto it = mValues.begin();
        cdr >> *((long double*)it->second);
#endif
        break;
    }
    case TK_CHAR8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mChar8Value;
#else
        auto it = mValues.begin();
        cdr >> *((char*)it->second);
#endif
        break;
    }
    case TK_CHAR16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mChar16Value;
#else
        auto it = mValues.begin();
        cdr >> *((wchar_t*)it->second);
#endif
        break;
    }
    case TK_BOOLEAN:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mBoolValue;
#else
        auto it = mValues.begin();
        cdr >> *((bool*)it->second);
#endif
        break;
    }
    case TK_BYTE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mByteValue;
#else
        auto it = mValues.begin();
        cdr >> *((octet*)it->second);
#endif
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mStringValue;
#else
        auto it = mValues.begin();
        cdr >> *((std::string*)it->second);
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mWStringValue;
#else
        auto it = mValues.begin();
        cdr >> *((std::wstring*)it->second);
#endif
        break;
    }
    case TK_ENUM:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mUInt32Value;
#else
        auto it = mValues.begin();
        cdr >> *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_BITSET:
    case TK_BITMASK:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> mUInt64Value;
#else
        auto it = mValues.begin();
        cdr >> *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_UNION:
    {
        cdr >> mUnionId;
        SetUnionId(mUnionId);
        if (mUnionId != MEMBER_ID_INVALID)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = mComplexValues.at(mUnionId);
#else
            DynamicData* it = (DynamicData*) mValues.at(mUnionId);
#endif
            it->deserialize(cdr);
        }
        break;
    }
    case TK_STRUCTURE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        //uint32_t size(static_cast<uint32_t>(mComplexValues.size())), memberId(MEMBER_ID_INVALID);
        for (uint32_t i = 0; i < mComplexValues.size(); ++i)
        {
            //cdr >> memberId;
            auto it = mComplexValues.find(i);
            if (it != mComplexValues.end())
            {
                it->second->deserialize(cdr);
            }
            else
            {
                DynamicData* pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                pData->deserialize(cdr);
                mComplexValues.insert(std::make_pair(i, pData));
            }
        }
#else
        //uint32_t size(static_cast<uint32_t>(mValues.size())), memberId(MEMBER_ID_INVALID);
        for (uint32_t i = 0; i < mValues.size(); ++i)
        {
            //cdr >> memberId;
            auto it = mValues.find(i);
            if (it != mValues.end())
            {
                ((DynamicData*)it->second)->deserialize(cdr);
            }
            else
            {
                DynamicData* pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                pData->deserialize(cdr);
                mValues.insert(std::make_pair(i, pData));
            }
        }
#endif
    }
    break;
    case TK_ARRAY:
    {
        uint32_t size(0);
        cdr >> size;
        if (size > 0)
        {
            DynamicData* inputData(nullptr);
            for (uint32_t i = 0; i < size; ++i)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = mComplexValues.find(i);
                if (it != mComplexValues.end())
                {
                    it->second->deserialize(cdr);
                }
                else
                {
                    if (inputData == nullptr)
                    {
                        inputData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                    }

                    inputData->deserialize(cdr);
                    if (!inputData->Equals(mDefaultArrayValue))
                    {
                        mComplexValues.insert(std::make_pair(i, inputData));
                        inputData = nullptr;
                    }
                }
#else
                auto it = mValues.find(i);
                if (it != mValues.end())
                {
                    ((DynamicData*)it->second)->deserialize(cdr);
                }
                else
                {
                    if (inputData == nullptr)
                    {
                        inputData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                    }

                    inputData->deserialize(cdr);
                    if (!inputData->Equals(mDefaultArrayValue))
                    {
                        mValues.insert(std::make_pair(i, inputData));
                        inputData = nullptr;
                    }
                }
#endif
            }
            if (inputData != nullptr)
            {
                DynamicDataFactory::GetInstance()->DeleteData(inputData);
            }
        }
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        uint32_t size(0);
        bool bKeyElement(false);
        cdr >> size;
        for (uint32_t i = 0; i < size; ++i)
        {
            //cdr >> memberId;
            if (GetKind() == TK_MAP)
            {
                bKeyElement = !bKeyElement;
            }

#ifdef DYNAMIC_TYPES_CHECKING
            auto it = mComplexValues.find(i);
            if (it != mComplexValues.end())
            {
                it->second->deserialize(cdr);
                it->second->mIsKeyElement = bKeyElement;
            }
            else
            {
                DynamicData* pData = nullptr;
                if (bKeyElement)
                {
                    pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetKeyElementType());
                }
                else
                {
                    pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                }
                pData->deserialize(cdr);
                pData->mIsKeyElement = bKeyElement;
                mComplexValues.insert(std::make_pair(i, pData));
            }
#else
            auto it = mValues.find(i);
            if (it != mValues.end())
            {
                ((DynamicData*)it->second)->deserialize(cdr);
                ((DynamicData*)it->second)->mIsKeyElement = bKeyElement;
            }
            else
            {
                DynamicData* pData = nullptr;
                if (bKeyElement)
                {
                    pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetKeyElementType());
                }
                else
                {
                    pData = DynamicDataFactory::GetInstance()->CreateData(mType->GetElementType());
                }
                pData->deserialize(cdr);
                pData->mIsKeyElement = bKeyElement;
                mValues.insert(std::make_pair(i, pData));
            }
#endif
        }
        break;
    }

    case TK_ALIAS:
        break;
    }
    return true;
}

size_t DynamicData::getCdrSerializedSize(const DynamicData* data, size_t current_alignment /*= 0*/)
{
    size_t initial_alignment = current_alignment;

    switch (data->GetKind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    case TK_BITSET:
    case TK_BITMASK:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16);
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        // string content (length + characters + 1)
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            data->mStringValue.length() + 1;
#else
        auto it = data->mValues.begin();
        // string content (length + characters + 1)
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            ((std::string*)it->second)->length() + 1;
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        // string content (length + ((characters + 1) * 4) )
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            (data->mWStringValue.length() + 1) * 4;
#else
        auto it = data->mValues.begin();
        // string content (length + ((characters + 1) * 4) )
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            (((std::wstring*)it->second)->length() + 1) * 4;
#endif
        break;
    }
    case TK_UNION:
    {
        // union id
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        if (data->mUnionId != MEMBER_ID_INVALID)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = data->mComplexValues.at(data->mUnionId);
#else
            auto it = (DynamicData*)data->mValues.at(data->mUnionId);
#endif
            current_alignment += getCdrSerializedSize(it, current_alignment);
        }
        break;
    }
    case TK_STRUCTURE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = data->mComplexValues.begin(); it != data->mComplexValues.end(); ++it)
        {
            current_alignment += getCdrSerializedSize(it->second, current_alignment);
        }
#else
        for (auto it = data->mValues.begin(); it != data->mValues.end(); ++it)
        {
            current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
        }
#endif
        break;
    }
    case TK_ARRAY:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        uint32_t arraySize = data->mType->GetTotalBounds();
        for (uint32_t idx = 0; idx < arraySize; ++idx)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = data->mComplexValues.find(idx);
            if (it != data->mComplexValues.end())
#else
            auto it = data->mValues.find(idx);
            if (it != data->mValues.end())
#endif
            {
                // Element Size
                current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
            }
            else
            {
                current_alignment += getEmptyCdrSerializedSize(data->mType->GetElementType(), current_alignment);
            }
        }
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = data->mComplexValues.begin(); it != data->mComplexValues.end(); ++it)
        {
            // Element Size
            current_alignment += getCdrSerializedSize(it->second, current_alignment);
        }
#else
        for (auto it = data->mValues.begin(); it != data->mValues.end(); ++it)
        {
            // Element Size
            current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
        }
#endif
        break;
    }
    case TK_ALIAS:
        break;
    }

    return current_alignment - initial_alignment;
}

size_t DynamicData::getMaxCdrSerializedSize(const DynamicType* type, size_t current_alignment /*= 0*/)
{
    size_t initial_alignment = current_alignment;

    switch (type->GetKind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    case TK_BITSET:
    case TK_BITMASK:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += sizeof(long double) + eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(long double));
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
        // string length + string content + 1
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + type->GetBounds() + 1;
        break;
    }
    case TK_STRING16:
    {
        // string length + ( string content + 1 ) * 4
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + ((type->GetBounds() + 1) * 4);
        break;
    }
    case TK_UNION:
    {
        // union id
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Check the size of all members and take the size of the biggest one.
        size_t temp_size(0);
        size_t max_element_size(0);
        for (auto it = type->mMemberById.begin(); it != type->mMemberById.end(); ++it)
        {
            temp_size = getMaxCdrSerializedSize(it->second->mDescriptor.mType, current_alignment);
            if (temp_size > max_element_size)
            {
                max_element_size = temp_size;
            }
        }
        current_alignment += max_element_size;
        break;
    }
    case TK_STRUCTURE:
    {
        for (auto it = type->mMemberById.begin(); it != type->mMemberById.end(); ++it)
        {
            current_alignment += getMaxCdrSerializedSize(it->second->mDescriptor.mType, current_alignment);
        }
        break;
    }
    case TK_ARRAY:
    case TK_SEQUENCE:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Element size with the maximum size
        current_alignment += type->GetTotalBounds() * (getMaxCdrSerializedSize(type->mDescriptor->GetElementType()));
        break;
    }
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Key Elements size with the maximum size
        current_alignment += type->GetTotalBounds() * (getMaxCdrSerializedSize(type->mDescriptor->GetKeyElementType()));

        // Value Elements size with the maximum size
        current_alignment += type->GetTotalBounds() * (getMaxCdrSerializedSize(type->mDescriptor->GetElementType()));
        break;
    }

    case TK_ALIAS:
        break;
    }

    return current_alignment - initial_alignment;
}

void DynamicData::serialize(eprosima::fastcdr::Cdr &cdr) const
{
    switch (GetKind())
    {
    default:
        break;
    case TK_INT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mInt32Value;
#else
        auto it = mValues.begin();
        cdr << *((int32_t*)it->second);
#endif
        break;
    }
    case TK_UINT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mUInt32Value;
#else
        auto it = mValues.begin();
        cdr << *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_INT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mInt16Value;
#else
        auto it = mValues.begin();
        cdr << *((int16_t*)it->second);
#endif
        break;
    }
    case TK_UINT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mUInt16Value;
#else
        auto it = mValues.begin();
        cdr << *((uint16_t*)it->second);
#endif
        break;
    }
    case TK_INT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mInt64Value;
#else
        auto it = mValues.begin();
        cdr << *((int64_t*)it->second);
#endif
        break;
    }
    case TK_UINT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mUInt64Value;
#else
        auto it = mValues.begin();
        cdr << *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_FLOAT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mFloat32Value;
#else
        auto it = mValues.begin();
        cdr << *((float*)it->second);
#endif
        break;
    }
    case TK_FLOAT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mFloat64Value;
#else
        auto it = mValues.begin();
        cdr << *((double*)it->second);
#endif
        break;
    }
    case TK_FLOAT128:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mFloat128Value;
#else
        auto it = mValues.begin();
        cdr << *((long double*)it->second);
#endif
        break;
    }
    case TK_CHAR8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mChar8Value;
#else
        auto it = mValues.begin();
        cdr << *((char*)it->second);
#endif
        break;
    }
    case TK_CHAR16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mChar16Value;
#else
        auto it = mValues.begin();
        cdr << *((wchar_t*)it->second);
#endif
        break;
    }
    case TK_BOOLEAN:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mBoolValue;
#else
        auto it = mValues.begin();
        cdr << *((bool*)it->second);
#endif
        break;
    }
    case TK_BYTE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mByteValue;
#else
        auto it = mValues.begin();
        cdr << *((octet*)it->second);
#endif
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mStringValue;
#else
        auto it = mValues.begin();
        cdr << *((std::string*)it->second);
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mWStringValue;
#else
        auto it = mValues.begin();
        cdr << *((std::wstring*)it->second);
#endif
        break;
    }
    case TK_ENUM:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mUInt32Value;
#else
        auto it = mValues.begin();
        cdr << *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_BITSET:
    case TK_BITMASK:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << mUInt64Value;
#else
        auto it = mValues.begin();
        cdr << *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_UNION:
    {
        cdr << mUnionId;
        if (mUnionId != MEMBER_ID_INVALID)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = mComplexValues.at(mUnionId);
#else
            auto it = (DynamicData*) mValues.at(mUnionId);
#endif
            it->serialize(cdr);
        }
        break;
    }
    case TK_SEQUENCE: // Sequence is like structure, but with size
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << static_cast<uint32_t>(mComplexValues.size());
#else
        cdr << static_cast<uint32_t>(mValues.size());
#endif
    case TK_STRUCTURE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(mComplexValues.size()); ++idx)
        {
            auto it = mComplexValues.at(idx);
            it->serialize(cdr);
        }
#else
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(mValues.size()); ++idx)
        {
            auto it = mValues.at(idx);
            ((DynamicData*)it)->serialize(cdr);
        }
#endif
        break;
    }
    case TK_ARRAY:
    {
        uint32_t arraySize = mType->GetTotalBounds();
        cdr << arraySize;
        for (uint32_t idx = 0; idx < arraySize; ++idx)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = mComplexValues.find(idx);
            if (it != mComplexValues.end())
#else
            auto it = mValues.find(idx);
            if (it != mValues.end())
#endif
            {
                ((DynamicData*)it->second)->serialize(cdr);
            }
            else
            {
                SerializeEmptyData(mType->GetElementType(), cdr);
            }
        }
        break;
    }
    case TK_MAP:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << static_cast<uint32_t>(mComplexValues.size());
        for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
        {
            it->second->serialize(cdr);
        }
#else
        cdr << static_cast<uint32_t>(mValues.size());
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            ((DynamicData*)it->second)->serialize(cdr);
        }
#endif
        break;
    }
    case TK_ALIAS:
        break;
    }
}

size_t DynamicData::getEmptyCdrSerializedSize(const DynamicType* type, size_t current_alignment /*= 0*/)
{
    size_t initial_alignment = current_alignment;

    switch (type->GetKind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    case TK_BITSET:
    case TK_BITMASK:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += sizeof(long double) + eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(long double));
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
        // string length + 1
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 1;
        break;
    }
    case TK_STRING16:
    {
        // string length +  4
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 4;
        break;
    }
    case TK_UNION:
    {
        // union id
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_STRUCTURE:
    {
        for (auto it = type->mMemberById.begin(); it != type->mMemberById.end(); ++it)
        {
            current_alignment += getEmptyCdrSerializedSize(it->second->mDescriptor.mType, current_alignment);
        }
        break;
    }
    case TK_ARRAY:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Element size with the maximum size
        current_alignment += type->GetTotalBounds() * (getEmptyCdrSerializedSize(type->mDescriptor->GetElementType()));
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }

    case TK_ALIAS:
        break;
    }

    return current_alignment - initial_alignment;
}

void DynamicData::SerializeEmptyData(const DynamicType* pType, eprosima::fastcdr::Cdr &cdr) const
{
    switch(pType->GetKind())
    {
    default:
        break;
    case TK_INT32:
    {
        cdr << static_cast<int32_t>(0);
        break;
    }
    case TK_UINT32:
    {
        cdr << static_cast<uint32_t>(0);
        break;
    }
    case TK_INT16:
    {
        cdr << static_cast<int16_t>(0);
        break;
    }
    case TK_UINT16:
    {
        cdr << static_cast<uint16_t>(0);
        break;
    }
    case TK_INT64:
    {
        cdr << static_cast<int64_t>(0);
        break;
    }
    case TK_UINT64:
    {
        cdr << static_cast<uint64_t>(0);
        break;
    }
    case TK_FLOAT32:
    {
        cdr << static_cast<float>(0.0f);
        break;
    }
    case TK_FLOAT64:
    {
        cdr << static_cast<double>(0.0);
        break;
    }
    case TK_FLOAT128:
    {
        cdr << static_cast<long double>(0.0);
        break;
    }
    case TK_CHAR8:
    {
        cdr << static_cast<char>(0);
        break;
    }
    case TK_CHAR16:
    {
        cdr << static_cast<uint32_t>(0);
        break;
    }
    case TK_BOOLEAN:
    {
        cdr << static_cast<uint8_t>(0);
        break;
    }
    case TK_BYTE:
    {
        cdr << static_cast<uint8_t>(0);
        break;
    }
    case TK_STRING8:
    {
        cdr << std::string();
        break;
    }
    case TK_STRING16:
    {
        cdr << std::wstring();
        break;
    }
    case TK_ENUM:
    {
        cdr << static_cast<uint32_t>(0);
        break;
    }
    case TK_BITSET:
    case TK_BITMASK:
    {
        cdr << static_cast<uint64_t>(0);
        break;
    }
    case TK_UNION:
    {
        cdr << static_cast<uint32_t>(MEMBER_ID_INVALID);
        break;
    }
    case TK_SEQUENCE: // Sequence is like structure, but with size
    {
        cdr << static_cast<uint32_t>(0);
        break;
    }
    case TK_STRUCTURE:
    {
        for (uint32_t idx = 0; idx < pType->mMemberById.size(); ++idx)
        {
            auto it = pType->mMemberById.at(idx);
            SerializeEmptyData(it->mDescriptor.mType, cdr);
        }
        break;
    }
    case TK_ARRAY:
    {
        uint32_t arraySize = pType->GetTotalBounds();
        cdr << arraySize;
        for (uint32_t i = 0; i < arraySize; ++i)
        {
            SerializeEmptyData(pType->GetElementType(), cdr);
        }
        break;
    }
    case TK_MAP:
    {
        cdr << static_cast<uint32_t>(0);
        break;
    }
    case TK_ALIAS:
        break;
    }
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
