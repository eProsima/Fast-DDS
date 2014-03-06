/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ParameterList.h
 *
 *  Created on: Mar 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARAMETERLIST_H_
#define PARAMETERLIST_H_

#include <vector>
#include <string>
#include <algorithm>

#include "rtps_all.h"

//using namespace eprosima::rtps;

namespace eprosima {
namespace rtps {

//Parameter Types
#define PID_PAD 0x0000
#define PID_SENTINEL 0x0001
#define PID_USER_DATA 0x002c
#define PID_TOPIC_NAME 0x0005
#define PID_TYPE_NAME 0x0007
#define PID_GROUP_DATA 0x002d
#define PID_TOPIC_DATA 0x002e
#define PID_DURABILITY 0x001d
#define PID_DURABILITY_SERVICE 0x001e
#define PID_DEADLINE 0x0023
#define PID_LATENCY_BUDGET 0x0027
#define PID_LIVELINESS 0x001b
#define PID_RELIABILITY 0x001A
#define PID_LIFESPAN 0x002b
#define PID_DESTINATION_ORDER 0x0025
#define PID_HISTORY 0x0040
#define PID_RESOURCE_LIMITS 0x0041
#define PID_OWNERSHIP 0x001f
#define PID_OWNERSHIP_STRENGTH 0x0006
#define PID_PRESENTATION 0x0021
#define PID_PARTITION 0x0029
#define PID_TIME_BASED_FILTER 0x0004
#define PID_TRANSPORT_PRIORITY 0x0049
#define PID_PROTOCOL_VERSION 0x0015
#define PID_VENDORID 0x0016
#define PID_UNICAST_LOCATOR 0x002f
#define PID_MULTICAST_LOCATOR 0x0030
#define PID_MULTICAST_IPADDRESS 0x0011
#define PID_DEFAULT_UNICAST_LOCATOR 0x0031
#define PID_DEFAULT_MULTICAST_LOCATOR 0x0048
#define PID_METATRAFFIC_UNICAST_LOCATOR 0x0032
#define PID_METATRAFFIC_MULTICAST_LOCATOR 0x0033
#define PID_DEFAULT_UNICAST_IPADDRESS 0x000c
#define PID_DEFAULT_UNICAST_PORT 0x000e
#define PID_METATRAFFIC_UNICAST_IPADDRESS 0x0045
#define PID_METATRAFFIC_UNICAST_PORT 0x000d
#define PID_METATRAFFIC_MULTICAST_IPADDRESS 0x000b
#define PID_METATRAFFIC_MULTICAST_PORT 0x0046
#define PID_EXPECTS_INLINE_QOS 0x0043
#define PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT 0x0034
#define PID_PARTICIPANT_BUILTIN_ENDPOINTS 0x0044
#define PID_PARTICIPANT_LEASE_DURATION 0x0002
#define PID_CONTENT_FILTER_PROPERTY 0x0035
#define PID_PARTICIPANT_GUID 0x0050
#define PID_PARTICIPANT_ENTITYID 0x0051
#define PID_GROUP_GUID 0x0052
#define PID_GROUP_ENTITYID 0x0053
#define PID_BUILTIN_ENDPOINT_SET 0x0058
#define PID_PROPERTY_LIST 0x0059
#define PID_TYPE_MAX_SIZE_SERIALIZED 0x0060
#define PID_ENTITY_NAME 0x0062
#define PID_KEY_HASH 0x0070
#define PID_STATUS_INFO 0x0071


typedef uint16_t ParameterId_t;





/** @defgroup inlineQoS Parameters
 * @ingroup RTPSMODULE
 * Specialization of parameter class for each of the inlineQos parameters.
 *  @{
 */
//!Base Parameter class with parameter Pid and parameter length in bytes.
class Parameter_t{
public:
	ParameterId_t Pid;
	uint16_t length;
};

class ParameterLocator_t:public Parameter_t{
public:
	Locator_t locator;
};

class ParameterString_t:public Parameter_t{
public:
	std::string p_str;
};

class ParameterPort_t:public Parameter_t{
public:
	uint32_t port;
};
/** @} */ // end of group1


/**
 * Class ParameterList to group all inlineQos parameters of a publisher or subscriber.
 * @ingroup RTPSMODULE
 */
class RTPS_DllAPI ParameterList {
public:
	ParameterList();
	virtual ~ParameterList();
	std::vector<Parameter_t*> params;
	CDRMessage_t ParamsMsg;
	bool has_changed;
	bool addParameterLocator(ParameterId_t pid,rtps::Locator_t loc);
	bool addParameterString(ParameterId_t pid,std::string in_str);
	bool addParameterPort(ParameterId_t pid,uint32_t port);
	bool updateMsg();
	bool assignParamList(CDRMessage_t* msg,uint32_t* size);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARAMETERLIST_H_ */
