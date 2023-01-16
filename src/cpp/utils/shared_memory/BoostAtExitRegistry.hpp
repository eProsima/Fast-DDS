// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_UTILS_SHARED_MEMORY__BOOSTATEXITREGISTRY_HPP_
#define FASTDDS_UTILS_SHARED_MEMORY__BOOSTATEXITREGISTRY_HPP_

#include <memory>
#include <mutex>
#include <vector>

namespace eprosima {
namespace detail {

/**
 * A singleton that holds pointers to the boost interprocess singleton destructors.
 */
struct BoostAtExitRegistry
{
    /**
     * @return BoostAtExitRegistry singleton instance.
     */
    static std::shared_ptr<BoostAtExitRegistry> get_instance()
    {
        // Note we need a custom deleter, since the destructor is private.
        static std::shared_ptr<BoostAtExitRegistry> instance(
            new BoostAtExitRegistry(),
            [](BoostAtExitRegistry* p)
            {
                delete p;
            });
        return instance;
    }

    /**
     * Register a function to be called when the instance of this singleton is destroyed.
     *
     * Since the goal of this singleton is to simulate the behavior of std::atexit, the registered functions will
     * be called in reverse registration order.
     *
     * @param f Function to be registered.
     */
    void at_exit_register(
            void (* f)())
    {
        std::lock_guard<std::mutex> _(mtx_);
        registered_functions_.push_back(f);
    }

private:

    ~BoostAtExitRegistry()
    {
        // Execute the registered functions in reverse order.
        std::unique_lock<std::mutex> lock(mtx_);
        while (!registered_functions_.empty())
        {
            // Pop one function
            auto f = registered_functions_.back();
            registered_functions_.pop_back();

            // Execute function with the mutex released, to allow registering another function in the mean time.
            lock.unlock();
            f();
            lock.lock();
        }
    }

    std::mutex mtx_;
    std::vector<void (*)()> registered_functions_;

};

}  // namespace detail
}  // namespace eprosima

#endif  // FASTDDS_UTILS_SHARED_MEMORY__BOOSTATEXITREGISTRY_HPP_
