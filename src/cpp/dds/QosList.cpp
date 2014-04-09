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
		qos->allQos.m_hasChangedMsg = true;
		if(pid==PID_TOPIC_NAME)
		{
			qos->inlineQos.m_parameters.push_back((Parameter_t*)p);
			qos->inlineQos.m_hasChangedMsg = true;
		}
		return true;
	}
	pWarning("PID not correspond with String Parameter."<<endl)
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Locator_t& loc) {
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = loc;
		p->length = PARAMETER_LOCATOR_LENGTH;
		qos->allQos.m_parameters.push_back((Parameter_t*)p);
		qos->allQos.m_hasChangedMsg = true;
		return true;
	}
	pWarning("PID not correspond with Locator Parameter."<<endl)
	return false;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	uint32_t port) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	GUID_t& guid) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	ProtocolVersion_t& protocol) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	VendorId_t& vendor) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid, octet o1,	octet o2, octet o3, octet o4) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Count_t& count) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	EntityId_t& entity) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	Time_t& entity) {
	//TODOG: Finish
	return true;
}

bool QosList::addQos(QosList_t* qos, ParameterId_t pid,	BuiltinEndpointSet_t& endpointset) {
	//TODOG: Finish
	return true;
}



} /* namespace dds */
} /* namespace eprosima */
