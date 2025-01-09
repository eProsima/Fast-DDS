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

#include <cassert>
#include <ios>
#include <sstream>
#include <string>

#include <fastcdr/xcdr/optional.hpp>

#include "DynamicTypeBuilderImpl.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <rtps/RTPSDomainImpl.hpp>

#include "idl_parser/Idl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

traits<DynamicTypeBuilderFactoryImpl>::ref_type DynamicTypeBuilderFactoryImpl::instance_;

//{{{ Utility functions

void DynamicTypeBuilderFactoryImpl::set_preprocessor(
        const std::string& preprocessor)
{
    preprocessor_ = preprocessor;
}

//}}}

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
    traits<DynamicTypeBuilder>::ref_type ret_val;

    try
    {
        xtypes::TypeObjectUtils::type_object_consistency(type_object);
        switch (type_object._d())
        {
            case xtypes::EK_COMPLETE:
                switch (type_object.complete()._d())
                {
                    case xtypes::TK_ALIAS:
                        ret_val = create_alias_type_w_complete_type_object(type_object.complete().alias_type());
                        break;
                    case xtypes::TK_ANNOTATION:
                        ret_val =
                                create_annotation_type_w_complete_type_object(type_object.complete().annotation_type());
                        break;
                    case xtypes::TK_STRUCTURE:
                        ret_val = create_structure_type_w_complete_type_object(type_object.complete().struct_type());
                        break;
                    case xtypes::TK_UNION:
                        ret_val = create_union_type_w_complete_type_object(type_object.complete().union_type());
                        break;
                    case xtypes::TK_BITSET:
                        ret_val = create_bitset_type_w_complete_type_object(type_object.complete().bitset_type());
                        break;
                    case xtypes::TK_SEQUENCE:
                        ret_val = create_sequence_type_w_complete_type_object(type_object.complete().sequence_type());
                        break;
                    case xtypes::TK_ARRAY:
                        ret_val = create_array_type_w_complete_type_object(type_object.complete().array_type());
                        break;
                    case xtypes::TK_MAP:
                        ret_val = create_map_type_w_complete_type_object(type_object.complete().map_type());
                        break;
                    case xtypes::TK_ENUM:
                        ret_val = create_enum_type_w_complete_type_object(type_object.complete().enumerated_type());
                        break;
                    case xtypes::TK_BITMASK:
                        ret_val = create_bitmask_type_w_complete_type_object(type_object.complete().bitmask_type());
                        break;
                    default:
                        break;
                }
                break;
            case xtypes::EK_MINIMAL:
                switch (type_object.minimal()._d())
                {
                    case xtypes::TK_ALIAS:
                        ret_val = create_alias_type_w_minimal_type_object(type_object.minimal().alias_type());
                        break;
                    case xtypes::TK_ANNOTATION:
                        ret_val = create_annotation_type_w_minimal_type_object(type_object.minimal().annotation_type());
                        break;
                    case xtypes::TK_STRUCTURE:
                        ret_val = create_structure_type_w_minimal_type_object(type_object.minimal().struct_type());
                        break;
                    case xtypes::TK_UNION:
                        ret_val = create_union_type_w_minimal_type_object(type_object.minimal().union_type());
                        break;
                    case xtypes::TK_BITSET:
                        ret_val = create_bitset_type_w_minimal_type_object(type_object.minimal().bitset_type());
                        break;
                    case xtypes::TK_SEQUENCE:
                        ret_val = create_sequence_type_w_minimal_type_object(type_object.minimal().sequence_type());
                        break;
                    case xtypes::TK_ARRAY:
                        ret_val = create_array_type_w_minimal_type_object(type_object.minimal().array_type());
                        break;
                    case xtypes::TK_MAP:
                        ret_val = create_map_type_w_minimal_type_object(type_object.minimal().map_type());
                        break;
                    case xtypes::TK_ENUM:
                        ret_val = create_enum_type_w_minimal_type_object(type_object.minimal().enumerated_type());
                        break;
                    case xtypes::TK_BITMASK:
                        ret_val = create_bitmask_type_w_minimal_type_object(type_object.minimal().bitmask_type());
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    catch (const xtypes::InvalidArgumentError& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent TypeObject: " << e.what());
        ret_val.reset();
    }

    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_type_w_uri(
        const std::string& document_url,
        const std::string& type_name,
        const IncludePathSeq& include_paths) noexcept
{
    traits<DynamicTypeBuilder>::ref_type ret_val;

    try
    {
        idlparser::Context context = idlparser::parse_file(document_url, type_name, include_paths, preprocessor_);
        ret_val = context.builder;
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, e.what());
        ret_val.reset();
    }

    return ret_val;
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
        traits<DynamicType>::ref_type& type) noexcept
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

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_alias_type_w_complete_type_object(
        const xtypes::CompleteAliasType& alias_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ALIAS);
    type_descriptor.name(alias_type.header().detail().type_name());
    // Aliased type
    traits<DynamicType>::ref_type base_type = base_type_from_type_identifier(alias_type.body().common().related_type());
    if (base_type)
    {
        type_descriptor.base_type(base_type);

        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

        apply_builtin_type_annotations(ret_val, alias_type.header().detail().ann_builtin());
        if (apply_custom_annotations(ret_val, alias_type.header().detail().ann_custom()))
        {
            apply_custom_annotations(ret_val, alias_type.body().ann_custom());
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent base TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_alias_type_w_minimal_type_object(
        const xtypes::MinimalAliasType& alias_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ALIAS);
    // Aliased type
    traits<DynamicType>::ref_type base_type = base_type_from_type_identifier(alias_type.body().common().related_type());
    if (base_type)
    {
        type_descriptor.base_type(base_type);

        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent base TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_annotation_type_w_complete_type_object(
        const xtypes::CompleteAnnotationType& annotation_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ANNOTATION);
    type_descriptor.name(annotation_type.header().annotation_name());

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    for (const xtypes::CompleteAnnotationParameter& parameter : annotation_type.member_seq())
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        traits<DynamicType>::ref_type type = base_type_from_type_identifier(parameter.common().member_type_id());
        if (!type)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Inconsistent annotation parameter TypeIdentifier " + parameter.name().to_string());
            ret_val.reset();
            break;
        }
        member_descriptor->type(type);
        member_descriptor->name(parameter.name());
        member_descriptor->default_value(get_annotation_parameter_value(parameter.default_value()));
        if (RETCODE_OK != ret_val->add_member(member_descriptor))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding annotation parameter " + member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_annotation_type_w_minimal_type_object(
        const xtypes::MinimalAnnotationType& annotation_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ANNOTATION);

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    for (const xtypes::MinimalAnnotationParameter& parameter : annotation_type.member_seq())
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->name(get_string_from_name_hash(parameter.name_hash()));
        traits<DynamicType>::ref_type type = base_type_from_type_identifier(parameter.common().member_type_id());
        if (!type)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Inconsistent annotation parameter TypeIdentifier " +
                    member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
        member_descriptor->type(type);
        member_descriptor->default_value(get_annotation_parameter_value(parameter.default_value()));
        if (RETCODE_OK != ret_val->add_member(member_descriptor))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding annotation parameter " + member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_structure_type_w_complete_type_object(
        const xtypes::CompleteStructType& struct_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_STRUCTURE);
    type_descriptor.name(struct_type.header().detail().type_name());
    type_descriptor.is_nested(struct_type.struct_flags() & xtypes::IS_NESTED);
    type_descriptor.extensibility_kind(struct_type.struct_flags() & xtypes::IS_FINAL ? ExtensibilityKind::FINAL :
            (struct_type.struct_flags() &
            xtypes::IS_MUTABLE ? ExtensibilityKind::MUTABLE : ExtensibilityKind::APPENDABLE));
    bool inheritance_correct {true};
    if (xtypes::TK_NONE != struct_type.header().base_type()._d())
    {
        traits<DynamicType>::ref_type base_type = base_type_from_type_identifier(struct_type.header().base_type());
        if (base_type)
        {
            type_descriptor.base_type(base_type);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent base TypeIdentifier");
            inheritance_correct = false;
        }
    }
    if (inheritance_correct)
    {
        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    }

    if (ret_val)
    {
        apply_builtin_type_annotations(ret_val, struct_type.header().detail().ann_builtin());
        if (apply_custom_annotations(ret_val, struct_type.header().detail().ann_custom()))
        {
            for (const xtypes::CompleteStructMember& member : struct_type.member_seq())
            {
                MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                member_descriptor->name(member.detail().name());
                traits<DynamicType>::ref_type type = base_type_from_type_identifier(member.common().member_type_id());
                if (!type)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES,
                            "Inconsistent struct member TypeIdentifier " +
                            member_descriptor->name().to_string());
                    ret_val.reset();
                    break;
                }
                member_descriptor->type(type);
                member_descriptor->id(member.common().member_id());
                apply_try_construct_flag(member_descriptor, member.common().member_flags());
                member_descriptor->is_key(member.common().member_flags() & xtypes::IS_KEY);
                member_descriptor->is_optional(member.common().member_flags() & xtypes::IS_OPTIONAL);
                member_descriptor->is_must_understand(member.common().member_flags() & xtypes::IS_MUST_UNDERSTAND);
                member_descriptor->is_shared(member.common().member_flags() & xtypes::IS_EXTERNAL);
                if (RETCODE_OK != ret_val->add_member(member_descriptor))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES,
                            "Error adding struct member " + member_descriptor->name().to_string());
                    ret_val.reset();
                    break;
                }
                if (!apply_custom_annotations(ret_val, member.detail().ann_custom(), member_descriptor->id()))
                {
                    break;
                }
            }
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_structure_type_w_minimal_type_object(
        const xtypes::MinimalStructType& struct_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_STRUCTURE);
    type_descriptor.is_nested(struct_type.struct_flags() & xtypes::IS_NESTED);
    type_descriptor.extensibility_kind(struct_type.struct_flags() & xtypes::IS_FINAL ? ExtensibilityKind::FINAL :
            (struct_type.struct_flags() &
            xtypes::IS_MUTABLE ? ExtensibilityKind::MUTABLE : ExtensibilityKind::APPENDABLE));
    bool inheritance_correct {true};
    if (xtypes::TK_NONE != struct_type.header().base_type()._d())
    {
        traits<DynamicType>::ref_type base_type = base_type_from_type_identifier(struct_type.header().base_type());
        if (base_type)
        {
            type_descriptor.base_type(base_type);
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent base TypeIdentifier");
            inheritance_correct = false;
        }
    }
    if (inheritance_correct)
    {
        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    }

    if (ret_val)
    {
        for (const xtypes::MinimalStructMember& member : struct_type.member_seq())
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(get_string_from_name_hash(member.detail().name_hash()));
            traits<DynamicType>::ref_type type = base_type_from_type_identifier(member.common().member_type_id());
            if (!type)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Inconsistent struct member TypeIdentifier " +
                        member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
            member_descriptor->type(type);
            member_descriptor->id(member.common().member_id());
            apply_try_construct_flag(member_descriptor, member.common().member_flags());
            member_descriptor->is_key(member.common().member_flags() & xtypes::IS_KEY);
            member_descriptor->is_optional(member.common().member_flags() & xtypes::IS_OPTIONAL);
            member_descriptor->is_must_understand(member.common().member_flags() & xtypes::IS_MUST_UNDERSTAND);
            member_descriptor->is_shared(member.common().member_flags() & xtypes::IS_EXTERNAL);
            if (RETCODE_OK != ret_val->add_member(member_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding struct member " + member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_union_type_w_complete_type_object(
        const xtypes::CompleteUnionType& union_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_UNION);
    type_descriptor.name(union_type.header().detail().type_name());
    type_descriptor.is_nested(union_type.union_flags() & xtypes::IS_NESTED);
    type_descriptor.extensibility_kind(union_type.union_flags() & xtypes::IS_FINAL ? ExtensibilityKind::FINAL :
            (union_type.union_flags() &
            xtypes::IS_MUTABLE ? ExtensibilityKind::MUTABLE : ExtensibilityKind::APPENDABLE));

    traits<DynamicType>::ref_type discriminator_type = base_type_from_type_identifier(
        union_type.discriminator().common().type_id());
    if (discriminator_type)
    {
        type_descriptor.discriminator_type(discriminator_type);
        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
        apply_builtin_type_annotations(ret_val, union_type.header().detail().ann_builtin());
        if (apply_custom_annotations(ret_val, union_type.header().detail().ann_custom()))
        {
            apply_builtin_type_annotations(ret_val, union_type.discriminator().ann_builtin());
            if (apply_custom_annotations(ret_val, union_type.discriminator().ann_custom(), 0))
            {
                for (const xtypes::CompleteUnionMember& member : union_type.member_seq())
                {
                    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                    member_descriptor->name(member.detail().name());
                    traits<DynamicType>::ref_type type = base_type_from_type_identifier(member.common().type_id());
                    if (!type)
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Inconsistent union member TypeIdentifier " +
                                member_descriptor->name().to_string());
                        ret_val.reset();
                        break;
                    }
                    member_descriptor->type(type);
                    member_descriptor->id(member.common().member_id());
                    apply_try_construct_flag(member_descriptor, member.common().member_flags());
                    member_descriptor->is_default_label(member.common().member_flags() & xtypes::IS_DEFAULT);
                    member_descriptor->is_shared(member.common().member_flags() & xtypes::IS_EXTERNAL);
                    member_descriptor->label(member.common().label_seq());
                    if (RETCODE_OK != ret_val->add_member(member_descriptor))
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Error adding union member " + member_descriptor->name().to_string());
                        ret_val.reset();
                        break;
                    }
                    if (!apply_custom_annotations(ret_val, member.detail().ann_custom(), member_descriptor->id()))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent discriminator TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_union_type_w_minimal_type_object(
        const xtypes::MinimalUnionType& union_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_UNION);
    type_descriptor.is_nested(union_type.union_flags() & xtypes::IS_NESTED);
    type_descriptor.extensibility_kind(union_type.union_flags() & xtypes::IS_FINAL ? ExtensibilityKind::FINAL :
            (union_type.union_flags() &
            xtypes::IS_MUTABLE ? ExtensibilityKind::MUTABLE : ExtensibilityKind::APPENDABLE));

    traits<DynamicType>::ref_type discriminator_type = base_type_from_type_identifier(
        union_type.discriminator().common().type_id());
    if (discriminator_type)
    {
        type_descriptor.discriminator_type(discriminator_type);
        ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
        for (const xtypes::MinimalUnionMember& member : union_type.member_seq())
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(get_string_from_name_hash(member.detail().name_hash()));
            traits<DynamicType>::ref_type type = base_type_from_type_identifier(member.common().type_id());
            if (!type)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Inconsistent union member TypeIdentifier " +
                        member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
            member_descriptor->type(type);
            member_descriptor->id(member.common().member_id());
            apply_try_construct_flag(member_descriptor, member.common().member_flags());
            member_descriptor->is_default_label(member.common().member_flags() & xtypes::IS_DEFAULT);
            member_descriptor->is_shared(member.common().member_flags() & xtypes::IS_EXTERNAL);
            member_descriptor->label(member.common().label_seq());
            if (RETCODE_OK != ret_val->add_member(member_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding union member " + member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent discriminator TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_bitset_type_w_complete_type_object(
        const xtypes::CompleteBitsetType& bitset_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_BITSET);
    type_descriptor.name(bitset_type.header().detail().type_name());

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    apply_builtin_type_annotations(ret_val, bitset_type.header().detail().ann_builtin());
    if (apply_custom_annotations(ret_val, bitset_type.header().detail().ann_custom()))
    {
        for (const xtypes::CompleteBitfield& bitfield : bitset_type.field_seq())
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(bitfield.detail().name());
            member_descriptor->type(get_primitive_type(bitfield.common().holder_type()));
            member_descriptor->id(bitfield.common().position());
            ret_val->get_descriptor().bound().push_back(bitfield.common().bitcount());
            if (RETCODE_OK != ret_val->add_member(member_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding bitfield " + member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
            if (!apply_custom_annotations(ret_val, bitfield.detail().ann_custom(), member_descriptor->id()))
            {
                break;
            }
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_bitset_type_w_minimal_type_object(
        const xtypes::MinimalBitsetType& bitset_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_BITSET);
    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    for (const xtypes::MinimalBitfield& bitfield : bitset_type.field_seq())
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->name(get_string_from_name_hash(bitfield.name_hash()));
        member_descriptor->type(get_primitive_type(bitfield.common().holder_type()));
        member_descriptor->id(bitfield.common().position());
        ret_val->get_descriptor().bound().push_back(bitfield.common().bitcount());
        if (RETCODE_OK != ret_val->add_member(member_descriptor))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding bitfield " + member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_sequence_type_w_complete_type_object(
        const xtypes::CompleteSequenceType& sequence_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_SEQUENCE);
    type_descriptor.bound().push_back(sequence_type.header().common().bound() != xtypes::INVALID_LBOUND ?
            sequence_type.header().common().bound() : LENGTH_UNLIMITED);

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    if (sequence_type.header().detail().has_value())
    {
        ret_val->get_descriptor().name(sequence_type.header().detail().value().type_name());
        apply_builtin_type_annotations(ret_val, sequence_type.header().detail().value().ann_builtin());
        apply_custom_annotations(ret_val, sequence_type.header().detail().value().ann_custom());
    }

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type =
            base_type_from_type_identifier(sequence_type.element().common().type());
    if (ret_val && element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else if (!element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_sequence_type_w_minimal_type_object(
        const xtypes::MinimalSequenceType& sequence_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_SEQUENCE);
    type_descriptor.bound().push_back(sequence_type.header().common().bound() != xtypes::INVALID_LBOUND ?
            sequence_type.header().common().bound() : LENGTH_UNLIMITED);

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type =
            base_type_from_type_identifier(sequence_type.element().common().type());
    if (element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_array_type_w_complete_type_object(
        const xtypes::CompleteArrayType& array_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ARRAY);
    type_descriptor.bound(array_type.header().common().bound_seq());

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    ret_val->get_descriptor().name(array_type.header().detail().type_name());
    apply_builtin_type_annotations(ret_val, array_type.header().detail().ann_builtin());
    apply_custom_annotations(ret_val, array_type.header().detail().ann_custom());

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(array_type.element().common().type());
    if (ret_val && element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else if (!element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_array_type_w_minimal_type_object(
        const xtypes::MinimalArrayType& array_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ARRAY);
    type_descriptor.bound(array_type.header().common().bound_seq());

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(array_type.element().common().type());
    if (element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else if (!element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_map_type_w_complete_type_object(
        const xtypes::CompleteMapType& map_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_MAP);
    type_descriptor.bound().push_back(map_type.header().common().bound() != xtypes::INVALID_LBOUND ?
            map_type.header().common().bound() : LENGTH_UNLIMITED);

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    if (map_type.header().detail().has_value())
    {
        ret_val->get_descriptor().name(map_type.header().detail().value().type_name());
        apply_builtin_type_annotations(ret_val, map_type.header().detail().value().ann_builtin());
        apply_custom_annotations(ret_val, map_type.header().detail().value().ann_custom());
    }

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(map_type.element().common().type());
    if (ret_val && element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else if (!element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }
    traits<DynamicType>::ref_type key_type = base_type_from_type_identifier(map_type.key().common().type());
    if (ret_val && key_type)
    {
        ret_val->get_descriptor().key_element_type(key_type);
    }
    else if (!key_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent key TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_map_type_w_minimal_type_object(
        const xtypes::MinimalMapType& map_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_MAP);
    type_descriptor.bound().push_back(map_type.header().common().bound() != xtypes::INVALID_LBOUND ?
            map_type.header().common().bound() : LENGTH_UNLIMITED);

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);

    // TODO(jlbueno): collection element annotations are not supported yet.
    traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(map_type.element().common().type());
    if (ret_val && element_type)
    {
        ret_val->get_descriptor().element_type(element_type);
    }
    else if (!element_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent element TypeIdentifier");
        ret_val.reset();
    }
    traits<DynamicType>::ref_type key_type = base_type_from_type_identifier(map_type.key().common().type());
    if (ret_val && key_type)
    {
        ret_val->get_descriptor().key_element_type(key_type);
    }
    else if (!key_type)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent key TypeIdentifier");
        ret_val.reset();
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_enum_type_w_complete_type_object(
        const xtypes::CompleteEnumeratedType& enum_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ENUM);
    type_descriptor.name(enum_type.header().detail().type_name());
    traits<DynamicType>::ref_type literal_type;
    if (enum_type.header().common().bit_bound() <= 8)
    {
        literal_type = get_primitive_type(TK_INT8);
    }
    else if (enum_type.header().common().bit_bound() <= 16)
    {
        literal_type = get_primitive_type(TK_INT16);
    }
    else
    {
        literal_type = get_primitive_type(TK_INT32);
    }

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    apply_builtin_type_annotations(ret_val, enum_type.header().detail().ann_builtin());
    if (apply_custom_annotations(ret_val, enum_type.header().detail().ann_custom()))
    {
        for (const xtypes::CompleteEnumeratedLiteral& literal : enum_type.literal_seq())
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(literal.detail().name());
            member_descriptor->type(literal_type);
            member_descriptor->default_value(std::to_string(literal.common().value()));
            member_descriptor->is_default_label(literal.common().flags() & xtypes::IS_DEFAULT);
            if (RETCODE_OK != ret_val->add_member(member_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Error adding enumeration literal " + member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
            // Literal annotations not supported.
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_enum_type_w_minimal_type_object(
        const xtypes::MinimalEnumeratedType& enum_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_ENUM);
    traits<DynamicType>::ref_type literal_type;
    if (enum_type.header().common().bit_bound() <= 8)
    {
        literal_type = get_primitive_type(TK_INT8);
    }
    else if (enum_type.header().common().bit_bound() <= 16)
    {
        literal_type = get_primitive_type(TK_INT16);
    }
    else
    {
        literal_type = get_primitive_type(TK_INT32);
    }

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    for (const xtypes::MinimalEnumeratedLiteral& literal : enum_type.literal_seq())
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->name(get_string_from_name_hash(literal.detail().name_hash()));
        member_descriptor->type(literal_type);
        member_descriptor->default_value(std::to_string(literal.common().value()));
        member_descriptor->is_default_label(literal.common().flags() & xtypes::IS_DEFAULT);
        if (RETCODE_OK != ret_val->add_member(member_descriptor))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding enumeration literal " + member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_bitmask_type_w_complete_type_object(
        const xtypes::CompleteBitmaskType& bitmask_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_BITMASK);
    type_descriptor.name(bitmask_type.header().detail().type_name());
    type_descriptor.bound().push_back(bitmask_type.header().common().bit_bound());
    type_descriptor.element_type(get_primitive_type(TK_BOOLEAN));

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    apply_builtin_type_annotations(ret_val, bitmask_type.header().detail().ann_builtin());
    if (apply_custom_annotations(ret_val, bitmask_type.header().detail().ann_custom()))
    {
        for (const xtypes::CompleteBitflag& bitflag : bitmask_type.flag_seq())
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(bitflag.detail().name());
            member_descriptor->type(get_primitive_type(TK_BOOLEAN));
            member_descriptor->id(bitflag.common().position());
            if (RETCODE_OK != ret_val->add_member(member_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding bitflag " + member_descriptor->name().to_string());
                ret_val.reset();
                break;
            }
            // Bitflag annotations not supported.
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderFactoryImpl::create_bitmask_type_w_minimal_type_object(
        const xtypes::MinimalBitmaskType& bitmask_type)
{
    traits<DynamicTypeBuilderImpl>::ref_type ret_val;

    TypeDescriptorImpl type_descriptor;
    type_descriptor.kind(TK_BITMASK);
    type_descriptor.bound().push_back(bitmask_type.header().common().bit_bound());
    type_descriptor.element_type(get_primitive_type(TK_BOOLEAN));

    ret_val = std::make_shared<DynamicTypeBuilderImpl>(type_descriptor);
    for (const xtypes::MinimalBitflag& bitflag : bitmask_type.flag_seq())
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->name(get_string_from_name_hash(bitflag.detail().name_hash()));
        member_descriptor->type(get_primitive_type(TK_BOOLEAN));
        member_descriptor->id(bitflag.common().position());
        if (RETCODE_OK != ret_val->add_member(member_descriptor))
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding bitflag " + member_descriptor->name().to_string());
            ret_val.reset();
            break;
        }
    }

    if (ret_val && !ret_val->get_descriptor().is_consistent())
    {
        ret_val.reset();
    }
    return ret_val;
}

traits<DynamicType>::ref_type DynamicTypeBuilderFactoryImpl::base_type_from_type_identifier(
        const xtypes::TypeIdentifier& type_identifier)
{
    traits<DynamicType>::ref_type ret_val;
    switch (type_identifier._d())
    {
        case xtypes::TK_BOOLEAN:
            ret_val = get_primitive_type(TK_BOOLEAN);
            break;
        case xtypes::TK_BYTE:
            ret_val = get_primitive_type(TK_BYTE);
            break;
        case xtypes::TK_INT8:
            ret_val = get_primitive_type(TK_INT8);
            break;
        case xtypes::TK_INT16:
            ret_val = get_primitive_type(TK_INT16);
            break;
        case xtypes::TK_INT32:
            ret_val = get_primitive_type(TK_INT32);
            break;
        case xtypes::TK_INT64:
            ret_val = get_primitive_type(TK_INT64);
            break;
        case xtypes::TK_UINT8:
            ret_val = get_primitive_type(TK_UINT8);
            break;
        case xtypes::TK_UINT16:
            ret_val = get_primitive_type(TK_UINT16);
            break;
        case xtypes::TK_UINT32:
            ret_val = get_primitive_type(TK_UINT32);
            break;
        case xtypes::TK_UINT64:
            ret_val = get_primitive_type(TK_UINT64);
            break;
        case xtypes::TK_FLOAT32:
            ret_val = get_primitive_type(TK_FLOAT32);
            break;
        case xtypes::TK_FLOAT64:
            ret_val = get_primitive_type(TK_FLOAT64);
            break;
        case xtypes::TK_FLOAT128:
            ret_val = get_primitive_type(TK_FLOAT128);
            break;
        case xtypes::TK_CHAR8:
            ret_val = get_primitive_type(TK_CHAR8);
            break;
        case xtypes::TK_CHAR16:
            ret_val = get_primitive_type(TK_CHAR16);
            break;
        case xtypes::TI_STRING8_SMALL:
            ret_val = create_string_type(type_identifier.string_sdefn().bound() != xtypes::INVALID_LBOUND ?
                            type_identifier.string_sdefn().bound() : LENGTH_UNLIMITED)->build();
            break;
        case xtypes::TI_STRING16_SMALL:
            ret_val = create_wstring_type(type_identifier.string_sdefn().bound() != xtypes::INVALID_LBOUND ?
                            type_identifier.string_sdefn().bound() : LENGTH_UNLIMITED)->build();
            break;
        case xtypes::TI_STRING8_LARGE:
            ret_val = create_string_type(type_identifier.string_ldefn().bound())->build();
            break;
        case xtypes::TI_STRING16_LARGE:
            ret_val = create_wstring_type(type_identifier.string_ldefn().bound())->build();
            break;
        // TODO(jlbueno): Collection annotations are not supported yet.
        case xtypes::TI_PLAIN_SEQUENCE_SMALL:
        {
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.seq_sdefn().element_identifier());
            ret_val = create_sequence_type(element_type, type_identifier.seq_sdefn().bound() != xtypes::INVALID_LBOUND ?
                            type_identifier.seq_sdefn().bound() : static_cast<uint32_t>(LENGTH_UNLIMITED))->build();
            break;
        }
        case xtypes::TI_PLAIN_SEQUENCE_LARGE:
        {
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.seq_ldefn().element_identifier());
            ret_val = create_sequence_type(element_type, type_identifier.seq_ldefn().bound())->build();
            break;
        }
        case xtypes::TI_PLAIN_ARRAY_SMALL:
        {
            BoundSeq array_bound_seq;
            for (xtypes::SBound bound : type_identifier.array_sdefn().array_bound_seq())
            {
                array_bound_seq.push_back(bound);
            }
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.array_sdefn().element_identifier());
            ret_val = create_array_type(element_type, array_bound_seq)->build();
            break;
        }
        case xtypes::TI_PLAIN_ARRAY_LARGE:
        {
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.array_ldefn().element_identifier());
            ret_val = create_array_type(element_type, type_identifier.array_ldefn().array_bound_seq())->build();
            break;
        }
        case xtypes::TI_PLAIN_MAP_SMALL:
        {
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.map_sdefn().element_identifier());
            traits<DynamicType>::ref_type key_type = base_type_from_type_identifier(
                *type_identifier.map_sdefn().key_identifier());
            ret_val = create_map_type(key_type, element_type,
                            type_identifier.map_sdefn().bound() != xtypes::INVALID_LBOUND ?
                            type_identifier.map_sdefn().bound() : static_cast<uint32_t>(LENGTH_UNLIMITED))->build();
            break;
        }
        case xtypes::TI_PLAIN_MAP_LARGE:
        {
            traits<DynamicType>::ref_type element_type = base_type_from_type_identifier(
                *type_identifier.map_ldefn().element_identifier());
            traits<DynamicType>::ref_type key_type = base_type_from_type_identifier(
                *type_identifier.map_ldefn().key_identifier());
            ret_val = create_map_type(key_type, element_type, type_identifier.map_ldefn().bound())->build();
            break;
        }
        case xtypes::EK_COMPLETE:
        case xtypes::EK_MINIMAL:
        {
            // Find related type object
            xtypes::TypeObject base_type_object;
            if (RETCODE_OK !=
                    fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().get_type_object(
                        type_identifier, base_type_object))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Given TypeIdentifier unknown to TypeObjectRegistry");
            }
            else
            {
                ret_val = create_type_w_type_object(base_type_object)->build();
            }
            break;
        }
        case xtypes::TK_NONE:
        case xtypes::TI_STRONGLY_CONNECTED_COMPONENT:
            break;
    }
    return ret_val;
}

bool DynamicTypeBuilderFactoryImpl::apply_custom_annotations(
        traits<DynamicTypeBuilderImpl>::ref_type& ret_val,
        const fastcdr::optional<xtypes::AppliedAnnotationSeq>& ann_custom,
        const MemberId& member_id)
{
    if (ann_custom.has_value())
    {
        for (const xtypes::AppliedAnnotation& annotation : ann_custom.value())
        {
            // Find annotation type object
            xtypes::TypeObject annotation_type_object;
            if (RETCODE_OK !=
                    fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().get_type_object(
                        annotation.annotation_typeid(), annotation_type_object))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Given annotation type identifier unknown to TypeObjectRegistry");
                ret_val.reset();
                break;
            }
            traits<AnnotationDescriptor>::ref_type annotation_descriptor {traits<AnnotationDescriptor>::make_shared()};
            annotation_descriptor->type(create_type_w_type_object(annotation_type_object)->build());
            if (annotation.param_seq().has_value())
            {
                for (const xtypes::AppliedAnnotationParameter& parameter : annotation.param_seq().value())
                {
                    ObjectName paramname = get_string_from_name_hash(parameter.paramname_hash());
                    // Search the real name of annotation parameter.
                    if (xtypes::EK_COMPLETE == annotation_type_object._d())
                    {
                        for (auto member : annotation_type_object.complete().annotation_type().member_seq())
                        {
                            xtypes::NameHash member_name_hash {xtypes::TypeObjectUtils::name_hash(
                                                                   member.name().to_string())};

                            if (parameter.paramname_hash() == member_name_hash)
                            {
                                paramname = member.name();
                                break;
                            }
                        }
                    }
                    annotation_descriptor->set_value(paramname, get_annotation_parameter_value(parameter.value()));
                }
            }
            if (member_id != MEMBER_ID_INVALID &&
                    ret_val && RETCODE_OK != ret_val->apply_annotation_to_member(member_id, annotation_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent member applied annotation");
                ret_val.reset();
                break;
            }
            if (member_id == MEMBER_ID_INVALID &&
                    ret_val && RETCODE_OK != ret_val->apply_annotation(annotation_descriptor))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistent type applied annotation");
                ret_val.reset();
                break;
            }
        }
    }
    if (ret_val)
    {
        return true;
    }
    return false;
}

void DynamicTypeBuilderFactoryImpl::apply_builtin_type_annotations(
        traits<DynamicTypeBuilderImpl>::ref_type& ret_val,
        const fastcdr::optional<xtypes::AppliedBuiltinTypeAnnotations>& ann_builtin)
{
    if (ann_builtin.has_value())
    {
        if (ann_builtin.value().verbatim().has_value())
        {
            if (ann_builtin.value().verbatim().value().language().to_string() == "C++" ||
                    ann_builtin.value().verbatim().value().language().to_string() == "c++" ||
                    ann_builtin.value().verbatim().value().language().to_string() == "*")
            {
                VerbatimTextDescriptorImpl verbatim;
                verbatim.placement(ann_builtin.value().verbatim().value().placement().to_string());
                verbatim.text(ann_builtin.value().verbatim().value().text());
                ret_val->get_verbatim().emplace_back();
                ret_val->get_verbatim().back().copy_from(verbatim);
            }
        }
    }
}

void DynamicTypeBuilderFactoryImpl::apply_try_construct_flag(
        MemberDescriptor::_ref_type& ret_val,
        const xtypes::MemberFlag& flags)
{
    if (flags & xtypes::TRY_CONSTRUCT1 && flags & xtypes::TRY_CONSTRUCT2)
    {
        ret_val->try_construct_kind(TryConstructKind::TRIM);
    }
    else if (flags & xtypes::TRY_CONSTRUCT2)
    {
        ret_val->try_construct_kind(TryConstructKind::USE_DEFAULT);
    }
    else
    {
        ret_val->try_construct_kind(TryConstructKind::DISCARD);
    }
}

std::string DynamicTypeBuilderFactoryImpl::get_annotation_parameter_value(
        const xtypes::AnnotationParameterValue& value)
{
    std::string ret_val;
    switch (value._d())
    {
        case xtypes::TK_BOOLEAN:
            ret_val = std::to_string(value.boolean_value());
            break;
        case xtypes::TK_BYTE:
            ret_val = std::to_string(value.byte_value());
            break;
        case xtypes::TK_INT8:
            ret_val = std::to_string(value.int8_value());
            break;
        case xtypes::TK_UINT8:
            ret_val = std::to_string(value.uint8_value());
            break;
        case xtypes::TK_INT16:
            ret_val = std::to_string(value.int16_value());
            break;
        case xtypes::TK_UINT16:
            ret_val = std::to_string(value.uint_16_value());
            break;
        case xtypes::TK_INT32:
            ret_val = std::to_string(value.int32_value());
            break;
        case xtypes::TK_UINT32:
            ret_val = std::to_string(value.uint32_value());
            break;
        case xtypes::TK_INT64:
            ret_val = std::to_string(value.int64_value());
            break;
        case xtypes::TK_UINT64:
            ret_val = std::to_string(value.uint64_value());
            break;
        case xtypes::TK_FLOAT32:
            ret_val = std::to_string(value.float32_value());
            break;
        case xtypes::TK_FLOAT64:
            ret_val = std::to_string(value.float64_value());
            break;
        case xtypes::TK_FLOAT128:
            ret_val = std::to_string(value.float128_value());
            break;
        case xtypes::TK_CHAR8:
            ret_val = std::to_string(value.char_value());
            break;
        case xtypes::TK_CHAR16:
            ret_val = std::to_string(value.wchar_value());
            break;
        case xtypes::TK_ENUM:
            ret_val = std::to_string(value.enumerated_value());
            break;
        case xtypes::TK_STRING8:
            ret_val = value.string8_value().to_string();
            break;
        case xtypes::TK_STRING16:
            // There is no official support in the STL to convert from wstring to string.
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "No support to create DynamicTypeBuilder with a TypeObject using custom annotations with wstring parameter");
            break;
        default:
            break;
    }
    return ret_val;
}

std::string DynamicTypeBuilderFactoryImpl::get_string_from_name_hash(
        const xtypes::NameHash& name)
{
    std::stringstream ss;
    ss << std::hex;
    ss << name[0];
    for (size_t i {1}; i < name.size(); ++i)
    {
        ss << "." << name[i];
    }
    return ss.str();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
