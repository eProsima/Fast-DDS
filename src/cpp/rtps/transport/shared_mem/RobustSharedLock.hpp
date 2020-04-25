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

/**
 * This class implement an interprocess named resource that holds a shared exclusion lock until
 * destroyed, or until the creator process dies
 */
class RobustSharedLock
{
public:

    /**
     * Create the interprocess lock.
     * @throw std::exception if the lock cannot be acquired
     */
    RobustSharedLock(
            const std::string& name)
    {
        auto file_path = get_file_path(name);

        fd_ = open_and_lock_file(file_path);

        name_ = name;
    }

    /**
     * Release de lock
     */
    ~RobustSharedLock()
    {
        unlock_and_close();
    }

    /**
     * Check if there is some process holding the lock
     * @return true is the resource is locked, false if not locked by anyone
     */
    static bool is_locked(
            const std::string& name)
    {
        return test_lock(get_file_path(name)) == LockStatus::LOCKED;
    }

    /**
     * Remove the object
     * @return true when success, false otherwise.
     */
    static bool remove(
            const std::string& name)
    {
        return 0 == std::remove(get_file_path(name).c_str());
    }

    /**
     * Creates a new lock instance.
     * @return nullptr if the resource is locked, std::unique_ptr to the new lock if the resource was not locked.
     */
    static std::unique_ptr<RobustSharedLock> try_lock_as_new(
            const std::string& name)
    {
        return try_lock_internal(name);
    }

private:

    RobustSharedLock(
            const std::string& name,
            int file_descriptor)
    {
        fd_ = file_descriptor;

        name_ = name;
    }

    std::string name_;
    int fd_;

    enum class LockStatus
    {
        NOT_LOCKED,
        OPEN_FAILED,
        LOCKED
    };

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

    int open_and_lock_file(
            const std::string& file_path)
    {
        int fd;
        // Lock shared
        auto ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | _O_RDONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

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

        test_lock(get_file_path(name_), true);
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
                logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << file_path);
            }
        }

        return lock_status;
    }

    static std::unique_ptr<RobustSharedLock> try_lock_internal(
        const std::string& name)
    {
        auto file_path = get_file_path(name);

        int fd;

        // Lock exclusive
        auto ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
        if (ret == 0)
        {
            _close(fd);
            // TODO(Adolfo): Is there a safer way (like in linux) to conmute the lock 
            // from exclusive to shared atomically (without close / open)?
            // Lock shared
            ret = _sopen_s(&fd, file_path.c_str(), O_CREAT | _O_RDONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

            if (ret == 0)
            {
                return std::unique_ptr<RobustSharedLock>(new RobustSharedLock(name, fd));
            }
        }

        return nullptr;
    }

#else

    int open_and_lock_file(
            const std::string& file_path)
    {
        auto fd = open(file_path.c_str(), O_CREAT | O_RDONLY, 0666);

        if (fd == -1)
        {
            throw std::runtime_error(("failed to open/create " + file_path + " " + std::strerror(errno)).c_str());
        }

        // Lock shared
        if (0 != flock(fd, LOCK_SH |LOCK_NB))
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

        test_lock(get_file_path(name_), true);
    }

    static LockStatus test_lock(
            const std::string& file_path,
            bool remove_if_unlocked = false)
    {
        LockStatus lock_status;

        auto fd = open(file_path.c_str(), O_RDONLY, 0666);

        if (fd != -1)
        {
            lock_status = LockStatus::NOT_LOCKED;

            // Try lock exclusive
            if (0 != flock(fd, LOCK_EX |LOCK_NB))
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
                logWarning(RTPS_TRANSPORT_SHM, "Failed to remove " << file_path);
            }
        }

        return lock_status;
    }

    static std::unique_ptr<RobustSharedLock> try_lock_internal(
            const std::string& name)
    {
        auto file_path = get_file_path(name);

        auto fd = open(file_path.c_str(), O_CREAT | O_RDONLY, 0666);

        if (fd == -1)
        {
            return nullptr;
        }

        // Lock exclusive
        if (0 == flock(fd, LOCK_EX |LOCK_NB))
        {
            // Exclusive => shared
            flock(fd, LOCK_SH | LOCK_NB);
            return std::unique_ptr<RobustSharedLock>(new RobustSharedLock(name, fd));
        }
        else
        {
            close(fd);
        }

        return nullptr;
    }

#endif

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_ROBUST_SHARED_LOCK_H_
