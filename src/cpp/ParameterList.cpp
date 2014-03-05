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
namespace rtps {

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
		p->length = 24;
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
		p->length = 4;
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
			CDRMessage::addUInt16(&ParamsMsg,p_loc->length);
			CDRMessage::addLocator(&ParamsMsg,&p_loc->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = (ParameterPort_t*)(*it);
			CDRMessage::addUInt16(&ParamsMsg,p_port->Pid);
			CDRMessage::addUInt16(&ParamsMsg,p_port->length);
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
			p_str->length = siz;
			CDRMessage::addData(&ParamsMsg,(unsigned char*)p_str->p_str.c_str(),siz);
			int rest = siz%4;
			if(rest!=0)
			{
				octet oc = '\0';
				for(int i=0;i<rest;i++)
				{
					CDRMessage::addOctet(&ParamsMsg,oc);
					p_str->length++;
				}
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

bool ParameterList::assignParamList(CDRMessage_t* msg,uint32_t* size)
{
	//Begin reading message to find parameters until we find PID_SENTINEL
	params.clear();
	size = 0;
	bool is_sentinel = false;
	bool valid = true;
	while(!is_sentinel)
	{
		valid = true;
		Parameter_t P;
		valid&=CDRMessage::readUInt16(msg,&P.Pid);
		valid&=CDRMessage::readUInt16(msg,&P.length);
		size+=4;

		switch(P.Pid)
		{
		case PID_UNICAST_LOCATOR:
		case PID_MULTICAST_LOCATOR:
		case PID_DEFAULT_UNICAST_LOCATOR:
		case PID_DEFAULT_MULTICAST_LOCATOR:
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p_loc = new ParameterLocator_t();
			p_loc->Pid = P.Pid;
			p_loc->length = P.length;
			valid &= CDRMessage::readLocator(msg,&p_loc->locator); //TODOG implement funciton read locator
			params.push_back((Parameter_t*)p_loc);
			size+=P.length;
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = new ParameterPort_t();
			p_port->Pid = P.Pid;
			p_port->length = P.length;
			valid &= CDRMessage::readUInt32(msg,&p_port->port);
			params.push_back((Parameter_t*)p_port);
			size+=P.length;
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* param_str = new ParameterString_t();
			param_str->Pid = P.Pid;
			param_str->length = P.length;
			param_str->p_str.resize(P.length);
			octet oc[P.length];
			valid &= CDRMessage::readData(msg,oc,P.length);
			for(uint32_t i =0;i<P.length;i++)
				param_str->p_str.at(i) = oc[i];
			params.push_back((Parameter_t*)param_str);
			size+=P.length;
			break;
		}
		case PID_SENTINEL:
		{
			is_sentinel = true;
			break;
		}
		default: //WE DONT KNOW WHAT PARAMETER IS
		{
			msg->pos+=P.length; //we dont process it
		}
		}
		if(!valid)
			return false;
	}
	return true;
}



} /* namespace dds */
} /* namespace eprosima */


