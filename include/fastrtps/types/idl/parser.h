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

#include "grammar.h"
#include "xtypes_assert.h"

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

// mimic posix pipe APIs
#ifdef _MSC_VER
#   pragma push_macro("popen")
#   define popen _popen
#   pragma push_macro("pipe")
#   define pipe _pipe
#   pragma push_macro("pclose")
#   define pclose _pclose
#endif

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
#endif

//namespace peg {
//
//using Ast = AstBase<EmptyType>;
//
//} // namespace peg

namespace eprosima {
namespace fastrtps {
namespace types {
namespace idl {

namespace log {

enum LogLevel
{
    xERROR,
    xWARNING,
    xINFO,
    xDEBUG
};

struct LogEntry
{
    std::string path;
    size_t line;
    size_t column;
    LogLevel level;
    std::string category;
    std::string message;

    LogEntry(
            const std::string& file,
            size_t line_number,
            size_t column_number,
            LogLevel log_level,
            const std::string& cat,
            const std::string& msg)
        : path(file)
        , line(line_number)
        , column(column_number)
        , level(log_level)
        , category(cat)
        , message(msg)
    {
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[";
        switch (level)
        {
            case xERROR:
                ss << "ERROR";
                break;
            case xWARNING:
                ss << "WARNING";
                break;
            case xINFO:
                ss << "INFO";
                break;
            case xDEBUG:
                ss << "DEBUG";
                break;
        }
        ss << "] ";
        ss << category << ": ";
        ss << message;
        // TODO - path, line and column may be confusing to the user, because if preprocessed
        // they may change.
        // << "(";
        //if (!path.empty())
        //{
        //    ss << path << ":";
        //}
        //ss << line << ":" << column << ")";
        return ss.str();
    }

};

} // namespace log

class LogContext
{
    mutable std::vector<log::LogEntry> log_;
    log::LogLevel log_level_ = log::LogLevel::xDEBUG;
    bool print_log_ = true;

public:

    const std::vector<log::LogEntry>& log() const
    {
        return log_;
    }

    std::vector<log::LogEntry> log(
            log::LogLevel level,
            bool strict = false) const
    {
        std::vector<log::LogEntry> result;
        for (const log::LogEntry& entry : log_)
        {
            if (entry.level == level || (!strict && entry.level < level))
            {
                result.push_back(entry);
            }
        }
        return result;
    }

    void log_level(
            log::LogLevel level)
    {
        log_level_ = level;
    }

    log::LogLevel log_level() const
    {
        return log_level_;
    }

    void print_log(
            bool enable)
    {
        print_log_ = enable;
    }

    //// Logging
    //void log(
    //        log::LogLevel level,
    //        const std::string& category,
    //        const std::string& message,
    //        std::shared_ptr<peg::Ast> ast = nullptr) const
    //{
    //    if (log_level_ >= level)
    //    {
    //        if (ast != nullptr)
    //        {
    //            log_.emplace_back(ast->path, ast->line, ast->column, level, category, message);
    //        }
    //        else
    //        {
    //            log_.emplace_back("", 0, 0, level, category, message);
    //        }
    //        if (print_log_)
    //        {
    //            const log::LogEntry& entry = log_.back();
    //            std::cout << entry.to_string() << std::endl;
    //        }
    //    }
    //}
    void log(
            log::LogLevel level,
            const std::string& category,
            const std::string& message) const
    {
        if (log_level_ >= level)
        {
            log_.emplace_back("", 0, 0, level, category, message);
            if (print_log_)
            {
                const log::LogEntry& entry = log_.back();
                std::cout << entry.to_string() << std::endl;
            }
        }
    }
};

class PreprocessorContext
    : public LogContext
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

        log(log::LogLevel::xDEBUG, "PREPROCESS",
                "Calling preprocessor with command: " + cmd);
        std::string output = exec(cmd);
        log(log::LogLevel::xDEBUG, "PREPROCESS", "Pre-processed IDL: " + output);
        return output;
    }

    std::string preprocess_string(const std::string& idl_string) const;

private:

    template<preprocess_strategy e>
    std::string preprocess_string(const std::string& idl_string) const;

#ifdef _MSC_VER
    std::pair<std::ofstream, std::string> get_temporary_file() const
    {
        // Create temporary filename
        char filename_buffer[L_tmpnam];
        auto res = std::tmpnam(filename_buffer);
        xtypes_assert(res, "Unable to create a temporary file", true);

        std::string tmp(filename_buffer);
        std::ofstream tmp_file(tmp);
        xtypes_assert(tmp_file, "Unable to create a temporary file", true);

        return std::make_pair(std::move(tmp_file), std::move(tmp));
    }
#else
    std::pair<std::ofstream, std::filesystem::path> get_temporary_file() const
    {
        // Create temporary filename
        static const std::filesystem::path tmpdir = std::filesystem::temp_directory_path();
        std::string tmp = (tmpdir / "xtypes_XXXXXX").string();
        int fd = mkstemp(tmp.data());
        xtypes_assert(fd != -1, "Unable to create a temporary file", true);

        std::ofstream tmp_file(tmp, std::ios_base::trunc | std::ios_base::out);
        close(fd);
        xtypes_assert(tmp_file, "Unable to create a temporary file", true);

        return std::make_pair(std::move(tmp_file), std::move(tmp));
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
};

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

    log(log::LogLevel::xDEBUG, "PREPROCESS",
            "Calling preprocessor '" + preprocessor_exec + "' for an IDL string.");

