// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file RefCountedPointer.hpp
 */

#ifndef UTILS__REFCOUNTEDPOINTER_HPP
#define UTILS__REFCOUNTEDPOINTER_HPP

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

namespace eprosima {
namespace fastdds {

/**
 * @brief Class to manage a local pointer with reference counting.
 *
 * It is similar to std::shared_ptr, but designed for cases where
 * a shared pointer cannot be used due to API restrictions.
 *
 * USAGE:
 * - On T class:
 *   - Add a shared_ptr<RefCountedPointer<T>> local_ptr_ member.
 *   - Call local_ptr_->deactivate() before destroying T.
 *
 * - On classes that need to use a pointer to T:
 *   - Keep a copy of the shared_ptr<RefCountedPointer<T>>.
 *   - Whenever you need to access T:
 *     RefCountedPointer<T>::Instance instance(local_ptr_)
 *     if (instance)
 *     {
 *         ptr->method();
 *     }
 */
template<typename T>
class RefCountedPointer
{
public:

    class Instance;

    /**
     * @brief Explicit constructor.
     * @param ptr Pointer to manage.
     *
     * @pre nullptr != ptr. We must ensure that the pointer we
     * are manaing is valid.
     */
    explicit RefCountedPointer(
            T* ptr)
        : ptr_(ptr)
        , is_active_(true)
        , instances_(0)
    {
        assert(nullptr != ptr);
    }

    ~RefCountedPointer() = default;

    // Non-copyable and non-movable
    RefCountedPointer(
            const RefCountedPointer&) = delete;
    RefCountedPointer& operator =(
            const RefCountedPointer&) = delete;
    RefCountedPointer(
            RefCountedPointer&&) = delete;
    RefCountedPointer& operator =(
            RefCountedPointer&&) = delete;

    /**
     * @brief Class to manage the local pointer instance.
     * It will increase the reference count on construction and decrease
     * it on destruction. Provides a facade to access the pointee.
     */
    class Instance
    {
    public:

        /**
         * @brief Constructor.
         * @param parent Shared pointer reference to its RefCountedPointer.
         */
        explicit Instance(
                const std::shared_ptr<RefCountedPointer<T>>& parent)
            : parent_(parent)
            , ptr_(parent && parent->is_active_ ? parent->ptr_ : nullptr)
        {
            if (parent_)
            {
                parent_->inc_instances();
            }
        }

        /**
         * @brief Destructor.
         */
        ~Instance()
        {
            if (parent_)
            {
                parent_->dec_instances();
            }
        }

        // Non-copyable, default movable
        Instance(
                const Instance&) = delete;
        Instance& operator =(
                const Instance&) = delete;
        Instance(
                Instance&&) = default;
        Instance& operator =(
                Instance&&) = default;

        /**
         * @brief operator to check if the pointer is valid.
         */
        operator bool() const
        {
            return nullptr != ptr_;
        }

        /**
         * @brief operator to call the T methods.
         */
        T* operator ->() const
        {
            assert(nullptr != ptr_);
            return ptr_;
        }

    private:

        std::shared_ptr<RefCountedPointer<T>> parent_;
        T* const ptr_;
    };

    /**
     * @brief Ensure no more valid local pointer instances are created, and wait for current ones to die.
     */
    void deactivate()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        is_active_ = false;
        cv_.wait(lock, [this]() -> bool
                {
                    return instances_ == 0;
                });
    }

private:

    /**
     * @brief Increase the reference count.
     */
    void inc_instances()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++instances_;
    }

    /**
     * @brief Decrease the reference count.
     */
    void dec_instances()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        --instances_;
        if (instances_ == 0)
        {
            cv_.notify_one();
        }
    }

    /**
     * Pointer to the managed object.
     */
    T* const ptr_;

    /**
     * Indicates whether the pointee is still alive
     * and accessing the pointer is valid.
     */
    std::atomic<bool> is_active_;

    /**
     * Protections for the number of instances.
     */
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    /**
     * Number of active instances (currently using the pointee).
     */
    size_t instances_;
};

}  // namespace fastdds
}  // namespace eprosima

#endif // UTILS__REFCOUNTEDPOINTER_HPP
