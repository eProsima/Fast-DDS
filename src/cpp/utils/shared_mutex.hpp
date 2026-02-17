/*
   Copyright Howard Hinnant 2007-2010. Distributed under the Boost
   Software License, Version 1.0. (see http://www.boost.org/LICENSE_1_0.txt)
   The original implementation has been modified to support the POSIX priorities:

       PTHREAD_RWLOCK_PREFER_READER_NP
              This is the default.  A thread may hold multiple read
              locks; that is, read locks are recursive.  According to
              The Single Unix Specification, the behavior is unspecified
              when a reader tries to place a lock, and there is no write
              lock but writers are waiting.  Giving preference to the
              reader, as is set by PTHREAD_RWLOCK_PREFER_READER_NP,
              implies that the reader will receive the requested lock,
              even if a writer is waiting.  As long as there are
              readers, the writer will be starved.

       PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
              Setting the lock kind to this avoids writer starvation as
              long as any read locking is not done in a recursive
              fashion.

    The C++ Standard has not yet (C++20) imposed any requirements on shared_mutex implementation thus
    each platform made its own choices:
        Windows & Boost defaults to PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP.
        Linux & Mac defaults to PTHREAD_RWLOCK_PREFER_READER_NP.
 */

/**
 * @file shared_mutex.hpp
 */

#ifndef _UTILS_SHARED_MUTEX_HPP_
#define _UTILS_SHARED_MUTEX_HPP_

#include <climits>
#include <condition_variable>
#include <map>
#include <mutex>
#include <system_error>
#include <thread>

namespace eprosima {
namespace detail {

// mimic POSIX Read-Write lock syntax
enum class shared_mutex_type
{
    PTHREAD_RWLOCK_PREFER_READER_NP, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
};

class shared_mutex_base
{

protected:

    typedef std::mutex mutex_t;
    typedef std::condition_variable cond_t;
    typedef unsigned count_t;

    mutex_t mut_;
    cond_t gate1_;
    count_t state_;

    static const count_t write_entered_ = 1U << (sizeof(count_t) * CHAR_BIT - 1);
    static const count_t n_readers_ = ~write_entered_;

public:

    shared_mutex_base()
        : state_(0)
    {
    }

    ~shared_mutex_base()
    {
        std::lock_guard<mutex_t> _(mut_);
    }

    shared_mutex_base(
            const shared_mutex_base&) = delete;
    shared_mutex_base& operator =(
            const shared_mutex_base&) = delete;

    // Exclusive ownership

    bool try_lock()
    {
        std::lock_guard<mutex_t> _(mut_);
        if (state_ == 0)
        {
            state_ = write_entered_;
            return true;
        }
        return false;
    }

    void unlock()
    {
        std::lock_guard<mutex_t> _(mut_);
        state_ = 0;
        gate1_.notify_all();
    }

    // Shared ownership

    void lock_shared()
    {
        std::unique_lock<mutex_t> lk(mut_);
        while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
        {
            gate1_.wait(lk);
        }
        count_t num_readers = (state_ & n_readers_) + 1;
        state_ &= ~n_readers_;
        state_ |= num_readers;
    }

