// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <stdint.h>
#include <stddef.h>

#include "fuzz_utils.hpp"

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/network/NetworkFactory.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

static RTPSParticipantAttributes participant_attributes;
static NetworkFactory network {participant_attributes};
static RTPSParticipantAllocationAttributes allocation_attrs;
static bool initialized = false;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (!initialized)
    {
        ignore_stdout();
        initialized = true;
    }

    if (size < 5 || size > 65536)
    {
        return 0;
    }

    const uint8_t selector = data[0] % 3u;
    const uint8_t* payload = data + 1;
    const size_t payload_size = size - 1;

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(payload);
    msg.length = static_cast<uint32_t>(payload_size);
    msg.max_size = static_cast<uint32_t>(payload_size);
    msg.reserved_size = static_cast<uint32_t>(payload_size);
    msg.pos = 0;

    try
    {
        switch (selector)
        {
            case 0:
            {
                ParticipantProxyData ppd(allocation_attrs);
                ppd.read_from_cdr_message(&msg, true, network, false, c_VendorId_eProsima);
                break;
            }
            case 1:
            {
                ReaderProxyData rpd(4u, 4u);
                rpd.read_from_cdr_message(&msg, c_VendorId_eProsima);
                break;
            }
            default:
            {
                WriterProxyData wpd(4u, 4u);
                wpd.read_from_cdr_message(&msg, c_VendorId_eProsima);
                break;
            }
        }
    }
    catch (...)
    {
        // Malformed parameter lists / out-of-range lengths throw inside the
        // CDR readers; expected for fuzzed input.
    }

    return 0;
}
