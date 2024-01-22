// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_
#define _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_

#include <utils/shared_memory/BoostAtExitRegistry.hpp>

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING
#define BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS 1000

// We have patched part of the boost code, protecting all changes with this define
#define BOOST_FASTDDS_PATCHES

// Starting on boost 1.76.0, this will force native emulation, which is what we want as
// it is more performant
#define BOOST_INTERPROCESS_FORCE_NATIVE_EMULATION

#ifdef ANDROID
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "/data/local/tmp"
#endif

#define BOOST_INTERPROCESS_ATEXIT(f) eprosima::detail::BoostAtExitRegistry::get_instance()->at_exit_register((f))

#ifdef _MSC_VER

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>

// TODO(Adolfo): This will fail if windows system without program data in C: drive
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "C:\\ProgramData\\eprosima\\fastdds_interprocess"

// Check that generic emulation has not been accidentally enabled
#include <boost/interprocess/detail/workaround.hpp>
#if defined(BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION)
#  error "BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION must be disabled in boost/interprocess/detail/workarround.hpp"
#endif  // BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION

#endif // _MSC_VER_

/**
 * This singleton class performs some necesary system dependent initializations
 * before start working with shared-memory
 */
class SharedMemEnvironment
{
public:

    SharedMemEnvironment()
#ifdef _MSC_VER
        : is_init_done_(false)
#endif // ifdef _MSC_VER
    {
    }

    /**
     * @return the singleton instance
     */
    static SharedMemEnvironment& get()
    {
        static SharedMemEnvironment singleton_instance;
        return singleton_instance;
    }

    /**
     * Perform the initializacion, only the first time is called.
     */
    void init()
    {
#ifdef _MSC_VER
        if (!is_init_done_)
        {
            get().create_shared_dir_if_doesnt_exist();

        }

        is_init_done_ = true;
#endif // ifdef _MSC_VER
    }

private:

#ifdef _MSC_VER

    bool is_init_done_;


    static void create_shared_dir_if_doesnt_exist()
    {
        struct stat info;

        // Cannot access shared_dir_path, we assume it doesn't exist
        if (stat(BOOST_INTERPROCESS_SHARED_DIR_PATH, &info) != 0)
        {
            // Try to create it
            if (system(("mkdir " BOOST_INTERPROCESS_SHARED_DIR_PATH)) != 0)
            {
                throw std::runtime_error("couldn't access nor create " BOOST_INTERPROCESS_SHARED_DIR_PATH);
            }
        }
    }

    void clean()
    {
        system("del " BOOST_INTERPROCESS_SHARED_DIR_PATH "\\*.* /Q");
    }

#endif // ifdef _MSC_VER

};

#endif // _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_
