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

#include "ParameterTypes.h"

#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {


// PARAMETER
Parameter_t::Parameter_t():Pid(PID_PAD), length(0) { };
Parameter_t::~Parameter_t(){};
Parameter_t::Parameter_t(Parameter_t* P):Pid(P->Pid), length(P->length){};

// PARAMETER LOCATOR
ParameterLocator_t::ParameterLocator_t(){ };
ParameterLocator_t::ParameterLocator_t(Parameter_t* P):Parameter_t(P) {};
bool ParameterLocator_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 24);//this->length);
	valid &= CDRMessage::addLocator(msg, &this->locator);
	return valid;
}

// PARAMETER STRING
ParameterString_t::ParameterString_t(){};
ParameterString_t::ParameterString_t(Parameter_t* P):Parameter_t(P) {};
bool ParameterString_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	//Str size
	uint32_t str_siz = this->m_string.size();
	int rest = str_siz % 4;
	if (rest != 0)
		rest = 4 - rest; //how many you have to add
	this->length = str_siz + 4 + rest;
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

// PARAMETER PORT
ParameterPort_t::ParameterPort_t():port(0) {};
ParameterPort_t::ParameterPort_t(Parameter_t* P):Parameter_t(P), port(0) {};
bool ParameterPort_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addUInt32(msg, this->port);
	return valid;
}

//PARAMETER GUID

ParameterGuid_t::ParameterGuid_t(){GUID_UNKNOWN(guid)};
ParameterGuid_t::ParameterGuid_t(Parameter_t* P):Parameter_t(P) {GUID_UNKNOWN(guid)};
bool ParameterGuid_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 16);//this->length);
	valid &= CDRMessage::addData(msg,this->guid.guidPrefix.value,12);
	valid &= CDRMessage::addData(msg,this->guid.entityId.value,4);
	return valid;
}


//PARAMETER PROTOCOL VERSION
ParameterProtocolVersion_t::ParameterProtocolVersion_t(){PROTOCOLVERSION(protocolVersion)};
ParameterProtocolVersion_t::ParameterProtocolVersion_t(Parameter_t*P):Parameter_t(P) {PROTOCOLVERSION(protocolVersion)};
bool ParameterProtocolVersion_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addOctet(msg,protocolVersion.m_major);
	valid &= CDRMessage::addOctet(msg,protocolVersion.m_minor);
	valid &= CDRMessage::addUInt16(msg, 0);
	return valid;
}

ParameterVendorId_t::ParameterVendorId_t(){VENDORID_EPROSIMA(vendorId)};
ParameterVendorId_t::ParameterVendorId_t(Parameter_t* P):Parameter_t(P) {VENDORID_EPROSIMA(vendorId)};
bool ParameterVendorId_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addOctet(msg,vendorId[0]);
	valid &= CDRMessage::addOctet(msg,vendorId[1]);
	valid &= CDRMessage::addUInt16(msg, 0);
	return valid;
}


//PARAMETER IP4ADDRESS
ParameterIP4Address_t::ParameterIP4Address_t(){this->setIP4Address(0,0,0,0);};
ParameterIP4Address_t::ParameterIP4Address_t(Parameter_t* P):Parameter_t(P){this->setIP4Address(0,0,0,0);};
bool ParameterIP4Address_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addData(msg,this->address,4);
	return valid;
}
void ParameterIP4Address_t::setIP4Address(octet o1,octet o2,octet o3,octet o4){
	address[0] = o1;
	address[1] = o2;
	address[2] = o3;
	address[3] = o4;
}

ParameterBool_t::ParameterBool_t():value(false){};
ParameterBool_t::ParameterBool_t(Parameter_t* P):Parameter_t(P),value(false){};
bool ParameterBool_t::addToCDRMessage(CDRMessage_t* msg){
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	octet val = value ? 1:0;
	valid &= CDRMessage::addOctet(msg,val);
	valid &= CDRMessage::addOctet(msg,0);
	valid &= CDRMessage::addUInt16(msg,0);
	return valid;
}

ParameterCount_t::ParameterCount_t():count(0){};
ParameterCount_t::ParameterCount_t(Parameter_t*P):Parameter_t(P),count(0){};
bool ParameterCount_t::addToCDRMessage(CDRMessage_t* msg){l
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addUInt32(msg,count);
	return valid;
}

ParameterEntityId_t::ParameterEntityId_t(){ENTITYID_UNKNOWN(entityId);};
ParameterEntityId_t::ParameterEntityId_t(Parameter_t*P):Parameter_t(P){ENTITYID_UNKNOWN(entityId);};
bool ParameterEntityId_t::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, 4);//this->length);
	valid &= CDRMessage::addEntityId(msg,&entityId);
	return valid;
}




} /* namespace dds */
} /* namespace eprosima */


