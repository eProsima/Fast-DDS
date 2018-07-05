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
{
    std::map<MemberId, DynamicTypeMember*> members;
    if (mType->GetAllMembers(members) == ResponseCode::RETCODE_OK)
    {
        if (mType->IsComplexKind())
        {
            // Bitmasks and enums register their members but only manages one value.
            if (mType->GetKind() == TK_BITMASK || mType->GetKind() == TK_ENUM)
            {
                AddValue(mType->GetKind(), MEMBER_ID_INVALID);
            }
            for (auto it = members.begin(); it != members.end(); ++it)
            {
                MemberDescriptor* newDescriptor = new MemberDescriptor();
                if (it->second->GetDescriptor(newDescriptor) == ResponseCode::RETCODE_OK)
                {
                    mDescriptors.insert(std::make_pair(it->first, newDescriptor));
                    if (mType->GetKind() != TK_BITMASK && mType->GetKind() != TK_ENUM)
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
        }
        else
        {
            AddValue(mType->GetKind(), MEMBER_ID_INVALID);
        }
    }
}

DynamicData::~DynamicData()
{
    Clean();
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
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                auto otherIt = other->mDescriptors.find(it->first);
                if (otherIt == other->mDescriptors.end() || !it->second->Equals(otherIt->second))
                {
                    return false;
                }
            }

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

            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                auto otherDescIt = other->mDescriptors.find(it->first);
                if (otherDescIt == other->mDescriptors.end() || !it->second->Equals(otherDescIt->second))
                {
                    return false;
                }
            }

            if (mType->IsComplexKind())
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
            return true;
        }
    }
    return false;
}

MemberId DynamicData::GetMemberIdByName(const std::string& name)
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

MemberId DynamicData::GetMemberIdAtIndex(uint32_t index)
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

TypeKind DynamicData::GetKind()
{
    return mType->GetKind();
}

