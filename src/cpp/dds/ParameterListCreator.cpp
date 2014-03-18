/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of eProsimaRTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterListCreator.cpp
 *  Parameter List creation methods.
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/ParameterListCreator.h"
#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {

ParameterListCreator::ParameterListCreator() {
	// TODO Auto-generated constructor stub

}

ParameterListCreator::~ParameterListCreator() {
	// TODO Auto-generated destructor stub
}


bool ParameterListCreator::addParameterLocator(ParameterList_t* plist,ParameterId_t pid, rtps::Locator_t loc) {
	if(pid == PID_UNICAST_LOCATOR || pid == PID_MULTICAST_LOCATOR ||
			pid == PID_DEFAULT_UNICAST_LOCATOR || pid == PID_DEFAULT_MULTICAST_LOCATOR ||
			pid == PID_METATRAFFIC_UNICAST_LOCATOR || PID_METATRAFFIC_MULTICAST_LOCATOR)
	{
		ParameterLocator_t* p = new ParameterLocator_t();
		p->Pid = pid;
		p->locator = loc;
		p->length = 24;
		plist->params.push_back((Parameter_t*)p);
		plist->has_changed = true;
		return true;
	}
	pWarning("PID not correspond with Locator Parameter.")

	return false;
}

bool ParameterListCreator::addParameterString(ParameterList_t* plist,ParameterId_t pid, std::string in_str) {
	if(pid == PID_TOPIC_NAME || pid == PID_TYPE_NAME )
	{
		ParameterString_t* p = new ParameterString_t();
		p->Pid = pid;
		p->p_str = in_str;
		plist->params.push_back((Parameter_t*)p);
		if(pid==PID_TOPIC_NAME)
			plist->inlineqos_params.push_back((Parameter_t*)p);
		plist->has_changed = true;
		return true;
	}
	pWarning("PID not correspond with String Parameter.")

	return false;
}

bool ParameterListCreator::addParameterPort(ParameterList_t* plist,ParameterId_t pid, uint32_t port) {
	if(pid == PID_DEFAULT_UNICAST_PORT || pid == PID_METATRAFFIC_UNICAST_PORT ||
			pid == PID_METATRAFFIC_MULTICAST_PORT)
	{
		ParameterPort_t* p = new ParameterPort_t();
		p->Pid = pid;
		p->port = port;
		p->length = 4;
		plist->params.push_back((Parameter_t*)p);
		plist->has_changed = true;
		return true;
	}

		pWarning("PID not correspond with Port Parameter.")
	return false;
}

bool ParameterListCreator::addParameterKey(ParameterList_t* plist,ParameterId_t pid, InstanceHandle_t handle) {
	if(pid == PID_KEY_HASH)
	{
		ParameterKey_t* p = new ParameterKey_t();
		p->Pid = pid;
		for(uint8_t i = 0;i<16;i++)
			p->value[i] = handle.value[i];
		p->length = 16;
		plist->params.push_back((Parameter_t*)p);
		plist->inlineqos_params.push_back((Parameter_t*)p);
		plist->has_changed = true;
		return true;
	}
	RTPSLog::Warning << "PID not correspond with Key Parameter."<<endl;
	RTPSLog::printWarning();
	return false;
}

bool ParameterListCreator::addParameterStatus(ParameterList_t* plist,ParameterId_t pid, octet status) {
	if(pid == PID_STATUS_INFO)
	{
		ParameterStatus_t* p = new ParameterStatus_t();
		p->Pid = pid;
		p->status = status;
		p->length = 4;
		plist->params.push_back((Parameter_t*)p);
		plist->inlineqos_params.push_back((Parameter_t*)p);
		plist->has_changed = true;
		return true;
	}
	RTPSLog::Warning << "PID not correspond with Status Parameter."<<endl;
		RTPSLog::printWarning();
	return false;
}

