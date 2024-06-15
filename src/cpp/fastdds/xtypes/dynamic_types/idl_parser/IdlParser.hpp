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
#include <map>
#include <memory>
#include <regex>
#include <thread>
#include <type_traits>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <iomanip>
#include <utility>

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
#include <pegtl/analyze.hpp>

#include "IdlGrammar.hpp"
#include "IdlModule.hpp"

// mimic posix pipe APIs
#ifdef _MSC_VER
#   pragma push_macro("popen")
#   define popen _popen
#   pragma push_macro("pipe")
#   define pipe _pipe
#   pragma push_macro("pclose")
#   define pclose _pclose
#endif // ifdef _MSC_VER

// define preprocessor strategy
#ifdef _MSC_VER
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cl /EP /I."
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::temporary_file
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "/I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>nul"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "rt"
#else
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cpp -H"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::pipe_stdin
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "-I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>/dev/null"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "r"
#endif // ifdef _MSC_VER

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

using namespace tao::TAO_PEGTL_NAMESPACE;

class PreprocessorContext
{
public:

    // Preprocessors capability to use shared memory (pipes) or stick to file input
    enum class preprocess_strategy
    {
        pipe_stdin,
        temporary_file
    };

    bool preprocess = true;
    std::string preprocessor_exec = EPROSIMA_PLATFORM_PREPROCESSOR;
    std::string error_redir = EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR;
    preprocess_strategy strategy = EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY;
    std::string include_flag = EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES;
    std::vector<std::string> include_paths;

    std::string preprocess_file(
            const std::string& idl_file) const
    {
        std::vector<std::string> includes;
        std::string args;
        for (const std::string& inc_path : include_paths)
        {
            args += include_flag + inc_path + " ";
        }

        std::string cmd = preprocessor_exec + " " + args + idl_file + error_redir;

        EPROSIMA_LOG_INFO(IDLPARSER, "Calling preprocessor with command: " << cmd);
        std::string output = exec(cmd);
        EPROSIMA_LOG_INFO(IDLPARSER, "Pre-processed IDL: " << output);
        return output;
    }

    std::string preprocess_string(
            const std::string& idl_string) const;

private:

    template<preprocess_strategy e>
    std::string preprocess_string(
            const std::string& idl_string) const;

#ifdef _MSC_VER
    std::pair<std::ofstream, std::string> get_temporary_file() const
    {
        // Create temporary filename
        std::array<char, L_tmpnam> filename_buffer;
        errno_t err = tmpnam_s(filename_buffer.data(), filename_buffer.size());
        if (err != 0)
        {
            throw std::runtime_error("Failed to generate a temporary filename.");
        }

        std::ofstream tmp_file(filename_buffer.data());
        if (!tmp_file)
        {
            throw std::runtime_error("Failed to open the temporary file.");
        }

        return std::make_pair(std::move(tmp_file), std::string(filename_buffer.data()));
    }

#else
    std::pair<std::ofstream, std::string> get_temporary_file() const
    {
        // TODO
        return std::make_pair(std::ofstream{}, std::string{});
    }

#endif // _MSC_VER

    void replace_all_string(
            std::string& str,
            const std::string& from,
            const std::string& to) const
    {
        size_t froms = from.size();
        size_t tos = to.size();
        size_t pos = str.find(from);
        const std::string escaped = "\\\\\"";
        size_t escaped_size = escaped.size();
        while (pos != std::string::npos)
        {
            str.replace(pos, froms, to);
            pos = str.find(from, pos + tos);
            while (str[pos - 1] == '\\')
            {
                str.replace(pos, froms, escaped);
                pos = str.find(from, pos + escaped_size);
            }

        }
    }

