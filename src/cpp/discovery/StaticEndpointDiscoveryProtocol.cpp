/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEndpointDiscoveryProtocol.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/StaticEndpointDiscoveryProtocol.h"


#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/Participant.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace eprosima {
namespace rtps {

StaticEndpointDiscoveryProtocol::StaticEndpointDiscoveryProtocol(Participant* p_par):
		mp_Participant(p_par)
{

}

StaticEndpointDiscoveryProtocol::~StaticEndpointDiscoveryProtocol() {
	// TODO Auto-generated destructor stub
}

bool StaticEndpointDiscoveryProtocol::loadStaticEndpointFile(const std::string& filename)
{
	this->m_StaticParticipantInfo.clear();
	// Create an empty property tree object
	using boost::property_tree::ptree;
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
	BOOST_FOREACH(ptree::value_type& xml_participant ,pt.get_child("staticdiscovery.participant"))
	{
		ParticipantStaticInfo_t participantInfo;
		participantInfo.m_name = xml_participant.second.get<std::string>("participantName");
		BOOST_FOREACH(ptree::value_type& xml_endpoint ,xml_participant.second.get_child("participant.endpoint"))
		{
			EndpointStaticInfo_t endpointInfo;
			endpointInfo.m_expectsInlineQos = xml_endpoint.second.get<bool>("expectsInlineQos");
			//FIXME: handle expections when parameters are not found.
			std::string auxString = xml_endpoint.second.get<std::string>("type");
			if(auxString == "READER")
				endpointInfo.m_kind = READER;
			else if (auxString == "WRITER")
				endpointInfo.m_kind = WRITER;
			else
			{
				pError("Bad XML file, endpoint of type: " << auxString << " is not valid"<<endl);
				break;
			}
			auxString = xml_endpoint.second.get<std::string>("stateKind");
			if(auxString == "STATELESS")
				endpointInfo.m_state = STATELESS;
			else if (auxString == "STATEFUL")
				endpointInfo.m_state = STATEFUL;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				break;
			}
			auxString = xml_endpoint.second.get<std::string>("realibilityKind");
			if(auxString == "RELIABLE")
			{
				if(endpointInfo.m_state == STATEFUL)
					endpointInfo.m_reliability = RELIABLE;
				else
				{
					pError("Unsupported combination of realiabilityKind and stateKind"<<endl);
					break;
				}
			}
			else if(auxString == "BEST_EFFORT")
			{
				if(endpointInfo.m_state == STATELESS)
					endpointInfo.m_reliability = BEST_EFFORT;
				else
				{
					pError("Unsupported combination of realiabilityKind and stateKind"<<endl);
					break;
				}
			}
			else
			{
				pError("Bad XML file, endpoint of reliabilityKind: " << auxString << " is not valid"<<endl);
				break;
			}
			endpointInfo.m_topicName = xml_endpoint.second.get<std::string>("topicName");
			Locator_t loc;
			BOOST_FOREACH(ptree::value_type &xml_unicast,xml_endpoint.second.get_child("endpoint.unicastLocator"))
			{
				loc.kind = 1;
				auxString = xml_unicast.second.get<std::string>("address");
				loc.set_IP4_address(auxString);
				loc.port = xml_unicast.second.get<uint32_t>("port");
				endpointInfo.m_unicastLocatorList.push_back(loc);
			}
			BOOST_FOREACH(ptree::value_type &xml_multicast,xml_endpoint.second.get_child("endpoint.multicastLocator"))
			{
				loc.kind = 1;
				auxString = xml_multicast.second.get<std::string>("address");
				loc.set_IP4_address(auxString);
				loc.port = xml_multicast.second.get<uint32_t>("port");
				endpointInfo.m_multicastLocatorList.push_back(loc);
			}
			participantInfo.m_endpoints.push_back(endpointInfo);
		}
		this->m_StaticParticipantInfo.push_back(participantInfo);
	}
	return true;
}

bool StaticEndpointDiscoveryProtocol::printLoadedXMLInfo()
{
	pInfo("Printing Loaded XML Info"<<endl);
	pInfo("Number of participants: " <<this->m_StaticParticipantInfo.size());
	pLongInfoPrint;
	std::string auxString;
	for(std::vector<ParticipantStaticInfo_t>::iterator pit =this->m_StaticParticipantInfo.begin();
			pit!=m_StaticParticipantInfo.end();++pit)
	{
		pInfo("Participant with name: " << pit->m_name <<" has " << pit->m_endpoints.size() << "endpoints"<<endl);
		for(std::vector<EndpointStaticInfo_t>::iterator eit =pit->m_endpoints.begin();
				eit!=pit->m_endpoints.end();++eit)
		{
			auxString = eit->m_state == STATELESS ? "STATELESS" : "STATEFUL";
			pLongInfo("Endoint " << auxString << " ");
			auxString = eit->m_kind == READER ? "READER" : "WRITER";
			pLongInfo(auxString << " expectsInlineQos: " << eit->m_expectsInlineQos <<endl);
			for(std::vector<Locator_t>::iterator lit = eit->m_unicastLocatorList.begin();
					lit!=eit->m_unicastLocatorList.end();++lit)
				pLongInfo("Unicast Locator: " << lit->to_IP4_string() << ":" << lit->port << endl);

			for(std::vector<Locator_t>::iterator lit = eit->m_multicastLocatorList.begin();
					lit!=eit->m_multicastLocatorList.end();++lit)
				pLongInfo("Multicast Locator: " << lit->to_IP4_string() << ":" << lit->port << endl);

			pLongInfoPrint;

		}
	}
	return true;
}


