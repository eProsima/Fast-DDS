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

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING
#define BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS 1000

#ifdef __APPLE__
#define BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION
#endif

#ifdef _MSC_VER

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>

//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_LASTBOOTUPTIME

// TODO(Adolfo): This will fail if windows system without program data in C: drive
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "C:\\ProgramData\\eprosima\\fastrtps_interprocess"

#include <boost/interprocess/detail/workaround.hpp>
#define BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION
/*#if defined(BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION)
#error "BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION must be disabled in boost/interprocess/detail/workarround.hpp"
#endif*/

// Todo(Adolfo): BlackBox.SHMTransportPubSub fail with BOOST_INTERPROCESS_USE_WINDOWS
//#define BOOST_INTERPROCESS_USE_WINDOWS
//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_SESSION_MANAGER_BASED

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
#endif
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
#endif
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
#endif

};

#endif // _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_
