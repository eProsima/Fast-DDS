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

#ifndef _FASTDDS_ROBUST_EXCLUSIVE_LOCK_H_
#define _FASTDDS_ROBUST_EXCLUSIVE_LOCK_H_

#ifdef  _MSC_VER
#include <io.h>
#else
#include <sys/file.h>
#endif // ifdef  _MSC_VER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "RobustLock.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This class implement an interprocess named resource that holds a mutual exclusion lock until
 * destroyed, or until the creator process dies
 */
class RobustExclusiveLock
{
public:

    /**
     * Open or create and acquire the interprocess lock.
     * @param in name Is the object's interprocess global name, visible for all processes in the same machine.
     * @param out was_lock_created If the lock succeeded, this parameter return whether the lock has been created
     * or it already exist.
     * @throw std::exception if lock coulnd't be acquired
     */
    RobustExclusiveLock(
            const std::string& name,
            bool* was_lock_created)
    {
        auto file_path = RobustLock::get_file_path(name);

        fd_ = open_and_lock_file(file_path, was_lock_created);

        name_ = name;
    }

    /**
     * Open or create and acquire the interprocess lock.
     * @param in name Is the object interprocess global name, visible for all processes in the same machine.
     * @throw std::exception if lock coulnd't be acquired
     */
    RobustExclusiveLock(
            const std::string& name)
    {
        bool was_lock_created;

        auto file_path = RobustLock::get_file_path(name);

        fd_ = open_and_lock_file(file_path, &was_lock_created);

        name_ = name;
    }

    /**
     * Checks whether the file is locked.
     * @param in name Is the object interprocess global name, visible for all processes in the same machine.
     */
    static bool is_locked(
            const std::string& name)
    {
        try
        {
            RobustExclusiveLock locked_test(name);
        }
        catch (const std::exception&)
        {
            return true;
        }

        return false;
    }

    /**
     *  Unlock the interprocess lock.
     */
    ~RobustExclusiveLock()
    {
        unlock_and_close();
    }

    /**
     * Remove the object
     * @return true when success, false otherwise.
     */
    static bool remove(
            const std::string& name)
    {
        return 0 == std::remove(RobustLock::get_file_path(name).c_str());
    }

private:

    std::string name_;
    int fd_;

#ifdef _MSC_VER

    int open_and_lock_file(
            const std::string& file_path,
            bool* was_lock_created) const
    {
        int test_exist;
        auto ret = _sopen_s(&test_exist, file_path.c_str(), O_RDONLY, _SH_DENYRW, _S_IREAD | _S_IWRITE);

        if (ret == 0)
        {
            *was_lock_created = false;
            return test_exist;
        }

        int fd;
        ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | O_RDONLY, _SH_DENYRW, _S_IREAD | _S_IWRITE);

        if (ret != 0)
        {
            char errmsg[1024];
            strerror_s(errmsg, sizeof(errmsg), errno);
            throw std::runtime_error("failed to open/create " + file_path + " " + std::string(errmsg));
        }

        *was_lock_created = true;

        return fd;
    }

    void unlock_and_close()
    {
        _close(fd_);

        if (0 != std::remove(RobustLock::get_file_path(name_).c_str()))
        {
            logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << RobustLock::get_file_path(name_));
        }
    }

#else

    int open_and_lock_file(
            const std::string& file_path,
            bool* was_lock_created) const
    {
        auto fd = open(file_path.c_str(), O_RDONLY, 0666);

        if (fd != -1)
        {
            *was_lock_created = false;
        }
        else
        {
            *was_lock_created = true;
            fd = open(file_path.c_str(), O_CREAT | O_RDONLY, 0666);
        }

        if (fd == -1)
        {
            throw std::runtime_error(("failed to open/create " + file_path + " " + std::strerror(errno)).c_str());
        }

        // Lock the file
        if (0 != flock(fd, LOCK_EX | LOCK_NB))
        {
            close(fd);
            throw std::runtime_error(("failed to lock " + file_path).c_str());
        }

        return fd;
    }

    void unlock_and_close()
    {
        flock(fd_, LOCK_UN | LOCK_NB);
        close(fd_);

        if (0 != std::remove(RobustLock::get_file_path(name_).c_str()))
        {
            logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << RobustLock::get_file_path(name_));
        }
    }

#endif // ifdef _MSC_VER

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ROBUST_EXCLUSIVE_LOCK_H_
