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

#include "eprosimartps/dds/ParameterList.h"
#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {

inline void ParameterList_t::get_QosMsg(CDRMessage_t** msg,Endianness_t endian)
{
	if(has_changed_Qos || endian != QosMsg.msg_endian)
	{
		ParameterList::updateQosMsg(this,endian);
	}
	*msg = &QosMsg;
}

inline void ParameterList_t::get_inlineQosMsg(CDRMessage_t** msg,Endianness_t endian)
{
	if(has_changed_inlineQos || endian != inlineQosMsg.msg_endian)
	{
		ParameterList::updateInlineQosMsg(this,endian);
	}
	*msg = &inlineQosMsg;
}

inline bool ParameterList::addParameterLocator(ParameterList_t* plist,ParameterId_t pid, rtps::Locator_t* loc) {
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

inline bool ParameterList::addParameterString(ParameterList_t* plist,ParameterId_t pid, std::string& in_str)
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

inline bool ParameterList::addParameterPort(ParameterList_t* plist,ParameterId_t pid, uint32_t port) {
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

inline bool ParameterList::updateQosMsg(ParameterList_t* plist,Endianness_t endian)
{
	if(updateMsg(&plist->QosParams,&plist->QosMsg,endian))
	{
		plist->has_changed_Qos = false;
		return true;
	}
	else
		return false;
}

inline bool ParameterList::updateInlineQosMsg(ParameterList_t* plist,Endianness_t endian)
{
	if(updateMsg(&plist->inlineQosParams,&plist->inlineQosMsg,endian))
	{
		plist->has_changed_inlineQos = false;
		return true;
	}
	else
		return false;
}


inline bool ParameterList::updateMsg(std::vector<Parameter_t*>* vec,CDRMessage_t* msg,Endianness_t endian)
{
	std::vector<Parameter_t*>::iterator it;
	CDRMessage::initCDRMsg(msg);
	msg->msg_endian = endian;
//	pDebugInfo("Adding parameters to list: " << vec->size() << endl;);
	for(it = vec->begin();it!=vec->end();++it)
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
		case PID_SENTINEL:
		case PID_PAD:
		case PID_KEY_HASH:
		case PID_STATUS_INFO:
		default:
		{
			break;
		}
		}
	}
	//pDebugInfo("Adding sentinel parameter "<<endl);
	//Last parameter is always PID SENTINEL FOLLOWED BY two 0 octets.
	CDRMessage::addUInt16(msg,PID_SENTINEL);
	CDRMessage::addUInt16(msg,0);
	pDebugInfo("Param msg created of size: " << msg->length << endl;)
	return true;
}


inline bool ParameterList::readParameterList(CDRMessage_t* msg,ParameterList_t* plist,uint32_t* size,ChangeKind_t* kind,InstanceHandle_t* iHandle)
{
	plist->QosParams.clear();
	uint32_t params_byte_size = 0;
	bool is_sentinel = false;
	bool valid;
	while(!is_sentinel)
	{
		valid = true;
		Parameter_t P;
		valid&=CDRMessage::readUInt16(msg,(uint16_t*)&P.Pid);
		valid&=CDRMessage::readUInt16(msg,&P.length);
		params_byte_size+=4;
		switch(P.Pid)
		{
		case PID_UNICAST_LOCATOR:
		case PID_MULTICAST_LOCATOR:
		case PID_DEFAULT_UNICAST_LOCATOR:
		case PID_DEFAULT_MULTICAST_LOCATOR:
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p_loc = new ParameterLocator_t(&P);
			valid &= CDRMessage::readLocator(msg,&p_loc->locator);
			plist->QosParams.push_back((Parameter_t*)p_loc);
			params_byte_size+=P.length;
			plist->has_changed_Qos = true;
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = new ParameterPort_t(&P);
			valid &= CDRMessage::readUInt32(msg,&p_port->port);
			plist->QosParams.push_back((Parameter_t*)p_port);
			params_byte_size+=P.length;
			plist->has_changed_Qos = true;
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* param_str = new ParameterString_t(&P);
			uint32_t str_size = 1;
			CDRMessage::readUInt32(msg,&str_size);
			param_str->p_str.resize(str_size);
			octet* oc=new octet[str_size];
			valid &= CDRMessage::readData(msg,oc,str_size);
			for(uint32_t i =0;i<str_size;i++)
				param_str->p_str.at(i) = oc[i];
			plist->QosParams.push_back((Parameter_t*)param_str);
			params_byte_size+=P.length;
			msg->pos += (P.length-str_size-4);
			if(P.Pid == PID_TOPIC_NAME)
			{
				plist->inlineQosParams.push_back((Parameter_t*)param_str);
				plist->has_changed_inlineQos = true;
			}
			plist->has_changed_Qos = true;
			break;
		}
		case PID_KEY_HASH:
		{
			CDRMessage::readData(msg,iHandle->value,16);
			params_byte_size+=16;
			break;
		}
		case PID_STATUS_INFO:
		{
			octet status = msg->buffer[msg->pos+3];
			if(status == 1)
				*kind = NOT_ALIVE_DISPOSED;
			else if (status == 2)
				*kind = NOT_ALIVE_UNREGISTERED;
			msg->pos+=4;
			params_byte_size+=4;
			break;
		}
		case PID_SENTINEL:
		{
			is_sentinel = true;
			break;
		}
		case PID_PAD:
		default: //WE DONT KNOW WHAT PARAMETER IS
		{
			msg->pos+=P.length; //we dont process it
			params_byte_size +=P.length;
		}
		}
		if(!valid)
			return false;
	}
	*size = params_byte_size;
	return true;
}






} /* namespace dds */
} /* namespace eprosima */
