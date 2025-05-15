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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSER_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSER_HPP

#include <array>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#   include <cstdio>
#else
#   include <stdlib.h>
#   include <unistd.h>
#endif //_MSC_VER

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "pegtl.hpp"
#include "pegtl/analyze.hpp"

#include "IdlGrammar.hpp"
#include "IdlModule.hpp"
#include "IdlPreprocessor.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

using namespace tao::TAO_PEGTL_NAMESPACE;

class Parser;

class Context
    : public PreprocessorContext
{
public:

    enum CharType
    {
        CHAR,
        UINT8,
        INT8
    };

    enum WideCharType
    {
        WCHAR_T,
        CHAR16_T
    };

    // Config
    bool ignore_case = false;
    bool clear = true;
    bool allow_keyword_identifiers = false;
    bool ignore_redefinition = false;
    CharType char_translation = CHAR;
    WideCharType wchar_type = WCHAR_T;

    // Results
    bool success = false;
    std::string target_type_name;

    traits<DynamicType>::ref_type get_type(
            std::map<std::string, std::string>& state,
            const std::string& type);

    std::vector<std::string> split_string(
            const std::string& str,
            char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream ss(str);
        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    DynamicTypeBuilder::_ref_type builder;

    Module& module()
    {
        if (!module_)
        {
            module_ = std::make_shared<Module>();
        }
        return *module_;
    }

    void clear_context()
    {
        if (clear)
        {
            parser_.reset();
            module_.reset();
        }
    }

    ~Context()
    {
        clear_context();
    }

private:

    friend class Parser;
    std::shared_ptr<Parser> parser_;
    std::shared_ptr<Module> module_;

}; // class Context


// Actions
template<typename Rule>
struct action
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& /*state*/,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // std::cout << "Rule: " << typeid(Rule).name() << " " << in.string() << std::endl;
    }

};

template<>
struct action<identifier>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        std::string identifier_name = in.string();

        if (state.count("enum_name"))
        {
            if (state["enum_name"].empty())
            {
                state["enum_name"] = identifier_name;
            }
            else
            {
                state["enum_member_names"] += identifier_name + ";";
            }
        }
        else if (state.count("struct_name"))
        {
            if (state["struct_name"].empty())
            {
                state["struct_name"] = identifier_name;
            }
            else
            {
                if (!state["type"].empty())
                {
                    // In case of the struct member is a sequence, two possible cases:
                    if (state["type"] == "sequence")
                    {
                        // 1. Matched identifier is the member name. In this case, update state with the member name.
                        if ((!state["element_type"].empty()) && state["current_struct_member_name"].empty())
                        {
                            // The identifier is the member name
                            state["current_struct_member_name"] = identifier_name;
                        }
                        // 2. Matched identifier is the element type of the sequence.
                        //    Element type should be previously declared, so updating state
                        //    should be managed in action<scoped_name> and not here.
                    }

                    // In case a struct member is an array and the array size is an identifier,
                    // use below check to avoid overwriting the member name with the size identifier.
                    else if (state["current_struct_member_name"].empty())
                    {
                        // The identifier is a member name
                        state["current_struct_member_name"] = identifier_name;
                    }
                }
            }
        }
        else if (state.count("union_name"))
        {
            if (state["union_name"].empty())
            {
                state["union_name"] = identifier_name;
            }
            else
            {
                if (state["union_discriminant"].empty())
                {
                    state["union_discriminant"] = identifier_name;
                }
                else if (state.count("arithmetic_expr"))
                {
                    // Identifier is part of case label. Handle it in scoped_name action.
                    return;
                }
                else if (!state["type"].empty())
                {
                    // In case of the union member is a sequence, two possible cases:
                    if (state["type"] == "sequence")
                    {
                        // 1. Matched identifier is the member name. In this case, update state with the member name.
                        if ((!state["element_type"].empty()) && state["current_union_member_name"].empty())
                        {
                            // The identifier is the member name
                            state["current_union_member_name"] = identifier_name;
                        }
                        // 2. Matched identifier is the element type of the sequence. In this case, element type
                        //    should be previously declared,
                        //    so updating state should be managed in action<scoped_name> ad not here.
                    }

                    // In case a union member is an array and the array size is an identifier,
                    // use below check to avoid overwriting the member name with the size identifier.
                    else if (state["current_union_member_name"].empty())
                    {
                        // The identifier is a member name
                        state["current_union_member_name"] = identifier_name;
                    }
                }
            }
        }
        else if (state.count("alias"))
        {
            // Store the new alias name into state["alias"]. There are different cases:

            // 1. The aliased type is a custom and previously declared type (parsed using action<scoped_name>),
            //    and identifier_name refers to the name of the alias.
            if (!state["alias"].empty())
            {
                // Append the identifier name to state["alias"].
                state["alias"] += "," + identifier_name;
            }

            // 2. The aliased type is not a previously declared type
            //    (i.e: typedef <type> <alias>, when <type> is a Primitive type, string type, sequence type or map type
            //    not previously declared).
            //    In this case, aliased type info is stored in state["type"] and other additional state keys,
            //    so state["alias"] is empty and state["type"] is non-empty.
            else if (!state["type"].empty())
            {

                // 2.1. Declared type contains internal types (e.g: element types in a sequence),
                //      and these types are previosly declared by the user,
                //      i.e: <type>:=external_type<internal_type_1,...>, being <internal_type_i> a previously declared type.
                //      In this case, identifier_name should be stored in specific states keys and must be handled depending on state["type"].
                if ((state["type"] == "sequence") && state.count("element_type") && state["element_type"].empty())
                {
                    // State updates should be handled in action<scoped_name>.
                    return;
                }

                // 2.2: Declared type does not contain internal types, or its internal types are not previously declared by the user.
                //      In this case, the identifier_name matched correspond to the alias name, so it should be stored in state["alias"].
                state["alias"] = identifier_name;
            }

            // 3. The aliased type is a custom and previously declared type,
            //    and the identifier_name refers to the name of the aliased type or its scope.
            //    In this case, state["alias"] and state["type"] should be both empty.
            else
            {
                // State updates should be handled in action<scoped_name>.
                return;
            }
        }
        else
        {
            // Keep the identifier for super-expression use
            state["identifier"] = identifier_name;
        }
    }

};

template<>
struct action<scoped_name>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        Module& module = ctx->module();
        std::string identifier_name = in.string();

        if (state.count("arithmetic_expr"))
        {
            if (module.has_constant(identifier_name))
            {
                DynamicData::_ref_type xdata = module.constant(identifier_name);
                operands.push_back(xdata);
            }
            else
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Unknown constant or identifier: " << identifier_name);
                throw std::runtime_error("Unknown constant or identifier: " + identifier_name);
            }
        }
        else if (state["type"] == "sequence" && state["element_type"].empty())
        {
            // <scoped_name> is the element type of a sequence (previously declared)
            state["element_type"] = identifier_name;

            // Ready to parse the sequence size (if provided)
            state["arithmetic_expr"] = "";
        }
        else if (state.count("alias"))
        {
            // <scoped_name> makes reference to the aliased type name.
            state["alias"] = identifier_name;
        }
        else
        {
            state["type"] = identifier_name;
        }
    }

};

