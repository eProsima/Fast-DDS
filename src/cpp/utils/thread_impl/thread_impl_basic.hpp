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

class thread : public std::thread
{
public:

    thread() = default;

    template<class _Fn>
    thread(
            int32_t /*stack_size*/,
            _Fn&& _Fx)
        : std::thread(std::forward<_Fn>(_Fx))
    {
    }

    // *INDENT-OFF*
    thread(thread&& _Other) noexcept = default;
    thread& operator =(thread&& _Other) noexcept = default;

    thread(const thread&) = delete;
    thread& operator =(const thread&) = delete;
    // *INDENT-ON*

    inline bool is_calling_thread() const noexcept
    {
        return get_id() == std::this_thread::get_id();
    }

};

} // eprosima
