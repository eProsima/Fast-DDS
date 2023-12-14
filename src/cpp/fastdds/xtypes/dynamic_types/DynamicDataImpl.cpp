// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <algorithm>
#include <codecvt>
#include <locale>
#include <memory>

#include "DynamicTypeImpl.hpp"
#include "DynamicDataImpl.hpp"
#include "DynamicDataFactoryImpl.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/string_convert.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

bool map_compare(
        const std::map<MemberId, std::shared_ptr<DynamicDataImpl>>& left,
        const std::map<MemberId, std::shared_ptr<DynamicDataImpl>>& right)
{
    auto pred = [](decltype(*left.begin()) a, decltype(a) b)
            {
                return a.first == b.first && (a.second == b.second || *a.second == *b.second);
            };

    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), pred);
}

DataState::DataState(
        const DynamicTypeImpl& type)
    : type_(type.shared_from_this())
{
}

DynamicDataImpl::DynamicDataImpl(
        use_the_create_method,
        const DynamicTypeImpl& type) noexcept
    : DataState(type)
{
    if (type.is_complex_kind())
    {
        // Bitmasks and enums register their members but only manages one value.
        if (type.get_kind() == TK_BITMASK || type.get_kind() == TK_ENUM)
        {
            add_value(type.get_kind(), MEMBER_ID_INVALID);
        }
        else
        {
            for (auto pm : type.get_all_members())
            {
                assert(pm);

                /*TODO(richiware)
                   std::shared_ptr<DynamicDataImpl> data = DynamicDataFactoryImpl::get_instance().create_data(
                    dynamic_cast<DynamicTypeImpl*>(pm->type().get()));
                   if (pm->type()->get_kind() != TK_BITSET &&
                        pm->type()->get_kind() != TK_STRUCTURE &&
                        pm->type()->get_kind() != TK_UNION &&
                        pm->type()->get_kind() != TK_SEQUENCE &&
                        pm->type()->get_kind() != TK_ARRAY &&
                        pm->type()->get_kind() != TK_MAP)
                   {
                    ObjectName def_value = pm->annotation_get_default();
                    if (0 == def_value.size())
                    {
                        data->set_value(std::string(def_value.c_str()));
                    }
                   }
                 #ifdef DYNAMIC_TYPES_CHECKING
                   complex_values_.emplace(pm->id(), data);
                 #else
                   values_.emplace(pm->get_id(), data);
                 #endif // ifdef DYNAMIC_TYPES_CHECKING

                 */

                // Set the default value for unions.
                /*TODO(richiware)
                   if (type.get_kind() == TK_UNION &&
                        pm->is_default_label())
                   {
                    set_union_id(pm->id());
                   }
                 */
            }
        }

        // If there isn't a default value... set the first element of the union
        if (type.get_kind() == TK_UNION &&
                get_union_id() == MEMBER_ID_INVALID &&
                type.get_member_count())
        {
            set_union_id(get_member_id_at_index(0));
        }
    }
    else
    {
        // Resolve alias type, avoid reference counting
        const DynamicTypeImpl* ptype = &type;
        while ( ptype->get_kind() == TK_ALIAS )
        {
            ptype = ptype->get_base_type().get();
        }

        assert(ptype);

        add_value(ptype->get_kind(), MEMBER_ID_INVALID);
    }

}

DynamicDataImpl::DynamicDataImpl(
        use_the_create_method,
        const DynamicDataImpl& data) noexcept
    : DataState(data)
{
    // only copy non-loaned objects
    assert(lender_.expired());
}

DynamicDataImpl::DynamicDataImpl(
        use_the_create_method,
        DynamicDataImpl&& data) noexcept
    : DataState(std::move(data))
{
    // only copy non-loaned objects
    assert(lender_.expired());
}

const DynamicTypeImpl& DynamicDataImpl::get_type() const noexcept
{
    assert(type_);
    return *type_;
}

ReturnCode_t DynamicDataImpl::get_descriptor(
        MemberDescriptor& value,
        MemberId id) const noexcept
{
    ReturnCode_t rc;
    const DynamicTypeMember* member = type_->get_interface().get_member(id, &rc);

    if (!rc)
    {
        return rc;
    }

    //TODO(richiware) return member->get_descriptor(value);
}

MemberId DynamicDataImpl::get_member_id_by_name(
        const std::string& name) const
{
    assert(type_);
    return type_->get_member_id_by_name(name);
}

MemberId DynamicDataImpl::get_member_id_at_index(
        uint32_t index) const
{
    assert(type_);
    return type_->get_member_id_at_index(index);
}

TypeKind DynamicDataImpl::get_kind() const
{
    assert(type_);
    return type_->get_kind();
}