uint32_t DynamicData::GetItemCount() const
{
    if (mType->GetKind() == TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(mComplexValues.size() / 2);
#else
        return static_cast<uint32_t>(mValues.size() / 2);
#endif
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
        if (mType->GetKind() == TK_SEQUENCE || mType->GetKind() == TK_MAP || mType->GetKind() == TK_ARRAY)
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
        SetInt32Value(id, value);
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
        SetUint32Value(id, value);
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
        SetInt16Value(id, value);
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
        SetUint16Value(id, value);
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
        SetInt64Value(id, value);
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
        SetUint64Value(id, value);
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
        SetFloat32Value(id, value);
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
        SetFloat64Value(id, value);
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
        SetFloat128Value(id, value);
    }
    break;
    case TK_CHAR8:
    {
        if (defaultValue.length() >= 1)
        {
            SetChar8Value(id, defaultValue[0]);
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

        SetChar16Value(id, value);
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
        SetBoolValue(id, value == 1 ? true : false);
    }
    break;
    case TK_BYTE:
    {
        if (defaultValue.length() >= 1)
        {
            SetByteValue(id, defaultValue[0]);
        }
    }
    break;
    case TK_STRING8:
    {
        SetStringValue(id, defaultValue);
    }
    break;
    case TK_STRING16:
    {
        SetWstringValue(id, std::wstring(defaultValue.begin(), defaultValue.end()));
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
        SetUint32Value(id, value);
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
        SetBoolValue(id, value == 1 ? true : false);
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
    if (std::find(mLoanedValues.begin(), mLoanedValues.end(), id) != mLoanedValues.end())
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            mLoanedValues.push_back(id);
            return it->second;
        }
#else
        auto it = mValues.find(id);
        if (it != mValues.end())
        {
            mLoanedValues.push_back(id);
            return (DynamicData*)it->second;
        }
#endif
        else
        {
            logError(DYN_TYPES, "Error loaning Value. MemberId not found.");
            return nullptr;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error loaning Value. The value has been loaned previously.");
        return nullptr;
    }
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
    if (mType->GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        value = mInt32Value;
        return ResponseCode::RETCODE_OK;

    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetInt32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            value = *((int32_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if(id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetInt32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt32Value(MemberId id, int32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        mInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetInt32Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt32Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            *((int32_t*)it->second) = value;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetInt32Value(MEMBER_ID_INVALID, value);
        }
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt32Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint32Value(uint32_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        value = mUInt32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetUint32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            value = *((uint32_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetUint32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint32Value(MemberId id, uint32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        mUInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetUint32Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint32Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            *((uint32_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetUint32Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint32Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetInt16Value(int16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        value = mInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetInt16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            value = *((int16_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetInt16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt16Value(MemberId id, int16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        mInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetInt16Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt16Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            *((int16_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetInt16Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt16Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint16Value(uint16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        value = mUInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetUint16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            value = *((uint16_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetUint16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint16Value(MemberId id, uint16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        mUInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetUint16Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint16Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            *((uint16_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetUint16Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint16Value(id, value);
        }
        return insertResult;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetInt64Value(int64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        value = mInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetInt64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            value = *((int64_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetInt64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetInt64Value(MemberId id, int64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        mInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetInt64Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetInt64Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            *((int64_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetInt64Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetInt64Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetUint64Value(uint64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        value = mUInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetUint64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            value = *((uint64_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetUint64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetUint64Value(MemberId id, uint64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        mUInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetUint64Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetUint64Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            *((uint64_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetUint64Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetUint64Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat32Value(float& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        value = mFloat32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetFloat32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            value = *((float*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetFloat32Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat32Value(MemberId id, float value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        mFloat32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetFloat32Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat32Value(id, value);
            }
            return insertResult;
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            *((float*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetFloat32Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat32Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat64Value(double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        value = mFloat64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetFloat64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            value = *((double*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetFloat64Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat64Value(MemberId id, double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        mFloat64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetFloat64Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat64Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            *((double*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetFloat64Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat64Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetFloat128Value(long double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        value = mFloat128Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetFloat128Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            value = *((long double*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetFloat128Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetFloat128Value(MemberId id, long double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        mFloat128Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetFloat128Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetFloat128Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            *((long double*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetFloat128Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetFloat128Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetChar8Value(char& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        value = mChar8Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetChar8Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            value = *((char*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetChar8Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetChar8Value(MemberId id, char value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        mChar8Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetChar8Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetChar8Value(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            *((char*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetChar8Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetChar8Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetChar16Value(wchar_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        value = mChar16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetChar16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            value = *((wchar_t*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetChar16Value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetChar16Value(MemberId id, wchar_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        mChar16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetChar16Value(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetChar16Value(id, value);
            }
            return insertResult;
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            *((wchar_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetChar16Value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetChar16Value(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetByteValue(octet& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        value = mByteValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetByteValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            value = *((octet*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetByteValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetByteValue(MemberId id, octet value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        mByteValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->SetByteValue(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetByteValue(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            *((octet*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->SetByteValue(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetByteValue(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetBoolValue(bool& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        value = mBoolValue;
        return ResponseCode::RETCODE_OK;
    }
    else if ((mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK) && id < mType->GetBounds())
    {
        value = (mUInt64Value & ((uint64_t)1 << id)) != 0;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetBoolValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.end();
    if (mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK)
    {
        it = mValues.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = mValues.find(id);
    }
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            value = *((bool*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if ((mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK) && id < mType->GetBounds())
        {
            value = (*((uint64_t*)it->second) & ((uint64_t)1 << id)) != 0;
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetBoolValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetBoolValue(MemberId id, bool value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        mBoolValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK)
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
            return it->second->SetBoolValue(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetBoolValue(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.end();
    if (mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK)
    {
        it = mValues.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = mValues.find(id);
    }

    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            *((bool*)it->second) = value;
        }
        else if (mType->GetKind() == TK_BITSET || mType->GetKind() == TK_BITMASK)
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
            return ((DynamicData*)it->second)->SetBoolValue(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetBoolValue(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::GetStringValue(std::string& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        value = mStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetStringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            value = *((std::string*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetStringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetStringValue(MemberId id, const std::string& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_STRING8 &&id == MEMBER_ID_INVALID)
    {
        if (mType->GetBounds() == LENGTH_UNLIMITED || value.length() <= mType->GetBounds())
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
            return it->second->SetStringValue(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetStringValue(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            if (mType->GetBounds() == LENGTH_UNLIMITED || value.length() <= mType->GetBounds())
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
            return ((DynamicData*)it->second)->SetStringValue(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetStringValue(id, value);
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

ResponseCode DynamicData::GetWstringValue(std::wstring& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        value = mWStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->GetWstringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            value = *((std::wstring*)it->second);
            return ResponseCode::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->GetWstringValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}


ResponseCode DynamicData::SetWstringValue(MemberId id, const std::wstring& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        if (mType->GetBounds() == LENGTH_UNLIMITED || value.length() <= mType->GetBounds())
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
            return it->second->SetWstringValue(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetWstringValue(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->GetKind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            if (mType->GetBounds() == LENGTH_UNLIMITED || value.length() <= mType->GetBounds())
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
            return ((DynamicData*)it->second)->SetWstringValue(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetWstringValue(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}
ResponseCode DynamicData::GetEnumValue(std::string& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
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
            return it->second->GetEnumValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto itValue = mValues.find(id);
    if (itValue != mValues.end())
    {
        if (mType->GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
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
            return ((DynamicData*)itValue->second)->GetEnumValue(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetEnumValue(MemberId id, const std::string& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
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
            return it->second->SetEnumValue(MEMBER_ID_INVALID, value);
        }
        else if (mType->GetKind() == TK_ARRAY)
        {
            ResponseCode insertResult = InsertArrayData(id);
            if (insertResult == ResponseCode::RETCODE_OK)
            {
                return SetEnumValue(id, value);
            }
            return insertResult;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto itValue = mValues.find(id);
    if (itValue != mValues.end())
    {
        if (mType->GetKind() == TK_ENUM && id == MEMBER_ID_INVALID)
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
            return ((DynamicData*)itValue->second)->SetEnumValue(MEMBER_ID_INVALID, value);
        }
    }
    else if (mType->GetKind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetEnumValue(id, value);
        }
        return insertResult;
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

void DynamicData::SortMemberIds(MemberId startId)
{
    MemberId curID = startId + 1;
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(curID);
    while (it != mComplexValues.end())
    {
        mComplexValues[curID - 1] = it->second;
        mComplexValues.erase(it);
        it = mComplexValues.find(++curID);
    }
#else
    auto it = mValues.find(curID);
    while (it != mValues.end())
    {
        mValues[curID - 1] = it->second;
        mValues.erase(it);
        it = mValues.find(++curID);
    }
#endif
}

MemberId DynamicData::GetArrayIndex(const std::vector<uint32_t>& position)
{
    if (mType->GetKind() == TK_ARRAY)
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
        logError(DYN_TYPES, "Error getting array index. The kind " << mType->GetKind() << "doesn't support it.");
    }
    return MEMBER_ID_INVALID;
}

ResponseCode DynamicData::InsertArrayData(MemberId indexId)
{
    if (mType->GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (mType->GetTotalBounds() == LENGTH_UNLIMITED || indexId < mType->GetTotalBounds())
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
        if (mType->GetTotalBounds() == LENGTH_UNLIMITED || indexId < mType->GetTotalBounds())
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
        logError(DYN_TYPES, "Error inserting data. The kind " << mType->GetKind() << " doesn't support this method");
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::RemoveArrayData(MemberId indexId)
{
    if (mType->GetKind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (mType->GetBounds() == LENGTH_UNLIMITED || indexId < mType->GetTotalBounds())
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
        if (mType->GetBounds() == LENGTH_UNLIMITED || mValues.size() < mType->GetBounds())
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
        logError(DYN_TYPES, "Error removing data. The kind " << mType->GetKind() << " doesn't support this method");
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::InsertSequenceData(MemberId& outId)
{
    outId = MEMBER_ID_INVALID;
    if (mType->GetKind() == TK_SEQUENCE)
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
        logError(DYN_TYPES, "Error inserting data. The kind " << mType->GetKind() << " doesn't support this method");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::RemoveSequenceData(MemberId id)
{
    if (mType->GetKind() == TK_SEQUENCE || mType->GetKind() == TK_ARRAY)
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

    logError(DYN_TYPES, "Error removing data. The current Kind " << mType->GetKind()
        << " doesn't support this method");

    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::InsertMapData(DynamicData* key, MemberId& outKeyId, MemberId& outValueId)
{
    if (mType->GetKind() == TK_MAP)
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
        logError(DYN_TYPES, "Error inserting to map. The current Kind " << mType->GetKind()
            << " doesn't support this method");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::RemoveMapData(MemberId keyId)
{
    if (mType->GetKind() == TK_MAP)
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
        logError(DYN_TYPES, "Error removing from map. The current Kind " << mType->GetKind()
            << " doesn't support this method");
        return ResponseCode::RETCODE_ERROR;
    }
}

ResponseCode DynamicData::ClearData()
{
    if (mType->GetKind() == TK_SEQUENCE || mType->GetKind() == TK_MAP || mType->GetKind() == TK_ARRAY)
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

    logError(DYN_TYPES, "Error clearing data. The current Kind " << mType->GetKind()
        << " doesn't support this method");

    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::GetComplexValue(DynamicData* value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(id);
    if (it != mComplexValues.end())
    {
        value = it->second->Clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = ((DynamicData*)it->second)->Clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::SetComplexValue(MemberId id, DynamicData* value)
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
    else if (mType->GetKind() == TK_ARRAY)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetComplexValue(id, value);
        }
        return insertResult;
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
    else if (mType->GetKind() == TK_ARRAY)
    {
        ResponseCode insertResult = InsertArrayData(id);
        if (insertResult == ResponseCode::RETCODE_OK)
        {
            return SetComplexValue(id, value);
        }
        return insertResult;
    }

    mValues.insert(std::make_pair(id, value));
#endif

    return ResponseCode::RETCODE_OK;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
