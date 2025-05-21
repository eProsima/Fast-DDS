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
#elif defined(MINGW_COMPILER)
#include <io.h>
#else
#include <sys/file.h>
#endif // ifdef  _MSC_VER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "SharedDir.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This class implement an interprocess named resource that holds a shared exclusion lock until
 * destroyed, or until the creator process dies
 */
class RobustSharedLock
{
public:

    /**
     * Open or create and acquire the interprocess lock.
     * @param in name Is the object's interprocess global name, visible for all processes in the same machine.
     * @param out was_lock_created If the lock succeeded, this parameter return whether the lock has been created
     * or it already exist.
     * @throw std::exception if lock coulnd't be acquired
     */
    RobustSharedLock(
            const std::string& name,
            bool* was_lock_created,
            bool* was_lock_released)
    {
        auto file_path = SharedDir::get_lock_path(name);

        fd_ = open_and_lock_file(file_path, was_lock_created, was_lock_released);

        name_ = name;
    }

    /**
     * Open or create and acquire the interprocess lock.
     * @param in name Is the object's interprocess global name, visible for all processes in the same machine.
     * @throw std::exception if lock coulnd't be acquired
     */
    RobustSharedLock(
            const std::string& name)
    {
        bool was_lock_created;

        auto file_path = SharedDir::get_lock_path(name);

        fd_ = open_and_lock_file(file_path, &was_lock_created, nullptr);

        name_ = name;
    }

    /**
     * Release the lock
     */
    ~RobustSharedLock()
    {
        unlock_and_close();
    }

    /**
     * Check if there is some process holding the lock
     * @param in name Is the object's interprocess global name.
     * @return true is the resource is locked, false if not locked by anyone
     */
    static bool is_locked(
            const std::string& name)
    {
        return test_lock(SharedDir::get_lock_path(name)) == LockStatus::LOCKED;
    }

    /**
     * Remove the object
     * @param in name Is the object's interprocess global name.
     * @return true when success, false otherwise.
     */
    static bool remove(
            const std::string& name)
    {
        return 0 == std::remove(SharedDir::get_lock_path(name).c_str());
    }

private:

    std::string name_;
    int fd_;

    enum class LockStatus
    {
        NOT_LOCKED,
        OPEN_FAILED,
        LOCKED
    };

#ifdef _MSC_VER

