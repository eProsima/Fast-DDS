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
    , mItemCount(0)
{
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
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicData::set_descriptor(MemberId id, const MemberDescriptor* value)
{
    if (mDescriptors.find(id) == mDescriptors.end())
    {
        mDescriptors.insert(std::make_pair(id, new MemberDescriptor(value)));
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

bool DynamicData::equals(const DynamicData* other)
{
    if (other != nullptr && mItemCount == other->mItemCount && mType->equals(other->mType) &&
        mDescriptors.size() == other->mDescriptors.size())
    {
        for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
        {
            auto otherIt = other->mDescriptors.find(it->first);
            if (otherIt == other->mDescriptors.end() || !it->second->equals(otherIt->second))
            {
                return false;
            }
        }
#ifdef DYNAMIC_TYPES_CHECKING
        if (!map_compare(mInt32Values, other->mInt32Values) ||
            !map_compare(mUInt32Values, other->mUInt32Values) ||
            !map_compare(mInt16Values, other->mInt16Values) ||
            !map_compare(mUInt16Values, other->mUInt16Values) ||
            !map_compare(mInt64Values, other->mInt64Values) ||
            !map_compare(mUInt64Values, other->mUInt64Values) ||
            !map_compare(mFloat32Values, other->mFloat32Values) ||
            !map_compare(mFloat64Values, other->mFloat64Values) ||
            !map_compare(mFloat128Values, other->mFloat128Values) ||
            !map_compare(mChar8Values, other->mChar8Values) ||
            !map_compare(mChar16Values, other->mChar16Values) ||
            !map_compare(mByteValues, other->mByteValues) ||
            !map_compare(mBoolValues, other->mBoolValues) ||
            !map_compare(mStringValues, other->mStringValues) ||
            !map_compare(mWStringValues, other->mWStringValues) ||
            !map_compare(mStringValues, other->mStringValues) ||
            !map_compare(mComplexValues, other->mComplexValues) ||
            !map_compare(mInt32ListValues, other->mInt32ListValues) ||
            !map_compare(mUInt32ListValues, other->mUInt32ListValues) ||
            !map_compare(mInt16ListValues, other->mInt16ListValues) ||
            !map_compare(mUInt16ListValues, other->mUInt16ListValues) ||
            !map_compare(mInt64ListValues, other->mInt64ListValues) ||
            !map_compare(mUInt64ListValues, other->mUInt64ListValues) ||
            !map_compare(mFloat32ListValues, other->mFloat32ListValues) ||
            !map_compare(mFloat64ListValues, other->mFloat64ListValues) ||
            !map_compare(mFloat128ListValues, other->mFloat128ListValues) ||
            !map_compare(mChar8ListValues, other->mChar8ListValues) ||
            !map_compare(mChar16ListValues, other->mChar16ListValues) ||
            !map_compare(mByteListValues, other->mByteListValues) ||
            !map_compare(mBoolListValues, other->mBoolListValues) ||
            !map_compare(mStringListValues, other->mStringListValues) ||
            !map_compare(mWStringListValues, other->mWStringListValues))
        {
            return false;
        }
#else
        //TODO: Check mValues
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
    if (index < mDescriptors.size())
    {
        auto it = mDescriptors.begin();
        for (uint32_t i = 0; i < index; ++i)
        {
            it = std::next(it);
        }
        return it->first;
    }
    return MEMBER_ID_INVALID;
}

uint32_t DynamicData::get_item_count()
{
    return mItemCount;
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
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        SetDefaultValue(it->first);
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_nonkey_values()
{
    //TODO: Avoid Key Values for Aggregated types ( UNION, STRUCTURES, ANNOTATIONS )
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        SetDefaultValue(it->first);
    }
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicData::clear_value(MemberId id)
{
    SetDefaultValue(id);
    return ResponseCode::RETCODE_OK;
}

void DynamicData::SetDefaultValue(MemberId id)
{
    auto it = mDescriptors.find(id);
    if (it != mDescriptors.end())
    {
        switch (it->second->get_kind())
        {
        default:
            break;
        case TK_INT32:
        {
            int32_t value(0);
            try
            {
                value = stoi(it->second->mDefaultValue);
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
                value = stoul(it->second->mDefaultValue);
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
                value = static_cast<int16_t>(stoi(it->second->mDefaultValue));
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
                value = static_cast<uint16_t>(stoul(it->second->mDefaultValue));
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
                value = stoll(it->second->mDefaultValue);
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
                value = stoul(it->second->mDefaultValue);
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
                value = stof(it->second->mDefaultValue);
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
                value = stod(it->second->mDefaultValue);
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
                value = stold(it->second->mDefaultValue);
            }
            catch (...) {}
            set_float128_value(id, value);
        }
        break;
        case TK_CHAR8:
        {
            if (it->second->mDefaultValue.length() >= 1)
            {
                set_char8_value(id, it->second->mDefaultValue[0]);
            }
        }
        break;
        case TK_CHAR16:
        {
            wchar_t value(0);
            try
            {
                std::wstring temp = std::wstring(it->second->mDefaultValue.begin(), it->second->mDefaultValue.end());
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
                value = stoi(it->second->mDefaultValue);
            }
            catch (...) {}
            set_bool_value(id, value == 1 ? true : false);
        }
            break;
        case TK_BYTE:
            if (it->second->mDefaultValue.length() >= 1)
            {
                set_byte_value(id, it->second->mDefaultValue[0]);
            }
            break;

        case TK_STRING8:
        {
            set_string_value(id, it->second->mDefaultValue);
        }
        break;
        case TK_STRING16:
        {
            set_wstring_value(id, std::wstring(it->second->mDefaultValue.begin(), it->second->mDefaultValue.end()));
        }
        break;
        }
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
    }
    return nullptr;
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
    return ResponseCode::RETCODE_PRECONDITION_NOT_MET;
}

DynamicData* DynamicData::clone() const
{
    DynamicData* newData = new DynamicData();
    newData->mType = new DynamicType(mType);
    newData->mItemCount = mItemCount;
    for (auto it = mDescriptors.begin(); it != mDescriptors.end(); ++it)
    {
        newData->mDescriptors.insert(std::make_pair(it->first, new MemberDescriptor(it->second)));
    }

#ifdef DYNAMIC_TYPES_CHECKING
    newData->mInt32Values = mInt32Values;
    newData->mUInt32Values = mUInt32Values;
    newData->mInt16Values = mInt16Values;
    newData->mUInt16Values = mUInt16Values;
    newData->mInt64Values = mInt64Values;
    newData->mUInt64Values = mUInt64Values;
    newData->mFloat32Values = mFloat32Values;
    newData->mFloat64Values = mFloat64Values;
    newData->mFloat128Values = mFloat128Values;
    newData->mChar8Values = mChar8Values;
    newData->mChar16Values = mChar16Values;
    newData->mByteValues = mByteValues;
    newData->mBoolValues = mBoolValues;
    newData->mStringValues = mStringValues;
    newData->mWStringValues = mWStringValues;

    for (auto it = mComplexValues.begin(); it != mComplexValues.end(); ++it)
    {
        newData->mComplexValues.insert(std::make_pair(it->first, it->second->clone()));
    }

    //newData->mInt32ListValues = mInt32ListValues;
    //newData->mUInt32ListValues = mUInt32ListValues;
    //newData->mInt16ListValues = mInt16ListValues;
    //newData->mUInt16ListValues = mUInt16ListValues;
    //newData->mInt64ListValues = mInt64ListValues;
    //newData->mUInt64ListValues = mUInt64ListValues;
    //newData->mFloat32ListValues = mFloat32ListValues;
    //newData->mFloat64ListValues = mFloat64ListValues;
    //newData->mFloat128ListValues = mFloat128ListValues;
    //newData->mChar8ListValues = mChar8ListValues;
    //newData->mChar16ListValues = mChar16ListValues;
    //newData->mByteListValues = mByteListValues;
    //newData->mBoolListValues = mBoolListValues;
    //newData->mStringListValues = mStringListValues;
    //newData->mWStringListValues = mWStringListValues;
#else
    //TODO: Clone mValues
#endif

    return newData;
}

ResponseCode DynamicData::get_int32_value(int32_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt32Values.find(id);
    if (it != mInt32Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((int32_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int32_value(MemberId id, int32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt32Values.find(id);
    if (it != mInt32Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((int32_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint32_value(uint32_t value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt32Values.find(id);
    if (it != mUInt32Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((uint32_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint32_value(MemberId id, uint32_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt32Values.find(id);
    if (it != mUInt32Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((uint32_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int16_value(int16_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt16Values.find(id);
    if (it != mInt16Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((int16_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int16_value(MemberId id, int16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt16Values.find(id);
    if (it != mInt16Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((int16_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint16_value(uint16_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt16Values.find(id);
    if (it != mUInt16Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((uint16_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint16_value(MemberId id, uint16_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt16Values.find(id);
    if (it != mUInt16Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((uint16_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int64_value(int64_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt64Values.find(id);
    if (it != mInt64Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((int64_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int64_value(MemberId id, int64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt64Values.find(id);
    if (it != mInt64Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((int64_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint64_value(uint64_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt64Values.find(id);
    if (it != mUInt64Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((uint64_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint64_value(MemberId id, uint64_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt64Values.find(id);
    if (it != mUInt64Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((uint64_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float32_value(float& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat32Values.find(id);
    if (it != mFloat32Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((float*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float32_value(MemberId id, float value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat32Values.find(id);
    if (it != mFloat32Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((float*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float64_value(double& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat64Values.find(id);
    if (it != mFloat64Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((double*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float64_value(MemberId id, double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat64Values.find(id);
    if (it != mFloat64Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((double*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float128_value(long double& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat128Values.find(id);
    if (it != mFloat128Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((long double*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float128_value(MemberId id, long double value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat128Values.find(id);
    if (it != mFloat128Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((long double*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char8_value(char& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar8Values.find(id);
    if (it != mChar8Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((char*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char8_value(MemberId id, char value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar8Values.find(id);
    if (it != mChar8Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((char*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char16_value(wchar_t& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar16Values.find(id);
    if (it != mChar16Values.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((wchar_t*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char16_value(MemberId id, wchar_t value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar16Values.find(id);
    if (it != mChar16Values.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((wchar_t*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_byte_value(octet& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mByteValues.find(id);
    if (it != mByteValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((octet*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_byte_value(MemberId id, octet value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mByteValues.find(id);
    if (it != mByteValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((octet*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_bool_value(bool& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mBoolValues.find(id);
    if (it != mBoolValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((bool*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_bool_value(MemberId id, bool value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mBoolValues.find(id);
    if (it != mBoolValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((bool*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_string_value(std::string& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mStringValues.find(id);
    if (it != mStringValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::string*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_string_value(MemberId id, std::string value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mStringValues.find(id);
    if (it != mStringValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::string*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_wstring_value(std::wstring& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mWStringValues.find(id);
    if (it != mWStringValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::wstring*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_wstring_value(MemberId id, const std::wstring value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mWStringValues.find(id);
    if (it != mWStringValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::wstring*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}


ResponseCode DynamicData::get_complex_value(DynamicData* value, MemberId id)
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
/*
ResponseCode DynamicData::get_int32_values(std::vector<int32_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt32ListValues.find(id);
    if (it != mInt32ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<int32_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int32_values(MemberId id, const std::vector<int32_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt32ListValues.find(id);
    if (it != mInt32ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<int32_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint32_values(std::vector<uint32_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt32ListValues.find(id);
    if (it != mUInt32ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<uint32_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint32_values(MemberId id, const std::vector<uint32_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt32ListValues.find(id);
    if (it != mUInt32ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<uint32_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int16_values(std::vector<int16_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt16ListValues.find(id);
    if (it != mInt16ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<int16_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int16_values(MemberId id, const std::vector<int16_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt16ListValues.find(id);
    if (it != mInt16ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<int16_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint16_values(std::vector<uint16_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt16ListValues.find(id);
    if (it != mUInt16ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<uint16_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint16_values(MemberId id, const std::vector<uint16_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt16ListValues.find(id);
    if (it != mUInt16ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<uint16_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_int64_values(std::vector<int64_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt64ListValues.find(id);
    if (it != mInt64ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<int64_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_int64_values(MemberId id, const std::vector<int64_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mInt64ListValues.find(id);
    if (it != mInt64ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<int64_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_uint64_values(std::vector<uint64_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt64ListValues.find(id);
    if (it != mUInt64ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<uint64_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_uint64_values(MemberId id, const std::vector<uint64_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mUInt64ListValues.find(id);
    if (it != mUInt64ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<uint64_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float32_values(std::vector<float>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat32ListValues.find(id);
    if (it != mFloat32ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<float>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float32_values(MemberId id, const std::vector<float>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat32ListValues.find(id);
    if (it != mFloat32ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<float>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float64_values(std::vector<double>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat64ListValues.find(id);
    if (it != mFloat64ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<double>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float64_values(MemberId id, const std::vector<double>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat64ListValues.find(id);
    if (it != mFloat64ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<double>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_float128_values(std::vector<long double>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat128ListValues.find(id);
    if (it != mFloat128ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<long double>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_float128_values(MemberId id, const std::vector<long double>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mFloat128ListValues.find(id);
    if (it != mFloat128ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<long double>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char8_values(std::vector<char>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar8ListValues.find(id);
    if (it != mChar8ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<char>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char8_values(MemberId id, const std::vector<char>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar8ListValues.find(id);
    if (it != mChar8ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<char>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_char16_values(std::vector<wchar_t>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar16ListValues.find(id);
    if (it != mChar16ListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<wchar_t>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_char16_values(MemberId id, const std::vector<wchar_t>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mChar16ListValues.find(id);
    if (it != mChar16ListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<wchar_t>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_byte_values(std::vector<octet>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mByteListValues.find(id);
    if (it != mByteListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<octet>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_byte_values(MemberId id, const std::vector<octet>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mByteListValues.find(id);
    if (it != mByteListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<octet>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_boolean_values(std::vector<bool>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mBoolListValues.find(id);
    if (it != mBoolListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<bool>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_boolean_values(MemberId id, const std::vector<bool>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mBoolListValues.find(id);
    if (it != mBoolListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<bool>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_string_values(std::vector<std::string>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mStringListValues.find(id);
    if (it != mStringListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<std::string>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_string_values(MemberId id, const std::vector<std::string>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mStringListValues.find(id);
    if (it != mStringListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<std::string>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::get_wstring_values(std::vector<std::wstring>& value, MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mWStringListValues.find(id);
    if (it != mWStringListValues.end())
    {
        value = it->second;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        value = *((std::vector<std::wstring>*)it->second);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}

ResponseCode DynamicData::set_wstring_values(MemberId id, const std::vector<std::wstring>& value)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto it = mWStringListValues.find(id);
    if (it != mWStringListValues.end())
    {
        it->second = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#else
    auto it = mValues.find(id);
    if (it != mValues.end())
    {
        *((std::vector<std::wstring>*)it->second) = value;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
#endif
}
*/

} // namespace types
} // namespace fastrtps
} // namespace eprosima
