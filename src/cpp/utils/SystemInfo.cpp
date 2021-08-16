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
#endif // _WIN32

#include <fastrtps/types/TypesBase.h>

namespace eprosima {

using ReturnCode_t = fastrtps::types::ReturnCode_t;

ReturnCode_t SystemInfo::get_env(
        const char* env_name,
        const char** env_value)
{
    if (env_name == nullptr || env_value == nullptr || *env_name == '\0')
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

#pragma warning(suppress:4996)
    *env_value = getenv(env_name);
    if (*env_value == nullptr)
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
    if (!GetUserName(user, &bufCharCount))
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

} // eprosima
