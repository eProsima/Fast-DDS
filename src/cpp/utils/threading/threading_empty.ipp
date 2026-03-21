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

#include <array>
#include <cstdint>

namespace eprosima {

void set_name_to_current_thread(
        std::array<char, 16>& /* thread_name_buffer */,
        const char* /* name */)
{
}

void set_name_to_current_thread(
        std::array<char, 16>& /* thread_name_buffer */,
        const char* /* fmt */,
        uint32_t /* arg */)
{
}

void set_name_to_current_thread(
        std::array<char, 16>& /* thread_name_buffer */,
        const char* /* fmt */,
        uint32_t /* arg1 */,
        uint32_t /* arg2 */)
{
}

void apply_thread_settings_to_current_thread(
        const char* /* thread_name */,
        const fastdds::rtps::ThreadSettings& /*settings*/)
{
}

}  // namespace eprosima
