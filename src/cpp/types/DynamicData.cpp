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
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>
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
        return a.first == b.first && a.second->equals(b.second);
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
    , mItemCount(0)
{
}

DynamicData::DynamicData(DynamicType* pType)
    : mType(pType)
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
    , mItemCount(0)
{
    std::map<MemberId, DynamicTypeMember*> members;
    if (mType->get_all_members(members) == ResponseCode::RETCODE_OK)
    {
        if (mType->is_complex_kind())
        {
            for (auto it = members.begin(); it != members.end(); ++it)
            {
                MemberDescriptor* newDescriptor = new MemberDescriptor();
                if (it->second->get_descriptor(newDescriptor) == ResponseCode::RETCODE_OK)
                {
                    mDescriptors.insert(std::make_pair(it->first, newDescriptor));
#ifdef DYNAMIC_TYPES_CHECKING
                    mComplexValues.insert(std::make_pair(it->first, new DynamicData(newDescriptor->mType)));
#else
                    mValues.insert(std::make_pair(it->first, new DynamicData(newDescriptor->mType)));
#endif
                }
                else
                {
                    delete newDescriptor;
                }
            }
        }
        else
        {
            AddValue(pType->get_kind(), MEMBER_ID_INVALID);
        }
    }
}

DynamicData::~DynamicData()
{
    Clean();
}

ResponseCode DynamicData::get_descriptor(MemberDescriptor& value, MemberId id)
{
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        value.copy_from(it->second);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting MemberDescriptor. MemberId not found.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicData::set_descriptor(MemberId id, const MemberDescriptor* value)
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

bool DynamicData::equals(const DynamicData* other)
{
    if (other != nullptr && mItemCount == other->mItemCount && mType->equals(other->mType) &&
        mDescriptors.size() == other->mDescriptors.size())
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            auto otherIt = other->mDescriptors.find(it->first);
            if (otherIt == other->mDescriptors.end() || !it->second->equals(otherIt->second))
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

        if (mType->is_complex_kind())
        {
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                auto otherDescIt = other->mDescriptors.find(it->first);
                if (otherDescIt == other->mDescriptors.end() || !it->second->equals(otherDescIt->second))
                {
                    return false;
                }

                auto otherIt = other->mValues.find(it->first);
                if (!((DynamicData*)mValues[it->first])->equals(((DynamicData*) otherIt->second)))
                {
                    return false;
                }
            }
        }
        else
        {
            for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
            {
                auto otherDescIt = other->mDescriptors.find(it->first);
                if (otherDescIt == other->mDescriptors.end() || !it->second->equals(otherDescIt->second))
                {
                    return false;
                }

                auto otherIt = other->mValues.find(it->first);
                if (!CompareValues(it->second->get_kind(), mValues[it->first], otherIt->second))
                {
                    return false;
                }
            }
        }
#endif
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

MemberId DynamicData::get_member_id_by_name(const std::string& name)
{
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        if (it->second->get_name() == name)
        {
            return it->first;
        }
    }
    return MEMBER_ID_INVALID;
}

MemberId DynamicData::get_member_id_at_index(uint32_t index)
{
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        if (it->second->get_index() == index)
        {
            return it->first;
        }
    }
    return MEMBER_ID_INVALID;
}

uint32_t DynamicData::get_item_count()
{
    return mItemCount;
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
    }
    SetDefaultValue(id);
}

void DynamicData::Clean()
{
    if (mType != nullptr)
    {
        delete mType;
        mType = nullptr;
    }

    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        delete it->second;
    }
    mDescriptors.clear();
}

