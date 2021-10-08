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
#define BOOST_FASTDDS_PATCHES
#define BOOST_INTERPROCESS_FORCE_NATIVE_EMULATION

#ifdef _MSC_VER

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>

// TODO(Adolfo): This will fail if windows system without program data in C: drive
#define CUSTOM_SHARED_DIR_PATH "C:\\ProgramData\\eprosima\\fastrtps_interprocess"
#define CUSTOM_SHARED_DIR_PATH_W L"C:\\ProgramData\\eprosima\\fastrtps_interprocess"

#define BOOST_INTERPROCESS_SHARED_DIR_FUNC
#define BOOST_INTERPROCESS_SHARED_DIR_FUNC_IS_TEMPLATE

namespace boost {
namespace interprocess {
namespace ipcdetail {

template<class CharT>
inline void get_shared_dir(std::basic_string<CharT>& shared_dir)
{
    shared_dir = CUSTOM_SHARED_DIR_PATH;
}

template<>
inline void get_shared_dir(std::wstring& shared_dir)
{
    shared_dir = CUSTOM_SHARED_DIR_PATH_W;
}

}  // namespace ipcdetail
}  // namespace interprocess
}  // namespace boost

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
        if (stat(CUSTOM_SHARED_DIR_PATH, &info) != 0)
        {
            // Try to create it
            if (system(("mkdir " CUSTOM_SHARED_DIR_PATH)) != 0)
            {
                throw std::runtime_error("couldn't access nor create " CUSTOM_SHARED_DIR_PATH);
            }
        }
    }

    void clean()
    {
        system("del " CUSTOM_SHARED_DIR_PATH "\\*.* /Q");
    }

#endif // ifdef _MSC_VER

};

#endif // _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_
