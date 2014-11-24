/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterTypes.h
*/

#ifndef PARAMETERTYPES_H_
#define PARAMETERTYPES_H_

#include "fastrtps/rtps/common/all_common.h"
#include "fastrtps/config/eprosima_stl_exports.hpp"

#include <string>
#include <vector>

using namespace eprosima::fastrtps::rtps;


namespace eprosima {
namespace fastrtps {

namespace rtps{
struct CDRMessage_t;
}


using namespace rtps;




#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * @addtogroup PARAMETERMODULE
 * @{
 */

enum ParameterId_t	:uint16_t
{
	PID_PAD = 0x0000,
			PID_SENTINEL = 0x0001,
			PID_USER_DATA = 0x002c,
			PID_TOPIC_NAME = 0x0005,
			PID_TYPE_NAME = 0x0007,
			PID_GROUP_DATA =0x002d,
			PID_TOPIC_DATA =0x002e,
			PID_DURABILITY =0x001d,
			PID_DURABILITY_SERVICE =0x001e,
			PID_DEADLINE =0x0023,
			PID_LATENCY_BUDGET =0x0027,
			PID_LIVELINESS =0x001b,
			PID_RELIABILITY =0x001A,
			PID_LIFESPAN =0x002b,
			PID_DESTINATION_ORDER =0x0025,
			PID_HISTORY =0x0040,
			PID_RESOURCE_LIMITS =0x0041,
			PID_OWNERSHIP =0x001f,
			PID_OWNERSHIP_STRENGTH =0x0006,
			PID_PRESENTATION =0x0021,
			PID_PARTITION =0x0029,
			PID_TIME_BASED_FILTER =0x0004,
			PID_TRANSPORT_PRIORITY =0x0049,
			PID_PROTOCOL_VERSION = 0x0015,
			PID_VENDORID = 0x0016,
			PID_UNICAST_LOCATOR = 0x002f,
			PID_MULTICAST_LOCATOR = 0x0030,
			PID_MULTICAST_IPADDRESS =0x0011,
			PID_DEFAULT_UNICAST_LOCATOR = 0x0031,
			PID_DEFAULT_MULTICAST_LOCATOR = 0x0048,
			PID_METATRAFFIC_UNICAST_LOCATOR = 0x0032,
			PID_METATRAFFIC_MULTICAST_LOCATOR = 0x0033,
			PID_DEFAULT_UNICAST_IPADDRESS =0x000c,
			PID_DEFAULT_UNICAST_PORT = 0x000e,
			PID_METATRAFFIC_UNICAST_IPADDRESS =0x0045,
			PID_METATRAFFIC_UNICAST_PORT = 0x000d,
			PID_METATRAFFIC_MULTICAST_IPADDRESS =0x000b,
			PID_METATRAFFIC_MULTICAST_PORT = 0x0046,
			PID_EXPECTS_INLINE_QOS =0x0043,
			PID_RTPSParticipant_MANUAL_LIVELINESS_COUNT =0x0034,
			PID_RTPSParticipant_BUILTIN_ENDPOINTS = 0x0044,
			PID_RTPSParticipant_LEASE_DURATION = 0x0002,
			PID_CONTENT_FILTER_PROPERTY =0x0035,
			PID_RTPSParticipant_GUID = 0x0050,
			PID_RTPSParticipant_ENTITYID =0x0051,
			PID_GROUP_GUID =0x0052,
			PID_GROUP_ENTITYID =0x0053,
			PID_BUILTIN_ENDPOINT_SET = 0x0058,
			PID_PROPERTY_LIST = 0x0059,
			PID_TYPE_MAX_SIZE_SERIALIZED =0x0060,
			PID_ENTITY_NAME = 0x0062,
			PID_KEY_HASH = 0x0070,
			PID_STATUS_INFO = 0x0071,
			PID_ENDPOINT_GUID = 0x005a
};









//!Base Parameter class with parameter PID and parameter length in bytes.
class RTPS_DllAPI Parameter_t {
public:
	ParameterId_t Pid;
	uint16_t length;
	Parameter_t();
	virtual ~Parameter_t();
	/**
	 * Constructor using a parameter PID and the parameter length
	 * @param pid Pid of the parameter
	 * @param length Its associated length
	 */
	Parameter_t(ParameterId_t pid,uint16_t length);
	/**
	 * Virtual method used to add the parameter to a CDRMessage_t message.
	 * @param[in,out] msg Pointer to the message where the parameter should be added.
	 * @return True if the parameter was correctly added.
	 */
	virtual bool addToCDRMessage(CDRMessage_t* msg) = 0;
};

class RTPS_DllAPI ParameterKey_t:public Parameter_t{
public:
	InstanceHandle_t key;
	ParameterKey_t(){};
	ParameterKey_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	ParameterKey_t(ParameterId_t pid,uint16_t in_length,InstanceHandle_t& ke):Parameter_t(pid,in_length),key(ke){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI ParameterLocator_t: public Parameter_t {
public:
	Locator_t locator;
	ParameterLocator_t(){};
	ParameterLocator_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	ParameterLocator_t(ParameterId_t pid,uint16_t in_length,Locator_t& loc):Parameter_t(pid,in_length),locator(loc){};
	bool addToCDRMessage(CDRMessage_t* msg);
};
#define PARAMETER_LOCATOR_LENGTH 24

class RTPS_DllAPI ParameterString_t: public Parameter_t {
public:
	std::string m_string;
	ParameterString_t(){};
	ParameterString_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	ParameterString_t(ParameterId_t pid,uint16_t in_length,std::string& strin):Parameter_t(pid,in_length),m_string(strin){}
	bool addToCDRMessage(CDRMessage_t* msg);
};

class RTPS_DllAPI ParameterPort_t: public Parameter_t {
public:
	uint32_t port;
	ParameterPort_t():port(0){};
	ParameterPort_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),port(0){};
	ParameterPort_t(ParameterId_t pid,uint16_t in_length,uint32_t po):Parameter_t(pid,in_length),port(po){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_PORT_LENGTH 4

class RTPS_DllAPI ParameterGuid_t: public Parameter_t {
public:
	GUID_t guid;
	ParameterGuid_t(){GUID_UNKNOWN(guid);};
	ParameterGuid_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){GUID_UNKNOWN(guid);};
	ParameterGuid_t(ParameterId_t pid,uint16_t in_length,GUID_t guidin):Parameter_t(pid,in_length),guid(guidin){};
	ParameterGuid_t(ParameterId_t pid,uint16_t in_length,InstanceHandle_t& iH):Parameter_t(pid,in_length)
	{
		for(uint8_t i =0;i<16;++i)
		{
			if(i<12)
				guid.guidPrefix.value[i] = iH.value[i];
			else
				guid.entityId.value[i-12] = iH.value[i];
		}
	};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_GUID_LENGTH 16

class RTPS_DllAPI ParameterProtocolVersion_t: public Parameter_t {
public:
	ProtocolVersion_t protocolVersion;
	ParameterProtocolVersion_t(){PROTOCOLVERSION(protocolVersion);};
	ParameterProtocolVersion_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){PROTOCOLVERSION(protocolVersion);};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_PROTOCOL_LENGTH 4

class RTPS_DllAPI ParameterVendorId_t:public Parameter_t{
public:
	VendorId_t vendorId;
	ParameterVendorId_t(){VENDORID_EPROSIMA(vendorId);};
	ParameterVendorId_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){VENDORID_EPROSIMA(vendorId);};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_VENDOR_LENGTH 4

class RTPS_DllAPI ParameterIP4Address_t :public Parameter_t{
public:
	octet address[4];
	ParameterIP4Address_t(){this->setIP4Address(0,0,0,0);};
	ParameterIP4Address_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){this->setIP4Address(0,0,0,0);};
	bool addToCDRMessage(CDRMessage_t* msg);
	void setIP4Address(octet o1,octet o2,octet o3,octet o4);
};

#define PARAMETER_IP4_LENGTH 4

class RTPS_DllAPI ParameterBool_t:public Parameter_t{
public:
	bool value;
	ParameterBool_t():value(false){};
	ParameterBool_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),value(false){};
	ParameterBool_t(ParameterId_t pid,uint16_t in_length,bool inbool):Parameter_t(pid,in_length),value(inbool){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_BOOL_LENGTH 4

class RTPS_DllAPI ParameterCount_t:public Parameter_t{
public:
	Count_t count;
	ParameterCount_t():count(0){};
	ParameterCount_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),count(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_COUNT_LENGTH 4

class RTPS_DllAPI ParameterEntityId_t:public Parameter_t{
public:
	EntityId_t entityId;
	ParameterEntityId_t():entityId(ENTITYID_UNKNOWN){};
	ParameterEntityId_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),entityId(ENTITYID_UNKNOWN){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_ENTITYID_LENGTH 4

class RTPS_DllAPI ParameterTime_t:public Parameter_t{
public:
	Time_t time;
	ParameterTime_t(){};
	ParameterTime_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_TIME_LENGTH 8


class RTPS_DllAPI ParameterBuiltinEndpointSet_t:public Parameter_t{
public:
	BuiltinEndpointSet_t endpointSet;
	ParameterBuiltinEndpointSet_t():endpointSet(0){};
	ParameterBuiltinEndpointSet_t(ParameterId_t pid,uint16_t in_length):Parameter_t(pid,in_length),endpointSet(0){};
	bool addToCDRMessage(CDRMessage_t* msg);
};

#define PARAMETER_BUILTINENDPOINTSET_LENGTH 4

class RTPS_DllAPI ParameterPropertyList_t:public Parameter_t{
public:
	std::vector<std::pair<std::string,std::string>> properties;
	ParameterPropertyList_t():Parameter_t(PID_PROPERTY_LIST,0){};
	ParameterPropertyList_t(ParameterId_t pid,uint16_t in_length):Parameter_t(PID_PROPERTY_LIST,in_length){};
	bool addToCDRMessage(CDRMessage_t* msg);
};



///@}

#endif

} //end of namespace
} //end of namespace eprosima


#endif /* PARAMETERTYPES_H_ */