ResponseCode DynamicData::clear_all_values()
{
    if (mType->is_complex_kind())
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = mComplexValues.find(it->first);
            if (itValue != mComplexValues.end())
            {
                itValue->second->clear_all_values();
            }
#else
            auto itValue = mValues.find(it->first);
            if (itValue != mValues.end())
            {
                ((DynamicData*)itValue->second)->clear_all_values();
            }
#endif
        }
    }
    else
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            SetDefaultValue(it->first);
        }
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_nonkey_values()
{
    if (mType->is_complex_kind())
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = mComplexValues.find(it->first);
            if (itValue != mComplexValues.end())
            {
                itValue->second->clear_nonkey_values();
            }
#else
            auto itValue = mValues.find(it->first);
            if (itValue != mValues.end())
            {
                ((DynamicData*)itValue->second)->clear_nonkey_values();
            }
#endif
        }
    }
    else
    {
        //ARCE: //TODO: Avoid Key Elements.
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            SetDefaultValue(it->first);
        }
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_value(MemberId id)
{
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        if (mType->is_complex_kind())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = mComplexValues.find(it->first);
            if (itValue != mComplexValues.end())
            {
                itValue->second->clear_all_values();
            }
#else
            auto itValue = mValues.find(it->first);
            if (itValue != mValues.end())
            {
                ((DynamicData*)itValue->second)->clear_all_values();
            }
#endif
        }
        else
        {
            SetDefaultValue(id);
        }
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
        get_int32_value(*newInt32, id);
        return newInt32;
    }
    break;
    case TK_UINT32:
    {
        uint32_t* newUInt32 = new uint32_t();
        get_uint32_value(*newUInt32, id);
        return newUInt32;
    }
    break;
    case TK_INT16:
    {
        int16_t* newInt16 = new int16_t();
        get_int16_value(*newInt16, id);
        return newInt16;
    }
    break;
    case TK_UINT16:
    {
        uint16_t* newUInt16 = new uint16_t();
        get_uint16_value(*newUInt16, id);
        return newUInt16;
    }
    break;
    case TK_INT64:
    {
        int64_t* newInt64 = new int64_t();
        get_int64_value(*newInt64, id);
        return newInt64;
    }
    break;
    case TK_UINT64:
    {
        uint64_t* newUInt64 = new uint64_t();
        get_uint64_value(*newUInt64, id);
        return newUInt64;
    }
    break;
    case TK_FLOAT32:
    {
        float* newFloat32 = new float();
        get_float32_value(*newFloat32, id);
        return newFloat32;
    }
    break;
    case TK_FLOAT64:
    {
        double* newFloat64 = new double();
        get_float64_value(*newFloat64, id);
        return newFloat64;
    }
    break;
    case TK_FLOAT128:
    {
        long double* newFloat128 = new long double();
        get_float128_value(*newFloat128, id);
        return newFloat128;
    }
    break;
    case TK_CHAR8:
    {
        char* newChar8 = new char();
        get_char8_value(*newChar8, id);
        return newChar8;
    }
    break;
    case TK_CHAR16:
    {
        wchar_t* newChar16 = new wchar_t();
        get_char16_value(*newChar16, id);
        return newChar16;
    }
    break;
    case TK_BOOLEAN:
    {
        bool* newBool = new bool();
        get_bool_value(*newBool, id);
        return newBool;
    }
    break;
    case TK_BYTE:
    {
        octet* newByte = new octet();
        get_byte_value(*newByte, id);
        return newByte;
    }
    break;
    case TK_STRING8:
    {
        std::string* newString = new std::string();
        get_string_value(*newString, id);
        return newString;
    }
    break;
    case TK_STRING16:
    {
        std::wstring* newString = new std::wstring();
        get_wstring_value(*newString, id);
        return newString;
    }
    break;
    }
    return nullptr;
}

bool DynamicData::CompareValues(TypeKind kind, void* left, void* right)
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
        set_int32_value(id, value);
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
        set_uint32_value(id, value);
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
        set_int16_value(id, value);
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
        set_uint16_value(id, value);
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
        set_int64_value(id, value);
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
        set_uint64_value(id, value);
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
        set_float32_value(id, value);
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
        set_float64_value(id, value);
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
        set_float128_value(id, value);
    }
    break;
    case TK_CHAR8:
    {
        if (defaultValue.length() >= 1)
        {
            set_char8_value(id, defaultValue[0]);
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

        set_char16_value(id, value);
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
        set_bool_value(id, value == 1 ? true : false);
    }
    break;
    case TK_BYTE:
    {
        if (defaultValue.length() >= 1)
        {
            set_byte_value(id, defaultValue[0]);
        }
    }
    break;
    case TK_STRING8:
    {
        set_string_value(id, defaultValue);
    }
    break;
    case TK_STRING16:
    {
        set_wstring_value(id, std::wstring(defaultValue.begin(), defaultValue.end()));
    }
    break;
    }
}

DynamicData* DynamicData::loan_value(MemberId id)
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

ResponseCode DynamicData::return_loaned_value(const DynamicData* value)
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

DynamicData* DynamicData::clone() const
{
    DynamicData* newData = new DynamicData();
    newData->mType = new DynamicType(mType);
    newData->mItemCount = mItemCount;

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
        newData->mComplexValues.insert(std::make_pair(it->first, it->second->clone()));
    }

#else
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        newData->mDescriptors.insert(std::make_pair(it->first, new MemberDescriptor(it->second)));
    }

    if (mType->is_complex_kind())
    {
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            newData->mValues.insert(std::make_pair(it->first, ((DynamicData*)it->second)->clone()));
        }
    }
    else
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            newData->mValues.insert(std::make_pair(it->first, CloneValue(it->first, it->second->get_kind())));
        }
    }
