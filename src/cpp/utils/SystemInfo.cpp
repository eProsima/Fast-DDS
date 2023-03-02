// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "SystemInfo.hpp"

#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/types.h>
#endif // _WIN32

#include <fstream>
#include <string>

#include <json.hpp>
#include <fastrtps/types/TypesBase.h>

#include <stdio.h>

namespace eprosima {

using ReturnCode_t = fastrtps::types::ReturnCode_t;

ReturnCode_t SystemInfo::get_env(
        const std::string& env_name,
        std::string& env_value)
{
    if (env_name.empty())
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    // Try to read environment variable from file
    if (!environment_file_.empty() && ReturnCode_t::RETCODE_OK == get_env(environment_file_, env_name, env_value))
    {
        return ReturnCode_t::RETCODE_OK;
    }

    char* data;
#pragma warning(suppress:4996)
    data = getenv(env_name.c_str());
    if (nullptr != data)
    {
        env_value = data;
    }
    else
    {
        return ReturnCode_t::RETCODE_NO_DATA;
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t SystemInfo::get_env(
        const std::string& filename,
        const std::string& env_name,
        std::string& env_value)
{
    // Check that the file exists
    if (!SystemInfo::file_exists(filename))
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    // Read json file
    std::ifstream file(filename);
    nlohmann::json file_content;

    try
    {
        file >> file_content;
    }
    catch (const nlohmann::json::exception&)
    {
        return ReturnCode_t::RETCODE_ERROR;
    }

    try
    {
        env_value = file_content.at(env_name);
    }
    catch (const nlohmann::json::exception&)
    {
        return ReturnCode_t::RETCODE_NO_DATA;
    }
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t SystemInfo::get_username(
        std::string& username)
{
#ifdef _WIN32
#define INFO_BUFFER_SIZE 32767
    char user[INFO_BUFFER_SIZE];
    DWORD bufCharCount = INFO_BUFFER_SIZE;
    if (!GetUserNameA(user, &bufCharCount))
    {
        return ReturnCode_t::RETCODE_ERROR;
    }
    username = user;
    return ReturnCode_t::RETCODE_OK;
#else
    uid_t user_id = geteuid();
    struct passwd* pwd = getpwuid(user_id);
    if (pwd != nullptr)
    {
        username = pwd->pw_name;
        if (!username.empty())
        {
            return ReturnCode_t::RETCODE_OK;
        }
    }
    return ReturnCode_t::RETCODE_ERROR;
#endif // _WIN32
}

bool SystemInfo::file_exists(
        const std::string& filename)
{
    struct stat s;
    // Check existence and that it is a regular file (and not a folder)
    return (stat(filename.c_str(), &s) == 0 && s.st_mode & S_IFREG);
}

ReturnCode_t SystemInfo::set_environment_file()
{
    return SystemInfo::get_env(FASTDDS_ENVIRONMENT_FILE_ENV_VAR, SystemInfo::environment_file_);
}

const std::string& SystemInfo::get_environment_file()
{
    return SystemInfo::environment_file_;
}

FileWatchHandle SystemInfo::watch_file(
        std::string filename,
        std::function<void()> callback)
{
#if defined(_WIN32) || defined(__unix__)
    return FileWatchHandle (new filewatch::FileWatch<std::string>(filename,
                   [callback](const std::string& /*path*/, const filewatch::Event change_type)
                   {
                       switch (change_type)
                       {
                           case filewatch::Event::modified:
                               callback();
                               break;
                           default:
                               // No-op
                               break;
                       }
                   }));
#else // defined(_WIN32) || defined(__unix__)
    static_cast<void>(filename);
    static_cast<void>(callback);
    return FileWatchHandle();
#endif // defined(_WIN32) || defined(__unix__)
}

void SystemInfo::stop_watching_file(
        FileWatchHandle& handle)
{
#if defined(_WIN32) || defined(__unix__)
    handle.reset();
#endif // if defined(_WIN32) || defined(__unix__)
    static_cast<void>(handle);
}

/**
 * @brief Trivial class to measure memory allocations
 */
class MemoryMonitor
{
public:
    MemoryMonitor() = default;

    /**
     * @brief Computes the amount of RSS memory allocated since the last call of this function.
     */
    uint64_t get_memory_delta()
    {
        uint64_t current_usage = get_memory_usage();
        uint64_t delta = current_usage - m_rss_kb;
        m_rss_kb = current_usage;
        return delta;
    }

    /**
     * @brief Computes the current RSS memory usage of this process.
     */
    uint64_t get_memory_usage()
    {
        // Sleep for a little bit to make sure that proc pages are updated
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Get rss
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);

        return usage.ru_maxrss;
    }

    void log_memory_delta(const char* msg, bool force)
    {
        uint64_t delta = get_memory_delta();
        if (delta > 0 || force) {
          printf("[fast-dds.mem] %s --> %lu\n", msg, delta);
        }
    }

private:
    uint64_t m_rss_kb{ 0 };
};

void log_memory_delta(
        const char* msg,
        bool force)
{
    static MemoryMonitor the_monitor;
    the_monitor.log_memory_delta(msg, force);
}

std::string SystemInfo::environment_file_;

} // eprosima
