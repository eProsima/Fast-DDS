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

#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>

#include <pthread.h>

namespace eprosima {

class thread
{
    // This method is a generic proxy that serves as the starting address of the thread
    template <typename CalleeType>
    static void* ThreadProxy(
            void* Ptr)
    {
        // Take ownership of the trampoline
        std::unique_ptr<CalleeType> Callee(static_cast<CalleeType*>(Ptr));
        // Call the trampoline
        (*Callee)();
        // Finish thread
        return nullptr;
    }

public:

    using native_handle_type = pthread_t;
    using id = pthread_t;

    thread()
        : thread_hnd_(native_handle_type())
    {
    }

    template<class _Fn>
    thread(
            int32_t stack_size,
            _Fn&& f)
    {
        int errnum;

        // Prepare trampoline to pass to ThreadProxy
        using CalleeType = typename std::decay<_Fn>::type;
        std::unique_ptr<CalleeType> callee(new CalleeType(std::forward<_Fn>(f)));

        // Construct the attributes object.
        pthread_attr_t attr;
        if ((errnum = ::pthread_attr_init(&attr)) != 0)
        {
            throw std::system_error(errnum, std::system_category(), "pthread_attr_init failed");
        }

        // Ensure the attributes object is destroyed
        auto attr_deleter = [](pthread_attr_t* a)
                {
                    int err;
                    if ((err = ::pthread_attr_destroy(a)) != 0)
                    {
                        throw std::system_error(err, std::system_category(), "pthread_attr_destroy failed");
                    }
                };
        std::unique_ptr<pthread_attr_t, decltype(attr_deleter)> attr_scope_destroy(&attr, attr_deleter);

        // Set the requested stack size, if given.
        if (stack_size > 0)
        {
            if (sizeof(unsigned) <= sizeof(int32_t) &&
                    stack_size > static_cast<int32_t>(std::numeric_limits<unsigned>::max() / 2))
            {
                throw std::invalid_argument("Cannot cast stack_size into unsigned");
            }

            if ((errnum = ::pthread_attr_setstacksize(&attr, stack_size)) != 0)
            {
                throw std::system_error(errnum, std::system_category(), "pthread_attr_setstacksize failed");
            }
        }

        // Construct and execute the thread.
        pthread_t hnd;
        if ((errnum = ::pthread_create(&hnd, &attr, ThreadProxy<CalleeType>, callee.get())) != 0)
        {
            throw std::system_error(errnum, std::system_category(), "pthread_create failed");
        }

        thread_hnd_ = hnd;
        if (thread_hnd_ != native_handle_type())
        {
            // Thread has been correctly created. Since the ThreadProxy will
            // take ownership of the trampoline, we need to release ownership here
            callee.release();
        }
    }

    ~thread()
    {
        if (joinable())
        {
            std::terminate();
        }
    }

    // *INDENT-OFF*
    thread(const thread&) = delete;
    thread& operator =(const thread&) = delete;
    // *INDENT-ON*

    thread(
            thread&& other) noexcept
        : thread_hnd_(native_handle_type())
    {
        std::swap(thread_hnd_, other.thread_hnd_);
    }

    thread& operator =(
            thread&& other) noexcept
    {
        if (joinable())
        {
            std::terminate();
        }
        thread_hnd_ = native_handle_type();
        std::swap(thread_hnd_, other.thread_hnd_);
        return *this;
    }

    void swap(
            thread& other) noexcept
    {
        std::swap(thread_hnd_, other.thread_hnd_);
    }

    inline bool joinable() const noexcept
    {
        return thread_hnd_ != native_handle_type();
    }

    inline id get_id() const noexcept
    {
        return thread_hnd_;
    }

    inline native_handle_type native_handle() const noexcept
    {
        return thread_hnd_;
    }

    static unsigned hardware_concurrency()
    {
        return std::thread::hardware_concurrency();
    }

    inline void join()
    {
        if (!joinable())
        {
            throw std::system_error(std::make_error_code(std::errc::invalid_argument));
        }

        if (is_calling_thread())
        {
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        }

        int errnum;

        if ((errnum = ::pthread_join(thread_hnd_, nullptr)) != 0)
        {
            throw std::system_error(std::make_error_code(std::errc::no_such_process), "pthread_join failed");
        }
        thread_hnd_ = native_handle_type();
    }

    inline void detach()
    {
        int errnum;

        if ((errnum = ::pthread_detach(thread_hnd_)) != 0)
        {
            throw std::system_error(errnum, std::system_category(), "pthread_detach failed");
        }
        thread_hnd_ = native_handle_type();
    }

    inline bool is_calling_thread() const noexcept
    {
        return thread_hnd_ == ::pthread_self();
    }

private:

    native_handle_type thread_hnd_;
};

} // eprosima
