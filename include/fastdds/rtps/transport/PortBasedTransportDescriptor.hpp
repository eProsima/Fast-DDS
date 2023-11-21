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

#ifndef _FASTDDS_PORT_BASED_TRANSPORT_DESCRIPTOR_H_
#define _FASTDDS_PORT_BASED_TRANSPORT_DESCRIPTOR_H_

#include <cstdint>
#include <map>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Base class for all port based transport descriptors
 *
 * This class provides a common thread settings configuration for all
 * port based transport descriptor implementations
 *
 * @ingroup TRANSPORT_MODULE
 */
class PortBasedTransportDescriptor : public TransportDescriptorInterface
{
public:

    using ReceptionThreadsConfigMap = std::map<uint32_t, ThreadSettings>;

    //! Constructor
    RTPS_DllAPI PortBasedTransportDescriptor(
            uint32_t maximumMessageSize,
            uint32_t maximumInitialPeersRange);

    //! Copy constructor
    RTPS_DllAPI PortBasedTransportDescriptor(
            const PortBasedTransportDescriptor& t) = default;

    //! Copy assignment
    RTPS_DllAPI PortBasedTransportDescriptor& operator =(
            const PortBasedTransportDescriptor& t) = default;

    //! Destructor
    virtual RTPS_DllAPI ~PortBasedTransportDescriptor() = default;

    //! Comparison operator
    bool RTPS_DllAPI operator ==(
            const PortBasedTransportDescriptor& t) const;

    /**
     * @brief Get the ThreadSettings for a specific port
     *
     * This function first looks for the port-specific ThreadSettings in the user-configured
     * reception threads map, i.e. the collection of ThreadSettings returned by @ref reception_threads().
     * If the ThreadSettings are found within said map, then @ref get_thread_config_for_port() returns them;
     * else it returns the default reception thread settings, i.e. the ThreadSettings returned by
     * @ref default_reception_threads().
     *
     * @warning This function will return the default reception thread ThreadSettings when called with a non-default,
     * non-user-configured port.
     *
     * @param port The port to which the returned ThreadSetting apply.
     *
     * @return The ThreadSettings for the given port.
     */
    virtual RTPS_DllAPI const ThreadSettings& get_thread_config_for_port(
            uint32_t port) const;

    virtual RTPS_DllAPI bool set_thread_config_for_port(
            const uint32_t& port,
            const ThreadSettings& thread_settings);

    //! Returns the ThreadSettings for the default reception threads
    RTPS_DllAPI const ThreadSettings& default_reception_threads() const;

    //! Set the ThreadSettings for the default reception threads
    virtual RTPS_DllAPI void default_reception_threads(
            const ThreadSettings& default_reception_threads);

    //! Returns the ThreadSettings for the user-configured reception threads
    RTPS_DllAPI const ReceptionThreadsConfigMap& reception_threads() const;

    //! Set the ThreadSettings for the user-configured reception threads
    virtual RTPS_DllAPI bool reception_threads(
            const ReceptionThreadsConfigMap& reception_threads);

protected:

    //! Thread settings for the default reception threads
    ThreadSettings default_reception_threads_;

    //! Thread settings for the specific reception threads, indexed by port
    ReceptionThreadsConfigMap reception_threads_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef _FASTDDS_PORT_BASED_TRANSPORT_DESCRIPTOR_H_
