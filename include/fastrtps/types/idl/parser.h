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

#ifndef TYPES_IDL_PARSER_H_
#define TYPES_IDL_PARSER_H_

#include <fastrtps/types/idl/grammar.h>
#include <fastrtps/types/idl/module.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastrtps/types/v1_3/DynamicTypeBuilder.hpp>
#include <fastrtps/types/v1_3/DynamicTypeBuilderFactory.hpp>
#include <fastrtps/types/v1_3/DynamicDataFactory.hpp>

#include "pegtl.hpp"
#include <pegtl/analyze.hpp>

#ifdef _MSC_VER
#   include <cstdio>
#else
#   include <stdlib.h>
#   include <unistd.h>
#endif //_MSC_VER

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

//namespace peg {
//
//using Ast = AstBase<EmptyType>;
//
//} // namespace peg

namespace eprosima {
namespace fastrtps {
namespace types {
namespace idl {

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
        if (std::tmpnam(filename_buffer.data()) == nullptr)
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

    v1_3::DynamicType_ptr get_type(
            std::map<std::string, std::string>& state);

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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        // state["enum"] being empty string indicates that the identifier is the enum;
        // being non-empty means the identifiers are enum's members.
        if (state.count("enum"))
        {
            if (state["enum"].empty())
            {
                state["enum"] = in.string();
            }
            else
            {
                evaluated += evaluated.empty() ? in.string() : "," + in.string();
            }
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
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
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
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
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

// TODO sequence type, map type

template<typename T> T promote(
        v1_3::DynamicData_ptr x)
{
    if (TypeKind::TK_UINT64 == x->get_kind())
    {
        long long value = x->get_uint64_value();
        return static_cast<T>(value);
    }
    else if (TypeKind::TK_FLOAT128 == x->get_kind())
    {
        long double value = x->get_float128_value();
        return static_cast<T>(value);
    }
    else if (TypeKind::TK_BOOLEAN == x->get_kind())
    {
        bool value = x->get_bool_value();
        return static_cast<T>(value);
    }
    else
    {
        throw std::runtime_error("bad promote");
    }
}

const TypeKind promotion_type(
        v1_3::DynamicData_ptr a,
        v1_3::DynamicData_ptr b)
{
    static std::map<TypeKind, int> priorities = {
        {TypeKind::TK_FLOAT128, 2},
        {TypeKind::TK_UINT64, 1},
        {TypeKind::TK_BOOLEAN, 0},
    };

    static std::array<TypeKind, 3> infos = {
        TypeKind::TK_BOOLEAN,
        TypeKind::TK_UINT64,
        TypeKind::TK_FLOAT128
    };

    if (a->get_kind() == b->get_kind())
    {
        return a->get_kind();
    }
    else
    {
        return infos[std::max(priorities.at(a->get_kind()), priorities.at(b->get_kind()))];
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        std::cout << "boolean_literal: " << typeid(boolean_literal).name()
                  << " " << in.string() << std::endl;

        evaluated += (evaluated.empty() ? "" : ";") + std::string{"bool"};

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

        bool res;
        ss >> std::boolalpha >> res;
        v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
        v1_3::DynamicTypeBuilder_cptr builder = factory.create_bool_type();
        auto type = builder->build();
        v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(type));
        data->set_bool_value(res);

        operands.push_back(data);
    }

};

#define load_literal_action(Rule, id, type, create_type, set_value) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
        { \
            std::cout << "load_literal_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            evaluated += (evaluated.empty() ? "" : ";") + std::string{#id}; \
 \
            std::istringstream ss(in.string()); \
            type res; \
            if (#id == "octal") ss >> std::setbase(std::ios_base::oct) >> res; \
            else if (#id == "hexa") ss >> std::setbase(std::ios_base::hex) >> res; \
            else ss >> res; \
 \
            v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance(); \
            v1_3::DynamicTypeBuilder_cptr builder = factory.create_type; \
            auto xtype = builder->build(); \
            v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
            data->set_value(res); \
            operands.push_back(data); \
        } \
    };

load_literal_action(dec_literal, decimal, long long, create_uint64_type(), set_uint64_value)
load_literal_action(oct_literal, octal, long long, create_uint64_type(), set_uint64_value)
load_literal_action(hex_literal, hexa, long long, create_uint64_type(), set_uint64_value)
load_literal_action(float_literal, float, long double, create_float128_type(), set_float128_value)
load_literal_action(fixed_pt_literal, fixed, long double, create_float128_type(), set_float128_value)

#define float_op_action(Rule, id, operation) \
    template<> \
    struct action<Rule> \
    { \
        template<typename Input> \
        static void apply( \
            const Input& in, \
            Context * ctx, \
            std::map<std::string, std::string>& state, \
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
        { \
            std::cout << "float_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            evaluated += (evaluated.empty() ? "" : ";") + std::string{#id}; \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            v1_3::DynamicData_ptr s1 = *it++, s2 = *it, res; \
            v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance(); \
            v1_3::DynamicTypeBuilder_cptr builder = nullptr; \
            v1_3::DynamicType_ptr xtype = nullptr; \
 \
            TypeKind pt = promotion_type(s1, s2); \
 \
            if (TypeKind::TK_UINT64 == pt) \
            { \
                long long value = promote<long long>(s2) operation promote<long long>(s1); \
                builder = factory.create_uint64_type(); \
                xtype = builder->build(); \
                v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
                data->set_uint64_value(value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
                res = data; \
            } \
            else if (TypeKind::TK_FLOAT128 == pt) \
            { \
                long double value = promote<long double>(s2) operation promote<long double>(s1); \
                builder = factory.create_float128_type(); \
                xtype = builder->build(); \
                v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
                data->set_float128_value(value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
                res = data; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(res); \
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
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
        { \
            std::cout << "int_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            evaluated += (evaluated.empty() ? "" : ";") + std::string{#id}; \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            v1_3::DynamicData_ptr s1 = *it++, s2 = *it, res; \
            v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance(); \
            v1_3::DynamicTypeBuilder_cptr builder = nullptr; \
            v1_3::DynamicType_ptr xtype = nullptr; \
 \
            TypeKind pt = promotion_type(s1, s2); \
 \
            if (TypeKind::TK_UINT64 == pt) \
            { \
                long long value = promote<long long>(s2) operation promote<long long>(s1); \
                builder = factory.create_uint64_type(); \
                xtype = builder->build(); \
                v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
                data->set_uint64_value(value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
                res = data; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(res); \
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
            std::string& evaluated, \
            std::vector<v1_3::DynamicData_ptr>& operands) \
        { \
            std::cout << "bool_op_action: " << typeid(Rule).name() << " " \
                      << in.string() << std::endl; \
 \
            evaluated += (evaluated.empty() ? "" : ";") + std::string{#id}; \
 \
            /* calculate the result */ \
            auto it = operands.rbegin(); \
            v1_3::DynamicData_ptr s1 = *it++, s2 = *it, res; \
            v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance(); \
            v1_3::DynamicTypeBuilder_cptr builder = nullptr; \
            v1_3::DynamicType_ptr xtype = nullptr; \
 \
            TypeKind pt = promotion_type(s1, s2); \
 \
            if (TypeKind::TK_UINT64 == pt) \
            { \
                long long value = promote<long long>(s2) operation promote<long long>(s1); \
                builder = factory.create_uint64_type(); \
                xtype = builder->build(); \
                v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
                data->set_uint64_value(value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
                res = data; \
            } \
            else if (TypeKind::TK_BOOLEAN == pt) \
            { \
                bool value = promote<bool>(s2) operation promote<bool>(s1); \
                builder = factory.create_bool_type(); \
                xtype = builder->build(); \
                v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(xtype)); \
                data->set_bool_value(value); \
                std::cout << "=========" << std::endl; \
                std::cout << #operation << ": " << value << std::endl; \
                std::cout << "=========" << std::endl; \
                res = data; \
            } \
            else \
            { \
                throw std::runtime_error("invalid arguments for the operation " #operation ); \
            } \
 \
            /* update the stack */ \
            operands.pop_back(); \
            operands.pop_back(); \
            operands.push_back(res); \
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        std::cout << "minus_exec: " << typeid(minus_exec).name() << " "
                  << in.string() << std::endl;

        evaluated += (evaluated.empty() ? "" : ";") + std::string{"minus"};

        if (TypeKind::TK_UINT64 == operands.back()->get_kind())
        {
            long long value = operands.back()->get_uint64_value();
            operands.back()->set_uint64_value(-value);
        }
        else if (TypeKind::TK_FLOAT128 == operands.back()->get_kind())
        {
            long double value = operands.back()->get_float128_value();
            operands.back()->set_float128_value(-value);
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        // noop
        std::cout << "plus_exec: " << typeid(plus_exec).name() << " "
                  << in.string() << std::endl;
        evaluated += (evaluated.empty() ? "" : ";") + std::string{"plus"};
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        std::cout << "inv_exec: " << typeid(inv_exec).name() << " "
                  << in.string() << std::endl;

        evaluated += (evaluated.empty() ? "" : ";") + std::string{"inv"};

        if (TypeKind::TK_UINT64 == operands.back()->get_kind())
        {
            long long value = operands.back()->get_uint64_value();
            operands.back()->set_uint64_value(~value);
        }
        else if (TypeKind::TK_BOOLEAN == operands.back()->get_kind())
        {
            bool value = operands.back()->get_bool_value();
            operands.back()->set_bool_value(!value);
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
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        const std::string& name = state["identifier"];
        Module& module = ctx->module();
        if (module.has_symbol(name, false))
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Struct " << name << " was already declared.");
            throw std::runtime_error("Struct " + name + " was already declared.");
        }

        v1_3::DynamicTypeBuilder_ptr builder =
                v1_3::DynamicTypeBuilderFactory::get_instance().create_struct_type();
        builder->set_name(name);
        auto struct_type = builder->build();
        EPROSIMA_LOG_INFO(IDLPARSER, "Found forward struct declaration: " << name);
        module.structure(std::move(const_cast<v1_3::DynamicType&>(*struct_type)));
    }

};

template<>
struct action<union_forward_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        const std::string& name = state["identifier"];
        Module& module = ctx->module();
        if (module.has_symbol(name, false))
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Union " << name << " was already declared.");
            throw std::runtime_error("Union " + name + " was already declared.");
        }

        v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
        v1_3::DynamicTypeBuilder_cptr discriminator = factory.create_int32_type();
        auto discriminant_type = discriminator->build();
        v1_3::DynamicTypeBuilder_ptr builder = factory.create_union_type(*discriminant_type);
        builder->set_name(name);
        auto union_type = builder->build();
        EPROSIMA_LOG_INFO(IDLPARSER, "Found forward union declaration: " << name);
        module.union_switch(std::move(const_cast<v1_3::DynamicType&>(*union_type)));
    }

};

template<>
struct action<const_dcl>
{
    template<typename Input>
    static void apply(
            const Input& in,
            Context* ctx,
            std::map<std::string, std::string>& state,
            std::string& evaluated,
            std::vector<v1_3::DynamicData_ptr>& operands)
    {
        const std::string& name = state["identifier"];
        Module& module = ctx->module();

        EPROSIMA_LOG_INFO(IDLPARSER, "Found const: " << name);
        module.create_constant(name, operands.back());
        operands.pop_back();
    }

};

template<>
struct action<kw_enum>
{
    template<typename Input>
    static void apply(
        const Input& in,
        Context* ctx,
        std::map<std::string, std::string>& state,
        std::string& evaluated,
        std::vector<v1_3::DynamicData_ptr>& operands)
    {
        // Set state["enum"] to empty string to indicate that next identifier is enum
        state["enum"] = "";
    }

};

template<>
struct action<enum_dcl>
{
    template<typename Input>
    static void apply(
        const Input& in,
        Context* ctx,
        std::map<std::string, std::string>& state,
        std::string& evaluated,
        std::vector<v1_3::DynamicData_ptr>& operands)
    {
        const std::string& name = state["enum"];
        Module& module = ctx->module();

        std::istringstream ss(evaluated);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ','))
        {
            tokens.push_back(token);
        }

        v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
        v1_3::DynamicTypeBuilder_ptr builder = factory.create_enum_type();

        for (int i = 0; i < tokens.size(); i++)
        {
            v1_3::DynamicTypeBuilder_cptr member_builder = factory.create_uint32_type();
            auto member_type = member_builder->build();
            v1_3::DynamicData_ptr data(v1_3::DynamicDataFactory::get_instance()->create_data(member_type));

            builder->add_member(i, tokens[i]);
            module.create_constant(tokens[i], data, false, true); // Mark it as "from_enum"
        }
        builder->set_name(name);
        auto enum_type = builder->build();
        EPROSIMA_LOG_INFO(IDLPARSER, "Found enum: " << name);
        module.enum_32(name, enum_type);

        state.erase("enum");
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
        std::map<std::string, std::string> parsing_state;
        std::vector<v1_3::DynamicData_ptr> operands;

        std::size_t issues = tao::TAO_PEGTL_NAMESPACE::analyze<document>(-1);
        if (issues > 0)
        {
            context_->success = false;
            EPROSIMA_LOG_ERROR(IDLPARSER, "IDL grammar error: " << tao::TAO_PEGTL_NAMESPACE::analyze<document>(1));
            return false;
        }

        context.parser_ = shared_from_this();
        context_ = &context;
        std::string evaluated;
        if (tao::TAO_PEGTL_NAMESPACE::parse<document, action>(input_mem, context_, parsing_state, evaluated,
                operands) && input_mem.empty())
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

    friend struct Context;
    //using LabelsCaseMemberPair = std::pair<std::vector<std::string>, Member>;

    //peg::parser parser_;
    Context* context_;

    v1_3::DynamicType_ptr type_spec(
            std::map<std::string, std::string>& state)
    {
        v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
        v1_3::DynamicTypeBuilder_cptr builder = nullptr;
        v1_3::DynamicType_ptr type = nullptr;

        if (state["type"] == "boolean")
        {
            builder = factory.create_bool_type();
            type = builder->build();
        }
        else if (state["type"] == "int8")
        {
            builder = factory.create_char8_type();
            type = builder->build();
        }
        else if (state["type"] == "uint8")
        {
            builder = factory.create_byte_type();
            type = builder->build();
        }
        else if (state["type"] == "int16")
        {
            builder = factory.create_int16_type();
            type = builder->build();
        }
        else if (state["type"] == "uint16")
        {
            builder = factory.create_uint16_type();
            type = builder->build();
        }
        else if (state["type"] == "int32")
        {
            builder = factory.create_int32_type();
            type = builder->build();
        }
        else if (state["type"] == "uint32")
        {
            builder = factory.create_uint32_type();
            type = builder->build();
        }
        else if (state["type"] == "int64")
        {
            builder = factory.create_int64_type();
            type = builder->build();
        }
        else if (state["type"] == "uint64")
        {
            builder = factory.create_uint64_type();
            type = builder->build();
        }
        else if (state["type"] == "float")
        {
            builder = factory.create_float32_type();
            type = builder->build();
        }
        else if (state["type"] == "double")
        {
            builder = factory.create_float64_type();
            type = builder->build();
        }
        else if (state["type"] == "long double")
        {
            builder = factory.create_float128_type();
            type = builder->build();
        }
        else if (state["type"] == "char")
        {
            builder = factory.create_char8_type();
            type = builder->build();
        }
        else if (state["type"] == "wchar" || state["type"] == "char16")
        {
            builder = factory.create_char16_type();
            type = builder->build();
        }
        else if (state["type"] == "string")
        {
            if (state.count("string_size"))
            {
                builder = factory.create_string_type(std::atoi(state["string_size"].c_str()));
                state.erase("string_size");
            }
            else
            {
                builder = factory.create_string_type();
            }
            type = builder->build();
        }
        else if (state["type"] == "wstring")
        {
            if (state.count("wstring_size"))
            {
                builder = factory.create_wstring_type(std::atoi(state["wstring_size"].c_str()));
                state.erase("wstring_size");
            }
            else
            {
                builder = factory.create_wstring_type();
            }
            type = builder->build();
        }

        return type;
    }

}; // class Parser


v1_3::DynamicType_ptr Context::get_type(
        std::map<std::string, std::string>& state)
{
    return parser_->type_spec(state);
}

} // namespace idl
} // namespace types
} // namespace fastrtps
} // namespace eprosima


#ifdef _MSVC_LANG
#   pragma pop_macro("popen")
#   pragma pop_macro("pipe")
#   pragma pop_macro("pclose")
#endif // ifdef _MSVC_LANG

#endif // TYPES_IDL_PARSER_H_
