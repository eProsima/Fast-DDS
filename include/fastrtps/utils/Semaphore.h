// Copyright 2016 Esteve Fernandez <esteve@apache.org>
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

#ifndef FASTRTPS_SEMAPHORE_H_
#define FASTRTPS_SEMAPHORE_H_

#include <condition_variable>
#include <mutex>

namespace eprosima {
namespace fastrtps {

class Semaphore {
public:
  explicit Semaphore(size_t count = 0);
  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;

  void post();
  void wait();
  void disable();
  void enable();
  void post(int n);

private:
  size_t count_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool disable_;
};

inline Semaphore::Semaphore(size_t count) : count_(count), disable_(false) {}

inline void Semaphore::post() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!disable_)
  {
    ++count_;
    cv_.notify_one();
  }
}

inline void Semaphore::post(int n) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!disable_)
  {
    count_ += n;
    for (int i = 0; i < n; ++i)
    {
      cv_.notify_one();
    }
  }
}

inline void Semaphore::disable() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!disable_)
  {
    count_ = (size_t)-1L;
    cv_.notify_all();
    disable_ = true;
  }
}

inline void Semaphore::enable() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (disable_)
  {
    count_ = 0;
    disable_ = false;
  }
}

inline void Semaphore::wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!disable_)
  {
    cv_.wait(lock, [&] { 
        if (disable_) return true;
        return count_ > 0; 
      });
    --count_;
  }
}

} // fastrtps
} // eprosima

#endif // FASTRTPS_SEMAPHORE_H_
