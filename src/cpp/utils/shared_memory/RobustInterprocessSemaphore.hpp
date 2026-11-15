// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_SEMAPHORE_
#define _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_SEMAPHORE_

#ifdef __APPLE__

#include <atomic>
#include <cstdint>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/interprocess/sync/spin/wait.hpp>
#include <boost/thread/thread_time.hpp>

#include <os/clock.h>
#include <os/os_sync_wait_on_address.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Process-shared counting semaphore for macOS.
 *
 * macOS has no process-shared unnamed POSIX semaphores, so boost::interprocess::interprocess_semaphore
 * falls back to a spin semaphore that sleeps between polls. That adds milliseconds of latency to
 * every shared-memory notification. This semaphore blocks in the kernel with
 * os_sync_wait_on_address(OS_SYNC_WAIT_ON_ADDRESS_SHARED) (macOS 14.4+), and spins like Boost
 * on older systems.
 *
 * It has the same layout as boost's spin_semaphore (a single 32-bit counter in shared memory),
 * so processes that still use the spin semaphore interoperate with it.
 */
class RobustInterprocessSemaphore
{
public:

    explicit RobustInterprocessSemaphore(
            uint32_t initial_count)
        : count_(initial_count)
    {
    }

    RobustInterprocessSemaphore(
            const RobustInterprocessSemaphore&) = delete;
    RobustInterprocessSemaphore& operator =(
            const RobustInterprocessSemaphore&) = delete;

    void post()
    {
        count_.fetch_add(1, std::memory_order_release);
        if (__builtin_available(macOS 14.4, *))
        {
            os_sync_wake_by_address_any(&count_, sizeof(uint32_t), OS_SYNC_WAKE_BY_ADDRESS_SHARED);
        }
    }

    /**
     * @return false if abs_time (UTC, as boost::get_system_time()) is reached, otherwise true.
     */
    bool timed_wait(
            const boost::posix_time::ptime& abs_time)
    {
        const bool infinite = abs_time.is_pos_infinity();
        boost::interprocess::spin_wait swait;

        while (!try_wait())
        {
            uint64_t timeout_ns = 0;
            if (!infinite)
            {
                const boost::posix_time::time_duration remaining = abs_time - boost::get_system_time();
                if (remaining.is_negative() || remaining.total_microseconds() == 0)
                {
                    return try_wait();
                }
                timeout_ns = static_cast<uint64_t>(remaining.total_microseconds()) * 1000u;
            }

            if (__builtin_available(macOS 14.4, *))
            {
                // Sleeps only while the counter is still 0, so a post() between try_wait() and
                // here is not lost. Each sleep is capped, because processes using an older Fast DDS
                // post with boost's spin_semaphore, which increments the counter without waking us.
                if (infinite || timeout_ns > LEGACY_POST_POLL_NS)
                {
                    timeout_ns = LEGACY_POST_POLL_NS;
                }
                os_sync_wait_on_address_with_timeout(&count_, 0, sizeof(uint32_t),
                        OS_SYNC_WAIT_ON_ADDRESS_SHARED, OS_CLOCK_MACH_ABSOLUTE_TIME, timeout_ns);
            }
            else
            {
                swait.yield();
            }
        }

        return true;
    }

private:

    //! Upper bound for one kernel wait, so posts from processes that don't wake us are still seen.
    static constexpr uint64_t LEGACY_POST_POLL_NS = 1000000u;

    bool try_wait()
    {
        uint32_t current = count_.load(std::memory_order_acquire);
        while (current != 0)
        {
            if (count_.compare_exchange_weak(current, current - 1, std::memory_order_acquire))
            {
                return true;
            }
        }
        return false;
    }

    std::atomic<uint32_t> count_;
    static_assert(sizeof(std::atomic<uint32_t>) == sizeof(uint32_t), "Must match boost spin_semaphore layout");
    static_assert(ATOMIC_INT_LOCK_FREE == 2, "Must be lock free to live in shared memory");
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __APPLE__

#endif // _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_SEMAPHORE_