    std::string exec(
            const std::string& cmd) const
    {
        std::unique_ptr<FILE, decltype(& pclose)> pipe(
            popen(cmd.c_str(), EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

    #ifdef _MSC_VER
        std::filebuf buff(pipe.get());
        std::ostringstream os;
        os << &buff;
        return os.str();
    #else
        std::array<char, 256> buffer;
        std::string result;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        return result;
    #endif // _MSC_VER
    }

}; // class PreprocessorContext

// preprocessing using pipes
template<>
inline std::string PreprocessorContext::preprocess_string<PreprocessorContext::preprocess_strategy::pipe_stdin>(
        const std::string& idl_string) const
{
    std::string args;
    for (const std::string& inc_path : include_paths)
    {
        args += include_flag + inc_path + " ";
    }
    // Escape double quotes inside the idl_string
    std::string escaped_idl_string = idl_string;
    replace_all_string(escaped_idl_string, "\"", "\\\"");
    std::string cmd = "echo \"" + escaped_idl_string + "\" | "
            + preprocessor_exec + " " + args + error_redir;

    EPROSIMA_LOG_INFO(IDLPARSER, "Calling preprocessor '" << preprocessor_exec << "' for an IDL string.");

    return exec(cmd);
}

// preprocessing using files
template<>
inline std::string PreprocessorContext::preprocess_string<PreprocessorContext::preprocess_strategy::temporary_file>(
        const std::string& idl_string) const
{
    std::string processed;

    try
    {
        auto os_tmp = get_temporary_file();

        // Populate
        os_tmp.first << idl_string;
        os_tmp.first.close();

        processed = preprocess_file(os_tmp.second);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Error: " << e.what());
    }

    return processed;
}

inline std::string PreprocessorContext::preprocess_string(
        const std::string& idl_string) const
{
    switch (strategy)
    {
        case preprocess_strategy::pipe_stdin:
            return PreprocessorContext::preprocess_string<preprocess_strategy::pipe_stdin>(idl_string);
        case preprocess_strategy::temporary_file:
            return PreprocessorContext::preprocess_string<preprocess_strategy::temporary_file>(idl_string);
        default:
            EPROSIMA_LOG_ERROR(IDLPARSER, "Unknown preprocessor strategy selected.");
            return "";
    }
}

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

    traits<DynamicType>::ref_type get_type(
            std::map<std::string, std::string>& state,
            const std::string& type);

    std::vector<std::string> split_string(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream ss(str);
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    //std::map<std::string, DynamicType::Ptr> get_all_types(
    //        bool scope = false)
    //{
    //    std::map<std::string, DynamicType::Ptr> result;

    //    if(module_)
    //    {
    //        module_->fill_all_types(result, scope);
    //    }

    //    return result;
    //}

    //std::map<std::string, DynamicType::Ptr> get_all_scoped_types()
    //{
    //    return get_all_types(true);
    //}

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
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        std::cout << "Rule: " << typeid(Rule).name() << " " << in.string() << std::endl;
    }

};

template<>
struct action<identifier>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        if (state.count("enum_name"))
        {
            if (state["enum_name"].empty())
            {
                state["enum_name"] = in.string();
            }
            else
            {
                state["enum_member_names"] += in.string() + ";";
            }
        }
        else if (state.count("struct_name"))
        {
            if (state["struct_name"].empty())
            {
                state["struct_name"] = in.string();
            }
            else
            {
                state["struct_member_types"] += state["type"] + ";";
                state["struct_member_names"] += in.string() + ";";
            }
        }
        else if (state.count("union_name"))
        {
            if (state["union_name"].empty())
            {
                state["union_name"] = in.string();
            }
            else
            {
                state["union_member_types"] += state["type"];
                state["union_member_names"] += in.string();
            }
        }
        else if (state.count("alias"))
        {
            // save alias type and alias name into state["alias"]
            state["alias"] += state["alias"].empty() ? in.string() : "," + in.string();
        }
        else
        {
            // Keep the identifier for super-expression use
            state["identifier"] = in.string();
        }
    }

};

#define load_type_action(Rule, id) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "load_type_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
            state["type"] = std::string{#id}; \
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

template<>
struct action<char_type>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        switch (ctx->char_translation)
        {
            case Context::CHAR:
                state["type"] = "char";
            case Context::UINT8:
                state["type"] = "uint8";
            case Context::INT8:
                state["type"] = "int8";
            default:
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid char type " << ctx->char_translation);
                state["type"] = "char";
        }
    }

};

