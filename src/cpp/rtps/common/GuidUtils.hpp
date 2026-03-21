// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file GuidUtils.hpp
 */

#ifndef RTPS_COMMON_GUIDUTILS_HPP_
#define RTPS_COMMON_GUIDUTILS_HPP_

#include <cstdint>
#include <limits>
#include <random>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <utils/SystemInfo.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This singleton handles the generation of GUID prefix
 */
class GuidUtils
{

public:

    /**
     * Create a GUID prefix based on a participant id.
     *
     * @param [in]  participant_id  Identifier of the participant for which to generate the GUID prefix.
     * @param [out] guid_prefix     Generated GUID prefix.
     */
    void guid_prefix_create(
            uint32_t participant_id,
            GuidPrefix_t& guid_prefix) const
    {
        // Use precalculated vendor-host-process part of the prefix
        std::copy(prefix_.value, prefix_.value + 8, guid_prefix.value);

        // Add little endian serialization of participant_id
        guid_prefix.value[8] = static_cast<octet>(participant_id & 0xFF);
        guid_prefix.value[9] = static_cast<octet>((participant_id >> 8) & 0xFF);
        guid_prefix.value[10] = static_cast<octet>((participant_id >> 16) & 0xFF);
        guid_prefix.value[11] = static_cast<octet>((participant_id >> 24) & 0xFF);
    }

    /**
     * Get a copy of \c prefix_ attribute.
     *
     * @return copy of \c prefix_ attribute.
     */
    GuidPrefix_t prefix() const
    {
        return prefix_;
    }

    /**
     * Get a reference to the singleton instance.
     *
     * @return reference to the singleton instance.
     */
    static const GuidUtils& instance()
    {
        static GuidUtils singleton;
        return singleton;
    }

private:

    GuidUtils()
    {
        // This is to comply with RTPS section 9.3.1.5 - Mapping of the GUID_t
        prefix_.value[0] = c_VendorId_eProsima[0];
        prefix_.value[1] = c_VendorId_eProsima[1];

        // On Fast DDS, next two bytes should be the same across all processes on the same host
        uint16_t host_id = SystemInfo::instance().host_id();
        prefix_.value[2] = static_cast<octet>(host_id & 0xFF);
        prefix_.value[3] = static_cast<octet>((host_id >> 8) & 0xFF);

        // On Fast DDS, next four bytes would be the same across all participants on the same process.
        // Even though using the process id here might seem a nice idea, there are cases where it might not serve as
        // unique identifier of the process:
        // - One of them is when using a Kubernetes pod on which several containers with their own PID namespace are
        //   created.
        // - Another one is when a system in which a Fast DDS application is started during boot time. If the system
        //   crashes and is then re-started, it may happen that the participant may be considered an old one if the
        //   announcement lease duration did not expire.
        // In order to behave correctly in those situations, we will use the 16 least-significant bits of the PID,
        // along with a random 16 bits value. This should not be a problem, as the PID is known to be 16 bits long on
        // several systems. On those where it is longer, using the 16 least-significant ones along with a random value
        // should still give enough uniqueness for our use cases.
        int pid = SystemInfo::instance().process_id();
        prefix_.value[4] = static_cast<octet>(pid & 0xFF);
        prefix_.value[5] = static_cast<octet>((pid >> 8) & 0xFF);

        std::random_device generator;
        std::uniform_int_distribution<uint16_t> distribution(0, std::numeric_limits<uint16_t>::max());
        uint16_t rand_value = distribution(generator);
        prefix_.value[6] = static_cast<octet>(rand_value & 0xFF);
        prefix_.value[7] = static_cast<octet>((rand_value >> 8) & 0xFF);
    }

    GuidPrefix_t prefix_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // RTPS_COMMON_GUIDUTILS_HPP_
