/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEDP.cpp
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/StaticEDP.h"

#include "eprosimartps/Participant.h"



namespace eprosima {
namespace rtps {
using boost::property_tree::ptree;

StaticEDP::StaticEDP() {
	// TODO Auto-generated constructor stub

}

StaticEDP::~StaticEDP() {
	// TODO Auto-generated destructor stub
}


bool StaticEDP::loadXMLFile(const std::string& filename)
{
	// Create an empty property tree object

	ptree pt;
	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try{
		read_xml(filename, pt);
	}
	catch (std::exception &e)
	{
		pError("Error reading xml file: " << e.what() << endl);
		return false;
	}

	m_endpointIds.clear();
	BOOST_FOREACH(ptree::value_type& xml_participant ,pt.get_child("staticdiscovery"))
	{
		if(xml_participant.first == "participant")
		{
			DiscoveredParticipantData pdata;
			BOOST_FOREACH(ptree::value_type& xml_participant_child,xml_participant.second)
			{
				if(xml_participant_child.first == "name")
					pdata.m_participantName = xml_participant_child.second.data();
				else if(xml_participant_child.first == "endpoint")
				{
					std::string auxString = xml_participant_child.second.get("<xmlattr>.type","");
					if(auxString == "READER")
					{

					}
					else if(auxString == "WRITER")
					{

					}
					else
					{
						pError("Endpoint must be defined as READER or WRITER"<<endl);
					}

				}
			}
			mp_DPDP->m_discoveredParticipants.push_back(pdata);
		}
	}
}

bool StaticEDP::loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata)
{
	DiscoveredWriterData wdata;
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		if(xml_endpoint_child.first == "id")
		{
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(!std::binary_search(m_endpointIds.begin(),m_endpointIds.end(),id))
			{
				wdata.userDefinedId= id;
				m_endpointIds.push_back(id);
			}
			else
			{
				pError("Repeated ID in XML File"<<endl);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			pWarning("Writers don't use expectsInlineQos tag"<<endl);
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			wdata.m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			wdata.m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
				wdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				wdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE")
				endpointInfo.m_reliability = RELIABLE;
			else if (auxString == "BEST_EFFORT")
				endpointInfo.m_reliability = BEST_EFFORT;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			endpointInfo.m_unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			endpointInfo.m_multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			endpointInfo.m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","");
			endpointInfo.m_topicDataType = xml_endpoint_child.second.get("<xmlattr>.dataType","");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","");
			if(auxString == "NO_KEY")
				endpointInfo.m_topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				endpointInfo.m_topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring..."<<endl)
		}








	}
}

bool StaticEDP::loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata)
{
	DiscoveredReaderData rdata;
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		if(xml_endpoint_child.first == "id")
		{
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(!std::binary_search(m_endpointIds.begin(),m_endpointIds.end(),id))
			{
				rdata.userDefinedId= id;
				m_endpointIds.push_back(id);
			}
			else
			{
				pError("Repeated ID in XML File"<<endl);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "true")
				rdata.expectsInlineQos = true;
			else if (auxString == "false")
				rdata.expectsInlineQos = false;
			else
			{
				pError("Bad XML file, endpoint of expectsInlineQos: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			rdata.m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			rdata.m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
				rdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				rdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE")
				endpointInfo.m_reliability = RELIABLE;
			else if (auxString == "BEST_EFFORT")
				endpointInfo.m_reliability = BEST_EFFORT;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			endpointInfo.m_unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			endpointInfo.m_multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			endpointInfo.m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","");
			endpointInfo.m_topicDataType = xml_endpoint_child.second.get("<xmlattr>.dataType","");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","");
			if(auxString == "NO_KEY")
				endpointInfo.m_topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				endpointInfo.m_topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring..."<<endl)
		}
	}
}


} /* namespace rtps */
} /* namespace eprosima */
