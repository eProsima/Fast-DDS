// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SimplePDPEndpoints.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTS_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTS_HPP_

#include <memory>

#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/reader/StatelessReader.h>
#include <fastdds/rtps/writer/StatelessWriter.h>

#include <rtps/builtin/BuiltinReader.hpp>
#include <rtps/builtin/BuiltinWriter.hpp>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/history/ITopicPayloadPool.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Container for the builtin endpoints of PDPSimple
 */
struct SimplePDPEndpoints : public PDPEndpoints
{
    ~SimplePDPEndpoints() override = default;

    fastrtps::rtps::BuiltinEndpointSet_t builtin_endpoints() const override
    {
        return DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    }

    //! Builtin Simple PDP reader
    BuiltinReader<fastrtps::rtps::StatelessReader> reader;

    //! Builtin Simple PDP writer
    BuiltinWriter<fastrtps::rtps::StatelessWriter> writer;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTS_HPP_
