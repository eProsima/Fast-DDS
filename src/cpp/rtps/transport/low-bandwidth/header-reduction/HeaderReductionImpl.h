// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef HEADER_REDUCTION_IMPL_H
#define HEADER_REDUCTION_IMPL_H

#include <fastrtps/rtps/common/Types.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * \brief This structure stores the bytes reduction in RTPS Header.
 */
struct HRCONFIG_RTPS_Header
{
    bool eliminate_protocol;
    bool eliminate_version;
    bool eliminate_vendorId;
    uint32_t reduce_guidPrefix[3]; // Bit level.
};

#define HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE 4
#define HRCONFIG_RTPS_HEADER_VERSION_SIZE 2
#define HRCONFIG_RTPS_HEADER_VENDORID_SIZE 2
#define HRCONFIG_RTPS_HEADER_GUIDPREFIX_SIZE 12
#define HRCONFIG_RTPS_HEADER_DEFAULT {false, false, false, {32, 32, 32 \
                                      } \
}

struct HRCONFIG_Submessage_Header
{
    bool combine_submessageId_with_flags;
};

#define HRCONFIG_SUBMESSAGE_HEADER_ID_SIZE 1
#define HRCONFIG_SUBMESSAGE_HEADER_FLAGS_SIZE 1
#define HRCONFIG_SUBMESSAGE_HEADER_DEFAULT {false}

struct HRCONFIG_Submessage_Body
{
    bool eliminate_extraFlags;
    uint32_t reduce_entitiesId[2];
    uint32_t reduce_sequenceNumber;
};

#define HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE 2
#define HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE 2
#define HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE 8
#define HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE 8
#define HRCONFIG_SUBMESSAGE_BODY_DEFAULT {false, {32, 32}, 64}

/**
 * \brief This structure stores the bytes reduction in the RTPS packet
 */
struct HeaderReductionOptions
{
    HRCONFIG_RTPS_Header rtps_header;
    HRCONFIG_Submessage_Header submessage_header;
    HRCONFIG_Submessage_Body submessage_body;
};

#define HRCONFIG_RTPS_PACKET_DEFAULT {HRCONFIG_RTPS_HEADER_DEFAULT, HRCONFIG_SUBMESSAGE_HEADER_DEFAULT, \
                                      HRCONFIG_SUBMESSAGE_BODY_DEFAULT}

bool HeaderReduction_Reduce(
    fastrtps::rtps::octet* dest_buffer,
    const fastrtps::rtps::octet* src_buffer,
    uint32_t & buffer_length,
    const HeaderReductionOptions & reductions);

bool HeaderReduction_Recover(
    fastrtps::rtps::octet* dest_buffer,
    const fastrtps::rtps::octet* src_buffer,
    uint32_t & buffer_length,
    const HeaderReductionOptions & reductions);

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