template<>
struct action<wide_char_type>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        switch (ctx->wchar_type)
        {
            case Context::WCHAR_T:
                state["type"] = "wchar";
            case Context::CHAR16_T:
                state["type"] = "char16";
            default:
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid wchar type " << ctx->char_translation);
                state["type"] = "wchar";
        }
    }

};

template<>
struct action<positive_int_const>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        state["positive_int_const"] = in.string();
    }

};

#define load_stringsize_action(Rule, id) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "load_stringsize_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
            if (state.count("positive_int_const")) \
            { \
                state[#id] = state["positive_int_const"]; \
                state.erase("positive_int_const"); \
            } \
        } \
    };

load_stringsize_action(string_size, string_size)
load_stringsize_action(wstring_size, wstring_size)

// // TODO sequence type, map type

template<typename T> T promote(
        DynamicData::_ref_type xdata)
{
    DynamicType::_ref_type xtype = xdata->type();
    if (TK_UINT64 == xtype->get_kind())
    {
        uint64_t value;
        xdata->get_uint64_value(value, MEMBER_ID_INVALID);
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
        throw std::runtime_error("bad promote");
    }
}

const TypeKind promotion_type(
        DynamicData::_ref_type a,
        DynamicData::_ref_type b)
{
    static std::map<TypeKind, int> priorities = {
        {TK_FLOAT128, 2},
        {TK_UINT64, 1},
        {TK_BOOLEAN, 0},
    };

    static std::array<TypeKind, 3> infos = {
        TK_BOOLEAN,
        TK_UINT64,
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
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        std::cout << "boolean_literal: " << typeid(boolean_literal).name()
                  << " " << in.string() << std::endl;

        state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{"bool"};

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

        operands.push_back(xdata);
    }

};

#define load_literal_action(Rule, id, type, type_kind, set_value) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "load_literal_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{#id}; \
 \
            std::istringstream ss(in.string()); \
            type value; \
            if (#id == "octal") ss >> std::setbase(std::ios_base::oct) >> value; \
            else if (#id == "hexa") ss >> std::setbase(std::ios_base::hex) >> value; \
            else ss >> value; \
 \
            DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()}; \
            DynamicType::_ref_type xtype {factory->get_primitive_type(type_kind)}; \
            DynamicData::_ref_type xdata {DynamicDataFactory::get_instance()->create_data(xtype)}; \
            xdata->set_value(MEMBER_ID_INVALID, value); \
            operands.push_back(xdata); \
        } \
    };

load_literal_action(dec_literal, decimal, uint64_t, TK_UINT64, set_uint64_value)
load_literal_action(oct_literal, octal, uint64_t, TK_UINT64, set_uint64_value)
load_literal_action(hex_literal, hexa, uint64_t, TK_UINT64, set_uint64_value)
load_literal_action(float_literal, float, long double, TK_FLOAT128, set_float128_value)
load_literal_action(fixed_pt_literal, fixed, long double, TK_FLOAT128, set_float128_value)

