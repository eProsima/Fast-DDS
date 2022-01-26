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

#include <fastdds/dds/log/Log.hpp>

#include "pegtl.hpp"
#include "pegtl/contrib/parse_tree.hpp"

#include "DDSFilterGrammar.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {
namespace parser {

using namespace tao::TAO_PEGTL_NAMESPACE;

// after a node is stored successfully, you can add an optional transformer like this:
struct rearrange
    : parse_tree::apply< rearrange >  // allows bulk selection, see selector<...>
{
    // recursively rearrange nodes. the basic principle is:
    //
    // from:          PROD/EXPR
    //                /   |   \          (LHS... may be one or more children, followed by OP,)
    //             LHS... OP   RHS       (which is one operator, and RHS, which is a single child)
    //
    // to:               OP
    //                  /  \             (OP now has two children, the original PROD/EXPR and RHS)
    //         PROD/EXPR    RHS          (Note that PROD/EXPR has two fewer children now)
    //             |
    //            LHS...
    //
    // if only one child is left for LHS..., replace the PROD/EXPR with the child directly.
    // otherwise, perform the above transformation, then apply it recursively until LHS...
    // becomes a single child, which then replaces the parent node and the recursion ends.
    template< typename ... States >
    static void transform(
            std::unique_ptr< parse_tree::node >& n,
            States&&... st)
    {
        if (n->children.size() == 1)
        {
            n = std::move(n->children.back());
        }
        else
        {
            n->remove_content();
            auto& c = n->children;
            auto r = std::move(c.back());
            c.pop_back();
            auto o = std::move(c.back());
            c.pop_back();
            o->children.emplace_back(std::move(n));
            o->children.emplace_back(std::move(r));
            n = std::move(o);
            transform(n->children.front(), st ...);
        }
    }

};

// select which rules in the grammar will produce parse tree nodes:
template< typename Rule >
using selector = parse_tree::selector <
    Rule,
    parse_tree::store_content::on<
        boolean_value,
        integer_value,
        float_value,
        char_value,
        string_value,
        parameter_value,
        index_part,
        identifier>,
    parse_tree::remove_content::on<
        fieldname_part,
        eq_op,
        gt_op,
        ge_op,
        lt_op,
        le_op,
        ne_op,
        like_op,
        and_op,
        or_op,
        not_op,
        dot_op,
        between_op,
        not_between_op >,
    rearrange::on< fieldname, ComparisonPredicate, BetweenPredicate, Range, Condition, FilterExpression >>;

std::unique_ptr<parse_tree::node> parse_filter_expression(
        const char* expression)
{
    memory_input<> in(expression, "");
    try
    {
        return parse_tree::parse< FilterExpressionGrammar, selector >(in);
    }
    catch (const parse_error& e)
    {
        const auto p = e.positions.front();
        logError(DDSSQLFILTER, "PARSE ERROR: " << e.what() << std::endl
                                               << in.line_at(p) << std::endl
                                               << std::string(p.byte_in_line, ' ') << '^');
    }
    catch (const std::exception& e)
    {
        logError(DDSSQLFILTER, "ERROR '" << e.what() << "' while parsing " << expression);
    }

    return nullptr;
}

}  // namespace parser
}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSIONPARSER_HPP_
