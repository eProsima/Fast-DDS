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
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastdds/dds/log/Log.hpp>

#include <locale>
#include <codecvt>
#include <algorithm>

using namespace eprosima::fastrtps::types;

template <typename Map>
bool map_compare(
        Map const& left,
        Map const& right)
{
    auto pred = [](decltype(*left.begin()) a, decltype(a) b)
            {
                return a.first == b.first && a.second == b.second;
            };

    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), pred);
}

template <>
bool map_compare(
        const std::map<MemberId, DynamicData*>& left,
        const std::map<MemberId, DynamicData*>& right)
{
    auto pred = [](decltype(*left.begin()) a, decltype(a) b)
            {
                return a.first == b.first && a.second->equals(b.second);
            };

    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), pred);
}

DynamicData::DynamicData(
        DynamicType_ptr pType)
    : type_(pType)
{
    create_members(type_);
}

DynamicData::DynamicData(
        const DynamicData* pData)
    : type_(pData->get_type())
#ifdef DYNAMIC_TYPES_CHECKING
    , int32_value_(pData->int32_value_)
    , uint32_value_(pData->uint32_value_)
    , int16_value_(pData->int16_value_)
    , uint16_value_(pData->uint16_value_)
    , int64_value_(pData->int64_value_)
    , uint64_value_(pData->uint64_value_)
    , float32_value_(pData->float32_value_)
    , float64_value_(pData->float64_value_)
    , float128_value_(pData->float128_value_)
    , char8_value_(pData->char8_value_)
    , char16_value_(pData->char16_value_)
    , byte_value_(pData->byte_value_)
    , bool_value_(pData->bool_value_)
    , string_value_(pData->string_value_)
    , wstring_value_(pData->wstring_value_)
#endif // ifdef DYNAMIC_TYPES_CHECKING
    , key_element_(pData->key_element_)
    , default_array_value_(pData->default_array_value_)
    , union_id_(pData->union_id_)
{
    create_members(pData);
}

DynamicData::~DynamicData()
{
    clean();
}

DynamicType_ptr DynamicData::get_type() const
{
    return type_;
}