#define float_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "float_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{#id}; \
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
            if (TK_UINT64 == pt) \
            { \
                uint64_t value = promote<uint64_t>(s2) operation promote<uint64_t>(s1); \
                xdata->set_uint64_value(MEMBER_ID_INVALID, value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
            } \
            else if (TK_FLOAT128 == pt) \
            { \
                long double value = promote<long double>(s2) operation promote<long double>(s1); \
                xdata->set_float128_value(MEMBER_ID_INVALID, value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(xdata); \
 \
        } \
    };

#define int_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "int_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{#id}; \
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
            if (TK_UINT64 == pt) \
            { \
                uint64_t value = promote<uint64_t>(s2) operation promote<uint64_t>(s1); \
                xdata->set_uint64_value(MEMBER_ID_INVALID, value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(xdata); \
 \
        } \
    };

#define bool_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::vector<traits<DynamicData>::ref_type>& operands) \
        { \
            std::cout << "bool_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{#id}; \
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
            if (TK_UINT64 == pt) \
            { \
                uint64_t value = promote<uint64_t>(s2) operation promote<uint64_t>(s1); \
                xdata->set_uint64_value(MEMBER_ID_INVALID, value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
            } \
            else if (TK_BOOLEAN == pt) \
            { \
                bool value = promote<bool>(s2) operation promote<bool>(s1); \
                xdata->set_boolean_value(MEMBER_ID_INVALID, value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(xdata); \
 \
        } \
    };

bool_op_action(or_exec, or, |)
bool_op_action(xor_exec, xor, ^)
bool_op_action(and_exec, and, &)
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
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        std::cout << "minus_exec: " << typeid(minus_exec).name() << " "
                  << in.string() << std::endl;

        state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{"minus"};

        DynamicData::_ref_type xdata = operands.back();
        DynamicType::_ref_type xtype = xdata->type();
        if (TK_UINT64 == xtype->get_kind())
        {
            uint64_t value;
            xdata->get_uint64_value(value, MEMBER_ID_INVALID);
            xdata->set_uint64_value(MEMBER_ID_INVALID, -value);
        }
        else if (TK_FLOAT128 == xtype->get_kind())
        {
            long double value;
            xdata->get_float128_value(value, MEMBER_ID_INVALID);
            xdata->set_float128_value(MEMBER_ID_INVALID, -value);
        }
        else
        {
            throw std::runtime_error("invalid argument for the minus unary operator");
        }
    }

};

template<>
struct action<plus_exec>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        // noop
        std::cout << "plus_exec: " << typeid(plus_exec).name() << " "
                  << in.string() << std::endl;
        state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{"plus"};
    }

};

template<>
struct action<inv_exec>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        std::cout << "inv_exec: " << typeid(inv_exec).name() << " "
                  << in.string() << std::endl;

        state["evaluated_expr"] += (state["evaluated_expr"].empty() ? "" : ";") + std::string{"inv"};

        DynamicData::_ref_type xdata = operands.back();
        DynamicType::_ref_type xtype = xdata->type();
        if (TK_UINT64 == xtype->get_kind())
        {
            uint64_t value;
            xdata->get_uint64_value(value, MEMBER_ID_INVALID);
            xdata->set_uint64_value(MEMBER_ID_INVALID, ~value);
        }
        else if (TK_BOOLEAN == xtype->get_kind())
        {
            bool value;
            xdata->get_boolean_value(value, MEMBER_ID_INVALID);
            xdata->set_boolean_value(MEMBER_ID_INVALID, !value);
        }
        else
        {
            throw std::runtime_error("invalid argument for the inverse unary operator");
        }
    }

};

template<>
struct action<struct_forward_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
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
        DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};

        EPROSIMA_LOG_INFO(IDLPARSER, "Found forward struct declaration: " << struct_name);
        module.structure(type_builder);

        ctx->builder = type_builder;

        state.erase("struct_name");
        state.erase("struct_member_types");
        state.erase("struct_member_names");
    }

};

// template<>
// struct action<union_forward_dcl>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();
//         const std::string& union_name = state["union_name"];
//         if (module.has_symbol(union_name, false))
//         {
//             EPROSIMA_LOG_ERROR(IDLPARSER, "Union " << union_name << " was already declared.");
//             throw std::runtime_error("Union " + union_name + " was already declared.");
//         }

//         v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
//         v1_3::DynamicTypeBuilder_cptr discriminator = factory.create_int32_type();
//         auto discriminant_type = discriminator->build();
//         v1_3::DynamicTypeBuilder_ptr builder = factory.create_union_type(*discriminant_type);
//         builder->set_name(union_name);
//         auto union_type = builder->build();
//         EPROSIMA_LOG_INFO(IDLPARSER, "Found forward union declaration: " << union_name);
//         module.union_switch(std::move(const_cast<v1_3::DynamicType&>(*union_type)));

//         state.erase("union_name");
//         state.erase("union_discriminant");
//         state.erase("union_labels");
//         state.erase("union_member_types");
//         state.erase("union_member_names");
//     }

