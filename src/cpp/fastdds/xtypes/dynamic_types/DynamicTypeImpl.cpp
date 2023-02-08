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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include "AnnotationDescriptorImpl.hpp"
#include "DynamicDataImpl.hpp"
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include "DynamicDataFactoryImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "TypeState.hpp"

#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

DynamicTypeImpl::DynamicTypeImpl(
        use_the_create_method)
{
}

DynamicTypeImpl::DynamicTypeImpl(
        use_the_create_method,
        const TypeState& state)
    : TypeState(state)
{
}

DynamicTypeImpl::DynamicTypeImpl(
        use_the_create_method,
        TypeState&& state)
    : TypeState(std::move(state))
{
}

DynamicTypeImpl::~DynamicTypeImpl()
{
    clear();
}

void DynamicTypeImpl::clear()
{
    TypeState::clean();
}

bool DynamicTypeImpl::is_complex_kind() const
{
    switch (kind_)
    {
        case TK_ANNOTATION:
        case TK_ARRAY:
        case TK_BITMASK:
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

bool DynamicTypeImpl::is_discriminator_type() const
{
    if (kind_ == TK_ALIAS && get_base_type())
    {
        return get_base_type()->is_discriminator_type();
    }

    switch (kind_)
    {
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_CHAR8:
        case TK_INT16:
        case TK_UINT16:
        case TK_CHAR16:
        case TK_INT32:
        case TK_UINT32:
        case TK_FLOAT32:
        case TK_INT64:
        case TK_UINT64:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_STRING8:
        case TK_STRING16:
        case TK_BITMASK:
        case TK_ENUM:
            return true;
        default:
            return false;
    }
}

size_t DynamicTypeImpl::get_size() const
{
    switch (kind_)
    {
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_CHAR8:
            return 1;
        case TK_INT16:
        case TK_UINT16:
        case TK_CHAR16:
            return 2;
        case TK_INT32:
        case TK_UINT32:
        case TK_FLOAT32:
            return 4;
        case TK_INT64:
        case TK_UINT64:
        case TK_FLOAT64:
            return 8;
        case TK_FLOAT128:
            return 16;
        case TK_BITMASK:
        case TK_ENUM:
        {
            size_t bits = get_bounds(0);

            if (bits % 8 == 0)
            {
                return bits / 8;
            }
            else
            {
                return (bits / 8) + 1;
            }
        }
        default:
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Called get_size() within a non primitive type! This is a program's logic error.");
    }
    return 0;
}

bool DynamicTypeImpl::deserialize_discriminator(
        DynamicDataImpl& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(discriminator_type_);
    uint64_t label = 0;

    switch (discriminator_type_->get_kind())
    {
        case TK_BOOLEAN:
        {
            bool id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_BYTE:
        {
            eprosima::fastrtps::rtps::octet id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_CHAR8:
        {
            char id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_INT16:
        {
            int16_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_UINT16:
        {
            uint16_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_CHAR16:
        {
            wchar_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_INT32:
        {
            int32_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_UINT32:
        case TK_ENUM:
        {
            uint32_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_FLOAT32:
        {
            float id;
            cdr >> id;
            label = static_cast<uint64_t>(id);
            break;
        }
        case TK_INT64:
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Possible loss of precision on discriminator deserialization");

            int64_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_UINT64:
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Possible loss of precision on discriminator deserialization");

            uint64_t id;
            cdr >> id;
            label = id;
            break;
        }
        case TK_FLOAT64:
        {
            double id;
            cdr >> id;
            label = static_cast<uint64_t>(id);
            break;
        }
        case TK_FLOAT128:
        {
            long double id;
            cdr >> id;
            label = static_cast<uint64_t>(id);
            break;
        }
        case TK_STRING8:
        {
            std::string id;
            cdr >> id;
            label = stoul(id);
            break;
        }
        case TK_STRING16:
        {
            std::wstring id;
            cdr >> id;
            label = stoul(id);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            switch (type_size)
            {
                case 1:
                {
                    uint8_t id;
                    cdr >> id;
                    label = id;
                    break;
                }
                case 2:
                {
                    uint16_t id;
                    cdr >> id;
                    label = id;
                    break;
                }
                case 3:
                {
                    uint32_t id;
                    cdr >> id;
                    label = id;
                    break;
                }
                case 4:
                {
                    EPROSIMA_LOG_WARNING(DYN_TYPES, "Possible loss of precision on discriminator deserialization");

                    uint64_t id;
                    cdr >> id;
                    label = id;
                    break;
                }
            }
        }
            FASTRTPS_FALLTHROUGH;
        default:
            EPROSIMA_LOG_ERROR(DYN_TYPES, "DynamicData with wrong discriminator type: "
                    << discriminator_type_->get_kind());
            return false;
    }

    // Search the MemberId associated to the label if any
    MemberId id =  get_id_from_label(label);
    if (MEMBER_ID_INVALID == id)
    {
        return false;
    }

    data.union_id_ = id;
    return true;
}

bool DynamicTypeImpl::deserialize(
        DynamicDataImpl& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    bool res = true;

    if (annotation_is_non_serialized())
    {
        return res;
    }

    // check data and type are related
    assert(kind_ == data.get_kind() ||
            kind_ == TK_ALIAS ||
            data.type_->is_subclass(*this));

    switch (get_kind())
    {
        default:
            break;
        case TK_INT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.int32_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<int32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint32_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<uint32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.int16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<int16_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<uint16_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.int64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<int64_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<uint64_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.float32_value_;
#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<float>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.float64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<double>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT128:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.float128_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<long double>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.char8_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<char>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.char16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<wchar_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BOOLEAN:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.bool_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<bool>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BYTE:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.byte_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<eprosima::fastrtps::rtps::octet>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.string_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<std::string>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.wstring_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<std::wstring>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ENUM:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint32_value_;

#else
            auto it = data.values_.begin();
            cdr >> *std::static_pointer_cast<uint32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
#ifdef DYNAMIC_TYPES_CHECKING
            switch (type_size)
            {
                case 1:
                {
                    uint8_t temp;
                    cdr >> temp;
                    data.uint64_value_ = temp;
                    break;
                }
                case 2:
                {
                    uint16_t temp;
                    cdr >> temp;
                    data.uint64_value_ = temp;
                    break;
                }
                case 3:
                {
                    uint32_t temp;
                    cdr >> temp;
                    data.uint64_value_ = temp;
                    break;
                }
                case 4: cdr >> data.uint64_value_; break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
#else
            auto it = data.values_.begin();
            switch (type_size)
            {
                case 1: cdr >> *std::static_pointer_cast<uint8_t>(it->second); break;
                case 2: cdr >> *std::static_pointer_cast<uint16_t>(it->second); break;
                case 3: cdr >> *std::static_pointer_cast<uint32_t>(it->second); break;
                case 4: cdr >> *std::static_pointer_cast<uint64_t>(it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UNION:
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
        case TK_BITSET:
        case TK_STRUCTURE:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto& value_col = data.complex_values_;
#else
            auto& value_col = data.values_;
#endif // ifdef DYNAMIC_TYPES_CHECKING

            // delegate in base clases if any
            if (base_type_)
            {
                res &= base_type_->deserialize(data, cdr);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    std::shared_ptr<DynamicDataImpl> member_data;
                    auto it = value_col.find(m.get_id());

                    if (it != value_col.end())
                    {
                        member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                    }
                    else
                    {
                        member_data = DynamicDataFactoryImpl::get_instance().create_data(*m.get_type());
                        value_col.emplace(it->first, member_data);
                    }

                    res &= member_data ? m.get_type()->deserialize(*member_data, cdr) : false;
                }
            }
        }
        break;
        case TK_ARRAY:
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
        case TK_SEQUENCE:
        case TK_MAP:
        {
            uint32_t size(0);
            bool bKeyElement(false);
            cdr >> size;

            if (get_kind() == TK_MAP)
            {
                size *= 2; // We serialize the number of pairs.
            }
            for (uint32_t i = 0; i < size; ++i)
            {
                //cdr >> memberId;
                if (get_kind() == TK_MAP)
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
        case TK_ALIAS:
            assert(base_type_);
            return base_type_->deserialize(data, cdr);
    }

    return res;
}

size_t DynamicTypeImpl::getCdrSerializedSize(
        const DynamicDataImpl& data,
        size_t current_alignment /*= 0*/) const
{
    if (data.type_ && annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    // check data and type are related
    assert(kind_ == data.get_kind() ||
            kind_ == TK_ALIAS ||
            data.type_->is_subclass(*this));

    switch (get_kind())
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
        {
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
            break;
        }
        case TK_FLOAT128:
        {
            current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
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
                    data.string_value_.length() + 1;
#else
            auto it = data.values_.begin();
            // string content (length + characters + 1)
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
                    std::static_pointer_cast<std::string>(it->second)->length() + 1;
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            // string content (length + (characters * 4) )
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
                    ((data.wstring_value_.length()) * 4);
#else
            auto it = data.values_.begin();
            // string content (length + (characters * 4) )
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
                    std::static_pointer_cast<std::wstring>(it->second)->length() * 4;
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UNION:
        {
            // Union discriminator
            current_alignment += sizeof(MemberId) + eprosima::fastcdr::Cdr::alignment(current_alignment,
                            sizeof(MemberId));

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
        case TK_BITSET:
        case TK_STRUCTURE:
        {

#ifdef DYNAMIC_TYPES_CHECKING
            auto& value_col = data.complex_values_;
#else
            auto& value_col = data.values_;
#endif // ifdef DYNAMIC_TYPES_CHECKING

            // calculate inheritance overhead
            if (base_type_)
            {
                current_alignment += base_type_->getCdrSerializedSize(data, current_alignment);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    auto it = value_col.find(m.get_id());

                    if (it != value_col.end())
                    {
                        auto member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                        current_alignment +=
                                m.get_type()->getCdrSerializedSize(*member_data, current_alignment);
                    }
                }
            }
            break;
        }
        case TK_ARRAY:
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
                        *std::static_pointer_cast<DynamicDataImpl>(it->second),
                        current_alignment);
                }
                else
                {
                    current_alignment += emptyElementSize;
                }
            }
            break;
        }
        case TK_SEQUENCE:
        {
            assert(element_type_);
            // Elements count
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            // on sequences there is no need of keys serialization
#ifdef DYNAMIC_TYPES_CHECKING
            for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
            {
                // Element Size
                current_alignment += element_type_->getCdrSerializedSize(*it->second, current_alignment);
            }
#else
            for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
            {
                // Element Size
                current_alignment += element_type_->getCdrSerializedSize(
                    *std::static_pointer_cast<DynamicDataImpl>(it->second),
                    current_alignment);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_MAP:
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
                    *std::static_pointer_cast<DynamicDataImpl>(it++->second),
                    current_alignment);
                // Element Size
                current_alignment += element_type_->getCdrSerializedSize(
                    *std::static_pointer_cast<DynamicDataImpl>(it->second),
                    current_alignment);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ALIAS:
            assert(base_type_);
            return base_type_->getCdrSerializedSize(data, current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t DynamicTypeImpl::getEmptyCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    if (annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (get_kind())
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
        {
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
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
            // string length + 1
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 1;
            break;
        }
        case TK_STRING16:
        {
            // string length
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
            break;
        }
        case TK_UNION:
        {
            // union discriminator
            current_alignment += get_discriminator_type()->getEmptyCdrSerializedSize(current_alignment);
            break;
        }
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            // calculate inheritance overhead
            if (base_type_)
            {
                current_alignment += base_type_->getEmptyCdrSerializedSize(current_alignment);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    current_alignment +=
                            m.get_type()->getEmptyCdrSerializedSize(current_alignment);
                }
            }
            break;
        }
        case TK_ARRAY:
        {
            // Elements count
            //current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            // Element size with the maximum size
            current_alignment += get_total_bounds() *
                    get_element_type()->getEmptyCdrSerializedSize();
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
            current_alignment += get_base_type()->getEmptyCdrSerializedSize();
            break;
    }

    return current_alignment - initial_alignment;
}

size_t DynamicTypeImpl::getKeyMaxCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    size_t initial_alignment = current_alignment;

    // Structures check the the size of the key for their children
    if (get_kind() == TK_STRUCTURE || get_kind() == TK_BITSET)
    {
        // calculate inheritance overhead
        if (base_type_)
        {
            current_alignment += base_type_->getKeyMaxCdrSerializedSize(current_alignment);
        }

        for (const DynamicTypeMemberImpl& m : members_)
        {
            if (m.key_annotation())
            {
                current_alignment += m.get_type()->getKeyMaxCdrSerializedSize(current_alignment);
            }
        }
    }
    else if (is_key_defined_)
    {
        return getMaxCdrSerializedSize(current_alignment);
    }
    return current_alignment - initial_alignment;
}

size_t DynamicTypeImpl::getMaxCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    if (annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (get_kind())
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
        {
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
            break;
        }
        case TK_FLOAT128:
        {
            current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
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
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + get_bounds() + 1;
            break;
        }
        case TK_STRING16:
        {
            // string length + ( string content * 4 )
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + (get_bounds() * 4);

            break;
        }
        case TK_UNION:
        {
            // union id
            current_alignment += get_discriminator_type()->getMaxCdrSerializedSize(current_alignment);

            // Check the size of all members and take the size of the biggest one.
            size_t temp_size(0);
            size_t max_element_size(0);
            for (const DynamicTypeMemberImpl& m : members_)
            {
                temp_size = m.get_type()->getMaxCdrSerializedSize(current_alignment);
                if (temp_size > max_element_size)
                {
                    max_element_size = temp_size;
                }
            }
            current_alignment += max_element_size;
            break;
        }
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            // calculate inheritance overhead
            if (base_type_)
            {
                current_alignment += base_type_->getMaxCdrSerializedSize(current_alignment);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    current_alignment += m.get_type()->getMaxCdrSerializedSize(current_alignment);
                }
            }
            break;
        }
        case TK_ARRAY:
        {
            // Element size with the maximum size
            current_alignment += get_total_bounds() *
                    get_element_type()->getMaxCdrSerializedSize();
            break;
        }
        case TK_SEQUENCE:
        {
            // Elements count
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            // Element size with the maximum size
            current_alignment += get_total_bounds() *
                    get_element_type()->getMaxCdrSerializedSize();
            break;
        }
        case TK_MAP:
        {
            // Elements count
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            // Key Elements size with the maximum size
            current_alignment += get_total_bounds() *
                    get_key_element_type()->getMaxCdrSerializedSize();

            // Value Elements size with the maximum size
            current_alignment += get_total_bounds() *
                    get_element_type()->getMaxCdrSerializedSize();
            break;
        }

        case TK_ALIAS:
        {
            current_alignment += get_base_type()->getMaxCdrSerializedSize();
            break;
        }
    }

    return current_alignment - initial_alignment;
}

void DynamicTypeImpl::serialize_discriminator(
        const DynamicDataImpl& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    assert(discriminator_type_);

    // retrieve the associated label
    uint64_t label = data.union_id_;

    try
    {
        auto& lbls = get_member(data.union_id_).get_union_labels();
        if (!lbls.empty())
        {
            label = *lbls.begin();
        }
    }
    catch (const std::system_error&)
    {
    }

    switch (discriminator_type_->get_kind())
    {
        case TK_BOOLEAN:
        {
            cdr << (bool)label;
            break;
        }
        case TK_BYTE:
        {
            cdr << (eprosima::fastrtps::rtps::octet)label;
            break;
        }
        case TK_CHAR8:
        {
            cdr << (char)label;
            break;
        }
        case TK_INT16:
        {
            cdr << (int16_t)label;
            break;
        }
        case TK_UINT16:
        {
            cdr << (uint16_t)label;
            break;
        }
        case TK_CHAR16:
        {
            cdr << (wchar_t)label;
            break;
        }
        case TK_INT32:
        {
            cdr << (int32_t)label;
            break;
        }
        case TK_UINT32:
        case TK_ENUM:
        {
            cdr << (uint32_t)label;
            break;
        }
        case TK_FLOAT32:
        {
            cdr << (float)label;
            break;
        }
        case TK_INT64:
        {
            cdr << (int64_t)label;
            break;
        }
        case TK_UINT64:
        {
            cdr << (uint64_t)label;
            break;
        }
        case TK_FLOAT64:
        {
            cdr << (double)label;
            break;
        }
        case TK_FLOAT128:
        {
            cdr << (long double)label;
            break;
        }
        case TK_STRING8:
        {
            cdr << std::to_string(label);
            break;
        }
        case TK_STRING16:
        {
            cdr << std::to_wstring(label);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            switch (type_size)
            {
                case 1: cdr << (uint8_t)label; break;
                case 2: cdr << (uint16_t)label; break;
                case 3: cdr << (uint32_t)label; break;
                case 4: cdr << (uint64_t)label; break;
            }
        }
            FASTRTPS_FALLTHROUGH;
        default:
            EPROSIMA_LOG_ERROR(DYN_TYPES, "DynamicData with wrong discriminator type: "
                    << discriminator_type_->get_kind());
            break;
    }
}

void DynamicTypeImpl::serialize(
        const DynamicDataImpl& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    if (annotation_is_non_serialized())
    {
        return;
    }

    // check data and type are related
    assert(kind_ == data.get_kind() ||
            kind_ == TK_ALIAS ||
            data.type_->is_subclass(*this));

    switch (get_kind())
    {
        default:
            break;
        case TK_INT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int32_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<int32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint32_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<uint32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int16_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<int16_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint16_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<uint16_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int64_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<int64_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint64_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<uint64_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float32_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<float>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float64_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<double>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT128:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float128_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<long double>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.char8_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<char>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.char16_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<wchar_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BOOLEAN:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.bool_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<bool>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BYTE:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.byte_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<eprosima::fastrtps::rtps::octet>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.string_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<std::string>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.wstring_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<std::wstring>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ENUM:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint32_value_;
#else
            auto it = data.values_.begin();
            cdr << *std::static_pointer_cast<uint32_t>(it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = get_size();
#ifdef DYNAMIC_TYPES_CHECKING
            switch (type_size)
            {
                case 1: cdr << (uint8_t)data.uint64_value_; break;
                case 2: cdr << (uint16_t)data.uint64_value_; break;
                case 3: cdr << (uint32_t)data.uint64_value_; break;
                case 4: cdr << data.uint64_value_; break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
            }
#else
            auto it = data.values_.begin();
            switch (type_size)
            {
                case 1: cdr << *std::static_pointer_cast<uint8_t>(it->second); break;
                case 2: cdr << *std::static_pointer_cast<uint16_t>(it->second); break;
                case 3: cdr << *std::static_pointer_cast<uint32_t>(it->second); break;
                case 4: cdr << *std::static_pointer_cast<uint64_t>(it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
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
        case TK_SEQUENCE: // Sequence is like structure, but with size
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << static_cast<uint32_t>(data.complex_values_.size());
            for (uint32_t idx = 0; idx < static_cast<uint32_t>(data.complex_values_.size()); ++idx)
            {
                auto it = data.complex_values_.at(MemberId{idx});
                it->serialize(cdr);
            }
#else
            cdr << static_cast<uint32_t>(data.values_.size());
            for (uint32_t idx = 0; idx < static_cast<uint32_t>(data.values_.size()); ++idx)
            {
                auto it = data.values_.at(MemberId(idx));
                std::static_pointer_cast<DynamicDataImpl>(it)->serialize(cdr);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BITSET:
        case TK_STRUCTURE:
        {

#ifdef DYNAMIC_TYPES_CHECKING
            auto& value_col = data.complex_values_;
#else
            auto& value_col = data.values_;
#endif // ifdef DYNAMIC_TYPES_CHECKING

            // delegate in base clases if any
            if (base_type_)
            {
                base_type_->serialize(data, cdr);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    auto it = value_col.find(m.get_id());

                    if (it != value_col.end())
                    {
                        auto member_data = std::static_pointer_cast<DynamicDataImpl>(it->second);
                        m.get_type()->serialize(*member_data, cdr);
                    }
                }
            }
            break;
        }
        case TK_ARRAY:
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
        case TK_MAP:
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
        case TK_ALIAS:
            assert(base_type_);
            return base_type_->serialize(data, cdr);
    }
}

void DynamicTypeImpl::serializeKey(
        const DynamicDataImpl& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    // Structures check the the size of the key for their children
    if (get_kind() == TK_STRUCTURE || get_kind() == TK_BITSET)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
        {
            auto& cdata = *it->second;
            cdata.type_->serializeKey(cdata, cdr);
        }
#else
        for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
        {
            auto& cdata = *std::static_pointer_cast<DynamicDataImpl>(it->second);
            cdata.type_->serializeKey(cdata, cdr);
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else if (is_key_defined_)
    {
        serialize(data, cdr);
    }
}

void DynamicTypeImpl::serialize_empty_data(
        eprosima::fastcdr::Cdr& cdr) const
{
    if (annotation_is_non_serialized())
    {
        return;
    }

    switch (get_kind())
    {
        default:
            break;
        case TK_ALIAS:
        {
            get_base_type()->serialize_empty_data(cdr);
            break;
        }
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
        case TK_BITMASK:
        {
            size_t type_size = get_size();
            switch (type_size)
            {
                case 1: cdr << static_cast<uint8_t>(0); break;
                case 2: cdr << static_cast<uint16_t>(0); break;
                case 3: cdr << static_cast<uint32_t>(0); break;
                case 4: cdr << static_cast<uint64_t>(0); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
            break;
        }
        case TK_UNION:
        {
            cdr << MEMBER_ID_INVALID;
            break;
        }
        case TK_SEQUENCE: // Sequence is like structure, but with size
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_BITSET:
        case TK_STRUCTURE:
        {
            // delegate in base clases if any
            if (base_type_)
            {
                base_type_->serialize_empty_data(cdr);
            }

            for (const DynamicTypeMemberImpl& m : members_)
            {
                if (!m.annotation_is_non_serialized())
                {
                    m.get_type()->serialize_empty_data(cdr);
                }
            }
            break;
        }
        case TK_ARRAY:
        {
            uint32_t arraySize = get_total_bounds();
            //cdr << arraySize;
            for (uint32_t i = 0; i < arraySize; ++i)
            {
                get_element_type()->serialize_empty_data(cdr);
            }
            break;
        }
        case TK_MAP:
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