template<>
struct action<semicolon>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        if (!state["type"].empty())
        {
            if (state.count("current_struct_member_name") && !state["current_struct_member_name"].empty())
            {
                // Add the type and name to the member lists
                // NOTE: For sequence types, the type stored in "struct_member_types"
                // is the type of each element in the sequence, not "sequence".
                // Sequence type can be inferred by checking if "sequence_sizes" contains a valid value for this member.
                std::string current_member_type = (state["type"] == "sequence") ? state["element_type"] : state["type"];
                state["struct_member_types"] += current_member_type + ";";
                state["struct_member_names"] += state["current_struct_member_name"] + ";";

                // Add the array dimensions for this member to `all_array_sizes`
                std::string current_array_sizes = state["current_array_sizes"].empty() ? "0" : state["current_array_sizes"];
                if (!state["all_array_sizes"].empty())
                {
                    state["all_array_sizes"] += ";" + current_array_sizes;
                }
                else
                {
                    state["all_array_sizes"] = current_array_sizes;
                }

                // Add the sequence size for this member to "sequence_sizes"
                std::string current_sequence_size = "0";
                if (state.count("sequence_size"))
                {
                    current_sequence_size = state["sequence_size"];

                    // Key not more needed, remove it
                    state.erase("sequence_size");
                }

                // Add the sequence size to the member list
                if (!state["sequence_sizes"].empty())
                {
                    state["sequence_sizes"] += ";" + current_sequence_size;
                }
                else
                {
                    state["sequence_sizes"] = current_sequence_size;
                }
            }
            else if (state.count("union_name") && !state["union_name"].empty())
            {
                // Add the type and name to the member lists
                // NOTE: For sequence types, the type stored in "union_member_types"
                // is the type of each element in the sequence, not "sequence".
                // Sequence type can be inferred by checking if "sequence_sizes" contains a valid value for this member.
                std::string current_member_type = (state["type"] == "sequence") ? state["element_type"] : state["type"];
                state["union_member_types"] += current_member_type;
                state["union_member_names"] += state["current_union_member_name"];

                // Add the sequence size for this member to "sequence_sizes"
                std::string current_sequence_size = "0";
                if (state.count("sequence_size"))
                {
                    current_sequence_size = state["sequence_size"];

                    // Key not more needed, remove it
                    state.erase("sequence_size");
                }

                // Add the sequence size to the member list
                state["sequence_sizes"] += current_sequence_size;
            }

            // Clear the temporary states for the next member
            state["type"].clear();

            if (state.count("element_type"))
            {
                state.erase("element_type");
            }

            if (state.count("alias"))
            {
                state.erase("alias");
            }

            if (state.count("current_struct_member_name"))
            {
                state["current_struct_member_name"].clear();
            }

            if (state.count("current_union_member_name"))
            {
                state["current_union_member_name"].clear();
            }

            if (state.count("current_array_sizes"))
            {
                state["current_array_sizes"].clear();
            }
        }
    }

};

#define load_type_action(Rule, id) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& /*in*/, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& /*operands*/) \
        { \
            std::string type{#id}; \
            if (type == "sequence") \
            { \
                state["type"] = type; \
                state.erase("parsing_sequence"); \
                if (state.count("arithmetic_expr")) \
                { \
                    state.erase("arithmetic_expr"); \
                } \
            } \
            else if (state.count("parsing_sequence") && (state["parsing_sequence"] == "true")) \
            { \
                state["element_type"] = type; \
                state["arithmetic_expr"] = ""; \
            } \
            else if (type == "string" || type == "wstring") \
            { \
                state["type"] = type; \
                state.erase("parsing_string"); \
            } \
            else \
            { \
                state["type"] = type; \
            } \
        } \
    };

load_type_action(boolean_type, boolean)
load_type_action(signed_tiny_int, int8)
load_type_action(unsigned_tiny_int, uint8)
load_type_action(octet_type, uint8)
load_type_action(signed_short_int, int16)
load_type_action(unsigned_short_int, uint16)
load_type_action(signed_long_int, int32)
load_type_action(unsigned_long_int, uint32)
load_type_action(signed_longlong_int, int64)
load_type_action(unsigned_longlong_int, uint64)
load_type_action(float_type, float)
load_type_action(double_type, double)
load_type_action(long_double_type, long double)
load_type_action(string_type, string)
load_type_action(wide_string_type, wstring)
load_type_action(sequence_type, sequence)

template<>
struct action<char_type>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        std::string type_key;
        if (state.count("parsing_sequence") && (state["parsing_sequence"] == "true"))
        {
            // We are parsing a sequence, thus <char_type> is the type of the sequence's elements
            type_key = "element_type";

            // Ready to parse the sequence size (if provided)
            state["arithmetic_expr"] = "";
        }
        else
        {
            type_key = "type";
        }

        switch (ctx->char_translation)
        {
            case Context::CHAR:
                state[type_key] = "char";
                break;
            case Context::UINT8:
                state[type_key] = "uint8";
                break;
            case Context::INT8:
                state[type_key] = "int8";
                break;
            default:
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid char type " << ctx->char_translation);
                state[type_key] = "char";
                break;
        }
    }

};

template<>
struct action<wide_char_type>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        std::string type_key;
        if (state.count("parsing_sequence") && (state["parsing_sequence"] == "true"))
        {
            // We are parsing a sequence, thus <char_type> is the type of the sequence's elements
            type_key = "element_type";

            // Ready to parse the sequence size (if provided)
            state["arithmetic_expr"] = "";
        }
        else
        {
            type_key = "type";
        }

        switch (ctx->wchar_type)
        {
            case Context::WCHAR_T:
                state[type_key] = "wchar";
                break;
            case Context::CHAR16_T:
                state[type_key] = "char16";
                break;
            default:
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid wchar type " << ctx->char_translation);
                state[type_key] = "wchar";
                break;
        }
    }

};

template<>
struct action<open_bracket>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["arithmetic_expr"] = "";
    }

};

