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

#define IF_VALID_CALL() {                                        \
                            if(!valid)                           \
                            {                                    \
                                return false;                    \
                            }                                    \
                            qos_size += plength;                 \
                        }


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
        // Align to 4 byte boundary
        qos_size = (qos_size + 3) & ~3;
        msg->pos = original_pos + qos_size;

        valid = true;
        valid &= CDRMessage::readUInt16(msg, (uint16_t*)&pid);
        valid &= CDRMessage::readUInt16(msg, &plength);
        qos_size += 4;
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
                    if (plength != PARAMETER_STATUS_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterStatusInfo_t p(pid, plength);
                    valid &= p.readFromCDRMessage(msg, plength);
                    IF_VALID_CALL()

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
                    if (plength != PARAMETER_KEY_LENGTH)
                    {
                        return false;
                    }
                    ParameterKey_t p(pid, plength);
                    valid &= p.readFromCDRMessage(msg, plength);
                    IF_VALID_CALL()

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
                    if (plength == 24)
                    {
                        ParameterSampleIdentity_t p(pid, plength);
                        valid &= p.readFromCDRMessage(msg, plength);
                        IF_VALID_CALL()

                        change.write_params.sample_identity(p.sample_id);
                    }
                    else if (plength > 24)
                    {
                        return false;
                    }
                    else
                    {
                        qos_size += plength;
                    }
                    break;
                }
                default:
                {
                    qos_size += plength;
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
