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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_KEYANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_KEYANNOTATION_HPP

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
 * @brief @key annotation
 */
class KeyAnnotation final : public BuiltinAnnotation
{
public:

    KeyAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_KEY_TAG)
        , initialized_(false)
    {
    }

    ~KeyAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                IDL_VALUE_TAG,
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN),
                IDL_TRUE_TAG);
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

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @key annotation to a TypeDescriptor.");
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
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_KEY_TAG <<
                    "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(utils::to_lower(parameters.at(IDL_VALUE_TAG)));
            descriptor->is_key(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_KEY_TAG << "': " <<
                    e.what());
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

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_KEYANNOTATION_HPP