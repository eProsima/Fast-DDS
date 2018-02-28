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
 * @file QosList.cpp
 *
 */

#include <fastrtps/qos/QosList.h>

#include <fastrtps/qos/QosPolicies.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

QosList_t::QosList_t()
{

}

QosList_t::~QosList_t()
{
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	std::string& string_in)
{
	if(string_in.size()==0)
		return false;
	if(pid == PID_TOPIC_NAME || pid == PID_TYPE_NAME || pid == PID_ENTITY_NAME)
	{
		ParameterString_t* p = new ParameterString_t();
		p->Pid = pid;
		p->setName(string_in.c_str());
		//p->length = string_in.size()+2;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		if(pid==PID_TOPIC_NAME)
			qos->inlineQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}

	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Locator_t& loc)
{
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || pid == PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = loc;
		p->length = PARAMETER_LOCATOR_LENGTH;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}

	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	uint32_t input_uint32)
{
	if(pid == PID_DEFAULT_UNICAST_PORT || pid == PID_METATRAFFIC_UNICAST_PORT ||
			pid == PID_METATRAFFIC_MULTICAST_PORT)
	{
		ParameterPort_t* p = new ParameterPort_t();
		p->Pid = pid;
		p->port = input_uint32;
		p->length = PARAMETER_PORT_LENGTH;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	else if(pid == PID_BUILTIN_ENDPOINT_SET)
	{

		ParameterBuiltinEndpointSet_t* p = new ParameterBuiltinEndpointSet_t();
		p->Pid = pid;
		p->length = PARAMETER_BUILTINENDPOINTSET_LENGTH;
		p->endpointSet = input_uint32;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	else if(pid == PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT)
	{

		ParameterCount_t* p = new ParameterCount_t();
		p->Pid = pid;
		p->length = PARAMETER_COUNT_LENGTH;
		p->count = input_uint32;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}

	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	bool in_bool)
{
	if(pid == PID_EXPECTS_INLINE_QOS)
	{

		ParameterBool_t* p = new ParameterBool_t();
		p->Pid = pid;
		p->length = PARAMETER_BOOL_LENGTH;
		p->value = in_bool;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}

	return false;
}



bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	const GUID_t& guid)
{
	if(pid == PID_PARTICIPANT_GUID || pid == PID_GROUP_GUID || pid == PID_PERSISTENCE_GUID)
	{

		ParameterGuid_t* p = new ParameterGuid_t();
		p->Pid = pid;
		p->length = PARAMETER_GUID_LENGTH;
		p->guid = guid;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	ProtocolVersion_t& protocol)
{
	if(pid == PID_PROTOCOL_VERSION)
	{
        ParameterProtocolVersion_t* p = new ParameterProtocolVersion_t();
		p->Pid = pid;
		p->length = PARAMETER_PROTOCOL_LENGTH;
		p->protocolVersion = protocol;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	VendorId_t& vendor)
{
    if(pid == PID_VENDORID)
    {
        ParameterVendorId_t* p = new ParameterVendorId_t();
        p->Pid = pid;
        p->length = PARAMETER_VENDOR_LENGTH;
        p->vendorId[0] = vendor[0];
        p->vendorId[1] = vendor[1];
        qos->allQos.m_parameters.push_back((Parameter_t*)p);
        return true;
    }
    return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid, octet o1,	octet o2, octet o3, octet o4)
{
	if(pid == PID_METATRAFFIC_MULTICAST_IPADDRESS || pid == PID_DEFAULT_UNICAST_IPADDRESS ||
			pid == PID_METATRAFFIC_UNICAST_IPADDRESS || pid == PID_MULTICAST_IPADDRESS)
	{
		ParameterIP4Address_t* p = new ParameterIP4Address_t();
		p->Pid = pid;
		p->length = PARAMETER_IP4_LENGTH;
		p->address[0] = o1;
		p->address[1] = o2;
		p->address[2] = o3;
		p->address[3] = o4;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos,ParameterId_t pid ,std::string& str1,std::string& str2)
{
	if(pid == PID_PROPERTY_LIST)
	{
		ParameterPropertyList_t* p = NULL;
		bool found = false;
		for(std::vector<Parameter_t*>::iterator it = qos->allQos.m_parameters.begin();
				it!=qos->allQos.m_parameters.end();++it)
		{
			if((*it)->Pid == PID_PROPERTY_LIST)
			{
				p = (ParameterPropertyList_t*)(*it);
				found = true;
				break;
			}
		}
		if(!found)
		{
			p = new ParameterPropertyList_t();
		}
		p->Pid = PID_PROPERTY_LIST;
		p->properties.push_back(std::pair<std::string,std::string>(str1,str2));
		if(!found)
			qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

 bool QosList::addQos(QosList_t* qos,ParameterId_t /*pid*/, const ParameterPropertyList_t& list)
 {
	 ParameterPropertyList_t* p = new ParameterPropertyList_t();
	 for(std::vector<std::pair<std::string,std::string>>::const_iterator it = list.properties.begin();
		 it!= list.properties.end();++it)
	 {
		 p->properties.push_back(*it);
	 }
	 qos->allQos.m_parameters.push_back((Parameter_t*)p);
	 return true;
 }



//bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Count_t& count)
//{
//	if(pid == PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT)
//	{
//		ParameterCount_t* p = new ParameterCount_t();
//		p->Pid = pid;
//		p->length = PARAMETER_COUNT_LENGTH;
//		p->count = count;
//		qos->allQos.m_parameters.push_back((Parameter_t*)p);
//		return true;
//	}
//	return false;
//}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	const EntityId_t& entity) {
	if(pid == PID_GROUP_ENTITYID)
	{
		ParameterEntityId_t* p = new ParameterEntityId_t();
		p->Pid = pid;
		p->length = PARAMETER_ENTITYID_LENGTH;
		p->entityId = entity;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Time_t& time_in) {
	if(pid == PID_PARTICIPANT_LEASE_DURATION)
	{
		ParameterTime_t* p = new ParameterTime_t();
		p->Pid = pid;
		p->length = PARAMETER_TIME_LENGTH;
		p->time = time_in;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	std::vector<octet>& ocVec) {
	if(pid == PID_USER_DATA)
	{
		UserDataQosPolicy* p = new UserDataQosPolicy();
		p->setDataVec(ocVec);
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	const IdentityToken& identity_token) {
	if(pid == PID_IDENTITY_TOKEN)
	{
		ParameterToken_t* p = new ParameterToken_t();
        p->Pid = pid;
        p->token = identity_token;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

//bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	BuiltinEndpointSet_t endpointset)
//{
//	if(pid == PID_BUILTIN_ENDPOINT_SET)
//	{
//
//		ParameterBuiltinEndpointSet_t* p = new ParameterBuiltinEndpointSet_t();
//		p->Pid = pid;
//		p->length = PARAMETER_BUILTINENDPOINTSET_LENGTH;
//		p->endpointSet = endpointset;
//		qos->allQos.m_parameters.push_back((Parameter_t*)p);
//		return true;
//	}
//	return false;
//}