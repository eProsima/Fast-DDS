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

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>

#include <fastcdr/Cdr.h>

using namespace eprosima::fastrtps::types;

DynamicType::DynamicType(
        use_the_create_method)
{
}

DynamicType::DynamicType(
        use_the_create_method,
        const TypeDescriptor& descriptor)
{
    // TODO BARR: refactor
    (void)descriptor;
//    descriptor_ = new TypeDescriptor(descriptor);
//    try
//    {
//        name_ = descriptor->get_name();
//        kind_ = descriptor->get_kind();
//    }
//    catch (...)
//    {
//        name_ = "";
//        kind_ = TK_NONE;
//    }
//
//    // Alias types use the same members than it's base class.
//    if (kind_ == TK_ALIAS)
//    {
//        for (auto it = descriptor_->get_base_type()->member_by_id_.begin();
//                it != descriptor_->get_base_type()->member_by_id_.end(); ++it)
//        {
//            member_by_name_.insert(std::make_pair(it->second->get_name(), it->second));
//        }
//    }
}

DynamicType::~DynamicType()
{
    clear();
}

void DynamicType::clear()
{
    TypeDescriptor::clean();
}

bool DynamicType::exists_member_by_name(
        const std::string& name) const
{
    if (get_base_type() != nullptr)
    {
        if (get_base_type()->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_name_.find(name) != member_by_name_.end();
}

bool DynamicType::equals(
        const DynamicType& other) const
{
    return get_type_descriptor() == other.get_type_descriptor();
}

bool DynamicType::has_children() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_MAP || kind_ == TK_SEQUENCE
           || kind_ == TK_STRUCTURE || kind_ == TK_UNION || kind_ == TK_BITSET;
}

bool DynamicType::is_complex_kind() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_BITMASK || kind_ == TK_ENUM
           || kind_ == TK_MAP || kind_ == TK_SEQUENCE || kind_ == TK_STRUCTURE || kind_ == TK_UNION ||
           kind_ == TK_BITSET;
}

bool DynamicType::is_discriminator_type() const
{
    if (kind_ == TK_ALIAS && get_base_type())
    {
        return get_base_type()->is_discriminator_type();
    }
    return kind_ == TK_BOOLEAN || kind_ == TK_BYTE || kind_ == TK_INT16 || kind_ == TK_INT32 ||
           kind_ == TK_INT64 || kind_ == TK_UINT16 || kind_ == TK_UINT32 || kind_ == TK_UINT64 ||
           kind_ == TK_FLOAT32 || kind_ == TK_FLOAT64 || kind_ == TK_FLOAT128 || kind_ == TK_CHAR8 ||
           kind_ == TK_CHAR16 || kind_ == TK_STRING8 || kind_ == TK_STRING16 || kind_ == TK_ENUM || kind_ == TK_BITMASK;
}

size_t DynamicType::get_size() const
{
    switch (kind_)
    {
        case TK_BOOLEAN: case TK_BYTE: case TK_CHAR8: return 1;
        case TK_INT16: case TK_UINT16: case TK_CHAR16:  return 2;
        case TK_INT32: case TK_UINT32: case TK_FLOAT32: return 4;
        case TK_INT64: case TK_UINT64: case TK_FLOAT64: return 8;
        case TK_FLOAT128: return 16;
        case TK_BITMASK: case TK_ENUM:
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
    }
    EPROSIMA_LOG_ERROR(DYN_TYPES, "Called get_size() within a non primitive type! This is a program's logic error.");
    return 0;
}

