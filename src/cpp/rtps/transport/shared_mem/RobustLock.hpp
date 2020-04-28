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

#ifndef _FASTDDS_ROBUST_LOCK_H_
#define _FASTDDS_ROBUST_LOCK_H_

#include <boostconfig.hpp>
#include <boost/interprocess/detail/shared_dir_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This class implement helpers used in RobustSharedLock & RobustExclusiveLock
 */
class RobustLock
{
public:

#if !defined(BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS)

    static std::string get_file_path(
            const std::string& file_name)
    {
        std::string shmfile;
        boost::interprocess::ipcdetail::shared_filepath(file_name.c_str(), shmfile);
        return shmfile;
    }

#else
    static std::string get_file_path(
            const std::string& filename)
    {
        static const char defaultdir[] = "/dev/shm/";

        std::string filepath;
        #if defined(BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SHARED_MEMORY)
        const bool add_leading_slash = false;
        #elif defined(BOOST_INTERPROCESS_RUNTIME_FILESYSTEM_BASED_POSIX_SHARED_MEMORY)
        const bool add_leading_slash = !boost::interprocess::shared_memory_object_detail::use_filesystem_based_posix();
        #else
        const bool add_leading_slash = true;
        #endif
        if (add_leading_slash)
        {
            boost::interprocess::ipcdetail::add_leading_slash(filename.c_str(), filepath);
        }
        else
        {
            boost::interprocess::ipcdetail::shared_filepath(filename.c_str(), filepath);
        }

        return defaultdir + filepath;
    }

#endif
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ROBUST_LOCK_H_
