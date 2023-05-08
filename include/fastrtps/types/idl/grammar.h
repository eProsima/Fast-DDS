// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_IDL_GRAMMAR_H_
#define TYPES_IDL_GRAMMAR_H_

#include "pegtl.hpp"

namespace eprosima {
namespace fastrtps {
namespace types {
namespace idl {

using namespace tao::TAO_PEGTL_NAMESPACE;

// *INDENT-OFF*  Allow curly braces on the same line to improve grammar readability

// COMMENTS
struct line_comment : seq<star<space>, TAO_PEGTL_KEYWORD("//"), until<eolf>> {};
struct block_comment : seq<star<space>, TAO_PEGTL_KEYWORD("/*"), until<TAO_PEGTL_KEYWORD("*/")>, star<space>> {};
struct comment : sor<line_comment, block_comment> {};
struct ws : sor<comment, plus<space>> {};

// DIGITS
struct octal_digit : range<'0', '7'> {};

// OPERATORS
struct equal_op : pad<one<'='>, ws> {};
struct or_op : pad<one<'|'>, ws> {};
struct xor_op : pad<one<'^'>, ws> {};
struct and_op : pad<one<'&'>, ws> {};
struct lshift_op : pad<TAO_PEGTL_KEYWORD("<<"), ws> {};
struct rshift_op : pad<TAO_PEGTL_KEYWORD(">>"), ws> {};
struct add_op : pad<one<'+'>, ws> {};
struct sub_op : pad<one<'-'>, ws> {};
struct mult_op : pad<one<'*'>, ws> {};
struct div_op : pad<one<'/'>, ws> {};
struct mod_op : pad<one<'%'>, ws> {};
struct neg_op : pad<one<'~'>, ws> {};
struct unary_op : sor<sub_op, add_op, neg_op> {};

// SYMBOLS
struct semicolon : pad<one<';'>, ws> {};
struct colon : pad<one<':'>, ws> {};
struct double_colon : pad<TAO_PEGTL_KEYWORD("::"), ws> {};
struct comma : pad<one<','>, ws> {};
struct open_parentheses : pad<one<'('>, ws> {};
struct close_parentheses : pad<one<')'>, ws> {};
struct open_bracket : pad<one<'['>, ws> {};
struct close_bracket : pad<one<']'>, ws> {};
struct open_ang_bracket : pad<one<'<'>, ws> {};
struct close_ang_bracket : pad<one<'>'>, ws> {};
struct open_brace : pad<one<'{'>, ws> {};
struct close_brace : pad<one<'}'>, ws> {};

// KEYWORDS
struct end_kw : not_at<identifier> {};
struct kw_const : seq<opt<ws>, TAO_PEGTL_KEYWORD("const"), end_kw, opt<ws>> {};
struct kw_module : seq<opt<ws>, TAO_PEGTL_KEYWORD("module"), end_kw, opt<ws>> {};
struct kw_sequence : seq<opt<ws>, TAO_PEGTL_KEYWORD("sequence"), end_kw, opt<ws>> {};
struct kw_string : seq<opt<ws>, TAO_PEGTL_KEYWORD("string"), end_kw, opt<ws>> {};
struct kw_wstring : seq<opt<ws>, TAO_PEGTL_KEYWORD("wstring"), end_kw, opt<ws>> {};
struct kw_fixed : seq<opt<ws>, TAO_PEGTL_KEYWORD("fixed"), end_kw, opt<ws>> {};
struct kw_union : seq<opt<ws>, TAO_PEGTL_KEYWORD("union"), end_kw, opt<ws>> {};
struct kw_struct : seq<opt<ws>, TAO_PEGTL_KEYWORD("struct"), end_kw, opt<ws>> {};
struct kw_enum : seq<opt<ws>, TAO_PEGTL_KEYWORD("enum"), end_kw, opt<ws>> {};
struct kw_switch : seq<opt<ws>, TAO_PEGTL_KEYWORD("switch"), end_kw, opt<ws>> {};
struct kw_case : seq<opt<ws>, TAO_PEGTL_KEYWORD("case"), end_kw, opt<ws>> {};
struct kw_default : seq<opt<ws>, TAO_PEGTL_KEYWORD("default"), end_kw, opt<ws>> {};
struct kw_native : seq<opt<ws>, TAO_PEGTL_KEYWORD("native"), end_kw, opt<ws>> {};
struct kw_typedef : seq<opt<ws>, TAO_PEGTL_KEYWORD("typedef"), end_kw, opt<ws>> {};
struct kw_any : seq<opt<ws>, TAO_PEGTL_KEYWORD("any"), end_kw, opt<ws>> {};
struct kw_map : seq<opt<ws>, TAO_PEGTL_KEYWORD("map"), end_kw, opt<ws>> {};
struct kw_bitset : seq<opt<ws>, TAO_PEGTL_KEYWORD("bitset"), end_kw, opt<ws>> {};
struct kw_bitfield : seq<opt<ws>, TAO_PEGTL_KEYWORD("bitfield"), end_kw, opt<ws>> {};
struct kw_bitmask : seq<opt<ws>, TAO_PEGTL_KEYWORD("bitmask"), end_kw, opt<ws>> {};
struct kw_annotation : seq<opt<ws>, TAO_PEGTL_KEYWORD("@annotation"), end_kw, opt<ws>> {};
struct kw_short : seq<opt<ws>, TAO_PEGTL_KEYWORD("short"), end_kw, opt<ws>> {};
struct kw_int16 : seq<opt<ws>, TAO_PEGTL_KEYWORD("int16"), end_kw, opt<ws>> {};
struct kw_long : seq<opt<ws>, TAO_PEGTL_KEYWORD("long"), end_kw, opt<ws>> {};
struct kw_long_long : seq<opt<ws>, TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("long"), end_kw, opt<ws>> {};
struct kw_int32 : seq<opt<ws>, TAO_PEGTL_KEYWORD("int32"), end_kw, opt<ws>> {};
struct kw_int64 : seq<opt<ws>, TAO_PEGTL_KEYWORD("int64"), end_kw, opt<ws>> {};
struct kw_unsigned_long : seq<opt<ws>, TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("long"), end_kw, opt<ws>> {};
struct kw_unsigned_long_long : seq<opt<ws>, TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("long"), end_kw, opt<ws>> {};
struct kw_unsigned_short : seq<opt<ws>, TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("short"), end_kw, opt<ws>> {};
struct kw_uint16 : seq<opt<ws>, TAO_PEGTL_KEYWORD("uint16"), end_kw, opt<ws>> {};
struct kw_uint32 : seq<opt<ws>, TAO_PEGTL_KEYWORD("uint32"), end_kw, opt<ws>> {};
struct kw_uint64 : seq<opt<ws>, TAO_PEGTL_KEYWORD("uint64"), end_kw, opt<ws>> {};
struct kw_int8 : seq<opt<ws>, TAO_PEGTL_KEYWORD("int8"), end_kw, opt<ws>> {};
struct kw_uint8 : seq<opt<ws>, TAO_PEGTL_KEYWORD("uint8"), end_kw, opt<ws>> {};
struct kw_char : seq<opt<ws>, TAO_PEGTL_KEYWORD("char"), end_kw, opt<ws>> {};
struct kw_wchar : seq<opt<ws>, TAO_PEGTL_KEYWORD("wchar"), end_kw, opt<ws>> {};
struct kw_boolean : seq<opt<ws>, TAO_PEGTL_KEYWORD("boolean"), end_kw, opt<ws>> {};
struct kw_octet : seq<opt<ws>, TAO_PEGTL_KEYWORD("octet"), end_kw, opt<ws>> {};
struct kw_float : seq<opt<ws>, TAO_PEGTL_KEYWORD("float"), end_kw, opt<ws>> {};
struct kw_double : seq<opt<ws>, TAO_PEGTL_KEYWORD("double"), end_kw, opt<ws>> {};
struct kw_long_double : seq<opt<ws>, TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("double"), end_kw, opt<ws>> {};

// ESCAPE SEQUENCES
struct es_unicode_char : seq<xdigit, opt<xdigit>, opt<xdigit>, opt<xdigit>> {};
struct es_hex_number : seq<xdigit, opt<xdigit>> {};
struct es_octal_number : seq<octal_digit, opt<octal_digit>, opt<octal_digit>> {};
struct es_unicode : seq<TAO_PEGTL_STRING("\\u"), es_unicode_char> {};
struct es_hex : seq<TAO_PEGTL_STRING("\\x"), es_hex_number> {};
struct es_octal : seq<TAO_PEGTL_STRING("\\"), es_octal_number> {};
struct es_double_quote : TAO_PEGTL_STRING("\\\"") {};
struct es_single_quote : TAO_PEGTL_STRING("\\'") {};
struct es_question_mark : TAO_PEGTL_STRING("\\?") {};
struct es_backslash : TAO_PEGTL_STRING("\\\\") {};
struct es_alert : TAO_PEGTL_STRING("\\a") {};
struct es_form_feed : TAO_PEGTL_STRING("\\f") {};
struct es_carriage_return : TAO_PEGTL_STRING("\\r") {};
struct es_backspace : TAO_PEGTL_STRING("\\b") {};
struct es_v_tab : TAO_PEGTL_STRING("\\v") {};
struct es_h_tab : TAO_PEGTL_STRING("\\t") {};
struct es_new_line : TAO_PEGTL_STRING("\\n") {};
struct escape_sequence : sor<
                              es_new_line, es_h_tab, es_v_tab, es_backspace, es_carriage_return, es_form_feed, es_alert,
                              es_backslash, es_question_mark, es_single_quote, es_double_quote, es_octal, es_hex, es_unicode
                            > {};

// LITERALS
struct boolean_literal : sor<TAO_PEGTL_KEYWORD("true"), TAO_PEGTL_KEYWORD("false")> {};
struct dec_literal : sor<seq<one<'-'>, plus<digit>>, plus<digit>> {};
struct oct_literal : seq<TAO_PEGTL_STRING("0"), plus<octal_digit>> {};
struct hex_literal : seq<sor<TAO_PEGTL_STRING("0x"), TAO_PEGTL_STRING("0X")>, plus<xdigit>> {};
struct integer_literal : sor<oct_literal, hex_literal, dec_literal> {};
struct char_literal : seq<one<'\''>, sor<escape_sequence, until<TAO_PEGTL_STRING("\'")>>, one<'\''>> {};
struct wide_char_literal : seq<one<'L'>, char_literal> {};
struct string_ws : sor<seq<one<'\"'>, opt<ws>, one<'\"'>>, opt<ws>> {};
// String literals must avoid '\0' inside them. Check after parsing!
struct substring_literal : if_must<TAO_PEGTL_STRING("\""), star<not_at<TAO_PEGTL_STRING("\"")>, any>, TAO_PEGTL_STRING("\"")> {};
struct string_literal : plus<substring_literal> {};
struct wide_substring_literal : substring_literal {};
struct wide_string_literal : seq<one<'L'>, plus<wide_substring_literal>> {};
struct exp_or_suffix : seq<one<'e', 'E'>, opt<one<'+', '-'>>, plus<digit>, opt<one<'d', 'D'>>> {};
struct float_literal : sor<
                            seq<plus<digit>, one<'.'>, star<digit>, opt<exp_or_suffix>>,
                            seq<one<'.'>, plus<digit>, opt<exp_or_suffix>>,
                            seq<plus<digit>, exp_or_suffix>
                          > {};
struct fixed_pt_literal : float_literal {};
struct literal : sor<
                      float_literal, fixed_pt_literal, integer_literal, boolean_literal,
                      char_literal, wide_char_literal, string_literal, wide_string_literal
                    > {};

struct scoped_name : seq<sor<seq<identifier, double_colon, scoped_name>, seq<double_colon, scoped_name>, identifier>> {};

// TYPES
struct float_type : kw_float {};
struct double_type : kw_double {};
struct long_double_type : kw_long_double {};
struct signed_tiny_int : kw_int8 {};
struct unsigned_tiny_int : kw_uint8 {};
struct signed_short_int : sor<kw_short, kw_int16> {};
struct unsigned_short_int : sor<kw_unsigned_short, kw_uint16> {};
struct signed_long_int : sor<kw_long, kw_int32> {};
struct unsigned_long_int : sor<kw_unsigned_long, kw_uint32> {};
struct signed_longlong_int : sor<kw_long_long, kw_int64> {};
struct unsigned_longlong_int : sor<kw_unsigned_long_long, kw_uint64> {};
struct signed_int : sor<signed_tiny_int, signed_short_int, signed_long_int, signed_longlong_int> {};
struct unsigned_int : sor<unsigned_tiny_int, unsigned_short_int, unsigned_long_int, unsigned_longlong_int> {};
struct integer_type : sor<signed_int, unsigned_int> {};
struct char_type : kw_char {};
struct wide_char_type : kw_wchar {};
struct boolean_type : kw_boolean {};
struct octet_type : kw_octet {};
struct any_type : kw_any {};
struct base_type_spec : sor<
                             float_type, double_type, long_double_type, integer_type,
                             char_type, wide_char_type, boolean_type, octet_type, any_type
                           > {};
struct fixed_pt_const_type : kw_fixed {};
struct positive_int_const : sor<integer_literal, scoped_name> {};
struct fixed_pt_type : seq<kw_fixed, open_ang_bracket, positive_int_const, comma, positive_int_const, close_ang_bracket> {};
struct string_size : seq<TAO_PEGTL_STRING("<"), positive_int_const, TAO_PEGTL_STRING(">")> {};
struct wstring_size : seq<TAO_PEGTL_STRING("<"), positive_int_const, TAO_PEGTL_STRING(">")> {};
struct collection_size : seq<comma, positive_int_const> {};
struct string_type : seq<kw_string, opt<string_size>> {};
struct wide_string_type : seq<kw_wstring, opt<wstring_size>> {};
struct map_type; // forward declaration
struct type_spec; // forward declaration
struct sequence_size : opt<collection_size> {};
struct sequence_type : seq<kw_sequence, open_ang_bracket, type_spec, sequence_size, close_ang_bracket> {};
struct template_type_spec : sor<map_type, sequence_type, string_type, wide_string_type, fixed_pt_type> {};
struct simple_type_spec : sor<base_type_spec, scoped_name> {};
struct type_spec : seq<opt<ws>, sor<template_type_spec, simple_type_spec>, opt<ws>> {};

struct any_shift_op : sor<lshift_op, rshift_op> {};
struct any_add_op : sor<add_op, sub_op> {};
struct any_mult_op : sor<mult_op, div_op, mod_op> {};

struct scoped_or_literal : sor<literal, scoped_name> {};
struct const_expr; // forward declaration
struct primary_expr : sor<seq<open_parentheses, const_expr, close_parentheses>, scoped_or_literal> {};
struct unary_expr : sor<seq<unary_op, primary_expr>, primary_expr> {};
struct mult_expr : sor<seq<unary_expr, any_mult_op, mult_expr>, unary_expr> {};
struct add_expr : sor<seq<mult_expr, any_add_op, add_expr>, mult_expr> {};
struct shift_expr : sor<seq<add_expr, any_shift_op, shift_expr>, add_expr> {};
struct and_expr : sor<seq<shift_expr, and_op, and_expr>, shift_expr> {};
struct xor_expr : sor<seq<and_expr, xor_op, xor_expr>, and_expr> {};
struct const_expr : sor<seq<xor_expr, or_op, const_expr>, xor_expr> {};

struct const_type : sor<
                         float_type, double_type, long_double_type, fixed_pt_const_type, integer_type,
                         char_type, wide_char_type, boolean_type, string_type, wide_string_type, scoped_name
                       > {};
struct simple_declarator : identifier {};
struct fixed_array_size : seq<open_bracket, positive_int_const, close_bracket> {};
struct array_declarator : seq<identifier, plus<fixed_array_size>> {};
struct declarator : sor<array_declarator, simple_declarator> {}; // same as any_declarator
struct declarators : seq<declarator, star<seq<comma, declarator>>> {};
struct any_declarator : sor<array_declarator, simple_declarator> {};
struct any_declarators : seq<any_declarator, star<seq<comma, any_declarator>>> {};
struct type_declarator; // forward declaration
struct typedef_dlc : seq<kw_typedef, type_declarator> {};
struct native_dcl : seq<kw_native, simple_declarator> {};
struct annotation_appl; // forward declaration
struct enumerator : seq<star<annotation_appl>, identifier> {};
struct enum_dcl : seq<kw_enum, identifier, open_brace, enumerator, star<comma, enumerator>, close_brace> {};
struct union_forward_dcl : seq<kw_union, identifier> {};
struct element_spec : seq<star<annotation_appl>, type_spec, declarator> {};
struct case_label : sor<seq<kw_case, const_expr, colon>, seq<kw_default, colon>> {};
struct case_branch : seq<plus<case_label>, element_spec, semicolon> {};
struct switch_body : plus<case_branch> {};
struct switch_type_spec : sor<integer_type, char_type, boolean_type, wide_char_type, octet_type, scoped_name> {};
struct union_def : seq<kw_union, identifier, kw_switch, open_parentheses, switch_type_spec, close_parentheses, open_brace, switch_body, close_brace> {};
struct union_dcl : sor<union_def, union_forward_dcl> {};
struct struct_forward_dcl : seq<kw_struct, identifier> {};
struct member : seq<star<annotation_appl>, type_spec, declarators, semicolon> {};
struct inhertance : seq<colon, scoped_name> {};
struct struct_def : seq<kw_struct, identifier, opt<inhertance>, open_brace, star<member>, close_brace> {};
struct struct_dcl : sor<struct_def, struct_forward_dcl> {};
struct bitset_dcl; // forward declaration
struct bitmask_dcl; // forward declaration
struct constr_type_dlc : seq<star<annotation_appl>, sor<struct_dcl, union_dcl, enum_dcl, bitset_dcl, bitmask_dcl>> {};
struct type_declarator : seq<sor<constr_type_dlc, template_type_spec, simple_type_spec>, opt<ws>, any_declarators> {};
struct type_dcl : sor<constr_type_dlc, native_dcl, typedef_dlc> {};
struct const_dcl : seq<kw_const, const_type, opt<ws>, identifier, equal_op, const_expr> {};

// ANNOTATIONS
struct annotation_appl_param : sor<seq<identifier, equal_op, const_expr>, const_expr> {};
struct annotation_appl_params : sor<seq<annotation_appl_param, star<seq<comma, annotation_appl_param>>>, annotation_appl_param> {};
struct annotation_appl : seq<TAO_PEGTL_KEYWORD("@"), scoped_name, opt<open_parentheses, annotation_appl_params, close_parentheses>> {};
struct any_const_type : kw_any {};
struct annotation_member_type : sor<const_type, any_const_type, scoped_name> {};
struct annotation_member : seq<opt<ws>, annotation_member_type, opt<ws>, simple_declarator, opt<seq<kw_default, const_expr>>, semicolon> {};
struct annotation_body : star<sor<annotation_member, seq<enum_dcl, semicolon>, seq<const_dcl, semicolon>, seq<typedef_dlc, semicolon>>> {};
struct annotation_header : seq<kw_annotation, identifier> {};
struct annotation_dcl : seq<annotation_header, open_brace, annotation_body, close_brace> {};

// XTYPES
struct map_key_type : type_spec {};
struct map_inner_type : type_spec {};
struct map_size : opt<collection_size> {};
struct map_type : seq<kw_map, open_ang_bracket, map_key_type, comma, map_inner_type, map_size, close_ang_bracket> {};
struct bit_value : seq<star<seq<annotation_appl, opt<ws>>>, identifier> {};
struct bitmask_dcl : seq<kw_bitmask, identifier, open_brace, bit_value, star<seq<comma, bit_value>>, close_brace> {};
struct destination_type : sor<boolean_type, octet_type, integer_type> {};
struct bitfield_spec : seq<kw_bitfield, open_ang_bracket, positive_int_const, opt<seq<comma, destination_type>>, close_ang_bracket> {};
struct bitfield : seq<star<annotation_appl>, bitfield_spec, star<identifier>, semicolon> {};
struct bitset_dcl : seq<kw_bitset, identifier, opt<inhertance>, open_brace, star<bitfield>, close_brace> {};

struct preprocessor_directive : seq<opt<ws>, one<'#'>, until<eolf>> {};
struct module_dcl; // forward declaration
struct definition : sor<seq<sor<module_dcl, const_dcl, type_dcl, annotation_dcl>, semicolon>, preprocessor_directive> {};
struct module_dcl : seq<star<annotation_appl>, kw_module, identifier, open_brace, plus<definition>, close_brace> {};
struct specification : plus<definition> {};
struct document : seq<opt<ws>, specification, opt<ws>> {};

} //namespace idl
} //namespace types
} //namespace fastrtps
} //namespace eprosima


#endif // TYPES_IDL_GRAMMAR_H_