bool StaticEndpointDiscoveryProtocol::remoteParticipantMatching(std::string participant_name,GuidPrefix_t& outpartGuidPrefix, Participant* p_MyPart)
{
	ParticipantStaticInfo_t* p_OutPart;
	bool found = false;
	for(std::vector<ParticipantStaticInfo_t>::iterator it= m_StaticParticipantInfo.begin();
			it!=m_StaticParticipantInfo.end();++it)
	{
		if(it->m_name == participant_name)
		{
			p_OutPart = &(*it);
			found = true;
			break;
		}
	}
	if(!found)
	{
		pWarning("Participant not defined in Static Information"<<endl);
		return false;
	}
	for(std::vector<EndpointStaticInfo_t>::iterator eit=p_OutPart->m_endpoints.begin();
			eit!=p_OutPart->m_endpoints.end();++eit)
	{
		if(eit->m_kind == READER)
		{
			RTPSWriter* p_rit;
			for(std::vector<RTPSWriter*>::iterator rit = p_MyPart->m_writerList.begin();
					rit!=p_MyPart->m_writerList.end();++rit)
			{
				p_rit = *rit;
				if(p_rit->getTopicName() == eit->m_topicName &&
						p_rit->getStateType() == eit->m_state)
				{
					if(p_rit->getStateType() == STATELESS)
					{
						StatelessWriter* p_SLW = (StatelessWriter*)p_rit;
						ReaderLocator RL;
						RL.expectsInlineQos = eit->m_expectsInlineQos;
						for(std::vector<Locator_t>::iterator lit = eit->m_unicastLocatorList.begin();
								lit != eit->m_unicastLocatorList.end();++lit)
						{
							RL.locator = *lit;
							p_SLW->reader_locator_add(RL);
						}
						for(std::vector<Locator_t>::iterator lit = eit->m_multicastLocatorList.begin();
								lit != eit->m_multicastLocatorList.end();++lit)
						{
							RL.locator = *lit;
							p_SLW->reader_locator_add(RL);
						}
					}
					else if(p_rit->getStateType() == STATEFUL)
					{
						StatefulWriter* p_SFW = (StatefulWriter*)p_rit;
						ReaderProxy_t RP;
						RP.expectsInlineQos = eit->m_expectsInlineQos;
						RP.unicastLocatorList = eit->m_unicastLocatorList;
						RP.multicastLocatorList = eit->m_multicastLocatorList;
						RP.remoteReaderGuid.guidPrefix = outpartGuidPrefix;
						RP.remoteReaderGuid.entityId = ENTITYID_UNKNOWN;
						p_SFW->matched_reader_add(RP);
					}
				}
			}
		}
		else if(eit->m_kind == WRITER)
		{
			RTPSReader* p_rit;
			for(std::vector<RTPSReader*>::iterator rit = p_MyPart->m_readerList.begin();
					rit!=p_MyPart->m_readerList.end();++rit)
			{
				p_rit = *rit;
				if(p_rit->m_topicName == eit->m_topicName &&
						p_rit->m_stateType == eit->m_state)
				{
					if(p_rit->m_stateType == STATELESS)
					{

					}
					else if(p_rit->m_stateType == STATEFUL)
					{
						StatefulReader* p_SFR = (StatefulReader*)p_rit;
						WriterProxy_t WP;
						WP.unicastLocatorList = eit->m_unicastLocatorList;
						WP.multicastLocatorList = eit->m_multicastLocatorList;
						WP.remoteWriterGuid.guidPrefix = outpartGuidPrefix;
						WP.remoteWriterGuid.entityId = ENTITYID_UNKNOWN;
						p_SFR->matched_writer_add(&WP);
					}
				}
			}
		}
	}
	return true;
}


bool StaticEndpointDiscoveryProtocol::localWriterMatching(RTPSWriter* writer)
{
	std::string topic_name = writer->getTopicName();
	std::vector<std::string> matched_part_names = mp_Participant->m_SPDP.getMatchedParticipantsNames();
	//Look in all the participants that have been found by the SPDP
	for(std::vector<std::string>::iterator it = matched_part_names.begin();
			it!=matched_part_names.end();++it)
	{
		//Look in the participants defined by the StaticEndpointDiscovery.
		for(std::vector<ParticipantStaticInfo_t>::iterator remotepit = m_StaticParticipantInfo.begin();
				remotepit != m_StaticParticipantInfo.end();++remotepit)
		{
			if(*it == remotepit->m_name) // Found a match, begin pairing
			{
				for(std::vector<EndpointStaticInfo_t>::iterator eit = remotepit->m_endpoints.begin();
						eit!=remotepit->m_endpoints.end();++eit)
				{
					if(eit->m_kind == READER && writer->getStateType() == eit->m_state)
					{

					}
				}
			}
		}
	}
return true;
}

bool StaticEndpointDiscoveryProtocol::localReaderMatching(RTPSReader* reader)
{
return false;
}


} /* namespace rtps */
} /* namespace eprosima */
