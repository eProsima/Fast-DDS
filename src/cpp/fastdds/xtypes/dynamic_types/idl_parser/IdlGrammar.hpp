// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLGRAMMAR_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLGRAMMAR_HPP

#include "pegtl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

using namespace tao::TAO_PEGTL_NAMESPACE;

// *INDENT-OFF*  Allow curly braces on the same line to improve grammar readability

// comments
struct line_comment : seq<star<space>, seq<one<'/'>, one<'/'>>, until<eolf>> {};
struct block_comment : seq<star<space>, seq<one<'/'>, one<'*'>>, until<seq<one<'*'>, one<'/'>>>, star<space>> {};
struct comment : sor<line_comment, block_comment> {};
struct ws : sor<comment, plus<space>> {};

// octal digits
struct odigit : range<'0', '7'> {};

// symbols
struct semicolon : pad<one<';'>, ws> {};
struct colon : pad<one<':'>, ws> {};
struct double_colon : pad<seq<one<':'>, one<':'>>, ws> {};
struct comma : pad<one<','>, ws> {};
struct open_parentheses : pad<one<'('>, ws> {};
struct close_parentheses : pad<one<')'>, ws> {};
struct open_bracket : pad<one<'['>, ws> {};
struct close_bracket : pad<one<']'>, ws> {};
struct open_ang_bracket : pad<one<'<'>, ws> {};
struct close_ang_bracket : pad<one<'>'>, ws> {};
struct open_brace : pad<one<'{'>, ws> {};
struct close_brace : pad<one<'}'>, ws> {};

// keywords
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
struct kw_unsigned_short : seq<TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("short"), end_kw> {};
struct kw_short : seq<TAO_PEGTL_KEYWORD("short"), end_kw> {};
struct kw_int16 : seq<TAO_PEGTL_KEYWORD("int16"), end_kw> {};
struct kw_unsigned_long_long : seq<TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("long"), end_kw> {};
struct kw_unsigned_long : seq<TAO_PEGTL_KEYWORD("unsigned"), ws, TAO_PEGTL_KEYWORD("long"), end_kw> {};
struct kw_long_long : seq<TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("long"), end_kw> {};
struct kw_long_double : seq<TAO_PEGTL_KEYWORD("long"), ws, TAO_PEGTL_KEYWORD("double"), end_kw> {};
struct kw_long : seq<TAO_PEGTL_KEYWORD("long"), end_kw> {};
struct kw_int32 : seq<TAO_PEGTL_KEYWORD("int32"), end_kw> {};
struct kw_int64 : seq<TAO_PEGTL_KEYWORD("int64"), end_kw> {};
struct kw_uint16 : seq<TAO_PEGTL_KEYWORD("uint16"), end_kw> {};
struct kw_uint32 : seq<TAO_PEGTL_KEYWORD("uint32"), end_kw> {};
struct kw_uint64 : seq<TAO_PEGTL_KEYWORD("uint64"), end_kw> {};
struct kw_int8 : seq<TAO_PEGTL_KEYWORD("int8"), end_kw> {};
struct kw_uint8 : seq<TAO_PEGTL_KEYWORD("uint8"), end_kw> {};
struct kw_char : seq<TAO_PEGTL_KEYWORD("char"), end_kw> {};
struct kw_wchar : seq<TAO_PEGTL_KEYWORD("wchar"), end_kw> {};
struct kw_boolean : seq<TAO_PEGTL_KEYWORD("boolean"), end_kw> {};
struct kw_octet : seq<TAO_PEGTL_KEYWORD("octet"), end_kw> {};
struct kw_float : seq<TAO_PEGTL_KEYWORD("float"), end_kw> {};
struct kw_double : seq<TAO_PEGTL_KEYWORD("double"), end_kw> {};

/* literal grammar */

// boolean literal
struct boolean_literal : sor<TAO_PEGTL_KEYWORD("TRUE"), TAO_PEGTL_KEYWORD("FALSE")> {};

// integer literals
using zero = one<'0'>;
struct dec_literal : seq<opt<one<'-'>>, plus<digit>> {};
struct oct_literal : seq<zero, plus<odigit>> {};
struct hex_literal : seq<zero, one<'x', 'X'>, plus<xdigit>> {};
struct float_literal;
struct fixed_pt_literal;
// prevent premature matching of integer literals
struct integer_literal : seq<
                                not_at<float_literal>,
                                not_at<fixed_pt_literal>,
                                sor<hex_literal, oct_literal, dec_literal>
                            > {};