void DynamicData::create_members(
        const DynamicData* pData)
{
    assert(type_ == pData->type_);

#ifdef DYNAMIC_TYPES_CHECKING
    for (auto it = pData->complex_values_.begin(); it != pData->complex_values_.end(); ++it)
    {
        complex_values_.insert(std::make_pair(it->first, DynamicDataFactory::get_instance()->create_copy(it->second)));
    }
#else
    if (type_->is_complex_kind())
    {
        for (auto it = pData->values_.begin(); it != pData->values_.end(); ++it)
        {
            values_.insert(std::make_pair(it->first,
                    DynamicDataFactory::get_instance()->create_copy((DynamicData*)it->second)));
        }
    }
    else if (type_->get_member_count() > 0)
    {
        for(auto pm : type_->get_all_members())
        {
            assert(pm);
            values_[pm->get_id()] = pData->clone_value(pm->get_id(), pm->get_kind());
        }
    }
    else
    {
        values_.insert(std::make_pair(MEMBER_ID_INVALID, pData->clone_value(MEMBER_ID_INVALID, pData->get_kind())));
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

void DynamicData::create_members(
        DynamicType_ptr pType)
{
    assert(pType);

    if (pType->is_complex_kind())
    {
        // Bitmasks and enums register their members but only manages one value.
        if (pType->get_kind() == TypeKind::TK_BITMASK || pType->get_kind() == TypeKind::TK_ENUM)
        {
            add_value(pType->get_kind(), MEMBER_ID_INVALID);
        }
        else
        {
            for (auto pm : pType->get_all_members())
            {
                assert(pm);

                DynamicData* data = DynamicDataFactory::get_instance()->create_data(pm->get_type());
                if (pm->get_kind() != TypeKind::TK_BITSET &&
                        pm->get_kind() != TypeKind::TK_STRUCTURE &&
                        pm->get_kind() != TypeKind::TK_UNION &&
                        pm->get_kind() != TypeKind::TK_SEQUENCE &&
                        pm->get_kind() != TypeKind::TK_ARRAY &&
                        pm->get_kind() != TypeKind::TK_MAP)
                {
                    std::string def_value = pm->annotation_get_default();
                    if (!def_value.empty())
                    {
                        data->set_value(def_value);
                    }
                }
#ifdef DYNAMIC_TYPES_CHECKING
                complex_values_.insert(std::make_pair(pm->get_id(), data));
#else
                values_.insert(std::make_pair(pm->get_id(), data));
#endif // ifdef DYNAMIC_TYPES_CHECKING

                // Set the default value for unions.
                if (pType->get_kind() == TypeKind::TK_UNION &&
                        pm->is_default_union_value())
                {
                    set_union_id(pm->get_id());
                }
            }
        }

        // If there isn't a default value... set the first element of the union
        if (pType->get_kind() == TypeKind::TK_UNION &&
                get_union_id() == MEMBER_ID_INVALID &&
                pType->get_member_count())
        {
            set_union_id(get_member_id_at_index(0));
        }
    }
    else
    {
        // Resolve alias type, avoid reference counting
        const DynamicType* type = pType.get();
        while ( type->get_kind() == TypeKind::TK_ALIAS )
        {
            type = type->get_base_type().get();
        }

        add_value(type->get_kind(), MEMBER_ID_INVALID);
    }
}

ReturnCode_t DynamicData::get_descriptor(
        MemberDescriptor& value,
        MemberId id)
{
    assert(type_);
    return type_->get_member(value, id);
}

bool DynamicData::equals(
        const DynamicData* other) const
{
    if (other != nullptr)
    {
        if (other == this)
        {
            return true;
        }
        else if (type_ == other->type_ || type_->equals(*other->type_.get()))
        {
            // Optimization for unions, only check the selected element.
            if (get_kind() == TypeKind::TK_UNION)
            {
                if (union_id_ != other->union_id_)
                {
                    return false;
                }
                else if (union_id_ != MEMBER_ID_INVALID)
                {
#ifdef DYNAMIC_TYPES_CHECKING
                    auto it = complex_values_.find(union_id_);
                    auto otherIt = other->complex_values_.find(union_id_);
#else
                    auto it = values_.find(union_id_);
                    auto otherIt = other->values_.find(union_id_);
#endif // ifdef DYNAMIC_TYPES_CHECKING
                    if (!((DynamicData*)it->second)->equals((DynamicData*)otherIt->second))
                    {
                        return false;
                    }
                }
            }
            else
            {
#ifdef DYNAMIC_TYPES_CHECKING
                bool bFail = false;
                bFail = int32_value_ != other->int32_value_;
                bFail = bFail || uint32_value_ != other->uint32_value_;
                bFail = bFail || int16_value_ != other->int16_value_;
                bFail = bFail || uint16_value_ != other->uint16_value_;
                bFail = bFail || int64_value_ != other->int64_value_;
                bFail = bFail || uint64_value_ != other->uint64_value_;
                bFail = bFail || float32_value_ != other->float32_value_;
                bFail = bFail || float64_value_ != other->float64_value_;
                bFail = bFail || float128_value_ != other->float128_value_;
                bFail = bFail || char8_value_ != other->char8_value_;
                bFail = bFail || char16_value_ != other->char16_value_;
                bFail = bFail || byte_value_ != other->byte_value_;
                bFail = bFail || bool_value_ != other->bool_value_;
                bFail = bFail || string_value_ != other->string_value_;
                bFail = bFail || wstring_value_ != other->wstring_value_;
                bFail = bFail || !map_compare(complex_values_, other->complex_values_);
                /*if (int32_value_ != other->int32_value_ || uint32_value_ != other->uint32_value_ ||
                    int16_value_ != other->int16_value_ || uint16_value_ != other->uint16_value_ ||
                    int64_value_ != other->int64_value_ || uint64_value_ != other->uint64_value_ ||
                    float32_value_ != other->float32_value_ || float64_value_ != other->float64_value_ ||
                    float128_value_ != other->float128_value_ || char8_value_ != other->char8_value_ ||
                    char16_value_ != other->char16_value_ || byte_value_ != other->byte_value_ ||
                    bool_value_ != other->bool_value_ || string_value_ != other->string_value_ ||
                    wstring_value_ != other->wstring_value_ ||
                    !map_compare(complex_values_, other->complex_values_))
                 */
                if (bFail)
                {
                    return false;
                }
#else
                if (get_kind() == TypeKind::TK_ENUM)
                {
                    if (!compare_values(TypeKind::TK_UINT32, values_.begin()->second, other->values_.begin()->second))
                    {
                        return false;
                    }
                }
                else if (get_kind() == TypeKind::TK_BITMASK)
                {
                    TypeKind bitmask_kind = TypeKind::TK_BYTE;
                    size_t type_size = type_->get_size();
                    switch (type_size)
                    {
                        case 1: bitmask_kind = TypeKind::TK_BYTE; break;
                        case 2: bitmask_kind = TypeKind::TK_UINT16; break;
                        case 4: bitmask_kind = TypeKind::TK_UINT32; break;
                        case 8: bitmask_kind = TypeKind::TK_UINT64; break;
                    }

                    assert(values_.size() && other->values_.size());

                    if (!compare_values(bitmask_kind, values_.begin()->second, other->values_.begin()->second))
                    {
                        return false;
                    }
                }
                else if (type_->is_complex_kind())
                {
                    // array, map, sequence, structure, bitset, anotation
                    return values_.size() == other->values_.size() &&
                           std::equal(
                                values_.begin(),
                                values_.end(),
                                other->values_.begin(),
                                [](const decltype(values_)::value_type& l, const decltype(values_)::value_type& r)
                                {
                                    DynamicData* left = (DynamicData*)l.second;
                                    DynamicData* right = (DynamicData*)r.second;
                                    bool res = left->equals(right);
                                    return res;
                                    // TODO: undo once finished debugging
                                    // return ((DynamicData*)l.second)->equals((DynamicData*)r.second);
                                });
                }
                else
                {
                    // primitives
                    if (!compare_values(get_kind(), values_.begin()->second, other->values_.begin()->second))
                    {
                        return false;
                    }
                }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            }
            return true;
        }
    }
    return false;
}

MemberId DynamicData::get_member_id_by_name(
        const std::string& name) const
{
    assert(type_);
    return type_->get_member_id_by_name(name);
}

MemberId DynamicData::get_member_id_at_index(
        uint32_t index) const
{
    assert(type_);
    return type_->get_member_id_at_index(index);
}

TypeKind DynamicData::get_kind() const
{
    assert(type_);
    return type_->get_kind();
}

uint32_t DynamicData::get_item_count() const
{
    if (get_kind() == TypeKind::TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(complex_values_.size() / 2);
#else
        return static_cast<uint32_t>(values_.size() / 2);
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else if (get_kind() == TypeKind::TK_ARRAY)
    {
        return type_->get_total_bounds();
    }
    else
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(complex_values_.size());
#else
        return static_cast<uint32_t>(values_.size());
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
}

std::string DynamicData::get_name()
{
    return type_->get_name();
}

void DynamicData::add_value(
        TypeKind kind,
        MemberId id)
{
    switch (kind)
    {
        default:
            break;
        case TypeKind::TK_INT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new int32_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_UINT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new uint32_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_INT16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new int16_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_UINT16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new uint16_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_INT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new int64_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_UINT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new uint64_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new float()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new double()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new long double()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_CHAR8:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new char()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_CHAR16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new wchar_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_BOOLEAN:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new bool()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_BYTE:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new octet()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_STRING8:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new std::string()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_STRING16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new std::wstring()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_ENUM:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new uint32_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TypeKind::TK_BITMASK:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.insert(std::make_pair(id, new uint64_t()));
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
    }
    set_default_value(id);
}

void DynamicData::clean()
{
    if (default_array_value_ != nullptr)
    {
        DynamicDataFactory::get_instance()->delete_data(default_array_value_);
        default_array_value_ = nullptr;
    }

    clean_members();

    type_.reset();
}

ReturnCode_t DynamicData::clear_all_values()
{
    if (type_->is_complex_kind())
    {
        if (get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_MAP || get_kind() == TypeKind::TK_ARRAY)
        {
            return clear_data();
        }
        else
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto& e : complex_values_)
            {
                e.second->clear_all_values();
            }
#else
            for (auto& e : values_)
            {
                ((DynamicData*)e.second)->clear_all_values();
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
    }
    else
    {
        set_default_value(MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_OK;
}

void DynamicData::clean_members()
{
#ifdef DYNAMIC_TYPES_CHECKING
    for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
    {
        DynamicDataFactory::get_instance()->delete_data(it->second);
    }
    complex_values_.clear();
#else
    if (has_children())
    {
        for (auto it = values_.begin(); it != values_.end(); ++it)
        {
            DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
        }
    }
    else
    {
        switch (get_kind())
        {
            default:
                break;
            case TypeKind::TK_INT32:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((int32_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_UINT32:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((uint32_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_INT16:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((int16_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_UINT16:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((uint16_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_INT64:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((int64_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_UINT64:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((uint64_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_FLOAT32:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((float*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_FLOAT64:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((double*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_FLOAT128:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((long double*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_CHAR8:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((char*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_CHAR16:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((wchar_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_BOOLEAN:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((bool*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_BYTE:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((octet*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_STRING8:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((std::string*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_STRING16:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((std::wstring*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_ENUM:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((uint32_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_BITMASK:
            {
#ifndef DYNAMIC_TYPES_CHECKING
                auto it = values_.begin();
                delete ((uint64_t*)it->second);
#endif // ifndef DYNAMIC_TYPES_CHECKING
                break;
            }
            case TypeKind::TK_UNION:
            case TypeKind::TK_STRUCTURE:
            case TypeKind::TK_ARRAY:
            case TypeKind::TK_SEQUENCE:
            case TypeKind::TK_MAP:
            case TypeKind::TK_ALIAS:
            case TypeKind::TK_BITSET:
            {
                break;
            }
        }
    }
    values_.clear();
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::clear_nonkey_values()
{
    if (type_->is_complex_kind())
    {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto& e : complex_values_)
            {
                e.second->clear_nonkey_values();
            }
#else
            for (auto& e : values_)
            {
                ((DynamicData*)e.second)->clear_nonkey_values();
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else
    {
        if (!key_element_)
        {
            set_default_value(MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DynamicData::clear_value(
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    auto itValue = complex_values_.find(id);
    if (itValue != complex_values_.end())
    {
        itValue->second->clear_all_values();
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        ((DynamicData*)itValue->second)->clear_all_values();
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    else
    {
        set_default_value(id);
    }
    return ReturnCode_t::RETCODE_OK;
}

void* DynamicData::clone_value(
        MemberId id,
        TypeKind kind) const
{
    switch (kind)
    {
        default:
            break;
        case TypeKind::TK_INT32:
        {
            int32_t* newInt32 = new int32_t();
            get_int32_value(*newInt32, id);
            return newInt32;
        }
        break;
        case TypeKind::TK_UINT32:
        {
            uint32_t* newUInt32 = new uint32_t();
            get_uint32_value(*newUInt32, id);
            return newUInt32;
        }
        break;
        case TypeKind::TK_INT16:
        {
            int16_t* newInt16 = new int16_t();
            get_int16_value(*newInt16, id);
            return newInt16;
        }
        break;
        case TypeKind::TK_UINT16:
        {
            uint16_t* newUInt16 = new uint16_t();
            get_uint16_value(*newUInt16, id);
            return newUInt16;
        }
        break;
        case TypeKind::TK_INT64:
        {
            int64_t* newInt64 = new int64_t();
            get_int64_value(*newInt64, id);
            return newInt64;
        }
        break;
        case TypeKind::TK_UINT64:
        {
            uint64_t* newUInt64 = new uint64_t();
            get_uint64_value(*newUInt64, id);
            return newUInt64;
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
            float* newFloat32 = new float();
            get_float32_value(*newFloat32, id);
            return newFloat32;
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
            double* newFloat64 = new double();
            get_float64_value(*newFloat64, id);
            return newFloat64;
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
            long double* newFloat128 = new long double();
            get_float128_value(*newFloat128, id);
            return newFloat128;
        }
        break;
        case TypeKind::TK_CHAR8:
        {
            char* newChar8 = new char();
            get_char8_value(*newChar8, id);
            return newChar8;
        }
        break;
        case TypeKind::TK_CHAR16:
        {
            wchar_t* newChar16 = new wchar_t();
            get_char16_value(*newChar16, id);
            return newChar16;
        }
        break;
        case TypeKind::TK_BOOLEAN:
        {
            bool* newBool = new bool();
            get_bool_value(*newBool, id);
            return newBool;
        }
        break;
        case TypeKind::TK_BYTE:
        {
            octet* newByte = new octet();
            get_byte_value(*newByte, id);
            return newByte;
        }
        break;
        case TypeKind::TK_STRING8:
        {
            std::string* newString = new std::string();
            get_string_value(*newString, id);
            return newString;
        }
        break;
        case TypeKind::TK_STRING16:
        {
            std::wstring* newString = new std::wstring();
            get_wstring_value(*newString, id);
            return newString;
        }
        break;
        case TypeKind::TK_ENUM:
        {
            uint32_t* newUInt32 = new uint32_t();
            get_enum_value(*newUInt32, id);
            return newUInt32;
        }
        break;
        case TypeKind::TK_BITMASK:
        {
            uint64_t* newBitset = new uint64_t();
            get_uint64_value(*newBitset, id);
            return newBitset;
        }
    }
    return nullptr;
}

bool DynamicData::compare_values(
        TypeKind kind,
        void* left,
        void* right) const
{
    switch (kind)
    {
        default:
            break;
        case TypeKind::TK_INT32:      {
            return *((int32_t*)left) == *((int32_t*)right);
        }
        case TypeKind::TK_UINT32:     {
            return *((uint32_t*)left) == *((uint32_t*)right);
        }
        case TypeKind::TK_INT16:      {
            return *((int16_t*)left) == *((int16_t*)right);
        }
        case TypeKind::TK_UINT16:     {
            return *((uint16_t*)left) == *((uint16_t*)right);
        }
        case TypeKind::TK_INT64:      {
            return *((int64_t*)left) == *((int64_t*)right);
        }
        case TypeKind::TK_UINT64:     {
            return *((uint64_t*)left) == *((uint64_t*)right);
        }
        case TypeKind::TK_FLOAT32:    {
            return *((float*)left) == *((float*)right);
        }
        case TypeKind::TK_FLOAT64:    {
            return *((double*)left) == *((double*)right);
        }
        case TypeKind::TK_FLOAT128:   {
            return *((long double*)left) == *((long double*)right);
        }
        case TypeKind::TK_CHAR8:      {
            return *((char*)left) == *((char*)right);
        }
        case TypeKind::TK_CHAR16:     {
            return *((wchar_t*)left) == *((wchar_t*)right);
        }
        case TypeKind::TK_BOOLEAN:    {
            return *((bool*)left) == *((bool*)right);
        }
        case TypeKind::TK_BYTE:       {
            return *((octet*)left) == *((octet*)right);
        }
        case TypeKind::TK_STRING8:    {
            return *((std::string*)left) == *((std::string*)right);
        }
        case TypeKind::TK_STRING16:   {
            return *((std::wstring*)left) == *((std::wstring*)right);
        }
        case TypeKind::TK_ENUM:       {
            return *((uint32_t*)left) == *((uint32_t*)right);
        }
    }
    return false;
}

void DynamicData::get_value(
        std::string& sOutValue,
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    switch (type_->get_kind())
    {
        default:
            break;
        case TypeKind::TK_INT32:
        {
            int32_t value(0);
            get_int32_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_UINT32:
        {
            uint32_t value(0);
            get_uint32_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_INT16:
        {
            int16_t value(0);
            get_int16_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_UINT16:
        {
            uint16_t value(0);
            get_uint16_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_INT64:
        {
            int64_t value(0);
            get_int64_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_UINT64:
        {
            uint64_t value(0);
            get_uint64_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
            float value(0.0f);
            get_float32_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
            double value(0.0f);
            get_float64_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
            long double value(0.0f);
            get_float128_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_CHAR8:
        {
            char value = 0;
            get_char8_value(value, id);
            sOutValue = value;
        }
        break;
        case TypeKind::TK_CHAR16:
        {
            wchar_t value(0);
            get_char16_value(value, id);
            std::wstring temp = L"";
            temp += value;
            sOutValue = wstring_to_bytes(temp);
        }
        break;
        case TypeKind::TK_BOOLEAN:
        {
            bool value(false);
            get_bool_value(value, id);
            sOutValue = std::to_string(value ? 1 : 0);
        }
        break;
        case TypeKind::TK_BYTE:
        {
            uint8_t value(0);
            get_byte_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_STRING8:
        {
            sOutValue = get_string_value(id);
        }
        break;
        case TypeKind::TK_STRING16:
        {
            std::wstring value;
            get_wstring_value(value, id);
            sOutValue = wstring_to_bytes(value);
        }
        break;
        case TypeKind::TK_ENUM:
        {
            uint32_t value;
            get_enum_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_BITMASK:
        {
            uint64_t value(0);
            get_uint64_value(value, id);
            sOutValue = std::to_string(value);
        }
        break;
        case TypeKind::TK_ARRAY:
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_BITSET:
        case TypeKind::TK_MAP:
        {
            // THESE TYPES DON'T MANAGE VALUES
        }
        break;
    }
}

void DynamicData::set_value(
        const std::string& sValue,
        MemberId id /*= MEMBER_ID_INVALID*/)
{
    switch (type_->get_kind())
    {
        default:
            break;
        case TypeKind::TK_INT32:
        {
            int32_t value(0);
            try
            {
                value = stoi(sValue);
            }
            catch (...)
            {
            }
            set_int32_value(value, id);
        }
        break;
        case TypeKind::TK_UINT32:
        {
            uint32_t value(0);
            try
            {
                value = stoul(sValue);
            }
            catch (...)
            {
            }
            set_uint32_value(value, id);
        }
        break;
        case TypeKind::TK_INT16:
        {
            int16_t value(0);
            try
            {
                value = static_cast<int16_t>(stoi(sValue));
            }
            catch (...)
            {
            }
            set_int16_value(value, id);
        }
        break;
        case TypeKind::TK_UINT16:
        {
            uint16_t value(0);
            try
            {
                value = static_cast<uint16_t>(stoul(sValue));
            }
            catch (...)
            {
            }
            set_uint16_value(value, id);
        }
        break;
        case TypeKind::TK_INT64:
        {
            int64_t value(0);
            try
            {
                value = stoll(sValue);
            }
            catch (...)
            {
            }
            set_int64_value(value, id);
        }
        break;
        case TypeKind::TK_UINT64:
        {
            uint64_t value(0);
            try
            {
                value = stoul(sValue);
            }
            catch (...)
            {
            }
            set_uint64_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
            float value(0.0f);
            try
            {
                value = stof(sValue);
            }
            catch (...)
            {
            }
            set_float32_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
            double value(0.0f);
            try
            {
                value = stod(sValue);
            }
            catch (...)
            {
            }
            set_float64_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
            long double value(0.0f);
            try
            {
                value = stold(sValue);
            }
            catch (...)
            {
            }
            set_float128_value(value, id);
        }
        break;
        case TypeKind::TK_CHAR8:
        {
            if (sValue.length() >= 1)
            {
                set_char8_value(sValue[0], id);
            }
        }
        break;
        case TypeKind::TK_CHAR16:
        {
            wchar_t value(0);
            try
            {
                std::wstring temp = std::wstring(sValue.begin(), sValue.end());
                value = temp[0];
            }
            catch (...)
            {
            }

            set_char16_value(value, id);
        }
        break;
        case TypeKind::TK_BOOLEAN:
        {
            int value(0);
            try
            {
                value = stoi(sValue);
            }
            catch (...)
            {
            }
            set_bool_value(value == 1 ? true : false, id);
        }
        break;
        case TypeKind::TK_BYTE:
        {
            if (sValue.length() >= 1)
            {
                uint8_t value(0);
                try
                {
                    value = static_cast<uint8_t>(stoul(sValue));
                }
                catch (...)
                {
                }
                set_byte_value(value, id);
            }
        }
        break;
        case TypeKind::TK_STRING8:
        {
            set_string_value(sValue, id);
        }
        break;
        case TypeKind::TK_STRING16:
        {
            set_wstring_value(std::wstring(sValue.begin(), sValue.end()), id);
        }
        break;
        case TypeKind::TK_ENUM:
        {
            uint32_t value(0);
            try
            {
                value = stoul(sValue);
            }
            catch (...)
            {
            }
            set_enum_value(value, id);
        }
        break;
        case TypeKind::TK_BITMASK:
        {
            uint64_t value(0);
            try
            {
                value = stoul(sValue);
            }
            catch (...)
            {
            }
            set_uint64_value(value, id);
        }
        break;
        case TypeKind::TK_ARRAY:
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_BITSET:
        case TypeKind::TK_MAP:
        {
            // THESE TYPES DON'T MANAGE VALUES
        }
        break;
    }
}

void DynamicData::set_default_value(
        MemberId id)
{
    assert(type_);

    std::string defaultValue;
    const DynamicTypeMember* pM;
    bool found;

    std::tie(pM, found) = type_->get_member(id);

    if(!found)
    {
        return;
    }

    defaultValue = pM->get_default_value();

    switch (type_->get_kind())
    {
        default:
            break;
        case TypeKind::TK_INT32:
        {
            int32_t value(0);
            try
            {
                value = stoi(defaultValue);
            }
            catch (...)
            {
            }
            set_int32_value(value, id);
        }
        break;
        case TypeKind::TK_UINT32:
        {
            uint32_t value(0);
            try
            {
                value = stoul(defaultValue);
            }
            catch (...)
            {
            }
            set_uint32_value(value, id);
        }
        break;
        case TypeKind::TK_INT16:
        {
            int16_t value(0);
            try
            {
                value = static_cast<int16_t>(stoi(defaultValue));
            }
            catch (...)
            {
            }
            set_int16_value(value, id);
        }
        break;
        case TypeKind::TK_UINT16:
        {
            uint16_t value(0);
            try
            {
                value = static_cast<uint16_t>(stoul(defaultValue));
            }
            catch (...)
            {
            }
            set_uint16_value(value, id);
        }
        break;
        case TypeKind::TK_INT64:
        {
            int64_t value(0);
            try
            {
                value = stoll(defaultValue);
            }
            catch (...)
            {
            }
            set_int64_value(value, id);
        }
        break;
        case TypeKind::TK_UINT64:
        {
            uint64_t value(0);
            try
            {
                value = stoul(defaultValue);
            }
            catch (...)
            {
            }
            set_uint64_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT32:
        {
            float value(0.0f);
            try
            {
                value = stof(defaultValue);
            }
            catch (...)
            {
            }
            set_float32_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT64:
        {
            double value(0.0f);
            try
            {
                value = stod(defaultValue);
            }
            catch (...)
            {
            }
            set_float64_value(value, id);
        }
        break;
        case TypeKind::TK_FLOAT128:
        {
            long double value(0.0f);
            try
            {
                value = stold(defaultValue);
            }
            catch (...)
            {
            }
            set_float128_value(value, id);
        }
        break;
        case TypeKind::TK_CHAR8:
        {
            if (defaultValue.length() >= 1)
            {
                set_char8_value(defaultValue[0], id);
            }
        }
        break;
        case TypeKind::TK_CHAR16:
        {
            wchar_t value(0);
            try
            {
                std::wstring temp = std::wstring(defaultValue.begin(), defaultValue.end());
                value = temp[0];
            }
            catch (...)
            {
            }

            set_char16_value(value, id);
        }
        break;
        case TypeKind::TK_BOOLEAN:
        {
            int value(0);
            try
            {
                value = stoi(defaultValue);
            }
            catch (...)
            {
            }
            set_bool_value(value == 1 ? true : false, id);
        }
        break;
        case TypeKind::TK_BYTE:
        {
            if (defaultValue.length() >= 1)
            {
                uint8_t value(0);
                try
                {
                    value = static_cast<uint8_t>(stoul(defaultValue));
                }
                catch (...)
                {
                }
                set_byte_value(value, id);
            }
        }
        break;
        case TypeKind::TK_STRING8:
        {
            set_string_value(defaultValue, id);
        }
        break;
        case TypeKind::TK_STRING16:
        {
            set_wstring_value(std::wstring(defaultValue.begin(), defaultValue.end()), id);
        }
        break;
        case TypeKind::TK_ENUM:
        {
            uint32_t value(0);
            try
            {
                value = stoul(defaultValue);
            }
            catch (...)
            {
            }
            set_enum_value(value, id);
        }
        break;
        case TypeKind::TK_BITMASK:
        {
            uint64_t value(0);
            try
            {
                value = stoul(defaultValue);
            }
            catch (...)
            {
            }
            set_uint64_value(value, id);
        }
        break;
        case TypeKind::TK_ARRAY:
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_BITSET:
        case TypeKind::TK_MAP:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto itValue = complex_values_.find(id);
            if (itValue != complex_values_.end())
            {
                if (!itValue->second->key_element_)
                {
                    itValue->second->set_default_value(MEMBER_ID_INVALID);
                }
            }
#else
            auto itValue = values_.find(id);
            if (itValue != values_.end())
            {
                if (!((DynamicData*)itValue->second)->key_element_)
                {
                    ((DynamicData*)itValue->second)->set_default_value(MEMBER_ID_INVALID);
                }
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        break;
    }
}

DynamicData* DynamicData::loan_value(
        MemberId id)
{
    if (id != MEMBER_ID_INVALID)
    {
        if (std::find(loaned_values_.begin(), loaned_values_.end(), id) == loaned_values_.end())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = complex_values_.find(id);
            if (it != complex_values_.end())
            {
                if (get_kind() == TypeKind::TK_MAP && it->second->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    if (get_kind() == TypeKind::TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                    loaned_values_.push_back(id);
                    return it->second;
                }
            }
            else if (get_kind() == TypeKind::TK_ARRAY)
            {
                if (insert_array_data(id) == ReturnCode_t::RETCODE_OK)
                {
                    loaned_values_.push_back(id);
                    return complex_values_.at(id);
                }
            }
#else
            auto it = values_.find(id);
            if (it != values_.end())
            {
                if (get_kind() == TypeKind::TK_MAP && ((DynamicData*)it->second)->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    if (get_kind() == TypeKind::TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                    loaned_values_.push_back(id);
                    return (DynamicData*)it->second;
                }
            }
            else if (get_kind() == TypeKind::TK_ARRAY)
            {
                if (insert_array_data(id) == ReturnCode_t::RETCODE_OK)
                {
                    loaned_values_.push_back(id);
                    return (DynamicData*)values_.at(id);
                }
            }

#endif // ifdef DYNAMIC_TYPES_CHECKING
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. MemberId not found.");
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. The value has been loaned previously.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Invalid MemberId.");
    }
    return nullptr;
}

ReturnCode_t DynamicData::return_loaned_value(
        const DynamicData* value)
{
    for (auto loanIt = loaned_values_.begin(); loanIt != loaned_values_.end(); ++loanIt)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(*loanIt);
        if (it != complex_values_.end() && it->second == value)
        {
            loaned_values_.erase(loanIt);
            return ReturnCode_t::RETCODE_OK;
        }
#else
        auto it = values_.find(*loanIt);
        if (it != values_.end() && it->second == value)
        {
            loaned_values_.erase(loanIt);
            return ReturnCode_t::RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error returning loaned Value. The value hasn't been loaned.");
    return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t DynamicData::get_int32_value(
        int32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT32 && id == MEMBER_ID_INVALID)
    {
        value = int32_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_int32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_int32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT32 && id == MEMBER_ID_INVALID)
        {
            value = *((int32_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_int32_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int32_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_int32_value(
        int32_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT32 && id == MEMBER_ID_INVALID)
    {
        int32_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_get_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                int32_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_int32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            int32_t default_value;
            auto default_res = default_array_value_->get_int32_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_int32_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT32 && id == MEMBER_ID_INVALID)
        {
            *((int32_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TypeKind::TK_BITSET)
            {
                const DynamicTypeMember* pM = nullptr;
                bool found;

                std::tie(pM, found) = type_->get_member(id);
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }

                uint16_t bit_bound = pM->annotation_get_bit_bound();
                int32_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }

            ReturnCode_t result = ((DynamicData*)it->second)->set_int32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int32_t default_value;
        auto default_res = default_array_value_->get_int32_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_int32_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_uint32_value(
        uint32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        value = uint32_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_uint32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_uint32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            value = *((uint32_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_uint32_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint32_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_uint32_value(
        uint32_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        uint32_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                uint32_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_uint32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            uint32_t default_value;
            auto default_res = default_array_value_->get_uint32_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_uint32_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            *((uint32_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                uint32_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_uint32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint32_t default_value;
        auto default_res = default_array_value_->get_uint32_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_uint32_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_int16_value(
        int16_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT16 && id == MEMBER_ID_INVALID)
    {
        value = int16_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_int16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_int16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT16 && id == MEMBER_ID_INVALID)
        {
            value = *((int16_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_int16_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int16_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_int16_value(
        int16_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT16 && id == MEMBER_ID_INVALID)
    {
        int16_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                int16_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_int16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            int16_t default_value;
            auto default_res = default_array_value_->get_int16_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_int16_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT16 && id == MEMBER_ID_INVALID)
        {
            *((int16_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                int16_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_int16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int16_t default_value;
        auto default_res = default_array_value_->get_int16_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_int16_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_uint16_value(
        uint16_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        value = uint16_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_uint16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_uint16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            value = *((uint16_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_uint16_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint16_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_uint16_value(
        uint16_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        uint16_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                uint16_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_uint16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            uint16_t default_value;
            auto default_res = default_array_value_->get_uint16_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_uint16_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            *((uint16_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                uint16_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_uint16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint16_t default_value;
        auto default_res = default_array_value_->get_uint16_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_uint16_value(value, id);
        }
        return insertResult;
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_int64_value(
        int64_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT64 && id == MEMBER_ID_INVALID)
    {
        value = int64_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_int64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_int64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT64 && id == MEMBER_ID_INVALID)
        {
            value = *((int64_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_int64_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int64_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_int64_value(
        int64_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_INT64 && id == MEMBER_ID_INVALID)
    {
        int64_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                int64_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_int64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            int64_t default_value;
            auto default_res = default_array_value_->get_int64_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_int64_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_INT64 && id == MEMBER_ID_INVALID)
        {
            *((int64_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                int64_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_int64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int64_t default_value;
        auto default_res = default_array_value_->get_int64_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_int64_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_uint64_value(
        uint64_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if ((get_kind() == TypeKind::TK_UINT64 || get_kind() == TypeKind::TK_BITMASK) && id == MEMBER_ID_INVALID)
    {
        value = uint64_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_uint64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_uint64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if ((get_kind() == TypeKind::TK_UINT64 || get_kind() == TypeKind::TK_BITMASK) && id == MEMBER_ID_INVALID)
        {
            value = *((uint64_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_uint64_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint64_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_uint64_value(
        uint64_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if ((get_kind() == TypeKind::TK_UINT64 || get_kind() == TypeKind::TK_BITMASK) && id == MEMBER_ID_INVALID)
    {
        uint64_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                uint64_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_uint64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            uint64_t default_value;
            auto default_res = default_array_value_->get_uint64_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_uint64_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if ((get_kind() == TypeKind::TK_UINT64 || get_kind() == TypeKind::TK_BITMASK) && id == MEMBER_ID_INVALID)
        {
            *((uint64_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                uint64_t mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_uint64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint64_t default_value;
        auto default_res = default_array_value_->get_uint64_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_uint64_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_float32_value(
        float& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        value = float32_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_float32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_float32_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            value = *((float*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_float32_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float32_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_float32_value(
        float value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        float32_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            float default_value;
            auto default_res = default_array_value_->get_float32_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_float32_value(value, id);
            }
            return insertResult;
        }
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            *((float*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_float32_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        float default_value;
        auto default_res = default_array_value_->get_float32_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_float32_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_float64_value(
        double& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        value = float64_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_float64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_float64_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            value = *((double*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_float64_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float64_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_float64_value(
        double value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        float64_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            double default_value;
            auto default_res = default_array_value_->get_float64_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_float64_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            *((double*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_float64_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        double default_value;
        auto default_res = default_array_value_->get_float64_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_float64_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_float128_value(
        long double& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        value = float128_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_float128_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_float128_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            value = *((long double*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_float128_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float128_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_float128_value(
        long double value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        float128_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float128_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            long double default_value;
            auto default_res = default_array_value_->get_float128_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_float128_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            *((long double*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_float128_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        long double default_value;
        auto default_res = default_array_value_->get_float128_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_float128_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_char8_value(
        char& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        value = char8_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_char8_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_char8_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            value = *((char*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_char8_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_char8_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_char8_value(
        char value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        char8_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_char8_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            char default_value;
            auto default_res = default_array_value_->get_char8_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_char8_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            *((char*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_char8_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        char default_value;
        auto default_res = default_array_value_->get_char8_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_char8_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_char16_value(
        wchar_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        value = char16_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_char16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_char16_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            value = *((wchar_t*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_char16_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_char16_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_char16_value(
        wchar_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        char16_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_char16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            wchar_t default_value;
            auto default_res = default_array_value_->get_char16_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_char16_value(value, id);
            }
            return insertResult;
        }
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            *((wchar_t*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_char16_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        wchar_t default_value;
        auto default_res = default_array_value_->get_char16_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_char16_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_byte_value(
        octet& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_BYTE && id == MEMBER_ID_INVALID)
    {
        value = byte_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_byte_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_byte_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_BYTE && id == MEMBER_ID_INVALID)
        {
            value = *((octet*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_byte_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_byte_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_byte_value(
        octet value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_BYTE && id == MEMBER_ID_INVALID)
    {
        byte_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicData* data = it->second;
            if (get_kind() == TypeKind::TK_BITSET && data->type_->get_descriptor().annotation_is_bit_bound())
            {
                uint16_t bit_bound = data->type_->get_descriptor().annotation_get_bit_bound();
                octet mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = it->second->set_byte_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            octet default_value;
            auto default_res = default_array_value_->get_byte_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_byte_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_BYTE && id == MEMBER_ID_INVALID)
        {
            *((octet*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            const DynamicTypeMember* member;
            bool found;

            std::tie(member, found) = type_->get_member(id);

            if (get_kind() == TypeKind::TK_BITSET)
            {
                if (!found)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                uint16_t bit_bound = member->annotation_get_bit_bound();
                octet mask = 0x00;
                for (uint16_t i = 0; i < bit_bound; ++i)
                {
                    mask = mask << 1;
                    mask += 1;
                }
                value &= mask;
            }
            ReturnCode_t result = ((DynamicData*)it->second)->set_byte_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        octet default_value;
        auto default_res = default_array_value_->get_byte_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_byte_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_bool_value(
        bool& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        value = bool_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (get_kind() == TypeKind::TK_BITMASK && id < type_->get_bounds())
    {
        value = (uint64_value_ & ((uint64_t)1 << id)) != 0;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_bool_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_bool_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.end();
    if (get_kind() == TypeKind::TK_BITMASK)
    {
        it = values_.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = values_.find(id);
    }
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            value = *((bool*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (get_kind() == TypeKind::TK_BITMASK && id < type_->get_bounds())
        {
            // Note that is not required for all bits in the mask to have an associated member
            value = (*((uint64_t*)it->second) & ((uint64_t)1 << *id)) != 0;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_bool_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_bool_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_bool_value(
        bool value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        bool_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (get_kind() == TypeKind::TK_BITMASK)
    {
        if (id == MEMBER_ID_INVALID)
        {
            if (value)
            {
                uint64_value_ = ~((uint64_t)0);
            }
            else
            {
                uint64_value_ = 0;
            }
        }
        else if (type_->get_bounds() == BOUND_UNLIMITED || id < type_->get_bounds())
        {
            if (value)
            {
                uint64_value_ |= ((uint64_t)1 << id);
            }
            else
            {
                uint64_value_ &= ~((uint64_t)1 << id);
            }
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting bool value. The given index is greater than the limit.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_bool_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            bool default_value;
            auto default_res = default_array_value_->get_bool_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_bool_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.end();
    if (get_kind() == TypeKind::TK_BITMASK)
    {
        it = values_.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = values_.find(id);
    }

    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            *((bool*)it->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (get_kind() == TypeKind::TK_BITMASK)
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
                return ReturnCode_t::RETCODE_OK;
            }
            else if (type_->get_bounds() == BOUND_UNLIMITED || id < type_->get_bounds())
            {
                // Note that is not required for all bits in the mask to have an associated member
                if (value)
                {
                    *((uint64_t*)it->second) |= ((uint64_t)1 << *id);
                }
                else
                {
                    *((uint64_t*)it->second) &= ~((uint64_t)1 << *id);
                }
                return ReturnCode_t::RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting bool value. The given index is greater than the limit.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_bool_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        bool default_value;
        auto default_res = default_array_value_->get_bool_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_bool_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_string_value(
        std::string& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        value = string_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_string_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_string_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            value = *((std::string*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_string_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_string_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_string_value(
        const std::string& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= type_->get_bounds())
        {
            string_value_ = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting string value. The given string is greater than the length limit.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_string_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            std::string default_value;
            auto default_res = default_array_value_->get_string_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_string_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= type_->get_bounds())
            {
                *((std::string*)it->second) = value;
                return ReturnCode_t::RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Error setting string value. The given string is greater than the length limit.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_string_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        std::string default_value;
        auto default_res = default_array_value_->get_string_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_string_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

MemberId DynamicData::get_union_id() const
{
    return union_id_;
}

ReturnCode_t DynamicData::set_union_id(
        MemberId id)
{
    if (get_kind() == TypeKind::TK_UNION)
    {
        if (id == MEMBER_ID_INVALID || type_->exists_member_by_id(id))
        {
            union_id_ = id;
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting union id. The id: " << id << " is unknown.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting union id. The kind: " << get_kind() << " doesn't support it.");
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::get_wstring_value(
        std::wstring& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        value = wstring_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_wstring_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_wstring_value(value, MEMBER_ID_INVALID);
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            value = *((std::wstring*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)it->second)->get_wstring_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_wstring_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::set_wstring_value(
        const std::wstring& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= type_->get_bounds())
        {
            wstring_value_ = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting wstring value. The given string is greater than the length limit.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_wstring_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            std::wstring default_value;
            auto default_res = default_array_value_->get_wstring_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_wstring_value(value, id);
            }
            return insertResult;
        }
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TypeKind::TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= type_->get_bounds())
            {
                *((std::wstring*)it->second) = value;
                return ReturnCode_t::RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Error setting wstring value. The given string is greater than the length limit.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)it->second)->set_wstring_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        std::wstring default_value;
        auto default_res = default_array_value_->get_wstring_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_wstring_value(value, id);
        }
        return insertResult;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicData::get_enum_value(
        uint32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if (uint32_value_ >= type_->get_member_count())
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        value = uint32_value_;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
        {
            uint32_t inner_value = *((uint32_t*)itValue->second);

            if (inner_value >= type_->get_member_count())
            {
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            value = inner_value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)itValue->second)->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::set_enum_value(
        const uint32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if(!type_->exists_member_by_id(value))
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        uint32_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            assert(default_array_value_);
            uint32_t default_value;
            auto default_res = default_array_value_->get_enum_value(default_value);

            if (!!default_res && value == default_value)
            { // don't add default elements
                return ReturnCode_t::RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_enum_value(value, id);
            }
            return insertResult;
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
        {

            if(!type_->exists_member_by_id(MemberId(value)))
            {
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            *((uint32_t*)itValue->second) = value;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)itValue->second)->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint32_t default_value;
        auto default_res = default_array_value_->get_enum_value(default_value);

        if (!!default_res && value == default_value)
        { // don't add default elements
            return ReturnCode_t::RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_enum_value(value, id);
        }
        return insertResult;
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::get_enum_value(
        std::string& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if(uint32_value_ >= type_->get_member_count())
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        MemberDescriptor md;
        ReturnCode_t res = get_member(md, uint32_value_);

        if(!!res)
        {
            value = md.get_name();
        }

        return res;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return it->second->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
        {
            uint32_t inner_value = *((uint32_t*)itValue->second);

            if(inner_value >= type_->get_member_count())
            {
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            MemberDescriptor md;
            ReturnCode_t res = type_->get_member(md, MemberId(inner_value));

            if(!!res)
            {
                value = md.get_name();
            }

            return res;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TypeKind::TK_UNION || id == union_id_)
            {
                return ((DynamicData*)itValue->second)->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::set_enum_value(
        const std::string& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
    {
        auto mid = get_member_id_by_name(value);
        if (mid == MEMBER_ID_INVALID)
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        uint32_value_ = mid;
        return ReturnCode_t::RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TypeKind::TK_ARRAY)
        {
            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == ReturnCode_t::RETCODE_OK)
            {
                return set_enum_value(value, id);
            }
            return insertResult;
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TypeKind::TK_ENUM && id == MEMBER_ID_INVALID)
        {
            auto mid = get_member_id_by_name(value);
            if (mid == MEMBER_ID_INVALID)
            {
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            *((uint32_t*)itValue->second) = *mid;
            return ReturnCode_t::RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = ((DynamicData*)itValue->second)->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == ReturnCode_t::RETCODE_OK && get_kind() == TypeKind::TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TypeKind::TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == ReturnCode_t::RETCODE_OK)
        {
            return set_enum_value(value, id);
        }
        return insertResult;
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::set_bitmask_value(
        uint64_t value)
{
    if (type_->get_kind() == TypeKind::TK_BITMASK)
    {
        return set_uint64_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::get_bitmask_value(
        uint64_t& value) const
{
    if (type_->get_kind() == TypeKind::TK_BITMASK)
    {
        return get_uint64_value(value, MEMBER_ID_INVALID);
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

MemberId DynamicData::get_array_index(
        const std::vector<uint32_t>& position)
{
    if (get_kind() == TypeKind::TK_ARRAY)
    {
        MemberId outPosition(0);
        uint32_t offset(1);
        if (position.size() == type_->get_bounds_size())
        {
            for (int32_t i = static_cast<int32_t>(position.size() - 1); i >= 0; --i)
            {
                outPosition += position[i] * offset;
                offset *= type_->get_bounds(static_cast<uint32_t>(i));
            }
            return outPosition;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error getting array index. Invalid dimension count.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error getting array index. The kind " << get_kind() << "doesn't support it.");
    }
    return MEMBER_ID_INVALID;
}

ReturnCode_t DynamicData::insert_array_data(
        MemberId indexId)
{
    if (get_kind() == TypeKind::TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < type_->get_total_bounds())
        {
            auto it = complex_values_.find(indexId);
            if (it != complex_values_.end())
            {
                DynamicDataFactory::get_instance()->delete_data(it->second);
                complex_values_.erase(it);
            }
            DynamicData* value = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            complex_values_.insert(std::make_pair(indexId, value));
            return ReturnCode_t::RETCODE_OK;
        }
#else
        if (indexId < type_->get_total_bounds())
        {
            auto it = values_.find(indexId);
            if (it != values_.end())
            {
                DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
                values_.erase(it);
            }
            DynamicData* value = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            values_.insert(std::make_pair(indexId, value));
            return ReturnCode_t::RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. Index out of bounds");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error inserting data. The kind " << get_kind() << " doesn't support this method");
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::clear_array_data(
        MemberId indexId)
{
    if (get_kind() == TypeKind::TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < type_->get_total_bounds())
        {
            auto it = complex_values_.find(indexId);
            if (it != complex_values_.end())
            {
                DynamicDataFactory::get_instance()->delete_data(it->second);
                complex_values_.erase(it);
            }
            return ReturnCode_t::RETCODE_OK;
        }
#else
        if (indexId < type_->get_total_bounds())
        {
            auto it = values_.find(indexId);
            if (it != values_.end())
            {
                DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
                values_.erase(it);
            }
            return ReturnCode_t::RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. Index out of bounds");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. The kind " << get_kind() << " doesn't support this method");
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::insert_int32_value(
        int32_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_INT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_int32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_uint32_value(
        uint32_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_UINT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_uint32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_int16_value(
        int16_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_INT16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_int16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_uint16_value(
        uint16_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_UINT16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_uint16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_int64_value(
        int64_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_INT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_int64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_uint64_value(
        uint64_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_UINT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_uint64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_float32_value(
        float value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_FLOAT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_float32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_float64_value(
        double value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_FLOAT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_float64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_float128_value(
        long double value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_FLOAT128)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_float128_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_char8_value(
        char value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_CHAR8)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_char8_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_char16_value(
        wchar_t value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_CHAR16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_char16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_byte_value(
        octet value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_BYTE)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_byte_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_bool_value(
        bool value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_BOOLEAN)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_bool_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_string_value(
        const std::string& value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_STRING8)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_string_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_wstring_value(
        const std::wstring& value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_STRING16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_wstring_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_enum_value(
        const std::string& value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->get_kind() == TypeKind::TK_ENUM)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == ReturnCode_t::RETCODE_OK)
        {
            result = set_enum_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_complex_value(
        const DynamicData* value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->equals(*value->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            outId = complex_values_.size();
            complex_values_.insert(std::make_pair(outId, DynamicDataFactory::get_instance()->create_copy(value)));
            return ReturnCode_t::RETCODE_OK;
#else
            outId = values_.size();
            values_.insert(std::make_pair(outId, DynamicDataFactory::get_instance()->create_copy(value)));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_complex_value(
        DynamicData_ptr value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->equals(*value->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            outId = complex_values_.size();
            complex_values_.insert(std::make_pair(outId, DynamicDataFactory::get_instance()->create_copy(value.get())));
            return ReturnCode_t::RETCODE_OK;
#else
            outId = values_.size();
            values_.insert(std::make_pair(outId, DynamicDataFactory::get_instance()->create_copy(value.get())));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_complex_value(
        DynamicData* value,
        MemberId& outId)
{
    if (get_kind() == TypeKind::TK_SEQUENCE && type_->get_element_type()->equals(*value->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            outId = complex_values_.size();
            complex_values_.insert(std::make_pair(outId, value));
            return ReturnCode_t::RETCODE_OK;
#else
            outId = values_.size();
            values_.insert(std::make_pair(outId, value));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_sequence_data(
        MemberId& outId)
{
    outId = MEMBER_ID_INVALID;
    if (get_kind() == TypeKind::TK_SEQUENCE)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            DynamicData* new_element = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            outId = complex_values_.size();
            complex_values_.insert(std::make_pair(outId, new_element));
            return ReturnCode_t::RETCODE_OK;
#else
            DynamicData* new_element = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            outId = values_.size();
            values_.insert(std::make_pair(outId, new_element));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error inserting data. The kind " << get_kind() << " doesn't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::remove_sequence_data(
        MemberId id)
{
    if (get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            DynamicDataFactory::get_instance()->delete_data(it->second);
            complex_values_.erase(it);
            return ReturnCode_t::RETCODE_OK;
        }
#else
        auto it = values_.find(id);
        if (it != values_.end())
        {
            DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
            values_.erase(it);
            return ReturnCode_t::RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. Member not found");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. The current Kind " << get_kind()
                                                                           << " doesn't support this method");

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData* key,
        MemberId& outKeyId,
        MemberId& outValueId)
{
    if (get_kind() == TypeKind::TK_MAP && type_->get_key_element_type()->equals(*key->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
            {
                if (it->second->key_element_ && it->second->equals(key))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKeyId = 0u;
            if (complex_values_.size())
            { // get largest key available
                outKeyId = complex_values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            complex_values_.insert(std::make_pair(outKeyId, keyCopy));

            DynamicData* new_element = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            outValueId = outKeyId + 1u;
            complex_values_.insert(std::make_pair(outValueId, new_element));
            return ReturnCode_t::RETCODE_OK;
#else
            for (auto it = values_.begin(); it != values_.end(); ++it)
            {
                if (((DynamicData*)it->second)->key_element_ && ((DynamicData*)it->second)->equals(key))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKeyId = 0u;
            if (values_.size())
            { // get largest key available
                outKeyId = values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            values_.insert(std::make_pair(outKeyId, keyCopy));

            DynamicData* new_element = DynamicDataFactory::get_instance()->create_data(type_->get_element_type());
            outValueId = outKeyId + 1u;
            values_.insert(std::make_pair(outValueId, new_element));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The map is full");
            return ReturnCode_t::RETCODE_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The current Kind " << get_kind()
                                                                                  << " doesn't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData* key,
        DynamicData* value,
        MemberId& outKey,
        MemberId& outValue)
{
    if (get_kind() == TypeKind::TK_MAP && type_->get_key_element_type()->equals(*key->type_.get()) &&
            type_->get_element_type()->equals(*value->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
            {
                if (it->second->key_element_ && it->second->equals(key))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKeyId = 0u;
            if (complex_values_.size())
            { // get largest key available
                outKeyId = complex_values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            complex_values_.insert(std::make_pair(outKey, keyCopy));

            outValueId = outKeyId + 1u;
            complex_values_.insert(std::make_pair(outValue, value));
            return ReturnCode_t::RETCODE_OK;
#else
            for (auto it = values_.begin(); it != values_.end(); ++it)
            {
                if (it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (values_.size())
            { // get largest key available
                outKey = values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            values_.insert(std::make_pair(outKey, keyCopy));

            outValue = outKey + 1u;
            values_.insert(std::make_pair(outValue, value));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The map is full");
            return ReturnCode_t::RETCODE_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The current Kind " << get_kind()
                                                                                  << " doesn't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData* key,
        const DynamicData* value,
        MemberId& outKey,
        MemberId& outValue)
{
    if (get_kind() == TypeKind::TK_MAP && type_->get_key_element_type()->equals(*key->type_.get()) &&
            type_->get_element_type()->equals(*value->type_.get()))
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
            {
                if (it->second->key_element_ && it->second->equals(key))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKeyId = 0u;
            if (complex_values_.size())
            { // get largest key available
                outKeyId = complex_values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            complex_values_.insert(std::make_pair(outKey, keyCopy));

            outValueId = outKeyId + 1u;
            DynamicData* valueCopy = DynamicDataFactory::get_instance()->create_copy(value);
            complex_values_.insert(std::make_pair(outValue, valueCopy));
            return ReturnCode_t::RETCODE_OK;
#else
            for (auto it = values_.begin(); it != values_.end(); ++it)
            {
                if (it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (values_.size())
            { // get largest key available
                outKey = values_.rbegin()->first + 1u;
            }
            DynamicData* keyCopy = DynamicDataFactory::get_instance()->create_copy(key);
            keyCopy->key_element_ = true;
            values_.insert(std::make_pair(outKey, keyCopy));

            outValue = outKey + 1u;
            DynamicData* valueCopy = DynamicDataFactory::get_instance()->create_copy(value);
            values_.insert(std::make_pair(outValue, valueCopy));
            return ReturnCode_t::RETCODE_OK;
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The map is full");
            return ReturnCode_t::RETCODE_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The current Kind " << get_kind()
                                                                                  << " doesn't support this method");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::insert_map_data(
        const DynamicData* key,
        DynamicData_ptr value,
        MemberId& outKey,
        MemberId& outValue)
{
    return insert_map_data(key, reinterpret_cast<const DynamicData*>(value.get()), outKey, outValue);
}

ReturnCode_t DynamicData::remove_map_data(
        MemberId keyId)
{
    if (get_kind() == TypeKind::TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto itKey = complex_values_.find(keyId);
        auto itValue = complex_values_.find(keyId + 1);
        if (itKey != complex_values_.end() && itValue != complex_values_.end() && itKey->second->key_element_)
        {
            DynamicDataFactory::get_instance()->delete_data(itKey->second);
            DynamicDataFactory::get_instance()->delete_data(itValue->second);
            complex_values_.erase(itKey);
            complex_values_.erase(itValue);
            return ReturnCode_t::RETCODE_OK;
        }
#else
        auto itKey = values_.find(keyId);
        auto itValue = values_.find(keyId + 1);
        if (itKey != values_.end() && itValue != values_.end() && ((DynamicData*)itKey->second)->key_element_)
        {
            DynamicDataFactory::get_instance()->delete_data(((DynamicData*)itKey->second));
            DynamicDataFactory::get_instance()->delete_data(((DynamicData*)itValue->second));
            values_.erase(itKey);
            values_.erase(itValue);
            return ReturnCode_t::RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing from map. Invalid input KeyId");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing from map. The current Kind " << get_kind()
                                                                                   << " doesn't support this method");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

ReturnCode_t DynamicData::clear_data()
{
    if (get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_MAP || get_kind() == TypeKind::TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
        {
            DynamicDataFactory::get_instance()->delete_data(it->second);
        }
        complex_values_.clear();
#else
        for (auto it = values_.begin(); it != values_.end(); ++it)
        {
            DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
        }
        values_.clear();
#endif // ifdef DYNAMIC_TYPES_CHECKING
        return ReturnCode_t::RETCODE_OK;
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error clearing data. The current Kind " << get_kind()
                                                                           << " doesn't support this method");

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicData::get_complex_value(
        DynamicData** value,
        MemberId id) const
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (id != MEMBER_ID_INVALID && (get_kind() == TypeKind::TK_STRUCTURE || get_kind() == TypeKind::TK_UNION ||
            get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_ARRAY || get_kind() == TypeKind::TK_MAP || get_kind() == TypeKind::TK_BITSET))
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            *value = DynamicDataFactory::get_instance()->create_copy(it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
#else
        auto it = values_.find(id);
        if (it != values_.end())
        {
            *value = DynamicDataFactory::get_instance()->create_copy((DynamicData*)it->second);
            return ReturnCode_t::RETCODE_OK;
        }
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::set_complex_value(
        DynamicData* value,
        MemberId id)
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (id != MEMBER_ID_INVALID && (get_kind() == TypeKind::TK_STRUCTURE || get_kind() == TypeKind::TK_UNION ||
            get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_ARRAY || get_kind() == TypeKind::TK_MAP || get_kind() == TypeKind::TK_BITSET))
    {
        // With containers, check that the index is valid
        if ((get_kind() == TypeKind::TK_SEQUENCE || get_kind() == TypeKind::TK_ARRAY || get_kind() == TypeKind::TK_MAP) &&
                id < type_->get_total_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = complex_values_.find(id);
            if (it != complex_values_.end())
            {
                if (get_kind() == TypeKind::TK_MAP && it->second->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. They given id is a Key value.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                else
                {
                    if (it->second != nullptr)
                    {
                        DynamicDataFactory::get_instance()->delete_data(it->second);
                    }
                    complex_values_.erase(it);

                    complex_values_.insert(std::make_pair(id, value));
                    if (get_kind() == TypeKind::TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                }
            }
            else if (get_kind() == TypeKind::TK_ARRAY)
            {
                complex_values_.insert(std::make_pair(id, value));
                return ReturnCode_t::RETCODE_OK;
            }

#else
            auto it = values_.find(id);
            if (it != values_.end())
            {
                if (get_kind() == TypeKind::TK_MAP && ((DynamicData*)it->second)->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. They given id is a Key value.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
                else
                {
                    if (it->second != nullptr)
                    {
                        DynamicDataFactory::get_instance()->delete_data((DynamicData*)it->second);
                    }
                    values_.erase(it);
                    values_.insert(std::make_pair(id, value));
                    if (get_kind() == TypeKind::TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                }
            }
            else if (get_kind() == TypeKind::TK_ARRAY)
            {
                values_.insert(std::make_pair(id, value));
                return ReturnCode_t::RETCODE_OK;
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. id out of bounds.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicData::get_union_label(
        uint64_t& value) const
{
    try
    {
        value = get_union_label();
    }
    catch(std::system_error& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());
        return e.code().value();
    }

    return ReturnCode_t::RETCODE_OK;
}

uint64_t DynamicData::get_union_label() const
{
    assert(type_);
    if(type_->get_kind() != TypeKind::TK_UNION)
    {
        throw std::system_error(
                ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
                "Error retrieving union label, underlying type is not an union.");
    }

    const DynamicTypeMember* member;
    bool found;

    std::tie(member, found) = type_->get_member(union_id_);

    // set_union cannot be inconsistent
    assert(found);

    // return label if available
    auto it = member->labels_.cbegin();
    if (it != member->labels_.cend())
    {
        return *it;
    }

    throw std::system_error(
            ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
            "Error retrieving union label, no label associated.");
}

MemberId DynamicData::get_discriminator_value() const
{
    if(type_->get_kind() != TypeKind::TK_UNION)
    {
        throw std::system_error(
                ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
                "Error retrieving discriminator, underlying type is not an union.");
    }

    return union_id_;
}

ReturnCode_t DynamicData::get_discriminator_value(MemberId& id) const noexcept
{
    try
    {
        id = get_discriminator_value();
    }
    catch(std::system_error& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());
        return e.code().value();
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DynamicData::set_discriminator_value(
        MemberId value) noexcept
{
    if(type_->get_kind() != TypeKind::TK_UNION)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    union_id_ = value;
    return ReturnCode_t::RETCODE_OK;
}

bool DynamicData::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    assert(type_);
    return type_->deserialize(*this, cdr);
}

size_t DynamicData::getCdrSerializedSize(
        const DynamicData* data,
        size_t current_alignment /*= 0*/)
{
    assert(data);

    return data->get_type()->getCdrSerializedSize(*data, current_alignment);
}

size_t DynamicData::getKeyMaxCdrSerializedSize(
        const DynamicType_ptr type,
        size_t current_alignment /*= 0*/)
{
    assert(type);
    return type->getKeyMaxCdrSerializedSize(current_alignment);
}

size_t DynamicData::getMaxCdrSerializedSize(
        const DynamicType_ptr type,
        size_t current_alignment /*= 0*/)
{
    assert(type);
    return type->getMaxCdrSerializedSize(current_alignment);
}

void DynamicData::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(type_);
    type_->serialize(*this, cdr);
}

void DynamicData::serializeKey(
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(type_);
    type_->serializeKey(*this, cdr);
}

size_t DynamicData::getEmptyCdrSerializedSize(
        const DynamicType* type,
        size_t current_alignment /*= 0*/)
{
    return type->getEmptyCdrSerializedSize(current_alignment);
}

bool DynamicData::has_children() const
{
    switch(get_kind())
    {
        case TypeKind::TK_ANNOTATION:
        case TypeKind::TK_ARRAY:
        case TypeKind::TK_MAP:
        case TypeKind::TK_SEQUENCE:
        case TypeKind::TK_STRUCTURE:
        case TypeKind::TK_UNION:
        case TypeKind::TK_BITSET:
            return true;
        default:
            return false;
    };
}
