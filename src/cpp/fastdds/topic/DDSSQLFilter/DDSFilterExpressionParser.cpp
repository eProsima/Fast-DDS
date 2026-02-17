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
 * @file DDSFilterExpressionParser.cpp
 */
#include "DDSFilterExpressionParser.hpp"

#include <memory>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "pegtl.hpp"
#include "pegtl/contrib/parse_tree.hpp"

#include "DDSFilterField.hpp"
#include "DDSFilterGrammar.hpp"
#include "DDSFilterParseNode.hpp"
#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {
namespace parser {

using namespace tao::TAO_PEGTL_NAMESPACE;
using namespace xtypes;

#include "DDSFilterExpressionParserImpl/rearrange.hpp"
#include "DDSFilterExpressionParserImpl/literal_values.hpp"
#include "DDSFilterExpressionParserImpl/identifiers.hpp"
#include "DDSFilterExpressionParserImpl/parameters.hpp"

// select which rules in the grammar will produce parse tree nodes:
template< typename Rule >
using selector = parse_tree::selector <
    Rule,
    literal_value_processor::on<
        true_value,
        false_value,
        hex_value,
        integer_value,
        float_value,
        char_value,
        string_value >,
    parameter_processor::on<
        parameter_value>,
    parse_tree::store_content::on<
        string_content,
        integer,
        index_part,
        identifier >,
    parse_tree::remove_content::on<
        eq_op,
        gt_op,
        ge_op,
        lt_op,
        le_op,
        ne_op,
        like_op,
        match_op,
        and_op,
        or_op,
        not_op,
        dot_op,
        between_op,
        not_between_op >,
    rearrange::on<
        boolean_value,
        ComparisonPredicate,
        BetweenPredicate,
        Range,
        Condition,
        FilterExpression >,
    identifier_processor::on<
        fieldname_part,
        fieldname >
    >;

std::unique_ptr<ParseNode> parse_filter_expression(
        const char* expression,
        const std::shared_ptr<TypeObject>& type_object)
{
    memory_input<> in(expression, "");
    try
    {
        CurrentIdentifierState identifier_state { type_object, {}, {} };
        return parse_tree::parse< FilterExpressionGrammar, ParseNode, selector >(in, identifier_state);
    }
    catch (const parse_error& e)
    {
        const auto p = e.positions.front();
        EPROSIMA_LOG_ERROR(DDSSQLFILTER, "PARSE ERROR: " << e.what() << std::endl
                                                         << in.line_at(p) << std::endl
                                                         << std::string(p.byte_in_line, ' ') << '^');
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(DDSSQLFILTER, "ERROR '" << e.what() << "' while parsing " << expression);
    }

    return nullptr;
}

std::unique_ptr<ParseNode> parse_literal_value(
        const char* expression)
{
    memory_input<> in(expression, "");
    try
    {
        CurrentIdentifierState identifier_state{ nullptr, nullptr, {} };
        return parse_tree::parse< LiteralGrammar, ParseNode, selector >(in, identifier_state);
    }
    catch (const parse_error& e)
    {
        const auto p = e.positions.front();
        EPROSIMA_LOG_ERROR(DDSSQLFILTER, "PARSE ERROR: " << e.what() << std::endl
                                                         << in.line_at(p) << std::endl
                                                         << std::string(p.byte_in_line, ' ') << '^');
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(DDSSQLFILTER, "ERROR '" << e.what() << "' while parsing " << expression);
    }

    return nullptr;
}

}  // namespace parser
}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