// float literals
using dot = one<'.'>;
using kw_exp = one<'e', 'E'>;
struct decimal_exponent : seq<kw_exp, opt<one<'-'>>, plus<digit>> {};
struct float_literal : seq<
                            not_at<fixed_pt_literal>,
                            opt<one<'-'>>,
                            not_at<seq<dot, kw_exp>>,
                            star<digit>,
                            sor<
                                seq<opt<seq<dot, star<digit>>>, decimal_exponent>,
                                seq<dot, star<digit>>
                               >
                          > {};

// fixed-point literals
using fixed_suffix = one<'d', 'D'>;
struct fixed_pt_literal : seq<
                                opt<one<'-'>>,
                                not_at<seq<dot, fixed_suffix>>,
                                star<digit>,
                                opt<seq<dot, star<digit>>>,
                                fixed_suffix
                             > {};

// char literals
using singlequote = one<'\''>;
using doublequote = one<'"'>;
using backslash = one<'\\'>;
using escapable_char = sor<
                            singlequote,
                            doublequote,
                            one<'?'>,
                            backslash,
                            one<'a'>,
                            one<'b'>,
                            one<'f'>,
                            one<'n'>,
                            one<'r'>,
                            one<'t'>,
                            one<'v'>
                          >;
struct escaped_octal : seq<backslash, rep_max<3, odigit>> {};
struct escaped_hexa : seq<backslash, one<'x'>, rep_max<2, xdigit>> {};
struct escaped_unicode : seq<backslash, one<'u'>, rep_max<4, xdigit>> {};
struct escape_sequence : sor<
                                seq<backslash, escapable_char>,
                                escaped_unicode,
                                escaped_hexa,
                                escaped_octal
                            > {};
struct character : sor<escape_sequence, seq<not_at<singlequote>, any>> {};
struct character_literal : seq<singlequote, character, singlequote> {};

// wide-char literals
struct wide_character : sor<escape_sequence, seq<not_at<singlequote>, utf8::any>> {};
struct wide_character_literal : seq<one<'L'>, singlequote, wide_character, singlequote> {};

// string literals
struct string_character : sor<escape_sequence, seq<not_at<doublequote>, any>> {};
struct substring_literal : seq<doublequote, star<string_character>, doublequote> {};
struct string_literal : seq<substring_literal, star<seq<space, substring_literal>>> {};

// wstring literals
struct wstring_character : sor<escape_sequence, seq<not_at<doublequote>, utf8::any>> {};
struct wide_substring_literal : seq<one<'L'>, doublequote, star<wstring_character>, doublequote> {};
struct wide_string_literal : seq<wide_substring_literal, star<seq<space, wide_substring_literal>>> {};

// literals
struct literal : sor<
                        pad<boolean_literal, ws>,
                        pad<integer_literal, ws>,
                        pad<float_literal, ws>,
                        pad<fixed_pt_literal, ws>,
                        pad<character_literal, ws>,
                        pad<wide_character_literal, ws>,
                        pad<string_literal, ws>,
                        pad<wide_string_literal, ws>
                    > {};

// operators
struct equal_op : pad<one<'='>, ws> {};
struct or_op : pad<one<'|'>, ws> {};
struct xor_op : pad<one<'^'>, ws> {};
struct and_op : pad<one<'&'>, ws> {};
struct lshift_op : pad<seq<one<'<'>, one<'<'>>, ws> {};
struct rshift_op : pad<seq<one<'>'>, one<'>'>>, ws> {};
struct add_op : pad<one<'+'>, ws> {};
struct sub_op : pad<one<'-'>, ws> {};
struct mult_op : pad<one<'*'>, ws> {};
struct div_op : pad<one<'/'>, ws> {};
struct mod_op : pad<one<'%'>, ws> {};
struct neg_op : pad<one<'~'>, ws> {};

// const expressions
struct scoped_name;
struct scoped_or_literal : sor<literal, scoped_name> {};
struct const_expr;
struct primary_expr : sor<seq<open_parentheses, const_expr, close_parentheses>, scoped_or_literal> {};

