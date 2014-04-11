/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosList.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/dds/QosList.h"

namespace eprosima {
namespace dds {

QosList_t::QosList_t()
{

}

QosList_t::~QosList_t()
{
	this->allQos.deleteParams();
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	std::string& string_in)
{
	if(pid == PID_TOPIC_NAME || pid == PID_TYPE_NAME )
	{
		ParameterString_t* p = new ParameterString_t();
		p->Pid = pid;
		p->m_string = string_in;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		if(pid==PID_TOPIC_NAME)
		{
			qos->inlineQos.m_parameters.push_back((Parameter_t*)p);
			qos->inlineQos.m_hasChanged = true;
		}
		return true;
	}
	pWarning("PID not correspond with String Parameter."<<endl)
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Locator_t& loc)
{
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = loc;
		p->length = PARAMETER_LOCATOR_LENGTH;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	pWarning("PID not correspond with Locator Parameter."<<endl)
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
		qos->allQos.m_hasChanged = true;
		return true;
	}
	else if(pid == PID_BUILTIN_ENDPOINT_SET)
	{
		//TODOG: Finish
		ParameterBuiltinEndpointSet_t* p = new ParameterBuiltinEndpointSet_t();
		p->Pid = pid;
		p->length = PARAMETER_BUILTINENDPOINTSET_LENGTH;
		p->endpointSet = input_uint32;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	else if(pid == PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT)
	{
		//TODOG: Finish
		ParameterCount_t* p = new ParameterCount_t();
		p->Pid = pid;
		p->length = PARAMETER_COUNT_LENGTH;
		p->count = input_uint32;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	pWarning("PID not correspond with Uint32_t Parameter."<<endl)
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	GUID_t& guid)
{
	if(pid == PID_PARTICIPANT_GUID || pid == PID_GROUP_GUID)
	{
		//TODOG: Finish
		ParameterGuid_t* p = new ParameterGuid_t();
		p->Pid = pid;
		p->length = PARAMETER_GUID_LENGTH;
		p->guid = guid;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	ProtocolVersion_t& protocol)
{
	if(pid == PID_PROTOCOL_VERSION)
	{
		//TODOG: Finish
		ParameterProtocolVersion_t* p = new ParameterProtocolVersion_t();
		p->Pid = pid;
		p->length = PARAMETER_PROTOCOL_LENGTH;
		p->protocolVersion = protocol;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	VendorId_t& vendor)
{
	if(pid == PID_VENDORID)
		{
			//TODOG: Finish
			ParameterVendorId_t* p = new ParameterVendorId_t();
			p->Pid = pid;
			p->length = PARAMETER_VENDOR_LENGTH;
			p->vendorId[0] = vendor[0];
			p->vendorId[1] = vendor[1];
			qos->allQos.m_parameters.push_back((Parameter_t*)p);
			qos->allQos.m_hasChanged = true;
			return true;
		}
		return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid, octet o1,	octet o2, octet o3, octet o4)
{
	if(pid == PID_METATRAFFIC_MULTICAST_IPADDRESS || pid == PID_DEFAULT_UNICAST_IPADDRESS ||
			pid == PID_METATRAFFIC_UNICAST_IPADDRESS || pid == PID_MULTICAST_IPADDRESS)
	{
		//TODOG: Finish
		ParameterIP4Address_t* p = new ParameterIP4Address_t();
		p->Pid = pid;
		p->length = PARAMETER_IP4_LENGTH;
		p->address[0] = o1;
		p->address[1] = o2;
		p->address[2] = o3;
		p->address[3] = o4;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	return false;
}

//bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Count_t& count)
//{
//	if(pid == PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT)
//	{
//		//TODOG: Finish
//		ParameterCount_t* p = new ParameterCount_t();
//		p->Pid = pid;
//		p->length = PARAMETER_COUNT_LENGTH;
//		p->count = count;
//		qos->allQos.m_parameters.push_back((Parameter_t*)p);
//		qos->allQos.m_hasChangedMsg = true;
//		return true;
//	}
//	return false;
//}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	EntityId_t& entity) {
	if(pid == PID_GROUP_ENTITYID)
	{
		//TODOG: Finish
		ParameterEntityId_t* p = new ParameterEntityId_t();
		p->Pid = pid;
		p->length = PARAMETER_ENTITYID_LENGTH;
		p->entityId = entity;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Time_t& time_in) {
	if(pid == PID_PARTICIPANT_LEASE_DURATION)
	{
		//TODOG: Finish
		ParameterTime_t* p = new ParameterTime_t();
		p->Pid = pid;
		p->length = PARAMETER_TIME_LENGTH;
		p->time = time_in;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChanged = true;
		return true;
	}
	return false;
}

//bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	BuiltinEndpointSet_t& endpointset)
//{
//	if(pid == PID_BUILTIN_ENDPOINT_SET)
//	{
//		//TODOG: Finish
//		ParameterBuiltinEndpointSet_t* p = new ParameterBuiltinEndpointSet_t();
//		p->Pid = pid;
//		p->length = PARAMETER_BUILTINENDPOINTSET_LENGTH;
//		p->endpointSet = endpointset;
//		qos->allQos.m_parameters.push_back((Parameter_t*)p);
//		qos->allQos.m_hasChangedMsg = true;
//		return true;
//	}
//	return false;
//}



} /* namespace dds */
} /* namespace eprosima */
