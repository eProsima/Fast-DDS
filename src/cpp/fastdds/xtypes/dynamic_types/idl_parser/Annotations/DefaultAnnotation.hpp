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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTANNOTATION_HPP

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
 * @brief @default annotation
 */
class DefaultAnnotation final : public BuiltinAnnotation
{
public:

    DefaultAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_DEFAULT_TAG)
        , initialized_(false)
    {
    }

    ~DefaultAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            // Note: The member is processed as a string, so it can be used to set the default value of any type.
            // In the future, it should be processed as a "any" type member.
            initialized_ = add_primitive_or_string_member(IDL_VALUE_TAG,
                            DynamicTypeBuilderFactory::get_instance()->create_string_type(
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
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

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @default annotation to a TypeDescriptor.");
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
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_DEFAULT_TAG <<
                    "'.");
            return false;
        }

        descriptor->default_value(parameters.at(IDL_VALUE_TAG));
        return true;
    }

    bool initialized_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTANNOTATION_HPP