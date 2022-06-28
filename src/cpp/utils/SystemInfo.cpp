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

#ifdef __unix__
#   include <sys/file.h>
#   include <unistd.h>
#endif // ifdef __unix__

#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#endif // _WIN32

#include <fstream>
#include <string>

#include <json.hpp>
#include <fastrtps/types/TypesBase.h>

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

bool SystemInfo::wait_for_file_closure(
        const std::string& filename,
        const std::chrono::seconds timeout)
{
    auto start = std::chrono::system_clock::now();

#ifdef _MSC_VER
    std::ofstream os;
    do
    {
        // MSVC specific
        os.open(filename, std::ios::out | std::ios::app, _SH_DENYWR);
        if (!os.is_open()
                // If the file is lock-opened in an external editor do not hang
                && (std::chrono::system_clock::now() - start) < timeout )
        {
            std::this_thread::yield();
        }
        else
        {
            break;
        }
    }
    while (true);
#elif __unix__
    int fd = open(filename.c_str(), O_WRONLY);

    while (flock(fd, LOCK_EX | LOCK_NB)
            // If the file is lock-opened in an external editor do not hang
            && (std::chrono::system_clock::now() - start) < timeout )
    {
        std::this_thread::yield();
    }

    flock(fd, LOCK_UN | LOCK_NB);
    close(fd);
#else
    // plain wait
    std::this_thread::sleep_for(timeout);
    // avoid unused warning
    (void)start;
    (void)filename;
#endif // ifdef _MSC_VER

    return std::chrono::system_clock::now() - start < timeout;
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

std::string SystemInfo::environment_file_;

} // eprosima
