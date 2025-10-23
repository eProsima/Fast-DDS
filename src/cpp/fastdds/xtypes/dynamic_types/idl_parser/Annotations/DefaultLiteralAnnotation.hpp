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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTLITERALANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTLITERALANNOTATION_HPP

#include <map>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#include "../IdlParserTags.hpp"
#include "BuiltinAnnotation.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief @default_literal annotation
 */
class DefaultLiteralAnnotation final : public BuiltinAnnotation
{
public:

    DefaultLiteralAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_DEFAULT_LITERAL_TAG)
    {
    }

    ~DefaultLiteralAnnotation() = default;

    bool initialize() override
    {
        // Nothing to do
        return true;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @default_literal annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(parameters);
        assert(descriptor != nullptr);
        descriptor->is_default_literal(true);
        return true;
    }

};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_DEFAULTLITERALANNOTATION_HPP