struct inv_exec : seq<neg_op, primary_expr> {};
struct plus_exec : seq<add_op, primary_expr> {};
struct minus_exec : seq<sub_op, primary_expr> {};
struct unary_expr : sor<inv_exec, plus_exec, minus_exec, primary_expr> {};

struct mult_expr;
struct mod_exec : seq<mod_op, mult_expr> {};
struct div_exec : seq<div_op, mult_expr> {};
struct mult_exec : seq<mult_op, mult_expr> {};
struct mult_expr : seq<unary_expr, opt<sor<mod_exec, div_exec, mult_exec>>> {};

struct add_expr;
struct sub_exec : seq<sub_op, add_expr> {};
struct add_exec : seq<add_op, add_expr> {};
struct add_expr : seq<mult_expr, opt<sor<sub_exec, add_exec>>> {};

struct shift_expr;
struct lshift_exec : seq<lshift_op, shift_expr> {};
struct rshift_exec : seq<rshift_op, shift_expr> {};
struct shift_expr : seq<add_expr, opt<sor<lshift_exec, rshift_exec>>> {};

struct and_expr;
struct and_exec : seq<and_op, and_expr> {};
struct and_expr : seq<shift_expr, opt<and_exec>> {};

struct xor_expr;
struct xor_exec : seq<xor_op, xor_expr> {};
struct xor_expr : seq<and_expr, opt<xor_exec>> {};

struct or_exec : seq<or_op, const_expr> {};
struct const_expr : seq<xor_expr, opt<or_exec>> {};

// types
struct float_type : seq<opt<ws>, kw_float, opt<ws>> {};
struct long_double_type : seq<opt<ws>, kw_long_double, opt<ws>> {};
struct double_type : seq<opt<ws>, kw_double, opt<ws>> {};
struct signed_tiny_int : seq<opt<ws>, kw_int8, opt<ws>> {};
struct unsigned_tiny_int : seq<opt<ws>, kw_uint8, opt<ws>> {};
struct signed_short_int : seq<opt<ws>, sor<kw_short, kw_int16>, opt<ws>> {};
struct unsigned_short_int : seq<opt<ws>, sor<kw_unsigned_short, kw_uint16>, opt<ws>> {};
struct unsigned_longlong_int : seq<opt<ws>, sor<kw_unsigned_long_long, kw_uint64>, opt<ws>> {};
struct unsigned_long_int : seq<opt<ws>, sor<kw_unsigned_long, kw_uint32>, opt<ws>> {};
struct signed_longlong_int : seq<opt<ws>, sor<kw_long_long, kw_int64>, opt<ws>> {};
struct signed_long_int : seq<opt<ws>, sor<kw_long, kw_int32>, opt<ws>> {};
struct unsigned_int : sor<unsigned_tiny_int, unsigned_short_int, unsigned_longlong_int, unsigned_long_int> {};
struct signed_int : sor<signed_tiny_int, signed_short_int, signed_longlong_int, signed_long_int> {};
struct integer_type : sor<unsigned_int, signed_int> {};
struct char_type : seq<opt<ws>, kw_char, opt<ws>> {};
struct wide_char_type : seq<opt<ws>, kw_wchar, opt<ws>> {};
struct boolean_type : seq<opt<ws>, kw_boolean, opt<ws>> {};
struct octet_type : seq<opt<ws>, kw_octet, opt<ws>> {};
struct any_type : kw_any {};
struct base_type_spec : sor<
                            float_type, long_double_type, double_type, integer_type,
                            char_type, wide_char_type, boolean_type, octet_type, any_type
                           > {};
struct fixed_pt_const_type : kw_fixed {};

struct scoped_name_tail : seq<double_colon, identifier> {};
struct scoped_name : seq<opt<double_colon>, identifier, star<scoped_name_tail>> {};
struct positive_int_const : const_expr {};
struct fixed_pt_type : seq<kw_fixed, open_ang_bracket, positive_int_const, comma, positive_int_const, close_ang_bracket> {};
struct string_size : seq<open_ang_bracket, positive_int_const, close_ang_bracket> {};
struct wstring_size : seq<open_ang_bracket, positive_int_const, close_ang_bracket> {};
struct collection_size : seq<comma, positive_int_const> {};
struct string_type : seq<kw_string, opt<string_size>> {};
struct wide_string_type : seq<kw_wstring, opt<wstring_size>> {};
struct map_type;
struct type_spec;
struct sequence_size : collection_size {};
struct sequence_type : seq<kw_sequence, open_ang_bracket, type_spec, opt<sequence_size>, close_ang_bracket> {};
struct template_type_spec : sor<map_type, sequence_type, string_type, wide_string_type, fixed_pt_type> {};
struct simple_type_spec : sor<base_type_spec, scoped_name> {};
struct type_spec : seq<opt<ws>, sor<template_type_spec, simple_type_spec>, opt<ws>> {};

