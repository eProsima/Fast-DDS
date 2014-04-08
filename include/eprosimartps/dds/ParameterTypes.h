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
	Parameter_t(Parameter_t* P);
	virtual bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterLocator_t: public Parameter_t {
public:
	Locator_t locator;
	ParameterLocator_t();
	ParameterLocator_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterString_t: public Parameter_t {
public:
	std::string m_string;
	ParameterString_t();
	ParameterString_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterPort_t: public Parameter_t {
public:
	uint32_t port;
	ParameterPort_t();
	ParameterPort_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterGuid_t: public Parameter_t {
	GUID_t guid;
	ParameterGuid_t();
	ParameterGuid_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterProtocolVersion_t: public Parameter_t {
	ProtocolVersion_t protocolVersion;
	ParameterProtocolVersion_t();
	ParameterProtocolVersion_t(Parameter_t*P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterVendorId_t:public Parameter_t{
	VendorId_t vendorId;
	ParameterVendorId_t();
	ParameterVendorId_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterIP4Address_t :public Parameter_t{
	octet address[4];
	ParameterIP4Address_t();
	ParameterIP4Address_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
	void setIP4Address(octet o1,octet o2,octet o3,octet o4);
};

class ParameterBool_t:public Parameter_t{
	bool value;
	ParameterBool_t();
	ParameterBool_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterCount_t:public Parameter_t{
	Count_t count;
	ParameterCount_t();
	ParameterCount_t(Parameter_t*P);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterEntityId_t:public Parameter_t{
	EntityId_t entityId;
	ParameterEntityId_t();
	ParameterEntityId_t(Parameter_t*);
	bool addToCDRMessage(CDRMessage_t* msg);
};

class ParameterUserData_t: public Parameter_t {
	std::vector<octet> value;
	ParameterUserData_t();
	ParameterUserData_t(Parameter_t* P);
	bool addToCDRMessage(CDRMessage_t* msg);
};


} //end of namespace dds
} //end of namespace eprosima


#endif /* PARAMETERTYPES_H_ */