    bool try_lock_shared()
    {
        std::lock_guard<mutex_t> _(mut_);
        count_t num_readers = state_ & n_readers_;
        if (!(state_ & write_entered_) && num_readers != n_readers_)
        {
            ++num_readers;
            state_ &= ~n_readers_;
            state_ |= num_readers;
            return true;
        }
        return false;
    }

};

template<shared_mutex_type>
class shared_mutex;

// original Hinnant implementation prioritizing writers

template<>
class shared_mutex<shared_mutex_type::PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP>
    : public shared_mutex_base
{
    cond_t gate2_;

public:

    void lock()
    {
        std::unique_lock<mutex_t> lk(mut_);
        while (state_ & write_entered_)
        {
            gate1_.wait(lk);
        }
        state_ |= write_entered_;
        while (state_ & n_readers_)
        {
            gate2_.wait(lk);
        }
    }

    void unlock_shared()
    {
        std::lock_guard<mutex_t> _(mut_);
        count_t num_readers = (state_ & n_readers_) - 1;
        state_ &= ~n_readers_;
        state_ |= num_readers;
        if (state_ & write_entered_)
        {
            if (num_readers == 0)
            {
                gate2_.notify_one();
            }
        }
        else if (num_readers == n_readers_ - 1)
        {
            gate1_.notify_one();
        }
    }

};

// implementation not locking readers on behalf of writers

template<>
class shared_mutex<shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP>
    : public shared_mutex_base
{
    count_t writer_waiting_ = 0;

public:

    void lock()
    {
        std::unique_lock<mutex_t> lk(mut_);
        ++writer_waiting_;
        while (state_ & n_readers_ || state_ & write_entered_)
        {
            gate1_.wait(lk);
        }
        state_ |= write_entered_;
        --writer_waiting_;
    }

    void unlock_shared()
    {
        std::lock_guard<mutex_t> _(mut_);
        count_t num_readers = (state_ & n_readers_) - 1;
        state_ &= ~n_readers_;
        state_ |= num_readers;

        if ((writer_waiting_ && num_readers == 0)
                || (num_readers == n_readers_ - 1))
        {
            gate1_.notify_one();
        }
    }

};

// Debugger wrapper class that provides insight
template<class sm>
class debug_wrapper : public sm
{
    std::mutex wm_;
    // Identity of the exclusive owner if any
    std::thread::id exclusive_owner_ = {};
    // key_type thread_id, mapped_type number of locks
    std::map<std::thread::id, unsigned int> shared_owners_;

public:

    ~debug_wrapper()
    {
        std::lock_guard<std::mutex> _(wm_);
    }

    // Exclusive ownership

    void lock()
    {
        sm::lock();
        std::lock_guard<std::mutex> _(wm_);
        exclusive_owner_ = std::this_thread::get_id();
    }

    bool try_lock()
    {
        bool res = sm::try_lock();
        std::lock_guard<std::mutex> _(wm_);
        if (res)
        {
            exclusive_owner_ = std::this_thread::get_id();
        }
        return res;
    }

    void unlock()
    {
        sm::unlock();
        std::lock_guard<std::mutex> _(wm_);
        exclusive_owner_ = std::thread::id();
    }

    // Shared ownership

    void lock_shared()
    {
        sm::lock_shared();
        std::lock_guard<std::mutex> _(wm_);
        ++shared_owners_[std::this_thread::get_id()];
    }

    bool try_lock_shared()
    {
        bool res = sm::try_lock_shared();
        std::lock_guard<std::mutex> _(wm_);
        if (res)
        {
            ++shared_owners_[std::this_thread::get_id()];
        }
        return res;
    }

    void unlock_shared()
    {
        sm::unlock_shared();
        std::lock_guard<std::mutex> _(wm_);
        auto owner = shared_owners_.find(std::this_thread::get_id());
        if ( owner != shared_owners_.end() && 0 == --owner->second )
        {
            shared_owners_.erase(owner);
        }
    }

};

} // namespace detail
} // namespace eprosima

#if defined(__has_include) && __has_include(<version>)
#   include <version>
#endif // if defined(__has_include) && __has_include(<version>)

// Detect if the shared_lock feature is available
#if defined(__has_include) && __has_include(<version>) && !defined(__cpp_lib_shared_mutex) || \
    /* deprecated procedure if the good one is not available*/ \
    ( !(defined(__has_include) && __has_include(<version>)) && \
    !(defined(HAVE_CXX17) && HAVE_CXX17) &&  __cplusplus < 201703 )

namespace eprosima {

template <class Mutex>
class shared_lock
{
public:

    typedef Mutex mutex_type;

private:

    mutex_type* m_;
    bool owns_;

    struct __nat
    {
        int _;
    };

public:

    shared_lock()
        : m_(nullptr)
        , owns_(false)
    {
    }

    explicit shared_lock(
            mutex_type& m)
        : m_(&m)
        , owns_(true)
    {
        m_->lock_shared();
    }