template<>
struct action<fixed_array_size>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (!operands.empty())
        {
            DynamicData::_ref_type xdata = operands.back();
            operands.pop_back();
            if (!operands.empty())
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Finished array size parsing with non-empty operands stack.");
                throw std::runtime_error("Finished array size parsing with non-empty operands stack.");
            }

            int64_t value;
            xdata->get_int64_value(value, MEMBER_ID_INVALID);
            if (value <= 0)
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Array size is non-positive: " << value);
                throw std::runtime_error("Array size is non-positive: " + std::to_string(value));
            }

            // Append the current array size to `current_array_sizes`, separating multiple dimensions with commas
            if (state["current_array_sizes"].empty())
            {
                state["current_array_sizes"] = std::to_string(value);
            }
            else
            {
                state["current_array_sizes"] += "," + std::to_string(value);
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Empty operands stack while parsing fixed_array_size");
            throw std::runtime_error("Empty operands stack while parsing fixed_array_size");
        }

        state.erase("arithmetic_expr");
    }

};

template<>
struct action<sequence_size>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (!operands.empty())
        {
            DynamicData::_ref_type xdata = operands.back();
            operands.pop_back();
            if (!operands.empty())
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Finished sequence size parsing with non-empty operands stack.");
                throw std::runtime_error("Finished sequence size parsing with non-empty operands stack.");
            }

            int64_t value;
            xdata->get_int64_value(value, MEMBER_ID_INVALID);

            if (value <= 0)
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid sequence size: " << value);
                throw std::runtime_error("Invalid sequence size: " + std::to_string(value));
            }

            state["sequence_size"] = std::to_string(value);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Empty operands stack while parsing sequence_size");
            throw std::runtime_error("Empty operands stack while parsing sequence_size");
        }

        state.erase("arithmetic_expr");
    }

};

template<>
struct action<kw_string>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // In case of parsing a sequence, <kw_string> represents the type of the sequence's elements.
        // Bounded string declaration inside a sequence is not supported,
        // so we don't need to parse string size in this case.
        if (!state.count("parsing_sequence"))
        {
            state["parsing_string"] = "true";
        }
    }

};

template<>
struct action<kw_wstring>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // In case of parsing a sequence, <kw_wstring> represents the type of the sequence's elements.
        // Bounded string declaration inside a sequence is not supported,
        // so we don't need to parse string size in this case.
        if (!state.count("parsing_sequence"))
        {
            state["parsing_string"] = "true";
        }
    }

};

template<>
struct action<open_ang_bracket>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        if ((state.count("parsing_string") && state["parsing_string"] == "true"))
        {
            state["arithmetic_expr"] = "";
        }
    }

};

#define load_stringsize_action(Rule, id) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& /*in*/, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            DynamicData::_ref_type xdata; \
            if (!operands.empty()) \
            { \
                xdata = operands.back(); \
                operands.pop_back(); \
                if (!operands.empty()) \
                { \
                    EPROSIMA_LOG_ERROR(IDLPARSER, "Finished string size parsing with non-empty operands stack."); \
                    throw std::runtime_error("Finished string size parsing with non-empty operands stack."); \
                } \
 \
                int64_t value; \
                xdata->get_int64_value(value, MEMBER_ID_INVALID); \
                if (value <= 0) \
                { \
                    EPROSIMA_LOG_ERROR(IDLPARSER, "String size is non-positive: " << value); \
                    throw std::runtime_error("String size is non-positive: " + std::to_string(value)); \
                } \
                state[#id] = std::to_string(value); \
            } \
            else \
            { \
                EPROSIMA_LOG_ERROR(IDLPARSER, "Empty operands stack while parsing string size"); \
                throw std::runtime_error("Empty operands stack while parsing string size"); \
            } \
 \
            state.erase("arithmetic_expr"); \
        } \
    };

load_stringsize_action(string_size, string_size)
load_stringsize_action(wstring_size, wstring_size)

template<>
struct action<kw_sequence>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = "sequence";
        // Set the default sequence size to LENGTH_UNLIMITED. In case of bounded sequence, it will be overrided later.
        state["sequence_size"] = std::to_string(LENGTH_UNLIMITED);
        state["parsing_sequence"] = "true";
        state["element_type"] = "";

        // Sequence size is only parsed after the sequence's elements
        if (state.count("arithmetic_expr"))
        {
            state.erase("arithmetic_expr");
        }
    }
};

template<>
struct action<map_type>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = in.string();
        EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] map_type parsing not supported: " << state["type"]);
    }

};


template<typename T> T promote(
        DynamicData::_ref_type xdata)
{
    DynamicType::_ref_type xtype = xdata->type();
    if (TK_INT64 == xtype->get_kind())
    {
        int64_t value;
        xdata->get_int64_value(value, MEMBER_ID_INVALID);
        return static_cast<T>(value);
    }
    else if (TK_FLOAT128 == xtype->get_kind())
    {
        long double value;
        xdata->get_float128_value(value, MEMBER_ID_INVALID);
        return static_cast<T>(value);
    }
    else if (TK_BOOLEAN == xtype->get_kind())
    {
        bool value;
        xdata->get_boolean_value(value, MEMBER_ID_INVALID);
        return static_cast<T>(value);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Bad promote");
        throw std::runtime_error("Bad promote");
    }
}

TypeKind promotion_type(
        DynamicData::_ref_type a,
        DynamicData::_ref_type b)
{
    static std::map<TypeKind, int> priorities = {
        {TK_FLOAT128, 2},
        {TK_INT64, 1},
        {TK_BOOLEAN, 0},
    };

    static std::array<TypeKind, 3> infos = {
        TK_BOOLEAN,
        TK_INT64,
        TK_FLOAT128
    };

    DynamicType::_ref_type atype = a->type();
    DynamicType::_ref_type btype = b->type();
    if (atype->get_kind() == btype->get_kind())
    {
        return atype->get_kind();
    }
    else
    {
        return infos[std::max(priorities.at(atype->get_kind()), priorities.at(btype->get_kind()))];
    }
}

template<>
struct action<boolean_literal>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (state.count("arithmetic_expr"))
        {
            state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{"bool"};
        }

        // Interpret boolean values from strings
        struct custom_tf : std::numpunct<char>
        {
            std::string do_truename()  const
            {
                return "TRUE";
            }

            std::string do_falsename() const
            {
                return "FALSE";
            }

        };
        std::istringstream ss(in.string());
        ss.imbue(std::locale(ss.getloc(), new custom_tf));
        bool value;
        ss >> std::boolalpha >> value;

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicType::_ref_type xtype {factory->get_primitive_type(TK_BOOLEAN)};
        DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)};
        xdata->set_boolean_value(MEMBER_ID_INVALID, value);

        if (state.count("arithmetic_expr"))
        {
            operands.push_back(xdata);
        }
    }

};

