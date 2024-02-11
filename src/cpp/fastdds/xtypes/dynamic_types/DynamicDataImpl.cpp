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
#include <numeric>
#include <string>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include "DynamicTypeMemberImpl.hpp"
#include "TypeValueConverter.hpp"

namespace eprosima {

namespace fastcdr {

template<>
FASTDDS_EXPORTED_API size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const fastdds::dds::traits<fastdds::dds::DynamicDataImpl>::ref_type& data,
        size_t& current_alignment)
{
    return data->calculate_serialized_size(calculator, current_alignment);
}

template<>
FASTDDS_EXPORTED_API void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const fastdds::dds::traits<fastdds::dds::DynamicDataImpl>::ref_type& data)
{
    return data->serialize(scdr);
}

template<>
FASTDDS_EXPORTED_API void deserialize(
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
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        uint32_t index = 0;
        for (auto& member : type_->get_all_members_by_index())
        {
            traits<DynamicData>::ref_type data = DynamicDataFactory::get_instance()->create_data(
                member->get_descriptor().type());
            traits<DynamicDataImpl>::ref_type data_impl = traits<DynamicData>::narrow<DynamicDataImpl>(data);

            if (TK_UNION == type_kind && 0 == index)
            {
                // Set default discriminator value.
                set_discriminator_value(type_->default_discriminator_value(),
                        traits<DynamicType>::narrow<DynamicTypeImpl>(member->get_descriptor().type()),
                        data_impl);
                selected_union_member_ = type_->default_union_member();
            }
            else
            {
                set_default_value(member, data_impl);

            }

            value_.emplace(member->get_id(), data);
            ++index;
        }
    }
    else if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        uint32_t sequence_size {0};

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

        auto sequence_type = get_enclosing_type(
            traits<DynamicType>::narrow<DynamicTypeImpl>(type_->get_descriptor().element_type()));

        add_sequence_value(sequence_type, sequence_size);
    }
    else if (TK_BITMASK == type_kind)
    {
        assert(1 == type_->get_descriptor().bound().size());
        value_.emplace(MEMBER_ID_INVALID,
                std::make_shared<std::vector<bool>>(type_->get_descriptor().bound().at(0), false));
    }
    else if (TK_MAP != type_kind)     // Primitives
    {
        add_value(type_kind, MEMBER_ID_INVALID);
    }
}

void DynamicDataImpl::add_sequence_value(
        const traits<DynamicTypeImpl>::ref_type& sequence_type,
        uint32_t sequence_size) noexcept
{
    if (is_complex_kind(sequence_type->get_kind()))
    {
        value_.emplace(MEMBER_ID_INVALID,
                std::make_shared<std::vector<traits<DynamicDataImpl>::ref_type>>(sequence_size));
        auto vector = std::static_pointer_cast <std::vector<traits<DynamicDataImpl>::ref_type >> (
            value_.begin()->second);
        for (uint32_t pos = 0; pos < sequence_size; ++pos)
        {
            if (!vector->at(pos))
            {
                vector->at(pos) = traits<DynamicData>::narrow<DynamicDataImpl>(
                    DynamicDataFactory::get_instance()->create_data(sequence_type));
            }
        }
    }
    else
    {
        switch (sequence_type->get_kind())
        {
            case TK_INT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_INT32>>(sequence_size));
            }
            break;
            case TK_UINT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_UINT32>>(sequence_size));
            }
            break;
            case TK_INT8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_INT8>>(sequence_size));
            }
            break;
            case TK_INT16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_INT16>>(sequence_size));
            }
            break;
            case TK_UINT16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_UINT16>>(sequence_size));
            }
            break;
            case TK_INT64:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_INT64>>(sequence_size));
            }
            break;
            case TK_UINT64:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_UINT64>>(sequence_size));
            }
            break;
            case TK_FLOAT32:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_FLOAT32>>(sequence_size));
            }
            break;
            case TK_FLOAT64:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_FLOAT64>>(sequence_size));
            }
            break;
            case TK_FLOAT128:
            {
                value_.emplace(MEMBER_ID_INVALID,
                        std::make_shared<SequenceTypeForKind<TK_FLOAT128>>(sequence_size));
            }
            break;
            case TK_CHAR8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_CHAR8>>(sequence_size));
            }
            break;
            case TK_CHAR16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_CHAR16>>(sequence_size));
            }
            break;
            case TK_BOOLEAN:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_BOOLEAN>>(sequence_size));
            }
            break;
            case TK_BYTE:
            case TK_UINT8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_UINT8>>(sequence_size));
            }
            break;
            case TK_STRING8:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_STRING8>>(sequence_size));
            }
            break;
            case TK_STRING16:
            {
                value_.emplace(MEMBER_ID_INVALID, std::make_shared<SequenceTypeForKind<TK_STRING16>>(sequence_size));
            }
            break;
            default:
                break;
        }
    }
}

std::map<MemberId, std::shared_ptr<void>>::iterator DynamicDataImpl::add_value(
        TypeKind kind,
        MemberId id) noexcept
{
    std::map<MemberId, std::shared_ptr<void>>::iterator ret_value {value_.end()};

    switch (kind)
    {
        case TK_INT32:
        {
            ret_value =  value_.emplace(id, std::make_shared<TypeForKind<TK_INT32>>()).first;
        }
        break;
        case TK_UINT32:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_UINT32>>()).first;
        }
        break;
        case TK_INT8:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_INT8>>()).first;
        }
        break;
        case TK_INT16:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_INT16>>()).first;
        }
        break;
        case TK_UINT16:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_UINT16>>()).first;
        }
        break;
        case TK_INT64:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_INT64>>()).first;
        }
        break;
        case TK_UINT64:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_UINT64>>()).first;
        }
        break;
        case TK_FLOAT32:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_FLOAT32>>()).first;
        }
        break;
        case TK_FLOAT64:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_FLOAT64>>()).first;
        }
        break;
        case TK_FLOAT128:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_FLOAT128>>()).first;
        }
        break;
        case TK_CHAR8:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_CHAR8>>()).first;
        }
        break;
        case TK_CHAR16:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_CHAR16>>()).first;
        }
        break;
        case TK_BOOLEAN:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_BOOLEAN>>()).first;
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_UINT8>>()).first;
        }
        break;
        case TK_STRING8:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_STRING8>>()).first;
        }
        break;
        case TK_STRING16:
        {
            ret_value = value_.emplace(id, std::make_shared<TypeForKind<TK_STRING16>>()).first;
        }
        break;
        default:
            break;
    }

    return ret_value;
}

void DynamicDataImpl::set_value(
        const ObjectName& sValue) noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    switch (type_kind)
    {
        case TK_INT32:
        {
            TypeForKind<TK_INT32> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int32_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_UINT32:
        {
            TypeForKind<TK_UINT32> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_uint32_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_INT8:
        {
            TypeForKind<TK_INT8> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int8_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_INT16:
        {
            TypeForKind<TK_INT16> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int16_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_UINT16:
        {
            TypeForKind<TK_UINT16> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_uint16_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_INT64:
        {
            TypeForKind<TK_INT64> value {0};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_int64_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_UINT64:
        {
            TypeForKind<TK_UINT64> value(0);
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_uint64_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_FLOAT32:
        {
            TypeForKind<TK_FLOAT32> value {0.0f};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float32_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_FLOAT64:
        {
            TypeForKind<TK_FLOAT64> value {0.0f};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float64_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_FLOAT128:
        {
            TypeForKind<TK_FLOAT128> value {0.0f};
            try
            {
                value = TypeValueConverter::sto(sValue.to_string());
            }
            catch (...)
            {
            }
            set_float128_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_CHAR8:
        {
            TypeForKind<TK_CHAR8> value {0};
            if (sValue.size() >= 1)
            {
                value = sValue[0];
            }

            set_char8_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_CHAR16:
        {
            TypeForKind<TK_CHAR16> value {0};
            try
            {
                std::string str = sValue.to_string();
                std::wstring temp = std::wstring(str.begin(), str.end());
                value = temp[0];
            }
            catch (...)
            {
            }

            set_char16_value(MEMBER_ID_INVALID, value);
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
            set_boolean_value(MEMBER_ID_INVALID, value == 0 ? false : true);
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            TypeForKind<TK_BYTE> value {0};
            if (sValue.size() >= 1)
            {
                try
                {
                    value = TypeValueConverter::sto(sValue.to_string());
                }
                catch (...)
                {
                }
            }
            set_byte_value(MEMBER_ID_INVALID, value);
        }
        break;
        case TK_STRING8:
        {
            set_string_value(MEMBER_ID_INVALID, sValue.to_string());
        }
        break;
        case TK_STRING16:
        {
            std::string str = sValue.to_string();
            set_wstring_value(MEMBER_ID_INVALID, std::wstring(str.begin(), str.end()));
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

        if (TK_ANNOTATION == type_kind ||
                TK_BITSET == type_kind ||
                TK_STRUCTURE == type_kind)
        {
            return value_.size() == other_data->value_.size() &&
                   std::equal(
                value_.begin(),
                value_.end(),
                other_data->value_.begin(),
                [](const decltype(value_)::value_type& l, const decltype(value_)::value_type& r)
                {
                    return l.first == r.first && std::static_pointer_cast<DynamicDataImpl>(l.second)->equals(
                        std::static_pointer_cast<DynamicDataImpl>(r.second));
                });
        }
        else if (TK_UNION == type_kind)
        {
            return std::static_pointer_cast<DynamicDataImpl>(value_.at(0))->equals(
                std::static_pointer_cast<DynamicDataImpl>(other_data->value_.at(0))) &&
                   (MEMBER_ID_INVALID == selected_union_member_ ||
                   std::static_pointer_cast<DynamicDataImpl>(value_.at(selected_union_member_))->equals(
                       std::static_pointer_cast<DynamicDataImpl>(other_data->value_.at(selected_union_member_))));
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
        else if (TK_MAP == type_kind)
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            assert(key_to_id_.size() == value_.size() && other_data->key_to_id_.size() == other_data->value_.size());

            return key_to_id_.size() == other_data->key_to_id_.size() &&
                   std::equal(
                key_to_id_.begin(),
                key_to_id_.end(),
                other_data->key_to_id_.begin(),
                [](const decltype(key_to_id_)::value_type& l, const decltype(key_to_id_)::value_type& r)
                {
                    return 0 == l.first.compare(r.first) && l.second == r.second;
                }) &&
                   value_.size() == other_data->value_.size() &&
                   std::equal(
                value_.begin(),
                value_.end(),
                other_data->value_.begin(),
                [&, element_kind](const decltype(value_)::value_type& l, const decltype(value_)::value_type& r)
                {
                    if (is_complex_kind(element_kind))
                    {
                        return std::static_pointer_cast<DynamicDataImpl>(l.second)->equals(
                            std::static_pointer_cast<DynamicDataImpl>(r.second));
                    }

                    return compare_values(element_kind, l.second, r.second);
                });
        }
        else if (TK_BITMASK == type_kind)
        {
            return *(std::static_pointer_cast<std::vector<bool>>(value_.begin()->second)) ==
                   *(std::static_pointer_cast<std::vector<bool>>(other_data->value_.begin()->second));
        }
        else // primitives
        {
            // primitives
            return compare_values(type_kind, value_.begin()->second, other_data->value_.begin()->second);
        }
    }

    return false;
}

MemberId DynamicDataImpl::get_member_id_by_name(
        const ObjectName& name) noexcept
{
    MemberId ret_value {MEMBER_ID_INVALID};
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        traits<DynamicTypeMember>::ref_type member;
        if (RETCODE_OK == type_->get_member_by_name(member, name))
        {
            ret_value =  member->get_id();
        }
    }
    else if (TK_BITMASK == type_kind)
    {
        traits<DynamicTypeMember>::ref_type member;
        if (RETCODE_OK == type_->get_member_by_name(member, name))
        {
            ret_value =  traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member)->get_descriptor().index();
        }
    }
    else if (TK_MAP == type_kind)
    {
        assert(type_->get_descriptor().key_element_type());
        TypeKind key_kind = get_enclosing_typekind(
            traits<DynamicType>::narrow<DynamicTypeImpl>(type_->get_descriptor().key_element_type()));
        if (TypeValueConverter::is_string_consistent(key_kind, name.to_string()))
        {
            auto it = key_to_id_.find(name.to_string());

            if (key_to_id_.end() == it)
            {
                if (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_->get_descriptor().bound().at(0) ||
                        type_->get_descriptor().bound().at(0) > key_to_id_.size())
                {
                    ret_value = next_map_member_id_++;
                    key_to_id_[name.to_string()] = ret_value;

                    TypeKind element_kind =
                            get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(type_->get_descriptor()
                                            .element_type()));

                    if (!is_complex_kind(element_kind))
                    {
                        add_value(element_kind, ret_value);
                    }
                    else
                    {
                        traits<DynamicData>::ref_type data = DynamicDataFactory::get_instance()->create_data(
                            type_->get_descriptor().element_type());
                        value_.emplace(ret_value, data);
                    }
                }
            }
            else
            {
                ret_value = it->second;
            }
        }
    }

    return ret_value;
}

MemberId DynamicDataImpl::get_member_id_at_index(
        uint32_t index) noexcept
{
    MemberId ret_value {MEMBER_ID_INVALID};
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        traits<DynamicTypeMember>::ref_type member;
        if (RETCODE_OK == type_->get_member_by_index(member, index))
        {
            ret_value = member->get_id();
        }
    }
    else if (TK_ARRAY == type_kind ||
            TK_BITMASK == type_kind ||
            TK_SEQUENCE == type_kind ||
            TK_STRING8 == type_kind ||
            TK_STRING16 == type_kind)
    {
        assert(1 == value_.size());
        if (index < get_item_count())
        {
            return index;
        }
    }

    return ret_value;
}

uint32_t DynamicDataImpl::get_item_count() noexcept
{
    uint32_t ret_value {0};

    TypeKind type_kind = get_enclosing_typekind(type_);
    if (TK_ARRAY == type_kind || TK_SEQUENCE == type_kind)
    {
        ret_value =  get_sequence_length();
    }
    else if (TK_STRING8 == type_kind)
    {
        assert(1 == value_.size());
        ret_value =  std::static_pointer_cast<TypeForKind<TK_STRING8>>(value_.begin()->second)->length();
    }
    else if (TK_STRING16 == type_kind)
    {
        assert(1 == value_.size());
        ret_value =  std::static_pointer_cast<TypeForKind<TK_STRING16>>(value_.begin()->second)->length();
    }
    else if (TK_UNION == type_kind)
    {
        ret_value =  1 + (MEMBER_ID_INVALID == selected_union_member_ ? 0 : 1);
    }
    else if (TK_BITMASK == type_kind)
    {
        ret_value =  std::static_pointer_cast<std::vector<bool>>(value_.begin()->second)->size();
    }
    else
    {
        ret_value = static_cast<uint32_t>(value_.size());
    }

    return ret_value;
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
            data->set_value(def_value);
        }
        else if (0 < member->get_descriptor().default_value().length())
        {
            data->set_value(member->get_descriptor().default_value());
        }
    }
}

ReturnCode_t DynamicDataImpl::clear_all_sequence(
        TypeKind type_kind) noexcept
{
    ReturnCode_t ret_value = RETCODE_OK;

    assert(TK_ARRAY == type_kind || TK_SEQUENCE == type_kind);
    auto element_type =
            get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                        type_->get_descriptor().element_type()));

    uint32_t sequence_size {0};

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

    switch (element_type->get_kind())
    {
        case TK_INT32:
            std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_UINT32:
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_INT8:
            std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_INT16:
            std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_UINT16:
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_INT64:
            std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_UINT64:
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_FLOAT32:
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_FLOAT64:
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_FLOAT128:
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_CHAR8:
            std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_CHAR16:
            std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_BOOLEAN:
            std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_BYTE:
        case TK_UINT8:
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_STRING8:
            std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second)->resize(sequence_size);
            break;
        case TK_STRING16:
            std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second)->clear();
            std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second)->resize(sequence_size);
            break;
        default:
            auto vector = std::static_pointer_cast <std::vector<traits<DynamicDataImpl>::ref_type >> (
                value_.begin()->second);
            vector->clear();
            vector->resize(sequence_size);
            for (uint32_t pos = 0; pos < sequence_size; ++pos)
            {
                if (!vector->at(pos))
                {
                    vector->at(pos) = traits<DynamicData>::narrow<DynamicDataImpl>(
                        DynamicDataFactory::get_instance()->create_data(element_type));
                }
            }
            break;
    }

    return ret_value;
}

