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

#ifndef _FASTDDS_SHAREDMEM_WATCHDOG_H_
#define _FASTDDS_SHAREDMEM_WATCHDOG_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_set>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This singleton launch a thread to perform background maintenance tasks.
 * As tasks are virtual, diferent types of task can be added.
 */
class SharedMemWatchdog
{
public:

    class Task
    {
    public:

        virtual void run() = 0;
    };

    static std::shared_ptr<SharedMemWatchdog>& get()
    {
        static std::shared_ptr<SharedMemWatchdog> watch_dog_instance(new SharedMemWatchdog());
        return watch_dog_instance;
    }

    /**
     * Add a new task to the tasks list
     * @param task Pointer to a singleton task
     */
    void add_task(
            Task* task)
    {
        std::lock_guard<std::mutex> lock(running_tasks_mutex_);

        tasks_.insert(task);
    }

    /**
     * Remove a task from the list
     * @param task Pointer to a singleton task
     */
    void remove_task(
            Task* task)
    {
        std::lock_guard<std::mutex> lock(running_tasks_mutex_);

        auto it = tasks_.find(task);
        if (it != tasks_.end())
        {
            tasks_.erase(it);
        }
    }

    static void set_thread_settings(
            const ThreadSettings& thr_config)
    {
        thread_settings() = thr_config;
    }

    static constexpr std::chrono::milliseconds period()
    {
        return std::chrono::milliseconds(1000);
    }

    ~SharedMemWatchdog()
    {
        exit_thread_ = true;
        wake_up();
        thread_run_.join();
    }

private:

    std::unordered_set<Task*> tasks_;
    eprosima::thread thread_run_;

    std::mutex running_tasks_mutex_;
    std::condition_variable wake_run_cv_;
    std::mutex wake_run_mutex_;
    bool wake_run_;

    std::atomic_bool exit_thread_;

    static ThreadSettings& thread_settings()
    {
        static ThreadSettings s_settings(ThreadSettings{});
        return s_settings;
    }

    SharedMemWatchdog()
        : wake_run_(false)
        , exit_thread_(false)
    {
        auto fn = [this]()
                {
                    run();
                };
        thread_run_ = create_thread(fn, thread_settings(), "dds.shm.wdog");
    }

    /**
     * Forces Wake-up of the checking thread
     */
    void wake_up()
    {
        {
            std::lock_guard<std::mutex> lock(wake_run_mutex_);
            wake_run_ = true;
        }

        wake_run_cv_.notify_one();
    }

    void run()
    {
        while (!exit_thread_)
        {
            {
                std::unique_lock<std::mutex> lock(wake_run_mutex_);

                wake_run_cv_.wait_for(
                    lock,
                    period(),
                    [&]
                    {
                        return wake_run_;
                    });

                wake_run_ = false;
            }

            std::lock_guard<std::mutex> lock(running_tasks_mutex_);

            for (auto task : tasks_)
            {
                task->run();
            }
        }
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef _FASTDDS_SHAREDMEM_WATCHDOG_H_