#define load_literal_action(Rule, id, type, type_kind, set_value) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            if (state.count("arithmetic_expr")) \
            { \
                state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{#id}; \
            } \
 \
            std::istringstream ss(in.string()); \
            type value; \
            if (std::string{#id} == "octal") \
            { \
                ss >> std::oct >> value; \
            } \
            else if (std::string{#id} == "hexa") \
            { \
                ss >> std::hex >> value; \
            } \
            else \
            { \
                ss >> value; \
            } \
 \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
            DynamicType::_ref_type xtype; \
            if (std::string{#id} != "string") \
            { \
                xtype = factory->get_primitive_type(type_kind); \
            } \
            else \
            { \
                xtype = factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build(); \
            } \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
            xdata->set_value(MEMBER_ID_INVALID, value); \
 \
            if (state.count("arithmetic_expr")) \
            { \
                operands.push_back(xdata); \
            } \
        } \
    };

load_literal_action(dec_literal, decimal, int64_t, TK_INT64, set_int64_value)
load_literal_action(oct_literal, octal, int64_t, TK_INT64, set_int64_value)
load_literal_action(hex_literal, hexa, int64_t, TK_INT64, set_int64_value)
load_literal_action(float_literal, float, long double, TK_FLOAT128, set_float128_value)
load_literal_action(fixed_pt_literal, fixed, long double, TK_FLOAT128, set_float128_value)
load_literal_action(string_literal, string, std::string, TK_STRING8, set_string_value)

#define load_character_action(Rule, id, type, type_kind, set_value, offset) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            if (state.count("arithmetic_expr")) \
            { \
                state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{#id}; \
            } \
 \
            const std::string content = in.string().substr(offset, in.string().size() - (offset + 1)); \
            if (content.empty() || (content.size() < 1)) \
            { \
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid character literal: " << content); \
                throw std::runtime_error("Invalid character literal: " + content); \
            } \
 \
            type value = static_cast<type>(content[0]); \
 \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
            DynamicType::_ref_type xtype = factory->get_primitive_type(type_kind); \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
            xdata->set_value(MEMBER_ID_INVALID, value); \
 \
            if (state.count("arithmetic_expr")) \
            { \
                operands.push_back(xdata); \
            } \
        } \
    };

load_character_action(character_literal, char8, char, TK_CHAR8, set_char8_value, 1)
load_character_action(wide_character_literal, char16, wchar_t, TK_CHAR16, set_char16_value, 2)

template<>
struct action<wide_string_literal>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (state.count("arithmetic_expr"))
        {
            state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{"wstring"};
        }

        std::string content = in.string().substr(2, in.string().size() - 3);
        std::wstring value(content.begin(), content.end());

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicType::_ref_type xtype {factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build()};
        DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)};
        xdata->set_wstring_value(MEMBER_ID_INVALID, value);

        if (state.count("arithmetic_expr"))
        {
            operands.push_back(xdata);
        }
    }

};

#define float_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& /*in*/, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            if (state.count("arithmetic_expr")) \
            { \
                state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{#id}; \
            } \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            DynamicData::_ref_type s1 = *it++, s2 = *it; \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
 \
            TypeKind pt = promotion_type(s1, s2); \
            DynamicType::_ref_type xtype {factory->get_primitive_type(pt)}; \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
 \
            if (TK_INT64 == pt) \
            { \
                int64_t value = promote<int64_t>(s2) operation promote<int64_t>(s1); \
                xdata->set_int64_value(MEMBER_ID_INVALID, value); \
            } \
            else if (TK_FLOAT128 == pt) \
            { \
                long double value = promote<long double>(s2) operation promote<long double>(s1); \
                xdata->set_float128_value(MEMBER_ID_INVALID, value); \
            } \
            else \
            { \
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid arguments for operation " << #operation); \
                throw std::runtime_error("Invalid arguments for operation " #operation ); \
            } \
 \
            if (state.count("arithmetic_expr")) \
            { \
                /* update the stack */ \
                operands.pop_back(); \
                operands.pop_back(); \
                operands.push_back(xdata); \
            } \
 \
        } \
    };

#define int_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& /*in*/, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            if (state.count("arithmetic_expr")) \
            { \
                state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{#id}; \
            } \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            DynamicData::_ref_type s1 = *it++, s2 = *it; \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
 \
            TypeKind pt = promotion_type(s1, s2); \
            DynamicType::_ref_type xtype {factory->get_primitive_type(pt)}; \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
 \
            if (TK_INT64 == pt) \
            { \
                int64_t value = promote<int64_t>(s2) operation promote<int64_t>(s1); \
                xdata->set_int64_value(MEMBER_ID_INVALID, value); \
            } \
            else \
            { \
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid arguments for operation " << #operation); \
                throw std::runtime_error("Invalid arguments for operation " #operation ); \
            } \
 \
            if (state.count("arithmetic_expr")) \
            { \
                /* update the stack */ \
                operands.pop_back(); \
                operands.pop_back(); \
                operands.push_back(xdata); \
            } \
 \
        } \
    };

#define bool_op_action(Rule, id, operation, logical_op) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& /*in*/, \
            Context* /*ctx*/, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            if (state.count("arithmetic_expr")) \
            { \
                state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{#id}; \
            } \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            DynamicData::_ref_type s1 = *it++, s2 = *it; \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
 \
            TypeKind pt = promotion_type(s1, s2); \
            DynamicType::_ref_type xtype {factory->get_primitive_type(pt)}; \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
 \
            if (TK_INT64 == pt) \
            { \
                int64_t value = promote<int64_t>(s2) operation promote<int64_t>(s1); \
                xdata->set_int64_value(MEMBER_ID_INVALID, value); \
            } \
            else if (TK_BOOLEAN == pt) \
            { \
                bool value = promote<bool>(s2) logical_op promote<bool>(s1); \
                xdata->set_boolean_value(MEMBER_ID_INVALID, value); \
            } \
            else \
            { \
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid arguments for operation " << #operation); \
                throw std::runtime_error("Invalid arguments for operation " #operation ); \
            } \
 \
            if (state.count("arithmetic_expr")) \
            { \
                /* update the stack */ \
                operands.pop_back(); \
                operands.pop_back(); \
                operands.push_back(xdata); \
            } \
 \
        } \
    };

bool_op_action(or_exec, or, |, ||)
bool_op_action(xor_exec, xor, ^, !=)
bool_op_action(and_exec, and, &, &&)
int_op_action(rshift_exec, >>, >>)
int_op_action(lshift_exec, <<, <<)
int_op_action(mod_exec, mod, %)
float_op_action(add_exec, add, +)
float_op_action(sub_exec, sub, -)
float_op_action(mult_exec, mult, *)
float_op_action(div_exec, div, / )

template<>
struct action<minus_exec>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (state.count("arithmetic_expr"))
        {
            state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{"minus"};
        }

        DynamicData::_ref_type xdata = operands.back();
        DynamicType::_ref_type xtype = xdata->type();
        if (TK_INT64 == xtype->get_kind())
        {
            int64_t value;
            xdata->get_int64_value(value, MEMBER_ID_INVALID);
            xdata->set_int64_value(MEMBER_ID_INVALID, -value);
        }
        else if (TK_FLOAT128 == xtype->get_kind())
        {
            long double value;
            xdata->get_float128_value(value, MEMBER_ID_INVALID);
            xdata->set_float128_value(MEMBER_ID_INVALID, -value);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid argument for the minus unary operator");
            throw std::runtime_error("Invalid argument for the minus unary operator");
        }
    }

};

template<>
struct action<plus_exec>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // noop
        if (state.count("arithmetic_expr"))
        {
            state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{"plus"};
        }
    }

};

template<>
struct action<inv_exec>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (state.count("arithmetic_expr"))
        {
            state["arithmetic_expr"] += (state["arithmetic_expr"].empty() ? "" : ";") + std::string{"inv"};
        }

        DynamicData::_ref_type xdata = operands.back();
        DynamicType::_ref_type xtype = xdata->type();
        if (TK_INT64 == xtype->get_kind())
        {
            int64_t value;
            xdata->get_int64_value(value, MEMBER_ID_INVALID);
            xdata->set_int64_value(MEMBER_ID_INVALID, ~value);
        }
        else if (TK_BOOLEAN == xtype->get_kind())
        {
            bool value;
            xdata->get_boolean_value(value, MEMBER_ID_INVALID);
            xdata->set_boolean_value(MEMBER_ID_INVALID, !value);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid argument for the inverse unary operator");
            throw std::runtime_error("Invalid argument for the inverse unary operator");
        }
    }

};

template<>
struct action<struct_forward_dcl>
{
    // Function to handle the cleanup of state
    static void cleanup_state(
            std::map<std::string, std::string>& state)
    {
        state.erase("struct_name");
        state.erase("struct_member_types");
        state.erase("struct_member_names");
    }

    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Ensure cleanup happens at the end of this scope using a RAII-style guard
        struct CleanupGuard
        {
            std::map<std::string, std::string>& state;
            ~CleanupGuard()
            {
                cleanup_state(state);
            }

        }
        cleanup_guard{state};

        Module& module = ctx->module();
        const std::string& struct_name = state["struct_name"];
        if (module.has_symbol(struct_name, false))
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Struct " << struct_name << " was already declared.");
            throw std::runtime_error("Struct " + struct_name + " was already declared.");
        }

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name(struct_name);
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        EPROSIMA_LOG_INFO(IDLPARSER, "Found forward struct declaration: " << struct_name);
        module.structure(builder);

        if (struct_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }
    }

};

