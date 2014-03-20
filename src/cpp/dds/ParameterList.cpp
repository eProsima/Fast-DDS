/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterList.cpp
 *
 *  Created on: Mar 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/ParameterList.h"

namespace eprosima {
namespace dds {

void ParameterList_t::get_QosMsg(CDRMessage_t** msg)
{
	if(has_changed_Qos)
	{
		ParameterList::updateQosMsg(this);
	}
	*msg = &QosMsg;
}

void ParameterList_t::get_inlineQosMsg(CDRMessage_t** msg)
{
	if(has_changed_inlineQos)
	{
		ParameterList::updateInlineQosMsg(this);
	}
	*msg = &inlineQosMsg;
}

bool ParameterList::addParameterLocator(ParameterList_t* plist,ParameterId_t pid, rtps::Locator_t* loc) {
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = *loc;
		p->length = 24;
		plist->QosParams.push_back((Parameter_t*)p);
		plist->has_changed_Qos = true;
		return true;
	}
	pWarning("PID not correspond with Locator Parameter."<<endl)

	return false;
}

bool ParameterList::addParameterString(ParameterList_t* plist,ParameterId_t pid, std::string& in_str)
{
	if(pid == PID_TOPIC_NAME || pid == PID_TYPE_NAME )
	{
		ParameterString_t* p = new ParameterString_t();
		p->Pid = pid;
		p->p_str = in_str;
		plist->QosParams.push_back((Parameter_t*)p);
		if(pid==PID_TOPIC_NAME)
		{
			plist->inlineQosParams.push_back((Parameter_t*)p);
			plist->has_changed_inlineQos = true;
		}
		plist->has_changed_Qos = true;
		return true;
	}
	pWarning("PID not correspond with String Parameter."<<endl)

	return false;
}

bool ParameterList::addParameterPort(ParameterList_t* plist,ParameterId_t pid, uint32_t port) {
	if(pid == PID_DEFAULT_UNICAST_PORT || pid == PID_METATRAFFIC_UNICAST_PORT ||
			pid == PID_METATRAFFIC_MULTICAST_PORT)
	{
		ParameterPort_t* p = new ParameterPort_t();
		p->Pid = pid;
		p->port = port;
		p->length = 4;
		plist->QosParams.push_back((Parameter_t*)p);
		plist->has_changed_Qos = true;
		return true;
	}

		pWarning("PID not correspond with Port Parameter."<<endl)
	return false;
}

bool ParameterList::updateQosMsg(ParameterList_t* plist)
{
	if(updateMsg(&plist->QosParams,&plist->QosMsg))
	{
		plist->has_changed_Qos = false;
		return true;
	}
	else
		return false;
}

bool ParameterList::updateInlineQosMsg(ParameterList_t* plist)
{
	if(updateMsg(&plist->QosParams,&plist->QosMsg))
	{
		plist->has_changed_inlineQos = false;
		return true;
	}
	else
		return false;
}


bool ParameterList::updateMsg(std::vector<Parameter_t*>* vec,CDRMessage_t* msg)
{
	std::vector<Parameter_t*>::iterator it;
	CDRMessage::initCDRMsg(msg);
	for(it = vec->begin();it!=vec->end();it++)
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
			CDRMessage::addUInt16(msg,p_loc->Pid);
			CDRMessage::addUInt16(msg,p_loc->length);
			CDRMessage::addLocator(msg,&p_loc->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = (ParameterPort_t*)(*it);
			CDRMessage::addUInt16(msg,p_port->Pid);
			CDRMessage::addUInt16(msg,p_port->length);
			CDRMessage::addUInt32(msg,p_port->port);
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* p_str = (ParameterString_t*)(*it);
			CDRMessage::addUInt16(msg,p_str->Pid);
			//Str size
			uint32_t str_siz = p_str->p_str.size();
			int rest = str_siz%4;
			if(rest!=0)
				rest = 4-rest; //how many you have to add
			p_str->length = str_siz+4+rest;
			CDRMessage::addUInt16(msg,p_str->length);
			CDRMessage::addUInt32(msg,str_siz);
			CDRMessage::addData(msg,(unsigned char*)p_str->p_str.c_str(),str_siz);
			if(rest!=0)
			{
				octet oc = '\0';
				for(int i=0;i<rest;i++)
				{
					CDRMessage::addOctet(msg,oc);
				}
			}
			break;
		}
		}
	}
	//Last parameter is always PID SENTINEL FOLLOWED BY two 0 octets.
	CDRMessage::addUInt16(msg,PID_SENTINEL);
	CDRMessage::addUInt16(msg,0);
	return true;
}






} /* namespace dds */
} /* namespace eprosima */
