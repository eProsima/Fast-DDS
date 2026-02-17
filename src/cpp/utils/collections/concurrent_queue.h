// Copyright 2020 Canonical ltd.
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


#ifndef _FASTDDS_UTILS_CONCURRENT_QUEUE_H_
#define _FASTDDS_UTILS_CONCURRENT_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace fastdds {

/**
 * @brief ConcurrentQueue. A thread-safe shared queue.
 *
 * Multiple producers, multiple consumers thread safe queue.
 * Since 'return by reference' is used this queue won't throw.
 *
 * Based on std::queue<T>.
 */
template<typename T, typename Sequence = std::deque<T>>
class ConcurrentQueue final
{
    using Queue = std::queue<T, Sequence>;

public:

    ConcurrentQueue()  = default;
    ~ConcurrentQueue() = default;

    ConcurrentQueue(
            const std::size_t count)
        : queue_(typename Queue::container_type(count))
    {
    }

    // not-copyable
    ConcurrentQueue& operator =(
            const ConcurrentQueue&) = delete;

    ConcurrentQueue(
            const ConcurrentQueue& other) = delete;

    // not-movable
    ConcurrentQueue& operator =(
            ConcurrentQueue&&)  = delete;

    ConcurrentQueue(
            const ConcurrentQueue&& other) = delete;

    /**
     * @brief push Move an object to the queue.
     * @param item The object to push to the queue.
     */
    template <typename O>
    void push(
            O&& item)
    {
        //    static_assert(std::is_same<typename std::remove_cv<O>::type, T>::value,
        //                  "Unexpected argument type.");

        std::unique_lock<std::mutex> lock(mutex_);
        queue_.emplace(std::forward<O>(item));

        // Unlock mutex before notifying
        lock.unlock();

        has_data_.notify_one();
    }

    /**
     * @brief Return true immediately if successful retrieval.
     * @param popped_item The popped item.
     * @return true if an object was pop, false otherwise.
     *
     * @see wait_pop
     */
    bool try_pop(
            T& popped_item)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.empty())
        {
            return false;
        }

        popped_item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /**
     * @brief wait_pop Wait until an object is popped.
     * @return The object popped out of the queue.
     *
     * @see try_pop
     */
    T wait_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        has_data_.wait(lock, [&]()
                {
                    return !queue_.empty();
                });

        auto popped_item = std::move(queue_.front());
        queue_.pop();
        return popped_item;
    }

    /**
     * @brief empty Whether the queue is empty or not
     * @return true if empty, false otherwise.
     */
    bool empty() const noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief size The queue size
     * @return size of the queue.
     */
    std::size_t size() const noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

protected:

    std::condition_variable has_data_;
    mutable std::mutex mutex_;

    Queue queue_;
};

} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UTILS_CONCURRENT_QUEUE_H_
