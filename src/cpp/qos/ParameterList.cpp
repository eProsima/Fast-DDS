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

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#define IF_VALID_ADD {if(valid){plist->m_parameters.push_back((Parameter_t*)p);paramlist_byte_size += plength;}else{delete(p);return -1;}break;}


bool ParameterList::writeParameterListToCDRMsg(CDRMessage_t* msg, ParameterList_t* plist, bool use_encapsulation)
{
    assert(msg != nullptr);
    assert(plist != nullptr);

    if(use_encapsulation)
    {
        // Set encapsulation
        CDRMessage::addOctet(msg, 0);
        if(msg->msg_endian == BIGEND)
        {
            CDRMessage::addOctet(msg, PL_CDR_BE);
        }
        else
        {
            CDRMessage::addOctet(msg, PL_CDR_LE);
        }
        CDRMessage::addUInt16(msg, 0);
    }

    for(std::vector<Parameter_t*>::iterator it=plist->m_parameters.begin();
            it!=plist->m_parameters.end();++it)
    {
        if(!(*it)->addToCDRMessage(msg))
        {
            return false;
        }
    }
    if(!CDRMessage::addParameterSentinel(msg))
    {
        return false;
    }

    return true;
}

int32_t ParameterList::readParameterListfromCDRMsg(CDRMessage_t*msg, ParameterList_t*plist, CacheChange_t *change,
        bool use_encapsulation)
{
    assert(msg != nullptr);
    assert(plist != nullptr);

    uint32_t paramlist_byte_size = 0;
    bool is_sentinel = false;
    bool valid = true;
    ParameterId_t pid;
    uint16_t plength;

    if(use_encapsulation)
    {
        // Read encapsulation
        msg->pos += 1;
        octet encapsulation = 0;
        CDRMessage::readOctet(msg, &encapsulation);
        if(encapsulation == PL_CDR_BE)
        {
            msg->msg_endian = BIGEND;
        }
        else if(encapsulation == PL_CDR_LE)
        {
            msg->msg_endian = LITTLEEND;
        }
        else
        {
            return -1;
        }
        if(change != NULL)
        {
            change->serializedPayload.encapsulation = (uint16_t)encapsulation;
        }
        // Skip encapsulation options
        msg->pos +=2;
    }


    while(!is_sentinel)
    {
        valid = true;
        valid&=CDRMessage::readUInt16(msg,(uint16_t*)&pid);
        valid&=CDRMessage::readUInt16(msg,&plength);
        paramlist_byte_size +=4;
        if(!valid || msg->pos > msg->length)
        {
            return -1;
        }
        try
        {
            switch(pid)
            {
                case PID_UNICAST_LOCATOR:
                case PID_MULTICAST_LOCATOR:
                case PID_DEFAULT_UNICAST_LOCATOR:
                case PID_DEFAULT_MULTICAST_LOCATOR:
                case PID_METATRAFFIC_UNICAST_LOCATOR:
                case PID_METATRAFFIC_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = new ParameterLocator_t(pid,plength);
                        valid &= CDRMessage::readLocator(msg,&p->locator);
                        if(plength == PARAMETER_LOCATOR_LENGTH && valid)
                        {
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_DEFAULT_UNICAST_PORT:
                case PID_METATRAFFIC_UNICAST_PORT:
                case PID_METATRAFFIC_MULTICAST_PORT:
                    {
                        ParameterPort_t* p = new ParameterPort_t(pid,plength);
                        valid &= CDRMessage::readUInt32(msg,&p->port);
                        if(plength == PARAMETER_LOCATOR_LENGTH && valid)
                        {
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_PROTOCOL_VERSION:
                    {
                        ParameterProtocolVersion_t* p = new ParameterProtocolVersion_t(pid,plength);
                        valid &= CDRMessage::readOctet(msg,&p->protocolVersion.m_major);
                        valid &= CDRMessage::readOctet(msg,&p->protocolVersion.m_minor);
                        msg->pos+=2;
                        if(plength == PARAMETER_PROTOCOL_LENGTH && valid)
                        {
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_EXPECTS_INLINE_QOS:
                    {
                        if(plength != PARAMETER_BOOL_LENGTH)
                        {
                            return -1;
                        }
                        ParameterBool_t * p = new ParameterBool_t(PID_EXPECTS_INLINE_QOS,plength);
                        valid &= CDRMessage::readOctet(msg,(octet*)&p->value);msg->pos+=3;
                        IF_VALID_ADD
                    }
                case PID_VENDORID:
                    {
                        ParameterVendorId_t* p = new ParameterVendorId_t(pid,plength);
                        valid &= CDRMessage::readOctet(msg,&p->vendorId[0]);
                        valid &= CDRMessage::readOctet(msg,&p->vendorId[1]);
                        msg->pos+=2;
                        if(plength == PARAMETER_VENDOR_LENGTH && valid)
                        {
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_MULTICAST_IPADDRESS:
                case PID_DEFAULT_UNICAST_IPADDRESS:
                case PID_METATRAFFIC_UNICAST_IPADDRESS:
                case PID_METATRAFFIC_MULTICAST_IPADDRESS:
                    {
                        ParameterIP4Address_t* p = new ParameterIP4Address_t(pid,plength);
                        if(plength == PARAMETER_IP4_LENGTH)
                        {
                            p->address[0] = msg->buffer[msg->pos];
                            p->address[1] = msg->buffer[msg->pos+1];
                            p->address[2] = msg->buffer[msg->pos+2];
                            p->address[3] = msg->buffer[msg->pos+3];
                            msg->pos +=4;
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_PARTICIPANT_GUID:
                case PID_GROUP_GUID:
                case PID_ENDPOINT_GUID:
                case PID_PERSISTENCE_GUID:
                    {
                        if(plength != PARAMETER_GUID_LENGTH)
                        {
                            return -1;
                        }
                        ParameterGuid_t* p = new ParameterGuid_t(pid,plength);
                        valid &= CDRMessage::readData(msg,p->guid.guidPrefix.value,12);
                        valid &= CDRMessage::readData(msg,p->guid.entityId.value,4);
                        if(valid)
                        {
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_TOPIC_NAME:
                case PID_TYPE_NAME:
                case PID_ENTITY_NAME:
                    {
                        if(plength > 256)
                        {
                            return -1;
                        }
                        ParameterString_t* p = new ParameterString_t(pid,plength);
                        std::string aux;
                        valid &= CDRMessage::readString(msg,&aux);
                        p->setName(aux.c_str());
                        //                cout << "READ: "<< p->m_string<<endl;
                        //                cout << msg->pos << endl;
                        IF_VALID_ADD
                    }
                case PID_PROPERTY_LIST:
                    {
                        uint32_t length_diff = 0;
                        uint32_t pos_ref = 0;
                        ParameterPropertyList_t* p = new ParameterPropertyList_t(pid,plength);
                        uint32_t num_properties;
                        valid&=CDRMessage::readUInt32(msg,&num_properties);
                        if(!valid)
                        {
                            delete(p);
                            return -1;
                        }

                        length_diff += 4;
                        std::string str;
                        std::pair<std::string,std::string> pair;
                        for(uint32_t n_prop =0;n_prop<num_properties;++n_prop)
                        {
                            pos_ref = msg->pos;
                            pair.first.clear();
                            valid &= CDRMessage::readString(msg,&pair.first);
                            if(!valid)
                            {
                                delete(p);
                                return -1;
                            }
                            length_diff += msg->pos-pos_ref;
                            pos_ref = msg->pos;
                            pair.second.clear();
                            valid &= CDRMessage::readString(msg,&pair.second);
                            if(!valid)
                            {
                                delete(p);
                                return -1;
                            }
                            length_diff += msg->pos - pos_ref;
                            p->properties.push_back(pair);
                        }
                        if(plength != length_diff)
                        {
                            delete(p);
                            return -1;
                        }
                        plist->m_parameters.push_back((Parameter_t*)p);
                        paramlist_byte_size += plength;
                        break;
                    }
                case PID_STATUS_INFO:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        octet status = msg->buffer[msg->pos+3];
                        if(change != NULL)
                        {
                            if(status == 1)
                            {
                                change->kind = NOT_ALIVE_DISPOSED;
                            }
                            else if (status == 2)
                            {
                                change->kind = NOT_ALIVE_UNREGISTERED;
                            }
                            else if (status == 3)
                            {
                                change->kind = NOT_ALIVE_DISPOSED_UNREGISTERED;
                            }
                        }
                        msg->pos+=plength;
                        paramlist_byte_size+=plength;
                        break;
                    }
                case PID_KEY_HASH:
                    {
                        ParameterKey_t* p = new ParameterKey_t();
                        p->Pid = PID_KEY_HASH;
                        p->length = 16;
                        valid&=CDRMessage::readData(msg,p->key.value,16);
                        if(change != NULL)
                        {
                            change->instanceHandle = p->key;
                        }
                        IF_VALID_ADD
                    }
                case PID_SENTINEL:
                    {
                        is_sentinel = true;
                        break;
                    }
                case PID_DURABILITY:
                    {
                        if(plength != PARAMETER_KIND_LENGTH)
                        {
                            return -1;
                        }
                        DurabilityQosPolicy* p = new DurabilityQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
                        msg->pos+=3;
                        IF_VALID_ADD
                    }
                case PID_DEADLINE:
                    {
                        if(plength != PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        DeadlineQosPolicy* p= new DeadlineQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->period.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->period.fraction);
                        IF_VALID_ADD
                    }
                case PID_LATENCY_BUDGET:
                    {
                        if(plength != PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        LatencyBudgetQosPolicy* p = new LatencyBudgetQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->duration.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->duration.fraction);
                        IF_VALID_ADD
                    }
                case PID_LIVELINESS:
                    {
                        if(plength != PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        LivelinessQosPolicy* p = new LivelinessQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
                        msg->pos+=3;
                        valid &= CDRMessage::readInt32(msg,&p->lease_duration.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->lease_duration.fraction);
                        IF_VALID_ADD
                    }
                case PID_OWNERSHIP:
                    {
                        if(plength != PARAMETER_KIND_LENGTH)
                        {
                            return -1;
                        }
                        OwnershipQosPolicy* p = new OwnershipQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
                        msg->pos+=3;
                        IF_VALID_ADD
                    }
                case PID_RELIABILITY:
                    {
                        if(plength != PARAMETER_KIND_LENGTH+PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        ReliabilityQosPolicy* p = new ReliabilityQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
                        msg->pos+=3;
                        valid &= CDRMessage::readInt32(msg,&p->max_blocking_time.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->max_blocking_time.fraction);
                        IF_VALID_ADD
                    }
                case PID_DESTINATION_ORDER:
                    {
                        if(plength != PARAMETER_KIND_LENGTH)
                        {
                            return -1;
                        }
                        DestinationOrderQosPolicy* p = new DestinationOrderQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
                        msg->pos+=3;
                        IF_VALID_ADD
                    }
                case PID_USER_DATA:
                    {
                        uint32_t length_diff = 0;
                        uint32_t pos_ref = 0;
                        UserDataQosPolicy* p = new UserDataQosPolicy();
                        p->length = plength;
                        uint32_t vec_size = 0;
                        valid&=CDRMessage::readUInt32(msg,&vec_size);
                        if(!valid || msg->pos+vec_size > msg->length)
                        {
                            delete(p);
                            return -1;
                        }
                        length_diff += 4;
                        p->dataVec.resize(vec_size);
                        pos_ref = msg->pos;
                        valid &= CDRMessage::readData(msg,p->dataVec.data(),vec_size);
                        if(valid)
                        {
                            msg->pos += (plength - 4 - vec_size);
                            length_diff += msg->pos - pos_ref;
                            if(plength != length_diff)
                            {
                                delete(p);
                                return -1;
                            }
                            plist->m_parameters.push_back((Parameter_t*)p);
                            paramlist_byte_size += plength;
                        }
                        else
                        {
                            delete(p);
                            return -1;
                        }
                        break;
                    }
                case PID_TIME_BASED_FILTER:
                    {
                        if(plength != PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        TimeBasedFilterQosPolicy* p = new TimeBasedFilterQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->minimum_separation.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->minimum_separation.fraction);
                        IF_VALID_ADD
                    }
                case PID_PRESENTATION:
                    {
                        if(plength != PARAMETER_PRESENTATION_LENGTH)
                        {
                            return -1;
                        }
                        PresentationQosPolicy* p = new PresentationQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->access_scope);
                        msg->pos+=3;
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->coherent_access);
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->ordered_access);
                        msg->pos+=2;
                        IF_VALID_ADD
                    }
                case PID_PARTITION:
                    {
                        uint32_t pos_ref = msg->pos;
                        PartitionQosPolicy * p = new PartitionQosPolicy();
                        p->length = plength;
                        uint32_t namessize = 0;
                        valid &= CDRMessage::readUInt32(msg,&namessize);
                        for(uint32_t i = 1;i<=namessize;++i)
                        {
                            std::string auxstr;
                            valid &= CDRMessage::readString(msg,&auxstr);

                            if(plength < msg->pos - pos_ref)
                            {
                                delete(p);
                                return -1;
                            }

                            p->names.push_back(auxstr);
                        }

                        IF_VALID_ADD
                    }
                case PID_TOPIC_DATA:
                    {
                        uint32_t length_diff = 0;
                        uint32_t pos_ref = 0;
                        TopicDataQosPolicy * p = new TopicDataQosPolicy();
                        p->length = plength;
                        pos_ref = msg->pos;
                        valid &= CDRMessage::readOctetVector(msg,&p->value);
                        length_diff += msg->pos - pos_ref;
                        if(plength != length_diff)
                        {
                            delete(p);
                            return -1;
                        }
                        IF_VALID_ADD
                    }
                case PID_GROUP_DATA:
                    {
                        uint32_t length_diff = 0;
                        uint32_t pos_ref = 0;
                        GroupDataQosPolicy * p = new GroupDataQosPolicy();
                        p->length = plength;
                        pos_ref = msg->pos;
                        valid &= CDRMessage::readOctetVector(msg,&p->value);
                        length_diff += msg->pos - pos_ref;
                        if(plength != length_diff)
                        {
                            delete(p);
                            return -1;
                        }
                        IF_VALID_ADD
                    }
                case PID_HISTORY:
                    {
                        if(plength != PARAMETER_KIND_LENGTH+4)
                        {
                            return -1;
                        }
                        HistoryQosPolicy* p = new HistoryQosPolicy();
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);msg->pos+=3;
                        valid &= CDRMessage::readInt32(msg,&p->depth);
                        IF_VALID_ADD
                    }
                case PID_DURABILITY_SERVICE:
                    {
                        if(plength != PARAMETER_TIME_LENGTH+PARAMETER_KIND_LENGTH+16)
                        {
                            return -1;
                        }
                        DurabilityServiceQosPolicy * p = new DurabilityServiceQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->service_cleanup_delay.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->service_cleanup_delay.fraction);
                        valid&=CDRMessage::readOctet(msg,(octet*)&p->history_kind);msg->pos+=3;
                        valid &= CDRMessage::readInt32(msg,&p->history_depth);
                        valid &= CDRMessage::readInt32(msg,&p->max_samples);
                        valid &= CDRMessage::readInt32(msg,&p->max_instances);
                        valid &= CDRMessage::readInt32(msg,&p->max_samples_per_instance);
                        IF_VALID_ADD
                    }
                case PID_LIFESPAN:
                    {
                        if(plength != PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        LifespanQosPolicy * p = new LifespanQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->duration.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->duration.fraction);
                        IF_VALID_ADD
                    }
                case PID_OWNERSHIP_STRENGTH:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        OwnershipStrengthQosPolicy * p = new OwnershipStrengthQosPolicy();
                        valid &= CDRMessage::readUInt32(msg,&p->value);
                        IF_VALID_ADD
                    }
                case PID_RESOURCE_LIMITS:
                    {
                        if(plength != 12)
                        {
                            return -1;
                        }
                        ResourceLimitsQosPolicy* p = new ResourceLimitsQosPolicy();
                        valid &= CDRMessage::readInt32(msg,&p->max_samples);
                        valid &= CDRMessage::readInt32(msg,&p->max_instances);
                        valid &= CDRMessage::readInt32(msg,&p->max_samples_per_instance);
                        IF_VALID_ADD
                    }
                case PID_TRANSPORT_PRIORITY:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        TransportPriorityQosPolicy * p = new TransportPriorityQosPolicy();
                        valid &= CDRMessage::readUInt32(msg,&p->value);
                        IF_VALID_ADD
                    }
                case PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        ParameterCount_t*p = new ParameterCount_t(PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT,plength);
                        valid&=CDRMessage::readUInt32(msg,&p->count);
                        IF_VALID_ADD
                    }
                case PID_PARTICIPANT_BUILTIN_ENDPOINTS:
                case PID_BUILTIN_ENDPOINT_SET:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        ParameterBuiltinEndpointSet_t * p = new ParameterBuiltinEndpointSet_t(pid,plength);
                        valid &= CDRMessage::readUInt32(msg,&p->endpointSet);
                        IF_VALID_ADD
                    }
                case PID_PARTICIPANT_LEASE_DURATION:
                    {
                        if(plength != PARAMETER_TIME_LENGTH)
                        {
                            return -1;
                        }
                        ParameterTime_t* p = new ParameterTime_t(PID_PARTICIPANT_LEASE_DURATION,plength);
                        valid &= CDRMessage::readInt32(msg,&p->time.seconds);
                        valid &= CDRMessage::readUInt32(msg,&p->time.fraction);
                        IF_VALID_ADD
                    }
                case PID_CONTENT_FILTER_PROPERTY:
                    {
                        if (plength > msg->length-msg->pos)
                        {
                            return -1;
                        }
                        msg->pos += plength;
                        paramlist_byte_size += plength;
                        break;
                    }
                case PID_PARTICIPANT_ENTITYID:
                case PID_GROUP_ENTITYID:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        ParameterEntityId_t * p = new ParameterEntityId_t(pid,plength);
                        valid &= CDRMessage::readEntityId(msg,&p->entityId);
                        IF_VALID_ADD
                    }
                case PID_TYPE_MAX_SIZE_SERIALIZED:
                    {
                        if(plength != 4)
                        {
                            return -1;
                        }
                        ParameterCount_t * p = new ParameterCount_t(pid,plength);
                        valid &= CDRMessage::readUInt32(msg,&p->count);
                        IF_VALID_ADD
                    }
                case PID_RELATED_SAMPLE_IDENTITY:
                    {
                        if(plength == 24)
                        {
                            ParameterSampleIdentity_t *p = new ParameterSampleIdentity_t(pid, plength);
                            valid &= CDRMessage::readData(msg, p->sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
                            valid &= CDRMessage::readData(msg, p->sample_id.writer_guid().entityId.value, EntityId_t::size);
                            valid &= CDRMessage::readInt32(msg, &p->sample_id.sequence_number().high);
                            valid &= CDRMessage::readUInt32(msg, &p->sample_id.sequence_number().low);

                            if(change != NULL)
                            {
                                change->write_params.sample_identity(p->sample_id);
                            }
                            IF_VALID_ADD
                        }
                        else if(plength > msg->length-msg->pos || plength > 24)
                        {
                            return -1;
                        }
                        else
                        {
                            msg->pos +=plength;
                            paramlist_byte_size +=plength;
                            break;
                        }
                    }
                case PID_IDENTITY_TOKEN:
                    {
                        ParameterToken_t* p = new ParameterToken_t(pid, plength);
                        valid &= CDRMessage::readDataHolder(msg, p->token);
                        msg->pos += (4 - msg->pos % 4) & 3; //align
                        IF_VALID_ADD
                    }
                case PID_PAD:
                default:
                    {
                        if (plength > msg->length-msg->pos)
                        {
                            return -1;
                        }
                        msg->pos +=plength;
                        paramlist_byte_size +=plength;
                        break;
                    }
            }
        }
        catch (std::bad_alloc& ba)
        {
            std::cerr << "bad_alloc caught: " << ba.what() << '\n';
            return -1;
        }
    }
    return paramlist_byte_size;
}

bool ParameterList::readInstanceHandleFromCDRMsg(CacheChange_t* change, const uint16_t search_pid)
{
    // Only process data when change does not already have a handle
    if (change->instanceHandle.isDefined())
    {
        return true;
    }

    // Use a temporary wraping message
    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = change->serializedPayload.data;
    msg.length = change->serializedPayload.length;

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
    if (change != NULL)
    {
        change->serializedPayload.encapsulation = (uint16_t)encapsulation;
    }
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
        if ( (pid == PID_SENTINEL) || !valid)
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
