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

#include <fastdds/dds/core/policy/ParameterList.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <functional>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace fastrtps::rtps;

#define IF_VALID_CALL() {                              \
                            if(valid)                  \
                            {                          \
                                qos_size += plength;   \
                                if(!processor(&p))     \
                                    return false;      \
                            }                          \
                            else                       \
                            {                          \
                                return false;          \
                            }                          \
                            break;                     \
                        }

bool ParameterList::writeEncapsulationToCDRMsg(fastrtps::rtps::CDRMessage_t* msg)
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
                    ParameterLocator_t p(pid, plength);
                    valid &= CDRMessage::readLocator(&msg, &p.locator);
                    valid &= (plength == PARAMETER_LOCATOR_LENGTH);
                    IF_VALID_CALL()
                }
                case PID_DEFAULT_UNICAST_PORT:
                case PID_METATRAFFIC_UNICAST_PORT:
                case PID_METATRAFFIC_MULTICAST_PORT:
                {
                    ParameterPort_t p(pid, plength);
                    valid &= CDRMessage::readUInt32(&msg, &p.port);
                    valid &= (plength == PARAMETER_LOCATOR_LENGTH);
                    IF_VALID_CALL()
                }
                case PID_PROTOCOL_VERSION:
                {
                    ParameterProtocolVersion_t p(pid, plength);
                    valid &= CDRMessage::readOctet(&msg, &p.protocolVersion.m_major);
                    valid &= CDRMessage::readOctet(&msg, &p.protocolVersion.m_minor);
                    valid &= (plength == PARAMETER_PROTOCOL_LENGTH);
                    IF_VALID_CALL()
                }
                case PID_EXPECTS_INLINE_QOS:
                {
                    if (plength != PARAMETER_BOOL_LENGTH)
                    {
                        return false;
                    }
                    ParameterBool_t p(PID_EXPECTS_INLINE_QOS, plength);
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.value);
                    IF_VALID_CALL()
                }
                case PID_VENDORID:
                {
                    ParameterVendorId_t p(pid, plength);
                    valid &= CDRMessage::readOctet(&msg, &p.vendorId[0]);
                    valid &= CDRMessage::readOctet(&msg, &p.vendorId[1]);
                    valid &= (plength == PARAMETER_VENDOR_LENGTH);
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
                    valid &= CDRMessage::readData(&msg, p.address, 4);
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
                    valid &= CDRMessage::readData(&msg, p.guid.guidPrefix.value, 12);
                    valid &= CDRMessage::readData(&msg, p.guid.entityId.value, 4);
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
                    fastrtps::string_255 aux;
                    valid &= CDRMessage::readString(&msg, &aux);
                    p.setName(aux.c_str());
                    //                cout << "READ: "<< p.m_string<<endl;
                    //                cout << msg.pos << endl;
                    IF_VALID_CALL()
                }
                case PID_PROPERTY_LIST:
                {
                    uint32_t length_diff = 0;
                    uint32_t pos_ref = 0;
                    ParameterPropertyList_t p(pid, plength);
                    uint32_t num_properties;
                    valid &= CDRMessage::readUInt32(&msg, &num_properties);
                    if (!valid)
                    {
                        return false;
                    }

                    length_diff += 4;
                    std::string str;
                    std::pair<std::string, std::string> pair;
                    for (uint32_t n_prop = 0; n_prop < num_properties; ++n_prop)
                    {
                        pos_ref = msg.pos;
                        pair.first.clear();
                        valid &= CDRMessage::readString(&msg, &pair.first);
                        if (!valid)
                        {
                            return false;
                        }
                        length_diff += msg.pos - pos_ref;
                        pos_ref = msg.pos;
                        pair.second.clear();
                        valid &= CDRMessage::readString(&msg, &pair.second);
                        if (!valid)
                        {
                            return false;
                        }
                        length_diff += msg.pos - pos_ref;
                        p.properties.push_back(pair);
                    }
                    qos_size += plength;
                    if (plength == length_diff)
                    {
                        if(!processor(&p)) return false;
                    }
                    break;
                }
                case PID_STATUS_INFO:
                {
                    if (plength != PARAMETER_STATUS_INFO_LENGTH)
                    {
                        return false;
                    }
                    octet status = msg.buffer[msg.pos + 3];
                    ParameterStatusInfo_t p(pid, plength, status);
                    qos_size += plength;
                    if(!processor(&p)) return false;
                    break;
                }
                case PID_KEY_HASH:
                {
                    if (plength != 16)
                    {
                        return false;
                    }
                    ParameterKey_t p(PID_KEY_HASH, 16);
                    valid &= CDRMessage::readData(&msg, p.key.value, 16);
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
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    IF_VALID_CALL()
                }
                case PID_DEADLINE:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    DeadlineQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.period.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.period.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_LATENCY_BUDGET:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LatencyBudgetQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.duration.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.duration.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_LIVELINESS:
                {
                    if (plength != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LivelinessQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    msg.pos += 3;
                    valid &= CDRMessage::readInt32(&msg, &p.lease_duration.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.lease_duration.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_OWNERSHIP:
                {
                    if (plength != PARAMETER_KIND_LENGTH)
                    {
                        return false;
                    }
                    OwnershipQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    IF_VALID_CALL()
                }
                case PID_RELIABILITY:
                {
                    if (plength != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    ReliabilityQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    msg.pos += 3;
                    valid &= CDRMessage::readInt32(&msg, &p.max_blocking_time.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.max_blocking_time.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_DESTINATION_ORDER:
                {
                    if (plength != PARAMETER_KIND_LENGTH)
                    {
                        return false;
                    }
                    DestinationOrderQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    IF_VALID_CALL()
                }
                case PID_USER_DATA:
                {
                    uint32_t length_diff = 0;
                    uint32_t pos_ref = 0;
                    UserDataQosPolicy p;
                    p.length = plength;
                    uint32_t vec_size = 0;
                    valid &= CDRMessage::readUInt32(&msg, &vec_size);
                    if (!valid || msg.pos + vec_size > msg.length)
                    {
                        return false;
                    }
                    length_diff += 4;
                    p.data_vec_.resize(vec_size);
                    pos_ref = msg.pos;
                    valid &= CDRMessage::readData(&msg, p.data_vec_.data(), vec_size);
                    if (valid)
                    {
                        msg.pos += (plength - 4 - vec_size);
                        length_diff += msg.pos - pos_ref;
                        if (plength != length_diff)
                        {
                            return false;
                        }
                        qos_size += plength;
                        if(!processor(&p)) return false;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                }
                case PID_TIME_BASED_FILTER:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    TimeBasedFilterQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.minimum_separation.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.minimum_separation.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_PRESENTATION:
                {
                    if (plength != PARAMETER_PRESENTATION_LENGTH)
                    {
                        return false;
                    }
                    PresentationQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.access_scope);
                    msg.pos += 3;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.coherent_access);
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.ordered_access);
                    IF_VALID_CALL()
                }
                case PID_PARTITION:
                {
                    uint32_t pos_ref = msg.pos;
                    PartitionQosPolicy p;
                    p.length = plength;
                    uint32_t namessize = 0;
                    valid &= CDRMessage::readUInt32(&msg, &namessize);
                    for (uint32_t i = 1; i <= namessize; ++i)
                    {
                        std::string auxstr;
                        valid &= CDRMessage::readString(&msg, &auxstr);

                        if (plength < msg.pos - pos_ref)
                        {
                            return false;
                        }

                        p.names_.push_back(auxstr);
                    }

                    IF_VALID_CALL()
                }
                case PID_TOPIC_DATA:
                {
                    uint32_t length_diff = 0;
                    uint32_t pos_ref = 0;
                    TopicDataQosPolicy p;
                    p.length = plength;
                    pos_ref = msg.pos;
                    valid &= CDRMessage::readOctetVector(&msg, &p.value);
                    length_diff += msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()
                }
                case PID_GROUP_DATA:
                {
                    uint32_t length_diff = 0;
                    uint32_t pos_ref = 0;
                    GroupDataQosPolicy p;
                    p.length = plength;
                    pos_ref = msg.pos;
                    valid &= CDRMessage::readOctetVector(&msg, &p.value);
                    length_diff += msg.pos - pos_ref;
                    if (plength != length_diff)
                    {
                        return false;
                    }
                    IF_VALID_CALL()
                }
                case PID_HISTORY:
                {
                    if (plength != PARAMETER_KIND_LENGTH + 4)
                    {
                        return false;
                    }
                    HistoryQosPolicy p;
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.kind);
                    msg.pos += 3;
                    valid &= CDRMessage::readInt32(&msg, &p.depth);
                    IF_VALID_CALL()
                }
                case PID_DURABILITY_SERVICE:
                {
                    if (plength != PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 16)
                    {
                        return false;
                    }
                    DurabilityServiceQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.service_cleanup_delay.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.service_cleanup_delay.fraction(frac);
                    valid &= CDRMessage::readOctet(&msg, (octet*)&p.history_kind);
                    msg.pos += 3;
                    valid &= CDRMessage::readInt32(&msg, &p.history_depth);
                    valid &= CDRMessage::readInt32(&msg, &p.max_samples);
                    valid &= CDRMessage::readInt32(&msg, &p.max_instances);
                    valid &= CDRMessage::readInt32(&msg, &p.max_samples_per_instance);
                    IF_VALID_CALL()
                }
                case PID_LIFESPAN:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    LifespanQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.duration.seconds);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.duration.fraction(frac);
                    IF_VALID_CALL()
                }
                case PID_OWNERSHIP_STRENGTH:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    OwnershipStrengthQosPolicy p;
                    valid &= CDRMessage::readUInt32(&msg, &p.value);
                    IF_VALID_CALL()
                }
                case PID_RESOURCE_LIMITS:
                {
                    if (plength != 12)
                    {
                        return false;
                    }
                    ResourceLimitsQosPolicy p;
                    valid &= CDRMessage::readInt32(&msg, &p.max_samples);
                    valid &= CDRMessage::readInt32(&msg, &p.max_instances);
                    valid &= CDRMessage::readInt32(&msg, &p.max_samples_per_instance);
                    IF_VALID_CALL()
                }
                case PID_TRANSPORT_PRIORITY:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    TransportPriorityQosPolicy p;
                    valid &= CDRMessage::readUInt32(&msg, &p.value);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterCount_t p(PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT, plength);
                    valid &= CDRMessage::readUInt32(&msg, &p.count);
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
                    valid &= CDRMessage::readUInt32(&msg, &p.endpointSet);
                    IF_VALID_CALL()
                }
                case PID_PARTICIPANT_LEASE_DURATION:
                {
                    if (plength != PARAMETER_TIME_LENGTH)
                    {
                        return false;
                    }
                    ParameterTime_t p(PID_PARTICIPANT_LEASE_DURATION, plength);
                    int32_t sec(0);
                    valid &= CDRMessage::readInt32(&msg, &sec);
                    p.time.seconds(sec);
                    uint32_t frac(0);
                    valid &= CDRMessage::readUInt32(&msg, &frac);
                    p.time.fraction(frac);
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
                    valid &= CDRMessage::readEntityId(&msg, &p.entityId);
                    IF_VALID_CALL()
                }
                case PID_TYPE_MAX_SIZE_SERIALIZED:
                {
                    if (plength != 4)
                    {
                        return false;
                    }
                    ParameterCount_t p(pid, plength);
                    valid &= CDRMessage::readUInt32(&msg, &p.count);
                    IF_VALID_CALL()
                }
                case PID_RELATED_SAMPLE_IDENTITY:
                {
                    if (plength == 24)
                    {
                        ParameterSampleIdentity_t p(pid, plength);
                        valid &= CDRMessage::readData(&msg, p.sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
                        valid &= CDRMessage::readData(&msg, p.sample_id.writer_guid().entityId.value, EntityId_t::size);
                        valid &= CDRMessage::readInt32(&msg, &p.sample_id.sequence_number().high);
                        valid &= CDRMessage::readUInt32(&msg, &p.sample_id.sequence_number().low);
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
                    int16_t temp(0);
                    uint32_t size(0);
                    valid &= CDRMessage::readUInt32(&msg, &size);
                    for (uint32_t i = 0; i < size; ++i)
                    {
                        valid &= CDRMessage::readInt16(&msg, &temp);
                        p.m_value.push_back(static_cast<DataRepresentationId_t>(temp));

                    }
                    if (size % 2 == 1) // Odd, we must read the alignment
                    {
                        valid &= CDRMessage::readInt16(&msg, &temp);
                    }
                    IF_VALID_CALL()
                }
                case PID_TYPE_CONSISTENCY_ENFORCEMENT:
                {
                    if (plength < 2)
                    {
                        return false;
                    }

                    uint16_t uKind(0);
                    octet temp(0);
                    TypeConsistencyEnforcementQosPolicy p;
                    p.m_ignore_sequence_bounds = false;
                    p.m_ignore_string_bounds = false;
                    p.m_ignore_member_names = false;
                    p.m_prevent_type_widening = false;
                    p.m_force_type_validation = false;

                    valid &= CDRMessage::readUInt16(&msg, &uKind);
                    p.m_kind = static_cast<TypeConsistencyKind>(uKind);

                    if (valid && plength >= 3)
                    {
                        valid &= CDRMessage::readOctet(&msg, &temp);
                        p.m_ignore_sequence_bounds = temp == 0 ? false : true;
                    }

                    if (valid && plength >= 4)
                    {
                        valid &= CDRMessage::readOctet(&msg, &temp);
                        p.m_ignore_string_bounds = temp == 0 ? false : true;
                    }

                    if (valid && plength >= 5)
                    {
                        valid &= CDRMessage::readOctet(&msg, &temp);
                        p.m_ignore_member_names = temp == 0 ? false : true;
                    }

                    if (valid && plength >= 6)
                    {
                        valid &= CDRMessage::readOctet(&msg, &temp);
                        p.m_prevent_type_widening = temp == 0 ? false : true;
                    }

                    if (valid && plength >= 7)
                    {
                        valid &= CDRMessage::readOctet(&msg, &temp);
                        p.m_force_type_validation = temp == 0 ? false : true;
                    }

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
                case PID_TYPE_INFORMATION:
                {
                    xtypes::TypeInformation p;
                    valid &= p.readFromCDRMessage(&msg, plength);
                    IF_VALID_CALL()
                }

#if HAVE_SECURITY
                case PID_IDENTITY_TOKEN:
                case PID_PERMISSIONS_TOKEN:
                {
                    ParameterToken_t p(pid, plength);
                    valid &= CDRMessage::readDataHolder(&msg, p.token);
                    IF_VALID_CALL()
                }

                case PID_PARTICIPANT_SECURITY_INFO:
                {
                    if (plength != PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterParticipantSecurityInfo_t p(pid, plength);
                    valid &= CDRMessage::readUInt32(&msg, &p.security_attributes);
                    valid &= CDRMessage::readUInt32(&msg, &p.plugin_security_attributes);
                    IF_VALID_CALL()
                }

                case PID_ENDPOINT_SECURITY_INFO:
                {
                    if (plength != PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
                    {
                        return false;
                    }
                    ParameterEndpointSecurityInfo_t p(pid, plength);
                    valid &= CDRMessage::readUInt32(&msg, &p.security_attributes);
                    valid &= CDRMessage::readUInt32(&msg, &p.plugin_security_attributes);
                    IF_VALID_CALL()
                }
#endif
                case PID_DISABLE_POSITIVE_ACKS:
                {
                    if (plength != PARAMETER_BOOL_LENGTH)
                    {
                        return false;
                    }
                    octet value(0);
                    octet tmp(0);
                    valid &= CDRMessage::readOctet(&msg, &value);
                    valid &= CDRMessage::readOctet(&msg, &tmp);
                    valid &= CDRMessage::readOctet(&msg, &tmp);
                    valid &= CDRMessage::readOctet(&msg, &tmp);

                    DisablePositiveACKsQosPolicy p;
                    p.enabled = (value == 0) ? false : true;
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

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
