// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef RTPS_TRANSPORT__ASIO_HELPERS_HPP_
#define RTPS_TRANSPORT__ASIO_HELPERS_HPP_

#include <cstdint>

#include <asio.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Helper functions for asio.
// NOTE: using a struct instead of a namespace to avoid linker errors when using inline free functions.
struct asio_helpers
{
    /**
     * @brief Try to set a buffer size on a socket, trying to set the initial value and then halving it until it is
     * possible to set it or the minimum value is reached.
     *
     * @tparam BufferOptionType Type of the buffer option to set.
     * @tparam SocketType Type of socket on which to set the buffer size option.
     *
     * @param socket Socket on which to set the buffer size option.
     * @param initial_buffer_value Initial value to try to set.
     * @param minimum_buffer_value Minimum value to set.
     * @param final_buffer_value Output parameter where the final value set will be stored.
     *
     * @return true if the buffer size was successfully set, false otherwise.
     */
    template <typename BufferOptionType, typename SocketType>
    static inline bool try_setting_buffer_size(
            SocketType& socket,
            const uint32_t initial_buffer_value,
            const uint32_t minimum_buffer_value,
            uint32_t& final_buffer_value)
    {
        asio::error_code ec;

        final_buffer_value = initial_buffer_value;
        while (final_buffer_value >= minimum_buffer_value)
        {
            socket.set_option(BufferOptionType(static_cast<int32_t>(final_buffer_value)), ec);
            if (!ec)
            {
                return true;
            }

            final_buffer_value /= 2;
        }

        final_buffer_value = minimum_buffer_value;
        socket.set_option(BufferOptionType(final_buffer_value), ec);
        return !ec;
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_TRANSPORT__ASIO_HELPERS_HPP_