uint32_t DynamicDataImpl::get_item_count() const
{
    if (get_kind() == TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        return static_cast<uint32_t>(complex_values_.size() / 2);
#else
        return static_cast<uint32_t>(values_.size() / 2);
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else if (get_kind() == TK_ARRAY)
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

std::string DynamicDataImpl::get_name()
{
    return type_->get_name();
}

void DynamicDataImpl::add_value(
        TypeKind kind,
        MemberId id)
{
    switch (kind)
    {
        default:
            break;
        case TK_INT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<int32_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_UINT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<uint32_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_INT16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<int16_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_UINT16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<uint16_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_INT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<int64_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_UINT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<uint64_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_FLOAT32:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<float>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_FLOAT64:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<double>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_FLOAT128:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<long double>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_CHAR8:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<char>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_CHAR16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<wchar_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_BOOLEAN:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<bool>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_BYTE:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<eprosima::fastrtps::rtps::octet>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_STRING8:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<std::string>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_STRING16:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<std::wstring>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_ENUM:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<uint32_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_BITMASK:
        {
#ifndef DYNAMIC_TYPES_CHECKING
            values_.emplace(id, std::make_shared<uint64_t>());
#endif // ifndef DYNAMIC_TYPES_CHECKING
        }
    }
    set_default_value(id);
}

void DynamicDataImpl::clean()
{
    // keep the type alive
    auto type = get_type().shared_from_this();
    // reset all
    *this = DynamicDataImpl(use_the_create_method{}, *type);
}

ReturnCode_t DynamicDataImpl::clear_all_values()
{
    if (type_->is_complex_kind())
    {
        if (get_kind() == TK_SEQUENCE || get_kind() == TK_MAP || get_kind() == TK_ARRAY)
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
                std::static_pointer_cast<DynamicDataImpl>(e.second)->clear_all_values();
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
    }
    else
    {
        set_default_value(MEMBER_ID_INVALID);
    }
    return RETCODE_OK;
}

void DynamicDataImpl::clean_members()
{
#ifdef DYNAMIC_TYPES_CHECKING
    complex_values_.clear();
#else
    values_.clear();
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::clear_nonkey_values()
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
            std::static_pointer_cast<DynamicDataImpl>(e.second)->clear_nonkey_values();
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
    return RETCODE_OK;
}

ReturnCode_t DynamicDataImpl::clear_value(
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
        std::static_pointer_cast<DynamicDataImpl>(itValue->second)->clear_all_values();
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    else
    {
        set_default_value(id);
    }
    return RETCODE_OK;
}

bool DynamicDataImpl::compare_values(
        TypeKind kind,
        std::shared_ptr<void> left,
        std::shared_ptr<void> right) const
{
    return compare_values(kind, left.get(), right.get());
}

bool DynamicDataImpl::compare_values(
        TypeKind kind,
        void* left,
        void* right) const
{
    switch (kind)
    {
        default:
            break;
        case TK_INT32:      {
            return *((int32_t*)left) == *((int32_t*)right);
        }
        case TK_UINT32:     {
            return *((uint32_t*)left) == *((uint32_t*)right);
        }
        case TK_INT16:      {
            return *((int16_t*)left) == *((int16_t*)right);
        }
        case TK_UINT16:     {
            return *((uint16_t*)left) == *((uint16_t*)right);
        }
        case TK_INT64:      {
            return *((int64_t*)left) == *((int64_t*)right);
        }
        case TK_UINT64:     {
            return *((uint64_t*)left) == *((uint64_t*)right);
        }
        case TK_FLOAT32:    {
            return *((float*)left) == *((float*)right);
        }
        case TK_FLOAT64:    {
            return *((double*)left) == *((double*)right);
        }
        case TK_FLOAT128:   {
            return *((long double*)left) == *((long double*)right);
        }
        case TK_CHAR8:      {
            return *((char*)left) == *((char*)right);
        }
        case TK_CHAR16:     {
            return *((wchar_t*)left) == *((wchar_t*)right);
        }
        case TK_BOOLEAN:    {
            return *((bool*)left) == *((bool*)right);
        }
        case TK_BYTE:       {
            return *((eprosima::fastrtps::rtps::octet*)left) == *((eprosima::fastrtps::rtps::octet*)right);
        }
        case TK_STRING8:    {
            return *((std::string*)left) == *((std::string*)right);
        }
        case TK_STRING16:   {
            return *((std::wstring*)left) == *((std::wstring*)right);
        }
        case TK_ENUM:       {
            return *((uint32_t*)left) == *((uint32_t*)right);
        }
    }
    return false;
}

std::string DynamicDataImpl::get_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    switch (type_->get_kind())
    {
        default:
            break;
        case TK_INT32:
        {
            int32_t value(0);
            get_int32_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_UINT32:
        {
            uint32_t value(0);
            get_uint32_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_INT16:
        {
            int16_t value(0);
            get_int16_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_UINT16:
        {
            uint16_t value(0);
            get_uint16_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_INT64:
        {
            int64_t value(0);
            get_int64_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_UINT64:
        {
            uint64_t value(0);
            get_uint64_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_FLOAT32:
        {
            float value(0.0f);
            get_float32_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_FLOAT64:
        {
            double value(0.0f);
            get_float64_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_FLOAT128:
        {
            long double value(0.0f);
            get_float128_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_CHAR8:
        {
            char value = 0;
            get_char8_value(value, id);
            return std::string(1, value);
        }
        break;
        case TK_CHAR16:
        {
            wchar_t value(0);
            get_char16_value(value, id);
            std::wstring temp = L"";
            temp += value;
            return fastrtps::wstring_to_bytes(temp);
        }
        break;
        case TK_BOOLEAN:
        {
            bool value(false);
            get_bool_value(value, id);
            return std::to_string(value ? 1 : 0);
        }
        break;
        case TK_BYTE:
        {
            uint8_t value(0);
            get_byte_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_STRING8:
        {
            return get_string_value(id);
        }
        break;
        case TK_STRING16:
        {
            std::wstring value;
            get_wstring_value(value, id);
            return fastrtps::wstring_to_bytes(value);
        }
        break;
        case TK_ENUM:
        {
            uint32_t value;
            get_enum_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_BITMASK:
        {
            uint64_t value(0);
            get_uint64_value(value, id);
            return std::to_string(value);
        }
        break;
        case TK_ARRAY:
        case TK_SEQUENCE:
        case TK_BITSET:
        case TK_MAP:
        {
            // THESE TYPES DON'T MANAGE VALUES
        }
        break;
    }

    return {};
}

void DynamicDataImpl::set_value(
        const std::string& sValue,
        MemberId id /*= MEMBER_ID_INVALID*/)
{
    switch (type_->get_kind())
    {
        default:
            break;
        case TK_INT32:
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
        case TK_UINT32:
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
        case TK_INT16:
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
        case TK_UINT16:
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
        case TK_INT64:
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
        case TK_UINT64:
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
        case TK_FLOAT32:
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
        case TK_FLOAT64:
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
        case TK_FLOAT128:
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
        case TK_CHAR8:
        {
            if (sValue.length() >= 1)
            {
                set_char8_value(sValue[0], id);
            }
        }
        break;
        case TK_CHAR16:
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
        case TK_BOOLEAN:
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
        case TK_BYTE:
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
        case TK_STRING8:
        {
            set_string_value(sValue, id);
        }
        break;
        case TK_STRING16:
        {
            set_wstring_value(std::wstring(sValue.begin(), sValue.end()), id);
        }
        break;
        case TK_ENUM:
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
        case TK_BITMASK:
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
        case TK_ARRAY:
        case TK_SEQUENCE:
        case TK_BITSET:
        case TK_MAP:
        {
            // THESE TYPES DON'T MANAGE VALUES
        }
        break;
    }
}

void DynamicDataImpl::set_default_value(
        MemberId id)
{
    assert(type_);

    std::string defaultValue;

    try
    {
        const DynamicTypeMemberImpl& m = type_->get_member(id);
        //TODO(richiware) defaultValue = m.get_default_value();
    }
    catch (const std::system_error& ec)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot set member " << id << " default value because: " << ec.what());
        return;
    }

    switch (type_->get_kind())
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
            catch (...)
            {
            }
            set_int32_value(value, id);
        }
        break;
        case TK_UINT32:
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
        case TK_INT16:
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
        case TK_UINT16:
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
        case TK_INT64:
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
        case TK_UINT64:
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
        case TK_FLOAT32:
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
        case TK_FLOAT64:
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
        case TK_FLOAT128:
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
        case TK_CHAR8:
        {
            if (defaultValue.length() >= 1)
            {
                set_char8_value(defaultValue[0], id);
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
            catch (...)
            {
            }

            set_char16_value(value, id);
        }
        break;
        case TK_BOOLEAN:
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
        case TK_BYTE:
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
        case TK_STRING8:
        {
            set_string_value(defaultValue, id);
        }
        break;
        case TK_STRING16:
        {
            set_wstring_value(std::wstring(defaultValue.begin(), defaultValue.end()), id);
        }
        break;
        case TK_ENUM:
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
        case TK_BITMASK:
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
        case TK_ARRAY:
        case TK_SEQUENCE:
        case TK_BITSET:
        case TK_MAP:
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
                std::shared_ptr<DynamicDataImpl> sp = std::static_pointer_cast<DynamicDataImpl>(itValue->second);
                if (!sp->key_element_)
                {
                    sp->set_default_value(MEMBER_ID_INVALID);
                }
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        break;
    }
}

std::shared_ptr<DynamicDataImpl> DynamicDataImpl::loan_value(
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
                if (get_kind() == TK_MAP && it->second->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    if (get_kind() == TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                    loaned_values_.push_back(id);
                    it->second->lender_ = weak_from_this();
                    return it->second;
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                if (insert_array_data(id) == RETCODE_OK)
                {
                    loaned_values_.push_back(id);
                    auto& sp = complex_values_.at(id);
                    sp->lender_ = weak_from_this();
                    return sp;
                }
            }
#else
            auto it = values_.find(id);
            if (it != values_.end())
            {
                if (get_kind() == TK_MAP &&
                        std::static_pointer_cast<DynamicDataImpl>(it->second)->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                }
                else
                {
                    auto sp = std::static_pointer_cast<DynamicDataImpl>(it->second);

                    if (get_kind() == TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }

                    loaned_values_.push_back(id);
                    sp->lender_ = weak_from_this();
                    return sp;
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                if (insert_array_data(id) == RETCODE_OK)
                {
                    auto sp = std::static_pointer_cast<DynamicDataImpl>(values_.at(id));

                    loaned_values_.push_back(id);
                    sp->lender_ = weak_from_this();
                    return sp;
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
    return {};
}

ReturnCode_t DynamicDataImpl::return_loaned_value(
        std::shared_ptr<DynamicDataImpl> value)
{
    for (auto loanIt = loaned_values_.begin(); loanIt != loaned_values_.end(); ++loanIt)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(*loanIt);
        if (it != complex_values_.end() && it->second == value)
        {
            value->lender_.reset();
            it->second = DynamicDataFactoryImpl::get_instance().create_copy(std::move(*value));
            loaned_values_.erase(loanIt);
            return RETCODE_OK;
        }
#else
        auto it = values_.find(*loanIt);
        if (it != values_.end() && std::static_pointer_cast<DynamicDataImpl>(it->second) == value)
        {
            value->lender_.reset();
            it->second = DynamicDataFactoryImpl::get_instance().create_copy(std::move(*value));
            loaned_values_.erase(loanIt);
            return RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error returning loaned Value. The value hasn't been loaned.");
    return RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t DynamicDataImpl::get_int32_value(
        int32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        value = int32_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_int32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_int32_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<int32_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_int32_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int32_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_int32_value(
        int32_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
    {
        int32_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            auto data = it->second;
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       int32_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_int32_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            int32_t default_value;
            auto default_res = default_array_value_->get_int32_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_int32_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT32 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<int32_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    int32_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }

            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_int32_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int32_t default_value;
        auto default_res = default_array_value_->get_int32_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_int32_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_uint32_value(
        uint32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        value = uint32_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_uint32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_uint32_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<uint32_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_uint32_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint32_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_uint32_value(
        uint32_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
    {
        uint32_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET )
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       uint32_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_uint32_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            uint32_t default_value;
            auto default_res = default_array_value_->get_uint32_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_uint32_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<uint32_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    uint32_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_uint32_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint32_t default_value;
        auto default_res = default_array_value_->get_uint32_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_uint32_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_int16_value(
        int16_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        value = int16_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_int16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_int16_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<int16_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_int16_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int16_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_int16_value(
        int16_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
    {
        int16_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       int16_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_int16_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            int16_t default_value;
            auto default_res = default_array_value_->get_int16_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_int16_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<int16_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    int16_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }

            auto result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_int16_value(value,
                            MEMBER_ID_INVALID);
            if (!!result && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int16_t default_value;
        auto default_res = default_array_value_->get_int16_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_int16_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_uint16_value(
        uint16_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        value = uint16_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_uint16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_uint16_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<uint16_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_uint16_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint16_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_uint16_value(
        uint16_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
    {
        uint16_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       uint16_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_uint16_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            uint16_t default_value;
            auto default_res = default_array_value_->get_uint16_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_uint16_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_UINT16 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<uint16_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    uint16_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            auto result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_uint16_value(value,
                            MEMBER_ID_INVALID);
            if (!!result && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint16_t default_value;
        auto default_res = default_array_value_->get_uint16_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_uint16_value(value, id);
        }
        return insertResult;
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_int64_value(
        int64_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        value = int64_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_int64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_int64_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<int64_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_int64_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_int64_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_int64_value(
        int64_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
    {
        int64_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       int64_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_int64_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            int64_t default_value;
            auto default_res = default_array_value_->get_int64_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_int64_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_INT64 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<int64_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    int64_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }

            auto result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_int64_value(value,
                            MEMBER_ID_INVALID);
            if (!!result && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        int64_t default_value;
        auto default_res = default_array_value_->get_int64_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_int64_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_uint64_value(
        uint64_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if ((get_kind() == TK_UINT64 || get_kind() == TK_BITMASK) && id == MEMBER_ID_INVALID)
    {
        value = uint64_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_uint64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_uint64_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if ((get_kind() == TK_UINT64 || get_kind() == TK_BITMASK) && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<uint64_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_uint64_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_uint64_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_uint64_value(
        uint64_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if ((get_kind() == TK_UINT64 || get_kind() == TK_BITMASK) && id == MEMBER_ID_INVALID)
    {
        uint64_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richiware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       uint64_t mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_uint64_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            uint64_t default_value;
            auto default_res = default_array_value_->get_uint64_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_uint64_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if ((get_kind() == TK_UINT64 || get_kind() == TK_BITMASK) && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<uint64_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    uint64_t mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_uint64_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint64_t default_value;
        auto default_res = default_array_value_->get_uint64_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_uint64_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_float32_value(
        float& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        value = float32_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_float32_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_float32_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<float>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_float32_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float32_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_float32_value(
        float value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
    {
        float32_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float32_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            float default_value;
            auto default_res = default_array_value_->get_float32_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_float32_value(value, id);
            }
            return insertResult;
        }
    }

    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT32 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<float>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_float32_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        float default_value;
        auto default_res = default_array_value_->get_float32_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_float32_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_float64_value(
        double& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        value = float64_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_float64_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_float64_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<double>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_float64_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float64_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_float64_value(
        double value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
    {
        float64_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float64_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            double default_value;
            auto default_res = default_array_value_->get_float64_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_float64_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT64 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<double>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_float64_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        double default_value;
        auto default_res = default_array_value_->get_float64_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_float64_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_float128_value(
        long double& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        value = float128_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_float128_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_float128_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<long double>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_float128_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_float128_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_float128_value(
        long double value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
    {
        float128_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_float128_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            long double default_value;
            auto default_res = default_array_value_->get_float128_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_float128_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_FLOAT128 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<long double>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_float128_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        long double default_value;
        auto default_res = default_array_value_->get_float128_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_float128_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_char8_value(
        char& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        value = char8_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_char8_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_char8_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<char>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_char8_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_char8_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_char8_value(
        char value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
    {
        char8_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_char8_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            char default_value;
            auto default_res = default_array_value_->get_char8_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_char8_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_CHAR8 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<char>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_char8_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        char default_value;
        auto default_res = default_array_value_->get_char8_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_char8_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_char16_value(
        wchar_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        value = char16_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_char16_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_char16_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<wchar_t>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_char16_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_char16_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_char16_value(
        wchar_t value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
    {
        char16_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_char16_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            wchar_t default_value;
            auto default_res = default_array_value_->get_char16_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_char16_value(value, id);
            }
            return insertResult;
        }
    }

    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_CHAR16 && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<wchar_t>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_char16_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        wchar_t default_value;
        auto default_res = default_array_value_->get_char16_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_char16_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_byte_value(
        eprosima::fastrtps::rtps::octet& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        value = byte_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_byte_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_byte_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<eprosima::fastrtps::rtps::octet>(it->second);
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_byte_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_byte_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_byte_value(
        eprosima::fastrtps::rtps::octet value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
    {
        byte_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    /*TODO(richware)
                       uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                       eprosima::fastrtps::rtps::octet mask = 0x00;
                       for (uint16_t i = 0; i < bit_bound; ++i)
                       {
                        mask = mask << 1;
                        mask += 1;
                       }
                       value &= mask;
                     */
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = it->second->set_byte_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            eprosima::fastrtps::rtps::octet default_value;
            auto default_res = default_array_value_->get_byte_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_byte_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_BYTE && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<eprosima::fastrtps::rtps::octet>(it->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() == TK_BITSET)
            {
                try
                {
                    uint16_t bit_bound = type_->get_member(id).annotation_get_bit_bound();
                    eprosima::fastrtps::rtps::octet mask = 0x00;
                    for (uint16_t i = 0; i < bit_bound; ++i)
                    {
                        mask = mask << 1;
                        mask += 1;
                    }
                    value &= mask;
                }
                catch (const std::system_error& e)
                {
                    return e.code().value();
                }
            }
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_byte_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        eprosima::fastrtps::rtps::octet default_value;
        auto default_res = default_array_value_->get_byte_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_byte_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_bool_value(
        bool& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        value = bool_value_;
        return RETCODE_OK;
    }
    else if (get_kind() == TK_BITMASK && id < type_->get_bounds())
    {
        value = (uint64_value_ & ((uint64_t)1 << id)) != 0;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_bool_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_bool_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.end();
    if (get_kind() == TK_BITMASK)
    {
        it = values_.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = values_.find(id);
    }
    if (it != values_.end())
    {
        if (get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            value = *std::static_pointer_cast<bool>(it->second);
            return RETCODE_OK;
        }
        else if (get_kind() == TK_BITMASK && id < type_->get_bounds())
        {
            // Note that is not required for all bits in the mask to have an associated member
            value = (*std::static_pointer_cast<uint64_t>(it->second) & ((uint64_t)1 << id)) != 0;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_bool_value(value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_bool_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_bool_value(
        bool value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
    {
        bool_value_ = value;
        return RETCODE_OK;
    }
    else if (get_kind() == TK_BITMASK)
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
            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting bool value. The given index is greater than the limit.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_bool_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            bool default_value;
            auto default_res = default_array_value_->get_bool_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_bool_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.end();
    if (get_kind() == TK_BITMASK)
    {
        it = values_.find(MEMBER_ID_INVALID);
    }
    else
    {
        it = values_.find(id);
    }

    if (it != values_.end())
    {
        if (get_kind() == TK_BOOLEAN && id == MEMBER_ID_INVALID)
        {
            *std::static_pointer_cast<bool>(it->second) = value;
            return RETCODE_OK;
        }
        else if (get_kind() == TK_BITMASK)
        {
            if (id == MEMBER_ID_INVALID)
            {
                if (value)
                {
                    *std::static_pointer_cast<uint64_t>(it->second) = ~((uint64_t)0);
                }
                else
                {
                    *std::static_pointer_cast<uint64_t>(it->second) = 0;
                }
                return RETCODE_OK;
            }
            else if (type_->get_bounds() == BOUND_UNLIMITED || id < type_->get_bounds())
            {
                // Note that is not required for all bits in the mask to have an associated member
                if (value)
                {
                    *std::static_pointer_cast<uint64_t>(it->second) |= ((uint64_t)1 << id);
                }
                else
                {
                    *std::static_pointer_cast<uint64_t>(it->second) &= ~((uint64_t)1 << id);
                }
                return RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting bool value. The given index is greater than the limit.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_bool_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        bool default_value;
        auto default_res = default_array_value_->get_bool_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_bool_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_string_value(
        std::string& value,
        MemberId id) const
{
    const char* val = nullptr;
    auto res = get_string_value(val, id);

    if (!!res)
    {
        value = val;
    }

    return res;
}

ReturnCode_t DynamicDataImpl::get_string_value(
        const char*& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        value = string_value_.c_str();
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_string_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_string_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            value = std::static_pointer_cast<std::string>(it->second)->c_str();
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_string_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_string_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_string_value(
        const std::string& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= type_->get_bounds())
        {
            string_value_ = value;
            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting string value. The given string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_string_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            std::string default_value;
            auto default_res = default_array_value_->get_string_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_string_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_STRING8 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= type_->get_bounds())
            {
                *std::static_pointer_cast<std::string>(it->second) = value;
                return RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Error setting string value. The given string is greater than the length limit.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_string_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        std::string default_value;
        auto default_res = default_array_value_->get_string_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_string_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

MemberId DynamicDataImpl::get_union_id() const
{
    return union_id_;
}

ReturnCode_t DynamicDataImpl::set_union_id(
        MemberId id)
{
    if (get_kind() == TK_UNION)
    {
        if (id == MEMBER_ID_INVALID || type_->exists_member_by_id(id))
        {
            union_id_ = id;
            return RETCODE_OK;
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
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_wstring_value(
        std::wstring& value,
        MemberId id) const
{
    const wchar_t* val = nullptr;
    auto ret = get_wstring_value(val, id);
    if (!!ret)
    {
        value = val;
    }
    return ret;
}

ReturnCode_t DynamicDataImpl::get_wstring_value(
        const wchar_t*& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        value = wstring_value_.c_str();
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_wstring_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_wstring_value(value, MEMBER_ID_INVALID);
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            value = std::static_pointer_cast<std::wstring>(it->second)->c_str();
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(it->second)->get_wstring_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_wstring_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::set_wstring_value(
        const std::wstring& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
    {
        if (value.length() <= type_->get_bounds())
        {
            wstring_value_ = value;
            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting wstring value. The given string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_wstring_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            std::wstring default_value;
            auto default_res = default_array_value_->get_wstring_value(default_value);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
            {
                return set_wstring_value(value, id);
            }
            return insertResult;
        }
    }
    return RETCODE_BAD_PARAMETER;
#else
    auto it = values_.find(id);
    if (it != values_.end())
    {
        if (get_kind() == TK_STRING16 && id == MEMBER_ID_INVALID)
        {
            if (value.length() <= type_->get_bounds())
            {
                *std::static_pointer_cast<std::wstring>(it->second) = value;
                return RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Error setting wstring value. The given string is greater than the length limit.");
                return RETCODE_BAD_PARAMETER;
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_wstring_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        std::wstring default_value;
        auto default_res = default_array_value_->get_wstring_value(default_value);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_wstring_value(value, id);
        }
        return insertResult;
    }

    return RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
}

ReturnCode_t DynamicDataImpl::get_enum_value(
        uint32_t& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if (uint32_value_ >= type_->get_member_count())
        {
            return RETCODE_BAD_PARAMETER;
        }

        value = uint32_value_;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            uint32_t inner_value = *std::static_pointer_cast<uint32_t>(itValue->second);

            if (inner_value >= type_->get_member_count())
            {
                return RETCODE_BAD_PARAMETER;
            }

            value = inner_value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(itValue->second)->get_enum_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_enum_value(
        const uint32_t& value,
        MemberId id /*= MEMBER_ID_INVALID*/)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if (!type_->exists_member_by_id(MemberId(value)))
        {
            return RETCODE_BAD_PARAMETER;
        }

        uint32_value_ = value;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            assert(default_array_value_);
            uint32_t default_value;
            auto default_res = default_array_value_->get_enum_value(default_value, id);

            if (!!default_res && value == default_value)
            {
                // don't add default elements
                return RETCODE_OK;
            }

            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
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
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {

            if (!type_->exists_member_by_id(MemberId(value)))
            {
                return RETCODE_BAD_PARAMETER;
            }

            *std::static_pointer_cast<uint32_t>(itValue->second) = value;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(itValue->second)->set_enum_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        assert(default_array_value_);
        uint32_t default_value;
        auto default_res = default_array_value_->get_enum_value(default_value, id);

        if (!!default_res && value == default_value)
        {
            // don't add default elements
            return RETCODE_OK;
        }

        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_enum_value(value, id);
        }
        return insertResult;
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_enum_value(
        std::string& value,
        MemberId id) const
{
    const char* val = nullptr;
    auto res = get_enum_value(val, id);

    if (!!res)
    {
        value = val;
    }

    return res;
}

ReturnCode_t DynamicDataImpl::get_enum_value(
        const char*& value,
        MemberId id) const
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        if (uint32_value_ >= type_->get_member_count())
        {
            return RETCODE_BAD_PARAMETER;
        }

        try
        {
            //TODO(richiware) value = type_->get_member(MemberId{uint32_value_}).name().c_str();
        }
        catch (const std::system_error& e)
        {
            return e.code().value();
        }
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return it->second->get_enum_value(value, MEMBER_ID_INVALID);
            }
        }
        else if (get_kind() == TK_ARRAY)
        {
            return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
        }
    }
#else
    auto itValue = values_.find(id);
    if (itValue != values_.end())
    {
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            uint32_t inner_value = *std::static_pointer_cast<uint32_t>(itValue->second);

            if (inner_value >= type_->get_member_count())
            {
                return RETCODE_BAD_PARAMETER;
            }

            try
            {
                value = type_->get_member(MemberId(inner_value)).get_name().c_str();
                return {};
            }
            catch (const std::system_error& e)
            {
                return e.code().value();
            }
        }
        else if (id != MEMBER_ID_INVALID)
        {
            if (get_kind() != TK_UNION || id == union_id_)
            {
                return std::static_pointer_cast<DynamicDataImpl>(itValue->second)->get_enum_value(value,
                               MEMBER_ID_INVALID);
            }
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        return default_array_value_->get_enum_value(value, MEMBER_ID_INVALID);
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_enum_value(
        const std::string& value,
        MemberId id)
{
#ifdef DYNAMIC_TYPES_CHECKING
    if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
    {
        auto mid = get_member_id_by_name(value);
        if (mid == MEMBER_ID_INVALID)
        {
            return RETCODE_BAD_PARAMETER;
        }

        uint32_value_ = mid;
        return RETCODE_OK;
    }
    else if (id != MEMBER_ID_INVALID)
    {
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            ReturnCode_t result = it->second->set_enum_value(value, MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
        else if (get_kind() == TK_ARRAY)
        {
            ReturnCode_t insertResult = insert_array_data(id);
            if (insertResult == RETCODE_OK)
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
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            auto mid = get_member_id_by_name(value);
            if (mid == MEMBER_ID_INVALID)
            {
                return RETCODE_BAD_PARAMETER;
            }

            *std::static_pointer_cast<uint32_t>(itValue->second) = mid;
            return RETCODE_OK;
        }
        else if (id != MEMBER_ID_INVALID)
        {
            ReturnCode_t result = std::static_pointer_cast<DynamicDataImpl>(itValue->second)->set_enum_value(value,
                            MEMBER_ID_INVALID);
            if (result == RETCODE_OK && get_kind() == TK_UNION)
            {
                set_union_id(id);
            }
            return result;
        }
    }
    else if (get_kind() == TK_ARRAY && id != MEMBER_ID_INVALID)
    {
        ReturnCode_t insertResult = insert_array_data(id);
        if (insertResult == RETCODE_OK)
        {
            return set_enum_value(value, id);
        }
        return insertResult;
    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_bitmask_value(
        uint64_t value)
{
    if (type_->get_kind() == TK_BITMASK)
    {
        return set_uint64_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_bitmask_value(
        uint64_t& value) const
{
    if (type_->get_kind() == TK_BITMASK)
    {
        return get_uint64_value(value, MEMBER_ID_INVALID);
    }
    return RETCODE_BAD_PARAMETER;
}

MemberId DynamicDataImpl::get_array_index(
        const std::vector<uint32_t>& position) const
{
    if (get_kind() == TK_ARRAY)
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

ReturnCode_t DynamicDataImpl::insert_array_data(
        MemberId indexId)
{
    if (get_kind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < type_->get_total_bounds())
        {
            auto value = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            complex_values_.emplace(indexId, value);
            return RETCODE_OK;
        }
#else
        if (indexId < type_->get_total_bounds())
        {
            auto value = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            values_.emplace(indexId, value);
            return RETCODE_OK;
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
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::clear_array_data(
        MemberId indexId)
{
    if (get_kind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        if (indexId < type_->get_total_bounds())
        {
            auto it = complex_values_.find(indexId);
            if (it != complex_values_.end())
            {
                complex_values_.erase(it);
            }
            return RETCODE_OK;
        }
#else
        if (indexId < type_->get_total_bounds())
        {
            auto it = values_.find(indexId);
            if (it != values_.end())
            {
                values_.erase(it);
            }
            return RETCODE_OK;
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
    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::insert_int32_value(
        int32_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_INT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_int32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_uint32_value(
        uint32_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_UINT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_uint32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_int16_value(
        int16_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_INT16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_int16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_uint16_value(
        uint16_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_UINT16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_uint16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_int64_value(
        int64_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_INT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_int64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_uint64_value(
        uint64_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_UINT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_uint64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_float32_value(
        float value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_FLOAT32)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_float32_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_float64_value(
        double value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_FLOAT64)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_float64_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_float128_value(
        long double value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_FLOAT128)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_float128_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_char8_value(
        char value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_CHAR8)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_char8_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_char16_value(
        wchar_t value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_CHAR16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_char16_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_byte_value(
        eprosima::fastrtps::rtps::octet value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_BYTE)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_byte_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_bool_value(
        bool value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_BOOLEAN)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_bool_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_string_value(
        const std::string& value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_STRING8)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_string_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_wstring_value(
        const std::wstring& value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_STRING16)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_wstring_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_enum_value(
        const std::string& value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && type_->get_element_type()->get_kind() == TK_ENUM)
    {
        ReturnCode_t result = insert_sequence_data(outId);
        if (result == RETCODE_OK)
        {
            result = set_enum_value(value, outId);
        }
        return result;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_complex_value(
        const DynamicDataImpl& value,
        MemberId& outId)
{
    if (get_kind() == TK_SEQUENCE && *type_->get_element_type() == *value.type_)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            outId = complex_values_.size();
            complex_values_.emplace(outId, DynamicDataFactoryImpl::get_instance().create_copy(value));
#else
            outId = values_.size();
            values_.emplace(outId, DynamicDataFactoryImpl::get_instance().create_copy(value));
#endif // ifdef DYNAMIC_TYPES_CHECKING
            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The current kinds don't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_sequence_data(
        MemberId& outId)
{
    outId = MEMBER_ID_INVALID;
    if (get_kind() == TK_SEQUENCE)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
            auto new_element = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
#ifdef DYNAMIC_TYPES_CHECKING
            outId = complex_values_.size();
            complex_values_.emplace(outId, new_element);
#else
            outId = values_.size();
            values_.emplace(outId, new_element);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting data. The container is full.");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error inserting data. The kind " << get_kind() << " doesn't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::remove_sequence_data(
        MemberId id)
{
    if (get_kind() == TK_SEQUENCE || get_kind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            complex_values_.erase(it);
            return RETCODE_OK;
        }
#else
        auto it = values_.find(id);
        if (it != values_.end())
        {
            values_.erase(it);
            return RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. Member not found");
        return RETCODE_BAD_PARAMETER;
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing data. The current Kind " << get_kind()
                                                                           << " doesn't support this method");

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::insert_map_data(
        const DynamicDataImpl& key,
        MemberId& outKeyId,
        MemberId& outValueId)
{
    if (get_kind() == fastrtps::types::TK_MAP && *type_->get_key_element_type() == *key.type_)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
            {
                if (it->second->key_element_ && *it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = complex_values_.size();
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            complex_values_.emplace(outKeyId, keyCopy);

            auto new_element = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            outValueId = complex_values_.size();
            complex_values_.emplace(outValueId, new_element);
#else
            for (auto it = values_.begin(); it != values_.end(); ++it)
            {
                auto data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                if (data->key_element_ && *data == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = values_.size();
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            values_.emplace(outKeyId, keyCopy);

            auto new_element = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            outValueId = values_.size();
            values_.emplace(outValueId, new_element);
#endif // ifdef DYNAMIC_TYPES_CHECKING

            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The map is full");
            return RETCODE_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The current Kind " << get_kind()
                                                                                  << " doesn't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::insert_map_data(
        const DynamicDataImpl& key,
        const DynamicDataImpl& value,
        MemberId& outKey,
        MemberId& outValue)
{
    if (get_kind() == TK_MAP &&
            *type_->get_key_element_type() == *key.type_ &&
            *type_->get_element_type() == *value.type_)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
            {
                if (it->second->key_element_ && *it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (complex_values_.size())
            {
                // get largest key available
                outKey = complex_values_.rbegin()->first + 1u;
            }

            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            complex_values_.emplace(outKey, keyCopy);

            outValue = outKey + 1u;
            auto valueCopy = DynamicDataFactoryImpl::get_instance().create_copy(value);
            complex_values_.emplace(outValue, valueCopy);
#else
            for (auto it = values_.begin(); it != values_.end(); ++it)
            {
                if (*std::static_pointer_cast<DynamicDataImpl>(it->second) == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (values_.size())
            {
                // get largest key available
                outKey = values_.rbegin()->first + 1u;
            }
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            values_.emplace(outKey, keyCopy);

            outValue = outKey + 1u;
            auto valueCopy = DynamicDataFactoryImpl::get_instance().create_copy(value);
            values_.emplace(outValue, valueCopy);
#endif // ifdef DYNAMIC_TYPES_CHECKING

            return RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The map is full");
            return RETCODE_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The current Kind " << get_kind()
                                                                                  << " doesn't support this method");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::remove_map_data(
        MemberId keyId)
{
    if (get_kind() == TK_MAP)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto itKey = complex_values_.find(keyId);
        auto itValue = complex_values_.find(keyId + 1);
        if (itKey != complex_values_.end() && itValue != complex_values_.end() && itKey->second->key_element_)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*itKey->second);
            DynamicDataFactoryImpl::get_instance().delete_data(*itValue->second);
            complex_values_.erase(itKey);
            complex_values_.erase(itValue);
            return RETCODE_OK;
        }
#else
        auto itKey = values_.find(keyId);
        auto itValue = values_.find(keyId + 1);
        if (itKey != values_.end() && itValue != values_.end() &&
                std::static_pointer_cast<DynamicDataImpl>(itKey->second)->key_element_)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(itKey->second));
            DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(itValue->
                            second));
            values_.erase(itKey);
            values_.erase(itValue);
            return RETCODE_OK;
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing from map. Invalid input KeyId");
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error removing from map. The current Kind " << get_kind()
                                                                                   << " doesn't support this method");
        return RETCODE_ERROR;
    }
}

ReturnCode_t DynamicDataImpl::clear_data()
{
    if (get_kind() == TK_SEQUENCE || get_kind() == TK_MAP || get_kind() == TK_ARRAY)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = complex_values_.begin(); it != complex_values_.end(); ++it)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*it->second);
        }
        complex_values_.clear();
#else
        for (auto it = values_.begin(); it != values_.end(); ++it)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(it->second));
        }
        values_.clear();
#endif // ifdef DYNAMIC_TYPES_CHECKING
        return RETCODE_OK;
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error clearing data. The current Kind " << get_kind()
                                                                           << " doesn't support this method");

    return RETCODE_BAD_PARAMETER;
}

std::shared_ptr<const DynamicDataImpl> DynamicDataImpl::get_complex_value(
        MemberId id /*= MEMBER_ID_INVALID*/) const
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (id != MEMBER_ID_INVALID && (get_kind() == TK_STRUCTURE || get_kind() == TK_UNION ||
            get_kind() == TK_SEQUENCE || get_kind() == TK_ARRAY || get_kind() == TK_MAP ||
            get_kind() == TK_BITSET))
    {
#ifdef DYNAMIC_TYPES_CHECKING
        auto it = complex_values_.find(id);
        if (it != complex_values_.end())
        {
            return DynamicDataFactoryImpl::get_instance().create_copy(*it->second);
        }
        throw RETCODE_BAD_PARAMETER;
#else
        auto it = values_.find(id);
        if (it != values_.end())
        {
            return DynamicDataFactoryImpl::get_instance().create_copy(*std::static_pointer_cast<const DynamicDataImpl>(
                               it->second));
        }
        throw RETCODE_BAD_PARAMETER;
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
        throw RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::set_complex_value(
        const DynamicDataImpl& value,
        MemberId id)
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (id != MEMBER_ID_INVALID && (get_kind() == TK_STRUCTURE || get_kind() == TK_UNION ||
            get_kind() == TK_SEQUENCE || get_kind() == TK_ARRAY || get_kind() == TK_MAP ||
            get_kind() == TK_BITSET))
    {
        // With containers, check that the index is valid
        if ((get_kind() == TK_SEQUENCE || get_kind() == TK_ARRAY ||
                get_kind() == TK_MAP) &&
                id < type_->get_total_bounds())
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = complex_values_.find(id);
            if (it != complex_values_.end())
            {
                if (get_kind() == TK_MAP && it->second->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. They given id is a Key value.");
                    return RETCODE_BAD_PARAMETER;
                }
                else
                {
                    if (it->second)
                    {
                        DynamicDataFactoryImpl::get_instance().delete_data(*it->second);
                    }
                    complex_values_.erase(it);

                    auto value_copy = DynamicDataFactoryImpl::get_instance().create_copy(value);
                    complex_values_.emplace(id, value_copy);
                    if (get_kind() == TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                auto value_copy = DynamicDataFactoryImpl::get_instance().create_copy(value);
                complex_values_.emplace(id, value_copy);
            }

#else
            auto it = values_.find(id);
            if (it != values_.end())
            {
                if (get_kind() == TK_MAP &&
                        std::static_pointer_cast<DynamicDataImpl>(it->second)->key_element_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. They given id is a Key value.");
                    return RETCODE_BAD_PARAMETER;
                }
                else
                {
                    if (it->second != nullptr)
                    {
                        DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(it
                                        ->second));
                    }
                    values_.erase(it);
                    auto value_copy = DynamicDataFactoryImpl::get_instance().create_copy(value);
                    values_.emplace(id, value_copy);
                    if (get_kind() == TK_UNION && union_id_ != id)
                    {
                        set_union_id(id);
                    }
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                auto value_copy = DynamicDataFactoryImpl::get_instance().create_copy(value);
                values_.emplace(id, value_copy);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. id out of bounds.");
            return RETCODE_BAD_PARAMETER;
        }
        return RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::get_union_label(
        uint64_t& value) const
{
    try
    {
        value = get_union_label();
    }
    catch (std::system_error& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());
        return e.code().value();
    }

    return RETCODE_OK;
}

uint64_t DynamicDataImpl::get_union_label() const
{
    assert(type_);
    if (type_->get_kind() != TK_UNION)
    {
        //TODO(richiware) throw std::system_error(
        //TODO(richiware)           RETCODE_PRECONDITION_NOT_MET,
        //TODO(richiware)           "Error retrieving union label, underlying type is not an union.");
    }

    // return label if available
    /*TODO(richiware)
       auto& labels = type_->get_member(union_id_).label();
       auto it = labels.cbegin();
       if (it != labels.cend())
       {
        return *it;
       }
     */

    //TODO(richiware) throw std::system_error(
    //TODO(richiware)           RETCODE_PRECONDITION_NOT_MET,
    //TODO(richiware)           "Error retrieving union label, no label associated.");
}

MemberId DynamicDataImpl::get_discriminator_value() const
{
    if (type_->get_kind() != TK_UNION)
    {
        //TODO(richiware) throw std::system_error(
        //TODO(richiware)           RETCODE_PRECONDITION_NOT_MET,
        //TODO(richiware)           "Error retrieving discriminator, underlying type is not an union.");
    }

    return union_id_;
}

ReturnCode_t DynamicDataImpl::get_discriminator_value(
        MemberId& id) const noexcept
{
    try
    {
        id = get_discriminator_value();
    }
    catch (std::system_error& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());
        return e.code().value();
    }

    return RETCODE_OK;
}

ReturnCode_t DynamicDataImpl::set_discriminator_value(
        MemberId value) noexcept
{
    if (type_->get_kind() != TK_UNION)
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    union_id_ = value;
    return RETCODE_OK;
}

bool DynamicDataImpl::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    assert(type_);
    return type_->deserialize(*this, cdr);
}

size_t DynamicDataImpl::getCdrSerializedSize(
        const DynamicDataImpl& data,
        size_t current_alignment /*= 0*/)
{
    return data.get_type().getCdrSerializedSize(data, current_alignment);
}

size_t DynamicDataImpl::getKeyMaxCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment /*= 0*/)
{
    return type.getKeyMaxCdrSerializedSize(current_alignment);
}

size_t DynamicDataImpl::getMaxCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment /*= 0*/)
{
    return type.getMaxCdrSerializedSize(current_alignment);
}

void DynamicDataImpl::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(type_);
    type_->serialize(*this, cdr);
}

void DynamicDataImpl::serializeKey(
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(type_);
    type_->serializeKey(*this, cdr);
}

size_t DynamicDataImpl::getEmptyCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment /*= 0*/)
{
    return type.getEmptyCdrSerializedSize(current_alignment);
}

bool DynamicDataImpl::has_children() const
{
    switch (get_kind())
    {
        case TK_ANNOTATION:
        case TK_ARRAY:
        case TK_MAP:
        case TK_SEQUENCE:
        case TK_STRUCTURE:
        case TK_UNION:
        case TK_BITSET:
            return true;
        default:
            return false;
    }
}

bool DynamicDataImpl::operator ==(
        const DynamicDataImpl& other) const noexcept
{
    if (&other == this)
    {
        return true;
    }
    else if (*type_ == *other.type_)
    {
        // Optimization for unions, only check the selected element.
        if (get_kind() == TK_UNION)
        {
            if (union_id_ != other.union_id_)
            {
                return false;
            }
            else if (union_id_ != MEMBER_ID_INVALID)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = complex_values_.find(union_id_);
                auto otherIt = other.complex_values_.find(union_id_);
#else
                auto it = values_.find(union_id_);
                auto otherIt = other.values_.find(union_id_);
#endif // ifdef DYNAMIC_TYPES_CHECKING

                return it->second == otherIt->second ||
                       *std::static_pointer_cast<DynamicDataImpl>(it->second) ==
                       *std::static_pointer_cast<DynamicDataImpl>(otherIt->second);
            }
        }
        else
        {
#ifdef DYNAMIC_TYPES_CHECKING
            bool bFail = false;
            bFail = int32_value_ != other.int32_value_;
            bFail = bFail || uint32_value_ != other.uint32_value_;
            bFail = bFail || int16_value_ != other.int16_value_;
            bFail = bFail || uint16_value_ != other.uint16_value_;
            bFail = bFail || int64_value_ != other.int64_value_;
            bFail = bFail || uint64_value_ != other.uint64_value_;
            bFail = bFail || float32_value_ != other.float32_value_;
            bFail = bFail || float64_value_ != other.float64_value_;
            bFail = bFail || float128_value_ != other.float128_value_;
            bFail = bFail || char8_value_ != other.char8_value_;
            bFail = bFail || char16_value_ != other.char16_value_;
            bFail = bFail || byte_value_ != other.byte_value_;
            bFail = bFail || bool_value_ != other.bool_value_;
            bFail = bFail || string_value_ != other.string_value_;
            bFail = bFail || wstring_value_ != other.wstring_value_;
            bFail = bFail || !map_compare(complex_values_, other.complex_values_);

            if (bFail)
            {
                return false;
            }
#else
            if (get_kind() == TK_ENUM)
            {
                return compare_values(TK_UINT32, values_.begin()->second, other.values_.begin()->second);
            }
            else if (get_kind() == TK_BITMASK)
            {
                TypeKind bitmask_kind = TK_BYTE;
                size_t type_size = type_->get_size();
                switch (type_size)
                {
                    case 1: bitmask_kind = TK_BYTE; break;
                    case 2: bitmask_kind = TK_UINT16; break;
                    case 4: bitmask_kind = TK_UINT32; break;
                    case 8: bitmask_kind = TK_UINT64; break;
                }

                assert(values_.size() && other.values_.size());

                return compare_values(bitmask_kind, values_.begin()->second, other.values_.begin()->second);
            }
            else if (type_->is_complex_kind())
            {
                // array, map, sequence, structure, bitset, anotation
                return values_.size() == other.values_.size() &&
                       std::equal(
                    values_.begin(),
                    values_.end(),
                    other.values_.begin(),
                    [](const decltype(values_)::value_type& l, const decltype(values_)::value_type& r)
                    {
                        return l.second == r.second ||
                        *std::static_pointer_cast<DynamicDataImpl>(l.second) ==
                        *std::static_pointer_cast<DynamicDataImpl>(r.second);
                    });
            }
            else
            {
                // primitives
                return compare_values(get_kind(), values_.begin()->second, other.values_.begin()->second);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        return true;
    }

    return false;
}

bool DynamicDataImpl::operator !=(
        const DynamicDataImpl& data) const noexcept
{
    return !(*this == data);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