ReturnCode_t DynamicDataImpl::clear_all_values() noexcept
{
    return clear_all_values(false);
}

ReturnCode_t DynamicDataImpl::clear_nonkey_values() noexcept
{
    return clear_all_values(true);
}

ReturnCode_t DynamicDataImpl::clear_all_values(
        bool only_non_keyed) noexcept
{
    ReturnCode_t ret_val = RETCODE_OK;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ARRAY == type_kind ||
            TK_SEQUENCE == type_kind)
    {
        ret_val = clear_all_sequence(type_kind);
    }
    else if (TK_MAP == type_kind)
    {
        next_map_member_id_ = 0;
        key_to_id_.clear();
        value_.clear();
    }
    else if (TK_BITMASK == type_kind)
    {
        assert(1 == value_.size());
        assert(1 == type_->get_descriptor().bound().size());
        auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
        assert(sequence->size() == type_->get_descriptor().bound().at(0));
        sequence->assign(sequence->size(), false);
    }
    else if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
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

                if (TK_UNION == type_kind && 0 == e.first)
                {
                    // Set default discriminator value.
                    set_discriminator_value(type_->default_discriminator_value(),
                            traits<DynamicType>::narrow<DynamicTypeImpl>(member->get_descriptor().type()),
                            data);
                    selected_union_member_ = type_->default_union_member();
                }
                else
                {
                    set_default_value(member, data);
                }
            }
        }
    }
    else
    {
        set_value("");
    }

    return ret_val;
}

ReturnCode_t DynamicDataImpl::clear_sequence_element(
        TypeKind type_kind,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    assert(TK_ARRAY == type_kind || TK_SEQUENCE == type_kind);

    auto element_type =
            get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                        type_->get_descriptor().element_type()));

    switch (element_type->get_kind())
    {
        case TK_INT32:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT32:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT8:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT16:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT16:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_INT64:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_UINT64:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT32:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT64:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_FLOAT128:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_CHAR8:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_CHAR16:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_BOOLEAN:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_STRING8:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        case TK_STRING16:
        {
            auto seq = std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    seq->emplace(it);
                }
                ret_value = RETCODE_OK;
            }
            break;
        }
        default:
            auto seq = std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(
                value_.begin()->second);
            if (seq->size() > id)
            {
                auto it = seq->erase(seq->begin() + id);
                if (TK_ARRAY == type_kind)
                {
                    it = seq->emplace(it);
                    seq->at(id) = traits<DynamicData>::narrow<DynamicDataImpl>(
                        DynamicDataFactory::get_instance()->create_data(element_type));
                }
                ret_value = RETCODE_OK;
            }
            break;
    }

    return ret_value;
}

ReturnCode_t DynamicDataImpl::clear_value(
        MemberId id) noexcept
{
    ReturnCode_t ret_val = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ARRAY == type_kind || TK_SEQUENCE == type_kind)
    {
        ret_val =  clear_sequence_element(type_kind, id);
    }
    else if (TK_MAP == type_kind)
    {
        auto key_it = std::find_if(key_to_id_.begin(), key_to_id_.end(), [id](
                            const decltype(key_to_id_)::value_type& value)
                        {
                            return value.second == id;
                        });

        if (key_to_id_.end() != key_it)
        {
            auto value_it = value_.find(id);
            assert(value_it != value_.end());

            key_to_id_.erase(key_it);
            value_.erase(value_it);
            ret_val = RETCODE_OK;
        }
        else
        {
            assert(value_.end() == value_.find(id));
        }
    }
    else if (TK_BITMASK == type_kind)
    {
        assert(1 == value_.size());
        assert(1 == type_->get_descriptor().bound().size());
        auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
        assert(sequence->size() == type_->get_descriptor().bound().at(0));

        if (sequence->size() > id)
        {
            sequence->at(id) = false;
            ret_val = RETCODE_OK;
        }
    }
    else if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind )
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
            ret_val = RETCODE_OK;
        }
    }
    else if (TK_UNION == type_kind)
    {
        if (0 == id || selected_union_member_ == id)
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

                if (0 == id)
                {
                    // Set default discriminator value.
                    set_discriminator_value(type_->default_discriminator_value(),
                            traits<DynamicType>::narrow<DynamicTypeImpl>(member->get_descriptor().type()),
                            data);
                    selected_union_member_ = type_->default_union_member();
                }
                else
                {
                    set_default_value(member, data);
                }

                ret_val = RETCODE_OK;
            }
        }
    }
    else
    {
        if (MEMBER_ID_INVALID == id)
        {
            set_value("");
            ret_val = RETCODE_OK;
        }
    }

    return ret_val;
}