// };

template<>
struct action<const_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        Module& module = ctx->module();
        const std::string& const_name = state["identifier"];

        EPROSIMA_LOG_INFO(IDLPARSER, "Found const: " << const_name);

        module.create_constant(const_name, operands.back());
        operands.pop_back();
        if (operands.empty())
        {
            state["evaluated_expr"].clear();
        }
    }

};

// template<>
// struct action<kw_enum>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         // Create empty enum states to indicate the start of parsing enum
//         state["enum_name"] = "";
//         state["enum_member_names"] = "";
//     }

// };

// template<>
// struct action<enum_dcl>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();

//         v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
//         v1_3::DynamicTypeBuilder_ptr builder = factory.create_enum_type();

//         std::vector<std::string> tokens = ctx->split_string(state["enum_member_names"], ';');

//         for (int i = 0; i < tokens.size(); i++)
//         {
//             v1_3::DynamicTypeBuilder_cptr member_builder = factory.create_uint32_type();
//             auto member_type = member_builder->build();
//             v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(member_type));

//             builder->add_member(i, tokens[i]);
//             module.create_constant(tokens[i], data, false, true); // Mark it as "from_enum"
//         }

//         const std::string& enum_name = state["enum_name"];
//         builder->set_name(enum_name);
//         auto enum_type = builder->build();
//         EPROSIMA_LOG_INFO(IDLPARSER, "Found enum: " << enum_name);
//         module.enum_32(enum_name, enum_type);

//         state.erase("enum_name");
//         state.erase("enum_member_names");
//     }

// };

template<>
struct action<kw_struct>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        // Create empty struct states to indicate the start of parsing struct
        state["struct_name"] = "";
        state["struct_member_types"] = "";
        state["struct_member_names"] = "";
    }

};

template<>
struct action<struct_def>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::vector<traits<DynamicData>::ref_type>& operands)
    {
        Module& module = ctx->module();

        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name(state["struct_name"]);
        DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};

        std::vector<std::string> types = ctx->split_string(state["struct_member_types"], ';');
        std::vector<std::string> names = ctx->split_string(state["struct_member_names"], ';');

        for (int i = 0; i < types.size(); i++)
        {
            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            member_descriptor->name(names[i]);
            member_descriptor->type(ctx->get_type(state, types[i]));
            type_builder->add_member(member_descriptor);
        }

        EPROSIMA_LOG_INFO(IDLPARSER, "Found struct: " << state["struct_name"]);
        module.structure(type_builder);

        ctx->builder = type_builder;

        state.erase("struct_name");
        state.erase("struct_member_types");
        state.erase("struct_member_names");
    }

};

// template<>
// struct action<kw_union>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         // Create empty union states to indicate the start of parsing union
//         state["union_name"] = "";
//         state["union_discriminant"] = "";
//         state["union_labels"] = "";
//         state["union_member_names"] = "";
//         state["union_member_types"] = "";
//     }

// };

// template<>
// struct action<case_label>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();
//         std::string label;

//         for (char c : in.string())
//         {
//             if (std::isdigit(c)) {
//                 label += c;
//             }
//         }
//         if (label.empty() && in.string().find("default") != std::string::npos)
//         {
//             if (state["union_labels"].empty() || state["union_labels"].back() == ';')
//             {
//                 state["union_labels"] += "default";
//             }
//             else
//             {
//                 state["union_labels"] += ",default";
//             }
//         }
//         else
//         {
//             if (state["union_labels"].empty() || state["union_labels"].back() == ';')
//             {
//                 state["union_labels"] += label;
//             }
//             else
//             {
//                 state["union_labels"] += "," + label;
//             }
//         }

//         if (state["union_discriminant"].empty())
//         {
//             state["union_discriminant"] = state["type"];
//         }
//     }

// };

// template<>
// struct action<switch_case>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();

//         state["union_labels"] += ";";
//         state["union_member_types"] += ";";
//         state["union_member_names"] += ";";
//     }

// };

