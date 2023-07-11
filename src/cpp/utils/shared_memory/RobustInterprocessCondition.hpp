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

#ifndef _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_CONDITION_
#define _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_CONDITION_

#include <boost/interprocess/sync/detail/locks.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "BoostAtExitRegistry.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

namespace bi = boost::interprocess;

class RobustInterprocessCondition
{
public:

    RobustInterprocessCondition()
        : list_listening_(SemaphoreList::LIST_NULL, SemaphoreList::LIST_NULL)
        , list_free_(0, MAX_LISTENERS - 1)
    {
        init_sem_list();
    }

    /**
     * If there is a thread waiting on *this, change that
     * thread's state to ready. Otherwise there is no effect.
     * @throw boost::interprocess::interprocess_exception on error.
     */
    void notify_one()
    {
        bi::scoped_lock<bi::interprocess_mutex> lock(semaphore_lists_mutex_);

        auto sem_index = list_listening_.head();

        if (sem_index != SemaphoreList::LIST_NULL)
        {
            semaphores_pool_[sem_index].sem.post();
        }
    }

    /**
     * Change the state of all threads waiting on *this to ready.
     * If there are no waiting threads, notify_all() has no effect.
     * @throw boost::interprocess::interprocess_exception on error.
     */
    void notify_all()
    {
        bi::scoped_lock<bi::interprocess_mutex> lock(semaphore_lists_mutex_);

        auto sem_index = list_listening_.head();

        while (sem_index != SemaphoreList::LIST_NULL)
        {
            semaphores_pool_[sem_index].sem.post();
            sem_index = semaphores_pool_[sem_index].next;
        }
    }

    /**
     * Releases the lock on the interprocess_mutex object associated with lock, blocks
     * the current thread of execution until readied by a call to
     * this->notify_one() or this->notify_all(), and then reacquires the lock.
     * @throw boost::interprocess::interprocess_exception on error.
     */
    template <typename L>
    void wait(
            L& lock)
    {
        do_wait(*lock.mutex());
    }

    /**
     * The same as:
     * while (!pred()) wait(lock)
     * @throw boost::interprocess::interprocess_exception on error.
     */
    template <typename L, typename Pr>
    void wait(
            L& lock,
            Pr pred)
    {
        while (!pred())
        {
            do_wait(*lock.mutex());
        }
    }

    /**
     * Releases the lock on the interprocess_mutex object associated with lock, blocks
     * the current thread of execution until readied by a call to
     * this->notify_one() or this->notify_all(), or until time abs_time is reached,
     * and then reacquires the lock.
     * @return false if time abs_time is reached, otherwise true.
     * @throw boost::interprocess::interprocess_exception on error.
     */
    template <typename L>
    bool timed_wait(
            L& lock,
            const boost::posix_time::ptime& abs_time)
    {
        //Handle infinity absolute time here to avoid complications in do_timed_wait
        if (abs_time == boost::posix_time::pos_infin)
        {
            this->wait(lock);
            return true;
        }
        return this->do_timed_wait(abs_time, *lock.mutex());
    }

    /**
     * The same as:
     * while (!pred())
     * {
     *     if (!timed_wait(lock, abs_time)) return pred();
     * }
     * return true;
     */
    template <typename L, typename Pr>
    bool timed_wait(
            L& lock,
            const boost::posix_time::ptime& abs_time,
            Pr pred)
    {
        // Posix does not support infinity absolute time so handle it here
        if (abs_time == boost::posix_time::pos_infin)
        {
            wait(lock, pred);
            return true;
        }
        while (!pred())
        {
            if (!do_timed_wait(abs_time, *lock.mutex()))
            {
                return pred();
            }
        }
        return true;
    }

    /**
     * Converts a std::chrono::time_point to a boost::posix_time::ptime.
     * As timed_wait only can handle boost ptime,
     * this conversion is required when the deadline is expressed as a time_point.
     * The resulting ptime will have microsecond precision if the library supports it,
     * or second precision otherwise.
     * @return boost ptime equivalent of the input std time_point.
     */
    static boost::posix_time::ptime steady_clock_time_point_to_ptime (
            const std::chrono::time_point<std::chrono::steady_clock>& time_point)
    {
        std::chrono::microseconds remaining =
                std::chrono::duration_cast<std::chrono::microseconds>(time_point - std::chrono::steady_clock::now());

        return boost::get_system_time() + boost::posix_time::microseconds(remaining.count());
    }

    /**
     * Releases the lock on the interprocess_mutex object associated with lock, blocks
     * the current thread of execution until readied by a call to
     * this->notify_one() or this->notify_all(), or until time abs_time is reached,
     * and then reacquires the lock.
     * @return false if time abs_time is reached, otherwise true.
     * @throw boost::interprocess::interprocess_exception on error.
     */
    template <typename L>
    bool timed_wait(
            L& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        return timed_wait(lock, RobustInterprocessCondition::steady_clock_time_point_to_ptime(max_blocking_time));
    }

    /**
     * The same as:
     * while (!pred())
     * {
     *     if (!timed_wait(lock, abs_time)) return pred();
     * }
     * return true;
     */
    template <typename L, typename Pr>
    bool timed_wait(
            L& lock,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
            Pr pred)
    {
        return timed_wait(lock, RobustInterprocessCondition::steady_clock_time_point_to_ptime(max_blocking_time), pred);
    }

private:

