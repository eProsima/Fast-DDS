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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_EXTENSIBILITYANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_EXTENSIBILITYANNOTATION_HPP

#include <exception>
#include <map>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#include "../../TypeValueConverter.hpp"
#include "../IdlParserTags.hpp"
#include "BuiltinAnnotation.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief @extensibility annotation
 */
class ExtensibilityAnnotation final : public BuiltinAnnotation
{
public:

    ExtensibilityAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_EXTENSIBILITY_TAG)
        , initialized_(false)
    {
    }

    ~ExtensibilityAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            bool success {true};

            TypeDescriptor::_ref_type enum_type_descriptor {traits<TypeDescriptor>::make_shared()};
            enum_type_descriptor->kind(TK_ENUM);
            enum_type_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG);
            DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                            enum_type_descriptor)};
            MemberDescriptor::_ref_type enum_member_descriptor {traits<MemberDescriptor>::make_shared()};
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_FINAL_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_APPENDABLE_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_MUTABLE_TAG);
            enum_builder->add_member(enum_member_descriptor);

            success &= add_declared_type(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG, enum_builder->build());
            success &= add_declared_type_member(IDL_VALUE_TAG, IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG);

            initialized_ = success;
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_EXTENSIBILITY_TAG <<
                    "'.");
            return false;
        }

        const std::string& value = parameters.at(IDL_VALUE_TAG);
        if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_FINAL_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        }
        else if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_APPENDABLE_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::APPENDABLE);
        }
        else if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_MUTABLE_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid value '" << value
                                      << "' for annotation '" << IDL_BUILTIN_ANN_EXTENSIBILITY_TAG << "'.");
            return false;
        }

        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @extensibility annotation to a MemberDescriptor.");
        return false;
    }

    bool initialized_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_EXTENSIBILITYANNOTATION_HPP