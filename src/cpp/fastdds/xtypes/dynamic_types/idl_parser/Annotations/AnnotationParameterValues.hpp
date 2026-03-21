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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONPARAMETERVALUES_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONPARAMETERVALUES_HPP

#include <string>
#include <map>
#include <sstream>

#include "../IdlParserUtils.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief Struct representing the input parameter values provided when annotating a type.
 *        They could be positional parameters (if the parameter's name is not specified) or
 *        keyword parameters (if the parameter's name is specified).
 */
struct AnnotationParameterValues
{
    // Constant expression representing the shortened parameter value in one-member annotations.
    std::string shortened_parameter;
    // Map containing the provided parameter values, where the key is the parameter's name
    std::map<std::string, std::string> keyword_parameters;

    /**
     * @brief Parse string input of parameter values separated by commas.
     */
    static AnnotationParameterValues from_string(
            const std::string& input)
    {
        AnnotationParameterValues result;

        std::stringstream ss(input);
        std::string param_token;

        while (getline(ss, param_token, ','))
        {
            param_token = utils::trim(param_token);
            if (param_token.empty())
            {
                continue;
            }

            // Remove escape characters in string values
            param_token = utils::remove_char(param_token, '\"');

            auto it = param_token.find("=");
            if (it != std::string::npos)
            {
                // Keyword parameter
                assert(result.shortened_parameter.empty());
                std::string key = utils::trim(param_token.substr(0, it));
                std::string value = utils::trim(param_token.substr(it + 1));
                result.keyword_parameters[key] = value;
            }
            else
            {
                assert(result.keyword_parameters.empty());
                result.shortened_parameter = param_token;
            }
        }

        return result;
    }

};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONPARAMETERVALUES_HPP