    int open_and_lock_file(
            const std::string& file_path,
            bool* was_lock_created,
            bool* was_lock_released)
    {
        int test_exist;

        // Try open exclusive
        auto ret = _sopen_s(&test_exist, file_path.c_str(), _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

        if (ret == 0)
        {
            *was_lock_created = false;

            if (was_lock_released)
            {
                *was_lock_released = true;
            }

            _close(test_exist);
        }
        else
        {
            // Try open shared
            ret = _sopen_s(&test_exist, file_path.c_str(), _O_RDONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

            if (ret == 0)
            {
                if (was_lock_released)
                {
                    *was_lock_released = false;
                }

                *was_lock_created = false;

                return test_exist;
            }
            else
            {
                if (was_lock_released)
                {
                    *was_lock_released = true;
                }

                *was_lock_created = true;
            }
        }

        int fd;
        // Open or create shared
        ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | _O_RDONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

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

        test_lock(SharedDir::get_lock_path(name_), true);
    }

    static LockStatus test_lock(
            const std::string& file_path,
            bool remove_if_unlocked = false)
    {
        LockStatus lock_status;

        int fd;
        auto ret = _sopen_s(&fd, file_path.c_str(), _O_RDONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE);

        if (ret == 0)
        {
            lock_status = LockStatus::NOT_LOCKED;

            _close(fd);

            // Lock exclusive
            ret = _sopen_s(&fd, file_path.c_str(), _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
            if (ret != 0)
            {
                lock_status = LockStatus::LOCKED;
            }
            else
            {
                _close(fd);
            }
        }
        else
        {
            lock_status = LockStatus::OPEN_FAILED;
        }

        if (lock_status == LockStatus::NOT_LOCKED && remove_if_unlocked)
        {
            if (0 != std::remove(file_path.c_str()))
            {
                EPROSIMA_LOG_WARNING(RTPS_TRANSPORT_SHM, "Failed to remove " << file_path);
            }
        }

        return lock_status;
    }

#elif defined(MINGW_COMPILER)
    int open_and_lock_file(
            const std::string& file_path,
            bool* was_lock_created,
            bool* was_lock_released)
    {
        int test_exist;

        // Try open exclusive
        auto ret = _sopen_s(&test_exist, file_path.c_str(), _O_WRONLY, 0x0010, _S_IREAD | _S_IWRITE);

        if (ret == 0)
        {
            *was_lock_created = false;

            if (was_lock_released)
            {
                *was_lock_released = true;
            }

            _close(test_exist);
        }
        else
        {
            // Try open shared
            ret = _sopen_s(&test_exist, file_path.c_str(), _O_RDONLY, 0x0010, _S_IREAD | _S_IWRITE);

            if (ret == 0)
            {
                if (was_lock_released)
                {
                    *was_lock_released = false;
                }

                *was_lock_created = false;

                return test_exist;
            }
            else
            {
                if (was_lock_released)
                {
                    *was_lock_released = true;
                }

                *was_lock_created = true;
            }
        }

        int fd;
        // Open or create shared
        ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | _O_RDONLY, 0x0010, _S_IREAD | _S_IWRITE);

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

        test_lock(SharedDir::get_lock_path(name_), true);
    }

    static LockStatus test_lock(
            const std::string& file_path,
            bool remove_if_unlocked = false)
    {
        LockStatus lock_status;

        int fd;
        auto ret = _sopen_s(&fd, file_path.c_str(), _O_RDONLY, 0x0010, _S_IREAD | _S_IWRITE);

        if (ret == 0)
        {
            lock_status = LockStatus::NOT_LOCKED;

            _close(fd);

            // Lock exclusive
            ret = _sopen_s(&fd, file_path.c_str(), _O_WRONLY, 0x0010, _S_IREAD | _S_IWRITE);
            if (ret != 0)
            {
                lock_status = LockStatus::LOCKED;
            }
            else
            {
                _close(fd);
            }
        }
        else
        {
            lock_status = LockStatus::OPEN_FAILED;
        }

        if (lock_status == LockStatus::NOT_LOCKED && remove_if_unlocked)
        {
            if (0 != std::remove(file_path.c_str()))
            {
                EPROSIMA_LOG_WARNING(RTPS_TRANSPORT_SHM, "Failed to remove " << file_path);
            }
        }

        return lock_status;
    }

#else

    int open_and_lock_file(
            const std::string& file_path,
            bool* was_lock_created,
            bool* was_lock_released)
    {
        int fd = -1;
        do
        {
            fd = open(file_path.c_str(), O_RDONLY, 0);

            if (fd != -1)
            {
                *was_lock_created = false;
            }
            else
            {
                *was_lock_created = true;
                fd = open(file_path.c_str(), O_CREAT | O_RDONLY, 0666);
            }

            if (was_lock_released != nullptr)
            {
                // Lock exclusive
                if (0 == flock(fd, LOCK_EX | LOCK_NB))
                {
                    // Check if file was deleted by clean up script between open and lock
                    // if yes, repeat file creation
                    struct stat buffer = {};
                    if (stat(file_path.c_str(), &buffer) != 0 && errno == ENOENT)
                    {
                        close(fd);
                        fd = -1;
                        continue;
                    }

                    // Exclusive => shared
                    flock(fd, LOCK_SH | LOCK_NB);
                    *was_lock_released = true;
                    return fd;
                }
                else
                {
                    *was_lock_released = false;
                }
            }

            // Lock shared
            if (0 != flock(fd, LOCK_SH | LOCK_NB))
            {
                close(fd);
                throw std::runtime_error(("failed to lock " + file_path).c_str());
            }
        } while (fd == -1);

        return fd;
    }

    void unlock_and_close()
    {
        flock(fd_, LOCK_UN | LOCK_NB);
        close(fd_);

        test_lock(SharedDir::get_lock_path(name_), true);
    }

    static LockStatus test_lock(
            const std::string& file_path,
            bool remove_if_unlocked = false)
    {
        LockStatus lock_status;

        auto fd = open(file_path.c_str(), O_RDONLY, 0);

        if (fd != -1)
        {
            lock_status = LockStatus::NOT_LOCKED;

            // Try lock exclusive
            if (0 != flock(fd, LOCK_EX | LOCK_NB))
            {
                // Failed so the file is locked shared
                flock(fd, LOCK_UN | LOCK_NB);
                lock_status = LockStatus::LOCKED;
            }

            close(fd);
        }
        else
        {
            lock_status = LockStatus::OPEN_FAILED;
        }

        if (lock_status == LockStatus::NOT_LOCKED && remove_if_unlocked)
        {
            if (0 != std::remove(file_path.c_str()))
            {
                EPROSIMA_LOG_WARNING(RTPS_TRANSPORT_SHM, "Failed to remove " << file_path);
            }
        }

        return lock_status;
    }

#endif // ifdef _MSC_VER

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ROBUST_SHARED_LOCK_H_
