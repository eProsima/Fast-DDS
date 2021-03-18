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

private:

    SystemInfo() = default;

};

} // namespace eprosima

#endif // UTILS_SYSTEMINFO_HPP_