template<>
struct action<union_forward_dcl>
{
    // Function to handle the cleanup of state
    static void cleanup_state(
            std::map<std::string, std::string>& state)
    {
        state.erase("union_name");
        state.erase("union_discriminant");
        state.erase("union_labels");
        state.erase("union_member_names");
        state.erase("union_member_types");
        state.erase("current_union_member_name");
        state.erase("sequence_sizes");
        state.erase("type");
        // state.erase("union_expecting_member_name");
    }

    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Ensure cleanup happens at the end of this scope using a RAII-style guard
        struct CleanupGuard
        {
            std::map<std::string, std::string>& state;
            ~CleanupGuard()
            {
                cleanup_state(state);
            }

        }
        cleanup_guard{state};

        Module& module = ctx->module();
        const std::string& union_name = state["union_name"];
        if (module.has_symbol(union_name, false))
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Union " << union_name << " was already declared.");
            throw std::runtime_error("Union " + union_name + " was already declared.");
        }

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_UNION);
        type_descriptor->name(union_name);
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        EPROSIMA_LOG_INFO(IDLPARSER, "Found forward union declaration: " << union_name);
        module.union_switch(builder);

        if (union_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }
    }

};

template<>
struct action<kw_const>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Create empty arithmetic_expr state to indicate the start of parsing const
        state["arithmetic_expr"] = "";
    }

};

template<>
struct action<const_dcl>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        Module& module = ctx->module();
        const std::string& const_name = state["identifier"];

        EPROSIMA_LOG_INFO(IDLPARSER, "Found const: " << const_name);

        module.create_constant(const_name, operands.back());
        operands.pop_back();
        if (!operands.empty())
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Finished const parsing with non-empty operands stack.");
            throw std::runtime_error("Finished const parsing with non-empty operands stack.");
        }
        state.erase("arithmetic_expr");
    }

};

template<>
struct action<kw_enum>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Create empty enum states to indicate the start of parsing enum
        state["enum_name"] = "";
        state["enum_member_names"] = "";
    }

};

template<>
struct action<enum_dcl>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        Module& module = ctx->module();
        const std::string& enum_name = state["enum_name"];

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_ENUM);
        type_descriptor->name(enum_name);
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        std::vector<std::string> tokens = ctx->split_string(state["enum_member_names"], ';');

        for (size_t i = 0; i < tokens.size(); i++)
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(tokens[i]);
            member_descriptor->type(factory->get_primitive_type(TK_INT32));
            builder->add_member(member_descriptor);

            DynamicType::_ref_type member_type {factory->get_primitive_type(TK_INT32)};
            DynamicData::_ref_type member_data {DynamicDataFactory::get_instance()->create_data(member_type)};
            member_data->set_int32_value(MEMBER_ID_INVALID, (int32_t)i);

            module.create_constant(tokens[i], member_data, false, true); // Mark it as "from_enum"
        }

        EPROSIMA_LOG_INFO(IDLPARSER, "Found enum: " << enum_name);
        module.enum_32(enum_name, builder);

        if (enum_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }

        state.erase("enum_name");
        state.erase("enum_member_names");
    }

};

template<>
struct action<kw_struct>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Create empty struct states to indicate the start of parsing struct
        state["struct_name"] = "";
        state["struct_member_types"] = "";
        state["struct_member_names"] = "";
        state["current_struct_member_name"] = "";
        state["type"] = "";
        state["all_array_sizes"] = "";
        state["current_array_sizes"] = "";
        state["sequence_sizes"] = "";
    }

};

template<>
struct action<struct_def>
{
    // Function to handle the cleanup of state
    static void cleanup_state(
            std::map<std::string, std::string>& state)
    {
        state.erase("struct_name");
        state.erase("struct_member_types");
        state.erase("struct_member_names");
        state.erase("current_struct_member_name");
        state["type"] = "";
        state["all_array_sizes"] = "";
        state["current_array_sizes"] = "";
        state["sequence_sizes"] = "";
    }

    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Ensure cleanup happens at the end of this scope using a RAII-style guard
        struct CleanupGuard
        {
            std::map<std::string, std::string>& state;
            ~CleanupGuard()
            {
                cleanup_state(state);
            }

        }
        cleanup_guard{state};

