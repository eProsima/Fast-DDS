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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPREPROCESSOR_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPREPROCESSOR_HPP

#include <array>
#include <exception>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#   include <cstdio>
#else
#   include <fcntl.h>
#   include <unistd.h>
#endif //_MSC_VER

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

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
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cl"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_FLAGS " /EP /I."
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::temporary_file
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "/I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>nul"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "rt"
#else
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cpp"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_FLAGS " -H"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::pipe_stdin
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "-I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>/dev/null"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "r"
#endif // ifdef _MSC_VER

class PreprocessorContext
{
public:

    // Preprocessors capability to use shared memory (pipes) or stick to file input
    enum class preprocess_strategy
    {
        pipe_stdin,
        temporary_file
    };

    // The preprocess flag below is disabled by default. It should only be enabled when
    // there are include paths to be preprocessed.
    bool preprocess = false;
    std::string preprocessor_exec = EPROSIMA_PLATFORM_PREPROCESSOR;
    std::string preprocessor_flags = EPROSIMA_PLATFORM_PREPROCESSOR_FLAGS;
    std::string error_redir = EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR;
    preprocess_strategy strategy = EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY;
    std::string include_flag = EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES;
    std::vector<std::string> include_paths;

    std::string preprocess_file(
            const std::string& idl_file) const
    {
        std::string args;
        for (const std::string& inc_path : include_paths)
        {
            args += include_flag + inc_path + " ";
        }

        std::string cmd = preprocessor_exec + preprocessor_flags + " " + args + idl_file + error_redir;

        EPROSIMA_LOG_INFO(IDLPARSER, "Calling preprocessor with command: " << cmd);
        std::string output = exec(cmd);
        EPROSIMA_LOG_INFO(IDLPARSER, "Preprocessed IDL: " << output);
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
        // Create a temporary filename template
        const char* literal = "/tmp/xtypes_XXXXXX";
        std::vector<char> filename_template(literal, literal + 18);

        // Create and open a temporary file securely
        int fd = mkstemp(filename_template.data());
        if (fd == -1)
        {
            throw std::runtime_error("Failed to create a temporary file: " + std::string(strerror(errno)));
        }

        // Close the file descriptor since we are going to handle file operations with fstream
        close(fd);

        // Convert file descriptor to ofstream
        std::ofstream tmp_file(filename_template.data(), std::ios::out | std::ios::in | std::ios::trunc);
        if (!tmp_file.is_open())
        {
            // If we fail to open the ofstream, we need to remove the file created by mkstemp
            unlink(filename_template.data());
            throw std::runtime_error("Failed to open the temporary file.");
        }

        // Return the file stream and the file name
        return std::make_pair(std::move(tmp_file), std::string(filename_template.begin(), filename_template.end() - 1));
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
        auto deleter = [](FILE* f)
                {
                    pclose(f);
                };
        std::unique_ptr<FILE, decltype(deleter)> pipe(
            popen(cmd.c_str(), EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS), deleter);
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
    auto os_tmp = get_temporary_file();

    // Populate
    os_tmp.first << idl_string;
    os_tmp.first.close();

    return preprocess_file(os_tmp.second);
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

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#ifdef _MSC_VER
#   pragma pop_macro("popen")
#   pragma pop_macro("pipe")
#   pragma pop_macro("pclose")
#endif // ifdef _MSC_VER

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPREPROCESSOR_HPP