    return exec(cmd);
}

// preprocessing using files
template<>
inline std::string PreprocessorContext::preprocess_string<PreprocessorContext::preprocess_strategy::temporary_file>(
            const std::string& idl_string) const
{
    auto os_tmp = get_temporary_file();

    // Populate
    os_tmp.first << idl_string;
    os_tmp.first.close();

    auto processed = preprocess_file(os_tmp.second);

    return processed;
}

inline std::string PreprocessorContext::preprocess_string(
        const std::string& idl_string) const
{
    switch(strategy)
    {
        case preprocess_strategy::pipe_stdin:
            return PreprocessorContext::preprocess_string<preprocess_strategy::pipe_stdin>(idl_string);
        case preprocess_strategy::temporary_file:
            return PreprocessorContext::preprocess_string<preprocess_strategy::temporary_file>(idl_string);
    }

    xtypes_assert(true, "unknown preprocessor strategy selected.", true);

    unreachable();
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

    static const Context& DEFAULT_CONTEXT()
    {
        static const Context context;
        return context;
    }

    // Results
    bool success = false;

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

    //idl::Module& module()
    //{
    //    if(!module_)
    //    {
    //        module_ = std::make_shared<idl::Module>();
    //    }
    //    return *module_;
    //}

    inline void clear_context();

    ~Context()
    {
        clear_context();
    }

private:

    friend class Parser;
    //std::shared_ptr<idl::Module> module_;
};


class Parser
    : public std::enable_shared_from_this<Parser>
{
public:

    static Parser& instance()
    {
        static Parser instance_;
        return instance_;
    }

    //Parser()
    //    : parser_(idl_grammar())
    //    , context_(nullptr)
    //{
    //    parser_.enable_ast();
    //    parser_.set_logger(
    //        std::function<void(size_t line, size_t col, const std::string & msg)>(
    //            std::bind(
    //                &Parser::parser_log_cb_,
    //                this,
    //                std::placeholders::_1,
    //                std::placeholders::_2,
    //                std::placeholders::_3)));
    //}

    Context parse(
        const std::string& idl_string)
    {
        Context context = Context::DEFAULT_CONTEXT();
        parse(idl_string, context);
        return context;
    }

    bool parse(
        const std::string& idl_string,
        Context& context)
    {
        context_ = &context;
        //std::shared_ptr<peg::Ast> ast;

        //if (!parser_.parse(idl_string.c_str(), ast))
        //{
        //    context_->success = false;
        //    context_->log(log::LogLevel::xDEBUG, "RESULT",
        //        "The parser found errors while parsing.");
        //    return false;
        //}

        //ast = parser_.optimize_ast(ast);
        //build_on_ast(ast);
        //context_->success = true;
        //context_->log(log::LogLevel::xDEBUG, "RESULT",
        //    "The parser finished.");
        return true;
    }

    Context parse_file(
        const std::string& idl_file)
    {
        Context context = Context::DEFAULT_CONTEXT();
        parse_file(idl_file, context);
        return context;
    }

    Context parse_string(
        const std::string& idl_string)
    {
        Context context = Context::DEFAULT_CONTEXT();
        parse_string(idl_string, context);
        return context;
    }

    bool parse_file(
        const std::string& idl_file,
        Context& context)
    {
        context_ = &context;
        //std::shared_ptr<peg::Ast> ast;
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

    //class exception : public std::runtime_error
    //{
    //private:

    //    std::string message_;
    //    std::shared_ptr<peg::Ast> ast_;

    //public:

    //    exception(
    //        const std::string& message,
    //        const std::shared_ptr<peg::Ast> ast)
    //        : std::runtime_error(
    //            std::string("Parser exception (" + (ast->path.empty() ? "<no file>" : ast->path)
    //                + ":" + std::to_string(ast->line)
    //                + ":" + std::to_string(ast->column) + "): " + message))
    //        , message_(message)
    //        , ast_(ast)
    //    {
    //    }

    //    const std::string& message() const
    //    {
    //        return message_;
    //    }

    //    const peg::Ast& ast() const
    //    {
    //        return *ast_;
    //    }
    //};

    static std::string preprocess(
        const std::string& idl_file,
        const std::vector<std::string>& includes)
    {
        Context ctx = Context::DEFAULT_CONTEXT();
        ctx.include_paths = includes;
        return ctx.preprocess_file(idl_file);
    }

private:

    //friend struct Context; // TODO is this needed?
    //using LabelsCaseMemberPair = std::pair<std::vector<std::string>, Member>;

    //peg::parser parser_;
    Context* context_;

    //void parser_log_cb_(
    //    size_t l,
    //    size_t c,
    //    const std::string& msg
    //) const
    //{
    //    context_->log(log::xDEBUG, "PEGLIB_PARSER", msg + " (" + std::to_string(
    //        l - CPP_PEGLIB_LINE_COUNT_ERROR) + ":" + std::to_string(c) + ")");
    //}

    Parser() : context_(nullptr) {}
    ~Parser() {}
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
};


void Context::clear_context()
{
    if (clear)
    {
        //module_.reset();
    }
}

} // namespace idl
} // namespace types
} // namespace fastrtps
} // namespace eprosima


#ifdef _MSVC_LANG
#   pragma pop_macro("popen")
#   pragma pop_macro("pipe")
#   pragma pop_macro("pclose")
#endif

#endif // TYPES_IDL_PARSER_H_