// template<>
// struct action<union_def>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();

//         auto discriminant_type = ctx->get_type(state, state["union_discriminant"]);
//         v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
//         v1_3::DynamicTypeBuilder_ptr builder = factory.create_union_type(*discriminant_type);

//         const std::string& union_name = state["union_name"];
//         std::vector<std::string> label_groups = ctx->split_string(state["union_labels"], ';');
//         std::vector<std::string> types = ctx->split_string(state["union_member_types"], ';');
//         std::vector<std::string> names = ctx->split_string(state["union_member_names"], ';');

//         std::vector<std::vector<uint64_t>> labels;
//         int idx = 0;
//         for (int i = 0; i < label_groups.size(); i++)
//         {
//             if (label_groups[i].empty()) continue; // Skip empty strings
//             std::vector<std::string> numbers_str = ctx->split_string(label_groups[i], ',');
//             std::vector<uint64_t> numbers;
//             for (const auto& num_str : numbers_str)
//             {
//                 if (num_str == "default")
//                 {
//                     numbers.push_back(std::numeric_limits<uint64_t>::max());
//                     idx = i; // mark the index of default label
//                 }
//                 else
//                 {
//                     numbers.push_back(std::stoi(num_str));
//                 }
//             }
//             if (!numbers.empty())   labels.push_back(numbers);
//         }

//         for (int i = 0; i < types.size(); i++)
//         {
//             auto member_type = ctx->get_type(state, types[i]);
//             builder->add_member(v1_3::MemberId{ i }, names[i], member_type, "", labels[i], i == idx ? true : false);
//         }
//         builder->set_name(union_name);
//         auto union_type = builder->build();
//         EPROSIMA_LOG_INFO(IDLPARSER, "Found union: " << union_name);
//         module.union_switch(std::move(const_cast<v1_3::DynamicType&>(*union_type)));

//         state.erase("union_name");
//         state.erase("union_discriminant");
//         state.erase("union_labels");
//         state.erase("union_member_types");
//         state.erase("union_member_names");
//     }

// };

// template<>
// struct action<kw_typedef>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         // Create empty alias states to indicate the start of parsing alias
//         state["alias"] = "";
//         state["alias_sizes"] = "";
//     }

// };

// template<>
// struct action<fixed_array_size>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         if (state.count("alias") && !state["alias"].empty())
//         {
//             std::string str = in.string();

//             // Find the opening and closing brackets
//             size_t start_pos = str.find('[');
//             size_t end_pos = str.find(']');

//             // Extract the substring between the brackets and trim spaces
//             std::string size = str.substr(start_pos + 1, end_pos - start_pos - 1);
//             size.erase(0, size.find_first_not_of(" \t\n\r"));
//             size.erase(size.find_last_not_of(" \t\n\r") + 1);

//             state["alias_sizes"] += size + ";";
//         }
//     }

// };

// template<>
// struct action<typedef_dcl>
// {
//     template<typename Input>
//     static void apply(
//             const Input& in,
//             Context* ctx,
//             std::map<std::string, std::string>& state,
//             std::vector<v1_3::DynamicData_ptr>& operands)
//     {
//         Module& module = ctx->module();

//         std::string alias_name;
//         std::vector<std::string> sizes_str = ctx->split_string(state["alias_sizes"], ';');

//         std::stringstream ss(state["alias"]);
//         std::getline(ss, state["type"], ',');
//         std::getline(ss, alias_name, ',');

//         auto alias_type = ctx->get_type(state, state["type"]);
//         if (sizes_str.empty())
//         {
//             module.create_alias(alias_name, alias_type);
//         }
//         else
//         {
//             std::vector<uint32_t> sizes;
//             for (const auto& size : sizes_str)
//             {
//                 sizes.push_back(static_cast<uint32_t>(std::stoul(size)));
//             }
//             v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
//             v1_3::DynamicTypeBuilder_ptr array_type_builder = factory.create_array_type(*alias_type, sizes);
//             auto array_type = array_type_builder->build();

//             module.create_alias(alias_name, array_type);
//         }

