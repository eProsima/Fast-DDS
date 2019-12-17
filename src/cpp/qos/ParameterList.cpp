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
                            if(valid)                            \
                            {                                    \
                                qos_size += plength;             \
                                if(!processor(&p))               \
                                    return false;                \
                            }                                    \
                            else                                 \
                            {                                    \
                                return false;                    \
                            }                                    \
                            break;                               \
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
    auto parameter_process = [&](const Parameter_t* p)
    {
        switch (p->Pid)
        {
            case PID_KEY_HASH:
            {
                const ParameterKey_t* p_key = dynamic_cast<const ParameterKey_t*>(p);
                assert(p_key != nullptr);
                change.instanceHandle = p_key->key;
                break;
            }

            case PID_RELATED_SAMPLE_IDENTITY:
            {
                const ParameterSampleIdentity_t* p_id = dynamic_cast<const ParameterSampleIdentity_t*>(p);
                assert(p_id != nullptr);
                change.write_params.sample_identity(p_id->sample_id);
                break;
            }

            case PID_STATUS_INFO:
            {
                const ParameterStatusInfo_t* p_status = dynamic_cast<const ParameterStatusInfo_t*>(p);
                assert(p_status != nullptr);
                if (p_status->status == 1)
                {
                    change.kind = NOT_ALIVE_DISPOSED;
                }
                else if (p_status->status == 2)
                {
                    change.kind = NOT_ALIVE_UNREGISTERED;
                }
                else if (p_status->status == 3)
                {
                    change.kind = NOT_ALIVE_DISPOSED_UNREGISTERED;
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

bool ParameterList::readParameterListfromCDRMsg(CDRMessage_t& msg, std::function<bool(const Parameter_t*)> processor,
    bool use_encapsulation, uint32_t& qos_size)
{
    bool is_sentinel = false;
    bool valid = true;
    ParameterId_t pid;
    uint16_t plength;
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
    while (!is_sentinel)
    {
        // Align to 4 byte boundary
        qos_size = (qos_size + 3) & ~3;
        msg.pos = original_pos + qos_size;

        valid = true;
        valid &= CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
        valid &= CDRMessage::readUInt16(&msg, &plength);
        qos_size += 4;
        if (!valid || ((msg.pos + plength) > msg.length))
        {
            return false;
        }
        try
        {
            switch (pid)
            {
                case PID_UNICAST_LOCATOR:
                case PID_MULTICAST_LOCATOR:
                case PID_DEFAULT_UNICAST_LOCATOR:
                case PID_DEFAULT_MULTICAST_LOCATOR:
                case PID_METATRAFFIC_UNICAST_LOCATOR:
                case PID_METATRAFFIC_MULTICAST_LOCATOR:
                {
                    valid &= (plength == PARAMETER_LOCATOR_LENGTH);
                    ParameterLocator_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_DEFAULT_UNICAST_PORT:
                case PID_METATRAFFIC_UNICAST_PORT:
                case PID_METATRAFFIC_MULTICAST_PORT:
                {
                    valid &= (plength == PARAMETER_LOCATOR_LENGTH);
                    ParameterPort_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PROTOCOL_VERSION:
                {
                    valid &= (plength == PARAMETER_PROTOCOL_LENGTH);
                    ParameterProtocolVersion_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_EXPECTS_INLINE_QOS:
                {
                    if (plength != PARAMETER_BOOL_LENGTH)
                    {
                        return false;
                    }
                    ParameterBool_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_VENDORID:
                {
                    valid &= (plength == PARAMETER_VENDOR_LENGTH);
                    ParameterVendorId_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_MULTICAST_IPADDRESS:
                case PID_DEFAULT_UNICAST_IPADDRESS:
                case PID_METATRAFFIC_UNICAST_IPADDRESS:
                case PID_METATRAFFIC_MULTICAST_IPADDRESS:
                {
                    if (plength != PARAMETER_IP4_LENGTH)
                    {
                        return false;
                    }
                    ParameterIP4Address_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_GUID:
                case PID_GROUP_GUID:
                case PID_ENDPOINT_GUID:
                case PID_PERSISTENCE_GUID:
                {
                    if (plength != PARAMETER_GUID_LENGTH)
                    {
                        return false;
                    }
                    ParameterGuid_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TOPIC_NAME:
                case PID_TYPE_NAME:
                case PID_ENTITY_NAME:
                {
                    if (plength > 256)
                    {
                        return false;
                    }
                    ParameterString_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PROPERTY_LIST:
                {
                    ParameterPropertyList_t p(pid, plength);
                    uint32_t pos_ref = msg.pos;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    uint32_t length_diff = msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()
                }
                case PID_STATUS_INFO:
                {
                    if (plength != PARAMETER_STATUS_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterStatusInfo_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_KEY_HASH:
                {
                    if (plength != PARAMETER_KEY_LENGTH)
                    {
                        return false;
                    }
                    ParameterKey_t p(PID_KEY_HASH, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_SENTINEL:
                {
                    is_sentinel = true;
                    break;
                }
                case PID_DURABILITY:
                {
                    if (plength != PARAMETER_KIND_LENGTH)
                    {
                        return false;
                    }
                    DurabilityQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_DEADLINE:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    DeadlineQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_LATENCY_BUDGET:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LatencyBudgetQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_LIVELINESS:
                {
                    if (plength != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LivelinessQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_OWNERSHIP:
                {
                    if (plength != PARAMETER_KIND_LENGTH)
                    {
                        return false;
                    }
                    OwnershipQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_RELIABILITY:
                {
                    if (plength != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    ReliabilityQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_DESTINATION_ORDER:
                {
                    if (plength != PARAMETER_KIND_LENGTH)
                    {
                        return false;
                    }
                    DestinationOrderQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_USER_DATA:
                {
                    UserDataQosPolicy p(plength);
                    uint32_t pos_ref = msg.pos;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    uint32_t length_diff = msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()
                }
                case PID_TIME_BASED_FILTER:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    TimeBasedFilterQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PRESENTATION:
                {
                    if (plength != PARAMETER_PRESENTATION_LENGTH)
                    {
                        return false;
                    }
                    PresentationQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PARTITION:
                {
                    PartitionQosPolicy p(plength);
                    uint32_t pos_ref = msg.pos;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    uint32_t length_diff = msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()                }
                case PID_TOPIC_DATA:
                {
                    TopicDataQosPolicy p(plength);
                    uint32_t pos_ref = msg.pos;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    uint32_t length_diff = msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()                }
                case PID_GROUP_DATA:
                {
                    GroupDataQosPolicy p(plength);
                    uint32_t pos_ref = msg.pos;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    uint32_t length_diff = msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()                }
                case PID_HISTORY:
                {
                    if (plength != PARAMETER_KIND_LENGTH + 4)
                    {
                        return false;
                    }
                    HistoryQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_DURABILITY_SERVICE:
                {
                    if (plength != PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 16)
                    {
                        return false;
                    }
                    DurabilityServiceQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_LIFESPAN:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LifespanQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_OWNERSHIP_STRENGTH:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    OwnershipStrengthQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_RESOURCE_LIMITS:
                {
                    if (plength != 12)
                    {
                        return false;
                    }
                    ResourceLimitsQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TRANSPORT_PRIORITY:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    TransportPriorityQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterCount_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_BUILTIN_ENDPOINTS:
                case PID_BUILTIN_ENDPOINT_SET:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterBuiltinEndpointSet_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_LEASE_DURATION:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    ParameterTime_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_CONTENT_FILTER_PROPERTY:
                {
                    qos_size += plength;
                    break;
                }
                case PID_PARTICIPANT_ENTITYID:
                case PID_GROUP_ENTITYID:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterEntityId_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TYPE_MAX_SIZE_SERIALIZED:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterCount_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_RELATED_SAMPLE_IDENTITY:
                {
                    if (plength == 24)
                    {
                        ParameterSampleIdentity_t p(pid, plength);
                        valid &= p.readFromCDRMessage(&msg, plength);
                        IF_VALID_CALL()
                    }
                    else if (plength > 24)
                    {
                        return false;
                    }
                    else
                    {
                        qos_size += plength;
                        break;
                    }
                }

                case PID_DATA_REPRESENTATION:
                {
                    DataRepresentationQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TYPE_CONSISTENCY_ENFORCEMENT:
                {
                    if (plength < 2)
                    {
                        return false;
                    }
                    TypeConsistencyEnforcementQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TYPE_IDV1:
                {
                    TypeIdV1 p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
                case PID_TYPE_OBJECTV1:
                {
                    TypeObjectV1 p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }

#if HAVE_SECURITY
                case PID_IDENTITY_TOKEN:
                case PID_PERMISSIONS_TOKEN:
                {
                    ParameterToken_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }

                case PID_PARTICIPANT_SECURITY_INFO:
                {
                    if (plength != PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterParticipantSecurityInfo_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }

                case PID_ENDPOINT_SECURITY_INFO:
                {
                    if (plength != PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterEndpointSecurityInfo_t p(pid, plength);
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }
#endif
                case PID_DISABLE_POSITIVE_ACKS:
                {
                    if (plength != PARAMETER_BOOL_LENGTH)
                    {
                        return false;
                    }
                    DisablePositiveACKsQosPolicy p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }

                case PID_PAD:
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
