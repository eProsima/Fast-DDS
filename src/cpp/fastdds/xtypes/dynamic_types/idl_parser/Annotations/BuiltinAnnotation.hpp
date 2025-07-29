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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BUILTIN_ANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BUILTIN_ANNOTATION_HPP

#include <string>

#include "Annotation.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief Base class for built-in annotations.
 */
class BuiltinAnnotation : public Annotation
{
public:

    BuiltinAnnotation(
            const std::string& name)
        : Annotation(name)
    {
    }

    ~BuiltinAnnotation() = default;

    bool is_builtin() const override
    {
        return true;
    }

    /**
     * @brief Configure the annotation's members and declared types.
     *
     * @return true if the annotation was successfully initialized, false otherwise.
     */
    virtual bool initialize() = 0;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BUILTIN_ANNOTATION_HPP