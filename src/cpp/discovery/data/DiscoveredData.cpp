/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredData.cpp
 *
 *  Created on: May 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#include "eprosimartps/discovery/data/DiscoveredData.h"

namespace eprosima{

namespace rtps{


bool DiscoveredData::processParameterList(ParameterList_t& param,DiscoveredData_t* data)
{
	for(std::vector<Parameter_t*>::iterator it = param.m_parameters.begin();
			it!=param.m_parameters.end();++it)
	{
		switch((*it)->Pid)
		{
		case PID_PROTOCOL_VERSION:
		{
			ProtocolVersion_t pv;
			PROTOCOLVERSION(pv);
			ParameterProtocolVersion_t * p = (ParameterProtocolVersion_t*)(*it);
			if(p->protocolVersion.m_major < pv.m_major)
			{
				return false;
			}
			data->m_protocolVersion = p->protocolVersion;
			break;
		}
		case PID_VENDORID:
		{
			ParameterVendorId_t * p = (ParameterVendorId_t*)(*it);
			data->m_VendorId[0] = p->vendorId[0];
			data->m_VendorId[1] = p->vendorId[1];
			break;
		}
		case PID_EXPECTS_INLINE_QOS:
		{
			ParameterBool_t * p = (ParameterBool_t*)(*it);
			data->m_expectsInlineQos = p->value;
			break;
		}
		case PID_PARTICIPANT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			data->m_participantGuid = p->guid;
			break;
		}
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			data->m_metatrafficMulticastLocatorList.push_back(p->locator);
			break;
		}
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			data->m_metatrafficUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			data->m_defaultUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			data->m_defaultMulticastLocatorList.push_back(p->locator);
			break;
		}
		case PID_PARTICIPANT_LEASE_DURATION:
		{
			ParameterTime_t* p = (ParameterTime_t*)(*it);
			data->m_leaseDuration = p->time;
			break;
		}
		case PID_BUILTIN_ENDPOINT_SET:
		{
			ParameterBuiltinEndpointSet_t* p = (ParameterBuiltinEndpointSet_t*)(*it);
			data->m_availableBuiltinEndpoints = p->endpointSet;
			break;
		}
		case PID_ENTITY_NAME:
		{
			ParameterString_t* p = (ParameterString_t*)(*it);
			data->m_participantName = p->m_string;
			break;
		}
		case PID_PROPERTY_LIST:
		{
			if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			{
				ParameterPropertyList_t* p = (ParameterPropertyList_t*)(*it);
				uint16_t userId;
				EntityId_t entityId;
				for(std::vector<std::pair<std::string,std::string>>::iterator it = p->properties.begin();
						it != p->properties.end();++it)
				{
					std::stringstream ss;
					std::string first_str = it->first.substr(0, it->first.find("_"));
					if(first_str == "staticedp")
					{
						std::string type = it->first.substr(10, it->first.find("_"));
						first_str = it->first.substr(17, it->first.find("_"));
						ss.clear();	ss.str(std::string());
						ss << first_str;ss >> userId;
						ss.clear();ss.str(std::string());
						ss << it->second;
						int a,b,c,d;
						char ch;
						ss >> a >> ch >> b >> ch >> c >>ch >> d;
						entityId.value[0] = (octet)a;entityId.value[1] = (octet)b;
						entityId.value[2] = (octet)c;entityId.value[3] = (octet)d;
						assignUserId(type,userId,entityId,Pdata);
					}
				}
			}
			break;
		}
		default: break;
		}
	}

}

bool eprosima::rtps::DiscoveredData::DiscoveredData2WriterData(
		DiscoveredData_t* ddata, DiscoveredWriterData* wdata) {
	return true;
}

bool eprosima::rtps::DiscoveredData::DiscoveredData2ReaderData(
		DiscoveredData_t* ddata, DiscoveredReaderData* wdata) {
	return true;
}


bool eprosima::rtps::DiscoveredData::DiscoveredData2TopicData(
		DiscoveredData_t* ddata, DiscoveredTopicData* wdata) {
	return true;
}

bool eprosima::rtps::DiscoveredData::DiscoveredData2ParticipantData(
		DiscoveredData_t* ddata, DiscoveredParticipantData* wdata) {
	return true;
}


}
}