#endif

    return newData;
}

ResponseCode DynamicData::get_int32_value(int32_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        value = mInt32Value;
        return ResponseCode::RETCODE_OK;

    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_int32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            value = *((int32_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_int32_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int32_value(MemberId id, int32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        mInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_int32_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            *((int32_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_int32_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint32_value(uint32_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        value = mUInt32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_uint32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            value = *((uint32_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_uint32_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint32_value(MemberId id, uint32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        mUInt32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_uint32_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            *((uint32_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_uint32_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int16_value(int16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        value = mInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_int16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            value = *((int16_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_int16_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int16_value(MemberId id, int16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        mInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_int16_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            *((int16_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_int16_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint16_value(uint16_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        value = mUInt16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_uint16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            value = *((uint16_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_uint16_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint16_value(MemberId id, uint16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        mUInt16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_uint16_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            *((uint16_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_uint16_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int64_value(int64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        value = mInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_int64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            value = *((int64_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_int64_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int64_value(MemberId id, int64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        mInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_int64_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            *((int64_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_int64_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint64_value(uint64_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        value = mUInt64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_uint64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            value = *((uint64_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_uint64_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint64_value(MemberId id, uint64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_UINT64 && id == MEMBER_ID_INVALID)
    {
        mUInt64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_uint64_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_UINT64 && id == MEMBER_ID_INVALID)
        {
            *((uint64_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_uint64_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float32_value(float& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        value = mFloat32Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_float32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            value = *((float*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_float32_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float32_value(MemberId id, float value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        mFloat32Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_float32_value(MEMBER_ID_INVALID, value);
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            *((float*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_float32_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float64_value(double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        value = mFloat64Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_float64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            value = *((double*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_float64_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float64_value(MemberId id, double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        mFloat64Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_float64_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            *((double*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_float64_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float128_value(long double& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        value = mFloat128Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_float128_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            value = *((long double*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_float128_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float128_value(MemberId id, long double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        mFloat128Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_float128_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            *((long double*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_float128_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char8_value(char& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        value = mChar8Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_char8_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            value = *((char*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_char8_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char8_value(MemberId id, char value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        mChar8Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_char8_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            *((char*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_char8_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char16_value(wchar_t& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        value = mChar16Value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_char16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            value = *((wchar_t*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_char16_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char16_value(MemberId id, wchar_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        mChar16Value = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_char16_value(MEMBER_ID_INVALID, value);
        }
    }

    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            *((wchar_t*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_char16_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_byte_value(octet& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        value = mByteValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_byte_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            value = *((octet*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_byte_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_byte_value(MemberId id, octet value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        mByteValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_byte_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            *((octet*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_byte_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_bool_value(bool& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        value = mBoolValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_bool_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            value = *((bool*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_bool_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_bool_value(MemberId id, bool value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        mBoolValue = value;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->set_bool_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            *((bool*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_bool_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_string_value(std::string& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        value = mStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_string_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            value = *((std::string*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_string_value(value, MEMBER_ID_INVALID);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_string_value(MemberId id, const std::string& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_STRING8 &&id == MEMBER_ID_INVALID)
    {
        if (value.length() <= mType->get_bounds(0))
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
            return it->second->set_string_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            *((std::string*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_string_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
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

ResponseCode DynamicData::get_wstring_value(std::wstring& value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        value = mWStringValue;
        return ResponseCode::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = mComplexValues.find(id);
        if (it != mComplexValues.end())
        {
            return it->second->get_wstring_value(value, MEMBER_ID_INVALID);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            value = *((std::wstring*)it->second);
        }
        else
        {
            return ((DynamicData*)it->second)->get_wstring_value(value, MEMBER_ID_INVALID);
        }

        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_wstring_value(MemberId id, const std::wstring& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (mType->get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= mType->get_bounds(0))
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
            return it->second->set_wstring_value(MEMBER_ID_INVALID, value);
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (mType->get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            *((std::wstring*)it->second) = value;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            return ((DynamicData*)it->second)->set_wstring_value(MEMBER_ID_INVALID, value);
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

/*
ResponseCode DynamicData::get_complex_value(DynamicData* value, MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(id);
    if (it != mComplexValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = ((DynamicData*)it->second)->clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_complex_value(MemberId id, const DynamicData* value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mComplexValues.find(id);
    if (it != mComplexValues.end())
    {
        it->second = value->clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        if (it->second != nullptr)
            delete it->second;
        it->second = value->clone();
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif

    return ResponseCode::RETCODE_OK;
}
*/

} // namespace types
} // namespace fastrtps
} // namespace eprosima
