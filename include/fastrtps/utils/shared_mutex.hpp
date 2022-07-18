// Copyright Howard Hinnant 2007-2010. Distributed under the Boost
// Software License, Version 1.0. (see http://www.boost.org/LICENSE_1_0.txt)

/**
 * @file shared_mutex.hpp
 */

#ifndef _UTILS_SHARED_MUTEX_HPP_
#define _UTILS_SHARED_MUTEX_HPP_

#ifndef USE_THIRDPARTY_SHARED_MUTEX
#   if defined(_MSC_VER) && _MSVC_LANG < 202302L
#       pragma message("warning: USE_THIRDPARTY_SHARED_MUTEX not defined. By default use framework version.")
#   else
#       warning "USE_THIRDPARTY_SHARED_MUTEX not defined. By default use framework version."
#   endif
#   define USE_THIRDPARTY_SHARED_MUTEX 0
#endif

#if defined(__has_include) && __has_include(<version>)
#   include <version>
#endif // if defined(__has_include) && __has_include(<version>)

// Detect if the share_mutex feature is available
#if defined(__has_include) && __has_include(<version>) && !defined(__cpp_lib_shared_mutex) || \
    /* allow users to ignore shared_mutex framework implementation */ \
    (~ USE_THIRDPARTY_SHARED_MUTEX + 1) || \
    /* deprecated procedure if the good one is not available*/ \
    ( !(defined(__has_include) && __has_include(<version>)) && \
    !(defined(HAVE_CXX17) && HAVE_CXX17) &&  __cplusplus < 201703 )

#include <mutex>
#include <condition_variable>
#include <climits>
#include <system_error>

namespace eprosima {

class shared_mutex
{
    typedef std::mutex mutex_t;
    typedef std::condition_variable cond_t;
    typedef unsigned count_t;

    mutex_t mut_;
    cond_t gate1_;
    cond_t gate2_;
    count_t state_;

    static const count_t write_entered_ = 1U << (sizeof(count_t) * CHAR_BIT - 1);
    static const count_t n_readers_ = ~write_entered_;

public:

    shared_mutex()
        : state_(0)
    {
    }

    ~shared_mutex()
    {
        std::lock_guard<mutex_t> _(mut_);
    }

    shared_mutex(
            const shared_mutex&) = delete;
    shared_mutex& operator =(
            const shared_mutex&) = delete;

    // Exclusive ownership

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

    bool try_lock()
    {
        std::unique_lock<mutex_t> lk(mut_);
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
        std::unique_lock<mutex_t> lk(mut_);
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
        else
        {
            if (num_readers == n_readers_ - 1)
            {
                gate1_.notify_one();
            }
        }
    }

};

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

} //namespace eprosima

#else // fallback to STL

#include <shared_mutex>

namespace eprosima {

using std::shared_mutex;
using std::shared_lock;

} //namespace eprosima

#endif // if (__cplusplus < 201402) || (defined(_MSC_VER) && _MSC_VER < 1900 )

#endif // _UTILS_SHARED_MUTEX_HPP_
