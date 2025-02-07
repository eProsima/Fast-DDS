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

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <thread>
#include <time.h>

#include <nlohmann/json.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <utils/threading.hpp>

namespace eprosima {

using IPFinder = fastdds::rtps::IPFinder;

SystemInfo::SystemInfo()
{
    // From ctime(3) linux man page:
    // According to POSIX.1-2004, localtime() is required to behave as though tzset(3) was called, while
    // localtime_r() does not have this requirement. For portable code tzset(3) should be called before
    // localtime_r().
#if (_POSIX_C_SOURCE >= 1) || defined(_XOPEN_SOURCE) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || \
    defined(_POSIX_SOURCE) || defined(__unix__)
    tzset();
#endif // if (_POSIX_C_SOURCE >= 1) || defined(_XOPEN_SOURCE) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) ||
       // defined(_POSIX_SOURCE) || defined(__unix__)

    update_interfaces();
}

fastdds::dds::ReturnCode_t SystemInfo::get_env(
        const std::string& env_name,
        std::string& env_value)
{
    if (env_name.empty())
    {
        return fastdds::dds::RETCODE_BAD_PARAMETER;
    }

    // Try to read environment variable from file
    if (!environment_file_.empty() && fastdds::dds::RETCODE_OK == get_env(environment_file_, env_name, env_value))
    {
        return fastdds::dds::RETCODE_OK;
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
        return fastdds::dds::RETCODE_NO_DATA;
    }

    return fastdds::dds::RETCODE_OK;
}

fastdds::dds::ReturnCode_t SystemInfo::get_env(
        const std::string& filename,
        const std::string& env_name,
        std::string& env_value)
{
    // Check that the file exists
    if (!SystemInfo::file_exists(filename))
    {
        return fastdds::dds::RETCODE_BAD_PARAMETER;
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
        return fastdds::dds::RETCODE_ERROR;
    }

    try
    {
        env_value = file_content.at(env_name);
    }
    catch (const nlohmann::json::exception&)
    {
        return fastdds::dds::RETCODE_NO_DATA;
    }
    return fastdds::dds::RETCODE_OK;
}

fastdds::dds::ReturnCode_t SystemInfo::get_username(
        std::string& username)
{
#ifdef _WIN32
#define INFO_BUFFER_SIZE 32767
    char user[INFO_BUFFER_SIZE];
    DWORD bufCharCount = INFO_BUFFER_SIZE;
    if (!GetUserNameA(user, &bufCharCount))
    {
        return fastdds::dds::RETCODE_ERROR;
    }
    username = user;
    return fastdds::dds::RETCODE_OK;
#else
    uid_t user_id = geteuid();
    struct passwd* pwd = getpwuid(user_id);
    if (pwd != nullptr)
    {
        username = pwd->pw_name;
        if (!username.empty())
        {
            return fastdds::dds::RETCODE_OK;
        }
    }
    return fastdds::dds::RETCODE_ERROR;
#endif // _WIN32
}

bool SystemInfo::file_exists(
        const std::string& filename)
{
#ifdef _WIN32
    // modify for mingw
    DWORD fileAttributes = GetFileAttributesA(filename.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat s;
    return (stat(filename.c_str(), &s) == 0 && s.st_mode & S_IFREG);
#endif // ifdef _WIN32
}

bool SystemInfo::wait_for_file_closure(
        const std::string& filename,
        const std::chrono::seconds timeout)
{
    auto start = std::chrono::steady_clock::now();

#ifdef _MSC_VER
    std::ofstream os;
    do
    {
        // MSVC specific
        os.open(filename, std::ios::out | std::ios::app, _SH_DENYWR);
        if (!os.is_open()
                // If the file is lock-opened in an external editor do not hang
                && (std::chrono::steady_clock::now() - start) < timeout )
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
            && (std::chrono::steady_clock::now() - start) < timeout )
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

    return std::chrono::steady_clock::now() - start < timeout;
}

fastdds::dds::ReturnCode_t SystemInfo::set_environment_file()
{
    return SystemInfo::get_env(FASTDDS_ENVIRONMENT_FILE_ENV_VAR, SystemInfo::environment_file_);
}

const std::string& SystemInfo::get_environment_file()
{
    return SystemInfo::environment_file_;
}

FileWatchHandle SystemInfo::watch_file(
        std::string filename,
        std::function<void()> callback,
        const fastdds::rtps::ThreadSettings& watch_thread_config,
        const fastdds::rtps::ThreadSettings& callback_thread_config)
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
                   }, watch_thread_config, callback_thread_config));
