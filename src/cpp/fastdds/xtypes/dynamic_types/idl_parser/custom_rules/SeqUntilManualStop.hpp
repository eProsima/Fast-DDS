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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_CUSTOM_RULES_SEQUNTILMANUALSTOP_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_CUSTOM_RULES_SEQUNTILMANUALSTOP_HPP

#include "pegtl.hpp"

#include "../IdlParserContext.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

using namespace tao::TAO_PEGTL_NAMESPACE;

/**
 * @brief Returns true if some context is provided and it is configured to stop parsing.
 * Otherwise, returns false.
 */
struct manual_stop_checker
{
    using analyze_t = analysis::generic< analysis::rule_type::opt >;

    template< tao::pegtl::apply_mode A,
            tao::pegtl::rewind_mode M,
            template< typename ... > class Action,
            template< typename ... > class Control,
            typename ParseInput>
    static bool match(
            ParseInput& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& /*state*/,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        if (ctx)
        {
            return !(ctx->should_continue());
        }

        return false;
    }

};

/**
 * @brief seq_until_manual_stop< R1, R2, ..., Rn >.
 * This rule behaves like a seq< R1, R2, ..., Rn >, but after matching each Ri rule, it checks if the
 * context is configured to stop parsing. If it is, the parsing stops and the match is successful.
 * @tparam Rules The rules to be matched in sequence.
 */
template < typename ... Rules >
struct seq_until_manual_stop;

// 1-rule specialization: just match the rule
template < typename Rule >
struct seq_until_manual_stop<Rule> : Rule {};

// N-rule specialization (N > 1): match each rule and check for manual stop
// If manual stop is requested, consume the remaining input
template < typename Rule, typename ... Rules >
struct seq_until_manual_stop< Rule, Rules... >
    : seq< Rule,
            if_then_else< manual_stop_checker,
            star< any >,
            seq_until_manual_stop< Rules... >>> {};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_CUSTOM_RULES_SEQUNTILMANUALSTOP_HPP
