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

#ifndef _FASTDDS_SHARED_DIR_H_
#define _FASTDDS_SHARED_DIR_H_

#include <boostconfig.hpp>
#include <boost/interprocess/detail/shared_dir_helpers.hpp>

#ifdef __QNXNTO__
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#endif  // __QNXNTO__

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This class implement helpers used for Shared Memory file support
 */
class SharedDir
{
public:

    static void get_default_shared_dir (
            std::string& shared_dir)
    {

#if !defined(BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS)
        boost::interprocess::ipcdetail::get_shared_dir(shared_dir);

#else
#ifdef __QNXNTO__
        static const char defaultdir[] = "/dev/shmem";
#else
        // Default value from: glibc-2.29/sysdeps/unix/sysv/linux/shm-directory.c
        static const char defaultdir[] = "/dev/shm";
#endif // __QNXNTO__

        std::string filepath;
        #if defined(BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SHARED_MEMORY)
        const bool add_leading_slash = false;
        #elif defined(BOOST_INTERPROCESS_RUNTIME_FILESYSTEM_BASED_POSIX_SHARED_MEMORY)
        const bool add_leading_slash = !boost::interprocess::shared_memory_object_detail::use_filesystem_based_posix();
        #else
        const bool add_leading_slash = true;
        #endif // if defined(BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SHARED_MEMORY)
        if (add_leading_slash)
        {
            shared_dir = defaultdir;
        }
        else
        {
            boost::interprocess::ipcdetail::get_shared_dir(shared_dir);
        }
#endif // if !defined(BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS)
    }

    static std::string get_file_path(
            const std::string& filename)
    {
        std::string path;
        get_default_shared_dir(path);
        return path + "/" + filename;
    }

    static std::string get_lock_path(
            const std::string& filename)
    {
#ifdef __QNXNTO__
        static const char defaultdir[] = "/var/lock";
        struct stat buf;
        // check directory status
        if (stat(defaultdir, &buf) != 0)
        {
            // directory not found, create it
            if (errno == ENOENT)
            {
                mkdir(defaultdir, 0777);
            }
            // if another error then throw exception
            else
            {
                std::string err("get_file_path() ");
                err = err + strerror(errno);
                throw std::runtime_error(err);
            }
        }
        else
        {
            // directory exists do nothing
        }
        return (std::string(defaultdir) + "/" + filename);
#else
        return SharedDir::get_file_path(filename);
#endif  // __QNXNTO__
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHARED_DIR_H_