#else // defined(_WIN32) || defined(__unix__)
    static_cast<void>(filename);
    static_cast<void>(callback);
    static_cast<void>(watch_thread_config);
    static_cast<void>(callback_thread_config);
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

std::string SystemInfo::get_timestamp(
        const char* format)
{
    std::stringstream stream;
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
    auto ms = static_cast<unsigned>(tp / std::chrono::milliseconds(1));

#if defined(_WIN32)
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_c);
    //#elif defined(__clang__) && !defined(std::put_time) // TODO arm64 doesn't seem to support std::put_time
    //    (void)now_c;
    //    (void)ms;
#elif (_POSIX_C_SOURCE >= 1) || defined(_XOPEN_SOURCE) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || \
    defined(_POSIX_SOURCE) || defined(__unix__)
    std::tm timeinfo;
    localtime_r(&now_c, &timeinfo);
#else
    std::tm timeinfo = *localtime(&now_c);
#endif // if defined(_WIN32)
    stream << std::put_time(&timeinfo, format) << "." << std::setw(3) << std::setfill('0') << ms;
    return stream.str();
}

bool SystemInfo::update_interfaces()
{
    std::vector<IPFinder::info_IP> ifaces;
    auto ret = IPFinder::getIPs(&ifaces, true);
    if (ret)
    {
        std::lock_guard<std::mutex> lock(interfaces_mtx_);
        // Copy fetched interfaces to attribute
        interfaces_ = ifaces;
        // Set to true when successful, but not to false if lookup failed (may have been successfully cached before)
        cached_interfaces_ = true;
    }
    return ret;
}

bool SystemInfo::get_ips(
        std::vector<IPFinder::info_IP>& vec_name,
        bool return_loopback,
        bool force_lookup)
{
    if (force_lookup)
    {
        return IPFinder::getIPs(&vec_name, return_loopback);
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(interfaces_mtx_);
            if (cached_interfaces_)
            {
                for (const auto& iface : interfaces_)
                {
                    if (return_loopback || (iface.type != IPFinder::IPTYPE::IP4_LOCAL &&
                            iface.type != IPFinder::IPTYPE::IP6_LOCAL))
                    {
                        vec_name.push_back(iface);
                    }
                }
                return true;
            }
        }
        // Interfaces not cached, perform lookup
        return IPFinder::getIPs(&vec_name, return_loopback);
    }
}

std::string SystemInfo::environment_file_;
bool SystemInfo::cached_interfaces_;
std::vector<IPFinder::info_IP> SystemInfo::interfaces_;
std::mutex SystemInfo::interfaces_mtx_;

} // eprosima

// threading.hpp implementations
#ifdef _WIN32
#include "threading/threading_win32.ipp"
#include "thread_impl/thread_impl_win32.ipp"
#elif defined(__APPLE__)
#include "threading/threading_osx.ipp"
#include "thread_impl/thread_impl_pthread.ipp"
#elif defined(_POSIX_SOURCE) || defined(__QNXNTO__) || defined(__ANDROID__)
#include "threading/threading_pthread.ipp"
#include "thread_impl/thread_impl_pthread.ipp"
#else
#include "threading/threading_empty.ipp"
#endif // Platform selection
