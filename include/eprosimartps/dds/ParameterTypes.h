/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterTypes.h
 *
 *  Created on: Mar 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARAMETERTYPES_H_
#define PARAMETERTYPES_H_

#include "eprosimartps/CDRMessage.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {

//!Base Parameter class with parameter PID and parameter length in bytes.
class Parameter_t {
public:
	ParameterId_t Pid;
	uint16_t length;
	Parameter_t();
	virtual ~Parameter_t();
	Parameter_t(ParameterId_t pid,uint16_t length);
	virtual bool addToCDRMessage(CDRMessage_t* msg) = 0;
};

class ParameterLocator_t: public Parameter_t {
public:
	Locator_t locator;
	ParameterLocator_t(){};
	ParameterLocator_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	bool addToCDRMessage(CDRMessage_t* msg);
};
#define PARAMETER_LOCATOR_LENGTH 24

class ParameterString_t: public Parameter_t {
public:
	std::string m_string;
	ParameterString_t(){};
	ParameterString_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterPort_t: public Parameter_t {
public:
	uint32_t port;
	ParameterPort_t():port(0){};
	ParameterPort_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),port(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_PORT_LENGTH 4

class ParameterGuid_t: public Parameter_t {
public:
	GUID_t guid;
	ParameterGuid_t(){GUID_UNKNOWN(guid)};
	ParameterGuid_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){GUID_UNKNOWN(guid)};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_GUID_LENGTH 16

class ParameterProtocolVersion_t: public Parameter_t {
public:
	ProtocolVersion_t protocolVersion;
	ParameterProtocolVersion_t(){PROTOCOLVERSION(protocolVersion)};
	ParameterProtocolVersion_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){PROTOCOLVERSION(protocolVersion)};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_PROTOCOL_LENGTH 4

class ParameterVendorId_t:public Parameter_t{
public:
	VendorId_t vendorId;
	ParameterVendorId_t(){VENDORID_EPROSIMA(vendorId);};
	ParameterVendorId_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){VENDORID_EPROSIMA(vendorId);};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_VENDOR_LENGTH 4

class ParameterIP4Address_t :public Parameter_t{
public:
	octet address[4];
	ParameterIP4Address_t(){this->setIP4Address(0,0,0,0);};
	ParameterIP4Address_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){this->setIP4Address(0,0,0,0);};
	bool addToCDRMessage(CDRMessage_t* msg);
	void setIP4Address(octet o1,octet o2,octet o3,octet o4);
};

#define PARAMETER_IP4_LENGTH 4

class ParameterBool_t:public Parameter_t{
public:
	bool value;
	ParameterBool_t():value(false){};
	ParameterBool_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),value(false){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_BOOL_LENGTH 4

class ParameterCount_t:public Parameter_t{
public:
	Count_t count;
	ParameterCount_t():count(0){};
	ParameterCount_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),count(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_COUNT_LENGTH 4

class ParameterEntityId_t:public Parameter_t{
public:
	EntityId_t entityId;
	ParameterEntityId_t():entityId(ENTITYID_UNKNOWN){};
	ParameterEntityId_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),entityId(ENTITYID_UNKNOWN){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_ENTITYID_LENGTH 4

class ParameterTime_t:public Parameter_t{
public:
	Time_t time;
	ParameterTime_t(){};
	ParameterTime_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_TIME_LENGTH 8


class ParameterBuiltinEndpointSet_t:public Parameter_t{
public:
	BuiltinEndpointSet_t endpointSet;
	ParameterBuiltinEndpointSet_t():endpointSet(0){};
	ParameterBuiltinEndpointSet_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),endpointSet(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_BUILTINENDPOINTSET_LENGTH 4


class ParameterUserData_t: public Parameter_t {
public:
	std::vector<octet> value;
	ParameterUserData_t();
	bool addToCDRMessage(CDRMessage_t* msg);
};

} //end of namespace dds
} //end of namespace eprosima


#endif /* PARAMETERTYPES_H_ */

