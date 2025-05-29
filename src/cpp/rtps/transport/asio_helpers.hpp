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

        assert(initial_buffer_value >= minimum_buffer_value);

        final_buffer_value = initial_buffer_value;
        while (final_buffer_value > minimum_buffer_value)
        {
            int32_t value_to_set = static_cast<int32_t>(final_buffer_value);
            socket.set_option(BufferOptionType(value_to_set), ec);
            if (!ec)
            {
                BufferOptionType option;
                socket.get_option(option, ec);
                if (!ec)
                {
                    if (option.value() == value_to_set)
                    {
                        // Option actually set to the desired value
                        return true;
                    }
                    // Try again with the value actually set
                    final_buffer_value = option.value();
                    continue;
                }
                // Could not determine the actual value, even though the option was set successfully.
                // The current buffer size is not defined.
                return false;
            }

            final_buffer_value /= 2;
        }

        // Perform a final attempt to set the minimum value
        final_buffer_value = minimum_buffer_value;
        int32_t value_to_set = static_cast<int32_t>(final_buffer_value);
        socket.set_option(BufferOptionType(value_to_set), ec);
        if (!ec)
        {
            // Last attempt was successful. Get the actual value set.
            int32_t max_value = static_cast<int32_t>(initial_buffer_value);
            BufferOptionType option;
            socket.get_option(option, ec);
            if (!ec && (option.value() >= value_to_set) && (option.value() <= max_value))
            {
                final_buffer_value = option.value();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Configure a buffer size on a socket, using the system default value if the initial value is 0.
     * Ensures that the final buffer size is at least the minimum value.
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
    static inline bool configure_buffer_size(
            SocketType& socket,
            const uint32_t initial_buffer_value,
            const uint32_t minimum_buffer_value,
            uint32_t& final_buffer_value)
    {
        final_buffer_value = initial_buffer_value;

        // If the initial value is 0, try using the system default value
        if (initial_buffer_value == 0)
        {
            asio::error_code ec;
            BufferOptionType option;
            socket.get_option(option, ec);
            if (!ec)
            {
                final_buffer_value = option.value();
            }
        }

        // Ensure the minimum value is used
        if (final_buffer_value < minimum_buffer_value)
        {
            final_buffer_value = minimum_buffer_value;
        }

        // Try to set the highest possible value the system allows
        return try_setting_buffer_size<BufferOptionType>(socket, final_buffer_value, minimum_buffer_value,
                       final_buffer_value);
    }

    /**
     * @brief Configure the send and receive buffer sizes on a socket, using the system default value if the initial
     * values are 0. Ensures that the final buffer sizes are at least the minimum value.
     *
     * @tparam SocketType Type of socket on which to set the buffer size options.
     *
     * @param socket Socket on which to set the buffer size options.
     * @param descriptor Transport descriptor with the buffer sizes to set.
     * @param final_send_buffer_size Output parameter where the final send buffer size will be stored.
     * @param final_receive_buffer_size Output parameter where the final receive buffer size will be stored.
     *
     * @return true if the buffer sizes were successfully set, false otherwise.
     */
    template<typename SocketType>
    static inline bool configure_buffer_sizes(
            SocketType& socket,
            const SocketTransportDescriptor& descriptor,
            uint32_t& final_send_buffer_size,
            uint32_t& final_receive_buffer_size)
    {
        uint32_t minimum_socket_buffer = descriptor.maxMessageSize;
        uint32_t send_buffer_size = descriptor.sendBufferSize;
        uint32_t receive_buffer_size = descriptor.receiveBufferSize;

        bool send_buffer_size_set = configure_buffer_size<asio::socket_base::send_buffer_size>(
            socket, send_buffer_size, minimum_socket_buffer, final_send_buffer_size);
        bool receive_buffer_size_set = configure_buffer_size<asio::socket_base::receive_buffer_size>(
            socket, receive_buffer_size, minimum_socket_buffer, final_receive_buffer_size);

        return send_buffer_size_set && receive_buffer_size_set;
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_TRANSPORT__ASIO_HELPERS_HPP_