        Module& module = ctx->module();
        const std::string& struct_name = state["struct_name"];

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name(struct_name);
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        std::vector<std::string> types = ctx->split_string(state["struct_member_types"], ';');
        std::vector<std::string> names = ctx->split_string(state["struct_member_names"], ';');
        std::vector<std::string> all_array_sizes = ctx->split_string(state["all_array_sizes"], ';');
        std::vector<std::string> sequence_sizes = ctx->split_string(state["sequence_sizes"], ';');

        for (size_t i = 0; i < types.size(); i++)
        {
            // For arrays/sequences, ctx->get_type() should return the type of each element
            DynamicType::_ref_type member_type = ctx->get_type(state, types[i]);
            if (!member_type)
            {
                EPROSIMA_LOG_WARNING(IDLPARSER, "[TODO] member type not supported: " << types[i]);
                return;
            }

            // If array sizes are specified for this member, create an array type
            if (i < all_array_sizes.size() && all_array_sizes[i] != "0")
            {
                std::vector<std::string> array_sizes = ctx->split_string(all_array_sizes[i], ',');
                std::vector<uint32_t> sizes;
                for (const auto& size : array_sizes)
                {
                    if (module.has_constant(size))
                    {
                        DynamicData::_ref_type xdata = module.constant(size);
                        int64_t size_val = 0;
                        xdata->get_int64_value(size_val, MEMBER_ID_INVALID);
                        sizes.push_back(static_cast<uint32_t>(size_val));
                    }
                    else
                    {
                        sizes.push_back(static_cast<uint32_t>(std::stoul(size)));
                    }
                }

                // Create the multi-dimensional array type
                DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(member_type, sizes)};
                if (!array_builder)
                {
                    EPROSIMA_LOG_ERROR(IDLPARSER, "Error creating array member.");
                }
                else
                {
                    member_type = array_builder->build();
                }
            }

            // If a non-null sequence size is specified for this member, create a sequence type.
            // If the member is not a sequence, it has a sequence size of 0 assigned.
            if (i < sequence_sizes.size() && sequence_sizes[i] != "0")
            {
                uint32_t size;

                if (module.has_constant(sequence_sizes[i]))
                {
                    DynamicData::_ref_type xdata = module.constant(sequence_sizes[i]);
                    int64_t size_val = 0;
                    xdata->get_int64_value(size_val, MEMBER_ID_INVALID);
                    size = static_cast<uint32_t>(size_val);
                }
                else
                {
                    size = static_cast<uint32_t>(std::stoul(sequence_sizes[i]));
                }

                // Create the sequence type
                DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(member_type, size)};
                if (!sequence_builder)
                {
                    EPROSIMA_LOG_ERROR(IDLPARSER, "Error creating sequence member.");
                }
                else
                {
                    member_type = sequence_builder->build();
                }
            }

            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(names[i]);
            member_descriptor->type(member_type);
            builder->add_member(member_descriptor);
        }

        EPROSIMA_LOG_INFO(IDLPARSER, "Found struct: " << struct_name);
        module.structure(builder);

        if (struct_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }
    }

};

template<>
struct action<kw_union>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Create empty union states to indicate the start of parsing union
        state["union_name"] = "";
        state["union_discriminant"] = "";
        state["union_labels"] = "";
        state["union_member_names"] = "";
        state["union_member_types"] = "";
        state["current_union_member_name"] = "";
        state["sequence_sizes"] = "";
        state["type"] = "";
    }

};

template<>
struct action<kw_case>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        if (state["union_discriminant"].empty())
        {
            state["union_discriminant"] = state["type"];
        }

        state["type"] = "";
        state["arithmetic_expr"] = "";
    }

};

template<>
struct action<case_label>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (!operands.empty())
        {
            DynamicData::_ref_type xdata = operands.back();
            operands.pop_back();
            if (!operands.empty())
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Finished case label parsing with non-empty operands stack.");
                throw std::runtime_error("Finished case label parsing with non-empty operands stack.");
            }

            std::string label;
            DynamicType::_ref_type xtype = xdata->type();
            if (TK_INT64 == xtype->get_kind() || TK_INT32 == xtype->get_kind())
            {
                int64_t value = 0;
                xdata->get_int64_value(value, MEMBER_ID_INVALID);
                label = std::to_string(value);
            }
            else if (TK_BOOLEAN == xtype->get_kind())
            {
                bool value = false;
                xdata->get_boolean_value(value, MEMBER_ID_INVALID);
                label = value ? "1" : "0";
            }
            else if (TK_CHAR8 == xtype->get_kind())
            {
                char value = '0';
                xdata->get_char8_value(value, MEMBER_ID_INVALID);
                label = value;
            }
            else if (TK_CHAR16 == xtype->get_kind())
            {
                wchar_t value = L'0';
                xdata->get_char16_value(value, MEMBER_ID_INVALID);
                label = static_cast<char>(value);
            }
            else
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Unsupported case label data type: " << xtype->get_kind());
                throw std::runtime_error("Unsupported case label data type: " + std::to_string(xtype->get_kind()));
            }

            if (state["union_labels"].empty() || state["union_labels"].back() == ';')
            {
                state["union_labels"] += label;
            }
            else
            {
                state["union_labels"] += "," + label;
            }
        }
        else if (in.string().find("default") != std::string::npos)
        {
            if (state["union_labels"].empty() || state["union_labels"].back() == ';')
            {
                state["union_labels"] += "default";
            }
            else
            {
                state["union_labels"] += ",default";
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Case label is neither const expression nor default.");
            throw std::runtime_error("Case label is neither const expression nor default.");
        }

        // Reset state["type"] helps parsing union member types and names
        state["type"] = "";
        state.erase("arithmetic_expr");
    }

};

template<>
struct action<switch_case>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["union_labels"] += ";";
        state["union_member_types"] += ";";
        state["union_member_names"] += ";";
        state["sequence_sizes"] += ";";
    }

};

template<>
struct action<union_def>
{
    // Function to handle the cleanup of state
    static void cleanup_state(
            std::map<std::string, std::string>& state)
    {
        state.erase("union_name");
        state.erase("union_discriminant");
        state.erase("union_labels");
        state.erase("union_member_types");
        state.erase("union_member_names");
        state.erase("sequence_sizes");
    }

    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Ensure cleanup happens at the end of this scope using a RAII-style guard
        struct CleanupGuard
        {
            std::map<std::string, std::string>& state;
            ~CleanupGuard()
            {
                cleanup_state(state);
            }

        }
        cleanup_guard{state};

