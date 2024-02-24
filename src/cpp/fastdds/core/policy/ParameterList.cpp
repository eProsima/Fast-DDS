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

#include <functional>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include "ParameterList.hpp"
#include "ParameterSerializer.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {


bool ParameterList::writeEncapsulationToCDRMsg(
        fastrtps::rtps::CDRMessage_t* msg)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(msg, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(msg, static_cast<fastrtps::rtps::octet>(PL_CDR_LE - msg->msg_endian));
    valid &= fastrtps::rtps::CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterList::updateCacheChangeFromInlineQos(
        fastrtps::rtps::CacheChange_t& change,
        fastrtps::rtps::CDRMessage_t* msg,
        uint32_t& qos_size)
{
    auto parameter_process = [&](
        fastrtps::rtps::CDRMessage_t* msg,
        const ParameterId_t pid,
        uint16_t plength)
            {
                switch (pid)
                {
                    case PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        change.instanceHandle = p.key;
                        break;
                    }

                    case PID_CUSTOM_RELATED_SAMPLE_IDENTITY:
                    case PID_RELATED_SAMPLE_IDENTITY:
                    {
                        // TODO(eduponz): This check is done here because an implicit fall through rises a warning.
                        // C++17 included a [[fallthrough]] attribute to avoid this kind of warning.
                        if (pid == PID_CUSTOM_RELATED_SAMPLE_IDENTITY)
                        {
                            // Ignore custom PID when coming from other vendors except RTI Connext
                            if ((rtps::c_VendorId_eProsima != change.vendor_id) &&
                                    (rtps::c_VendorId_rti_connext != change.vendor_id))
                            {
                                return true;
                            }
                        }

                        if (plength >= 24)
                        {
                            ParameterSampleIdentity_t p(pid, plength);
                            if (!fastdds::dds::ParameterSerializer<ParameterSampleIdentity_t>::read_from_cdr_message(p,
                                    msg, plength))
                            {
                                return false;
                            }

                            /*
                             * TODO(eduponz): The data from this PID should be used to filled the
                             * related_sample_identity field, not the sample_identity one.
                             * Changing this here implies a behaviour change in the
                             * RTPS layer, so it is postponed until the next major release.
                             */
                            FASTDDS_TODO_BEFORE(3, 0, "Fill related sample identity instead");
                            change.write_params.sample_identity(p.sample_id);
                        }
                        break;
                    }

                    case PID_STATUS_INFO:
                    {
                        ParameterStatusInfo_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterStatusInfo_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        if (p.status == 1)
                        {
                            change.kind = fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED;
                        }
                        else if (p.status == 2)
                        {
                            change.kind = fastrtps::rtps::ChangeKind_t::NOT_ALIVE_UNREGISTERED;
                        }
                        else if (p.status == 3)
                        {
                            change.kind = fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED;
                        }
                        break;
                    }

                    default:
                        break;
                }

                return true;
            };

    return readParameterListfromCDRMsg(*msg, parameter_process, false, qos_size);
}

bool ParameterList::read_guid_from_cdr_msg(
        fastrtps::rtps::CDRMessage_t& msg,
        uint16_t search_pid,
        fastrtps::rtps::GUID_t& guid)
{
    bool valid = false;
    uint16_t pid;
    uint16_t plength;
    while (msg.pos < msg.length)
    {
        valid = true;
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, &pid);
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, &plength);
        if ((pid == PID_SENTINEL) || !valid)
        {
            break;
        }
        if (pid == search_pid)
        {
            valid &= fastrtps::rtps::CDRMessage::readData(&msg, guid.guidPrefix.value,
                            fastrtps::rtps::GuidPrefix_t::size);
            valid &= fastrtps::rtps::CDRMessage::readData(&msg, guid.entityId.value, fastrtps::rtps::EntityId_t::size);
            return valid;
        }
        msg.pos += (plength + 3) & ~3;
    }
    return false;
}

bool ParameterList::readInstanceHandleFromCDRMsg(
        fastrtps::rtps::CacheChange_t* change,
        const uint16_t search_pid)
{
    assert(change != nullptr);

    // Only process data when change does not already have a handle
    if (change->instanceHandle.isDefined())
    {
        return true;
    }

    // Use a temporary wraping message
    fastrtps::rtps::CDRMessage_t msg(change->serializedPayload);

    // Read encapsulation
    msg.pos += 1;
    fastrtps::rtps::octet encapsulation = 0;
    fastrtps::rtps::CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = fastrtps::rtps::Endianness_t::BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = fastrtps::rtps::Endianness_t::LITTLEEND;
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
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, &plength);
        if ((pid == PID_SENTINEL) || !valid)
        {
            break;
        }
        if (pid == PID_KEY_HASH)
        {
            valid &= fastrtps::rtps::CDRMessage::readData(&msg, change->instanceHandle.value, 16);
            return valid;
        }
        if (pid == search_pid)
        {
            valid &= fastrtps::rtps::CDRMessage::readData(&msg, change->instanceHandle.value, 16);
            return valid;
        }
        msg.pos += (plength + 3) & ~3;
    }
    return false;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
