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

#if !defined(_WIN32)
#include <pthread.h>
#endif  // !defined(_WIN32)

namespace eprosima {

class thread
{

#ifdef _WIN32
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

#else
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

#endif  // _WIN32

public:

#ifdef _WIN32
    typedef void* native_handle_type;
    typedef unsigned long id;
#else
    using native_handle_type = pthread_t;
    using id = pthread_t;
#endif  // _WIN32

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

        // Start thread
        thread_hnd_ = start_thread_impl(stack_size, ThreadProxy<CalleeType>, callee.get());
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
        swap(other);
    }

    thread& operator =(
            thread&& other) noexcept
    {
        if (joinable())
        {
            std::terminate();
        }
        thread_hnd_ = native_handle_type();
        swap(other);
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
        return get_thread_id_impl(thread_hnd_);
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

        join_thread_impl(thread_hnd_);
        thread_hnd_ = native_handle_type();
    }

    inline void detach()
    {
        detach_thread_impl(thread_hnd_);
        thread_hnd_ = native_handle_type();
    }

    inline bool is_calling_thread() const noexcept
    {
        return get_id() == get_current_thread_id_impl();
    }

private:

    native_handle_type thread_hnd_;

#ifdef _WIN32
    using start_routine_type = unsigned(__stdcall*)(void*);
#else
    using start_routine_type = void* (*)(void*);
#endif // ifdef _WIN32

    static native_handle_type start_thread_impl(
            int32_t stack_size,
            start_routine_type start,
            void* arg);
    static id get_thread_id_impl(
            native_handle_type hnd);
    static void join_thread_impl(
            native_handle_type hnd);
    static void detach_thread_impl(
            native_handle_type hnd);
    static id get_current_thread_id_impl();
};

} // eprosima
