// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_TRYCONSTRUCTANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_TRYCONSTRUCTANNOTATION_HPP

#include <map>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#include "../IdlParserTags.hpp"
#include "BuiltinAnnotation.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief @try_construct annotation
 */
class TryConstructAnnotation final : public BuiltinAnnotation
{
public:

    TryConstructAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG)
        , initialized_(false)
    {
    }

    ~TryConstructAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            bool success {true};

            TypeDescriptor::_ref_type enum_type_descriptor = traits<TypeDescriptor>::make_shared();
            enum_type_descriptor->kind(TK_ENUM);
            enum_type_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG);
            DynamicTypeBuilder::_ref_type enum_builder = DynamicTypeBuilderFactory::get_instance()->create_type(
                enum_type_descriptor);
            MemberDescriptor::_ref_type enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_DISCARD_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TRIM_TAG);
            enum_builder->add_member(enum_member_descriptor);

            success &= add_declared_type(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG, enum_builder->build());
            success &= add_declared_type_member(
                IDL_VALUE_TAG,
                IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG,
                IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG);

            initialized_ = success;
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @try_construct annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG <<
                    "'.");
            return false;
        }

        const std::string& value = parameters.at(IDL_VALUE_TAG);
        if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_DISCARD_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::DISCARD);
        }
        else if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::USE_DEFAULT);
        }
        else if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TRIM_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::TRIM);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid value '" << value
                                      << "' for annotation '" << IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG << "'.");
            return false;
        }

        return true;
    }

    bool initialized_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_TRYCONSTRUCTANNOTATION_HPP