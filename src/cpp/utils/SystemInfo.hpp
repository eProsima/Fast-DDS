// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UTILS_SYSTEMINFO_HPP_
#define UTILS_SYSTEMINFO_HPP_

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif // if defined(_WIN32)

#include <cstdint>
#include <memory>
#include <string>

#include <fastrtps/types/TypesBase.h>
#include <utils/Host.hpp>

#if defined(_WIN32) || defined(__unix__)
#include <FileWatch.hpp>
#endif // defined(_WIN32) || defined(__unix__)

namespace eprosima {

using ReturnCode_t = fastrtps::types::ReturnCode_t;
#if defined(_WIN32) || defined(__unix__)
using FileWatchHandle = std::unique_ptr<filewatch::FileWatch<std::string>>;
#else
using FileWatchHandle = void*;
#endif // defined(_WIN32) || defined(__unix__)

/**
 * This singleton serves as a centralized point from where to obtain platform dependent system information.
 */
class SystemInfo
{
public:

    /**
     * Get the identifier of the current process.
     *
     * @return the identifier of the current process.
     */
    inline int process_id() const
    {
#if defined(__cplusplus_winrt)
        return (int)GetCurrentProcessId();
#elif defined(_WIN32)
        return (int)_getpid();
#else
        return (int)getpid();
#endif // platform selection
    }

    /**
     * Get the identifier of the current host.
     *
     * @return the identifier of the current host.
     */
    inline uint16_t host_id() const
    {
        return Host::instance().id();
    }

    /**
     * Get a reference to the singleton instance.
     *
     * @return reference to the singleton instance.
     */
    static const SystemInfo& instance()
    {
        static SystemInfo singleton;
        return singleton;
    }

    /**
     * Retrieve the value of a given environment variable if it exists.
     * The string which is returned in the env_value output parameter is not modified
     * if the environment variable is not set.
     *
     * This function is thread-safe as long as no other function modifies the host environment (in particular, POSIX
     * functions setenv, unsetenv and putenv would introduce a data race if called without synchronization.)
     *
     * \param [in] env_name name of the environment variable.
     * \param [out] env_value value of the environment variable.
     * @return RETCODE_OK if the environment variable is set.
     * RETCODE_NO_DATA if the environment variable is unset.
     * RETCODE_BAD_PARAMETER if the provided parameters are not valid.
     */
    static ReturnCode_t get_env(
            const std::string& env_name,
            std::string& env_value);

    /**
     * Read environment variable contained in the environment file.
     *
     * @param [in] filename path/name of the environment file.
     * @param [in] env_name environment variable name to read from the file.
     * @param [out] env_value environment variable value read from the file.
     *
     * @return RETCODE_OK if succesful.
     * RETCODE_BAD_PARAMETER if the file does not exist.
     * RETCODE_NO_DATA if the file exists but there is no information about the environment variable.
     * RETCODE_ERROR if the file is empty or malformed.
     */
    static ReturnCode_t get_env(
            const std::string& filename,
            const std::string& env_name,
            std::string& env_value);

    /**
     * Get the effective username of the person that launched the application
     * This implementation follows the whoami implementation for POSIX
     * (https://github.com/coreutils/coreutils/blob/master/src/whoami.c)
     *
     * geteuid is always successful.
     *
     * getpwuid returns a pointer to a struct passwd if a matching entry is found and nullptr otherwise.
     * This pointer is only valid until the next time getpwuid function is called, because it is a direct
     * pointer to the static storage.
     * Modifying the pointer invokes undefined behavior.
     *
     * This function is thread-safe as long as no other function modifies the storage areas pointed within the structure
     * (in particular, POSIX functions getpwent or getpwnam might overwrite some of the storage areas.)
     *
     * \param [out] username string with the effective username.
     * @return RETCODE_OK if successful.
     * RETCODE_ERROR if the username information cannot be determined.
     */
    static ReturnCode_t get_username(
            std::string& username);

    /**
     * Check if the file wiht name \c filename exists.
     * \c filename can also include the path to the file.
     *
     * \param [in] filename path/name of the file to check.
     * @return True if the file exists. False otherwise.
     */
    static bool file_exists(
            const std::string& filename);

    /**
     * Read FASTDDS_ENVIRONMENT_FILE_ENV_VAR environment value and save its value.
     * Use get_environment_file to read its value.
     *
     * @return RETCODE_OK if the environment variable is set.
     * RETCODE_NO_DATA if the environment variable is unset.
     */
    static ReturnCode_t set_environment_file();

    /**
     * Getter for the path/filename contained in the FASTDDS_ENVIRONMENT_FILE_ENV_VAR.
     * The value is set calling set_environment_file().
     *
     * @return Path/filename contained in the environment variable.
     */
    static const std::string& get_environment_file();

    /**
     * Start a thread that watches for changes in the given file and executes a callback when the file changes.
     *
     * The method returns a handle to the object that implements the watcher containing the thread.
     * The scope of the listening thread is the same as the watching object, so when the object
     * goes out of scope, the thread is terminated.
     *
     * The thread can be terminated earlier with stop_watching_file(FileWatchHandle)
     *
     * @param [in] filename Path/name of the file to watch.
     * @param [in] callback Callback to execute when the file changes.
     *
     * @return The handle that represents the watcher object.
     */
    static FileWatchHandle watch_file(
            std::string filename,
            std::function<void()> callback);

    /**
     * Stop a file watcher.
     *
     * This method effectively destroys the file watcher and the thread that were created with watch_file.
     * Once this method returns, the handle is no longer valid.
     *
     * @param [in] handle The watcher handle as returned by watch_file.
     */
    static void stop_watching_file(
            FileWatchHandle& handle);

private:

    SystemInfo() = default;

    static std::string environment_file_;

};

/**
 * Environment variable to specify the name of a file (including or not the path) where the environment variables
 * could be defined.
 * Thus, the user can modify the environment variables' values in runtime.
 *
 * TODO(jlbueno) Currently only ROS_DISCOVERY_SERVER environment variable is supported.
 */
const char* const FASTDDS_ENVIRONMENT_FILE_ENV_VAR = "FASTDDS_ENVIRONMENT_FILE";

} // namespace eprosima

#endif // UTILS_SYSTEMINFO_HPP_