//         state.erase("alias");
//         state.erase("alias_sizes");
//     }

// };

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
        parsing_state["evaluated_expr"] = "";

        if (tao::TAO_PEGTL_NAMESPACE::parse<document, action>(input_mem, context_, parsing_state, operands) && input_mem.empty())
        {
            context_->success = true;
            EPROSIMA_LOG_INFO(IDLPARSER, "IDL parsing succeeded.");
            return true;
        }
        else
        {
            context_->success = false;
            EPROSIMA_LOG_INFO(IDLPARSER, "IDL parsing failed.");
            return false;
        }
    }

    Context parse_file(
            const std::string& idl_file)
    {
        Context context;
        parse_file(idl_file, context);
        return context;
    }

    Context parse_string(
            const std::string& idl_string)
    {
        Context context;
        parse_string(idl_string, context);
        return context;
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

    //void get_all_types(
    //    std::map<std::string, DynamicType::Ptr>& types_map)
    //{
    //    if (context_)
    //    {
    //        context_->module().fill_all_types(types_map);
    //    }
    //}

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
    //using LabelsCaseMemberPair = std::pair<std::vector<std::string>, Member>;

    //peg::parser parser_;
    Context* context_;

    traits<DynamicType>::ref_type type_spec(
            std::map<std::string, std::string>& state,
            const std::string& type)
    {
        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicTypeBuilder::_ref_type builder;
        traits<DynamicType>::ref_type dtype;

        if (type == "boolean")
        {
            dtype = factory->get_primitive_type(TK_BOOLEAN);
        }
        else if (type == "int8")
        {
            dtype = factory->get_primitive_type(TK_INT8);
        }
        else if (type == "uint8")
        {
            dtype = factory->get_primitive_type(TK_UINT8);
        }
        else if (type == "int16")
        {
            dtype = factory->get_primitive_type(TK_INT16);
        }
        else if (type == "uint16")
        {
            dtype = factory->get_primitive_type(TK_UINT16);
        }
        else if (type == "int32")
        {
            dtype = factory->get_primitive_type(TK_INT32);
        }
        else if (type == "uint32")
        {
            dtype = factory->get_primitive_type(TK_UINT32);
        }
        else if (type == "int64")
        {
            dtype = factory->get_primitive_type(TK_INT64);
        }
        else if (type == "uint64")
        {
            dtype = factory->get_primitive_type(TK_UINT64);
        }
        else if (type == "float")
        {
            dtype = factory->get_primitive_type(TK_FLOAT32);
        }
        else if (type == "double")
        {
            dtype = factory->get_primitive_type(TK_FLOAT64);
        }
        else if (type == "long double")
        {
            dtype = factory->get_primitive_type(TK_FLOAT128);
        }
        // else if (type == "char")
        // {
        //     builder = factory.create_char8_type();
        //     dtype = builder->build();
        // }
        // else if (type == "wchar" || type == "char16")
        // {
        //     builder = factory.create_char16_type();
        //     dtype = builder->build();
        // }
        // else if (type == "string")
        // {
        //     if (state.count("string_size"))
        //     {
        //         builder = factory.create_string_type(std::atoi(state["string_size"].c_str()));
        //         state.erase("string_size");
        //     }
        //     else
        //     {
        //         builder = factory.create_string_type();
        //     }
        //     dtype = builder->build();
        // }
        // else if (type == "wstring")
        // {
        //     if (state.count("wstring_size"))
        //     {
        //         builder = factory.create_wstring_type(std::atoi(state["wstring_size"].c_str()));
        //         state.erase("wstring_size");
        //     }
        //     else
        //     {
        //         builder = factory.create_wstring_type();
        //     }
        //     dtype = builder->build();
        // }
        // else
        // {
        //     dtype = context_->module().type(type);
        // }

        return dtype;
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


#ifdef _MSVC_LANG
#   pragma pop_macro("popen")
#   pragma pop_macro("pipe")
#   pragma pop_macro("pclose")
#endif // ifdef _MSVC_LANG

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSER_HPP