bool DynamicType::deserialize(
        DynamicData& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    if (get_type_descriptor().annotation_is_non_serialized())
    {
        return true;
    }

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
            cdr >> *(int32_t*)it->second;
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint32_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((uint32_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.int16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((int16_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((uint16_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.int64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((int64_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((uint64_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.float32_value_;
#else
            auto it = data.values_.begin();
            cdr >> *((float*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> fdata.loat64_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((double*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT128:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> fdata.loat128_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((long double*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> cdata.har8_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((char*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.char16_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((wchar_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BOOLEAN:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.bool_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((bool*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BYTE:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.byte_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((octet*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.string_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((std::string*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.wstring_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((std::wstring*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ENUM:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr >> data.uint32_value_;

#else
            auto it = data.values_.begin();
            cdr >> *((uint32_t*)it->second);
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
                case 1: cdr >> *((uint8_t*)it->second); break;
                case 2: cdr >> *((uint16_t*)it->second); break;
                case 3: cdr >> *((uint32_t*)it->second); break;
                case 4: cdr >> *((uint64_t*)it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UNION:
        {
            deserialize_discriminator(data.discriminator_value_, cdr);
            data.update_union_discriminator();
            data.set_union_id(data.union_id_);
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
                    ((DynamicData*)it->second)->deserialize(cdr);
                }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            }
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            //uint32_t size(static_cast<uint32_t>(data.complex_values_.size())), memberId(MEMBER_ID_INVALID);
            for (uint32_t i = 0; i < data.complex_values_.size(); ++i)
            {
                //cdr >> memberId;
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(i));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.complex_values_.find(i);
                        if (it != data.complex_values_.end())
                        {
                            it->second->deserialize(cdr);
                        }
                        else
                        {
                            DynamicData* pData = DynamicDataFactory::get_instance()->create_data(
                                get_element_type());
                            pData->deserialize(cdr);
                            data.complex_values_.insert(std::make_pair(i, pData));
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << i);
                }
            }
#else
            //uint32_t size(static_cast<uint32_t>(data.values_.size())), memberId(MEMBER_ID_INVALID);
            for (uint32_t i = 0; i < data.values_.size(); ++i)
            {
                //cdr >> memberId;
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(i));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.values_.find(i);
                        if (it != data.values_.end())
                        {
                            ((DynamicData*)it->second)->deserialize(cdr);
                        }
                        else
                        {
                            DynamicData* pData = DynamicDataFactory::get_instance()->create_data(
                                get_element_type());
                            pData->deserialize(cdr);
                            data.values_.insert(std::make_pair(i, pData));
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << i);
                }
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
        }
        break;
        case TK_ARRAY:
        {
            uint32_t size(get_total_bounds());
            if (size > 0)
            {
                DynamicData* inputData(nullptr);
                for (uint32_t i = 0; i < size; ++i)
                {
#ifdef DYNAMIC_TYPES_CHECKING
                    auto it = data.complex_values_.find(i);
                    if (it != data.complex_values_.end())
                    {
                        it->second->deserialize(cdr);
                    }
                    else
                    {
                        if (inputData == nullptr)
                        {
                            inputData = DynamicDataFactory::get_instance()->create_data(get_element_type());
                        }

                        inputData->deserialize(cdr);
                        if (!inputData->equals(data.default_array_value_))
                        {
                            data.complex_values_.insert(std::make_pair(i, inputData));
                            inputData = nullptr;
                        }
                    }
#else
                    auto it = data.values_.find(i);
                    if (it != data.values_.end())
                    {
                        ((DynamicData*)it->second)->deserialize(cdr);
                    }
                    else
                    {
                        if (inputData == nullptr)
                        {
                            inputData = DynamicDataFactory::get_instance()->create_data(get_element_type());
                        }

                        inputData->deserialize(cdr);
                        if (!inputData->equals(data.default_array_value_))
                        {
                            data.values_.insert(std::make_pair(i, inputData));
                            inputData = nullptr;
                        }
                    }
#endif // ifdef DYNAMIC_TYPES_CHECKING
                }
                if (inputData != nullptr)
                {
                    DynamicDataFactory::get_instance()->delete_data(inputData);
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
                auto it = data.complex_values_.find(i);
                if (it != data.complex_values_.end())
                {
                    it->second->deserialize(cdr);
                    it->second->data.key_element_ = bKeyElement;
                }
                else
                {
                    DynamicData* pData = nullptr;
                    if (bKeyElement)
                    {
                        pData = DynamicDataFactory::get_instance()->create_data(get_key_element_type());
                    }
                    else
                    {
                        pData = DynamicDataFactory::get_instance()->create_data(get_element_type());
                    }
                    pData->deserialize(cdr);
                    pData->key_element_ = bKeyElement;
                    data.complex_values_.insert(std::make_pair(i, pData));
                }
#else
                auto it = data.values_.find(i);
                if (it != data.values_.end())
                {
                    ((DynamicData*)it->second)->deserialize(cdr);
                    ((DynamicData*)it->second)->key_element_ = bKeyElement;
                }
                else
                {
                    DynamicData* pData = nullptr;
                    if (bKeyElement)
                    {
                        pData = DynamicDataFactory::get_instance()->create_data(get_key_element_type());
                    }
                    else
                    {
                        pData = DynamicDataFactory::get_instance()->create_data(get_element_type());
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
            break;
    }
    return true;

}

bool DynamicType::deserialize_discriminator(
        uint64_t& discriminator_value,
        eprosima::fastcdr::Cdr& cdr) const
{
    switch (get_kind())
    {
        case TK_INT32:
        {
            int32_t aux;
            cdr >> aux;
            discriminator_value = static_cast<int32_t>(aux);
            break;
        }
        case TK_UINT32:
        {
            uint32_t aux;
            cdr >> aux;
            discriminator_value = static_cast<uint32_t>(aux);
            break;
        }
        case TK_INT16:
        {
            int16_t aux;
            cdr >> aux;
            discriminator_value = static_cast<int16_t>(aux);
            break;
        }
        case TK_UINT16:
        {
            uint16_t aux;
            cdr >> aux;
            discriminator_value = static_cast<uint16_t>(aux);
            break;
        }
        case TK_INT64:
        {
            int64_t aux;
            cdr >> aux;
            discriminator_value = static_cast<int64_t>(aux);
            break;
        }
        case TK_UINT64:
        {
            uint64_t aux;
            cdr >> aux;
            discriminator_value = static_cast<uint64_t>(aux);
            break;
        }
        case TK_CHAR8:
        {
            char aux;
            cdr >> aux;
            discriminator_value = static_cast<char>(aux);
            break;
        }
        case TK_CHAR16:
        {
            wchar_t aux;
            cdr >> aux;
            discriminator_value = static_cast<wchar_t>(aux);
            break;
        }
        case TK_BOOLEAN:
        {
            bool aux;
            cdr >> aux;
            discriminator_value = static_cast<bool>(aux);
            break;
        }
        case TK_BYTE:
        {
            octet aux;
            cdr >> aux;
            discriminator_value = static_cast<octet>(aux);
            break;
        }
        case TK_ENUM:
        {
            uint32_t aux;
            cdr >> aux;
            discriminator_value = static_cast<uint32_t>(aux);
            break;
        }
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_STRING8:
        case TK_STRING16:
        case TK_BITMASK:
        case TK_UNION:
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_ARRAY:
        case TK_SEQUENCE:
        case TK_MAP:
        case TK_ALIAS:
        default:
            break;
    }
    return true;

}

size_t DynamicType::getCdrSerializedSize(
        const DynamicData& data,
        size_t current_alignment /*= 0*/) const
{
    if (data.type_ != nullptr && get_descriptor().annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (data.get_kind())
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
                    ((std::string*)it->second)->length() + 1;
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
                    (((std::wstring*)it->second)->length() * 4);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UNION:
        {
            // Union discriminator
            current_alignment += getCdrSerializedSize(data.union_discriminator_, current_alignment);

            if (data.union_id_ != MEMBER_ID_INVALID)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.at(data.union_id_);
#else
                auto it = (DynamicData*)data.values_.at(data.union_id_);
#endif // ifdef DYNAMIC_TYPES_CHECKING
                current_alignment += getCdrSerializedSize(it, current_alignment);
            }
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            //for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
            //{
            //    current_alignment += getCdrSerializedSize(it->second, current_alignment);
            //}
            for (uint32_t i = 0; i < data.complex_values_.size(); ++i)
            {
                //cdr >> memberId;
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(i));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.complex_values_.find(i);
                        if (it != data.complex_values_.end())
                        {
                            current_alignment += getCdrSerializedSize(it->second, current_alignment);
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << i);
                }
            }

#else
            //for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
            //{
            //    current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
            //}
            for (uint32_t i = 0; i < data.values_.size(); ++i)
            {
                //cdr >> memberId;
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(i));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.values_.find(i);
                        if (it != data.values_.end())
                        {
                            current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << i);
                }
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ARRAY:
        {
            uint32_t arraySize = get_total_bounds();
            size_t emptyElementSize =
                    get_element_type()->getEmptyCdrSerializedSize(current_alignment);
            for (uint32_t idx = 0; idx < arraySize; ++idx)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(idx);
                if (it != data.complex_values_.end())
#else
                auto it = data.values_.find(idx);
                if (it != data.values_.end())
#endif // ifdef DYNAMIC_TYPES_CHECKING
                {
                    // Element Size
                    current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
                }
                else
                {
                    current_alignment += emptyElementSize;
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
            for (auto it = data.complex_values_.begin(); it != data.complex_values_.end(); ++it)
            {
                // Element Size
                current_alignment += getCdrSerializedSize(it->second, current_alignment);
            }
#else
            for (auto it = data.values_.begin(); it != data.values_.end(); ++it)
            {
                // Element Size
                current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ALIAS:
            break;
    }

    return current_alignment - initial_alignment;
}

size_t DynamicType::getEmptyCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    if (get_descriptor().annotation_is_non_serialized())
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
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            for (const DynamicTypeMember& m : members_)
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

size_t DynamicType::getKeyMaxCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    size_t initial_alignment = current_alignment;

    // Structures check the the size of the key for their children
    if (get_kind() == TK_STRUCTURE || get_kind() == TK_BITSET)
    {
        for (const DynamicTypeMember& m : members_)
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

size_t DynamicType::getMaxCdrSerializedSize(
        size_t current_alignment /*= 0*/) const
{
    if (get_descriptor().annotation_is_non_serialized())
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
            for (const DynamicTypeMember& m : members_)
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
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            for (const DynamicTypeMember& m : members_)
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

void DynamicType::serialize(
        const DynamicData& data,
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
        case TK_INT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int32_value_;
#else
            auto it = data.values_.begin();
            cdr << *((int32_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint32_value_;
#else
            auto it = data.values_.begin();
            cdr << *((uint32_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int16_value_;
#else
            auto it = data.values_.begin();
            cdr << *((int16_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint16_value_;
#else
            auto it = data.values_.begin();
            cdr << *((uint16_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_INT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.int64_value_;
#else
            auto it = data.values_.begin();
            cdr << *((int64_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UINT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint64_value_;
#else
            auto it = data.values_.begin();
            cdr << *((uint64_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT32:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float32_value_;
#else
            auto it = data.values_.begin();
            cdr << *((float*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT64:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float64_value_;
#else
            auto it = data.values_.begin();
            cdr << *((double*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_FLOAT128:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.float128_value_;
#else
            auto it = data.values_.begin();
            cdr << *((long double*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.char8_value_;
#else
            auto it = data.values_.begin();
            cdr << *((char*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_CHAR16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.char16_value_;
#else
            auto it = data.values_.begin();
            cdr << *((wchar_t*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BOOLEAN:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.bool_value_;
#else
            auto it = data.values_.begin();
            cdr << *((bool*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_BYTE:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.byte_value_;
#else
            auto it = data.values_.begin();
            cdr << *((octet*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING8:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.string_value_;
#else
            auto it = data.values_.begin();
            cdr << *((std::string*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRING16:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.wstring_value_;
#else
            auto it = data.values_.begin();
            cdr << *((std::wstring*)it->second);
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ENUM:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            cdr << data.uint32_value_;
#else
            auto it = data.values_.begin();
            cdr << *((uint32_t*)it->second);
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
                case 1: cdr << *((uint8_t*)it->second); break;
                case 2: cdr << *((uint16_t*)it->second); break;
                case 3: cdr << *((uint32_t*)it->second); break;
                case 4: cdr << *((uint64_t*)it->second); break;
                default: EPROSIMA_LOG_ERROR(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_UNION:
        {
            serialize_discriminator(*data.union_discriminator_, cdr);
            //cdr << data.union_id_;
            if (data.union_id_ != MEMBER_ID_INVALID)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.at(data.union_id_);
#else
                auto it = (DynamicData*) data.values_.at(data.union_id_);
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
                auto it = data.complex_values_.at(idx);
                it->serialize(cdr);
            }
#else
            cdr << static_cast<uint32_t>(data.values_.size());
            for (uint32_t idx = 0; idx < static_cast<uint32_t>(data.values_.size()); ++idx)
            {
                auto it = data.values_.at(idx);
                ((DynamicData*)it)->serialize(cdr);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
#ifdef DYNAMIC_TYPES_CHECKING
            for (uint32_t idx = 0; idx < static_cast<uint32_t>(data.complex_values_.size()); ++idx)
            {
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(i));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.complex_values_.at(idx);
                        it->serialize(cdr);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << idx);
                }
            }
#else
            for (uint32_t idx = 0; idx < static_cast<uint32_t>(data.values_.size()); ++idx)
            {
                const DynamicTypeMember* member_desc;
                bool found;

                std::tie(member_desc, found) = get_member(get_member_id_at_index(idx));

                if (found)
                {
                    if (!member_desc->annotation_is_non_serialized())
                    {
                        auto it = data.values_.at(idx);
                        ((DynamicData*)it)->serialize(cdr);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Missing MemberDescriptor " << idx);
                }
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ARRAY:
        {
            uint32_t arraySize = get_total_bounds();
            for (uint32_t idx = 0; idx < arraySize; ++idx)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = data.complex_values_.find(idx);
                if (it != data.complex_values_.end())
#else
                auto it = data.values_.find(idx);
                if (it != data.values_.end())
#endif // ifdef DYNAMIC_TYPES_CHECKING
                {
                    ((DynamicData*)it->second)->serialize(cdr);
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
                ((DynamicData*)it->second)->serialize(cdr);
            }
#endif // ifdef DYNAMIC_TYPES_CHECKING
            break;
        }
        case TK_ALIAS:
            break;
    }
}

void DynamicType::serialize_discriminator(
        DynamicData& data,
        eprosima::fastcdr::Cdr& cdr) const
{
    switch (get_kind())
    {
        case TK_INT32:
        {
            int32_t aux = static_cast<int32_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_UINT32:
        {
            uint32_t aux = static_cast<uint32_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_INT16:
        {
            int16_t aux = static_cast<int16_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_UINT16:
        {
            uint16_t aux = static_cast<uint16_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_INT64:
        {
            int64_t aux = static_cast<int64_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_UINT64:
        {
            uint64_t aux = static_cast<uint64_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_CHAR8:
        {
            char aux = static_cast<char>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_CHAR16:
        {
            wchar_t aux = static_cast<wchar_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_BOOLEAN:
        {
            bool aux = !!(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_BYTE:
        {
            octet aux = static_cast<octet>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_ENUM:
        {
            uint32_t aux = static_cast<uint32_t>(data.discriminator_value_);
            cdr << aux;
            break;
        }
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_STRING8:
        case TK_STRING16:
        case TK_BITMASK:
        case TK_UNION:
        case TK_SEQUENCE:
        case TK_STRUCTURE:
        case TK_BITSET:
        case TK_ARRAY:
        case TK_MAP:
        case TK_ALIAS:
        default:
            break;
    }
}

void DynamicType::serializeKey(
        const DynamicData& data,
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
            auto& cdata = *static_cast<DynamicData*>(it->second);
            cdata.type_->serializeKey(cdata, cdr);
        }
#endif // ifdef DYNAMIC_TYPES_CHECKING
    }
    else if (is_key_defined_)
    {
        serialize(data, cdr);
    }
}

void DynamicType::serialize_empty_data(
        eprosima::fastcdr::Cdr& cdr) const
{
    if (get_descriptor().annotation_is_non_serialized())
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
            cdr << static_cast<uint32_t>(MEMBER_ID_INVALID);
            break;
        }
        case TK_SEQUENCE: // Sequence is like structure, but with size
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            for (const DynamicTypeMember& m : members_)
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
