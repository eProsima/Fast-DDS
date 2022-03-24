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
 * @file DDSFilterGrammar.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERGRAMMAR_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERGRAMMAR_HPP_

#include "pegtl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

using namespace tao::TAO_PEGTL_NAMESPACE;

// *INDENT-OFF*  Allow curly braces on the same line to improve grammar readability

// Some basic constructs
struct sign : one< '+', '-'> {};
struct integer : plus< digit > {};

// FIELDNAME
struct dot_op : one< '.' > {};
struct index_part : seq< one<'['>, integer, one<']'> > {};
struct fieldname_part : seq< identifier, opt< index_part > > {};
struct fieldname : list<fieldname_part, dot_op> {};

// CHARVALUE, STRING, ENUMERATEDVALUE
struct open_quote : one< '`', '\'' > {};
struct close_quote : one< '\'' > {};
struct char_value : seq< open_quote, any, close_quote > {};
struct string_content : star< not_one< '\'', '\r', '\n'> > {};
struct string_value : seq< open_quote, string_content, close_quote > {};

// BOOLEANVALUE
struct false_value : pad< sor< TAO_PEGTL_KEYWORD("FALSE"), TAO_PEGTL_KEYWORD("false") >, space > {};
struct true_value : pad< sor< TAO_PEGTL_KEYWORD("TRUE"), TAO_PEGTL_KEYWORD("true") >, space > {};
struct boolean_value : sor<false_value, true_value> {};

// INTEGERVALUE, FLOATVALUE
struct integer_value : seq< opt< sign >, integer > {};
struct fractional : seq< dot_op, integer > {};
struct exponent : seq< one< 'e', 'E' >, integer_value > {};
struct float_value : seq < opt< sign >, integer, sor < exponent, seq< fractional, opt< exponent > > > > {};
struct hex_value : seq< opt< sign >, one< '0' >, one< 'x', 'X' >, plus< xdigit > > {};

// PARAMETER
struct parameter_value : seq< one< '%' >, digit, opt< digit > > {};

// Keyword based operators
struct and_op : pad< sor< TAO_PEGTL_KEYWORD("AND"), TAO_PEGTL_KEYWORD("and") >, space > {};
struct or_op : pad< sor< TAO_PEGTL_KEYWORD("OR"), TAO_PEGTL_KEYWORD("or") >, space> {};
struct not_op : pad< sor< TAO_PEGTL_KEYWORD("NOT"), TAO_PEGTL_KEYWORD("not") >, space> {};
struct between_op : pad< sor< TAO_PEGTL_KEYWORD("BETWEEN"), TAO_PEGTL_KEYWORD("between") >, space> {};
struct not_between_op : pad< sor< TAO_PEGTL_KEYWORD("NOT BETWEEN"), TAO_PEGTL_KEYWORD("not between") >, space> {};

// RelOp
struct eq_op : pad< one<'='>, space> {};
struct gt_op : pad< one<'>'>, space> {};
struct ge_op : pad< TAO_PEGTL_KEYWORD(">="), space> {};
struct lt_op : pad< one<'<'>, space> {};
struct le_op : pad< TAO_PEGTL_KEYWORD("<="), space> {};
struct ne_op : pad< sor< TAO_PEGTL_KEYWORD("<>"), TAO_PEGTL_KEYWORD("!=") >, space> {};
struct like_op : pad< sor< TAO_PEGTL_KEYWORD("LIKE"), TAO_PEGTL_KEYWORD("like") >, space> {};
struct match_op : pad< sor< TAO_PEGTL_KEYWORD("MATCH"), TAO_PEGTL_KEYWORD("match") >, space> {};
struct rel_op : sor< match_op, like_op, ne_op, le_op, ge_op, lt_op, gt_op, eq_op > {};

// Parameter, Range
struct Literal : sor< boolean_value, float_value, hex_value, integer_value, char_value, string_value > {};
struct Parameter : sor< Literal, parameter_value > {};
struct Range : seq< Parameter, and_op, Parameter > {};

// Predicates
struct BetweenPredicate : seq< fieldname, sor< not_between_op, between_op >, Range > {};
struct ComparisonPredicate : sor<
                                  seq< Parameter, rel_op, fieldname >,
                                  seq< fieldname, rel_op, Parameter >,
                                  seq< fieldname, rel_op, fieldname >
                                > {};
struct Predicate : sor< ComparisonPredicate, BetweenPredicate > {};

// Brackets
struct open_bracket : seq< one< '(' >, star< space > > {};
struct close_bracket : seq< star< space >, one< ')' > > {};

// Condition, FilterExpression
struct Condition;
struct ConditionList : list_must< Condition, sor< and_op, or_op > > {};
struct Condition : sor<
                        seq< open_bracket, ConditionList, close_bracket >,
                        seq< not_op, ConditionList >,
                        Predicate
                      > {};
struct FilterExpression : ConditionList {};

// Main grammar
struct FilterExpressionGrammar : must< FilterExpression, tao::TAO_PEGTL_NAMESPACE::eof > {};
struct LiteralGrammar : must< Literal, tao::TAO_PEGTL_NAMESPACE::eof > {};

// *INDENT-ON*

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERGRAMMAR_HPP_
