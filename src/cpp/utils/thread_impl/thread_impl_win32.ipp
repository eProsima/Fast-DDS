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

#include <thread>

namespace eprosima {

class thread { // class for observing and managing threads
public:
    using id = _Thrd_id_t;

    thread() noexcept : _Thr{} {}

private:
#if _HAS_CXX20
    friend jthread;
#endif // _HAS_CXX20

    template <class _Tuple, size_t... _Indices>
    static unsigned int __stdcall _Invoke(void* _RawVals) noexcept /* terminates */ {
        // adapt invoke of user's callable object to _beginthreadex's thread procedure
        const _STD unique_ptr<_Tuple> _FnVals(static_cast<_Tuple*>(_RawVals));
        _Tuple& _Tup = *_FnVals;
        _STD invoke(_STD move(_STD get<_Indices>(_Tup))...);
        _Cnd_do_broadcast_at_thread_exit(); // TRANSITION, ABI
        return 0;
    }

    template <class _Tuple, size_t... _Indices>
    _NODISCARD static constexpr auto _Get_invoke(_STD index_sequence<_Indices...>) noexcept {
        return &_Invoke<_Tuple, _Indices...>;
    }

    template <class _Fn, class... _Args>
    void _Start(_Fn&& _Fx, _Args&&... _Ax) {
        using _Tuple                 = _STD tuple<_STD decay_t<_Fn>, _STD decay_t<_Args>...>;
        auto _Decay_copied           = _STD make_unique<_Tuple>(_STD forward<_Fn>(_Fx), _STD forward<_Args>(_Ax)...);
        constexpr auto _Invoker_proc = _Get_invoke<_Tuple>(_STD make_index_sequence<1 + sizeof...(_Args)>{});

#pragma warning(push)
#pragma warning(disable : 5039) // pointer or reference to potentially throwing function passed to
                                // extern C function under -EHc. Undefined behavior may occur
                                // if this function throws an exception. (/Wall)
        _Thr._Hnd =
            reinterpret_cast<void*>(_CSTD _beginthreadex(nullptr, 0, _Invoker_proc, _Decay_copied.get(), 0, &_Thr._Id));
#pragma warning(pop)

        if (_Thr._Hnd) { // ownership transferred to the thread
            (void) _Decay_copied.release();
        } else { // failed to start thread
            _Thr._Id = 0;
            _Throw_Cpp_error(_STD _RESOURCE_UNAVAILABLE_TRY_AGAIN);
        }
    }

public:
    template <class _Fn, class... _Args>
    _NODISCARD_CTOR explicit thread(int32_t /*stack_size*/, _Fn&& _Fx, _Args&&... _Ax) {
        _Start(_STD forward<_Fn>(_Fx), _STD forward<_Args>(_Ax)...);
    }

    ~thread() noexcept {
        if (joinable()) {
            _STD terminate();
        }
    }

    thread(thread&& _Other) noexcept : _Thr(_STD exchange(_Other._Thr, {})) {}

    thread& operator=(thread&& _Other) noexcept {
        if (joinable()) {
            _STD terminate();
        }

        _Thr = _STD exchange(_Other._Thr, {});
        return *this;
    }

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    void swap(thread& _Other) noexcept {
        _STD swap(_Thr, _Other._Thr);
    }

    _NODISCARD bool joinable() const noexcept {
        return _Thr._Id != 0;
    }

    void join() {
        if (!joinable()) {
            _STD _Throw_Cpp_error(_STD _INVALID_ARGUMENT);
        }

        if (_Thr._Id == _Thrd_id()) {
            _STD _Throw_Cpp_error(_STD _RESOURCE_DEADLOCK_WOULD_OCCUR);
        }

        if (_Thrd_join(_Thr, nullptr) != _Thrd_success) {
            _STD _Throw_Cpp_error(_STD _NO_SUCH_PROCESS);
        }

        _Thr = {};
    }

    void detach() {
        if (!joinable()) {
            _STD _Throw_Cpp_error(_STD _INVALID_ARGUMENT);
        }

        _STD _Check_C_return(_Thrd_detach(_Thr));
        _Thr = {};
    }

    _NODISCARD inline id get_id() const noexcept {
        return _Thr._Id;
    }

    inline bool is_calling_thread() const noexcept {
        return _Thrd_id() == _Thr._Id;
    }

    _NODISCARD static unsigned int hardware_concurrency() noexcept {
        return std::thread::hardware_concurrency();
    }

    _NODISCARD std::thread::native_handle_type native_handle() { // return Win32 HANDLE as void *
        return _Thr._Hnd;
    }

private:
    _Thrd_t _Thr;
};

} // eprosima
