// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParamListt.cpp
 *
 */

#include <fastrtps/qos/ParameterList.h>
#include <fastrtps/qos/QosPolicies.h>

#include <functional>
#include <stdexcept>

namespace eprosima {
namespace fastrtps {

using namespace rtps;


bool ParameterList::writeEncapsulationToCDRMsg(rtps::CDRMessage_t* msg)
{
    bool valid = CDRMessage::addOctet(msg, 0);
    if (msg->msg_endian == BIGEND)
    {
        valid &= CDRMessage::addOctet(msg, PL_CDR_BE);
    }
    else
    {
        valid &= CDRMessage::addOctet(msg, PL_CDR_LE);
    }
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterList::updateCacheChangeFromInlineQos(CacheChange_t& change, CDRMessage_t* msg, uint32_t& qos_size)
{
    bool is_sentinel = false;
    bool valid = true;
    ParameterId_t pid;
    uint16_t plength;
    qos_size = 0;

    uint32_t original_pos = msg->pos;
    while (!is_sentinel)
    {
        msg->pos = original_pos + qos_size;

        valid = true;
        valid &= CDRMessage::readUInt16(msg, (uint16_t*)&pid);
        valid &= CDRMessage::readUInt16(msg, &plength);
        qos_size += (4 + plength);

        // Align to 4 byte boundary and prepare for next iteration
        qos_size = (qos_size + 3) & ~3;

        if (!valid || ((msg->pos + plength) > msg->length))
        {
            return false;
        }
        try
        {
            switch (pid)
            {
                case PID_STATUS_INFO:
                {
                    ParameterStatusInfo_t p(pid, plength);
                    if (!p.readFromCDRMessage(msg, plength))
                    {
                        return false;
                    }

                    if (p.status == 1)
                    {
                        change.kind = NOT_ALIVE_DISPOSED;
                    }
                    else if (p.status == 2)
                    {
                        change.kind = NOT_ALIVE_UNREGISTERED;
                    }
                    else if (p.status == 3)
                    {
                        change.kind = NOT_ALIVE_DISPOSED_UNREGISTERED;
                    }
                    break;
                }
                case PID_KEY_HASH:
                {
                    ParameterKey_t p(pid, plength);
                    if (!p.readFromCDRMessage(msg, plength))
                    {
                        return false;
                    }

                    change.instanceHandle = p.key;
                    break;
                }
                case PID_SENTINEL:
                {
                    is_sentinel = true;
                    break;
                }
                case PID_RELATED_SAMPLE_IDENTITY:
                {
                    if (plength < 24)
                    {
                        continue;
                    }

                    ParameterSampleIdentity_t p(pid, plength);
                    if (!p.readFromCDRMessage(msg, plength))
                    {
                        return false;
                    }

                    change.write_params.sample_identity(p.sample_id);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        catch (std::bad_alloc& ba)
        {
            std::cerr << "bad_alloc caught: " << ba.what() << '\n';
            return false;
        }
    }
    return true;
}

bool ParameterList::readParameterListfromCDRMsg(
        rtps::CDRMessage_t& msg,
        std::function<bool(rtps::CDRMessage_t*, const ParameterId_t, uint16_t)> processor,
        bool use_encapsulation,
        uint32_t& qos_size)
{
    qos_size = 0;

    if (use_encapsulation)
    {
        // Read encapsulation
        msg.pos += 1;
        octet encapsulation = 0;
        CDRMessage::readOctet(&msg, &encapsulation);
        if (encapsulation == PL_CDR_BE)
        {
            msg.msg_endian = BIGEND;
        }
        else if (encapsulation == PL_CDR_LE)
        {
            msg.msg_endian = LITTLEEND;
        }
        else
        {
            return false;
        }
        // Skip encapsulation options
        msg.pos += 2;
    }

    uint32_t original_pos = msg.pos;
    bool is_sentinel = false;
    while (!is_sentinel)
    {
        msg.pos = original_pos + qos_size;

        ParameterId_t pid;
        uint16_t plength;
        bool valid = true;
        valid &= CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
        valid &= CDRMessage::readUInt16(&msg, &plength);
        qos_size += (4 + plength);

        // Align to 4 byte boundary and prepare for next iteration
        qos_size = (qos_size + 3) & ~3;

        if (!valid || ((msg.pos + plength) > msg.length))
        {
            return false;
        }

        if (pid == PID_SENTINEL)
        {
            is_sentinel = true;
        }
        else if (!processor(&msg, pid, plength))
        {
            return false;
        }
    }
    return true;
}

bool ParameterList::readInstanceHandleFromCDRMsg(CacheChange_t* change, const uint16_t search_pid)
{
    assert(change != nullptr);

    // Only process data when change does not already have a handle
    if (change->instanceHandle.isDefined())
    {
        return true;
    }

    // Use a temporary wraping message
    CDRMessage_t msg(change->serializedPayload);

    // Read encapsulation
    msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = LITTLEEND;
    }
    else
    {
        return false;
    }

    change->serializedPayload.encapsulation = (uint16_t)encapsulation;

    // Skip encapsulation options
    msg.pos += 2;

    bool valid = false;
    uint16_t pid;
    uint16_t plength;
    while (msg.pos < msg.length)
    {
        valid = true;
        valid &= CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
        valid &= CDRMessage::readUInt16(&msg, &plength);
        if ((pid == PID_SENTINEL) || !valid)
        {
            break;
        }
        if (pid == PID_KEY_HASH)
        {
            valid &= CDRMessage::readData(&msg, change->instanceHandle.value, 16);
            return valid;
        }
        if (pid == search_pid)
        {
            valid &= CDRMessage::readData(&msg, change->instanceHandle.value, 16);
            return valid;
        }
        msg.pos += plength;
    }
    return false;
}

}  // namespace fastrtps
}  // namespace eprosima