traits<DynamicData>::ref_type DynamicDataImpl::loan_value(
        MemberId id) noexcept
{
    if (id != MEMBER_ID_INVALID)
    {
        TypeKind type_kind = get_enclosing_typekind(type_);

        if (std::find(loaned_values_.begin(), loaned_values_.end(), id) == loaned_values_.end())
        {
            if (TK_ANNOTATION == type_kind ||
                    TK_STRUCTURE == type_kind ||
                    TK_UNION == type_kind)
            {
                auto it = value_.find(id);
                if (it != value_.end())
                {
                    auto sp = std::static_pointer_cast<DynamicData>(it->second);

                    if (TK_UNION == type_kind && 0 != id)     // Set new discriminator.
                    {
                        set_discriminator_value(id);
                    }

                    loaned_values_.push_back(id);
                    return sp;
                }
            }
            else if (TK_ARRAY == type_kind ||
                    TK_SEQUENCE == type_kind)
            {
                auto element_type =
                        get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    type_->get_descriptor().element_type()));

                if (is_complex_kind(element_type->get_kind()))
                {
                    auto it = value_.cbegin();
                    assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
                    auto sequence =
                            std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(it->second);
                    assert(sequence);
                    if ((TK_ARRAY == type_kind && sequence->size() >= id + 1) ||
                            (TK_SEQUENCE == type_kind &&
                            (static_cast<uint32_t>(LENGTH_UNLIMITED) ==
                            type_->get_descriptor().bound().at(0) ||
                            type_->get_descriptor().bound().at(0) >= id + 1)))
                    {
                        if (sequence->size() < id + 1)
                        {
                            auto last_pos = sequence->size();
                            sequence->resize(id + 1);

                            for (auto pos = last_pos; pos < sequence->size(); ++pos)
                            {
                                sequence->at(pos) = traits<DynamicData>::narrow<DynamicDataImpl>(
                                    DynamicDataFactory::get_instance()->create_data(element_type));
                            }
                        }

                        loaned_values_.push_back(id);
                        return sequence->at(id);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning a collection of primitives");
                }
            }
            else if (TK_MAP == type_kind)
            {
                auto element_type =
                        get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    type_->get_descriptor().element_type()));

                if (is_complex_kind(element_type->get_kind()))
                {
                    auto it = value_.find(id);
                    if (it != value_.end())
                    {
                        auto sp = std::static_pointer_cast<DynamicData>(it->second);
                        loaned_values_.push_back(id);
                        return sp;
                    }
                }
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
        traits<DynamicData>::ref_type value) noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    for (auto loan_it {loaned_values_.begin()}; loan_it != loaned_values_.end(); ++loan_it)
    {
        if (TK_ANNOTATION == type_kind ||
                TK_MAP == type_kind ||
                TK_STRUCTURE == type_kind ||
                TK_UNION == type_kind)
        {
            auto it = value_.find(*loan_it);
            if (it != value_.end() && std::static_pointer_cast<DynamicData>(it->second) == value)
            {
                loaned_values_.erase(loan_it);
                return RETCODE_OK;
            }
        }
        else if (TK_ARRAY == type_kind ||
                TK_SEQUENCE == type_kind)
        {
            auto it = value_.cbegin();
            assert(value_.cend() != it && MEMBER_ID_INVALID == it->first);
            auto sequence =
                    std::static_pointer_cast<std::vector<traits<DynamicData>::ref_type>>(it->second);
            assert((TK_ARRAY == type_kind && sequence->size() >= *loan_it + 1) ||
                    (TK_SEQUENCE == type_kind &&
                    (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_->get_descriptor().bound().at(0) ||
                    type_->get_descriptor().bound().at(0) >= *loan_it + 1)));
            if (sequence->size() >= *loan_it + 1)
            {
                if (sequence->at(*loan_it) == value)
                {
                    loaned_values_.erase(loan_it);
                    return RETCODE_OK;
                }
            }
        }
    }

    EPROSIMA_LOG_ERROR(DYN_TYPES, "Error returning loaned Value. The value hasn't been loaned.");
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
    return get_value<TK_INT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int32_value(
        MemberId id,
        int32_t value) noexcept
{
    return set_value<TK_INT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint32_value(
        uint32_t& value,
        MemberId id) noexcept
{
    return get_value<TK_UINT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint32_value(
        MemberId id,
        uint32_t value) noexcept
{
    return set_value<TK_UINT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int8_value(
        int8_t& value,
        MemberId id) noexcept
{
    return get_value<TK_INT8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int8_value(
        MemberId id,
        int8_t value) noexcept
{
    return set_value<TK_INT8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint8_value(
        uint8_t& value,
        MemberId id) noexcept
{
    return get_value<TK_UINT8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint8_value(
        MemberId id,
        uint8_t value) noexcept
{
    return set_value<TK_UINT8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int16_value(
        int16_t& value,
        MemberId id) noexcept
{
    return get_value<TK_INT16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int16_value(
        MemberId id,
        int16_t value) noexcept
{
    return set_value<TK_INT16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint16_value(
        uint16_t& value,
        MemberId id) noexcept
{
    return get_value<TK_UINT16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint16_value(
        MemberId id,
        uint16_t value) noexcept
{
    return set_value<TK_UINT16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int64_value(
        int64_t& value,
        MemberId id) noexcept
{
    return get_value<TK_INT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int64_value(
        MemberId id,
        int64_t value) noexcept
{
    return set_value<TK_INT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint64_value(
        uint64_t& value,
        MemberId id) noexcept
{
    return get_value<TK_UINT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint64_value(
        MemberId id,
        uint64_t value) noexcept
{
    return set_value<TK_UINT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float32_value(
        float& value,
        MemberId id) noexcept
{
    return get_value<TK_FLOAT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float32_value(
        MemberId id,
        float value) noexcept
{
    return set_value<TK_FLOAT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float64_value(
        double& value,
        MemberId id) noexcept
{
    return get_value<TK_FLOAT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float64_value(
        MemberId id,
        double value) noexcept
{
    return set_value<TK_FLOAT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float128_value(
        long double& value,
        MemberId id) noexcept
{
    return get_value<TK_FLOAT128>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float128_value(
        MemberId id,
        long double value) noexcept
{
    return set_value<TK_FLOAT128>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char8_value(
        char& value,
        MemberId id) noexcept
{
    return get_value<TK_CHAR8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char8_value(
        MemberId id,
        char value) noexcept
{
    return set_value<TK_CHAR8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char16_value(
        wchar_t& value,
        MemberId id) noexcept
{
    return get_value<TK_CHAR16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char16_value(
        MemberId id,
        wchar_t value) noexcept
{
    return set_value<TK_CHAR16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_byte_value(
        eprosima::fastrtps::rtps::octet& value,
        MemberId id) noexcept
{
    return get_value<TK_BYTE>(value, id);
}

ReturnCode_t DynamicDataImpl::set_byte_value(
        MemberId id,
        eprosima::fastrtps::rtps::octet value) noexcept
{
    return set_value<TK_BYTE>(id, value);
}

ReturnCode_t DynamicDataImpl::get_boolean_value(
        bool& value,
        MemberId id) noexcept
{
    return get_value<TK_BOOLEAN>(value, id);
}

ReturnCode_t DynamicDataImpl::set_boolean_value(
        MemberId id,
        bool value) noexcept
{
    return set_value<TK_BOOLEAN>(id, value);
}

ReturnCode_t DynamicDataImpl::get_string_value(
        std::string& value,
        MemberId id) noexcept
{
    return get_value<TK_STRING8>(value, id);
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
    }

    return set_value<TK_STRING8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_wstring_value(
        std::wstring& value,
        MemberId id) noexcept
{
    return get_value<TK_STRING16>(value, id);
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
    }

    return set_value<TK_STRING16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_complex_value(
        traits<DynamicData>::ref_type value,
        MemberId id) noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (MEMBER_ID_INVALID != id)
    {
        if (TK_ANNOTATION == type_kind ||
                TK_STRUCTURE == type_kind ||
                (TK_UNION == type_kind && (0 == id || selected_union_member_ == id)))
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                value = std::static_pointer_cast<DynamicData>(it->second)->clone();
                return RETCODE_OK;
            }
        }
    }
    // TODO array and sequences
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error loaning Value. Invalid MemberId.");
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
    return get_sequence_values<TK_INT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int32_values(
        MemberId id,
        const Int32Seq& value) noexcept
{
    return set_sequence_values<TK_INT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint32_values(
        UInt32Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_UINT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint32_values(
        MemberId id,
        const UInt32Seq& value) noexcept
{
    return set_sequence_values<TK_UINT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int8_values(
        Int8Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_INT8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int8_values(
        MemberId id,
        const Int8Seq& value) noexcept
{
    return set_sequence_values<TK_INT8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint8_values(
        UInt8Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_UINT8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint8_values(
        MemberId id,
        const UInt8Seq& value) noexcept
{
    return set_sequence_values<TK_UINT8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int16_values(
        Int16Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_INT16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int16_values(
        MemberId id,
        const Int16Seq& value) noexcept
{
    return set_sequence_values<TK_INT16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint16_values(
        UInt16Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_UINT16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint16_values(
        MemberId id,
        const UInt16Seq& value) noexcept
{
    return set_sequence_values<TK_UINT16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_int64_values(
        Int64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_INT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_int64_values(
        MemberId id,
        const Int64Seq& value) noexcept
{
    return set_sequence_values<TK_INT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_uint64_values(
        UInt64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_UINT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_uint64_values(
        MemberId id,
        const UInt64Seq& value) noexcept
{
    return set_sequence_values<TK_UINT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float32_values(
        Float32Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_FLOAT32>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float32_values(
        MemberId id,
        const Float32Seq& value) noexcept
{
    return set_sequence_values<TK_FLOAT32>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float64_values(
        Float64Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_FLOAT64>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float64_values(
        MemberId id,
        const Float64Seq& value) noexcept
{
    return set_sequence_values<TK_FLOAT64>(id, value);
}

ReturnCode_t DynamicDataImpl::get_float128_values(
        Float128Seq& value,
        MemberId id) noexcept
{
    return get_sequence_values< TK_FLOAT128>(value, id);
}

ReturnCode_t DynamicDataImpl::set_float128_values(
        MemberId id,
        const Float128Seq& value) noexcept
{
    return set_sequence_values<TK_FLOAT128>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char8_values(
        CharSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_CHAR8>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char8_values(
        MemberId id,
        const CharSeq& value) noexcept
{
    return set_sequence_values<TK_CHAR8>(id, value);
}

ReturnCode_t DynamicDataImpl::get_char16_values(
        WcharSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_CHAR16>(value, id);
}

ReturnCode_t DynamicDataImpl::set_char16_values(
        MemberId id,
        const WcharSeq& value) noexcept
{
    return set_sequence_values<TK_CHAR16>(id, value);
}

ReturnCode_t DynamicDataImpl::get_byte_values(
        ByteSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_BYTE>(value, id);
}

ReturnCode_t DynamicDataImpl::set_byte_values(
        MemberId id,
        const ByteSeq& value) noexcept
{
    return set_sequence_values<TK_BYTE>(id, value);
}

ReturnCode_t DynamicDataImpl::get_boolean_values(
        BooleanSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_BOOLEAN>(value, id);
}

ReturnCode_t DynamicDataImpl::set_boolean_values(
        MemberId id,
        const BooleanSeq& value) noexcept
{
    return set_sequence_values<TK_BOOLEAN>(id, value);
}

ReturnCode_t DynamicDataImpl::get_string_values(
        StringSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_STRING8>(value, id);
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

        return set_sequence_values<TK_STRING8>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataImpl::get_wstring_values(
        WstringSeq& value,
        MemberId id) noexcept
{
    return get_sequence_values<TK_STRING16>(value, id);
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

        return set_sequence_values<TK_STRING16>(id, value);
    }

    return RETCODE_BAD_PARAMETER;
}

template<TypeKind TK>
void DynamicDataImpl::apply_bitset_mask(
        MemberId member_id,
        TypeForKind<TK>& value) const noexcept
{
    // Get member index.
    assert(type_->get_all_members().end() != type_->get_all_members().find(member_id));
    const auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                                type_->get_all_members().at(member_id))};
    const auto member_index {member_impl->get_descriptor().index()};
    const auto bound {type_->get_descriptor().bound().at(member_index)};
    uint64_t mask {0XFFFFFFFFFFFFFFFFllu << bound};
    value &= static_cast<TypeForKind<TK>>(~mask);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_FLOAT32>(
        MemberId,
        TypeForKind<TK_FLOAT32>&) const noexcept
{
    assert(false);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_FLOAT64>(
        MemberId,
        TypeForKind<TK_FLOAT64>&) const noexcept
{
    assert(false);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_FLOAT128>(
        MemberId,
        TypeForKind<TK_FLOAT128>&) const noexcept
{
    assert(false);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_CHAR16>(
        MemberId,
        TypeForKind<TK_CHAR16>&) const noexcept
{
    assert(false);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_STRING8>(
        MemberId,
        TypeForKind<TK_STRING8>&) const noexcept
{
    assert(false);
}

template<>
void DynamicDataImpl::apply_bitset_mask<TK_STRING16>(
        MemberId,
        TypeForKind<TK_STRING16>&) const noexcept
{
    assert(false);
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
            return *((SequenceTypeForKind<TK_INT32>*)left) == *((SequenceTypeForKind<TK_INT32>*)right);
        }
        case TK_UINT32:     {
            return *((SequenceTypeForKind<TK_UINT32>*)left) == *((SequenceTypeForKind<TK_UINT32>*)right);
        }
        case TK_INT8:      {
            return *((SequenceTypeForKind<TK_INT8>*)left) == *((SequenceTypeForKind<TK_INT8>*)right);
        }
        case TK_INT16:      {
            return *((SequenceTypeForKind<TK_INT16>*)left) == *((SequenceTypeForKind<TK_INT16>*)right);
        }
        case TK_UINT16:     {
            return *((SequenceTypeForKind<TK_UINT16>*)left) == *((SequenceTypeForKind<TK_UINT16>*)right);
        }
        case TK_INT64:      {
            return *((SequenceTypeForKind<TK_INT64>*)left) == *((SequenceTypeForKind<TK_INT64>*)right);
        }
        case TK_UINT64:     {
            return *((SequenceTypeForKind<TK_UINT64>*)left) == *((SequenceTypeForKind<TK_UINT64>*)right);
        }
        case TK_FLOAT32:    {
            return *((SequenceTypeForKind<TK_FLOAT32>*)left) == *((SequenceTypeForKind<TK_FLOAT32>*)right);
        }
        case TK_FLOAT64:    {
            return *((SequenceTypeForKind<TK_FLOAT64>*)left) == *((SequenceTypeForKind<TK_FLOAT64>*)right);
        }
        case TK_FLOAT128:   {
            return *((SequenceTypeForKind<TK_FLOAT128>*)left) == *((SequenceTypeForKind<TK_FLOAT128>*)right);
        }
        case TK_CHAR8:      {
            return *((SequenceTypeForKind<TK_CHAR8>*)left) == *((SequenceTypeForKind<TK_CHAR8>*)right);
        }
        case TK_CHAR16:     {
            return *((SequenceTypeForKind<TK_CHAR16>*)left) == *((SequenceTypeForKind<TK_CHAR16>*)right);
        }
        case TK_BOOLEAN:    {
            return *((SequenceTypeForKind<TK_BOOLEAN>*)left) == *((SequenceTypeForKind<TK_BOOLEAN>*)right);
        }
        case TK_BYTE:
        case TK_UINT8:      {
            return *((SequenceTypeForKind<TK_UINT8>*)left) == *((SequenceTypeForKind<TK_UINT8>*)right);
        }
        case TK_STRING8:    {
            return *((SequenceTypeForKind<TK_STRING8>*)left) == *((SequenceTypeForKind<TK_STRING8>*)right);
        }
        case TK_STRING16:   {
            return *((SequenceTypeForKind<TK_STRING16>*)left) == *((SequenceTypeForKind<TK_STRING16>*)right);
        }
        default:            {
            auto* dl = (std::vector<traits<DynamicDataImpl>::ref_type>*)left;
            auto* dr = (std::vector<traits<DynamicDataImpl>::ref_type>*)right;
            return dl->size() == dr->size() && std::equal(dl->begin(), dl->end(), dr->begin(),
                           [](traits<DynamicDataImpl>::ref_type& x, traits<DynamicDataImpl>::ref_type& y)
                           {
                               return x->equals(y);
                           });
        }
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

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::get_bitmask_bit(
        TypeForKind<TK>& value,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (MEMBER_ID_INVALID != id)
    {
        auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
        assert(sequence);
        if (sequence->size() >= id + 1 && (TK_BOOLEAN == TK || TypePromotion<TK_BOOLEAN, TK>::value))
        {
            value = static_cast<TypeForKind<TK_BOOLEAN>>(sequence->at(id));
            ret_value =  RETCODE_OK;
        }
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::get_bitmask_bit<TK_STRING8>(
        TypeForKind<TK_STRING8>&,
        MemberId) noexcept
{
    return RETCODE_BAD_PARAMETER;
}

template<>
ReturnCode_t DynamicDataImpl::get_bitmask_bit<TK_STRING16>(
        TypeForKind<TK_STRING16>&,
        MemberId) noexcept
{
    return RETCODE_BAD_PARAMETER;
}

traits<DynamicTypeImpl>::ref_type DynamicDataImpl::get_enclosing_type(
        traits<DynamicTypeImpl>::ref_type type) noexcept
{
    traits<DynamicTypeImpl>::ref_type ret_value = type;

    if (TK_ENUM == ret_value->get_kind())     // If enum, get enclosing type.
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
    else if (TK_ALIAS == ret_value->get_kind())     // If alias, get enclosing type.
    {
        do {
            ret_value = traits<DynamicType>::narrow<DynamicTypeImpl>(ret_value->get_descriptor().base_type());
        } while (TK_ALIAS == ret_value->get_kind());
    }

    return ret_value;
}

TypeKind DynamicDataImpl::get_enclosing_typekind(
        traits<DynamicTypeImpl>::ref_type type) noexcept
{
    return get_enclosing_type(type)->get_kind();
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::get_primitive_value(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        TypeForKind<TK>& value,
        MemberId member_id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK == element_kind)
    {
        value = *std::static_pointer_cast<TypeForKind<TK>>(value_iterator->second);
        ret_value =  RETCODE_OK;
    }
    else
    {
        switch (element_kind)
        {
            case TK_INT8:
                if (TypePromotion<TK_INT8, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_INT8>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT8:
                if (TypePromotion<TK_UINT8, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_UINT8>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT16:
                if (TypePromotion<TK_INT16, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_INT16>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT16:
                if (TypePromotion<TK_UINT16, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_UINT16>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT32:
                if (TypePromotion<TK_INT32, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_INT32>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT32:
                if (TypePromotion<TK_UINT32, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_UINT32>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT64:
                if (TypePromotion<TK_INT64, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_INT64>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT64:
                if (TypePromotion<TK_UINT64, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_UINT64>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT32:
                if (TypePromotion<TK_FLOAT32, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT64:
                if (TypePromotion<TK_FLOAT64, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT128:
                if (TypePromotion<TK_FLOAT128, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_CHAR8:
                if (TypePromotion<TK_CHAR8, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_CHAR8>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_CHAR16:
                if (TypePromotion<TK_CHAR16, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_CHAR16>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_BYTE:
                if (TypePromotion<TK_BYTE, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_BYTE>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_BOOLEAN:
                if (TypePromotion<TK_BOOLEAN, TK>::value)
                {
                    assert(MEMBER_ID_INVALID == member_id);
                    value =
                            static_cast<TypeForKind<TK>>(*std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(
                                value_iterator->second));
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_STRING8:
                if (MEMBER_ID_INVALID != member_id && (TK == TK_CHAR8 || TypePromotion<TK_CHAR8, TK>::value))
                {
                    auto str = std::static_pointer_cast<TypeForKind<TK_STRING8>>(value_iterator->second);
                    if (member_id < str->length())
                    {
                        value = str->at(member_id);
                        ret_value = RETCODE_OK;
                    }

                }
                break;
            case TK_STRING16:
                if (MEMBER_ID_INVALID != member_id && (TK == TK_CHAR16 || TypePromotion<TK_CHAR16, TK>::value))
                {
                    auto str = std::static_pointer_cast<TypeForKind<TK_STRING16>>(value_iterator->second);
                    if (member_id < str->length())
                    {
                        value = str->at(member_id);
                        ret_value = RETCODE_OK;
                    }

                }
                break;
            default:
                break;
        }
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::get_primitive_value<TK_STRING8>(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        TypeForKind<TK_STRING8>& value,
        MemberId member_id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK_STRING8 == element_kind)
    {
        auto str = std::static_pointer_cast<TypeForKind<TK_STRING8>>(value_iterator->second);

        if (MEMBER_ID_INVALID == member_id)
        {
            value = *str;
            ret_value =  RETCODE_OK;
        }
        else if (member_id < str->length())
        {
            value = str->at(member_id);
            ret_value = RETCODE_OK;
        }
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::get_primitive_value<TK_STRING16>(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        TypeForKind<TK_STRING16>& value,
        MemberId member_id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK_STRING16 == element_kind)
    {
        auto str = std::static_pointer_cast<TypeForKind<TK_STRING16>>(value_iterator->second);

        if (MEMBER_ID_INVALID == member_id)
        {
            value = *str;
            ret_value =  RETCODE_OK;
        }
        else if (member_id < str->length())
        {
            value = str->at(member_id);
            ret_value = RETCODE_OK;
        }
    }

    return ret_value;
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
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT32:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT8:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT16:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT16:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second)->size();
        }
        break;
        case TK_INT64:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second)->size();
        }
        break;
        case TK_UINT64:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT32:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT64:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second)->size();
        }
        break;
        case TK_FLOAT128:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second)->size();
        }
        break;
        case TK_CHAR8:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second)->size();
        }
        break;
        case TK_CHAR16:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second)->size();
        }
        break;
        case TK_BOOLEAN:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second)->size();
        }
        break;
        case TK_BYTE:
        case TK_UINT8:
        {
            ret_value =
                    std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second)->
                            size();
        }
        break;
        case TK_STRING8:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second)->size();
        }
        break;
        case TK_STRING16:
        {
            ret_value =  std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second)->size();
        }
        break;
        default:
            ret_value =
                    std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(value_.begin()->second)
                            ->size();
    }

    return static_cast<uint32_t>(ret_value);
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::get_sequence_values(
        SequenceTypeForKind<TK>& value,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = type_->get_kind();

    if (TK_ANNOTATION == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id && (TK_UNION != type_kind || 0 == id || selected_union_member_ == id))
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->get_sequence_values<TK>(
                    value, 0);

                ret_value = RETCODE_OK;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot find MemberId " << id);
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
            auto sequence = std::static_pointer_cast<SequenceTypeForKind<TK>>(it->second);
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
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Unsupported type kind");
    }

    return ret_value;
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::get_value(
        TypeForKind<TK>& value,
        MemberId id) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    auto type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id && (TK_UNION != type_kind || 0 == id || selected_union_member_ == id))
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                ret_value =  std::static_pointer_cast<DynamicDataImpl>(it->second)->get_value<TK>(
                    value,
                    MEMBER_ID_INVALID);
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
            auto sequence = std::static_pointer_cast<SequenceTypeForKind<TK>>(it->second);
            assert(sequence);
            if (sequence->size() > id)
            {
                value = sequence->at(id);
                return RETCODE_OK;
            }
        }
    }
    else if (TK_MAP == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                TypeKind element_kind =
                        get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    type_->get_descriptor().element_type()));
                ret_value = get_primitive_value<TK>(element_kind, it, value, MEMBER_ID_INVALID);
            }
        }
    }
    else if (TK_BITMASK == type_kind)
    {
        ret_value = get_bitmask_bit<TK>(value, id);
    }
    else // Primitives
    {
        if (MEMBER_ID_INVALID == id || TK_STRING8 == type_kind || TK_STRING16 == type_kind)
        {
            assert(1 == value_.size() && MEMBER_ID_INVALID == value_.begin()->first);
            ret_value = get_primitive_value<TK>(type_kind, value_.begin(), value, id);
        }
    }

    return ret_value;
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::set_bitmask_bit(
        MemberId id,
        const TypeForKind<TK>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_UNSUPPORTED;

    if (MEMBER_ID_INVALID != id)
    {
        auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
        assert(sequence);
        if (sequence->size() >= id + 1 && (TK_BOOLEAN == TK || TypePromotion<TK, TK_BOOLEAN>::value))
        {
            sequence->at(id) = static_cast<TypeForKind<TK_BOOLEAN>>(value);
            ret_value =  RETCODE_OK;
        }
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::set_bitmask_bit<TK_STRING8>(
        MemberId,
        const TypeForKind<TK_STRING8>&) noexcept
{
    return RETCODE_BAD_PARAMETER;
}

template<>
ReturnCode_t DynamicDataImpl::set_bitmask_bit<TK_STRING16>(
        MemberId,
        const TypeForKind<TK_STRING16>&) noexcept
{
    return RETCODE_BAD_PARAMETER;
}

void DynamicDataImpl::set_discriminator_value(
        MemberId id) noexcept
{
    assert(TK_UNION == type_->get_kind());

    traits<DynamicTypeMember>::ref_type member;
    type_->get_member(member, id);
    assert(member);
    auto m_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);
    int32_t label {0};

    if (m_impl->get_descriptor().is_default_label())
    {
        label = type_->default_discriminator_value();
    }
    else
    {
        assert(0 < m_impl->get_descriptor().label().size());
        label = m_impl->get_descriptor().label().at(0);
    }

    assert(value_.find(0) != value_.end());
    traits<DynamicDataImpl>::ref_type data_impl = std::static_pointer_cast<DynamicDataImpl>(value_.at(0));

    // Set new discriminator value.
    set_discriminator_value(label,
            traits<DynamicType>::narrow<DynamicTypeImpl>(type_->get_descriptor().discriminator_type()),
            data_impl);

    selected_union_member_ = id;
}

void DynamicDataImpl::set_discriminator_value(
        int32_t new_discriminator_value,
        const traits<DynamicTypeImpl>::ref_type& discriminator_type,
        traits<DynamicDataImpl>::ref_type& data) noexcept
{
    assert(TK_UNION == get_enclosing_typekind(type_));
    TypeKind discriminator_kind = get_enclosing_typekind(discriminator_type);

    switch (discriminator_kind)
    {
        case TK_INT32:
            data->set_int32_value(MEMBER_ID_INVALID, new_discriminator_value);
            break;
        case TK_UINT32:
            data->set_uint32_value(MEMBER_ID_INVALID, static_cast<uint32_t>(new_discriminator_value));
            break;
        case TK_INT8:
            data->set_int8_value(MEMBER_ID_INVALID, static_cast<int8_t>(new_discriminator_value));
            break;
        case TK_INT16:
            data->set_int16_value(MEMBER_ID_INVALID, static_cast<int16_t>(new_discriminator_value));
            break;
        case TK_UINT16:
            data->set_uint16_value(MEMBER_ID_INVALID, static_cast<uint16_t>(new_discriminator_value));
            break;
        case TK_CHAR8:
            data->set_char8_value(MEMBER_ID_INVALID, static_cast<char>(new_discriminator_value));
            break;
        case TK_CHAR16:
            data->set_char16_value(MEMBER_ID_INVALID, static_cast<wchar_t>(new_discriminator_value));
            break;
        case TK_BOOLEAN:
            data->set_boolean_value(MEMBER_ID_INVALID, 0 == new_discriminator_value ? false : true);
            break;
        case TK_BYTE:
        case TK_UINT8:
            data->set_uint8_value(MEMBER_ID_INVALID, static_cast<uint8_t>(new_discriminator_value));
            break;
        default:
            break;
    }
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::set_primitive_value(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        const TypeForKind<TK>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK == element_kind)
    {
        *std::static_pointer_cast<TypeForKind<TK>>(value_iterator->second) = value;
        ret_value =  RETCODE_OK;
    }
    else
    {
        switch (element_kind)
        {
            case TK_INT8:
                if (TypePromotion<TK, TK_INT8>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_INT8>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_INT8>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT8:
                if (TypePromotion<TK, TK_UINT8>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_UINT8>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_UINT8>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT16:
                if (TypePromotion<TK, TK_INT16>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_INT16>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_INT16>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT16:
                if (TypePromotion<TK, TK_UINT16>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_UINT16>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_UINT16>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT32:
                if (TypePromotion<TK, TK_INT32>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_INT32>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_INT32>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT32:
                if (TypePromotion<TK, TK_UINT32>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_UINT32>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_UINT32>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_INT64:
                if (TypePromotion<TK, TK_INT64>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_INT64>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_INT64>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_UINT64:
                if (TypePromotion<TK, TK_UINT64>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_UINT64>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_UINT64>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT32:
                if (TypePromotion<TK, TK_FLOAT32>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_FLOAT32>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT64:
                if (TypePromotion<TK, TK_FLOAT64>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_FLOAT64>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_FLOAT128:
                if (TypePromotion<TK, TK_FLOAT128>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_FLOAT128>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_CHAR8:
                if (TypePromotion<TK, TK_CHAR8>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_CHAR8>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_CHAR8>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_CHAR16:
                if (TypePromotion<TK, TK_CHAR16>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_CHAR16>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_CHAR16>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_BYTE:
                if (TypePromotion<TK, TK_BYTE>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_BYTE>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_BYTE>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            case TK_BOOLEAN:
                if (TypePromotion<TK, TK_BOOLEAN>::value)
                {
                    *std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(value_iterator->second) =
                            static_cast<TypeForKind<TK_BOOLEAN>>(value);
                    ret_value =  RETCODE_OK;
                }
                break;
            default:
                break;
        }
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::set_primitive_value<TK_STRING8>(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        const TypeForKind<TK_STRING8>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK_STRING8 == element_kind)
    {
        *std::static_pointer_cast<TypeForKind<TK_STRING8>>(value_iterator->second) = value;
        ret_value =  RETCODE_OK;
    }

    return ret_value;
}

template<>
ReturnCode_t DynamicDataImpl::set_primitive_value<TK_STRING16>(
        TypeKind element_kind,
        std::map<MemberId, std::shared_ptr<void>>::iterator value_iterator,
        const TypeForKind<TK_STRING16>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;

    if (TK_STRING16 == element_kind)
    {
        *std::static_pointer_cast<TypeForKind<TK_STRING16>>(value_iterator->second) = value;
        ret_value =  RETCODE_OK;
    }

    return ret_value;
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::set_sequence_values(
        MemberId id,
        const SequenceTypeForKind<TK>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_MAP == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            if (TK_UNION == type_kind && 0 == id)     // Check setting discriminator is correct.
            {
                if (!check_new_discriminator_value(value))
                {
                    return RETCODE_BAD_PARAMETER;
                }
            }

            auto it = value_.find(id);
            if (it != value_.end())
            {

                ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_sequence_values<TK>(
                    0,
                    value);
            }
            else
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot find MemberId " << id);
            }

            if (RETCODE_OK == ret_value && TK_UNION == type_kind && 0 != id)     // Set new discriminator.
            {
                set_discriminator_value(id);
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
            auto sequence = std::static_pointer_cast<SequenceTypeForKind<TK>>(it->second);
            assert(sequence);
            if ((TK_ARRAY == type_kind && sequence->size() >= id + value.size()) ||
                    (TK_SEQUENCE == type_kind &&
                    (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_->get_descriptor().bound().at(0) ||
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
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Unsupported type kind");
    }

    return ret_value;
}

template<TypeKind TK>
ReturnCode_t DynamicDataImpl::set_value(
        MemberId id,
        const TypeForKind<TK>& value) noexcept
{
    ReturnCode_t ret_value = RETCODE_BAD_PARAMETER;
    TypeKind type_kind = get_enclosing_typekind(type_);

    if (TK_ANNOTATION == type_kind ||
            TK_BITSET == type_kind ||
            TK_STRUCTURE == type_kind ||
            TK_UNION == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            if (TK_UNION == type_kind && 0 == id)     // Check setting discriminator is correct.
            {
                if (!check_new_discriminator_value(value))
                {
                    return RETCODE_BAD_PARAMETER;
                }
            }

            auto it = value_.find(id);
            if (it != value_.end())
            {
                TypeForKind<TK> new_value {value};

                // In case of BITSET, apply mask.
                if (TK_BITSET == type_kind)
                {
                    apply_bitset_mask<TK>(id, new_value);
                }

                ret_value = std::static_pointer_cast<DynamicDataImpl>(it->second)->set_value<TK>(
                    MEMBER_ID_INVALID, new_value);
            }

            if (RETCODE_OK == ret_value && TK_UNION == type_kind && 0 != id)     // Set new discriminator.
            {
                set_discriminator_value(id);
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
            auto sequence = std::static_pointer_cast<SequenceTypeForKind<TK>>(it->second);
            assert(sequence);
            if ((TK_ARRAY == type_kind && sequence->size() >= id + 1) ||
                    (TK_SEQUENCE == type_kind &&
                    (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_->get_descriptor().bound().at(0) ||
                    type_->get_descriptor().bound().at(0) >= id + 1)))
            {
                if (sequence->size() < id + 1)
                {
                    sequence->resize(id + 1);
                }
                auto initial_pos = sequence->begin() + id;
                *initial_pos = value;
                ret_value =  RETCODE_OK;
            }
        }
    }
    else if (TK_MAP == type_kind)
    {
        if (MEMBER_ID_INVALID != id)
        {
            auto it = value_.find(id);
            if (it != value_.end())
            {
                TypeKind element_kind =
                        get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    type_->get_descriptor().element_type()));
                ret_value = set_primitive_value<TK>(element_kind, it, value);
            }
        }
    }
    else if (TK_BITMASK == type_kind)
    {
        ret_value = set_bitmask_bit<TK>(id, value);
    }
    else // Primitives
    {
        if (MEMBER_ID_INVALID == id)
        {
            assert(1 == value_.size() && MEMBER_ID_INVALID == value_.begin()->first);
            ret_value = set_primitive_value<TK>(type_kind, value_.begin(), value);
        }
    }

    return ret_value;
}

void DynamicDataImpl::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    serialize(cdr, type_);
}

void DynamicDataImpl::serialize(
        eprosima::fastcdr::Cdr& cdr,
        const traits<DynamicTypeImpl>::ref_type type) const
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
            cdr << *std::static_pointer_cast<TypeForKind<TK_INT32>>(it->second);
            break;
        }
        case TK_UINT32:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_UINT32>>(it->second);
            break;
        }
        case TK_INT8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_INT8>>(it->second);
            break;
        }
        case TK_INT16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_INT16>>(it->second);
            break;
        }
        case TK_UINT16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_UINT16>>(it->second);
            break;
        }
        case TK_INT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_INT64>>(it->second);
            break;
        }
        case TK_UINT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_UINT64>>(it->second);
            break;
        }
        case TK_FLOAT32:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(it->second);
            break;
        }
        case TK_FLOAT64:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(it->second);
            break;
        }
        case TK_FLOAT128:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(it->second);
            break;
        }
        case TK_CHAR8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_CHAR8>>(it->second);
            break;
        }
        case TK_CHAR16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_CHAR16>>(it->second);
            break;
        }
        case TK_BOOLEAN:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(it->second);
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_UINT8>>(it->second);
            break;
        }
        case TK_STRING8:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_STRING8>>(it->second);
            break;
        }
        case TK_STRING16:
        {
            auto it = value_.begin();
            cdr << *std::static_pointer_cast<TypeForKind<TK_STRING16>>(it->second);
            break;
        }
        case TK_UNION:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }

            eprosima::fastcdr::Cdr::state current_state(cdr);
            cdr.begin_serialize_type(current_state, encoding);

            // The union_id_ must be serialized as a discriminator_type_
            auto discriminator_data {std::static_pointer_cast<DynamicDataImpl>(value_.at(0))};
            cdr << eprosima::fastcdr::MemberId{0} << discriminator_data;

            if (MEMBER_ID_INVALID != selected_union_member_)
            {
                auto member_data = std::static_pointer_cast<DynamicDataImpl>(value_.at(selected_union_member_));
                cdr << eprosima::fastcdr::MemberId{selected_union_member_} << member_data;
            }

            cdr.end_serialize_type(current_state);
            break;
        }
        case TK_BITSET:
        {
            size_t index = 0;
            size_t sum {0};
            std::bitset<64> bitset;
            for (auto& member : type->get_all_members_by_index())
            {
                MemberId member_id {member->get_id()};
                auto it = value_.find(member_id);

                if (it != value_.end())
                {
                    int64_t value {0};
                    auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                    if (RETCODE_OK == member_data->get_value<TK_INT64>(value, MEMBER_ID_INVALID))
                    {
                        auto base {member_id};
                        auto size {type_->get_descriptor().bound().at(index)};
                        for (auto i = base; i < base + size; ++i)
                        {
                            bitset.set(i, !!(value & 0x01));
                            value = value >> 1;
                        }
                        sum += size;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error retrieving bitset bitfield value");
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES,
                            "Error serializing bitset bitfield because not found on DynamicData");
                }

                ++index;
            }

            if (9 > sum)
            {
                std::bitset<8> new_bitset(bitset.to_ullong());
                cdr << new_bitset;
            }
            else if (17 > sum)
            {
                std::bitset<16> new_bitset(bitset.to_ullong());
                cdr << new_bitset;
            }
            else if (33 > sum)
            {
                std::bitset<32> new_bitset(bitset.to_ullong());
                cdr << new_bitset;
            }
            else
            {
                cdr << bitset;
            }
            break;
        }
        case TK_STRUCTURE:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }

            eprosima::fastcdr::Cdr::state current_state(cdr);
            cdr.begin_serialize_type(current_state, encoding);

            for (auto& member : type->get_all_members_by_index())
            {
                //TODO(richiware) if (!m.annotation_is_non_serialized())
                {
                    auto it = value_.find(member->get_id());

                    if (it != value_.end())
                    {
                        auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                        cdr << eprosima::fastcdr::MemberId{member->get_id()} << member_data;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error serializing structure member because not found on DynamicData");
                    }
                }
            }

            cdr.end_serialize_type(current_state);
            break;
        }
        case TK_ARRAY:
        {
            TypeKind element_kind {get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                               type_->get_descriptor().element_type()))};
            switch (element_kind)
            {
                case TK_INT32:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second));
                    break;
                case TK_UINT32:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(
                                value_.begin()->second));
                    break;
                case TK_INT8:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second));
                    break;
                case TK_INT16:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second));
                    break;
                case TK_UINT16:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(
                                value_.begin()->second));
                    break;
                case TK_INT64:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second));
                    break;
                case TK_UINT64:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(
                                value_.begin()->second));
                    break;
                case TK_FLOAT32:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->
                                    second));
                    break;
                case TK_FLOAT64:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->
                                    second));
                    break;
                case TK_FLOAT128:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->
                                    second));
                    break;
                case TK_CHAR8:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second));
                    break;
                case TK_CHAR16:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(
                                value_.begin()->second));
                    break;
                case TK_BOOLEAN:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->
                                    second));
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second));
                    break;
                case TK_STRING8:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->
                                    second));
                    break;
                case TK_STRING16:
                    cdr.serialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->
                                    second));
                    break;
                default:
                    cdr.serialize_array(*std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(
                                value_.begin()->second));
                    break;
            }

            break;
        }
        case TK_SEQUENCE:
        {
            TypeKind element_kind {get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                               type_->get_descriptor().element_type()))};
            switch (element_kind)
            {
                case TK_INT32:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second);
                    break;
                case TK_UINT32:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second);
                    break;
                case TK_INT8:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second);
                    break;
                case TK_INT16:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second);
                    break;
                case TK_UINT16:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second);
                    break;
                case TK_INT64:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second);
                    break;
                case TK_UINT64:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second);
                    break;
                case TK_FLOAT32:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second);
                    break;
                case TK_FLOAT64:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second);
                    break;
                case TK_FLOAT128:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second);
                    break;
                case TK_CHAR8:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second);
                    break;
                case TK_CHAR16:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second);
                    break;
                case TK_BOOLEAN:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second);
                    break;
                case TK_STRING8:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second);
                    break;
                case TK_STRING16:
                    cdr << *std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second);
                    break;
                default:
                    cdr << *std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(
                        value_.begin()->second);
                    break;
            }

            break;
        }
        case TK_MAP:
        {
            TypeKind key_kind {get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                           type_->get_descriptor().key_element_type()))};
            TypeKind element_kind {get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                               type_->get_descriptor().element_type()))};
            bool is_primitive {!is_complex_kind(element_kind)};
            eprosima::fastcdr::Cdr::state dheader_state{!is_primitive ? cdr.allocate_xcdrv2_dheader() :
                                                        eprosima::fastcdr::Cdr::state{cdr}};

            assert(key_to_id_.size() == value_.size());
            cdr << static_cast<uint32_t>(key_to_id_.size());

            for (auto it = key_to_id_.begin(); it != key_to_id_.end(); ++it)
            {
                switch (key_kind)
                {
                    case TK_INT32:
                    {
                        TypeForKind<TK_INT32> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_UINT32:
                    {
                        TypeForKind<TK_UINT32> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_INT8:
                    {
                        TypeForKind<TK_INT8> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_INT16:
                    {
                        TypeForKind<TK_INT16> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_UINT16:
                    {
                        TypeForKind<TK_UINT16> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_INT64:
                    {
                        TypeForKind<TK_INT64> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_UINT64:
                    {
                        TypeForKind<TK_UINT64> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_FLOAT32:
                    {
                        TypeForKind<TK_FLOAT32> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_FLOAT64:
                    {
                        TypeForKind<TK_FLOAT64> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_FLOAT128:
                    {
                        TypeForKind<TK_FLOAT128> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_CHAR8:
                    {
                        TypeForKind<TK_CHAR8> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_CHAR16:
                    {
                        TypeForKind<TK_CHAR16> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_BOOLEAN:
                    {
                        TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_BYTE:
                    case TK_UINT8:
                    {
                        TypeForKind<TK_UINT8> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_STRING8:
                    {
                        TypeForKind<TK_STRING8> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    case TK_STRING16:
                    {
                        TypeForKind<TK_STRING16> value = TypeValueConverter::sto(it->first);
                        cdr << value;
                    }
                    break;
                    default:
                        assert(false);
                        break;

                }
                assert(value_.end() != value_.find(it->second));
                switch (element_kind)
                {
                    case TK_INT32:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_INT32>>(value_.find(it->second)->second);
                        break;
                    case TK_UINT32:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_UINT32>>(value_.find(it->second)->second);
                        break;
                    case TK_INT8:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_INT8>>(value_.find(it->second)->second);
                        break;
                    case TK_INT16:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_INT16>>(value_.find(it->second)->second);
                        break;
                    case TK_UINT16:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_UINT16>>(value_.find(it->second)->second);
                        break;
                    case TK_INT64:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_INT64>>(value_.find(it->second)->second);
                        break;
                    case TK_UINT64:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_UINT64>>(value_.find(it->second)->second);
                        break;
                    case TK_FLOAT32:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(value_.find(it->second)->second);
                        break;
                    case TK_FLOAT64:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(value_.find(it->second)->second);
                        break;
                    case TK_FLOAT128:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(value_.find(it->second)->second);
                        break;
                    case TK_CHAR8:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_CHAR8>>(value_.find(it->second)->second);
                        break;
                    case TK_CHAR16:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_CHAR16>>(value_.find(it->second)->second);
                        break;
                    case TK_BOOLEAN:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(value_.find(it->second)->second);
                        break;
                    case TK_BYTE:
                    case TK_UINT8:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_UINT8>>(value_.find(it->second)->second);
                        break;
                    case TK_STRING8:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_STRING8>>(value_.find(it->second)->second);
                        break;
                    case TK_STRING16:
                        cdr << *std::static_pointer_cast<TypeForKind<TK_STRING16>>(value_.find(it->second)->second);
                        break;
                    default:
                        cdr << std::static_pointer_cast<DynamicDataImpl>(value_.find(it->second)->second);
                        break;

                }
            }

            if (!is_primitive)
            {
                cdr.set_xcdrv2_dheader(dheader_state);
            }

            break;
        }
        case TK_BITMASK:
        {
            uint64_t value {0};
            auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
            auto bound = type_->get_descriptor().bound().at(0);

            for (size_t pos {0}; pos < sequence->size(); ++pos)
            {
                if (sequence->at(pos))
                {
                    value |= 1 << pos;
                }
            }

            if (9 > bound)
            {
                cdr << static_cast<uint8_t>(value);
            }
            else if (17 > bound)
            {
                cdr << static_cast<uint16_t>(value);
            }
            else if (33 > bound)
            {
                cdr << static_cast<uint32_t>(value);
            }
            else
            {
                cdr << value;
            }
        }
        break;
    }
}

bool DynamicDataImpl::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    return deserialize(cdr, type_);
}

bool DynamicDataImpl::deserialize(
        eprosima::fastcdr::Cdr& cdr,
        const traits<DynamicTypeImpl>::ref_type type)
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
            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT32>>(it->second);
            break;
        }
        case TK_UINT32:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT32>>(it->second);
            break;
        }
        case TK_INT8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT8>>(it->second);
            break;
        }
        case TK_INT16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT16>>(it->second);
            break;
        }
        case TK_UINT16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT16>>(it->second);
            break;
        }
        case TK_INT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT64>>(it->second);
            break;
        }
        case TK_UINT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT64>>(it->second);
            break;
        }
        case TK_FLOAT32:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(it->second);
            break;
        }
        case TK_FLOAT64:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(it->second);
            break;
        }
        case TK_FLOAT128:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(it->second);
            break;
        }
        case TK_CHAR8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_CHAR8>>(it->second);
            break;
        }
        case TK_CHAR16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_CHAR16>>(it->second);
            break;
        }
        case TK_BOOLEAN:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(it->second);
            break;
        }
        case TK_BYTE:
        case TK_UINT8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT8>>(it->second);
            break;
        }
        case TK_STRING8:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_STRING8>>(it->second);
            break;
        }
        case TK_STRING16:
        {
            auto it = value_.begin();
            cdr >> *std::static_pointer_cast<TypeForKind<TK_STRING16>>(it->second);
            break;
        }
        case TK_UNION:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }
            cdr.deserialize_type(encoding,
                    [&](eprosima::fastcdr::Cdr& dcdr,
                    const eprosima::fastcdr::MemberId& mid) -> bool
                    {
                        bool ret_value = true;
                        traits<DynamicTypeMember>::ref_type member;

                        switch (mid.id)
                        {
                            case 0:
                                {
                                    traits<DynamicDataImpl>::ref_type member_data =
                                    std::static_pointer_cast<DynamicDataImpl>(value_.at(
                                        0));
                                    dcdr >> member_data;

                                    // Select member pointed by discriminator.
                                    int32_t discriminator {0};
                                    selected_union_member_ = MEMBER_ID_INVALID;
                                    if (RETCODE_OK == get_int32_value(discriminator, 0))
                                    {
                                        for (auto member : type_->get_all_members_by_index())
                                        {
                                            auto m_impl =
                                            traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);

                                            for (auto label : m_impl->get_descriptor().label())
                                            {
                                                if (discriminator == label)
                                                {
                                                    selected_union_member_ = m_impl->get_id();
                                                    break;
                                                }
                                            }
                                        }

                                        if (MEMBER_ID_INVALID == selected_union_member_)
                                        {
                                            selected_union_member_ = type_->default_union_member();
                                        }
                                    }
                                }
                                break;
                            default:
                                {
                                    // Check MemberId in mutable case.
                                    traits<DynamicDataImpl>::ref_type member_data =
                                    std::static_pointer_cast<DynamicDataImpl>(value_.at(
                                        selected_union_member_));
                                    dcdr >> member_data;
                                }
                                break;
                        }

                        return ret_value;
                    });

            break;
        }
        case TK_BITSET:
        {
            std::bitset<64> bitset;
            auto sum =
                    std::accumulate(type_->get_descriptor().bound().begin(), type_->get_descriptor().bound().end(), 0);

            if (9 > sum)
            {
                std::bitset<8> new_bitset;
                cdr >> new_bitset;
                bitset = {new_bitset.to_ullong()};
            }
            else if (17 > sum)
            {
                std::bitset<16> new_bitset;
                cdr >> new_bitset;
                bitset = {new_bitset.to_ullong()};
            }
            else if (33 > sum)
            {
                std::bitset<32> new_bitset;
                cdr >> new_bitset;
                bitset = {new_bitset.to_ullong()};
            }
            else
            {
                cdr << bitset;
            }

            size_t index = 0;
            for (auto& member : type->get_all_members_by_index())
            {
                MemberId member_id {member->get_id()};
                auto it = value_.find(member_id);

                if (it != value_.end())
                {
                    int64_t value {0};
                    auto base {member_id};
                    auto size {type_->get_descriptor().bound().at(index)};
                    auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                    for (uint32_t i = 0; i < size; ++i)
                    {
                        if (bitset.test(i + base))
                        {
                            value |= 0x1 << i;
                        }
                    }

                    TypeKind element_kind {
                        get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                    member->get_descriptor().type()))};
                    ReturnCode_t ret_value {RETCODE_ERROR};

                    switch (element_kind)
                    {
                        case TK_INT8:
                            ret_value = member_data->set_value<TK_INT8>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_UINT8:
                            ret_value = member_data->set_value<TK_UINT8>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_INT16:
                            ret_value = member_data->set_value<TK_INT16>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_UINT16:
                            ret_value = member_data->set_value<TK_UINT16>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_INT32:
                            ret_value = member_data->set_value<TK_INT32>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_UINT32:
                            ret_value = member_data->set_value<TK_UINT32>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_INT64:
                            ret_value = member_data->set_value<TK_INT64>(MEMBER_ID_INVALID, value);
                            break;
                        case TK_UINT64:
                            ret_value = member_data->set_value<TK_UINT64>(MEMBER_ID_INVALID, value);
                            break;
                        default:
                            break;
                    }

                    if (RETCODE_OK != ret_value)
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error retrieving bitset bitfield value");
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES,
                            "Error serializing bitset bitfield because not found on DynamicData");
                }

                ++index;
            }
            break;
        }
        case TK_STRUCTURE:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }
            cdr.deserialize_type(encoding,
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
            break;
        }
        case TK_ARRAY:     // Sequence is like structure, but with size
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->
                                    second));
                    break;
                case TK_UINT32:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->
                                    second));
                    break;
                case TK_INT8:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(
                                value_.begin()->second));
                    break;
                case TK_INT16:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->
                                    second));
                    break;
                case TK_UINT16:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->
                                    second));
                    break;
                case TK_INT64:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->
                                    second));
                    break;
                case TK_UINT64:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->
                                    second));
                    break;
                case TK_FLOAT32:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->
                                    second));
                    break;
                case TK_FLOAT64:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->
                                    second));
                    break;
                case TK_FLOAT128:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(
                                value_.begin()->second));
                    break;
                case TK_CHAR8:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->
                                    second));
                    break;
                case TK_CHAR16:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->
                                    second));
                    break;
                case TK_BOOLEAN:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->
                                    second));
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->
                                    second));
                    break;
                case TK_STRING8:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(
                                value_.begin()->second));
                    break;
                case TK_STRING16:
                    cdr.deserialize_array(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->
                                    second));
                    break;
                default:
                    cdr.deserialize_array(*std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(
                                value_.begin()->second));
                    break;
            }
            break;
        }
        case TK_SEQUENCE:     // Sequence is like structure, but with size
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(value_.begin()->second);
                    break;
                case TK_UINT32:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(value_.begin()->second);
                    break;
                case TK_INT8:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(value_.begin()->second);
                    break;
                case TK_INT16:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(value_.begin()->second);
                    break;
                case TK_UINT16:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(value_.begin()->second);
                    break;
                case TK_INT64:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(value_.begin()->second);
                    break;
                case TK_UINT64:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(value_.begin()->second);
                    break;
                case TK_FLOAT32:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(value_.begin()->second);
                    break;
                case TK_FLOAT64:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(value_.begin()->second);
                    break;
                case TK_FLOAT128:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(value_.begin()->second);
                    break;
                case TK_CHAR8:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(value_.begin()->second);
                    break;
                case TK_CHAR16:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(value_.begin()->second);
                    break;
                case TK_BOOLEAN:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(value_.begin()->second);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(value_.begin()->second);
                    break;
                case TK_STRING8:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(value_.begin()->second);
                    break;
                case TK_STRING16:
                    cdr >> *std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(value_.begin()->second);
                    break;
                default:
                {
                    auto vector_t = std::static_pointer_cast <std::vector<traits<DynamicDataImpl>::ref_type >> (
                        value_.begin()->second);
                    uint32_t sequence_length {0};

                    if (fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
                    {
                        uint32_t dheader {0};
                        cdr.deserialize(dheader);

                        auto offset {cdr.get_current_position()};

                        cdr >> sequence_length;

                        if (0 == sequence_length)
                        {
                            vector_t->clear();
                        }
                        else
                        {
                            auto element_type =
                                    get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                type_->get_descriptor().element_type()));

                            vector_t->resize(sequence_length);

                            for (size_t pos = 0; pos < vector_t->size(); ++pos)
                            {
                                if (!vector_t->at(pos))
                                {
                                    vector_t->at(pos) = traits<DynamicData>::narrow<DynamicDataImpl>(
                                        DynamicDataFactory::get_instance()->create_data(element_type));
                                }
                            }

                            uint32_t count {0};
                            while (cdr.get_current_position() - offset < dheader && count < sequence_length)
                            {
                                cdr.deserialize(vector_t->data()[count]);
                                ++count;
                            }

                            if (cdr.get_current_position() - offset != dheader)
                            {
                                throw fastcdr::exception::BadParamException(
                                          "Member size differs from the size specified by DHEADER");
                            }
                        }
                    }
                    else
                    {
                        fastcdr::Cdr::state state_before_error(cdr);

                        cdr.deserialize(sequence_length);

                        if (sequence_length == 0)
                        {
                            vector_t->clear();
                        }
                        else
                        {
                            auto element_type =
                                    get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                type_->get_descriptor().element_type()));
                            try
                            {
                                vector_t->resize(sequence_length);
                                for (size_t pos = 0; pos < vector_t->size(); ++pos)
                                {
                                    if (!vector_t->at(pos))
                                    {
                                        vector_t->at(pos) = traits<DynamicData>::narrow<DynamicDataImpl>(
                                            DynamicDataFactory::get_instance()->create_data(element_type));
                                    }
                                }
                                cdr.deserialize_array(vector_t->data(), vector_t->size());
                            }
                            catch (fastcdr::exception::Exception& ex)
                            {
                                cdr.set_state(state_before_error);
                                ex.raise();
                            }
                        }
                    }
                }
                break;
            }
            break;
        }
        case TK_MAP:
        {
            TypeKind key_kind { get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                            type_->get_descriptor().key_element_type()))};
            TypeKind element_kind { get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                type_->get_descriptor().element_type()))};
            bool is_primitive {!is_complex_kind(element_kind)};
            uint32_t dheader {0};

            if (!is_primitive && eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version())
            {
                cdr >> dheader;
            }

            clear_all_values();

            auto offset {cdr.get_current_position()};
            uint32_t length {0};
            cdr >> length;

            for (uint32_t count {0};
                    (is_primitive || eprosima::fastcdr::CdrVersion::XCDRv2 != cdr.get_cdr_version() ||
                    cdr.get_current_position() - offset < dheader) && count < length; ++count)
            {
                if (static_cast<uint32_t>(LENGTH_UNLIMITED) == type_->get_descriptor().bound().at(0) ||
                        type_->get_descriptor().bound().at(0) > key_to_id_.size())
                {
                    std::string key;
                    switch (key_kind)
                    {
                        case TK_INT32:
                        {
                            TypeForKind<TK_INT32> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_UINT32:
                        {
                            TypeForKind<TK_UINT32> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_INT8:
                        {
                            TypeForKind<TK_INT8> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_INT16:
                        {
                            TypeForKind<TK_INT16> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_UINT16:
                        {
                            TypeForKind<TK_UINT16> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_INT64:
                        {
                            TypeForKind<TK_INT64> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_UINT64:
                        {
                            TypeForKind<TK_UINT64> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_FLOAT32:
                        {
                            TypeForKind<TK_FLOAT32> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_FLOAT64:
                        {
                            TypeForKind<TK_FLOAT64> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_FLOAT128:
                        {
                            TypeForKind<TK_FLOAT128> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_CHAR8:
                        {
                            TypeForKind<TK_CHAR8> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_CHAR16:
                        {
                            TypeForKind<TK_CHAR16> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_BOOLEAN:
                        {
                            TypeForKind<TK_BOOLEAN> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_BYTE:
                        case TK_UINT8:
                        {
                            TypeForKind<TK_UINT8> value {0};
                            cdr >> value;
                            key = std::to_string(value);
                        }
                        break;
                        case TK_STRING8:
                        {
                            TypeForKind<TK_STRING8> value;
                            cdr >> value;
                            key = value;
                        }
                        break;
                        case TK_STRING16:
                        {
                            TypeForKind<TK_STRING16> value;
                            cdr >> value;
                            key = std::string(value.begin(), value.end());
                        }
                        break;
                        default:
                            assert(false);
                            break;

                    }
                    MemberId id = next_map_member_id_++;
                    key_to_id_[key] = id;
                    std::map<MemberId, std::shared_ptr<void>>::iterator insert_it {value_.end()};

                    if (!is_complex_kind(element_kind))
                    {
                        insert_it = add_value(element_kind, id);
                    }
                    else
                    {
                        traits<DynamicData>::ref_type data = DynamicDataFactory::get_instance()->create_data(
                            type_->get_descriptor().element_type());
                        insert_it = value_.emplace(id, data).first;
                    }

                    switch (element_kind)
                    {
                        case TK_INT32:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT32>>(insert_it->second);
                            break;
                        case TK_UINT32:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT32>>(insert_it->second);
                            break;
                        case TK_INT8:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT8>>(insert_it->second);
                            break;
                        case TK_INT16:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT16>>(insert_it->second);
                            break;
                        case TK_UINT16:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT16>>(insert_it->second);
                            break;
                        case TK_INT64:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_INT64>>(insert_it->second);
                            break;
                        case TK_UINT64:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT64>>(insert_it->second);
                            break;
                        case TK_FLOAT32:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(insert_it->second);
                            break;
                        case TK_FLOAT64:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(insert_it->second);
                            break;
                        case TK_FLOAT128:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(insert_it->second);
                            break;
                        case TK_CHAR8:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_CHAR8>>(insert_it->second);
                            break;
                        case TK_CHAR16:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_CHAR16>>(insert_it->second);
                            break;
                        case TK_BOOLEAN:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(insert_it->second);
                            break;
                        case TK_BYTE:
                        case TK_UINT8:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_UINT8>>(insert_it->second);
                            break;
                        case TK_STRING8:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_STRING8>>(insert_it->second);
                            break;
                        case TK_STRING16:
                            cdr >> *std::static_pointer_cast<TypeForKind<TK_STRING16>>(insert_it->second);
                            break;
                        default:
                            traits<DynamicDataImpl>::ref_type member_data = std::static_pointer_cast<DynamicDataImpl>(
                                insert_it->second);
                            cdr >> member_data;
                            break;

                    }

                }
            }

            if (!is_primitive && eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() &&
                    cdr.get_current_position() - offset != dheader)
            {
                throw fastcdr::exception::BadParamException(
                          "Member size differs from the size specified by DHEADER");
            }

            break;
        }
        case TK_BITMASK:
        {
            uint64_t value {0};
            auto sequence = std::static_pointer_cast<std::vector<bool>>(value_.begin()->second);
            auto bound = type_->get_descriptor().bound().at(0);

            if (9 > bound)
            {
                uint8_t value_get {0};
                cdr >> value_get;
                value = value_get;
            }
            else if (17 > bound)
            {
                uint16_t value_get {0};
                cdr >> value_get;
                value = value_get;
            }
            else if (33 > bound)
            {
                uint32_t value_get {0};
                cdr >> value_get;
                value = value_get;
            }
            else
            {
                cdr >> value;
            }

            for (size_t pos {0}; pos < sequence->size(); ++pos)
            {
                if (value & (1 << pos))
                {
                    sequence->at(pos) = true;
                }
                else
                {
                    sequence->at(pos) = false;
                }
            }

        }
        break;
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
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT32>>(
                                it->second), current_alignment);
            break;
        case TK_UINT32:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT32>>(
                                it->second), current_alignment);
            break;
        case TK_FLOAT32:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(
                                it->second), current_alignment);
            break;
        case TK_CHAR16:     // WCHARS NEED 32 Bits on Linux & MacOS
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_CHAR16>>(
                                it->second), current_alignment);
            break;
        case TK_INT8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT8>>(
                                it->second), current_alignment);
            break;
        case TK_INT16:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT16>>(
                                it->second), current_alignment);
            break;
        case TK_UINT16:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT16>>(
                                it->second), current_alignment);
            break;
        case TK_INT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT64>>(
                                it->second), current_alignment);
            break;
        case TK_UINT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT64>>(
                                it->second), current_alignment);
            break;
        case TK_FLOAT64:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(
                                it->second), current_alignment);
            break;
        case TK_FLOAT128:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(
                                it->second), current_alignment);
            break;
        case TK_CHAR8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_CHAR8>>(
                                it->second), current_alignment);
            break;
        case TK_BOOLEAN:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(
                                it->second), current_alignment);
            break;
        case TK_BYTE:
        case TK_UINT8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT8>>(
                                it->second), current_alignment);
            break;
        case TK_STRING8:
            calculated_size = calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_STRING8>>(
                                it->second), current_alignment);
            break;
        case TK_STRING16:
            calculated_size =
                    calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_STRING16>>(
                                it->second),
                            current_alignment);
            break;
        case TK_UNION:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }

            eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
            calculated_size = calculator.begin_calculate_type_serialized_size(encoding, current_alignment);

            // Union discriminator
            auto discriminator_data = std::static_pointer_cast<DynamicDataImpl>(value_.at(0));
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                            discriminator_data, current_alignment);

            if (MEMBER_ID_INVALID != selected_union_member_)
            {
                auto member_data = std::static_pointer_cast<DynamicDataImpl>(value_.at(selected_union_member_));
                calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                                member_data, current_alignment);
            }

            calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);
            break;
        }
        case TK_BITSET:
        {
            auto sum =
                    std::accumulate(type_->get_descriptor().bound().begin(), type_->get_descriptor().bound().end(), 0);
            if (9 > sum)
            {
                std::bitset<8> bitset;
                calculated_size = calculator.calculate_serialized_size(bitset, current_alignment);
            }
            else if (17 > sum)
            {
                std::bitset<16> bitset;
                calculated_size = calculator.calculate_serialized_size(bitset, current_alignment);
            }
            else if (33 > sum)
            {
                std::bitset<32> bitset;
                calculated_size = calculator.calculate_serialized_size(bitset, current_alignment);
            }
            else
            {
                std::bitset<64> bitset;
                calculated_size = calculator.calculate_serialized_size(bitset, current_alignment);
            }
            break;
        }
        case TK_STRUCTURE:
        {
            eprosima::fastcdr::EncodingAlgorithmFlag encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

            if (eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version())
            {
                switch (type_->get_descriptor().extensibility_kind())
                {
                    case ExtensibilityKind::MUTABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                        break;
                    case ExtensibilityKind::APPENDABLE:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                        break;
                    case ExtensibilityKind::FINAL:
                        encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                        break;
                }
            }
            else
            {
                if (ExtensibilityKind::MUTABLE == type_->get_descriptor().extensibility_kind())
                {
                    encoding = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
                }
            }

            eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
            calculated_size = calculator.begin_calculate_type_serialized_size(encoding, current_alignment);

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
        case TK_ARRAY:
        case TK_SEQUENCE:
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_->get_descriptor().element_type()));
            switch (element_kind)
            {
                case TK_INT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_INT32>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT32>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_INT8>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_INT16>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT16>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_INT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_INT64>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_UINT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT64>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT32:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT32>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT64:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT64>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_FLOAT128:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_FLOAT128>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_CHAR8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR8>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_CHAR16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_CHAR16>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_BOOLEAN:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_BOOLEAN>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_BYTE:
                case TK_UINT8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_UINT8>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_STRING8:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING8>>(
                                        value_.begin()->second), current_alignment);
                    break;
                case TK_STRING16:
                    calculated_size =
                            calculator.calculate_serialized_size(*std::static_pointer_cast<SequenceTypeForKind<TK_STRING16>>(
                                        value_.begin()->second), current_alignment);
                    break;
                default:
                    calculated_size =
                            calculator.calculate_serialized_size(
                        *std::static_pointer_cast<std::vector<traits<DynamicDataImpl>::ref_type>>(
                            value_.begin()->second), current_alignment);
                    break;

            }

            if (TK_ARRAY == type_kind)
            {
                calculated_size -= 4;
            }

            break;
        }
        case TK_MAP:
        {
            TypeKind key_kind { get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                            type_->get_descriptor().key_element_type()))};
            TypeKind element_kind { get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                type_->get_descriptor().element_type()))};
            bool is_primitive {!is_complex_kind(element_kind) &&
                               TK_STRING8 != element_kind &&
                               TK_STRING16 != element_kind};
            size_t initial_alignment {current_alignment};

            if (!is_primitive && eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version())
            {
                // DHEADER
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            calculated_size = current_alignment - initial_alignment;
            for (auto it = key_to_id_.begin(); it != key_to_id_.end(); ++it)
            {
                switch (key_kind)
                {
                    case TK_INT32:
                    {
                        TypeForKind<TK_INT32> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_UINT32:
                    {
                        TypeForKind<TK_UINT32> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_INT8:
                    {
                        TypeForKind<TK_INT8> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_INT16:
                    {
                        TypeForKind<TK_INT16> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_UINT16:
                    {
                        TypeForKind<TK_UINT16> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_INT64:
                    {
                        TypeForKind<TK_INT64> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_UINT64:
                    {
                        TypeForKind<TK_UINT64> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_FLOAT32:
                    {
                        TypeForKind<TK_FLOAT32> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_FLOAT64:
                    {
                        TypeForKind<TK_FLOAT64> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_FLOAT128:
                    {
                        TypeForKind<TK_FLOAT128> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_CHAR8:
                    {
                        TypeForKind<TK_CHAR8> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_CHAR16:
                    {
                        TypeForKind<TK_CHAR16> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_BOOLEAN:
                    {
                        TypeForKind<TK_BOOLEAN> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_BYTE:
                    case TK_UINT8:
                    {
                        TypeForKind<TK_UINT8> value {0};
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_STRING8:
                    {
                        TypeForKind<TK_STRING8> value;
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    case TK_STRING16:
                    {
                        TypeForKind<TK_STRING16> value;
                        calculated_size += calculator.calculate_serialized_size(value, current_alignment);
                    }
                    break;
                    default:
                        assert(false);
                        break;

                }
                assert(value_.end() != value_.find(it->second));
                switch (element_kind)
                {
                    case TK_INT32:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT32>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_UINT32:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT32>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_INT8:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT8>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_INT16:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT16>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_UINT16:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT16>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_INT64:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_INT64>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_UINT64:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT64>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_FLOAT32:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT32>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_FLOAT64:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT64>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_FLOAT128:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_FLOAT128>>(
                                            value_.find(
                                                it->second)->second), current_alignment);
                        break;
                    case TK_CHAR8:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_CHAR8>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_CHAR16:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_CHAR16>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_BOOLEAN:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_BOOLEAN>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_BYTE:
                    case TK_UINT8:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_UINT8>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_STRING8:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_STRING8>>(
                                            value_.find(it->second)->second), current_alignment);
                        break;
                    case TK_STRING16:
                        calculated_size +=
                                calculator.calculate_serialized_size(*std::static_pointer_cast<TypeForKind<TK_STRING16>>(
                                            value_.find(
                                                it->second)->second), current_alignment);
                        break;
                    default:
                        calculated_size +=
                                calculator.calculate_serialized_size(std::static_pointer_cast<DynamicDataImpl>(
                                            value_.find(it->second)->second), current_alignment);
                        break;

                }
            }

            if (!is_primitive && eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version())
            {
                // Inform DHEADER can be joined with NEXTINT
                // serialized_member_size_ = SERIALIZED_MEMBER_SIZE;
            }

            break;
        }
        case TK_BITMASK:
        {
            auto bound = type_->get_descriptor().bound().at(0);

            if (9 > bound)
            {
                uint8_t value {0};
                calculated_size = calculator.calculate_serialized_size(value, current_alignment);
            }
            else if (17 > bound)
            {
                uint16_t value {0};
                calculated_size = calculator.calculate_serialized_size(value, current_alignment);
            }
            else if (33 > bound)
            {
                uint32_t value {0};
                calculated_size = calculator.calculate_serialized_size(value, current_alignment);
            }
            else
            {
                uint64_t value {0};
                calculated_size = calculator.calculate_serialized_size(value, current_alignment);
            }
        }
        break;
        default:
            break;
    }

    return calculated_size;
}

void DynamicDataImpl::serialize_key(
        eprosima::fastcdr::Cdr& cdr) const noexcept
{
    TypeKind type_kind = get_enclosing_typekind(type_);

    switch (type_kind)
    {
        default:
            serialize(cdr);
            break;
        case TK_STRUCTURE:
        {
            bool there_is_keyed_member {false};
            for (auto& member : type_->get_all_members())
            {
                auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member.second)};
                if (member_impl->get_descriptor().is_key())
                {
                    there_is_keyed_member = true;
                    auto it = value_.find(member.first);

                    if (it != value_.end())
                    {
                        auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                        member_data->serialize_key(cdr);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error serializing structure member because not found on DynamicData");
                    }
                }
            }

            if (!there_is_keyed_member)
            {
                for (auto& member : type_->get_all_members())
                {
                    auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member.second)};
                    auto it = value_.find(member.first);

                    if (it != value_.end())
                    {
                        auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                        member_data->serialize_key(cdr);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error serializing structure member because not found on DynamicData");
                    }
                }
            }

            break;
        }
        case TK_UNION:
        {
            // The union_id_ must be serialized as a discriminator_type_
            auto discriminator_data {std::static_pointer_cast<DynamicDataImpl>(value_.at(0))};
            discriminator_data->serialize_key(cdr);

            if (MEMBER_ID_INVALID != selected_union_member_)
            {
                auto member_data = std::static_pointer_cast<DynamicDataImpl>(value_.at(selected_union_member_));
                member_data->serialize_key(cdr);
            }
            break;
        }
    }
}

size_t DynamicDataImpl::calculate_key_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        size_t& current_alignment) const noexcept
{
    size_t calculated_size {0};

    TypeKind type_kind = get_enclosing_typekind(type_);

    switch (type_kind)
    {
        default:
            calculated_size = calculate_serialized_size(calculator, current_alignment);
            break;
        case TK_STRUCTURE:
        {
            bool there_is_keyed_member {false};
            for (auto& member : type_->get_all_members())
            {
                auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member.second)};
                if (member_impl->get_descriptor().is_key())
                {
                    there_is_keyed_member = true;
                    auto it = value_.find(member.first);

                    if (it != value_.end())
                    {
                        auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                        calculated_size += calculator.calculate_member_serialized_size(
                            member.first, member_data, current_alignment);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error calculating size structure member because not found on DynamicData");
                    }
                }
            }

            if (!there_is_keyed_member)
            {
                for (auto& member : type_->get_all_members())
                {
                    auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member.second)};
                    auto it = value_.find(member.first);

                    if (it != value_.end())
                    {
                        auto member_data {std::static_pointer_cast<DynamicDataImpl>(it->second)};

                        calculated_size += calculator.calculate_member_serialized_size(
                            member.first, member_data, current_alignment);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error serializing structure member because not found on DynamicData");
                    }
                }
            }

            break;
        }
        case TK_UNION:
        {
            // The union_id_ must be serialized as a discriminator_type_
            auto discriminator_data {std::static_pointer_cast<DynamicDataImpl>(value_.at(0))};
            calculated_size += calculator.calculate_member_serialized_size(
                0, discriminator_data, current_alignment);

            if (MEMBER_ID_INVALID != selected_union_member_)
            {
                auto member_data = std::static_pointer_cast<DynamicDataImpl>(value_.at(selected_union_member_));
                calculated_size += calculator.calculate_member_serialized_size(
                    selected_union_member_, member_data, current_alignment);
            }
            break;
        }
    }

    return calculated_size;
}

size_t DynamicDataImpl::calculate_max_serialized_size(
        traits<DynamicType>::ref_type type,
        size_t current_alignment)
{
    size_t initial_alignment {current_alignment};

    /*TODO(richiware)
       if (data.type_ && annotation_is_non_serialized())
       {
        return 0;
       }
     */

    auto type_impl = get_enclosing_type(traits<DynamicType>::narrow<DynamicTypeImpl>(type));

    switch (type_impl->get_kind())
    {
        case TK_FLOAT128:
            current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            break;
        case TK_FLOAT64:
        case TK_INT64:
        case TK_UINT64:
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            break;
        case TK_FLOAT32:
        case TK_INT32:
        case TK_UINT32:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            break;
        case TK_INT16:
        case TK_UINT16:
        case TK_CHAR16:
            current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
            break;
        case TK_CHAR8:
        case TK_BYTE:
        case TK_BOOLEAN:
        case TK_INT8:
        case TK_UINT8:
            current_alignment += 1;
            break;
        case TK_STRING8:
        {
            size_t max_size = type_impl->get_descriptor().bound().at(0);
            if (static_cast<uint32_t>(LENGTH_UNLIMITED) == max_size)
            {
                max_size = 255;
            }
            current_alignment = 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + max_size + 1;
            break;
        }
        case TK_STRING16:
        {
            size_t max_size = type_impl->get_descriptor().bound().at(0);
            if (static_cast<uint32_t>(LENGTH_UNLIMITED) == max_size)
            {
                max_size = 255;
            }
            current_alignment = 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + (max_size * 2);
            break;
        }
        case TK_UNION:
        {
            size_t reset_alignment {0};
            size_t union_max_size_serialized {0};
            if (ExtensibilityKind::FINAL != type_impl->get_descriptor().extensibility_kind())
            {
                // For APPENDABLE and MUTABLE, the maximum is the XCDR2 header (DHEADER(0) : Int32).
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            current_alignment += calculate_max_serialized_size(
                type_impl->get_descriptor().discriminator_type(), current_alignment);

            for (auto& member : type_impl->get_all_members_by_index())
            {

                auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);
                reset_alignment = current_alignment;

                if (ExtensibilityKind::MUTABLE == type_impl->get_descriptor().extensibility_kind() ||
                        member->get_descriptor().is_optional())
                {
                    // If member is from a MUTABLE type (or it is optional member) the maximum is XCDR1 LongMemberHeader.
                    // << ALIGN(4)
                    // << { FLAG_I + FLAG_M + PID_EXTENDED : UInt16 }
                    // << { slength=8 : UInt16 }
                    // << { M.id : <<: UInt32 }
                    // << { M.value.ssize : UInt32 }
                    reset_alignment += 4 + 4 + 4 + eprosima::fastcdr::Cdr::alignment(reset_alignment, 4);
                }

                reset_alignment += calculate_max_serialized_size(member_impl->get_descriptor().type(), reset_alignment);

                if (union_max_size_serialized < reset_alignment)
                {
                    union_max_size_serialized = reset_alignment;
                }
            }

            current_alignment = union_max_size_serialized;

            if (ExtensibilityKind::MUTABLE != type_impl->get_descriptor().extensibility_kind())
            {
                // For MUTABLE, extra alignment for the PID_SENTINAL.
                current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }
            break;
        }
        case TK_BITSET:
        {
            auto sum =
                    std::accumulate(type_impl->get_descriptor().bound().begin(),
                            type_impl->get_descriptor().bound().end(), 0);
            if (9 > sum)
            {
                current_alignment += 1;
            }
            else if (17 > sum)
            {
                current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
            }
            else if (33 > sum)
            {
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }
            else
            {
                current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            }
            break;
        }
        case TK_STRUCTURE:
        {
            if (ExtensibilityKind::FINAL != type_impl->get_descriptor().extensibility_kind())
            {
                // For APPENDABLE and MUTABLE, the maximum is the XCDR2 header (DHEADER(0) : Int32).
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            for (auto& member : type_impl->get_all_members_by_index())
            {
                auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);

                if (ExtensibilityKind::MUTABLE == type_impl->get_descriptor().extensibility_kind() ||
                        member->get_descriptor().is_optional())
                {
                    // If member is from a MUTABLE type (or it is optional member) the maximum is XCDR1 LongMemberHeader.
                    // << ALIGN(4)
                    // << { FLAG_I + FLAG_M + PID_EXTENDED : UInt16 }
                    // << { slength=8 : UInt16 }
                    // << { M.id : <<: UInt32 }
                    // << { M.value.ssize : UInt32 }
                    current_alignment += 4 + 4 + 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
                }

                current_alignment += calculate_max_serialized_size(
                    member_impl->get_descriptor().type(), current_alignment);
            }

            if (ExtensibilityKind::MUTABLE != type_impl->get_descriptor().extensibility_kind())
            {
                // For MUTABLE, extra alignment for the PID_SENTINAL.
                current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }
            break;
        }
        case TK_ARRAY:
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_impl->get_descriptor().element_type()));

            if (is_complex_kind(element_kind) ||
                    TK_STRING8 == element_kind ||
                    TK_STRING16 == element_kind)
            {
                // DHEADER if XCDRv2
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            auto dimension {std::accumulate(type_impl->get_descriptor().bound().begin(),
                                    type_impl->get_descriptor().bound().end(), 1, std::multiplies<uint32_t>())};

            if (0 < dimension)
            {
                current_alignment += calculate_max_serialized_size(
                    type_impl->get_descriptor().element_type(), current_alignment);

                if (1 < dimension)
                {
                    auto element_size_after_first = calculate_max_serialized_size(
                        type_impl->get_descriptor().element_type(), current_alignment);
                    current_alignment += element_size_after_first * (dimension - 1);
                }
            }

            break;
        }
        case TK_SEQUENCE:
        {
            TypeKind element_kind =
                    get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                type_impl->get_descriptor().element_type()));

            if (is_complex_kind(element_kind) ||
                    TK_STRING8 == element_kind ||
                    TK_STRING16 == element_kind)
            {
                // DHEADER if XCDRv2
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            // Sequence length.
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            auto bound {type_impl->get_descriptor().bound().at(0)};

            if (static_cast<uint32_t>(LENGTH_UNLIMITED) != bound)
            {
                current_alignment += calculate_max_serialized_size(
                    type_impl->get_descriptor().element_type(), current_alignment);

                if (1 < bound)
                {
                    auto element_size_after_first = calculate_max_serialized_size(
                        type_impl->get_descriptor().element_type(), current_alignment);
                    current_alignment += element_size_after_first * (bound - 1);
                }
            }


            break;
        }
        case TK_MAP:
        {
            TypeKind element_kind { get_enclosing_typekind(traits<DynamicType>::narrow<DynamicTypeImpl>(
                                                type_impl->get_descriptor().element_type()))};

            if (is_complex_kind(element_kind) ||
                    TK_STRING8 == element_kind ||
                    TK_STRING16 == element_kind)
            {
                // DHEADER if XCDRv2
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }

            // Map length
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            auto bound {type_impl->get_descriptor().bound().at(0)};

            if (static_cast<uint32_t>(LENGTH_UNLIMITED) != bound)
            {
                current_alignment += calculate_max_serialized_size(
                    type_impl->get_descriptor().key_element_type(), current_alignment);
                current_alignment += calculate_max_serialized_size(
                    type_impl->get_descriptor().element_type(), current_alignment);

                if (1 < bound)
                {
                    auto element_size_after_first = calculate_max_serialized_size(
                        type_impl->get_descriptor().key_element_type(), current_alignment);
                    element_size_after_first += calculate_max_serialized_size(
                        type_impl->get_descriptor().element_type(), current_alignment);
                    current_alignment += element_size_after_first * (bound - 1);
                }
            }

            break;
        }
        case TK_BITMASK:
        {
            auto bound = type_impl->get_descriptor().bound().at(0);

            if (9 > bound)
            {
                current_alignment += 1;
            }
            else if (17 > bound)
            {
                current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
            }
            else if (33 > bound)
            {
                current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            }
            else
            {
                current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            }
        }
        break;
        default:
            break;
    }

    return current_alignment - initial_alignment;
}

} // namespace dds

} // namespace fastdds
} // namespace eprosima