        Module& module = ctx->module();
        const std::string& union_name = state["union_name"];

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        DynamicType::_ref_type discriminant_type = ctx->get_type(state, state["union_discriminant"]);
        if (!discriminant_type)
        {
            EPROSIMA_LOG_WARNING(IDLPARSER, "[TODO] union type not supported: " << state["union_discriminant"]);
            return;
        }
        type_descriptor->kind(TK_UNION);
        type_descriptor->name(union_name);
        type_descriptor->discriminator_type(discriminant_type);
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        std::vector<std::string> label_groups = ctx->split_string(state["union_labels"], ';');
        std::vector<std::string> types = ctx->split_string(state["union_member_types"], ';');
        std::vector<std::string> names = ctx->split_string(state["union_member_names"], ';');
        std::vector<std::string> sequence_sizes = ctx->split_string(state["sequence_sizes"], ';');

        std::vector<std::vector<int32_t>> labels(types.size());
        int default_label_index = -1;

        for (size_t i = 0; i < label_groups.size(); i++)
        {
            if (label_groups[i].empty())
            {
                continue; // Skip empty strings
            }
            std::vector<std::string> numbers_str = ctx->split_string(label_groups[i], ',');
            for (const auto& num_str : numbers_str)
            {
                if (num_str == "default")
                {
                    default_label_index = static_cast<int>(i); // mark the index of default label
                }
                else
                {
                    if (std::all_of(num_str.begin(), num_str.end(), ::isdigit))
                    {
                        labels[i].push_back(std::stoi(num_str));
                    }
                    else
                    {
                        labels[i].push_back(static_cast<int32_t>(num_str[0]));
                    }
                }
            }
        }

        for (size_t i = 0; i < types.size(); i++)
        {
            // For sequences, ctx->get_type() should return the type of each element
            DynamicType::_ref_type member_type = ctx->get_type(state, types[i]);
            if (!member_type)
            {
                EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] member type not supported: " << types[i]);
                return;
            }

            // If a non-null sequence size is specified for this member, create a sequence type.
            // If the member is not a sequence, it has a sequence size of 0 assigned.
            if (i < sequence_sizes.size() && sequence_sizes[i] != "0")
            {
                uint32_t size;

                if (module.has_constant(sequence_sizes[i]))
                {
                    DynamicData::_ref_type xdata = module.constant(sequence_sizes[i]);
                    int64_t size_val = 0;
                    xdata->get_int64_value(size_val, MEMBER_ID_INVALID);
                    size = static_cast<uint32_t>(size_val);
                }
                else
                {
                    size = static_cast<uint32_t>(std::stoul(sequence_sizes[i]));
                }

                // Create the sequence type
                DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(member_type, size)};
                if (!sequence_builder)
                {
                    EPROSIMA_LOG_ERROR(IDLPARSER, "Error creating sequence member.");
                }
                else
                {
                    member_type = sequence_builder->build();
                }
            }

            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(names[i]);
            member_descriptor->type(member_type);

            if (default_label_index == static_cast<int>(i))
            {
                member_descriptor->is_default_label(true);
                if (!labels[i].empty())
                {
                    member_descriptor->label(labels[i]);
                }
            }
            else
            {
                member_descriptor->is_default_label(false);
                member_descriptor->label(labels[i]);
            }

            builder->add_member(member_descriptor);
        }

        EPROSIMA_LOG_INFO(IDLPARSER, "Found union: " << union_name);
        module.union_switch(builder);

        if (union_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }
    }

};

template<>
struct action<kw_typedef>
{
    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Create empty alias states to indicate the start of parsing alias
        state["alias"] = "";
        state["current_array_sizes"] = "";
    }

};

template<>
struct action<typedef_dcl>
{
    // Function to handle the cleanup of state
    static void cleanup_state(
            std::map<std::string, std::string>& state)
    {
        state.erase("alias");
        state["current_array_sizes"] = "";

        if (state["type"] == "sequence")
        {
            state.erase("sequence_size");
            state.erase("element_type");
        }

        state["type"].clear();
    }

    template<typename Input>
    static void apply(
            const Input& /*in*/,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        // Ensure cleanup happens at the end of this scope using a RAII-style guard
        struct CleanupGuard
        {
            std::map<std::string, std::string>& state;
            ~CleanupGuard()
            {
                cleanup_state(state);
            }

        }
        cleanup_guard{state};

        Module& module = ctx->module();

        std::string alias_name;
        std::vector<std::string> array_sizes = ctx->split_string(state["current_array_sizes"], ',');

        // state["alias"] is supposed to contain up to two fields, alias type (optional) and name
        std::ptrdiff_t comma_count = std::count(state["alias"].begin(), state["alias"].end(), ',');
        if (comma_count > 1)
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid state[\"alias\"]: " << state["alias"]);
            throw std::runtime_error("Invalid state[\"alias\"]: " + state["alias"]);
        }

        if (comma_count == 1)
        {
            std::stringstream ss(state["alias"]);
            std::getline(ss, state["type"], ',');
            std::getline(ss, alias_name, ',');
        }
        else
        {
            // When alias type is a primitive type
            alias_name = state["alias"];
        }

        // For sequence types, ctx->get_type() should return the type of the elements
        DynamicType::_ref_type alias_type = (state["type"] == "sequence") ? ctx->get_type(state, state["element_type"])
                : ctx->get_type(state, state["type"]);

        if (!alias_type)
        {
            EPROSIMA_LOG_WARNING(IDLPARSER, "[TODO] alias type not supported: " << state["type"]);
            return;
        }

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_ALIAS);
        type_descriptor->name(alias_name);

        if (state["type"] == "sequence")
        {
            // Aliased type is a sequence type
            assert(state.count("sequence_size"));
            DynamicTypeBuilder::_ref_type sequence_builder
            {
                factory->create_sequence_type(
                    alias_type,
                    static_cast<uint32_t>(std::stoul(state["sequence_size"])))
            };
            type_descriptor->base_type(sequence_builder->build());
        }
        else if (!array_sizes.empty())
        {
            // Aliased type is an array type
            std::vector<uint32_t> sizes;
            for (const auto& size : array_sizes)
            {
                sizes.push_back(static_cast<uint32_t>(std::stoul(size)));
            }
            DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(alias_type, sizes)};
            type_descriptor->base_type(array_builder->build());
        }
        else
        {
            type_descriptor->base_type(alias_type);
        }

        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};

        EPROSIMA_LOG_INFO(IDLPARSER, "Found alias: " << alias_name);
        module.create_alias(alias_name, builder);

        if (alias_name == ctx->target_type_name)
        {
            ctx->builder = builder;
        }
    }

};

template<>
struct action<annotation_appl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = in.string();
        EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] annotation_appl parsing not supported: " << state["type"]);
    }

};

template<>
struct action<bitmask_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = in.string();
        EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] bitmask_dcl parsing not supported: " << state["type"]);
    }

};

template<>
struct action<bitset_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = in.string();
        EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] bitset_dcl parsing not supported: " << state["type"]);
    }

};