    struct SemaphoreNode
    {
        bi::interprocess_semaphore sem {0};
        uint32_t next;
        uint32_t prev;
    };

    static constexpr uint32_t MAX_LISTENERS = 512;
    SemaphoreNode semaphores_pool_[MAX_LISTENERS];

    class SemaphoreList
    {
    private:

        uint32_t head_;
        uint32_t tail_;

    public:

        static constexpr uint32_t LIST_NULL = static_cast<uint32_t>(-1);

        SemaphoreList(
                uint32_t head,
                uint32_t tail)
            : head_(head)
            , tail_(tail)
        {
        }

        inline void push(
                uint32_t sem_index,
                SemaphoreNode* sem_pool)
        {
            if (tail_ != LIST_NULL)
            {
                sem_pool[tail_].next = sem_index;
            }

            sem_pool[sem_index].prev = tail_;
            sem_pool[sem_index].next = LIST_NULL;

            tail_ = sem_index;

            if (head_ == LIST_NULL)
            {
                head_ = sem_index;
            }
        }

        inline uint32_t pop(
                SemaphoreNode* sem_pool)
        {
            if (tail_ == LIST_NULL)
            {
                throw bi::interprocess_exception("RobustInterprocessCondition: pop() on empty list!");
            }

            uint32_t sem_index = tail_;
            tail_ = sem_pool[tail_].prev;

            if (tail_ != LIST_NULL)
            {
                sem_pool[tail_].next = LIST_NULL;
            }
            else
            {
                head_ = LIST_NULL;
            }

            return sem_index;
        }

        inline uint32_t tail() const
        {
            return tail_;
        }

        inline uint32_t head() const
        {
            return head_;
        }

        inline void remove(
                uint32_t sem_index,
                SemaphoreNode* sem_pool)
        {
            assert(sem_index != LIST_NULL);

            auto prev = sem_pool[sem_index].prev;
            auto next = sem_pool[sem_index].next;

            if (prev != LIST_NULL)
            {
                sem_pool[prev].next = next;
            }

            if (next != LIST_NULL)
            {
                sem_pool[next].prev = prev;
            }

            if (head_ == sem_index)
            {
                head_ = next;
            }

            if (tail_ == sem_index)
            {
                tail_ = prev;
            }
        }

    };

    SemaphoreList list_listening_;
    SemaphoreList list_free_;
    bi::interprocess_mutex semaphore_lists_mutex_;

    void init_sem_list()
    {
        semaphores_pool_[0].prev = SemaphoreList::LIST_NULL;
        semaphores_pool_[0].next = 1;

        for (uint32_t i = 1; i < MAX_LISTENERS - 1; i++)
        {
            semaphores_pool_[i].next = i + 1;
            semaphores_pool_[i].prev = i - 1;
        }

        semaphores_pool_[MAX_LISTENERS - 1].prev = MAX_LISTENERS - 2;
        semaphores_pool_[MAX_LISTENERS - 1].next = SemaphoreList::LIST_NULL;
    }

    inline uint32_t enqueue_listener()
    {
        auto sem_index = list_free_.pop(semaphores_pool_);
        list_listening_.push(sem_index, semaphores_pool_);
        return sem_index;
    }

    inline void dequeue_listener(
            uint32_t sem_index)
    {
        list_listening_.remove(sem_index, semaphores_pool_);
        list_free_.push(sem_index, semaphores_pool_);
    }

    inline void do_wait(
            bi::interprocess_mutex& mut)
    {
        uint32_t sem_index;

        {
            bi::scoped_lock<bi::interprocess_mutex> lock_enqueue(semaphore_lists_mutex_);
            sem_index = enqueue_listener();
        }

        {
            // Release caller's lock
            bi::ipcdetail::lock_inverter<bi::interprocess_mutex> inverted_lock(mut);
            bi::scoped_lock<bi::ipcdetail::lock_inverter<bi::interprocess_mutex>> unlock(inverted_lock);

            // timed_wait (infin) is used, instead wait, because wait on semaphores could throw when
            // BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING is set. We don't want that for our condition_variables
            semaphores_pool_[sem_index].sem.timed_wait(boost::posix_time::ptime(boost::posix_time::pos_infin));
        }

        {
            bi::scoped_lock<bi::interprocess_mutex> lock_dequeue(semaphore_lists_mutex_);
            dequeue_listener(sem_index);
        }
    }

    inline bool do_timed_wait(
            const boost::posix_time::ptime& abs_time,
            bi::interprocess_mutex& mut)
    {
        bool ret;
        uint32_t sem_index;

        {
            bi::scoped_lock<bi::interprocess_mutex> lock_enqueue(semaphore_lists_mutex_);
            sem_index = enqueue_listener();
        }

        {
            // Release caller's lock
            bi::ipcdetail::lock_inverter<bi::interprocess_mutex> inverted_lock(mut);
            bi::scoped_lock<bi::ipcdetail::lock_inverter<bi::interprocess_mutex>> unlock(inverted_lock);

            ret = semaphores_pool_[sem_index].sem.timed_wait(abs_time);
        }

        {
            bi::scoped_lock<bi::interprocess_mutex> lock_dequeue(semaphore_lists_mutex_);
            dequeue_listener(sem_index);
        }

        return ret;
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_ROBUST_INTERPROCESS_CONDITION_
