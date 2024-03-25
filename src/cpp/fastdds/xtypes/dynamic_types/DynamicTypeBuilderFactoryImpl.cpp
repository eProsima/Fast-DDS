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

#include "DynamicTypeBuilderFactoryImpl.hpp"

#include "DynamicTypeBuilderImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

traits<DynamicTypeBuilderFactoryImpl>::ref_type DynamicTypeBuilderFactoryImpl::instance_;

//{{{ Functions to create types

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    auto descriptor_impl = traits<TypeDescriptor>::narrow<TypeDescriptorImpl>(descriptor);

    if (descriptor_impl->is_consistent())
    {
        return std::make_shared<DynamicTypeBuilderImpl>(*descriptor_impl);
    }

    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type_copy(
        traits<DynamicType>::ref_type type) noexcept
{
    auto ret_val = std::make_shared<DynamicTypeBuilderImpl>(TypeDescriptorImpl{TK_NONE, ""});
    ret_val->copy_from(traits<DynamicType>::narrow<DynamicTypeImpl>(type));
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type_w_document(
        const std::string& document,
        const std::string& type_name,
        const IncludePathSeq& include_paths) noexcept
{
    traits<DynamicTypeBuilder>::ref_type nil;
    static_cast<void>(document);
    static_cast<void>(type_name);
    static_cast<void>(include_paths);
    return nil;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type_w_type_object(
        const xtypes::TypeObject& type_object) noexcept
{
    traits<DynamicTypeBuilder>::ref_type nil;
    static_cast<void>(type_object);
    return nil;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type_w_uri(
        const std::string& document_url,
        const std::string& type_name,
        const IncludePathSeq& include_paths) noexcept
{
    traits<DynamicTypeBuilder>::ref_type nil;
    static_cast<void>(document_url);
    static_cast<void>(type_name);
    static_cast<void>(include_paths);
    return nil;
}

//}}}

//{{{ Functions to create specific types

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_array_type(
        traits<DynamicType>::ref_type element_type,
        const BoundSeq& bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_ARRAY, ""})};
    ret_val->get_descriptor().element_type(element_type);
    ret_val->get_descriptor().bound() = bound;

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }
    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_bitmask_type(
        uint32_t bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_BITMASK, ""})};
    ret_val->get_descriptor().element_type(bool_type_);
    ret_val->get_descriptor().bound().push_back(bound);

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }

    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_map_type(
        traits<DynamicType>::ref_type key_element_type,
        traits<DynamicType>::ref_type element_type,
        uint32_t bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_MAP, ""})};
    ret_val->get_descriptor().key_element_type(key_element_type);
    ret_val->get_descriptor().element_type(element_type);
    ret_val->get_descriptor().bound().push_back(bound);

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }

    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_sequence_type(
        traits<DynamicType>::ref_type element_type,
        uint32_t bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_SEQUENCE, ""})};
    ret_val->get_descriptor().element_type(element_type);
    ret_val->get_descriptor().bound().push_back(bound);

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }
    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_string_type(
        uint32_t bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_STRING8, ""})};
    ret_val->get_descriptor().element_type(char8_type_);
    ret_val->get_descriptor().bound().push_back(bound);

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }

    return {};
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_wstring_type(
        uint32_t bound) noexcept
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val {std::make_shared<DynamicTypeBuilderImpl>(
                                                          TypeDescriptorImpl{TK_STRING16, ""})};
    ret_val->get_descriptor().element_type(char16_type_);
    ret_val->get_descriptor().bound().push_back(bound);

    if (ret_val->get_descriptor().is_consistent())
    {
        return ret_val;
    }

    return {};
}

//}}}

ReturnCode_t DynamicTypeBuilderFactoryImpl::delete_instance() noexcept
{
    if (!instance_)
    {
        return RETCODE_BAD_PARAMETER;
    }
    instance_.reset();
    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderFactoryImpl::delete_type(
        traits<DynamicType>::ref_type type) noexcept
{
    type.reset();
    return RETCODE_OK;
}

traits<DynamicTypeBuilderFactory>::ref_type DynamicTypeBuilderFactoryImpl::get_instance() noexcept
{
    if (!instance_)
    {
        instance_ = std::make_shared<DynamicTypeBuilderFactoryImpl>();
    }

    return instance_;
}

traits<DynamicType>::ref_type DynamicTypeBuilderFactoryImpl::get_primitive_type(
        TypeKind kind) noexcept
{
    traits<DynamicTypeImpl>::ref_type ret_val;

    switch (kind)
    {
        case TK_BOOLEAN:
            ret_val = bool_type_;
            break;
        case TK_BYTE:
            ret_val = byte_type_;
            break;
        case TK_INT16:
            ret_val = int16_type_;
            break;
        case TK_INT32:
            ret_val = int32_type_;
            break;
        case TK_INT64:
            ret_val = int64_type_;
            break;
        case TK_UINT16:
            ret_val = uint16_type_;
            break;
        case TK_UINT32:
            ret_val = uint32_type_;
            break;
        case TK_UINT64:
            ret_val = uint64_type_;
            break;
        case TK_FLOAT32:
            ret_val = float32_type_;
            break;
        case TK_FLOAT64:
            ret_val = float64_type_;
            break;
        case TK_FLOAT128:
            ret_val = float128_type_;
            break;
        case TK_INT8:
            ret_val = int8_type_;
            break;
        case TK_UINT8:
            ret_val = uint8_type_;
            break;
        case TK_CHAR8:
            ret_val = char8_type_;
            break;
        case TK_CHAR16:
            ret_val = char16_type_;
            break;
        default:
            break;
    }

    return ret_val;
}

traits<DynamicTypeBuilderFactory>::ref_type DynamicTypeBuilderFactoryImpl::_this()
{
    return shared_from_this();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