template<>
struct action<module_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* /*ctx*/,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& /*operands*/)
    {
        state["type"] = in.string();
        EPROSIMA_LOG_INFO(IDLPARSER, "[TODO] module_dcl parsing not supported: " << state["type"]);
    }

};

class Parser
    : public std::enable_shared_from_this<Parser>
{
protected:

    // Use protected struct to prevent the public Parser constructor being
    // invoked externally, indicate user to use the instance method instead,
    // and take advantage of make_shared in instance() which requires the
    // Parser constructor to be public.
    struct use_the_instance_method
    {
        explicit use_the_instance_method() = default;
    };

public:

    Parser(
            use_the_instance_method)
        : context_(nullptr)
    {
    }

    Parser(
            const Parser&) = delete;
    Parser& operator =(
            const Parser&) = delete;

    static std::shared_ptr<Parser> instance()
    {
        static std::shared_ptr<Parser> instance_ = std::make_shared<Parser>(use_the_instance_method{});
        return instance_;
    }

    Context parse(
            const std::string& idl_string)
    {
        Context context;
        parse(idl_string, context);
        return context;
    }

    bool parse(
            const std::string& idl_string,
            Context& context)
    {
        memory_input<> input_mem(idl_string, "idlparser");

        std::size_t issues = tao::TAO_PEGTL_NAMESPACE::analyze<document>(true);
        if (issues > 0)
        {
            context_->success = false;
            EPROSIMA_LOG_ERROR(IDLPARSER, "IDL grammar error: " << tao::TAO_PEGTL_NAMESPACE::analyze<document>(1));
            return false;
        }

        context.parser_ = shared_from_this();
        context_ = &context;

        std::map<std::string, std::string> parsing_state;
        std::vector<traits<DynamicData>::ref_type> operands;

        if (tao::TAO_PEGTL_NAMESPACE::parse<document, action>(input_mem, context_, parsing_state,
                operands) && input_mem.empty())
        {
            context_->success = true;
            EPROSIMA_LOG_INFO(IDLPARSER, "IDL parsing succeeded.");
            return true;
        }
        else
        {
            context_->success = false;
            EPROSIMA_LOG_ERROR(IDLPARSER, "IDL parsing failed.");
            return false;
        }
    }

    bool parse_file(
            const std::string& idl_file,
            Context& context)
    {
        context.parser_ = shared_from_this();
        context_ = &context;
        if (context_->preprocess)
        {
            std::string file_content = context_->preprocess_file(idl_file);
            return parse(file_content, context);
        }

        std::ostringstream os;
        os << std::ifstream(idl_file).rdbuf();
        return parse(os.str(), context);
    }

    Context parse_file(
            const std::string& idl_file)
    {
        Context context;
        parse_file(idl_file, context);
        return context;
    }

    Context parse_file(
            const std::string& idl_file,
            const std::string& type_name,
            const IncludePathSeq& include_paths,
            const std::string& preprocessor)
    {
        Context context;
        context.target_type_name = type_name;
        if (!include_paths.empty())
        {
            context.include_paths = include_paths;
            context.preprocess = true;
            context.preprocessor_exec = preprocessor;
            if (context.preprocessor_exec.empty())
            {
                context.preprocessor_exec = EPROSIMA_PLATFORM_PREPROCESSOR;
            }
        }
        parse_file(idl_file, context);
        return context;
    }

    bool parse_string(
            const std::string& idl_string,
            Context& context)
    {
        context.parser_ = shared_from_this();

        if (context.preprocess)
        {
            return parse(context.preprocess_string(idl_string), context);
        }
        else
        {
            return parse(idl_string, context);
        }
    }

    Context parse_string(
            const std::string& idl_string)
    {
        Context context;
        parse_string(idl_string, context);
        return context;
    }

    static std::string preprocess(
            const std::string& idl_file,
            const std::vector<std::string>& includes)
    {
        Context context;
        context.include_paths = includes;
        return context.preprocess_file(idl_file);
    }

private:

    friend class Context;

    Context* context_;

    traits<DynamicType>::ref_type type_spec(
            std::map<std::string, std::string>& state,
            const std::string& type)
    {
        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicTypeBuilder::_ref_type builder;
        DynamicType::_ref_type xtype;

        if (type == "boolean")
        {
            xtype = factory->get_primitive_type(TK_BOOLEAN);
        }
        else if (type == "int8")
        {
            xtype = factory->get_primitive_type(TK_INT8);
        }
        else if (type == "uint8")
        {
            xtype = factory->get_primitive_type(TK_UINT8);
        }
        else if (type == "int16")
        {
            xtype = factory->get_primitive_type(TK_INT16);
        }
        else if (type == "uint16")
        {
            xtype = factory->get_primitive_type(TK_UINT16);
        }
        else if (type == "int32")
        {
            xtype = factory->get_primitive_type(TK_INT32);
        }
        else if (type == "uint32")
        {
            xtype = factory->get_primitive_type(TK_UINT32);
        }
        else if (type == "int64")
        {
            xtype = factory->get_primitive_type(TK_INT64);
        }
        else if (type == "uint64")
        {
            xtype = factory->get_primitive_type(TK_UINT64);
        }
        else if (type == "float")
        {
            xtype = factory->get_primitive_type(TK_FLOAT32);
        }
        else if (type == "double")
        {
            xtype = factory->get_primitive_type(TK_FLOAT64);
        }
        else if (type == "long double")
        {
            xtype = factory->get_primitive_type(TK_FLOAT128);
        }
        else if (type == "char")
        {
            xtype = factory->get_primitive_type(TK_CHAR8);
        }
        else if (type == "wchar" || type == "char16")
        {
            xtype = factory->get_primitive_type(TK_CHAR16);
        }
        else if (type == "string")
        {
            uint32_t length = static_cast<uint32_t>(LENGTH_UNLIMITED);
            if (state.count("string_size"))
            {
                length = std::atoi(state["string_size"].c_str());
                state.erase("string_size");
            }
            builder = factory->create_string_type(length);
            xtype = builder->build();
        }
        else if (type == "wstring")
        {
            uint32_t length = static_cast<uint32_t>(LENGTH_UNLIMITED);
            if (state.count("wstring_size"))
            {
                length = std::atoi(state["wstring_size"].c_str());
                state.erase("wstring_size");
            }
            builder = factory->create_wstring_type(length);
            xtype = builder->build();
        }
        else
        {
            builder = context_->module().get_builder(type);
            if (builder)
            {
                xtype = builder->build();
            }
        }

        return xtype;
    }

}; // class Parser


traits<DynamicType>::ref_type Context::get_type(
        std::map<std::string, std::string>& state,
        const std::string& type)
{
    return parser_->type_spec(state, type);
}

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSER_HPP