    shared_lock(
            mutex_type& m,
            std::defer_lock_t)
        : m_(&m)
        , owns_(false)
    {
    }

    shared_lock(
            mutex_type& m,
            std::try_to_lock_t)
        : m_(&m)
        , owns_(m.try_lock_shared())
    {
    }

    shared_lock(
            mutex_type& m,
            std::adopt_lock_t)
        : m_(&m)
        , owns_(true)
    {
    }

    template <class Clock, class Duration>
    shared_lock(
            mutex_type& m,
            const std::chrono::time_point<Clock, Duration>& abs_time)
        : m_(&m)
        , owns_(m.try_lock_shared_until(abs_time))
    {
    }

    template <class Rep, class Period>
    shared_lock(
            mutex_type& m,
            const std::chrono::duration<Rep, Period>& rel_time)
        : m_(&m)
        , owns_(m.try_lock_shared_for(rel_time))
    {
    }

    ~shared_lock()
    {
        if (owns_)
        {
            m_->unlock_shared();
        }
    }

    shared_lock(
            shared_lock const&) = delete;
    shared_lock& operator =(
            shared_lock const&) = delete;

    shared_lock(
            shared_lock&& sl)
        : m_(sl.m_)
        , owns_(sl.owns_)
    {
        sl.m_ = nullptr; sl.owns_ = false;
    }

    shared_lock& operator =(
            shared_lock&& sl)
    {
        if (owns_)
        {
            m_->unlock_shared();
        }
        m_ = sl.m_;
        owns_ = sl.owns_;
        sl.m_ = nullptr;
        sl.owns_ = false;
        return *this;
    }

    explicit shared_lock(
            std::unique_lock<mutex_type>&& ul)
        : m_(ul.mutex())
        , owns_(ul.owns_lock())
    {
        if (owns_)
        {
            m_->unlock_and_lock_shared();
        }
        ul.release();
    }

    void lock();
    bool try_lock();
    template <class Rep, class Period>
    bool try_lock_for(
            const std::chrono::duration<Rep, Period>& rel_time)
    {
        return try_lock_until(std::chrono::steady_clock::now() + rel_time);
    }

    template <class Clock, class Duration>
    bool
    try_lock_until(
            const std::chrono::time_point<Clock, Duration>& abs_time);
    void unlock();

    void swap(
            shared_lock&& u)
    {
        std::swap(m_, u.m_);
        std::swap(owns_, u.owns_);
    }

    mutex_type* release()
    {
        mutex_type* r = m_;
        m_ = nullptr;
        owns_ = false;
        return r;
    }

    bool owns_lock() const
    {
        return owns_;
    }

    operator int __nat::* () const {
        return owns_ ? &__nat::_ : 0;
    }
    mutex_type* mutex() const
    {
        return m_;
    }

};

template <class Mutex>
void
shared_lock<Mutex>::lock()
{
    if (m_ == nullptr)
    {
        throw std::system_error(std::error_code(EPERM, std::system_category()),
                      "shared_lock::lock: references null mutex");
    }
    if (owns_)
    {
        throw std::system_error(std::error_code(EDEADLK, std::system_category()),
                      "shared_lock::lock: already locked");
    }
    m_->lock_shared();
    owns_ = true;
}

template <class Mutex>
bool
shared_lock<Mutex>::try_lock()
{
    if (m_ == nullptr)
    {
        throw std::system_error(std::error_code(EPERM, std::system_category()),
                      "shared_lock::try_lock: references null mutex");
    }
    if (owns_)
    {
        throw std::system_error(std::error_code(EDEADLK, std::system_category()),
                      "shared_lock::try_lock: already locked");
    }
    owns_ = m_->try_lock_shared();
    return owns_;
}

template <class Mutex>
template <class Clock, class Duration>
bool
shared_lock<Mutex>::try_lock_until(
        const std::chrono::time_point<Clock, Duration>& abs_time)
{
    if (m_ == nullptr)
    {
        throw std::system_error(std::error_code(EPERM, std::system_category()),
                      "shared_lock::try_lock_until: references null mutex");
    }
    if (owns_)
    {
        throw std::system_error(std::error_code(EDEADLK, std::system_category()),
                      "shared_lock::try_lock_until: already locked");
    }
    owns_ = m_->try_lock_shared_until(abs_time);
    return owns_;
}

template <class Mutex>
void
shared_lock<Mutex>::unlock()
{
    if (!owns_)
    {
        throw std::system_error(std::error_code(EPERM, std::system_category()),
                      "shared_lock::unlock: not locked");
    }
    m_->unlock_shared();
    owns_ = false;
}

template <class Mutex>
inline
void
swap(
        shared_lock<Mutex>&  x,
        shared_lock<Mutex>&  y)
{
    x.swap(y);
}

} //namespace eprosima

