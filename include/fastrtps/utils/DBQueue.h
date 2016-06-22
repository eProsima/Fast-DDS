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
#ifndef DBQUEUE_H
#define DBQUEUE_H

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

namespace eprosima {
namespace fastrtps{

/**
 * Double buffered, threadsafe queue for MPSC (multi-producer, single-consumer) comms.
 */
template<class T> 
class DBQueue {

public:
   DBQueue();

   //! Clears front queue and swaps queues.
   void Swap();

   //! Pushes to the background queue.
   void Push(const T&);

   //! Returns a reference to the front element
   //! in the foregrund queue.
   T& Front(); 
   const T& Front() const;

   //! Pops from the foreground queue.
   void Pop();

   //! Reports whether the foreground queue is empty.
   bool Empty() const;

   //! Reports the size of the foreground queue.
   size_t Size() const;

private:
   // Underlying queues
   std::queue<T> mQueueAlpha;
   std::queue<T> mQueueBeta;

   // Front and background queue references (double buffering)
   std::queue<T>& mForegroundQueue;
   std::queue<T>& mBackgroundQueue;

   mutable std::mutex mForegroundMutex;
   mutable std::mutex mBackgroundMutex;
};


} // namespace fastrtps
} // namespace eprosima

#endif
