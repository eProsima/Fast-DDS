/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rtps_elem_parameter.h
 *
 *  Created on: Feb 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RTPS_ELEM_PARAMETER_H_
#define RTPS_ELEM_PARAMETER_H_


typedef short ParameterId_t;

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


//!@brief Structure Parameter_t, to store Qos parameters.
typedef struct Parameter_t{
	ParameterId_t parameterId;
	int16_t length; //length of the parameter. Must be a multiple of 4.
	octet* value; //pointer to the octet values
	Parameter_t(){
		parameterId = PID_SENTINEL;
		length = 0;
		value = NULL;
	}
	Parameter_t(short len){
		parameterId = PID_SENTINEL;
		length = len;
		value = (octet*)malloc(len);
	}
	~Parameter_t(){
//		if(value!=NULL)
//			free(value);
	}
	bool reset(short len){
		parameterId = PID_SENTINEL;
		length = len;
		if(value!=NULL)
			free(value);
		value= (octet*)malloc(len);
		return true;
	}
	bool create(ParameterId_t id,std::string str){
		switch(id){
		case PID_TOPIC_NAME:
		case PID_TYPE_NAME:
		{
			parameterId = id;
			length = str.size();
			int rest = length%4;
			cout << "length, rest " << length << " " << rest << endl;
			if(value!=NULL)
				free(value);
			if(rest!=0){
				value = (octet*)malloc(length+4-rest);
				memset(value+length,'\0',4-rest);
				length+=(4-rest);
				memcpy(value,str.c_str(),length);
			}
			else{
				value = (octet*)malloc(length);
				memcpy(value,str.c_str(),length);
			}
			return true;
			break;
		}
		default:
			break;
		}
		return false;
	}
	bool createParameterLocator(ParameterId_t Pid, Locator_t loc){
		parameterId = Pid;
		length = 24;
		octet* o;
		o =(octet*)&(loc.kind);
		value = (octet*)malloc(length);
		value[0] = o[0];value[1] = o[1];value[2] = o[2];value[3] = o[3];
		o = (octet*)&(loc.port);
		value[4] = o[0];value[5] = o[1];value[6] = o[2];value[7] = o[3];
		for(int i=0;i<16;i++)
		 value[8+i] = loc.address[i];
		return true;
	}

}Parameter_t;


class ParameterCreator{

public:
	static bool createParameterLocator(Parameter_t* p,ParameterId_t pid, Locator_t loc)
	{

	}
	static bool
};

typedef std::vector<Parameter_t> ParameterList_t;




#endif /* RTPS_ELEM_PARAMETER_H_ */
