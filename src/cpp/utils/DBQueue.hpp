// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#ifndef _FASTDDS_DBQUEUE_HPP
#define _FASTDDS_DBQUEUE_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace eprosima {
namespace fastdds {

/**
 * Double buffered, threadsafe queue for MPSC (multi-producer, single-consumer) comms.
 */
template<class T>
class DBQueue
{

public:

    DBQueue()
        : mForegroundQueue(&mQueueAlpha)
        , mBackgroundQueue(&mQueueBeta)
    {
    }

    //! Clears foreground queue and swaps queues.
    void Swap()
    {
        std::unique_lock<std::mutex> fgGuard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);

        // Clear the foreground queue.
        std::queue<T>().swap(*mForegroundQueue);

        auto* swap       = mBackgroundQueue;
        mBackgroundQueue = mForegroundQueue;
        mForegroundQueue = swap;
    }

    //! Pushes to the background queue. Copy constructor.
    void Push(
            const T& item)
    {
        std::unique_lock<std::mutex> guard(mBackgroundMutex);
        mBackgroundQueue->push(item);
    }

    //! Pushes to the background queue. Move constructor.
    void Push(
            T&& item)
    {
        std::unique_lock<std::mutex> guard(mBackgroundMutex);
        mBackgroundQueue->push(std::move(item));
    }

    //! Returns a reference to the front element
    //! in the foregrund queue.
    T& Front()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->front();
    }

    const T& Front() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->front();
    }

    //! Pops from the foreground queue.
    void Pop()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        mForegroundQueue->pop();
    }

    //! Return the front element in the foreground queue by moving it and erase it from the queue.
    T FrontAndPop()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);

        // Get value by moving the internal queue reference to a new value
        T value = std::move(mForegroundQueue->front());
        // At this point mForegroundQueue contains a non valid element, but mutex is taken and next instruction erase it

        // Pop value from queue
        mForegroundQueue->pop();

        // Return value (as it has been created in this scope, it will not be copied but moved or directly forwarded)
        return value;
    }

    //! Reports whether the foreground queue is empty.
    bool Empty() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->empty();
    }

    //! Reports whether the both queues are empty.
    bool BothEmpty() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);
        return mForegroundQueue->empty() && mBackgroundQueue->empty();
    }

    //! Reports the size of the foreground queue.
    size_t Size() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->size();
    }

    //! Clears foreground and background.
    void Clear()
    {
        std::unique_lock<std::mutex> fgGuard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);
        std::queue<T>().swap(*mForegroundQueue);
        std::queue<T>().swap(*mBackgroundQueue);
    }

private:

    // Underlying queues
    std::queue<T> mQueueAlpha;
    std::queue<T> mQueueBeta;

    // Front and background queue references (double buffering)
    std::queue<T>* mForegroundQueue;
    std::queue<T>* mBackgroundQueue;

    mutable std::mutex mForegroundMutex;
    mutable std::mutex mBackgroundMutex;
};


} // namespace fastdds
} // namespace eprosima

#endif // ifndef DBQUEUE_H