#else // fallback to STL

#include <shared_mutex>

namespace eprosima {

using std::shared_lock;
using std::swap;

} //namespace eprosima

#endif // shared_lock selection

#ifndef USE_THIRDPARTY_SHARED_MUTEX
#   if defined(_MSC_VER) && _MSVC_LANG < 202302L
#       pragma message("warning: USE_THIRDPARTY_SHARED_MUTEX not defined. By default use framework version.")
#   else
#       warning "USE_THIRDPARTY_SHARED_MUTEX not defined. By default use framework version."
#   endif // if defined(_MSC_VER) && _MSVC_LANG < 202302L
#   define USE_THIRDPARTY_SHARED_MUTEX 0
#endif // ifndef USE_THIRDPARTY_SHARED_MUTEX

// Detect if the share_mutex feature is available or if the user forces it
#if defined(__has_include) && __has_include(<version>) && !defined(__cpp_lib_shared_mutex) || \
    /* allow users to ignore shared_mutex framework implementation */ \
    (~USE_THIRDPARTY_SHARED_MUTEX + 1) || \
    /* deprecated procedure if the good one is not available*/ \
    ( !(defined(__has_include) && __has_include(<version>)) && \
    !(defined(HAVE_CXX17) && HAVE_CXX17) &&  __cplusplus < 201703 )

/*
    Fast DDS defaults to PTHREAD_RWLOCK_PREFER_READER_NP for two main reasons:

    - It allows reader side recursiveness.  If we have two threads (T1, T2) and
      called S a shared lock and E and exclusive one.

        T1: S -> S
        T2:   E

      PTHREAD_RWLOCK_PREFER_READER_NP will never deadlock. The S locks are not
      influenced by the E locks.

      PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP will deadlock.  before T1
      takes S twice. That happens because:
 + T1's second S will wait for E (writer is prioritized)
 + E will wait for T1's first S lock (writer needs atomic access)
 + T1's first S cannot unlock because is blocked in the second S.

      Thus, shared_mutex<PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP> is
      non-recursive.

    - It prevents ABBA deadlocks with other mutexes. If we have three threads
      (Ti) and P is an ordinary mutex:

        T1: P -> S
        T2: S -> P
        T3:   E

      PTHREAD_RWLOCK_PREFER_READER_NP will never deadlock. The S locks are not
      influenced by the E locks. Starvation issues can be managed in the user
      code.

      PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP will deadlock if T3 takes E
      before T1 takes S. That happens because:
 + T1's S will wait for E (writer is prioritized)
 + E will wait for T2's S lock (writer needs atomic access)
 + T2's S cannot unlock because is blocked in P (owned by T1).

      Thus, shared_mutex<PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP> must be
      managed like an ordinary mutex in deadlock sense.
 */

namespace eprosima {

#ifdef NDEBUG
using shared_mutex = detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP>;
#else
using shared_mutex =
        detail::debug_wrapper<detail::shared_mutex<detail::shared_mutex_type::PTHREAD_RWLOCK_PREFER_READER_NP>>;
#endif // NDEBUG

} //namespace eprosima

#else // fallback to STL

#include <shared_mutex>

namespace eprosima {

using std::shared_mutex;

} //namespace eprosima

#endif // shared_mutex selection

#endif // _UTILS_SHARED_MUTEX_HPP_
