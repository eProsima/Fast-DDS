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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERFACTORYIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERFACTORYIMPL_HPP

#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include "DynamicTypeImpl.hpp"


namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * This class is conceived as a singleton in charged of creating the @ref DynamicTypeBuilderImpl objects.
 * For simplicity direct primitive types instantiation is also possible.
 */
class DynamicTypeBuilderFactoryImpl : public traits<DynamicTypeBuilderFactory>::base_type
{
public:

    static traits<DynamicTypeBuilderFactory>::ref_type get_instance() noexcept;

    static ReturnCode_t delete_instance() noexcept;

    traits<DynamicType>::ref_type get_primitive_type(
            TypeKind kind) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_type(
            traits<TypeDescriptor>::ref_type descriptor) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_type_copy(
            traits<DynamicType>::ref_type type) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_type_w_type_object(
            const xtypes::TypeObject& type_object) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_string_type(
            uint32_t bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_wstring_type(
            uint32_t bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_sequence_type(
            traits<DynamicType>::ref_type element_type,
            uint32_t bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_array_type(
            traits<DynamicType>::ref_type element_type,
            const BoundSeq& bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_map_type(
            traits<DynamicType>::ref_type key_element_type,
            traits<DynamicType>::ref_type element_type,
            uint32_t bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_bitmask_type(
            uint32_t bound) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_type_w_uri(
            const std::string& document_url,
            const std::string& type_name,
            const IncludePathSeq& include_paths) noexcept override;

    traits<DynamicTypeBuilder>::ref_type create_type_w_document(
            const std::string& document,
            const std::string& type_name,
            const IncludePathSeq& include_paths) noexcept override;

    ReturnCode_t delete_type(
            traits<DynamicType>::ref_type type) noexcept override;

private:

    static traits<DynamicTypeBuilderFactoryImpl>::ref_type instance_;

    // Cached primitive types.
    traits<DynamicTypeImpl>::ref_type bool_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_BOOLEAN,
                                                                                                       ""})};
    traits<DynamicTypeImpl>::ref_type byte_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_BYTE, ""})};
    traits<DynamicTypeImpl>::ref_type int16_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_INT16, ""})};
    traits<DynamicTypeImpl>::ref_type int32_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_INT32, ""})};
    traits<DynamicTypeImpl>::ref_type int64_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_INT64, ""})};
    traits<DynamicTypeImpl>::ref_type uint16_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_UINT16,
                                                                                                         ""})};
    traits<DynamicTypeImpl>::ref_type uint32_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_UINT32,
                                                                                                         ""})};
    traits<DynamicTypeImpl>::ref_type uint64_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_UINT64,
                                                                                                         ""})};
    traits<DynamicTypeImpl>::ref_type float32_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_FLOAT32,
                                                                                                          ""})};
    traits<DynamicTypeImpl>::ref_type float64_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_FLOAT64,
                                                                                                          ""})};
    traits<DynamicTypeImpl>::ref_type float128_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_FLOAT128,
                                                                                                           ""})};
    traits<DynamicTypeImpl>::ref_type int8_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_INT8, ""})};
    traits<DynamicTypeImpl>::ref_type uint8_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_UINT8, ""})};
    traits<DynamicTypeImpl>::ref_type char8_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_CHAR8, ""})};
    traits<DynamicTypeImpl>::ref_type char16_type_ {std::make_shared<DynamicTypeImpl>(TypeDescriptorImpl{TK_CHAR16,
                                                                                                         ""})};

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTYPEBUILDERFACTORYIMPL_HPP
