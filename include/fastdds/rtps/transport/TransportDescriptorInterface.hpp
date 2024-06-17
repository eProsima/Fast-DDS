// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TransportDescriptorInterface.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TRANSPORTDESCRIPTORINTERFACE_HPP
#define FASTDDS_RTPS_TRANSPORT__TRANSPORTDESCRIPTORINTERFACE_HPP

#include <cstdint>
#include <mutex>
#include <vector>

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * Virtual base class for the data type used to define transport configuration.
 * It acts as a builder for a given transport meaning that it allows to configure
 * the transport, and then a new Transport can be built according to this configuration
 * using its create_transport() factory member function.
 *
 * - maxMessageSize: maximum size of a single message in the transport.
 *
 * - maxInitialPeersRange: number of channels opened with each initial remote peer.
 *
 * @ingroup RTPS_MODULE
 * */
struct TransportDescriptorInterface
{
    //! Constructor
    FASTDDS_EXPORTED_API TransportDescriptorInterface(
            uint32_t maximumMessageSize,
            uint32_t maximumInitialPeersRange)
        : maxMessageSize(maximumMessageSize)
        , maxInitialPeersRange(maximumInitialPeersRange)
    {
    }

    //! Copy constructor
    FASTDDS_EXPORTED_API TransportDescriptorInterface(
            const TransportDescriptorInterface& t)
        : maxMessageSize(t.maxMessageSize)
        , maxInitialPeersRange(t.maxInitialPeersRange)
    {
    }

    //! Copy assignment
    FASTDDS_EXPORTED_API TransportDescriptorInterface& operator =(
            const TransportDescriptorInterface& t)
    {
        maxMessageSize = t.maxMessageSize;
        maxInitialPeersRange = t.maxInitialPeersRange;
        return *this;
    }

    //! Destructor
    virtual FASTDDS_EXPORTED_API ~TransportDescriptorInterface() = default;

    /**
     * Factory method pattern. It will create and return a TransportInterface
     * corresponding to this descriptor. This provides an interface to the NetworkFactory
     * to create the transports without the need to know about their type
     */
    virtual FASTDDS_EXPORTED_API TransportInterface* create_transport() const = 0;

    //! Returns the minimum size required for a send operation.
    virtual FASTDDS_EXPORTED_API uint32_t min_send_buffer_size() const = 0;

    //! Returns the maximum size expected for received messages.
    virtual FASTDDS_EXPORTED_API uint32_t max_message_size() const
    {
        return maxMessageSize;
    }

    /** Returns the maximum number of opened channels for each initial remote peer
     * (maximum number of guessed initial peers to try to connect)
     */
    virtual FASTDDS_EXPORTED_API uint32_t max_initial_peers_range() const
    {
        return maxInitialPeersRange;
    }

    //! Comparison operator
    FASTDDS_EXPORTED_API bool operator ==(
            const TransportDescriptorInterface& t) const
    {
        return (this->maxMessageSize == t.max_message_size() &&
               this->maxInitialPeersRange == t.max_initial_peers_range());
    }

    //! Lock internal mutex (for Fast-DDS internal use)
    FASTDDS_EXPORTED_API void lock()
    {
        mtx_.lock();
    }

    //! Unlock internal mutex (for Fast-DDS internal use)
    FASTDDS_EXPORTED_API void unlock()
    {
        mtx_.unlock();
    }

    //! Maximum size of a single message in the transport
    uint32_t maxMessageSize;

    //! Number of channels opened with each initial remote peer.
    uint32_t maxInitialPeersRange;

private:

    mutable std::mutex mtx_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TRANSPORTDESCRIPTORINTERFACE_HPP
