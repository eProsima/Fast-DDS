/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ParameterList.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/ParameterList.h"
#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {

ParameterList::ParameterList() {
	// TODO Auto-generated constructor stub

}

ParameterList::~ParameterList() {
	// TODO Auto-generated destructor stub
}

bool ParameterList::addParameterLocator(ParameterId_t pid, rtps::Locator_t loc) {
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = loc;
		params.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool ParameterList::addParameterString(ParameterId_t pid, std::string in_str) {
	if(pid == PID_TOPIC_NAME || pid == PID_TYPE_NAME )
	{
		ParameterString_t* p = new ParameterString_t();
		p->Pid = pid;
		p->p_str = in_str;
		params.push_back((Parameter_t*)p);
		return true;

	}
	return false;
}

bool ParameterList::addParameterPort(ParameterId_t pid, uint32_t port) {
	if(pid == PID_DEFAULT_UNICAST_PORT || pid == PID_METATRAFFIC_UNICAST_PORT ||
			pid == PID_METATRAFFIC_MULTICAST_PORT)
	{
		ParameterPort_t* p = new ParameterPort_t();
		p->Pid = pid;
		p->port = port;
		params.push_back((Parameter_t*)p);
		return true;
	}
	return false;
}

bool ParameterList::updateMsg() {
	std::vector<Parameter_t*>::iterator it;
	CDRMessage::initCDRMsg(&ParamsMsg);
	for(it = params.begin();it!=params.end();it++)
	{
		switch((*it)->Pid)
		{
		case PID_UNICAST_LOCATOR:
		case PID_MULTICAST_LOCATOR:
		case PID_DEFAULT_UNICAST_LOCATOR:
		case PID_DEFAULT_MULTICAST_LOCATOR:
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p_loc = (ParameterLocator_t*)(*it);
			CDRMessage::addUInt16(&ParamsMsg,p_loc->Pid);
			CDRMessage::addUInt16(&ParamsMsg,24);
			CDRMessage::addLocator(&ParamsMsg,&p_loc->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = (ParameterPort_t*)(*it);
			CDRMessage::addUInt16(&ParamsMsg,p_port->Pid);
			CDRMessage::addUInt16(&ParamsMsg,4);
			CDRMessage::addUInt32(&ParamsMsg,p_port->port);
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* p_str = (ParameterString_t*)(*it);
			CDRMessage::addUInt16(&ParamsMsg,p_str->Pid);
			//Str size
			int siz = p_str->p_str.size();
			CDRMessage::addData(&ParamsMsg,(unsigned char*)p_str->p_str.c_str(),siz);
			int rest = siz%4;
			if(rest!=0)
			{
				octet oc = '\0';
				for(int i=0;i<rest;i++)
					CDRMessage::addOctet(&ParamsMsg,oc);
			}
			break;
		}
		}
	}
	//Last parameter is always PID SENTINEL FOLLOWED BY two 0 octets.
	CDRMessage::addUInt16(&ParamsMsg,PID_SENTINEL);
	CDRMessage::addUInt16(&ParamsMsg,0);
	return true;
}

} /* namespace dds */
} /* namespace eprosima */


