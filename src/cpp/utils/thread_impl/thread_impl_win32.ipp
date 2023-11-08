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

#include <process.h>

#ifndef NOMINMAX
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <windows.h>
#endif

namespace eprosima {

class thread
{
    // This method is a generic proxy that serves as the starting address of the thread
    template <typename CalleeType>
    static unsigned __stdcall ThreadProxy(
            void* Ptr)
    {
        // Take ownership of the trampoline
        std::unique_ptr<CalleeType> Callee(static_cast<CalleeType*>(Ptr));
        // Call the trampoline
        (*Callee)();
        // Finish thread
        return 0;
    }

public:

    using native_handle_type = HANDLE;
    using id = DWORD;

    thread()
        : thread_hnd_(native_handle_type())
    {
    }

    template<class _Fn>
    thread(
            int32_t stack_size,
            _Fn&& f)
    {
        // Prepare trampoline to pass to ThreadProxy
        using CalleeType = typename std::decay<_Fn>::type;
        std::unique_ptr<CalleeType> callee(new CalleeType(std::forward<_Fn>(f)));

        // Set the requested stack size, if given.
        unsigned stack_attr = 0;
        if (stack_size > 0)
        {
            if (sizeof(unsigned) <= sizeof(int32_t) &&
                    stack_size > static_cast<int32_t>(std::numeric_limits<unsigned>::max() / 2))
            {
                throw std::invalid_argument("Cannot cast stack_size into unsigned");
            }

            stack_attr = static_cast<unsigned>(stack_size);
        }

        // Construct and execute the thread.
        HANDLE hnd = (HANDLE) ::_beginthreadex(NULL, stack_attr, ThreadProxy<CalleeType>, callee.get(), 0, NULL);
        if(!hnd)
        {
            throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
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
        return ::GetThreadId(thread_hnd_);;
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

        if (::WaitForSingleObject(thread_hnd_, INFINITE) == WAIT_FAILED)
        {
            throw std::system_error(std::make_error_code(std::errc::no_such_process));
        }

        thread_hnd_ = native_handle_type();
    }

    inline void detach()
    {
        if (::CloseHandle(thread_hnd_) == FALSE)
        {
            throw std::system_error(std::make_error_code(std::errc::no_such_process));
        }

        thread_hnd_ = native_handle_type();
    }

    inline bool is_calling_thread() const noexcept
    {
        return get_id() == ::GetCurrentThreadId();
    }

private:

    native_handle_type thread_hnd_;
};

} // eprosima
