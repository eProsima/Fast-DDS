/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParamListt.cpp
 *
 */

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/DDSQosPolicies.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace dds {

#define IF_VALID_ADD {if(valid){plist->m_parameters.push_back((Parameter_t*)p);plist->m_hasChanged = true;paramlist_byte_size += plength;}else{delete(p);return -1;}break;}



bool ParameterList::updateCDRMsg(ParameterList_t* plist,Endianness_t endian)
{
	CDRMessage::initCDRMsg(&plist->m_cdrmsg);
	plist->m_cdrmsg.msg_endian = endian;
	for(std::vector<Parameter_t*>::iterator it=plist->m_parameters.begin();
			it!=plist->m_parameters.end();++it)
	{
		if(!(*it)->addToCDRMessage(&plist->m_cdrmsg))
		{
			return false;
		}
	}
	if(!CDRMessage::addParameterSentinel(&plist->m_cdrmsg))
	{
		return false;
	}
	else
	{
		plist->m_hasChanged = false;
		pDebugInfo("Param msg created correctly"<<endl);
		return true;
	}
}


uint32_t ParameterList::readParameterListfromCDRMsg(CDRMessage_t*msg,ParameterList_t*plist,InstanceHandle_t* handle,ChangeKind_t* chkind)
{
	uint32_t paramlist_byte_size = 0;
	bool is_sentinel = false;
	bool valid = true;
	ParameterId_t pid;
	uint16_t plength;
	while(!is_sentinel)
	{
		valid = true;
		valid&=CDRMessage::readUInt16(msg,(uint16_t*)&pid);
		valid&=CDRMessage::readUInt16(msg,&plength);
		paramlist_byte_size +=4;
		if(valid)
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
					plist->m_hasChanged = true;
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
					plist->m_hasChanged = true;
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
					plist->m_hasChanged = true;
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
					plist->m_hasChanged = true;
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
					plist->m_hasChanged = true;
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
			{
				ParameterGuid_t* p = new ParameterGuid_t(pid,plength);
				valid &= CDRMessage::readData(msg,p->guid.guidPrefix.value,12);
				valid &= CDRMessage::readData(msg,p->guid.entityId.value,4);
				if(plength == PARAMETER_GUID_LENGTH && valid)
				{
					plist->m_parameters.push_back((Parameter_t*)p);
					plist->m_hasChanged = true;
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
//				cout << msg->pos << endl;
				ParameterString_t* p = new ParameterString_t(pid,plength);
				valid &= CDRMessage::readString(msg,&p->m_string);
//				cout << "READ: "<< p->m_string<<endl;
//				cout << msg->pos << endl;
				IF_VALID_ADD
			}
			case PID_PROPERTY_LIST:
			{
				ParameterPropertyList_t* p = new ParameterPropertyList_t(pid,plength);
				uint32_t num_properties;
				valid&=CDRMessage::readUInt32(msg,&num_properties);
								//uint16_t msg_pos_first = msg->pos;
				std::string str;
				std::pair<std::string,std::string> pair;
				//uint32_t rest=0;
				for(uint32_t n_prop =0;n_prop<num_properties;++n_prop)
				{
					pair.first.clear();
					valid &= CDRMessage::readString(msg,&pair.first);
					pair.second.clear();
					valid &= CDRMessage::readString(msg,&pair.second);
//					//STRING 1
//					uint32_t str_size = 1;
//					valid&=CDRMessage::readUInt32(msg,&str_size);
//
//					str = std::string();str.resize(str_size);
//					octet* oc1 = new octet[str_size];
//					valid &= CDRMessage::readData(msg,oc1,str_size);
//					for(uint32_t i =0;i<str_size;i++)
//						str.at(i) = oc1[i];
//					pair.first = str;
//					rest = (uint32_t)(str_size-4*floor((float)str_size/4));
//					rest = rest==0 ? 0 : 4-rest;
//					msg->pos+=rest;
//					//STRING 2
//					valid&=CDRMessage::readUInt32(msg,&str_size);
//					str = std::string();str.resize(str_size);
//
//					octet* oc2 = new octet[str_size];
//					valid &= CDRMessage::readData(msg,oc2,str_size);
//					for(uint32_t i =0;i<str_size;i++)
//						str.at(i) = oc2[i];
//					pair.second = str;
//					rest = (uint32_t)(str_size-4*floor((float)str_size/4));
//										rest = rest==0 ? 0 : 4-rest;
//										msg->pos+=rest;
					p->properties.push_back(pair);
//					delete(oc1);
//					delete(oc2);
				}
				if(valid)
				{

					plist->m_parameters.push_back((Parameter_t*)p);
					plist->m_hasChanged = true;
					paramlist_byte_size += plength;
				}
				else
				{
					delete(p);
					return -1;
				}
				break;
				break;
			}
			case PID_STATUS_INFO:
			{
				octet status = msg->buffer[msg->pos+3];
				if(status == 1)
					*chkind = NOT_ALIVE_DISPOSED;
				else if (status == 2)
					*chkind = NOT_ALIVE_UNREGISTERED;
				msg->pos+=4;
				paramlist_byte_size+=4;
				break;
			}
			case PID_KEY_HASH:
			{
      			ParameterKey_t* p = new ParameterKey_t();
				p->Pid = PID_KEY_HASH;
				p->length = 16;
				valid&=CDRMessage::readData(msg,p->key.value,16);
				if(handle!=NULL)
					*handle = p->key;
				IF_VALID_ADD
			}
			case PID_SENTINEL:
			{
				is_sentinel = true;
				break;
			}
			case PID_DURABILITY:
			{
				DurabilityQosPolicy* p = new DurabilityQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
				msg->pos+=3;
				IF_VALID_ADD
			}
			case PID_DEADLINE:
			{
				DeadlineQosPolicy* p= new DeadlineQosPolicy();
				valid &= CDRMessage::readInt32(msg,&p->period.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->period.fraction);
				IF_VALID_ADD
			}
			case PID_LATENCY_BUDGET:
			{
				LatencyBudgetQosPolicy* p = new LatencyBudgetQosPolicy();
				valid &= CDRMessage::readInt32(msg,&p->duration.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->duration.fraction);
				IF_VALID_ADD
			}
			case PID_LIVELINESS:
			{
				LivelinessQosPolicy* p = new LivelinessQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
				msg->pos+=3;
				valid &= CDRMessage::readInt32(msg,&p->lease_duration.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->lease_duration.fraction);
				IF_VALID_ADD
			}
			case PID_OWNERSHIP:
			{
				OwnershipQosPolicy* p = new OwnershipQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
				msg->pos+=3;
				IF_VALID_ADD
			}
			case PID_RELIABILITY:
			{
				ReliabilityQosPolicy* p = new ReliabilityQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
				msg->pos+=3;
				valid &= CDRMessage::readInt32(msg,&p->max_blocking_time.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->max_blocking_time.fraction);
				IF_VALID_ADD
			}
			case PID_DESTINATION_ORDER:
			{
				DestinationOrderQosPolicy* p = new DestinationOrderQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);
				msg->pos+=3;
				IF_VALID_ADD
			}
			case PID_USER_DATA:
			{
				UserDataQosPolicy* p = new UserDataQosPolicy();
				p->length = plength;
				uint32_t str_size = 1;
				valid&=CDRMessage::readUInt32(msg,&str_size);
				p->data.resize(str_size);
				octet* oc=new octet[str_size];
				valid &= CDRMessage::readData(msg,oc,str_size);
				for(uint32_t i =0;i<str_size;i++)
					p->data.at(i) = oc[i];
				if(valid)
				{
					plist->m_parameters.push_back((Parameter_t*)p);
					plist->m_hasChanged = true;
					paramlist_byte_size += plength;
					delete(oc);
				}
				else
				{
					delete(p);
					delete(oc);
					return -1;
				}
				break;
			}
			case PID_TIME_BASED_FILTER:
			{
				TimeBasedFilterQosPolicy* p = new TimeBasedFilterQosPolicy();
				valid &= CDRMessage::readInt32(msg,&p->minimum_separation.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->minimum_separation.fraction);
				IF_VALID_ADD
			}
			case PID_PRESENTATION:
			{
				PresentationQosPolicy* p = new PresentationQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->access_scope);
				valid&=CDRMessage::readOctet(msg,(octet*)&p->coherent_access);
				valid&=CDRMessage::readOctet(msg,(octet*)&p->ordered_access);
				msg->pos++;
				IF_VALID_ADD
			}
			case PID_PARTITION:
			{
				PartitionQosPolicy * p = new PartitionQosPolicy();
				p->length = plength;
				valid &= CDRMessage::readOctetVector(msg,&p->name);
				IF_VALID_ADD
			}
			case PID_TOPIC_DATA:
			{
				TopicDataQosPolicy * p = new TopicDataQosPolicy();
				p->length = plength;
				valid &= CDRMessage::readOctetVector(msg,&p->value);
				IF_VALID_ADD
			}
			case PID_GROUP_DATA:
			{
				GroupDataQosPolicy * p = new GroupDataQosPolicy();
				p->length = plength;
				valid &= CDRMessage::readOctetVector(msg,&p->value);
				IF_VALID_ADD
			}
			case PID_HISTORY:
			{
				HistoryQosPolicy* p = new HistoryQosPolicy();
				valid&=CDRMessage::readOctet(msg,(octet*)&p->kind);msg->pos+=3;
				valid &= CDRMessage::readInt32(msg,&p->depth);
				IF_VALID_ADD
			}
			case PID_DURABILITY_SERVICE:
			{
				DurabilityServiceQosPolicy * p = new DurabilityServiceQosPolicy();
				valid &= CDRMessage::readInt32(msg,&p->service_cleanup_delay.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->service_cleanup_delay.fraction);
				valid&=CDRMessage::readOctet(msg,(octet*)&p->history_kind);msg->pos+=3;
				valid &= CDRMessage::readUInt32(msg,&p->history_depth);
				valid &= CDRMessage::readUInt32(msg,&p->max_samples);
				valid &= CDRMessage::readUInt32(msg,&p->max_instances);
				valid &= CDRMessage::readUInt32(msg,&p->max_samples_per_instance);
				IF_VALID_ADD
			}
			case PID_LIFESPAN:
			{
				LifespanQosPolicy * p = new LifespanQosPolicy();
				valid &= CDRMessage::readInt32(msg,&p->duration.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->duration.fraction);
				IF_VALID_ADD
			}
			case PID_OWNERSHIP_STRENGTH:
			{
				OwnershipStrengthQosPolicy * p = new OwnershipStrengthQosPolicy();
				valid &= CDRMessage::readUInt32(msg,&p->value);
				IF_VALID_ADD
			}
			case PID_RESOURCE_LIMITS:
			{
				ResourceLimitsQosPolicy* p = new ResourceLimitsQosPolicy();
				valid &= CDRMessage::readUInt32(msg,&p->max_samples);
				valid &= CDRMessage::readUInt32(msg,&p->max_instances);
				valid &= CDRMessage::readUInt32(msg,&p->max_samples_per_instance);
				IF_VALID_ADD
			}
			case PID_TRANSPORT_PRIORITY:
			{
				TransportPriorityQosPolicy * p = new TransportPriorityQosPolicy();
				valid &= CDRMessage::readUInt32(msg,&p->value);
				IF_VALID_ADD
			}
			case PID_PAD:
			default:
			{
				msg->pos +=plength;
				paramlist_byte_size +=plength;
				break;
			}
			case PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT:
			{
				ParameterCount_t*p = new ParameterCount_t(PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT,plength);
				valid&=CDRMessage::readUInt32(msg,&p->count);
				IF_VALID_ADD
			}
			case PID_PARTICIPANT_BUILTIN_ENDPOINTS:
			case PID_BUILTIN_ENDPOINT_SET:
			{
				ParameterBuiltinEndpointSet_t * p = new ParameterBuiltinEndpointSet_t(pid,plength);
				valid &= CDRMessage::readUInt32(msg,&p->endpointSet);
				IF_VALID_ADD
			}
			case PID_PARTICIPANT_LEASE_DURATION:
			{
				ParameterTime_t* p = new ParameterTime_t(PID_PARTICIPANT_LEASE_DURATION,plength);
				valid &= CDRMessage::readInt32(msg,&p->time.seconds);
				valid &= CDRMessage::readUInt32(msg,&p->time.fraction);
				IF_VALID_ADD
			}
			case PID_CONTENT_FILTER_PROPERTY:
			{
				pWarning("ContentFilter not supported in current version"<<endl);
				msg->pos +=plength;
				paramlist_byte_size +=plength;
				break;
			}
			case PID_PARTICIPANT_ENTITYID:
			case PID_GROUP_ENTITYID:
			{
				ParameterEntityId_t * p = new ParameterEntityId_t(pid,plength);
				valid &= CDRMessage::readEntityId(msg,&p->entityId);
				IF_VALID_ADD
			}
			case PID_TYPE_MAX_SIZE_SERIALIZED:
			{
				ParameterCount_t * p = new ParameterCount_t(pid,plength);
				valid &= CDRMessage::readUInt32(msg,&p->count);
				IF_VALID_ADD
			}
			}
		}
		else
		{
			return -1;
		}
	}
	return paramlist_byte_size;
}

} /* namespace dds */
} /* namespace eprosima */




