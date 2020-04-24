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

#ifndef _FASTDDS_ROBUST_SHARED_LOCK_H_
#define _FASTDDS_ROBUST_SHARED_LOCK_H_

#ifdef  _MSC_VER
#include <io.h>
#else
#include <sys/file.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boostconfig.hpp>
#include <boost/interprocess/detail/shared_dir_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RobustSharedLock
{
public:

    RobustSharedLock(
            const std::string& name)
    {
        auto file_path = get_file_path(name);

        fd_ = open_and_lock_file(file_path);

        name_ = name;
    }

    ~RobustSharedLock()
    {
        unlock_and_close();
    }

private:

    std::string name_;
    int fd_;

#if !defined(BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS)

    std::string get_file_path(  
            const std::string& file_name)
    {
        std::string shmfile;
        boost::interprocess::ipcdetail::shared_filepath(file_name.c_str(), shmfile);
        return shmfile;
    }

#else

    std::string get_file_path(
            const std::string& filename)
    {
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

        return "/dev/shm" + filepath;
    }

#endif

#ifdef _MSC_VER

    int open_and_lock_file(const std::string& file_path)
    {
        int fd;
        auto ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | O_RDONLY, _SH_DENYRW, _S_IREAD | _S_IWRITE);

        if (ret != 0)
        {
            char errmsg[1024];
            strerror_s(errmsg, sizeof(errmsg), errno);
            throw std::runtime_error("failed to open/create " + file_path + " " + std::string(errmsg));
        }

        return fd;
    }

    void unlock_and_close()
    {
        _close(fd_);

        if(0 != std::remove(get_file_path(name_).c_str()))
        {
            logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << get_file_path(name_));
        }
    }

#else

    int open_and_lock_file(const std::string& file_path)
    {
        auto fd = open(file_path.c_str(), O_CREAT | O_RDONLY, 0444);

        if (fd == -1)
        {
            throw std::runtime_error(("failed to open/create " + file_path + " " + std::strerror(errno)).c_str());
        }

        // Lock the file other processes
        if (0 != flock(fd, LOCK_EX | LOCK_NB))
        {
            close(fd_);
            throw std::runtime_error(("failed to lock " + file_path).c_str());
        }

        return fd;
    }

    void unlock_and_close()
    {
        flock(fd_, LOCK_UN | LOCK_NB);
        close(fd_);

        if(0 != std::remove(get_file_path(name_).c_str()))
        {
            logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << get_file_path(name_));
        }
    }

#endif

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ROBUST_SHARED_LOCK_H_
