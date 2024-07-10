// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file DDSFilterExpressionParser.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSIONPARSER_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSIONPARSER_HPP_

#include <memory>

#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "DDSFilterParseNode.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {
namespace parser {

/**
 * Performs parsing of a string containing a DDS-SQL filter expression.
 *
 * @param [in]  expression   The string to parse.
 * @param [in]  type_object  The TypeObject representing the type of the topic being filtered.
 *
 * @return nullptr when there is a parsing error.
 * @return A pointer to the root node of the AST tree for the expression.
 */
std::unique_ptr<ParseNode> parse_filter_expression(
        const char* expression,
        const std::shared_ptr<eprosima::fastdds::dds::xtypes::TypeObject>& type_object);

/**
 * Performs parsing of a string containing a literal value.
 * This method is used to perform parsing of parameter values.
 *
 * @param [in]  value  The string to parse.
 *
 * @return nullptr when there is a parsing error.
 * @return A simple tree consisting of a root node, with a single child that contains the generated DDSFilterValue.
 */
std::unique_ptr<ParseNode> parse_literal_value(
        const char* value);

}  // namespace parser
}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSIONPARSER_HPP_
