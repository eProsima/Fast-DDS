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

#include "DynamicDataImpl.hpp"

#include <algorithm>
#include <string>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include "DynamicTypeMemberImpl.hpp"

namespace eprosima {

namespace fastcdr {

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const fastdds::dds::traits<fastdds::dds::DynamicDataImpl>::ref_type& data,
        size_t& current_alignment)
{
    return data->calculate_serialized_size(calculator, current_alignment);
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const fastdds::dds::traits<fastdds::dds::DynamicDataImpl>::ref_type& data)
{
    return data->serialize(scdr);
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        fastdds::dds::traits<fastdds::dds::DynamicDataImpl>::ref_type& data)
{
    data->deserialize(cdr);
}

} // namespace fastcdr

namespace fastdds {
namespace dds {

bool is_complex_kind(
        TypeKind kind)
{
    switch (kind)
    {
        case TK_ANNOTATION:
        case TK_ARRAY:
        case TK_BITMASK:
        case TK_BITSET:
        case TK_ENUM:
        case TK_MAP:
        case TK_SEQUENCE:
        case TK_STRUCTURE:
        case TK_UNION:
            return true;
        default:
            return false;
    }
}

DynamicDataImpl::DynamicDataImpl(
        traits<DynamicType>::ref_type type) noexcept
    : type_(traits<DynamicType>::narrow<DynamicTypeImpl>(type))
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        for (auto& member : type_->get_all_members_by_index())
        {
            traits<DynamicData>::ref_type data = DynamicDataFactory::get_instance()->create_data(
                member->get_descriptor().type());
            traits<DynamicDataImpl>::ref_type data_impl = traits<DynamicData>::narrow<DynamicDataImpl>(data);
            set_default_value(member, data_impl);

            value_.emplace(member->get_id(), data);

            /*TODO(richiware) think
               // Set the default value for unions.
               if (TK_UNION == type_kind &&
                    member->get_descriptor().is_default_label())
               {
                set_union_id(pm->id());
               }
             */
        }
        // If there isn't a default value... set the first element of the union
        /*TODO(richiware) think
           if (type_kind == TK_UNION &&
           get_union_id() == MEMBER_ID_INVALID &&
           type.get_member_count())
           {
           set_union_id(get_member_id_at_index(0));
           }
         */
    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        auto sequence_size {0};

        if (TK_ARRAY == type_kind)
        {
            auto bounds_it {type_->get_descriptor().bound().cbegin()};
            assert(bounds_it != type_->get_descriptor().bound().cend());
            sequence_size = *bounds_it;

            while (++bounds_it != type_->get_descriptor().bound().cend())
            {
                sequence_size *= *bounds_it;
            }
        }

        TypeKind sequence_type = get_enclosing_typekind(
            traits<DynamicType>::narrow<DynamicTypeImpl>(type_->get_descriptor().element_type()));

        add_sequence_value(sequence_type, sequence_size);
    }
    else if (TK_BITSET != type_kind &&
            TK_MAP != type_kind) // Primitives
    {
        add_value(type_kind, MEMBER_ID_INVALID);
    }
}

void DynamicDataImpl::add_sequence_value(
        TypeKind sequence_kind,
        uint32_t sequence_size) noexcept
{
    if (is_complex_kind(sequence_kind))
    {
        if (TK_ANNOTATION == sequence_kind ||
                TK_STRUCTURE == sequence_kind ||
                TK_UNION == sequence_kind)
        {
            value_.emplace(MEMBER_ID_INVALID,
                    std::make_shared<std::vector<traits<DynamicData>::ref_type>>(sequence_size));
        }
    }
    else
    {
        switch (sequence_kind)
        {
            case TK_INT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<int32_t>>(sequence_size));
            }
            break;
            case TK_UINT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<uint32_t>>(sequence_size));
            }
            break;
            case TK_INT8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<int8_t>>(sequence_size));
            }
            break;
            case TK_INT16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<int16_t>>(sequence_size));
            }
            break;
            case TK_UINT16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<uint16_t>>(sequence_size));
            }
            break;
            case TK_INT64:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<int64_t>>(sequence_size));
            }
            break;
            case TK_UINT64:
            case TK_BITMASK:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<uint64_t>>(sequence_size));
            }
            break;
            case TK_FLOAT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<float>>(sequence_size));
            }
            break;
            case TK_FLOAT64:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<double>>(sequence_size));
            }
            break;
            case TK_FLOAT128:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<long double>>(sequence_size));
            }
            break;
            case TK_CHAR8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<char>>(sequence_size));
            }
            break;
            case TK_CHAR16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<wchar_t>>(sequence_size));
            }
            break;
            case TK_BOOLEAN:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<bool>>(sequence_size));
            }
            break;
            case TK_BYTE:
            case TK_UINT8:
            {
                value_.emplace(MEMBER_ID_INVALID,
                        std::make_shared<std::vector<eprosima::fastrtps::rtps::octet>>(sequence_size));
            }
            break;
            case TK_STRING8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<std::string>>(sequence_size));
            }
            break;
            case TK_STRING16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<std::vector<std::wstring>>(sequence_size));
            }
            break;
            default:
                break;
        }
    }
}

void DynamicDataImpl::add_value(
        TypeKind kind,
        MemberId id) noexcept
{
    switch (kind)
    {
        case TK_INT32:
        {
            value_.emplace(id, std::make_shared<int32_t>());
        }
        break;
        case TK_UINT32:
        {
            value_.emplace(id, std::make_shared<uint32_t>());
        }
        break;
        case TK_INT8:
        {
            value_.emplace(id, std::make_shared<int8_t>());
        }
        break;
        case TK_INT16:
        {
            value_.emplace(id, std::make_shared<int16_t>());
        }
        break;
        case TK_UINT16:
        {
            value_.emplace(id, std::make_shared<uint16_t>());
        }
        break;
        case TK_INT64:
        {
            value_.emplace(id, std::make_shared<int64_t>());
        }
        break;
        case TK_UINT64:
        case TK_BITMASK:
        {
            value_.emplace(id, std::make_shared<uint64_t>());
        }
        break;
        case TK_FLOAT32:
        {
            value_.emplace(id, std::make_shared<float>());
        }
        break;
        case TK_FLOAT64:
        {
            value_.emplace(id, std::make_shared<double>());
        }
        break;
        case TK_FLOAT128:
        {
            value_.emplace(id, std::make_shared<long double>());
        }
        break;
        case TK_CHAR8:
        {
            value_.emplace(id, std::make_shared<char>());
        }
        break;
        case TK_CHAR16:
        {
            value_.emplace(id, std::make_shared<wchar_t>());
        }
        break;
        case TK_BOOLEAN:
        {
            value_.emplace(id, std::make_shared<bool>());
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            value_.emplace(id, std::make_shared<eprosima::fastrtps::rtps::octet>());
        }
        break;
        case TK_STRING8:
        {
            value_.emplace(id, std::make_shared<std::string>());
        }
        break;
        case TK_STRING16:
        {
            value_.emplace(id, std::make_shared<std::wstring>());
        }
        break;
        default:
            break;
    }
}

