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

/**
 * @file NetworkBuffer.hpp
 */

#ifndef FASTDDS_RTPS_TRANSPORT__NETWORKBUFFER_HPP
#define FASTDDS_RTPS_TRANSPORT__NETWORKBUFFER_HPP

#include <cstdint>

namespace asio {
// Forward declaration of asio::const_buffer
class const_buffer;
} // namespace asio

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * A slice of data to be sent to one or more transports.
 * An RTPS datagram is made up of headers and one or more NetworkBuffer instances.
 */
struct NetworkBuffer final
{
    //! Pointer to the buffer where the data is stored.
    const void* buffer = nullptr;
    //! Number of bytes to use starting at @c buffer.
    uint32_t size = 0;

    NetworkBuffer() = default;

    NetworkBuffer(
            const void* ptr,
            uint32_t s)
        : buffer(ptr)
        , size(s)
    {
    }

    //! Conversion operator to asio::const_buffer.
    operator asio::const_buffer() const;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_TRANSPORT__NETWORKBUFFER_HPP