struct const_type : sor<
                        float_type, long_double_type, double_type, fixed_pt_const_type, integer_type,
                        char_type, wide_char_type, boolean_type, string_type, wide_string_type, scoped_name
                       > {};
struct simple_declarator : identifier {};
struct fixed_array_size : seq<open_bracket, positive_int_const, close_bracket> {};
struct array_declarator : seq<identifier, plus<fixed_array_size>> {};
struct declarator : sor<array_declarator, simple_declarator> {}; // same as any_declarator
struct declarators : seq<declarator, star<seq<comma, declarator>>> {};
struct any_declarator : sor<array_declarator, simple_declarator> {};
struct any_declarators : seq<any_declarator, star<seq<comma, any_declarator>>> {};
struct type_declarator;
struct typedef_dcl : seq<kw_typedef, type_declarator> {};
struct native_dcl : seq<kw_native, simple_declarator> {};
struct annotation_appl;
struct enumerator : seq<star<annotation_appl>, identifier> {};
struct enum_dcl : seq<kw_enum, identifier, open_brace, enumerator, star<comma, enumerator>, close_brace> {};
struct union_forward_dcl : seq<kw_union, identifier> {};
struct element_spec : seq<star<annotation_appl>, type_spec, declarator> {};
struct case_label : sor<seq<kw_case, const_expr, colon>, seq<kw_default, colon>> {};
struct switch_case : seq<plus<case_label>, element_spec, semicolon> {};
struct switch_body : plus<switch_case> {};
struct switch_type_spec : sor<integer_type, char_type, boolean_type, wide_char_type, octet_type, scoped_name> {};
struct union_def : seq<kw_union, identifier, kw_switch, open_parentheses, switch_type_spec, close_parentheses, open_brace, switch_body, close_brace> {};
struct union_dcl : sor<union_def, union_forward_dcl> {};
struct struct_forward_dcl : seq<kw_struct, identifier, not_at<open_brace>> {};
struct member : seq<star<annotation_appl>, type_spec, declarators, semicolon> {};
struct inhertance : seq<colon, scoped_name> {};
struct struct_def : seq<kw_struct, identifier, opt<inhertance>, open_brace, star<member>, close_brace> {};
struct struct_dcl : sor<struct_def, struct_forward_dcl> {};
struct bitset_dcl;
struct bitmask_dcl;
struct constr_type_dcl : seq<star<annotation_appl>, sor<struct_dcl, union_dcl, enum_dcl, bitset_dcl, bitmask_dcl>> {};
struct type_declarator : seq<sor<constr_type_dcl, template_type_spec, simple_type_spec>, opt<ws>, any_declarators> {};
struct type_dcl : sor<constr_type_dcl, native_dcl, typedef_dcl> {};
struct const_dcl : seq<kw_const, const_type, opt<ws>, identifier, equal_op, const_expr> {};

// ANNOTATIONS
struct annotation_appl_param : sor<seq<identifier, equal_op, const_expr>, const_expr> {};
struct annotation_appl_params : sor<seq<annotation_appl_param, star<seq<comma, annotation_appl_param>>>, annotation_appl_param> {};
struct annotation_appl : seq<TAO_PEGTL_STRING("@"), scoped_name, opt<open_parentheses, annotation_appl_params, close_parentheses>> {};
struct any_const_type : kw_any {};
struct annotation_member_type : sor<const_type, any_const_type, scoped_name> {};
struct annotation_member : seq<opt<ws>, annotation_member_type, opt<ws>, simple_declarator, opt<seq<kw_default, const_expr>>, semicolon> {};
struct annotation_body : star<sor<annotation_member, seq<enum_dcl, semicolon>, seq<const_dcl, semicolon>, seq<typedef_dcl, semicolon>>> {};
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

} //namespace idlparser
} //namespace dds
} //namespace fastdds
} //namespace eprosima


#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLGRAMMAR_HPP
