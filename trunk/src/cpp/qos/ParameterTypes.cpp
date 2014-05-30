/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterTypes.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/qos/ParameterTypes.h"

#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {


// PARAMETER
Parameter_t::Parameter_t():Pid(PID_PAD), length(0) { };
Parameter_t::~Parameter_t(){};
Parameter_t::Parameter_t(ParameterId_t pid,uint16_t in_length):Pid(pid),length(in_length) {};

// PARAMETER LOCATOR
bool ParameterLocator_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_LOCATOR_LENGTH);//this->length);
	valid &= CDRMessage::addLocator(msg, &this->locator);
	return valid;
}

//PARAMTERKEY
bool ParameterKey_t::addToCDRMessage(CDRMessage_t* msg)
{
	return CDRMessage::addParameterKey(msg,&this->key);

}

// PARAMETER_ STRING
bool ParameterString_t::addToCDRMessage(CDRMessage_t* msg)
{
	if(this->m_string.size()==0)
		return false;
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	//Str size
	uint32_t str_siz = this->m_string.size();
	int rest = str_siz % 4;
	if (rest != 0)
		rest = 4 - rest; //how many you have to add
	this->length = str_siz + 4 + rest;
//	cout << "string: "<<this->m_string << endl;
//	cout << "param length: " << this->length << " str size: "<< str_siz << " rest: " << rest << endl;
	valid &= CDRMessage::addUInt16(msg, this->length);
	valid &= CDRMessage::addUInt32(msg, str_siz);
	valid &= CDRMessage::addData(msg,
			(unsigned char*) this->m_string.c_str(), str_siz);
	if (rest != 0) {
		octet oc = '\0';
		for (int i = 0; i < rest; i++) {
			valid &= CDRMessage::addOctet(msg, oc);
		}
	}
	return valid;
}

// PARAMETER_ PORT
bool ParameterPort_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_PORT_LENGTH);//this->length);
	valid &= CDRMessage::addUInt32(msg, this->port);
	return valid;
}

//PARAMETER_ GUID
bool ParameterGuid_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_GUID_LENGTH);//this->length);
	valid &= CDRMessage::addData(msg,this->guid.guidPrefix.value,12);
	valid &= CDRMessage::addData(msg,this->guid.entityId.value,4);
	return valid;
}


//PARAMETER_ PROTOCOL VERSION
bool ParameterProtocolVersion_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_PROTOCOL_LENGTH);//this->length);
	valid &= CDRMessage::addOctet(msg,protocolVersion.m_major);
	valid &= CDRMessage::addOctet(msg,protocolVersion.m_minor);
	valid &= CDRMessage::addUInt16(msg, 0);
	return valid;
}

bool ParameterVendorId_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_VENDOR_LENGTH);//this->length);
	valid &= CDRMessage::addOctet(msg,vendorId[0]);
	valid &= CDRMessage::addOctet(msg,vendorId[1]);
	valid &= CDRMessage::addUInt16(msg, 0);
	return valid;
}


//PARAMETER_ IP4ADDRESS
bool ParameterIP4Address_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_IP4_LENGTH);//this->length);
	valid &= CDRMessage::addData(msg,this->address,4);
	return valid;
}
void ParameterIP4Address_t::setIP4Address(octet o1,octet o2,octet o3,octet o4){
	address[0] = o1;
	address[1] = o2;
	address[2] = o3;
	address[3] = o4;
}


bool ParameterBool_t::addToCDRMessage(CDRMessage_t* msg){
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_BOOL_LENGTH);//this->length);
	octet val = value ? 1:0;
	valid &= CDRMessage::addOctet(msg,val);
	valid &= CDRMessage::addOctet(msg,0);
	valid &= CDRMessage::addUInt16(msg,0);
	return valid;
}


bool ParameterCount_t::addToCDRMessage(CDRMessage_t* msg){
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_COUNT_LENGTH);//this->length);
	valid &= CDRMessage::addUInt32(msg,count);
	return valid;
}


bool ParameterEntityId_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_ENTITYID_LENGTH);//this->length);
	valid &= CDRMessage::addEntityId(msg,&entityId);
	return valid;
}

bool ParameterTime_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_TIME_LENGTH);//this->length);
	valid &= CDRMessage::addInt32(msg,time.seconds);
	valid &= CDRMessage::addInt32(msg,time.nanoseconds);
	return valid;
}

bool ParameterBuiltinEndpointSet_t::addToCDRMessage(CDRMessage_t*msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, PARAMETER_BUILTINENDPOINTSET_LENGTH);//this->length);
	valid &= CDRMessage::addUInt32(msg,this->endpointSet);
	return valid;
}

bool ParameterPropertyList_t::addToCDRMessage(CDRMessage_t*msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	uint16_t pos_str = msg->pos;
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addUInt32(msg,this->properties.size());
	for(std::vector<std::pair<std::string,std::string>>::iterator it = this->properties.begin();
			it!=this->properties.end();++it)
	{
		valid &= CDRMessage::addString(msg,it->first);
		valid &= CDRMessage::addString(msg,it->second);
	}
	uint16_t pos_param_end = msg->pos;
	this->length = pos_param_end-pos_str-2;
	msg->pos = pos_str;
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	msg->pos = pos_param_end;
	msg->length-=2;
	return valid;
}


} /* namespace dds */
} /* namespace eprosima */