void DynamicDataImpl::set_value(
        const ObjectName& sValue,
        MemberId id) noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    switch (type_kind)
    {
        case TK_INT32:
        {
            int32_t value {0};
            try
            {
                value = stoi(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int32_value(id, value);
        }
        break;
        case TK_UINT32:
        {
            uint32_t value {0};
            try
            {
                value = stoul(sValue.to_string());
            }
            catch (...)
            {
            }
            set_uint32_value(id, value);
        }
        break;
        case TK_INT8:
        {
            int8_t value {0};
            try
            {
                value = static_cast<int8_t>(stol(sValue.to_string()));
            }
            catch (...)
            {
            }
            set_int8_value(id, value);
        }
        break;
        case TK_INT16:
        {
            int16_t value {0};
            try
            {
                value = static_cast<int16_t>(stoi(sValue.to_string()));
            }
            catch (...)
            {
            }
            set_int16_value(id, value);
        }
        break;
        case TK_UINT16:
        {
            uint16_t value {0};
            try
            {
                value = static_cast<uint16_t>(stoul(sValue.to_string()));
            }
            catch (...)
            {
            }
            set_uint16_value(id, value);
        }
        break;
        case TK_INT64:
        {
            int64_t value {0};
            try
            {
                value = stoll(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int64_value(id, value);
        }
        break;
        case TK_UINT64:
        case TK_BITMASK:
        {
            uint64_t value(0);
            try
            {
                value = stoul(sValue.to_string());
            }
            catch (...)
            {
            }
            set_uint64_value(id, value);
        }
        break;
        case TK_FLOAT32:
        {
            float value {0.0f};
            try
            {
                value = stof(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float32_value(id, value);
        }
        break;
        case TK_FLOAT64:
        {
            double value {0.0f};
            try
            {
                value = stod(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float64_value(id, value);
        }
        break;
        case TK_FLOAT128:
        {
            long double value {0.0f};
            try
            {
                value = stold(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float128_value(id, value);
        }
        break;
        case TK_CHAR8:
        {
            char value {0};
            if (sValue.size() >= 1)
            {
                value = sValue[0];
            }

            set_char8_value(id, value);
        }
        break;
        case TK_CHAR16:
        {
            wchar_t value {0};
            try
            {
                std::string str = sValue.to_string();
                std::wstring temp = std::wstring(str.begin(), str.end());
                value = temp[0];
            }
            catch (...)
            {
            }

            set_char16_value(id, value);
        }
        break;
        case TK_BOOLEAN:
        {
            int value {0};
            try
            {
                value = stoi(sValue.to_string());
            }
            catch (...)
            {
            }
            set_boolean_value(id, value == 0 ? false : true);
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            uint8_t value {0};
            if (sValue.size() >= 1)
            {
                try
                {
                    value = static_cast<uint8_t>(stoul(sValue.to_string()));
                }
                catch (...)
                {
                }
            }
            set_byte_value(id, value);
        }
        break;
        case TK_STRING8:
        {
            set_string_value(id, sValue.to_string());
        }
        break;
        case TK_STRING16:
        {
            std::string str = sValue.to_string();
            set_wstring_value(id, std::wstring(str.begin(), str.end()));
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
        default:
            break;
    }
}

traits<DynamicType>::ref_type DynamicDataImpl::type() noexcept
{
    return type_;
}

ReturnCode_t DynamicDataImpl::get_descriptor(
        traits<MemberDescriptor>::ref_type& value,
        MemberId id) noexcept
{
    traits<DynamicTypeMember>::ref_type member;

    if (RETCODE_OK == type_->get_member(member, id))
    {
        return member->get_descriptor(value);
    }

    return RETCODE_BAD_PARAMETER;
}

bool DynamicDataImpl::equals(
        traits<DynamicData>::ref_type other) noexcept
{
    auto other_data = traits<DynamicData>::narrow<DynamicDataImpl>(other);

    if (type_->equals(other_data->type_))
    {
        TypeKind type_kind = get_enclosing_typekind(type_);
        // Optimization for unions, only check the selected element.
        /*TODO(richiware)
           if (type_kind == TK_UNION)
           {
            if (union_id_ != other.union_id_)
            {
                return false;
            }
            else if (union_id_ != MEMBER_ID_INVALID)
            {
         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = complex_value_.find(union_id_);
                auto otherIt = other.complex_value_.find(union_id_);
         #else
                auto it = value_.find(union_id_);
                auto otherIt = other.value_.find(union_id_);
         #endif // ifdef DYNAMIC_TYPES_CHECKING

                return it->second == otherIt->second ||
         * std::static_pointer_cast<DynamicDataImpl>(it->second) ==
         * std::static_pointer_cast<DynamicDataImpl>(otherIt->second);
            }
           }
           else
         */
        {
            /*TODO(richiware)
               else if (TK_BITMASK == type_kind)
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

                assert(value_.size() && other.value_.size());

                return compare_values(bitmask_kind, value_.begin()->second, other.value_.begin()->second);
               }
             */
            if (TK_ANNOTATION == type_kind ||
                    TK_STRUCTURE == type_kind ||
                    TK_UNION == type_kind)
            {
                // structure, bitset, anotation
                return value_.size() == other_data->value_.size() &&
                       std::equal(
                    value_.begin(),
                    value_.end(),
                    other_data->value_.begin(),
                    [](const decltype(value_)::value_type& l, const decltype(value_)::value_type& r)
                    {
                        return std::static_pointer_cast<DynamicDataImpl>(l.second)->equals(
                            std::static_pointer_cast<DynamicDataImpl>(r.second));
                    });
            }
            else if (TK_ARRAY == type_kind ||
                    TK_SEQUENCE == type_kind)
            {
                TypeKind element_kind =
                        get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    type_->get_descriptor().element_type()));
                return compare_sequence_values(element_kind, value_.begin()->second,
                               other_data->value_.begin()->second);
            }
            else if (TK_BITSET != type_kind &&
                    TK_MAP != type_kind) // primitives
            {
                // primitives
                return compare_values(type_kind, value_.begin()->second, other_data->value_.begin()->second);
            }
        }

        return true;
    }

    return false;
}

MemberId DynamicDataImpl::get_member_id_by_name(
        const ObjectName& name) noexcept
{
    traits<DynamicTypeMember>::ref_type member;

    if (RETCODE_OK == type_->get_member_by_name(member, name))
    {
        return member->get_id();
    }

    return MEMBER_ID_INVALID;
}

MemberId DynamicDataImpl::get_member_id_at_index(
        uint32_t index) noexcept
{
    traits<DynamicTypeMember>::ref_type member;

    if (RETCODE_OK == type_->get_member_by_index(member, index))
    {
        return member->get_id();
    }

    return MEMBER_ID_INVALID;
}

uint32_t DynamicDataImpl::get_item_count() noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);
    if (TK_MAP == type_kind)
    {
        return static_cast<uint32_t>(value_.size() / 2);
    }
    else if (TK_ARRAY == type_kind || TK_SEQUENCE == type_kind)
    {
        return get_sequence_length();
    }
    else if (TK_STRING8 == type_kind)
    {
        assert(1 == value_.size());
        return std::static_pointer_cast<std::string>(value_.begin()->second)->length();
    }
    else if (TK_STRING16 == type_kind)
    {
        assert(1 == value_.size());
        return std::static_pointer_cast<std::wstring>(value_.begin()->second)->length();
    }
    else
    {
        return static_cast<uint32_t>(value_.size());
    }
}

void DynamicDataImpl::set_default_value(
        const traits<DynamicTypeMemberImpl>::ref_type member,
        traits<DynamicDataImpl>::ref_type data) noexcept
{
    traits<DynamicType>::ref_type member_type = member->get_descriptor().type();
    TypeKind member_kind = member_type->get_kind();
    if (TK_BITSET != member_kind &&
            TK_STRUCTURE != member_kind &&
            TK_UNION != member_kind &&
            TK_SEQUENCE != member_kind &&
            TK_ARRAY != member_kind &&
            TK_MAP != member_kind)
    {
        ObjectName def_value;
        if (member->annotation_get_default(def_value))
        {
            data->set_value(def_value, MEMBER_ID_INVALID);
        }
        else if (0 < member->get_descriptor().default_value().length())
        {
            data->set_value(member->get_descriptor().default_value(), MEMBER_ID_INVALID);
        }
    }
}

ReturnCode_t DynamicDataImpl::clear_all_values() noexcept
{
    return clear_all_values(false);
}

ReturnCode_t DynamicDataImpl::clear_nonkey_values() noexcept
{
    return clear_all_values(true);
}

ReturnCode_t DynamicDataImpl::clear_all_sequence() noexcept
{
    ReturnCode_t ret_value = RETCODE_OK;

    assert(TK_SEQUENCE == type_->get_kind());
    TypeKind element_kind =
            get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                        type_->get_descriptor().element_type()));

    switch (element_kind)
    {
        case TK_INT32:
            std::static_pointer_cast<std::vector<int32_t>>(value_.begin()->second)->clear();
            break;
        case TK_UINT32:
            std::static_pointer_cast<std::vector<uint32_t>>(value_.begin()->second)->clear();
            break;
        case TK_INT8:
            std::static_pointer_cast<std::vector<int8_t>>(value_.begin()->second)->clear();
            break;
        case TK_INT16:
            std::static_pointer_cast<std::vector<int16_t>>(value_.begin()->second)->clear();
            break;
        case TK_UINT16:
            std::static_pointer_cast<std::vector<uint16_t>>(value_.begin()->second)->clear();
            break;
        case TK_INT64:
            std::static_pointer_cast<std::vector<int64_t>>(value_.begin()->second)->clear();
            break;
        case TK_UINT64:
            std::static_pointer_cast<std::vector<uint64_t>>(value_.begin()->second)->clear();
            break;
        case TK_FLOAT32:
            std::static_pointer_cast<std::vector<float>>(value_.begin()->second)->clear();
            break;
        case TK_FLOAT64:
            std::static_pointer_cast<std::vector<double>>(value_.begin()->second)->clear();
            break;
        case TK_FLOAT128:
            std::static_pointer_cast<std::vector<long double>>(value_.begin()->second)->clear();
            break;
        case TK_CHAR8:
            std::static_pointer_cast<std::vector<char>>(value_.begin()->second)->clear();
            break;
        case TK_CHAR16:
            std::static_pointer_cast<std::vector<wchar_t>>(value_.begin()->second)->clear();
            break;
        case TK_BOOLEAN:
            std::static_pointer_cast<std::vector<bool>>(value_.begin()->second)->clear();
            break;
        case TK_BYTE:
        case TK_UINT8:
            std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second)->clear();
            break;
        case TK_STRING8:
            std::static_pointer_cast<std::vector<std::string>>(value_.begin()->second)->clear();
            break;
        case TK_STRING16:
            std::static_pointer_cast<std::vector<std::wstring>>(value_.begin()->second)->clear();
            break;
        default:
            ret_value = RETCODE_BAD_PARAMETER;
            break;
    }

    return ret_value;
}

ReturnCode_t DynamicDataImpl::clear_all_values(
        bool only_non_keyed) noexcept
{
    ReturnCode_t ret_val = RETCODE_OK;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_SEQUENCE == type_kind)
    {
        ret_val = clear_all_sequence();
    }
    else if (TK_ANNOTATION == type_kind || TK_STRUCTURE == type_kind || TK_UNION == type_kind)
    {
        const auto& members = type_->get_all_members();
        for (auto& e : value_)
        {
            const auto it = members.find(e.first);
            assert(members.end() != it);
            auto member = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(it->second);
            if (!only_non_keyed || !member->get_descriptor().is_key())
            {
                auto data = std::static_pointer_cast<DynamicDataImpl>(e.second);

                data->clear_all_values();
                set_default_value(member, data);
            }
        }
    }
    else if (TK_BITMASK != type_kind && TK_BITSET != type_kind && TK_MAP != type_kind && TK_ARRAY != type_kind)
    {
        set_value("", MEMBER_ID_INVALID);
    }

    return ret_val;
}

ReturnCode_t DynamicDataImpl::clear_sequence_element(
        MemberId id) noexcept
{
    bool ret_value = RETCODE_BAD_PARAMETER;

    assert(TK_SEQUENCE == type_->get_kind());

    TypeKind element_kind =
            get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                        type_->get_descriptor().element_type()));

    switch (element_kind)
    {
        case TK_INT32:
        {
            auto seq = std::static_pointer_cast<std::vector<int32_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT32:
        {
            auto seq = std::static_pointer_cast<std::vector<uint32_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT8:
        {
            auto seq = std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT16:
        {
            auto seq = std::static_pointer_cast<std::vector<int16_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT16:
        {
            auto seq = std::static_pointer_cast<std::vector<uint16_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT64:
        {
            auto seq = std::static_pointer_cast<std::vector<int64_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT64:
        {
            auto seq = std::static_pointer_cast<std::vector<uint64_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT32:
        {
            auto seq = std::static_pointer_cast<std::vector<float>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT64:
        {
            auto seq = std::static_pointer_cast<std::vector<double>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT128:
        {
            auto seq = std::static_pointer_cast<std::vector<long double>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_CHAR8:
        {
            auto seq = std::static_pointer_cast<std::vector<char>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_CHAR16:
        {
            auto seq = std::static_pointer_cast<std::vector<wchar_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_BOOLEAN:
        {
            auto seq = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto seq = std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_STRING8:
        {
            auto seq = std::static_pointer_cast<std::vector<std::string>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_STRING16:
        {
            auto seq = std::static_pointer_cast<std::vector<std::wstring>>(value_.begin()->second);
            if (seq->size() > id + 1)
            {
                seq->erase(seq->begin() + id);
                ret_value = RETCODE_OK;
            }
            break;
        }
        default:
            ret_value = RETCODE_BAD_PARAMETER;
            break;
    }

    return ret_value;
}

ReturnCode_t DynamicDataImpl::clear_value(
        MemberId id) noexcept
{
    ReturnCode_t ret_val = RETCODE_OK;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_SEQUENCE == type_kind)
    {
        return clear_sequence_element(id);
    }
    else if (TK_ANNOTATION == type_kind || TK_STRUCTURE == type_kind || TK_UNION == type_kind)
    {
        const auto& members = type_->get_all_members();
        auto it_value = value_.find(id);

        if (it_value != value_.end())
        {
            const auto it = members.find(it_value->first);
            assert(members.end() != it);
            auto member = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(it->second);
            auto data = std::static_pointer_cast<DynamicDataImpl>(it_value->second);

            data->clear_all_values();
            set_default_value(member, data);
        }
    }
    else if (TK_BITMASK != type_kind && TK_BITSET != type_kind && TK_MAP != type_kind && TK_ARRAY != type_kind)
    {
        if (MEMBER_ID_INVALID == id)
        {
            set_value("", MEMBER_ID_INVALID);
        }
        else
        {
            ret_val = RETCODE_BAD_PARAMETER;
        }
    }

    return ret_val;
}

traits<DynamicData>::ref_type DynamicDataImpl::loan_value(
        MemberId id) noexcept
{
    if (id != MEMBER_ID_INVALID)
    {
        if (std::find(loaned_values_.begin(), loaned_values_.end(), id) == loaned_values_.end())
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                /*TODO(richiware)
                   if (TK_MAP == type_->get_kind() &&
                        std::static_pointer_cast<DynamicData>(it->second)->key_element_)
                   {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Key values can't be loaned.");
                    return nullptr;
                   }
                   else
                 */
                {
                    auto sp = std::static_pointer_cast<DynamicData>(it->second);

                    /*TODO(richiware)
                       if (get_kind() == TK_UNION && union_id_ != id)
                       {
                        set_union_id(id);
                       }
                     */

                    loaned_values_.push_back(id);
                    return sp;
                }
            }
            else if (TK_ARRAY == type_->get_kind())
            {
                /*TODO(richiware)
                   if (insert_array_data(id) == RETCODE_OK)
                   {
                    auto sp = std::static_pointer_cast<DynamicData>(value_.at(id));

                    loaned_values_.push_back(id);
                    return sp;
                   }
                 */
            }

            else
            {
                //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. MemberId not found.");
            }
        }
        else
        {
            //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. The value has been loaned previously.");
        }
    }
    else
    {
        //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Invalid MemberId.");
    }

    return {};
}

ReturnCode_t DynamicDataImpl::return_loaned_value(
        traits<DynamicData>::ref_type value) noexcept
{
    for (auto loan_it {loaned_values_.begin()}; loan_it != loaned_values_.end(); ++loan_it)
    {
        auto it = value_.find(*loan_it);
        if (it != value_.end() && std::static_pointer_cast<DynamicData>(it->second) == value)
        {
            //TODO(richiware) it->second = DynamicDataFactory::get_instance().create_copy(std::move(value));
            loaned_values_.erase(loan_it);
            return RETCODE_OK;
        }
    }

    //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error returning loaned Value. The value hasn't been loaned.");
    return RETCODE_PRECONDITION_NOT_MET;
}

traits<DynamicData>::ref_type DynamicDataImpl::clone() noexcept
{
    //TODO(richiware)
    return {};
}

ReturnCode_t DynamicDataImpl::get_int32_value(
        int32_t& value,
        MemberId id) noexcept
{
    return get_value<int32_t, TK_INT32, Int32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int32_value(
        MemberId id,
        int32_t value) noexcept
{
    return set_value<int32_t, TK_INT32, Int32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint32_value(
        uint32_t& value,
        MemberId id) noexcept
{
    return get_value<uint32_t, TK_UINT32, UInt32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint32_value(
        MemberId id,
        uint32_t value) noexcept
{
    return set_value<uint32_t, TK_UINT32, UInt32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int8_value(
        int8_t& value,
        MemberId id) noexcept
{
    return get_value<int8_t, TK_INT8, Int8Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int8_value(
        MemberId id,
        int8_t value) noexcept
{
    return set_value<int8_t, TK_INT8, Int8Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint8_value(
        uint8_t& value,
        MemberId id) noexcept
{
    return get_value<uint8_t, TK_UINT8, UInt8Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint8_value(
        MemberId id,
        uint8_t value) noexcept
{
    return set_value<uint8_t, TK_UINT8, UInt8Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int16_value(
        int16_t& value,
        MemberId id) noexcept
{
    return get_value<int16_t, TK_INT16, Int16Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int16_value(
        MemberId id,
        int16_t value) noexcept
{
    return set_value<int16_t, TK_INT16, Int16Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint16_value(
        uint16_t& value,
        MemberId id) noexcept
{
    return get_value<uint16_t, TK_UINT16, UInt16Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint16_value(
        MemberId id,
        uint16_t value) noexcept
{
    return set_value<uint16_t, TK_UINT16, UInt16Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int64_value(
        int64_t& value,
        MemberId id) noexcept
{
    return get_value<int64_t, TK_INT64, Int64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int64_value(
        MemberId id,
        int64_t value) noexcept
{
    return set_value<int64_t, TK_INT64, Int64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint64_value(
        uint64_t& value,
        MemberId id) noexcept
{
    return get_value<uint64_t, TK_UINT64, UInt64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint64_value(
        MemberId id,
        uint64_t value) noexcept
{
    return set_value<uint64_t, TK_UINT64, UInt64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float32_value(
        float& value,
        MemberId id) noexcept
{
    return get_value<float, TK_FLOAT32, Float32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float32_value(
        MemberId id,
        float value) noexcept
{
    return set_value<float, TK_FLOAT32, Float32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float64_value(
        double& value,
        MemberId id) noexcept
{
    return get_value<double, TK_FLOAT64, Float64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float64_value(
        MemberId id,
        double value) noexcept
{
    return set_value<double, TK_FLOAT64, Float64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float128_value(
        long double& value,
        MemberId id) noexcept
{
    return get_value<long double, TK_FLOAT128, Float128Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float128_value(
        MemberId id,
        long double value) noexcept
{
    return set_value<long double, TK_FLOAT128, Float128Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char8_value(
        char& value,
        MemberId id) noexcept
{
    return get_value<char, TK_CHAR8, CharSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char8_value(
        MemberId id,
        char value) noexcept
{
    return set_value<char, TK_CHAR8, CharSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char16_value(
        wchar_t& value,
        MemberId id) noexcept
{
    return get_value<wchar_t, TK_CHAR16, WcharSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char16_value(
        MemberId id,
        wchar_t value) noexcept
{
    return set_value<wchar_t, TK_CHAR16, WcharSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_byte_value(
        eprosima::fastrtps::rtps::octet& value,
        MemberId id) noexcept
{
    return get_value<eprosima::fastrtps::rtps::octet, TK_BYTE, ByteSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_byte_value(
        MemberId id,
        eprosima::fastrtps::rtps::octet value) noexcept
{
    return set_value<eprosima::fastrtps::rtps::octet, TK_BYTE, ByteSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_boolean_value(
        bool& value,
        MemberId id) noexcept
{
    return get_value<bool, TK_BOOLEAN, BooleanSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_boolean_value(
        MemberId id,
        bool value) noexcept
{
    return set_value<bool, TK_BOOLEAN, BooleanSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_string_value(
        std::string& value,
        MemberId id) noexcept
{
    return get_value<std::string, TK_STRING8, StringSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_string_value(
        MemberId id,
        const std::string& value) noexcept
{
    auto type = get_enclosing_type(type_);

    if (TK_STRING8 == type->get_kind())
    {
        auto bound = type->get_descriptor().bound().at(0);
        if (0 != bound && value.length() > bound)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting string value. The given string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }
        return set_value<std::string, TK_STRING8, StringSeq>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_wstring_value(
        std::wstring& value,
        MemberId id) noexcept
{
    return get_value<std::wstring, TK_STRING16, WstringSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_wstring_value(
        MemberId id,
        const std::wstring& value) noexcept
{
    auto type = get_enclosing_type(type_);

    if (TK_STRING16 == type->get_kind())
    {
        auto bound = type->get_descriptor().bound().at(0);
        if (0 != bound && value.length() > bound)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting string value. The given string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }

        return set_value<std::wstring, TK_STRING16, WstringSeq>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_complex_value(
        traits<DynamicData>::ref_type value,
        MemberId id) noexcept
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (MEMBER_ID_INVALID != id &&
            (TK_STRUCTURE == type_->get_kind() ||
            TK_UNION == type_->get_kind() ||
            TK_SEQUENCE == type_->get_kind() ||
            TK_ARRAY == type_->get_kind() ||
            TK_MAP == type_->get_kind() ||
            TK_BITSET == type_->get_kind()))
    {
        auto it = value_.find(id);
        if (it != value_.end())
        {
            value = std::static_pointer_cast<DynamicData>(it->second)->clone();
            return RETCODE_OK;
        }
    }
    else
    {
        //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::set_complex_value(
        MemberId id,
        traits<DynamicData>::ref_type value) noexcept
{
    // Check that the type is complex and in case of dynamic containers, check that the index is valid
    if (MEMBER_ID_INVALID != id && (
                TK_STRUCTURE == type_->get_kind() ||
                TK_UNION  == type_->get_kind() ||
                TK_SEQUENCE == type_->get_kind() ||
                TK_ARRAY == type_->get_kind() ||
                TK_MAP  == type_->get_kind() ||
                TK_BITSET == type_->get_kind()))
    {
        // With containers, check that the index is valid
        if ((TK_SEQUENCE == type_->get_kind() || TK_ARRAY == type_->get_kind() || TK_MAP == type_->get_kind()) /*TODO(richiware)&&
                                                                                                                  id < type_->get_total_bounds()*/)
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                /*TODO(richiware)
                   if TK_MAP == type_->get_kind() &&
                        std::static_pointer_cast<DynamicData>(it->second)->key_element_)
                   {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. They given id is a Key value.");
                    return RETCODE_BAD_PARAMETER;
                   }
                   else
                 */
                {
                    if (it->second)
                    {
                        DynamicDataFactory::get_instance()->delete_data(std::static_pointer_cast<DynamicData>(it
                                        ->second));
                    }
                    value_.erase(it);
                    auto value_copy = value->clone();
                    value_.emplace(id, value_copy);
                    /*TODO(richiware)
                       if (TK_UNION == type_->get_kind() && union_id_ != id)
                       {
                        set_union_id(id);
                       }
                     */
                }
            }
            else if (TK_ARRAY == type_->get_kind())
            {
                auto value_copy = value->clone();
                value_.emplace(id, value_copy);
            }
        }
        else
        {
            //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error setting complex Value. id out of bounds.");
            return RETCODE_BAD_PARAMETER;
        }
        return RETCODE_OK;
    }
    else
    {
        //EPROSIMA_LOG_ERROR(DYN_TYPES, "Error settings complex value. The kind " << get_kind() << "doesn't support it");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicDataImpl::get_int32_values(
        Int32Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<int32_t, TK_INT32, Int32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int32_values(
        MemberId id,
        const Int32Seq& value) noexcept
{
    return set_sequence_values<int32_t, TK_INT32, Int32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint32_values(
        UInt32Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<uint32_t, TK_UINT32, UInt32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint32_values(
        MemberId id,
        const UInt32Seq& value) noexcept
{
    return set_sequence_values<uint32_t, TK_UINT32, UInt32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int8_values(
        Int8Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<int8_t, TK_INT8, Int8Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int8_values(
        MemberId id,
        const Int8Seq& value) noexcept
{
    return set_sequence_values<int8_t, TK_INT8, Int8Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint8_values(
        UInt8Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<uint8_t, TK_UINT8, UInt8Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint8_values(
        MemberId id,
        const UInt8Seq& value) noexcept
{
    return set_sequence_values<uint8_t, TK_UINT8, UInt8Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int16_values(
        Int16Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<int16_t, TK_INT16, Int16Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int16_values(
        MemberId id,
        const Int16Seq& value) noexcept
{
    return set_sequence_values<int16_t, TK_INT16, Int16Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint16_values(
        UInt16Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<uint16_t, TK_UINT16, UInt16Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint16_values(
        MemberId id,
        const UInt16Seq& value) noexcept
{
    return set_sequence_values<uint16_t, TK_UINT16, UInt16Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int64_values(
        Int64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<int64_t, TK_INT64, Int64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int64_values(
        MemberId id,
        const Int64Seq& value) noexcept
{
    return set_sequence_values<int64_t, TK_INT64, Int64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint64_values(
        UInt64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<uint64_t, TK_UINT64, UInt64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint64_values(
        MemberId id,
        const UInt64Seq& value) noexcept
{
    return set_sequence_values<uint64_t, TK_UINT64, UInt64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float32_values(
        Float32Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<float, TK_FLOAT32, Float32Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float32_values(
        MemberId id,
        const Float32Seq& value) noexcept
{
    return set_sequence_values<float, TK_FLOAT32, Float32Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float64_values(
        Float64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<double, TK_FLOAT64, Float64Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float64_values(
        MemberId id,
        const Float64Seq& value) noexcept
{
    return set_sequence_values<double, TK_FLOAT64, Float64Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float128_values(
        Float128Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<long double, TK_FLOAT128, Float128Seq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float128_values(
        MemberId id,
        const Float128Seq& value) noexcept
{
    return set_sequence_values<long double, TK_FLOAT128, Float128Seq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char8_values(
        CharSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<char, TK_CHAR8, CharSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char8_values(
        MemberId id,
        const CharSeq& value) noexcept
{
    return set_sequence_values<char, TK_CHAR8, CharSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char16_values(
        WcharSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<wchar_t, TK_CHAR16, WcharSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char16_values(
        MemberId id,
        const WcharSeq& value) noexcept
{
    return set_sequence_values<wchar_t, TK_CHAR16, WcharSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_byte_values(
        ByteSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<eprosima::fastrtps::rtps::octet, TK_BYTE, ByteSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_byte_values(
        MemberId id,
        const ByteSeq& value) noexcept
{
    return set_sequence_values<eprosima::fastrtps::rtps::octet, TK_BYTE, ByteSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_boolean_values(
        BooleanSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<bool, TK_BOOLEAN, BooleanSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_boolean_values(
        MemberId id,
        const BooleanSeq& value) noexcept
{
    return set_sequence_values<bool, TK_BOOLEAN, BooleanSeq>(id, value);
}

ReturnCode_t DynamicDataImpl::get_string_values(
        StringSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<std::string, TK_STRING8, StringSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_string_values(
        MemberId id,
        const StringSeq& value) noexcept
{
    auto type = get_enclosing_type(type_);

    if (TK_STRING8 == type->get_kind())
    {
        auto bound = type->get_descriptor().bound().at(0);
        if (0 != bound && value.end() != std::find_if(value.begin(), value.end(), [bound](const std::string& str)
                {
                    return str.length() > bound;
                }))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting an array with a string value. The string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }

        return set_sequence_values<std::string, TK_STRING8, StringSeq>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_wstring_values(
        WstringSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<std::wstring, TK_STRING16, WstringSeq>(value, id);
}

ReturnCode_t DynamicDataImpl::set_wstring_values(
        MemberId id,
        const WstringSeq& value) noexcept
{
    auto type = get_enclosing_type(type_);

    if (TK_STRING16 == type->get_kind())
    {
        auto bound = type->get_descriptor().bound().at(0);
        if (0 != bound && value.end() != std::find_if(value.begin(), value.end(), [bound](const std::wstring& str)
                {
                    return str.length() > bound;
                }))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error setting an array with a string value. The string is greater than the length limit.");
            return RETCODE_BAD_PARAMETER;
        }

        return set_sequence_values<std::wstring, TK_STRING16, WstringSeq>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

bool DynamicDataImpl::compare_sequence_values(
        TypeKind kind,
        std::shared_ptr<void> l,
        std::shared_ptr<void> r) const noexcept
{
    void* left = l.get();
    void* right = r.get();

    switch (kind)
    {
        case TK_INT32:      {
            return *((std::vector<int32_t>*)left) == *((std::vector<int32_t>*)right);
        }
        case TK_UINT32:     {
            return *((std::vector<uint32_t>*)left) == *((std::vector<uint32_t>*)right);
        }
        case TK_INT8:      {
            return *((std::vector<int8_t>*)left) == *((std::vector<int8_t>*)right);
        }
        case TK_INT16:      {
            return *((std::vector<int16_t>*)left) == *((std::vector<int16_t>*)right);
        }
        case TK_UINT16:     {
            return *((std::vector<uint16_t>*)left) == *((std::vector<uint16_t>*)right);
        }
        case TK_INT64:      {
            return *((std::vector<int64_t>*)left) == *((std::vector<int64_t>*)right);
        }
        case TK_UINT64:     {
            return *((std::vector<uint64_t>*)left) == *((std::vector<uint64_t>*)right);
        }
        case TK_FLOAT32:    {
            return *((std::vector<float>*)left) == *((std::vector<float>*)right);
        }
        case TK_FLOAT64:    {
            return *((std::vector<double>*)left) == *((std::vector<double>*)right);
        }
        case TK_FLOAT128:   {
            return *((std::vector<long double>*)left) == *((std::vector<long double>*)right);
        }
        case TK_CHAR8:      {
            return *((std::vector<char>*)left) == *((std::vector<char>*)right);
        }
        case TK_CHAR16:     {
            return *((std::vector<wchar_t>*)left) == *((std::vector<wchar_t>*)right);
        }
        case TK_BOOLEAN:    {
            return *((std::vector<bool>*)left) == *((std::vector<bool>*)right);
        }
        case TK_BYTE:
        case TK_UINT8:      {
            return *((std::vector<uint8_t>*)left) == *((std::vector<uint8_t>*)right);
        }
        case TK_STRING8:    {
            return *((std::vector<std::string>*)left) == *((std::vector<std::string>*)right);
        }
        case TK_STRING16:   {
            return *((std::vector<std::wstring>*)left) == *((std::vector<std::wstring>*)right);
        }
        //TODO(richiware) structures unions...
        default:
            break;
    }
    return false;
}

bool DynamicDataImpl::compare_values(
        TypeKind kind,
        std::shared_ptr<void> l,
        std::shared_ptr<void> r) const noexcept
{
    void* left = l.get();
    void* right = r.get();

    switch (kind)
    {
        case TK_INT32:      {
            return *((int32_t*)left) == *((int32_t*)right);
        }
        case TK_UINT32:     {
            return *((uint32_t*)left) == *((uint32_t*)right);
        }
        case TK_INT8:      {
            return *((int8_t*)left) == *((int8_t*)right);
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
        case TK_BYTE:
        case TK_UINT8:      {
            return *((uint8_t*)left) == *((uint8_t*)right);
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
        default:
            break;
    }
    return false;
}

traits<DynamicTypeImpl>::ref_type DynamicDataImpl::get_enclosing_type(
        traits<DynamicTypeImpl>::ref_type type) const noexcept
{
    traits<DynamicTypeImpl>::ref_type ret_value = type;

    if (TK_ENUM == ret_value->get_kind()) // If enum, get enclosing type.
    {
        if (0 == ret_value->get_all_members_by_index().size())
        {
            ret_value = traits<DynamicType>::narrow<DynamicTypeImpl>(
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
        }
        else
        {
            ret_value = traits<DynamicType>::narrow<DynamicTypeImpl>(type->get_all_members_by_index().at(
                                0)->get_descriptor().type());
        }
    }
    else if (TK_ALIAS == ret_value->get_kind()) // If alias, get enclosing type.
    {
        do {
            ret_value = traits<DynamicType>::narrow<DynamicTypeImpl>(ret_value->get_descriptor().base_type());
        } while (TK_ALIAS == ret_value->get_kind());
    }

    return ret_value;
}

TypeKind DynamicDataImpl::get_enclosing_typekind(
        traits<DynamicTypeImpl>::ref_type type) const noexcept
{
    return get_enclosing_type(type)->get_kind();
}

uint32_t DynamicDataImpl::get_sequence_length()
{
    assert(TK_ARRAY == type_->get_kind() || TK_SEQUENCE == type_->get_kind());
    assert(type_->get_descriptor().element_type());
    assert(1 == value_.size());

    size_t ret_value {0};
    TypeKind type_kind =
            get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                        type_->get_descriptor().element_type()));

    switch (type_kind)
    {
        case TK_INT32:
        {
            ret_value =  std::static_pointer_cast<std::vector<int32_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT32:
        {
            ret_value =  std::static_pointer_cast<std::vector<uint32_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT8:
        {
            ret_value =  std::static_pointer_cast<std::vector<int8_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT16:
        {
            ret_value =  std::static_pointer_cast<std::vector<int16_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT16:
        {
            ret_value =  std::static_pointer_cast<std::vector<uint16_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT64:
        {
            ret_value =  std::static_pointer_cast<std::vector<int64_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT64:
        case TK_BITMASK:
        {
            ret_value =  std::static_pointer_cast<std::vector<uint64_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT32:
        {
            ret_value =  std::static_pointer_cast<std::vector<float>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT64:
        {
            ret_value =  std::static_pointer_cast<std::vector<double>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT128:
        {
            ret_value =  std::static_pointer_cast<std::vector<long double>>(value_.begin()->second)->size();
        }
        break;
        case TK_CHAR8:
        {
            ret_value =  std::static_pointer_cast<std::vector<char>>(value_.begin()->second)->size();
        }
        break;
        case TK_CHAR16:
        {
            ret_value =  std::static_pointer_cast<std::vector<wchar_t>>(value_.begin()->second)->size();
        }
        break;
        case TK_BOOLEAN:
        {
            ret_value =  std::static_pointer_cast<std::vector<bool>>(value_.begin()->second)->size();
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            ret_value =
                    std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second)->
                            size();
        }
        break;
        case TK_STRING8:
        {
            ret_value =  std::static_pointer_cast<std::vector<std::string>>(value_.begin()->second)->size();
        }
        break;
        case TK_STRING16:
        {
            ret_value =  std::static_pointer_cast<std::vector<std::wstring>>(value_.begin()->second)->size();
        }
        break;
            break;
        default:
            break;
    }

    return static_cast<uint32_t>(ret_value);
}

template<typename T, int TK, class Sequence>
ReturnCode_t DynamicDataImpl::get_sequence_values(
        Sequence& value,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = type_->get_kind();

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        //TODO(richiware) on union check discriminator.
        auto it = value_.find(id);
        if (it != value_.end())
        {
            ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->get_sequence_values<T, TK, Sequence>(
                value, 0);

            ret_value = RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot find MemberId " << id);
        }

    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        TypeKind element_kind =
                get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                            type_->get_descriptor().element_type()));
        if (MEMBER_ID_INVALID != id && TK == element_kind)
        {
            auto it = value_.cbegin();
            assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
            auto sequence = std::static_pointer_cast<Sequence>(it->second);
            assert(sequence);
            if (sequence->size() > id)
            {
                auto initial_pos = sequence->begin() + id;
                value.clear();
                value.insert(value.begin(), initial_pos, sequence->end());
                return RETCODE_OK;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Element kind is not which expected");
        }
    }
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Unsupported type kind");
    }

    return ret_value;
}

template<typename T, int TK, class Sequence>
ReturnCode_t DynamicDataImpl::get_value(
        T& value,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    auto type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                ret_value =  std::static_pointer_cast<DynamicDataImpl>(it->second)->get_value<T, TK, Sequence>(value,
                                MEMBER_ID_INVALID);
                // TODO(richiware) unions
            }
        }
    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        TypeKind element_kind =
                get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                            type_->get_descriptor().element_type()));
        if (MEMBER_ID_INVALID != id && TK == element_kind)
        {
            auto it = value_.cbegin();
            assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
            auto sequence = std::static_pointer_cast<Sequence>(it->second);
            assert(sequence);
            if (sequence->size() > id)
            {
                value = sequence->at(id);
                return RETCODE_OK;
            }
        }
    }
    else if (TK_BITSET != type_kind &&
            TK_MAP != type_kind) // Primitives
    {
        if (MEMBER_ID_INVALID == id && TK == type_kind)
        {
            assert(1 == value_.size() && MEMBER_ID_INVALID == value_.begin()->first);
            value = *std::static_pointer_cast<T>(value_.begin()->second);
            ret_value =  RETCODE_OK;
        }
    }

    return ret_value;
}

template<typename T, int TK, class Sequence>
ReturnCode_t DynamicDataImpl::set_sequence_values(
        MemberId id,
        const Sequence& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        auto it = value_.find(id);
        if (it != value_.end())
        {
            ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_sequence_values<T, TK, Sequence>(0,
                            value);

            //TODO(richiware) on union set discriminator.
            ret_value = RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot find MemberId " << id);
        }

    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        TypeKind element_kind =
                get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                            type_->get_descriptor().element_type()));
        if (MEMBER_ID_INVALID != id && TK == element_kind)
        {
            auto it = value_.cbegin();
            assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
            auto sequence = std::static_pointer_cast<Sequence>(it->second);
            assert(sequence);
            if ((TK_ARRAY == type_kind && sequence->size() >= id + value.size()) ||
                    (TK_SEQUENCE == type_kind && (0 == type_->get_descriptor().bound().at(0) ||
                    type_->get_descriptor().bound().at(0) >= id + value.size())))
            {
                if (sequence->size() < id + value.size())
                {
                    sequence->resize(id + value.size());
                }
                auto pos = sequence->begin() + id;
                for (size_t count {0}; count < value.size(); ++count)
                {
                    *pos = value.at(count);
                    ++pos;
                }
                return RETCODE_OK;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Element kind is not which expected");
        }
    }
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Unsupported type kind");
    }

    return ret_value;
}

template<typename T, int TK, class Sequence>
ReturnCode_t DynamicDataImpl::set_value(
        MemberId id,
        const T& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_value<T, TK, Sequence>(
                    MEMBER_ID_INVALID, value);
                //TODO(richiware)set_union_id(id);
            }
        }
    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        TypeKind element_kind =
                get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                            type_->get_descriptor().element_type()));
        if (MEMBER_ID_INVALID != id && TK == element_kind)
        {
            auto it = value_.cbegin();
            assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
            auto sequence = std::static_pointer_cast<Sequence>(it->second);
            assert(sequence);
            if ((TK_ARRAY == type_kind && sequence->size() >= id + 1) ||
                    (TK_SEQUENCE == type_kind && (0 == type_->get_descriptor().bound().at(0) ||
                    type_->get_descriptor().bound().at(0) >= id + 1)))
            {
                if (sequence->size() < id + 1)
                {
                    sequence->resize(id + 1);
                }
                auto initial_pos = sequence->begin() + id;
                *initial_pos = value;
                return RETCODE_OK;
            }
        }
    }
    else if (TK_BITSET != type_kind &&
            TK_MAP != type_kind) // Primitives
    {
        if (MEMBER_ID_INVALID == id && TK == type_kind)
        {
            assert(1 == value_.size() && MEMBER_ID_INVALID == value_.begin()->first);
            *std::static_pointer_cast<T>(value_.begin()->second) = value;
            ret_value =  RETCODE_OK;
        }
    }

    return ret_value;
}

void DynamicDataImpl::serialize(
        eprosima::fastcdr::Cdr& cdr) const noexcept
{
    serialize(cdr, type_);
}

void DynamicDataImpl::serialize(
        eprosima::fastcdr::Cdr& cdr,
        const traits<DynamicTypeImpl>::ref_type type) const noexcept
{
    /*TODO(richware)
       if (annotation_is_non_serialized())
       {
        return;
       }
     */
    TypeKind type_kind = get_enclosing_typekind(type);

    switch (type_kind)
    {
        default:
            break;
        case TK_INT32:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<int32_t>(it->second);
            break;
        }
        case TK_UINT32:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<uint32_t>(it->second);
            break;
        }
        case TK_INT8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<int8_t>(it->second);
            break;
        }
        case TK_INT16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<int16_t>(it->second);
            break;
        }
        case TK_UINT16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<uint16_t>(it->second);
            break;
        }
        case TK_INT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<int64_t>(it->second);
            break;
        }
        case TK_UINT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<uint64_t>(it->second);
            break;
        }
        case TK_FLOAT32:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<float>(it->second);
            break;
        }
        case TK_FLOAT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<double>(it->second);
            break;
        }
        case TK_FLOAT128:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<long double>(it->second);
            break;
        }
        case TK_CHAR8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<char>(it->second);
            break;
        }
        case TK_CHAR16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<wchar_t>(it->second);
            break;
        }
        case TK_BOOLEAN:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<bool>(it->second);
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<uint8_t>(it->second);
            break;
        }
        case TK_STRING8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<std::string>(it->second);
            break;
        }
        case TK_STRING16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<std::wstring>(it->second);
            break;
        }
        /*TODO(richiware)
           case TK_BITMASK:
           {
            size_t type_size = get_size();
            auto it = data.values_.begin();
            switch (type_size)
            {
                case 1: cdr << *std::static_pointer_cast<uint8_t>(it->second); break;
                case 2: cdr << *std::static_pointer_cast<uint16_t>(it->second); break;
                case 3: cdr << *std::static_pointer_cast<uint32_t>(it->second); break;
                case 4: cdr << *std::static_pointer_cast<uint64_t>(it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
            }
            break;
           }
           case TK_UNION:
           {
            // The union_id_ must be serialized as a discriminator_type_
            // cdr << data.union_id_;
            serialize_discriminator(data, cdr);

            if (data.union_id_ != MEMBER_ID_INVALID)
            {
         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.at(data.union_id_);
         #else
                auto it = std::static_pointer_cast<DynamicDataImpl>(data.values_.at(data.union_id_));
         #endif // ifdef DYNAMIC_TYPES_CHECKING
                it->serialize(cdr);
            }
            break;
           }
         */
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            eprosima::fastcdr::Cdr::state current_state(cdr);
            cdr.begin_serialize_type(current_state,
                    eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
                    eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2:                     //TODO(richiware) get
                                                                                                //extensibility
                    eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);

            for (auto& member : type->get_all_members_by_index())
            {
                //TODO(richiware) if (!m.annotation_is_non_serialized())
                {
                    auto it = value_.find(member->get_id());

                    if (it != value_.end())
                    {
                        auto member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);

                        cdr << MemberId(member->get_id()) << member_data;
                    }
                }
            }

            cdr.end_serialize_type(current_state);
            break;
        }
        /*
           case TypeKind::TK_ARRAY:
           {
            uint32_t arraySize = get_total_bounds();
            for (uint32_t idx = 0; idx < arraySize; ++idx)
            {
         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(MemberId{idx});
                if (it != data.complex_values_.end())
         #else
                auto it = data.values_.find(MemberId{idx});
                if (it != data.values_.end())
         #endif // ifdef DYNAMIC_TYPES_CHECKING
                {
                    std::static_pointer_cast<DynamicDataImpl>(it->second)->serialize(cdr);
                }
                else
                {
                    get_element_type()->serialize_empty_data(cdr);
                }
            }
            break;
           }
         */
        case TK_SEQUENCE:    // Sequence is like structure, but with size
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    cdr << *std::static_pointer_cast<std::vector<int32_t>>(value_.begin()->second);
                    break;
                case TK_UINT32:
                    cdr << *std::static_pointer_cast<std::vector<uint32_t>>(value_.begin()->second);
                    break;
                case TK_INT8:
                    cdr << *std::static_pointer_cast<std::vector<int8_t>>(value_.begin()->second);
                    break;
                case TK_INT16:
                    cdr << *std::static_pointer_cast<std::vector<int16_t>>(value_.begin()->second);
                    break;
                case TK_UINT16:
                    cdr << *std::static_pointer_cast<std::vector<uint16_t>>(value_.begin()->second);
                    break;
                case TK_INT64:
                    cdr << *std::static_pointer_cast<std::vector<int64_t>>(value_.begin()->second);
                    break;
                case TK_UINT64:
                    cdr << *std::static_pointer_cast<std::vector<uint64_t>>(value_.begin()->second);
                    break;
                case TK_FLOAT32:
                    cdr << *std::static_pointer_cast<std::vector<float>>(value_.begin()->second);
                    break;
                case TK_FLOAT64:
                    cdr << *std::static_pointer_cast<std::vector<double>>(value_.begin()->second);
                    break;
                case TK_FLOAT128:
                    cdr << *std::static_pointer_cast<std::vector<long double>>(value_.begin()->second);
                    break;
                case TK_CHAR8:
                    cdr << *std::static_pointer_cast<std::vector<char>>(value_.begin()->second);
                    break;
                case TK_CHAR16:
                    cdr << *std::static_pointer_cast<std::vector<wchar_t>>(value_.begin()->second);
                    break;
                case TK_BOOLEAN:
                    cdr << *std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr << *std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second);
                    break;
                case TK_STRING8:
                    cdr << *std::static_pointer_cast<std::vector<std::string>>(value_.begin()->second);
                    break;
                case TK_STRING16:
                    cdr << *std::static_pointer_cast<std::vector<std::wstring>>(value_.begin()->second);
                    break;
                default:
                    break;
            }

            break;
        }
        /*TODO(richiware)
           case TypeKind::TK_MAP:
           {
         #ifdef DYNAMIC_TYPES_CHECKING
            cdr << static_cast<uint32_t>(data.complex_values_.size() / 2); // Number of pairs
            for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
            {
                it->second->serialize(cdr);
            }
         #else
            cdr << static_cast<uint32_t>(data.values_.size() / 2);
            for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
            {
                std::static_pointer_cast<DynamicDataImpl>(it->second)->serialize(cdr);
            }
         #endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
           }
         */
        case TK_ALIAS:
            assert(type->get_descriptor().base_type());
            serialize(cdr, traits<DynamicType>::narrow<DynamicTypeImpl>(type->get_descriptor().base_type()));
            break;
    }
}

bool DynamicDataImpl::deserialize(
        eprosima::fastcdr::Cdr& cdr) noexcept
{
    return deserialize(cdr, type_);
}

bool DynamicDataImpl::deserialize(
        eprosima::fastcdr::Cdr& cdr,
        const traits<DynamicTypeImpl>::ref_type type) noexcept
{
    bool res = true;

    /*TODO(richiware)
        if (annotation_is_non_serialized())
        {
            return res;
        }
     */

    TypeKind type_kind = get_enclosing_typekind(type);

    switch (type_kind)
    {
        case TK_INT32:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<int32_t>(it->second);
            break;
        }
        case TK_UINT32:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<uint32_t>(it->second);
            break;
        }
        case TK_INT8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<int8_t>(it->second);
            break;
        }
        case TK_INT16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<int16_t>(it->second);
            break;
        }
        case TK_UINT16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<uint16_t>(it->second);
            break;
        }
        case TK_INT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<int64_t>(it->second);
            break;
        }
        case TK_UINT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<uint64_t>(it->second);
            break;
        }
        case TK_FLOAT32:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<float>(it->second);
            break;
        }
        case TK_FLOAT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<double>(it->second);
            break;
        }
        case TK_FLOAT128:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<long double>(it->second);
            break;
        }
        case TK_CHAR8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<char>(it->second);
            break;
        }
        case TK_CHAR16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<wchar_t>(it->second);
            break;
        }
        case TK_BOOLEAN:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<bool>(it->second);
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<fastrtps::rtps::octet>(it->second);
            break;
        }
        case TK_STRING8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<std::string>(it->second);
            break;
        }
        case TK_STRING16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<std::wstring>(it->second);
            break;
        }
        /*TODO(richiware)
           case TK_BITMASK:
           {
            size_t type_size = get_size();
            auto it = data.values_.begin();
            switch (type_size)
            {
                case 1: cdr >> *std::static_pointer_cast<uint8_t>(it->second); break;
                case 2: cdr >> *std::static_pointer_cast<uint16_t>(it->second); break;
                case 3: cdr >> *std::static_pointer_cast<uint32_t>(it->second); break;
                case 4: cdr >> *std::static_pointer_cast<uint64_t>(it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
            break;
           }
           case TypeKind::TK_UNION:
           {
            // The union_id_ must be deserialized as a discriminator_type_
            // cdr >> data.union_id_;
            deserialize_discriminator(data, cdr);

            if (data.union_id_ != MEMBER_ID_INVALID)
            {

         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(data.union_id_);
                if (it != data.complex_values_.end())
                {
                    it->second->deserialize(cdr);
                }
         #else
                auto it = data.values_.find(data.union_id_);
                if (it != data.values_.end())
                {
                    std::static_pointer_cast<DynamicDataImpl>(it->second)->deserialize(cdr);
                }
         #endif // ifdef DYNAMIC_TYPES_CHECKING
            }
            break;
           }
         */
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
                    eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :     //TODO(richiware) get extensibility
                    eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                    [&](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
                    {
                        bool ret_value = true;

                        traits<DynamicTypeMember>::ref_type member;

                        //TODO(richiware) check if mutable extension and use get_member.
                        if (RETCODE_OK == type_->get_member_by_index(member, mid.id))
                        {
                            auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);
                            traits<DynamicDataImpl>::ref_type member_data;
                            auto it = value_.find(member_impl->get_id());

                            if (it != value_.end())
                            {
                                member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                            }
                            else
                            {
                                member_data =
                                traits<DynamicData>::narrow<DynamicDataImpl>(DynamicDataFactory::get_instance()
                                        ->create_data(
                                    member_impl->get_descriptor().type()));
                                value_.emplace(it->first, member_data);
                            }

                            dcdr >> member_data;
                        }
                        else
                        {
                            ret_value = false;
                        }

                        return ret_value;
                    });
        }
        break;
        case TK_SEQUENCE:    // Sequence is like structure, but with size
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    cdr >> *std::static_pointer_cast<std::vector<int32_t>>(value_.begin()->second);
                    break;
                case TK_UINT32:
                    cdr >> *std::static_pointer_cast<std::vector<uint32_t>>(value_.begin()->second);
                    break;
                case TK_INT8:
                    cdr >> *std::static_pointer_cast<std::vector<int8_t>>(value_.begin()->second);
                    break;
                case TK_INT16:
                    cdr >> *std::static_pointer_cast<std::vector<int16_t>>(value_.begin()->second);
                    break;
                case TK_UINT16:
                    cdr >> *std::static_pointer_cast<std::vector<uint16_t>>(value_.begin()->second);
                    break;
                case TK_INT64:
                    cdr >> *std::static_pointer_cast<std::vector<int64_t>>(value_.begin()->second);
                    break;
                case TK_UINT64:
                    cdr >> *std::static_pointer_cast<std::vector<uint64_t>>(value_.begin()->second);
                    break;
                case TK_FLOAT32:
                    cdr >> *std::static_pointer_cast<std::vector<float>>(value_.begin()->second);
                    break;
                case TK_FLOAT64:
                    cdr >> *std::static_pointer_cast<std::vector<double>>(value_.begin()->second);
                    break;
                case TK_FLOAT128:
                    cdr >> *std::static_pointer_cast<std::vector<long double>>(value_.begin()->second);
                    break;
                case TK_CHAR8:
                    cdr >> *std::static_pointer_cast<std::vector<char>>(value_.begin()->second);
                    break;
                case TK_CHAR16:
                    cdr >> *std::static_pointer_cast<std::vector<wchar_t>>(value_.begin()->second);
                    break;
                case TK_BOOLEAN:
                    cdr >> *std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr >> *std::static_pointer_cast<std::vector<uint8_t>>(value_.begin()->second);
                    break;
                case TK_STRING8:
                    cdr >> *std::static_pointer_cast<std::vector<std::string>>(value_.begin()->second);
                    break;
                case TK_STRING16:
                    cdr >> *std::static_pointer_cast<std::vector<std::wstring>>(value_.begin()->second);
                    break;
                default:
                    break;
            }
            break;
        }
        /*TODO(richiware)
           case TypeKind::TK_ARRAY:
           {
            uint32_t size(get_total_bounds());
            if (size > 0)
            {
                std::shared_ptr<DynamicDataImpl> inputData;
                for (uint32_t i = 0; i < size; ++i)
                {
         #ifdef DYNAMIC_TYPES_CHECKING
                    auto it = data.complex_values_.find(MemberId{i});
                    if (it != data.complex_values_.end())
                    {
                        it->second->deserialize(cdr);
                    }
                    else
                    {
                        if (!inputData)
                        {
                            inputData = DynamicDataFactoryImpl::get_instance().create_data(*get_element_type());
                        }

                        inputData->deserialize(cdr);
                        if (*inputData != *data.default_array_value_)
                        {
                            data.complex_values_.emplace(i, inputData);
                        }
                    }
         #else
                    auto it = data.values_.find(MemberId(i));
                    if (it != data.values_.end())
                    {
                        std::static_pointer_cast<DynamicDataImpl>(it->second)->deserialize(cdr);
                    }
                    else
                    {
                        if (!inputData)
                        {
                            inputData = DynamicDataFactoryImpl::get_instance().create_data(*get_element_type());
                        }

                        inputData->deserialize(cdr);
                        if (inputData != data.default_array_value_)
                        {
                            data.values_.emplace(i, inputData);
                        }
                    }
         #endif // ifdef DYNAMIC_TYPES_CHECKING
                }
                if (inputData)
                {
                    DynamicDataFactoryImpl::get_instance().delete_data(*inputData);
                }
            }
            break;
           }
           case TypeKind::TK_MAP:
           {
            uint32_t size(0);
            bool bKeyElement(false);
            cdr >> size;

            if (get_kind() == TypeKind::TK_MAP)
            {
                size *= 2; // We serialize the number of pairs.
            }
            for (uint32_t i = 0; i < size; ++i)
            {
                //cdr >> memberId;
                if (get_kind() == TypeKind::TK_MAP)
                {
                    bKeyElement = !bKeyElement;
                }

         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(MemberId{i});
                if (it != data.complex_values_.end())
                {
                    it->second->deserialize(cdr);
                    it->second->key_element_ = bKeyElement;
                }
                else
                {
                    std::shared_ptr<DynamicDataImpl> pData;
                    if (bKeyElement)
                    {
                        pData = DynamicDataFactoryImpl::get_instance().create_data(*get_key_element_type());
                    }
                    else
                    {
                        pData = DynamicDataFactoryImpl::get_instance().create_data(*get_element_type());
                    }
                    pData->deserialize(cdr);
                    pData->key_element_ = bKeyElement;
                    data.complex_values_.insert(std::make_pair(i, pData));
                }
         #else
                auto it = data.values_.find(MemberId(i));
                if (it != data.values_.end())
                {
                    std::static_pointer_cast<DynamicDataImpl>(it->second)->deserialize(cdr);
                    std::static_pointer_cast<DynamicDataImpl>(it->second)->key_element_ = bKeyElement;
                }
                else
                {
                    std::shared_ptr<DynamicDataImpl> pData;
                    if (bKeyElement)
                    {
                        pData = DynamicDataFactoryImpl::get_instance().create_data(*get_key_element_type());
                    }
                    else
                    {
                        pData = DynamicDataFactoryImpl::get_instance().create_data(*get_element_type());
                    }
                    pData->deserialize(cdr);
                    pData->key_element_ = bKeyElement;
                    data.values_.insert(std::make_pair(i, pData));
                }
         #endif // ifdef DYNAMIC_TYPES_CHECKING
            }
            break;
           }
         */
        case TK_ALIAS:
            assert(type->get_descriptor().base_type());
            return deserialize(cdr,
                           traits<DynamicType>::narrow<DynamicTypeImpl>(type->get_descriptor().base_type()));
        default:
            break;
    }

    return res;
}

size_t DynamicDataImpl::calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const traits<DynamicTypeImpl>::ref_type type,
        size_t& current_alignment) const noexcept
{
    size_t calculated_size {0};
    auto it = value_.begin();

    /*TODO(richiware)
       if (data.type_ && annotation_is_non_serialized())
       {
        return 0;
       }
     */

    TypeKind type_kind = get_enclosing_typekind(type);

    switch (type_kind)
    {
        case TK_INT32:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<int32_t>(
                                it->second), current_alignment);
            break;
        case TK_UINT32:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<uint32_t>(
                                it->second), current_alignment);
            break;
        case TK_FLOAT32:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<float>(
                                it->second), current_alignment);
            break;
        case TK_CHAR16:     // WCHARS NEED 32 Bits on Linux & MacOS
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<wchar_t>(
                                it->second), current_alignment);
            break;
        case TK_INT8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<int8_t>(
                                it->second), current_alignment);
            break;
        case TK_INT16:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<int16_t>(
                                it->second), current_alignment);
            break;
        case TK_UINT16:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<uint16_t>(
                                it->second), current_alignment);
            break;
        case TK_INT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<int64_t>(
                                it->second), current_alignment);
            break;
        case TK_UINT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<uint64_t>(
                                it->second), current_alignment);
            break;
        case TK_FLOAT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<double>(
                                it->second), current_alignment);
            break;
        /*TODO(richiware)
           case TK_BITMASK:
           {
            size_t type_size = get_size();
            current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
            break;
           }
         */
        case TK_FLOAT128:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<long double>(
                                it->second), current_alignment);
            break;
        case TK_CHAR8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<char>(
                                it->second), current_alignment);
            break;
        case TK_BOOLEAN:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<bool>(
                                it->second), current_alignment);
            break;
        case TK_BYTE:
        case TK_UINT8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<uint8_t>(
                                it->second), current_alignment);
            break;
        case TK_STRING8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<std::string>(
                                it->second), current_alignment);
            break;
        case TK_STRING16:
            calculated_size =
                    calculator.calculate_serialized_size(*std::static_pointer_cast<std::wstring>(
                                it->second),
                            current_alignment);
            break;
        /*TODO(richiware)
           case TypeKind::TK_UNION:
           {
            // Union discriminator
            current_alignment += MemberId::serialized_size + eprosima::fastcdr::Cdr::alignment(current_alignment,
                            MemberId::serialized_size);

            if (data.union_id_ != MEMBER_ID_INVALID)
            {
         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.at(data.union_id_);
         #else
                auto it = std::static_pointer_cast<DynamicDataImpl>(data.values_.at(data.union_id_));
         #endif // ifdef DYNAMIC_TYPES_CHECKING

                current_alignment +=
                        get_member(data.union_id_).get_type()->getCdrSerializedSize(*it, current_alignment);
            }
            break;
           }
         */
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
            calculated_size = calculator.begin_calculate_type_serialized_size(
                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2:                         //TODO(richiware) get
                                                                                                //extensibility
                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                current_alignment);

            for (auto& member : type->get_all_members_by_index())
            {
                //TODO(richiware) if (!m.annotation_is_non_serialized())
                {
                    it = value_.find(member->get_id());

                    if (it != value_.end())
                    {
                        auto member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                        calculated_size += calculator.calculate_member_serialized_size(
                            member->get_id(), member_data, current_alignment);
                    }
                }
            }

            calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);
            break;
        }
        case TK_SEQUENCE:    // Sequence is like structure, but with size
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<int32_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<uint32_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<int8_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<int16_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<uint16_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<int64_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<uint64_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<float>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<double>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT128:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<long double>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_CHAR8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<char>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_CHAR16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<wchar_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_BOOLEAN:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<bool>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<uint8_t>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_STRING8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<std::string>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_STRING16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<std::vector<std::wstring>>(
                                        value_.begin()->second), current_alignment);
                    break;
                default:
                    break;
            }
            break;
        }
        /*TODO(richiware)
           case TypeKind::TK_ARRAY:
           {
            assert(element_type_);

            uint32_t arraySize = get_total_bounds();
            size_t emptyElementSize =
                    get_element_type()->getEmptyCdrSerializedSize(current_alignment);
            for (uint32_t idx = 0; idx < arraySize; ++idx)
            {
         #ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(MemberId{idx});
                if (it != data.complex_values_.end())
         #else
                auto it = data.values_.find(MemberId{idx});
                if (it != data.values_.end())
         #endif // ifdef DYNAMIC_TYPES_CHECKING
                {
                    // Element Size
                    current_alignment += element_type_->getCdrSerializedSize(
         * std::static_pointer_cast<DynamicDataImpl>(it->second),
                        current_alignment);
                }
                else
                {
                    current_alignment += emptyElementSize;
                }
            }
            break;
           }
           case TypeKind::TK_MAP:
           {
            assert(element_type_);
            // Elements count
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            // on maps some nodes are keys and other values, that that into account
         #ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
            {
                // Key Size
                current_alignment += key_element_type_->getCdrSerializedSize(*it++->second, current_alignment);
                // Element Size
                current_alignment += element_type_->getCdrSerializedSize(*it->second, current_alignment);
            }
         #else
            for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
            {
                // Key Size
                current_alignment += key_element_type_->getCdrSerializedSize(
         * std::static_pointer_cast<DynamicDataImpl>(it++->second),
                    current_alignment);
                // Element Size
                current_alignment += element_type_->getCdrSerializedSize(
         * std::static_pointer_cast<DynamicDataImpl>(it->second),
                    current_alignment);
            }
         #endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
           }
         */
        case TK_ALIAS:
            assert(type->get_descriptor().base_type());
            calculated_size = calculate_serialized_size(
                calculator,
                traits<DynamicType>::narrow<DynamicTypeImpl>(type->get_descriptor().base_type()),
                current_alignment);
            break;
        default:
            break;
    }

    return calculated_size;
}

void DynamicDataImpl::serialize_key(
        eprosima::fastcdr::Cdr& cdr) const noexcept
{
}

size_t DynamicDataImpl::get_key_max_cdr_serialized_size(
        traits<DynamicType>::ref_type type,
        size_t current_alignment)
{
    return 0;
}

size_t DynamicDataImpl::get_max_cdr_serialized_size(
        traits<DynamicType>::ref_type type,
        size_t current_alignment)
{
    return 0;
}

/*





   void DynamicDataImpl::clean()
   {
    // keep the type alive
    auto type = get_type().shared_from_this();
    // reset all
 * this = DynamicDataImpl(use_the_create_method{}, *type);
   }


   void DynamicDataImpl::clean_members()
   {
 #ifdef DYNAMIC_TYPES_CHECKING
    complex_value_.clear();
 #else
    value_.clear();
 #endif // ifdef DYNAMIC_TYPES_CHECKING
   }

   ReturnCode_t DynamicDataImpl::clear_nonkey_values()
   {
    if (type_->is_complex_kind())
    {
 #ifdef DYNAMIC_TYPES_CHECKING
        for (auto& e : complex_value_)
        {
            e.second->clear_nonkey_values();
        }
 #else
        for (auto& e : value_)
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


   bool DynamicDataImpl::compare_values(
        TypeKind kind,
        std::shared_ptr<void> left,
        std::shared_ptr<void> right) const
   {
    return compare_values(kind, left.get(), right.get());
   }


   std::string DynamicDataImpl::get_value(
        MemberId id) const
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



   std::shared_ptr<DynamicDataImpl> DynamicDataImpl::loan_value(
        MemberId id)
   {
    if (id != MEMBER_ID_INVALID)
    {
        if (std::find(loaned_value_.begin(), loaned_value_.end(), id) == loaned_value_.end())
        {
 #ifdef DYNAMIC_TYPES_CHECKING
            auto it = complex_value_.find(id);
            if (it != complex_value_.end())
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
                    loaned_value_.push_back(id);
                    it->second->lender_ = weak_from_this();
                    return it->second;
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                if (insert_array_data(id) == RETCODE_OK)
                {
                    loaned_value_.push_back(id);
                    auto& sp = complex_value_.at(id);
                    sp->lender_ = weak_from_this();
                    return sp;
                }
            }
 #else
            auto it = value_.find(id);
            if (it != value_.end())
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

                    loaned_value_.push_back(id);
                    sp->lender_ = weak_from_this();
                    return sp;
                }
            }
            else if (get_kind() == TK_ARRAY)
            {
                if (insert_array_data(id) == RETCODE_OK)
                {
                    auto sp = std::static_pointer_cast<DynamicDataImpl>(value_.at(id));

                    loaned_value_.push_back(id);
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto it = value_.find(id);
    if (it != value_.end())
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
        {
            if (get_kind() == TK_BITSET )
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
    auto it = value_.find(id);
    if (it != value_.end())
    {
        if (get_kind() == TK_UINT32 && id == MEMBER_ID_INVALID)
        {
 * std::static_pointer_cast<uint32_t>(it->second) = value;
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto it = value_.find(id);
    if (it != value_.end())
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto it = value_.find(id);
    if (it != value_.end())
    {
        if (get_kind() == TK_INT16 && id == MEMBER_ID_INVALID)
        {
 * std::static_pointer_cast<int16_t>(it->second) = value;
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto it = value_.find(id);
    if (it != value_.end())
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto itValue = value_.find(id);
    if (itValue != value_.end())
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
        MemberId id)
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto itValue = value_.find(id);
    if (itValue != value_.end())
    {
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {

            if (!type_->exists_member_by_id(MemberId(value)))
            {
                return RETCODE_BAD_PARAMETER;
            }

 * std::static_pointer_cast<uint32_t>(itValue->second) = value;
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto itValue = value_.find(id);
    if (itValue != value_.end())
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
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
    auto itValue = value_.find(id);
    if (itValue != value_.end())
    {
        if (get_kind() == TK_ENUM && id == MEMBER_ID_INVALID)
        {
            auto mid = get_member_id_by_name(value);
            if (mid == MEMBER_ID_INVALID)
            {
                return RETCODE_BAD_PARAMETER;
            }

 * std::static_pointer_cast<uint32_t>(itValue->second) = mid;
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
            complex_value_.emplace(indexId, value);
            return RETCODE_OK;
        }
 #else
        if (indexId < type_->get_total_bounds())
        {
            auto value = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            value_.emplace(indexId, value);
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
            auto it = complex_value_.find(indexId);
            if (it != complex_value_.end())
            {
                complex_value_.erase(it);
            }
            return RETCODE_OK;
        }
 #else
        if (indexId < type_->get_total_bounds())
        {
            auto it = value_.find(indexId);
            if (it != value_.end())
            {
                value_.erase(it);
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
            outId = complex_value_.size();
            complex_value_.emplace(outId, DynamicDataFactoryImpl::get_instance().create_copy(value));
 #else
            outId = value_.size();
            value_.emplace(outId, DynamicDataFactoryImpl::get_instance().create_copy(value));
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
            outId = complex_value_.size();
            complex_value_.emplace(outId, new_element);
 #else
            outId = value_.size();
            value_.emplace(outId, new_element);
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
        auto it = complex_value_.find(id);
        if (it != complex_value_.end())
        {
            complex_value_.erase(it);
            return RETCODE_OK;
        }
 #else
        auto it = value_.find(id);
        if (it != value_.end())
        {
            value_.erase(it);
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
            for (auto it = complex_value_.begin(); it != complex_value_.end(); ++it)
            {
                if (it->second->key_element_ && *it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = complex_value_.size();
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            complex_value_.emplace(outKeyId, keyCopy);

            auto new_element = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            outValueId = complex_value_.size();
            complex_value_.emplace(outValueId, new_element);
 #else
            for (auto it = value_.begin(); it != value_.end(); ++it)
            {
                auto data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                if (data->key_element_ && *data == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            outKeyId = value_.size();
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            value_.emplace(outKeyId, keyCopy);

            auto new_element = DynamicDataFactoryImpl::get_instance().create_data(*type_->get_element_type());
            outValueId = value_.size();
            value_.emplace(outValueId, new_element);
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
 * type_->get_key_element_type() == *key.type_ &&
 * type_->get_element_type() == *value.type_)
    {
        if (type_->get_bounds() == BOUND_UNLIMITED || get_item_count() < type_->get_bounds())
        {
 #ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = complex_value_.begin(); it != complex_value_.end(); ++it)
            {
                if (it->second->key_element_ && *it->second == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (complex_value_.size())
            {
                // get largest key available
                outKey = complex_value_.rbegin()->first + 1u;
            }

            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            complex_value_.emplace(outKey, keyCopy);

            outValue = outKey + 1u;
            auto valueCopy = DynamicDataFactoryImpl::get_instance().create_copy(value);
            complex_value_.emplace(outValue, valueCopy);
 #else
            for (auto it = value_.begin(); it != value_.end(); ++it)
            {
                if (*std::static_pointer_cast<DynamicDataImpl>(it->second) == key)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error inserting to map. The key already exists.");
                    return RETCODE_BAD_PARAMETER;
                }
            }

            outKey = 0u;
            if (value_.size())
            {
                // get largest key available
                outKey = value_.rbegin()->first + 1u;
            }
            auto keyCopy = DynamicDataFactoryImpl::get_instance().create_copy(key);
            keyCopy->key_element_ = true;
            value_.emplace(outKey, keyCopy);

            outValue = outKey + 1u;
            auto valueCopy = DynamicDataFactoryImpl::get_instance().create_copy(value);
            value_.emplace(outValue, valueCopy);
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
        auto itKey = complex_value_.find(keyId);
        auto itValue = complex_value_.find(keyId + 1);
        if (itKey != complex_value_.end() && itValue != complex_value_.end() && itKey->second->key_element_)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*itKey->second);
            DynamicDataFactoryImpl::get_instance().delete_data(*itValue->second);
            complex_value_.erase(itKey);
            complex_value_.erase(itValue);
            return RETCODE_OK;
        }
 #else
        auto itKey = value_.find(keyId);
        auto itValue = value_.find(keyId + 1);
        if (itKey != value_.end() && itValue != value_.end() &&
                std::static_pointer_cast<DynamicDataImpl>(itKey->second)->key_element_)
        {
            DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(itKey->second));
            DynamicDataFactoryImpl::get_instance().delete_data(*std::static_pointer_cast<DynamicDataImpl>(itValue->
                            second));
            value_.erase(itKey);
            value_.erase(itValue);
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
       auto& labels = type_->get_member(union_id_).label();
       auto it = labels.cbegin();
       if (it != labels.cend())
       {
        return *it;
       }

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

   size_t DynamicDataImpl::getKeyMaxCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment)
   {
    return type.getKeyMaxCdrSerializedSize(current_alignment);
   }

   size_t DynamicDataImpl::getMaxCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment)
   {
    return type.getMaxCdrSerializedSize(current_alignment);
   }

   void DynamicDataImpl::serializeKey(
        eprosima::fastcdr::Cdr& cdr) const
   {
    assert(type_);
    type_->serializeKey(*this, cdr);
   }

   size_t DynamicDataImpl::getEmptyCdrSerializedSize(
        const DynamicTypeImpl& type,
        size_t current_alignment)
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

 */

} // namespace dds

} // namespace fastdds
} // namespace eprosima