bool ParameterListCreator::updateMsg(ParameterList_t* plist) {

	std::vector<Parameter_t*>::iterator it;
	CDRMessage::initCDRMsg(&plist->ParamsMsg);
	plist->ParamsMsg.msg_endian = EPROSIMA_ENDIAN;
	for(it = plist->params.begin();it!=plist->params.end();it++)
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
			CDRMessage::addUInt16(&plist->ParamsMsg,p_loc->Pid);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_loc->length);
			CDRMessage::addLocator(&plist->ParamsMsg,&p_loc->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_PORT:
		case PID_METATRAFFIC_UNICAST_PORT:
		case PID_METATRAFFIC_MULTICAST_PORT:
		{
			ParameterPort_t* p_port = (ParameterPort_t*)(*it);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_port->Pid);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_port->length);
			CDRMessage::addUInt32(&plist->ParamsMsg,p_port->port);
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* p_str = (ParameterString_t*)(*it);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_str->Pid);
			//Str size
			uint32_t str_siz = p_str->p_str.size();
			int rest = str_siz%4;
			if(rest!=0)
				rest = 4-rest; //how many you have to add
			p_str->length = str_siz+4+rest;
			CDRMessage::addUInt16(&plist->ParamsMsg,p_str->length);
			CDRMessage::addUInt32(&plist->ParamsMsg,str_siz);
			CDRMessage::addData(&plist->ParamsMsg,(unsigned char*)p_str->p_str.c_str(),str_siz);
			if(rest!=0)
			{
				octet oc = '\0';
				for(int i=0;i<rest;i++)
				{
					CDRMessage::addOctet(&plist->ParamsMsg,oc);
				}
			}
			break;
		}
		case PID_KEY_HASH:
		{
			ParameterKey_t* p_key = (ParameterKey_t*)(*it);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_key->Pid);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_key->length);
			CDRMessage::addData(&plist->ParamsMsg,p_key->value,16);
			break;
		}
		case PID_STATUS_INFO:
		{
			ParameterStatus_t* p_status = (ParameterStatus_t*)(*it);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_status->Pid);
			CDRMessage::addUInt16(&plist->ParamsMsg,p_status->length);
			CDRMessage::addOctet(&plist->ParamsMsg,0);
			CDRMessage::addOctet(&plist->ParamsMsg,0);
			CDRMessage::addOctet(&plist->ParamsMsg,0);
			CDRMessage::addOctet(&plist->ParamsMsg,p_status->status);
			break;
		}
		}
	}
	//Last parameter is always PID SENTINEL FOLLOWED BY two 0 octets.
	CDRMessage::addUInt16(&plist->ParamsMsg,PID_SENTINEL);
	CDRMessage::addUInt16(&plist->ParamsMsg,0);
	plist->has_changed = false;
	return true;

}

bool ParameterListCreator::getKeyStatus(ParameterList_t* plist,InstanceHandle_t* handle, octet* status)
{
	std::vector<Parameter_t*>::iterator it;
	bool keyfound = false;
	bool statusfound = false;
	for(it = plist->params.begin();it!=plist->params.end();it++)
	{
		switch((*it)->Pid)
		{
		case PID_KEY_HASH:
		{
			ParameterKey_t* p_key = (ParameterKey_t*)(*it);
			for(uint8_t i =0;i<16;i++)
				handle->value[i] = p_key->value[i];
			keyfound = true;
			break;
		}
		case PID_STATUS_INFO:
		{
			ParameterStatus_t* p_status = (ParameterStatus_t*)(*it);
			*status = p_status->status;
			statusfound = true;
			break;
		}
		}
	}
	if(keyfound || statusfound)
		return true;
	return false;
}






bool ParameterListCreator::readParamListfromCDRmessage(ParameterList_t* plist,CDRMessage_t* msg,uint32_t* size)
{
	//Begin reading message to find parameters until we find PID_SENTINEL
	plist->params.clear();
	uint32_t params_byte_size = 0;

	bool is_sentinel = false;
	bool valid = true;
	while(!is_sentinel)
	{
		valid = true;
		Parameter_t P;
		valid&=CDRMessage::readUInt16(msg,&P.Pid);
		valid&=CDRMessage::readUInt16(msg,&P.length);
	//	cout << "Param with PID: " << P.Pid << " and length: " << P.length << endl;
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
			ParameterLocator_t* p_loc = new ParameterLocator_t();
			p_loc->Pid = P.Pid;
			p_loc->length = P.length;
			valid &= CDRMessage::readLocator(msg,&p_loc->locator);
			plist->params.push_back((Parameter_t*)p_loc);
			params_byte_size+=P.length;
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
			plist->params.push_back((Parameter_t*)p_port);
			params_byte_size+=P.length;
			break;
		}
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			ParameterString_t* param_str = new ParameterString_t();
			param_str->Pid = P.Pid;
			param_str->length = P.length;
			uint32_t str_size;
			CDRMessage::readUInt32(msg,&str_size);

			param_str->p_str.resize(str_size);
			octet* oc = new octet[str_size];
			valid &= CDRMessage::readData(msg,oc,str_size);
			for(uint32_t i =0;i<str_size;i++)
				param_str->p_str.at(i) = oc[i];
			plist->params.push_back((Parameter_t*)param_str);
			params_byte_size+=P.length;
			msg->pos += (P.length-str_size-4);
			delete(oc);
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
			params_byte_size +=P.length;
		}
		}
		if(!valid)
			return false;
	}
	*size = params_byte_size;
	plist->has_changed = true;
	return true;
}



} /* namespace dds */
} /* namespace eprosima */
