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

#include <fastrtps/types/TypesBase.h>
#include <utils/Host.hpp>

namespace eprosima {

using ReturnCode_t = fastrtps::types::ReturnCode_t;

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
     * Retrieve the value of a given environment variable if it exists, or nullptr.
     * The c-string which is returned in the env_value output parameter is only valid until the next time this function
     * is called, because it is a direct pointer to the static storage.
     * Modifying the string returned in env_value invokes undefined behavior.
     * If the environment variable is not set, a nullptr will be returned.
     *
     * This function is thread-safe as long as no other function modifies the host environment (in particular, POSIX
     * functions setenv, unsetenv and putenv would introduce a data race if called without synchronization.)
     *
     * \param [in] env_name the name of the environment variable
     * \param [out] env_value pointer to the value c-string
     * @return RETCODE_OK if the environment variable is set.
     * RETCODE_NO_DATA if the environment variable is unset.
     * RETCODE_BAD_PARAMETER if the provided parameters are not valid.
     */
    static ReturnCode_t get_env(
            const char* env_name,
            const char** env_value);

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
     * Read environment vairable contained in the environment file.
     * 
     * @param [in] filename path/name of the environment file.
     * @param [in] env_name environment variable name to read from the file.
     * @param [out] env_value environment variable value read from the file.
     * 
     * @return RETCODE_OK if succesful.
     * RETCODE_BAD_PARAMETER if the file does not exist.
     * RETCODE_NO_DATA if the file exists but there is no information about the environment variable
     */
    static ReturnCode_t load_environment_file(
            const std::string& filename,
            const std::string& env_name,
            std::string& env_value);

private:

    SystemInfo() = default;

};

} // namespace eprosima

#endif // UTILS_SYSTEMINFO_HPP_
